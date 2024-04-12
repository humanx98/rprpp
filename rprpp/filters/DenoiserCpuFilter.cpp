#include "DenoiserCpuFilter.h"
#include "rprpp/Error.h"
#include <cassert>

#include <boost/log/trivial.hpp>

namespace rprpp::filters {

DenoiserCpuFilter::DenoiserCpuFilter(Context* context, oidn::DeviceRef& device)
    : DenoiserFilter(context, device)
{
}

vk::Semaphore DenoiserCpuFilter::run(std::optional<vk::Semaphore> waitSemaphore)
{
    validateInputsAndOutput();

    if (m_dirty) {
        deviceContext().queue.waitIdle();
        m_filter.release();
        m_colorBuffer.release();
        m_albedoBuffer.release();
        m_normalBuffer.release();
        m_stagingColorBuffer.reset();
        m_stagingAlbedoBuffer.reset();
        m_stagingNormalBuffer.reset();
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

    m_filter.execute();
    const char* errorMessage;
    if (m_device.getError(errorMessage) != oidn::Error::None) {
        BOOST_LOG_TRIVIAL(error) << errorMessage;
        throw InternalError(std::string("oidn error: ") + errorMessage);
    }

    submitInfo = vk::SubmitInfo();
    submitInfo.setCommandBuffers(*m_copyOutputCommand.get());
    submitInfo.setSignalSemaphores(*m_finishedSemaphore);
    deviceContext().queue.submit(submitInfo);

    return *m_finishedSemaphore;
}

std::unique_ptr<Buffer> DenoiserCpuFilter::createStagingBufferFor(Image* image)
{
    auto usage = vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eTransferDst;
    auto props = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;
    size_t size = image->description().width * image->description().height * to_pixel_size(image->description().format);
    return std::make_unique<Buffer>(image->context(), size, usage, props);
}

void DenoiserCpuFilter::initialize()
{
    m_stagingColorBuffer = std::move(createStagingBufferFor(m_input));

    m_copyOutputCommand.get().begin(vk::CommandBufferBeginInfo(vk::CommandBufferUsageFlagBits::eSimultaneousUse));
    copyBufferToImage(m_copyOutputCommand, m_stagingColorBuffer.get(), m_output);
    m_copyOutputCommand.get().end();

    m_copyInputsCommands.get().begin(vk::CommandBufferBeginInfo(vk::CommandBufferUsageFlagBits::eSimultaneousUse));
    copyImageToBuffer(m_copyInputsCommands, m_input, m_stagingColorBuffer.get());
    if (m_albedo && m_normal) {
        m_stagingAlbedoBuffer = std::move(createStagingBufferFor(m_albedo));
        copyImageToBuffer(m_copyInputsCommands, m_albedo, m_stagingAlbedoBuffer.get());

        m_stagingNormalBuffer = std::move(createStagingBufferFor(m_normal));
        copyImageToBuffer(m_copyInputsCommands, m_normal, m_stagingNormalBuffer.get());
    }
    m_copyInputsCommands.get().end();

    m_filter = m_device.newFilter("RT"); // generic ray tracing filter

    const uint32_t width = m_input->description().width;
    const uint32_t height = m_input->description().height;
    void* mappedStaginColorBuffer = m_stagingColorBuffer->map(m_stagingColorBuffer->size());
    m_filter.setImage("color", mappedStaginColorBuffer, oidn::Format::Float3, width, height, 0, to_pixel_size(m_input->description().format), 0);
    m_filter.setImage("output", mappedStaginColorBuffer, oidn::Format::Float3, width, height, 0, to_pixel_size(m_output->description().format), 0);
    if (m_stagingAlbedoBuffer.get() && m_stagingNormalBuffer.get()) {
        m_filter.setImage("albedo", m_stagingAlbedoBuffer->map(m_stagingAlbedoBuffer->size()), oidn::Format::Float3, width, height, 0, to_pixel_size(m_albedo->description().format), 0);
        m_filter.setImage("normal", m_stagingNormalBuffer->map(m_stagingNormalBuffer->size()), oidn::Format::Float3, width, height, 0, to_pixel_size(m_normal->description().format), 0);
    }
    m_filter.set("hdr", true); // beauty image is HDR
    m_filter.commit();
}

}