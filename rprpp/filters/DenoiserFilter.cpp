#include "DenoiserFilter.h"
#include "rprpp/Error.h"
#include <cassert>

#include <boost/log/trivial.hpp>

namespace rprpp::filters {

DenoiserFilter::DenoiserFilter(Context* context, oidn::DeviceRef& device)
    : Filter(context)
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

    if (m_input->description() != m_output->description()) {
        throw InvalidParameter("output and input", "output and input should have the same image description");
    }

    if (is_ldr(m_input->description().format) || is_ldr(m_output->description().format)) {
        throw InvalidParameter("output and input", "output and input cannot be in low dynamic range");
    }

    if (m_albedo && m_normal) {
        if (is_ldr(m_albedo->description().format) || is_ldr(m_normal->description().format)) {
            throw InvalidParameter("albedo and normal", "albedo and normal cannot be in low dynamic range");
        }
    }
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
    m_input = image;
    m_dirty = true;
}

void DenoiserFilter::setAovAlbedo(Image* image)
{
    m_albedo = image;
    m_dirty = true;
}

void DenoiserFilter::setAovNormal(Image* image)
{
    m_normal = image;
    m_dirty = true;
}

void DenoiserFilter::setOutput(Image* image)
{
    m_output = image;
    m_dirty = true;
}

}