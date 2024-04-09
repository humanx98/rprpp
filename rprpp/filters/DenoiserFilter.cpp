#include "DenoiserFilter.h"
#include "rprpp/Error.h"
#include <cassert>

#include <boost/log/trivial.hpp>

constexpr int WorkgroupSize = 32;

namespace rprpp::filters {

DenoiserFilter::DenoiserFilter(Context* context, oidn::DeviceRef& device)
    : Filter(context)
    , m_dirty(true)
    , m_device(device)
    , m_finishedSemaphore(deviceContext().device.createSemaphore({}))
    , m_copyInputsCommands(&deviceContext())
    , m_copyOutputCommand(&deviceContext())
{
}

void DenoiserFilter::validateInputsAndOutput()
{
    if (!m_input) {
        throw InvalidParameter("input", "cannot be null");
    }

    if (!m_output) {
        throw InvalidParameter("output", "cannot be null");
    }

    if (m_input->description().width != m_output->description().width
        || m_input->description().height != m_output->description().height) {
        throw InvalidParameter("output and input", "output and input should have the same image description");
    }

    if (is_ldr(m_input->description().format) || is_ldr(m_output->description().format)) {
        throw InvalidParameter("output and input", "output and input cannot be in low dynamic range");
    }
}

vk::Semaphore DenoiserFilter::run(std::optional<vk::Semaphore> waitSemaphore)
{
    validateInputsAndOutput();

    if (m_dirty) {
        deviceContext().queue.waitIdle();
        initialize();
        m_dirty = false;
    }


    vk::PipelineStageFlags waitStage = vk::PipelineStageFlagBits::eAllCommands;
    vk::SubmitInfo submitInfo;
    submitInfo.setCommandBuffers(*m_copyInputsCommands.get());
    if (waitSemaphore.has_value()) {
        submitInfo.setWaitDstStageMask(waitStage);
        submitInfo.setWaitSemaphores(waitSemaphore.value());
    }

    deviceContext().queue.submit(submitInfo);
    deviceContext().queue.waitIdle();

    // m_filter.executeAsync();
    m_filter.execute();

    // Check for errors
    const char* errorMessage;
    if (m_device.getError(errorMessage) != oidn::Error::None) {
        BOOST_LOG_TRIVIAL(error) << errorMessage;
        throw std::runtime_error(errorMessage);
    }

    submitInfo = vk::SubmitInfo();
    submitInfo.setCommandBuffers(*m_copyOutputCommand.get());
    submitInfo.setSignalSemaphores(*m_finishedSemaphore);
    deviceContext().queue.submit(submitInfo);
    return *m_finishedSemaphore;
}

std::unique_ptr<Buffer> DenoiserFilter::createStagingBufferFor(Image* image)
{
    auto usage = vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eStorageBuffer;
    auto props = vk::MemoryPropertyFlagBits::eDeviceLocal;
    size_t size = image->description().width * image->description().height * sizeof(float) * 4;
    return std::make_unique<Buffer>(image->context(), size, usage, props, true);
}

void DenoiserFilter::initialize()
{
    BOOST_LOG_TRIVIAL(trace) << "DenoiserFilter::initialize";

    m_stagingColorBuffer.reset();
    m_stagingAlbedoBuffer.reset();
    m_stagingNormalBuffer.reset();
    m_filter.release();
    m_colorBuffer.release();
    m_albedoBuffer.release();
    m_normalBuffer.release();

    m_stagingColorBuffer = std::move(createStagingBufferFor(m_input));

    m_copyOutputCommand.get().begin(vk::CommandBufferBeginInfo(vk::CommandBufferUsageFlagBits::eSimultaneousUse));
    copyBufferToImage(m_copyOutputCommand, m_stagingColorBuffer.get(), m_output);
    m_copyOutputCommand.get().end();

    m_copyInputsCommands.get().begin(vk::CommandBufferBeginInfo(vk::CommandBufferUsageFlagBits::eSimultaneousUse));
    copyImageToBuffer(m_copyInputsCommands, m_input, m_stagingColorBuffer.get());
    if (m_stagingAlbedoBuffer.get() && m_stagingNormalBuffer.get()) {
        m_stagingAlbedoBuffer = std::move(createStagingBufferFor(m_albedo));
        copyImageToBuffer(m_copyInputsCommands, m_albedo, m_stagingAlbedoBuffer.get());

        m_stagingNormalBuffer = std::move(createStagingBufferFor(m_normal));
        copyImageToBuffer(m_copyInputsCommands, m_normal, m_stagingNormalBuffer.get());
    }
    m_copyInputsCommands.get().end();

    m_filter = m_device.newFilter("RT"); // generic ray tracing filter

    const uint32_t width = m_input->description().width;
    const uint32_t height = m_input->description().height;
    vk::MemoryGetWin32HandleInfoKHR handleInfo(m_stagingColorBuffer->memory(), vk::ExternalMemoryHandleTypeFlagBits::eOpaqueWin32);
    m_colorBuffer = m_device.newBuffer(oidn::ExternalMemoryTypeFlag::OpaqueWin32, deviceContext().device.getMemoryWin32HandleKHR(handleInfo), nullptr, m_stagingColorBuffer->size());
    m_filter.setImage("color", m_colorBuffer, oidn::Format::Float3, width, height, 0, sizeof(float) * 4, 0); // beauty
    m_filter.setImage("output", m_colorBuffer, oidn::Format::Float3, width, height, 0, sizeof(float) * 4, 0); // denoised beauty
    if (m_stagingAlbedoBuffer.get() && m_stagingNormalBuffer.get()) {
        handleInfo.memory = m_stagingAlbedoBuffer->memory();
        m_albedoBuffer = m_device.newBuffer(oidn::ExternalMemoryTypeFlag::OpaqueWin32, deviceContext().device.getMemoryWin32HandleKHR(handleInfo), nullptr, m_stagingAlbedoBuffer->size());
        m_filter.setImage("albedo", m_albedoBuffer, oidn::Format::Float3, width, height, 0, sizeof(float) * 4, 0); // auxiliary

        handleInfo.memory = m_stagingNormalBuffer->memory();
        m_normalBuffer = m_device.newBuffer(oidn::ExternalMemoryTypeFlag::OpaqueWin32, deviceContext().device.getMemoryWin32HandleKHR(handleInfo), nullptr, m_stagingNormalBuffer->size());
        m_filter.setImage("normal", m_normalBuffer, oidn::Format::Float3, width, height, 0, sizeof(float) * 4, 0); // auxiliary
    }
    m_filter.set("hdr", true); // beauty image is HDR
    m_filter.commit();

    BOOST_LOG_TRIVIAL(debug) << "denoiser filter created";
}

void DenoiserFilter::copyBufferToImage(vk::helper::CommandBuffer& commandBuffer, Buffer* buffer, Image* image)
{
    size_t size = image->description().width * image->description().height * to_pixel_size(image->description().format);
    if (buffer->size() < size) {
        throw InvalidParameter("buffer", "The provided buffer doesn't fit destination image");
    }

    vk::AccessFlags oldAccess = image->access();
    vk::ImageLayout oldLayout = image->layout();
    vk::PipelineStageFlags oldStage = image->stages();

    image->transitionImageLayout(commandBuffer.get(),
        vk::AccessFlagBits::eTransferWrite,
        vk::ImageLayout::eTransferDstOptimal,
        vk::PipelineStageFlagBits::eTransfer);
    {
        vk::ImageSubresourceLayers imageSubresource(vk::ImageAspectFlagBits::eColor, 0, 0, 1);
        vk::BufferImageCopy region(0, 0, 0, imageSubresource, { 0, 0, 0 }, { image->description().width, image->description().height, 1 });
        commandBuffer.get().copyBufferToImage(buffer->get(), image->image(), vk::ImageLayout::eTransferDstOptimal, region);
    }
    image->transitionImageLayout(commandBuffer.get(), oldAccess, oldLayout, oldStage);
}

void DenoiserFilter::copyImageToBuffer(vk::helper::CommandBuffer& commandBuffer, Image* image, Buffer* buffer)
{
    size_t size = image->description().width * image->description().height * to_pixel_size(image->description().format);
    if (buffer->size() < size) {
        throw InvalidParameter("buffer", "The provided buffer doesn't fit destination image");
    }

    vk::AccessFlags oldAccess = image->access();
    vk::ImageLayout oldLayout = image->layout();
    vk::PipelineStageFlags oldStage = image->stages();

    image->transitionImageLayout(commandBuffer.get(),
        vk::AccessFlagBits::eTransferRead,
        vk::ImageLayout::eTransferSrcOptimal,
        vk::PipelineStageFlagBits::eTransfer);
    {
        vk::ImageSubresourceLayers imageSubresource(vk::ImageAspectFlagBits::eColor, 0, 0, 1);
        vk::BufferImageCopy region(0, 0, 0, imageSubresource, { 0, 0, 0 }, { image->description().width, image->description().height, 1 });
        commandBuffer.get().copyImageToBuffer(image->image(), vk::ImageLayout::eTransferSrcOptimal, buffer->get(), region);
    }
    image->transitionImageLayout(commandBuffer.get(), oldAccess, oldLayout, oldStage);
}

void DenoiserFilter::setInput(Image* image)
{
    BOOST_LOG_TRIVIAL(trace) << "DenoiserFilter::setInput";

    assert(image);

    m_input = image;
    m_dirty = true;
}

void DenoiserFilter::setOutput(Image* image)
{
    BOOST_LOG_TRIVIAL(trace) << "DenoiserFilter::setOutput";

    m_output = image;
    m_dirty = true;
}

}