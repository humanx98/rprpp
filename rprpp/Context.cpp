#include "Context.h"
#include "Error.h"

namespace rprpp {

Context::Context(const std::shared_ptr<vk::helper::DeviceContext>& dctx)
    : m_deviceContext(dctx)
{
}

std::unique_ptr<Context> Context::create(uint32_t deviceId)
{
    vk::helper::DeviceContext dctx = vk::helper::createDeviceContext(deviceId);
    return std::make_unique<Context>(std::make_shared<vk::helper::DeviceContext>(std::move(dctx)));
}

PostProcessing* Context::createPostProcessing()
{
    Buffer uboBuffer = Buffer::create(*m_deviceContext,
        sizeof(UniformBufferObject),
        vk::BufferUsageFlagBits::eUniformBuffer,
        vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

    vk::raii::Sampler sampler(m_deviceContext->device, vk::SamplerCreateInfo());

    auto pp = std::make_unique<PostProcessing>(m_deviceContext, std::move(uboBuffer), std::move(sampler));

    PostProcessing* ptr = pp.get();
    m_postProcessings.emplace(ptr, std::move(pp));
    return ptr;
}

void Context::destroyPostProcessing(PostProcessing* pp)
{
    m_postProcessings.erase(pp);
}

Buffer* Context::createBuffer(size_t size)
{
    auto usage = vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eTransferDst;
    auto props = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;
    auto buffer = std::make_unique<Buffer>(Buffer::create(*m_deviceContext, size, usage, props));

    Buffer* ptr = buffer.get();
    m_buffers.emplace(ptr, std::move(buffer));
    return ptr;
}

void Context::destroyBuffer(Buffer* buffer)
{
    m_buffers.erase(buffer);
}

Image* Context::createImage(const ImageDescription& desc)
{
    auto image = std::make_unique<Image>(Image::create(*m_deviceContext, desc));

    Image* ptr = image.get();
    m_images.emplace(ptr, std::move(image));
    return ptr;
}

Image* Context::createFromVkSampledImage(vk::Image vkSampledImage, const ImageDescription& desc)
{
    auto image = std::make_unique<Image>(Image::createFromVkSampledImage(*m_deviceContext, vkSampledImage, desc));

    Image* ptr = image.get();
    m_images.emplace(ptr, std::move(image));
    return ptr;
}

Image* Context::createImageFromDx11Texture(HANDLE dx11textureHandle, const ImageDescription& desc)
{
    if (dx11textureHandle == nullptr) {
        throw InvalidParameter("dx11textureHandle", "Cannot be null");
    }

    auto image = std::make_unique<Image>(Image::createFromDx11Texture(*m_deviceContext, dx11textureHandle, desc));

    Image* ptr = image.get();
    m_images.emplace(ptr, std::move(image));
    return ptr;
}

void Context::destroyImage(Image* image)
{
    m_images.erase(image);
}

void Context::copyBufferToImage(Buffer* buffer, Image* image)
{
    size_t size = image->description().width * image->description().height * to_pixel_size(image->description().format);
    if (buffer->size() < size) {
        throw InvalidParameter("buffer", "The provided buffer doesn't fit destination image");
    }

    vk::AccessFlags oldAccess = image->getAccess();
    vk::ImageLayout oldLayout = image->getLayout();
    vk::PipelineStageFlags oldStage = image->getPipelineStages();

    vk::raii::CommandBuffer commandBuffer = m_deviceContext->takeCommandBuffer();
    commandBuffer.begin(vk::CommandBufferBeginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit));
    Image::transitionImageLayout(commandBuffer, *image,
        vk::AccessFlagBits::eTransferWrite,
        vk::ImageLayout::eTransferDstOptimal,
        vk::PipelineStageFlagBits::eTransfer);
    {
        vk::ImageSubresourceLayers imageSubresource(vk::ImageAspectFlagBits::eColor, 0, 0, 1);
        vk::BufferImageCopy region(0, 0, 0, imageSubresource, { 0, 0, 0 }, { image->description().width, image->description().height, 1 });
        commandBuffer.copyBufferToImage(buffer->get(), image->get(), vk::ImageLayout::eTransferDstOptimal, region);
    }
    Image::transitionImageLayout(commandBuffer, *image, oldAccess, oldLayout, oldStage);
    commandBuffer.end();

    vk::SubmitInfo submitInfo(nullptr, nullptr, *commandBuffer);
    m_deviceContext->queue.submit(submitInfo);
    m_deviceContext->queue.waitIdle();
    m_deviceContext->returnCommandBuffer(std::move(commandBuffer));
}

void Context::copyImageToBuffer(Image* image, Buffer* buffer)
{
    size_t size = image->description().width * image->description().height * to_pixel_size(image->description().format);
    if (buffer->size() < size) {
        throw InvalidParameter("buffer", "The provided buffer doesn't fit destination image");
    }

    vk::AccessFlags oldAccess = image->getAccess();
    vk::ImageLayout oldLayout = image->getLayout();
    vk::PipelineStageFlags oldStage = image->getPipelineStages();

    vk::raii::CommandBuffer commandBuffer = m_deviceContext->takeCommandBuffer();
    commandBuffer.begin(vk::CommandBufferBeginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit));
    Image::transitionImageLayout(commandBuffer, *image,
        vk::AccessFlagBits::eTransferRead,
        vk::ImageLayout::eTransferSrcOptimal,
        vk::PipelineStageFlagBits::eTransfer);
    {
        vk::ImageSubresourceLayers imageSubresource(vk::ImageAspectFlagBits::eColor, 0, 0, 1);
        vk::BufferImageCopy region(0, 0, 0, imageSubresource, { 0, 0, 0 }, { image->description().width, image->description().height, 1 });
        commandBuffer.copyImageToBuffer(image->get(), vk::ImageLayout::eTransferSrcOptimal, buffer->get(), region);
    }
    Image::transitionImageLayout(commandBuffer, *image, oldAccess, oldLayout, oldStage);
    commandBuffer.end();

