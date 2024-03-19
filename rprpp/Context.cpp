#include "Context.h"
#include "Error.h"
#include "filters/BloomFilter.h"
#include "filters/ComposeColorShadowReflectionFilter.h"
#include "filters/ComposeOpacityShadowFilter.h"
#include "filters/ToneMapFilter.h"

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

filters::BloomFilter* Context::createBloomFilter()
{
    auto params = UniformObjectBuffer<filters::BloomParams>::create(*m_deviceContext);
    auto filter = std::make_unique<filters::BloomFilter>(m_deviceContext, std::move(params));

    filters::BloomFilter* ptr = filter.get();
    m_filters.emplace(ptr, std::move(filter));
    return ptr;
}

filters::ComposeColorShadowReflectionFilter* Context::createComposeColorShadowReflectionFilter()
{
    auto params = UniformObjectBuffer<filters::ComposeColorShadowReflectionParams>::create(*m_deviceContext);
    vk::SamplerCreateInfo samplerInfo;
    samplerInfo.unnormalizedCoordinates = vk::True;
    samplerInfo.addressModeU = vk::SamplerAddressMode::eClampToEdge;
    samplerInfo.addressModeV = vk::SamplerAddressMode::eClampToEdge;
    auto filter = std::make_unique<filters::ComposeColorShadowReflectionFilter>(m_deviceContext, std::move(params), vk::raii::Sampler(m_deviceContext->device, samplerInfo));

    filters::ComposeColorShadowReflectionFilter* ptr = filter.get();
    m_filters.emplace(ptr, std::move(filter));
    return ptr;
}

filters::ComposeOpacityShadowFilter* Context::createComposeOpacityShadowFilter()
{
    auto params = UniformObjectBuffer<filters::ComposeOpacityShadowParams>::create(*m_deviceContext);
    auto filter = std::make_unique<filters::ComposeOpacityShadowFilter>(m_deviceContext, std::move(params), vk::raii::Sampler(m_deviceContext->device, vk::SamplerCreateInfo()));

    filters::ComposeOpacityShadowFilter* ptr = filter.get();
    m_filters.emplace(ptr, std::move(filter));
    return ptr;
}

filters::DenoiserFilter* Context::createDenoiserFilter()
{
    auto filter = std::make_unique<filters::DenoiserFilter>(m_deviceContext);

    filters::DenoiserFilter* ptr = filter.get();
    m_filters.emplace(ptr, std::move(filter));
    return ptr;
}

filters::ToneMapFilter* Context::createToneMapFilter()
{
    auto params = UniformObjectBuffer<filters::ToneMapParams>::create(*m_deviceContext);
    auto filter = std::make_unique<filters::ToneMapFilter>(m_deviceContext, std::move(params));

    filters::ToneMapFilter* ptr = filter.get();
    m_filters.emplace(ptr, std::move(filter));
    return ptr;
}

void Context::destroyFilter(filters::Filter* filter)
{
    m_filters.erase(filter);
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

    vk::helper::CommandBuffer commandBuffer(m_deviceContext);
    commandBuffer.get().begin(vk::CommandBufferBeginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit));
    Image::transitionImageLayout(commandBuffer.get(), *image,
        vk::AccessFlagBits::eTransferWrite,
        vk::ImageLayout::eTransferDstOptimal,
        vk::PipelineStageFlagBits::eTransfer);
    {
        vk::ImageSubresourceLayers imageSubresource(vk::ImageAspectFlagBits::eColor, 0, 0, 1);
        vk::BufferImageCopy region(0, 0, 0, imageSubresource, { 0, 0, 0 }, { image->description().width, image->description().height, 1 });
        commandBuffer.get().copyBufferToImage(buffer->get(), image->get(), vk::ImageLayout::eTransferDstOptimal, region);
    }
    Image::transitionImageLayout(commandBuffer.get(), *image, oldAccess, oldLayout, oldStage);
    commandBuffer.get().end();

    vk::SubmitInfo submitInfo(nullptr, nullptr, *commandBuffer.get());
    m_deviceContext->queue.submit(submitInfo);
    m_deviceContext->queue.waitIdle();
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

    vk::helper::CommandBuffer commandBuffer(m_deviceContext);
    commandBuffer.get().begin(vk::CommandBufferBeginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit));
    Image::transitionImageLayout(commandBuffer.get(), *image,
        vk::AccessFlagBits::eTransferRead,
        vk::ImageLayout::eTransferSrcOptimal,
        vk::PipelineStageFlagBits::eTransfer);
    {
        vk::ImageSubresourceLayers imageSubresource(vk::ImageAspectFlagBits::eColor, 0, 0, 1);
        vk::BufferImageCopy region(0, 0, 0, imageSubresource, { 0, 0, 0 }, { image->description().width, image->description().height, 1 });
        commandBuffer.get().copyImageToBuffer(image->get(), vk::ImageLayout::eTransferSrcOptimal, buffer->get(), region);
    }
    Image::transitionImageLayout(commandBuffer.get(), *image, oldAccess, oldLayout, oldStage);
    commandBuffer.get().end();

    vk::SubmitInfo submitInfo(nullptr, nullptr, *commandBuffer.get());
    m_deviceContext->queue.submit(submitInfo);
    m_deviceContext->queue.waitIdle();
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

    vk::helper::CommandBuffer commandBuffer(m_deviceContext);
    commandBuffer.get().begin(vk::CommandBufferBeginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit));
    Image::transitionImageLayout(commandBuffer.get(),
        *dst,
        vk::AccessFlagBits::eTransferWrite,
        vk::ImageLayout::eTransferDstOptimal,
        vk::PipelineStageFlagBits::eTransfer);
    Image::transitionImageLayout(commandBuffer.get(),
        *src,
        vk::AccessFlagBits::eTransferRead,
        vk::ImageLayout::eTransferSrcOptimal,
        vk::PipelineStageFlagBits::eTransfer);
    {
        vk::ImageSubresourceLayers imageSubresource(vk::ImageAspectFlagBits::eColor, 0, 0, 1);
        vk::ImageCopy region(imageSubresource, { 0, 0, 0 }, imageSubresource, { 0, 0, 0 }, { src->description().width, src->description().height, 1 });
        commandBuffer.get().copyImage(src->get(), vk::ImageLayout::eTransferSrcOptimal, dst->get(), vk::ImageLayout::eTransferDstOptimal, region);
    }
    Image::transitionImageLayout(commandBuffer.get(), *src, oldSrcAccess, oldSrcLayout, oldSrcStage);
    Image::transitionImageLayout(commandBuffer.get(), *dst, oldDstAccess, oldDstLayout, oldDstStage);
    commandBuffer.get().end();

    vk::SubmitInfo submitInfo(nullptr, nullptr, *commandBuffer.get());
    m_deviceContext->queue.submit(submitInfo);
    m_deviceContext->queue.waitIdle();
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