    vk::SubmitInfo submitInfo(nullptr, nullptr, *commandBuffer);
    m_deviceContext->queue.submit(submitInfo);
    m_deviceContext->queue.waitIdle();
    m_deviceContext->returnCommandBuffer(std::move(commandBuffer));
}

void Context::copyImage(Image* src, Image* dst)
{
    if (src->description() != dst->description()) {
        throw InvalidParameter("dst", "Destination image description has to be equal to source description");
    }

    vk::AccessFlags oldDstAccess = dst->getAccess();
    vk::ImageLayout oldDstLayout = dst->getLayout();
    vk::PipelineStageFlags oldDstStage = dst->getPipelineStages();

    vk::AccessFlags oldSrcAccess = src->getAccess();
    vk::ImageLayout oldSrcLayout = src->getLayout();
    vk::PipelineStageFlags oldSrcStage = src->getPipelineStages();

    vk::raii::CommandBuffer commandBuffer = m_deviceContext->takeCommandBuffer();
    commandBuffer.begin(vk::CommandBufferBeginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit));
    Image::transitionImageLayout(commandBuffer,
        *dst,
        vk::AccessFlagBits::eTransferWrite,
        vk::ImageLayout::eTransferDstOptimal,
        vk::PipelineStageFlagBits::eTransfer);
    Image::transitionImageLayout(commandBuffer,
        *src,
        vk::AccessFlagBits::eTransferRead,
        vk::ImageLayout::eTransferSrcOptimal,
        vk::PipelineStageFlagBits::eTransfer);
    {
        vk::ImageSubresourceLayers imageSubresource(vk::ImageAspectFlagBits::eColor, 0, 0, 1);
        vk::ImageCopy region(imageSubresource, { 0, 0, 0 }, imageSubresource, { 0, 0, 0 }, { src->description().width, src->description().height, 1 });
        commandBuffer.copyImage(src->get(), vk::ImageLayout::eTransferSrcOptimal, dst->get(), vk::ImageLayout::eTransferDstOptimal, region);
    }
    Image::transitionImageLayout(commandBuffer, *src, oldSrcAccess, oldSrcLayout, oldSrcStage);
    Image::transitionImageLayout(commandBuffer, *dst, oldDstAccess, oldDstLayout, oldDstStage);
    commandBuffer.end();

    vk::SubmitInfo submitInfo(nullptr, nullptr, *commandBuffer);
    m_deviceContext->queue.submit(submitInfo);
    m_deviceContext->queue.waitIdle();
    m_deviceContext->returnCommandBuffer(std::move(commandBuffer));
}

VkPhysicalDevice Context::getVkPhysicalDevice() const noexcept
{
    return static_cast<VkPhysicalDevice>(*m_deviceContext->physicalDevice);
}

VkDevice Context::getVkDevice() const noexcept
{
    return static_cast<VkDevice>(*m_deviceContext->device);
}

VkQueue Context::getVkQueue() const noexcept
{
    return static_cast<VkQueue>(*m_deviceContext->queue);
}

void Context::waitQueueIdle()
{
    m_deviceContext->queue.waitIdle();
}

}