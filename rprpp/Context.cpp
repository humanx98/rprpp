#include "Context.h"
#include "Error.h"
#include "filters/BloomFilter.h"
#include "filters/ComposeColorShadowReflectionFilter.h"
#include "filters/ComposeOpacityShadowFilter.h"
#include "filters/ToneMapFilter.h"

namespace rprpp {

Context::Context(uint32_t deviceId)
 : m_deviceContext(vk::helper::createDeviceContext(deviceId))
{
}

filters::BloomFilter* Context::createBloomFilter()
{
    return m_objects.emplaceCastReturn<filters::BloomFilter>(this);

    /* auto iter = m_childrens.emplace(
        std::piecewise_construct,
        std::forward_as_tuple(m_tagGenerator()),
        std::forward_as_tuple(std::in_place_type<filters::BloomFilter>, &m_deviceContext)
    );

    return &(std::get<filters::BloomFilter>(iter.first->second));*/

    //return nullptr;

 }

filters::ComposeColorShadowReflectionFilter* Context::createComposeColorShadowReflectionFilter()
 {
     /* auto params = UniformObjectBuffer<filters::ComposeColorShadowReflectionParams>(this);
     vk::SamplerCreateInfo samplerInfo;
     samplerInfo.unnormalizedCoordinates = vk::True;
     samplerInfo.addressModeU = vk::SamplerAddressMode::eClampToEdge;
     samplerInfo.addressModeV = vk::SamplerAddressMode::eClampToEdge;*/

      return m_objects.emplaceCastReturn<filters::ComposeColorShadowReflectionFilter>(this);

     /* auto filter = std::make_unique<filters::ComposeColorShadowReflectionFilter>(&m_deviceContext, std::move(params), vk::raii::Sampler(m_deviceContext.device, samplerInfo));

     filters::ComposeColorShadowReflectionFilter* ptr = filter.get();
     m_filters.emplace(ptr, std::move(filter));
     return ptr;*/

    return nullptr;
 }

 filters::ComposeOpacityShadowFilter* Context::createComposeOpacityShadowFilter()
{
    //auto iter = m_objects.emplace(filters::ComposeOpacityShadowFilter(this));
    //return static_cast<filters::ComposeOpacityShadowFilter*>(iter.first->get());

    /* auto params = UniformObjectBuffer<filters::ComposeOpacityShadowParams>(this);
    return nullptr;*/

    /*auto filter = std::make_unique<filters::ComposeOpacityShadowFilter>(&m_deviceContext, std::move(params), vk::raii::Sampler(m_deviceContext.device, vk::SamplerCreateInfo()));

    filters::ComposeOpacityShadowFilter* ptr = filter.get();
    m_filters.emplace(ptr, std::move(filter));
    return ptr;*/

    return m_objects.emplaceCastReturn<filters::ComposeOpacityShadowFilter>(this);

}

filters::DenoiserFilter* Context::createDenoiserFilter()
{
    return m_objects.emplaceCastReturn<filters::DenoiserFilter>(this);
    //auto iter = m_objects.emplace(filters::DenoiserFilter(this));
    //return static_cast<filters::DenoiserFilter*>(iter.first->get());

    /* auto filter = std::make_unique<filters::DenoiserFilter>(&m_deviceContext);

    filters::DenoiserFilter* ptr = filter.get();
    m_filters.emplace(ptr, std::move(filter));
    return ptr;*/
}

filters::ToneMapFilter* Context::createToneMapFilter()
{
    return m_objects.emplaceCastReturn<filters::ToneMapFilter>(this);
    //auto iter = m_objects.emplace(filters::ToneMapFilter(this));
    //return static_cast<filters::ToneMapFilter*>(iter.first->get());

    /* auto filter = std::make_unique<filters::ToneMapFilter>(&m_deviceContext, std::move(params));

    filters::ToneMapFilter* ptr = filter.get();
    m_filters.emplace(ptr, std::move(filter));
    return ptr;*/

    return nullptr;
}

void Context::destroyFilter(filters::Filter* filter)
{
    m_objects.erase(filter);
    //m_filters.erase(filter);
}

Buffer* Context::createBuffer(size_t size)
{
    auto usage = vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eTransferDst;
    auto props = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;

    return m_objects.emplaceCastReturn<Buffer>(this, size, usage, props);

    /* auto iter = m_objects.emplace(std::make_unique<Buffer>(this, size, usage, props));
    return static_cast<Buffer*>(iter.first->get());
    */

    //return nullptr;

    /* auto buffer = std::make_unique<Buffer>(Buffer::create(m_deviceContext, size, usage, props));

    Buffer* ptr = buffer.get();
    m_buffers.emplace(ptr, std::move(buffer));
    return ptr;*/
}

void Context::destroyBuffer(Buffer* buffer)
{
    m_objects.erase(buffer);
    //m_buffers.erase(buffer);
}

Image* Context::createImage(const ImageDescription& desc)
{
    return m_objects.emplaceCastReturn<Image>(Image::create(m_deviceContext, desc));

    /* auto image = std::make_unique<Image>(Image::create(m_deviceContext, desc));

    Image* ptr = image.get();
    m_images.emplace(ptr, std::move(image));
    return ptr;*/

    return nullptr;
}

Image* Context::createFromVkSampledImage(vk::Image vkSampledImage, const ImageDescription& desc)
{
/*
    auto image = std::make_unique<Image>(Image::createFromVkSampledImage(m_deviceContext, vkSampledImage, desc));

    Image* ptr = image.get();
    m_images.emplace(ptr, std::move(image));
    return ptr;*/

    return nullptr;
}

Image* Context::createImageFromDx11Texture(HANDLE dx11textureHandle, const ImageDescription& desc)
{
    assert(dx11textureHandle);

    /* auto image = std::make_unique<Image>(Image::createFromDx11Texture(m_deviceContext, dx11textureHandle, desc));

    Image* ptr = image.get();
    m_images.emplace(ptr, std::move(image));
    return ptr;*/

    return nullptr;
}

void Context::destroyImage(Image* image)
{
    //m_images.erase(image);
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

    vk::helper::CommandBuffer commandBuffer(&m_deviceContext);
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
    m_deviceContext.queue.submit(submitInfo);
    m_deviceContext.queue.waitIdle();
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

    vk::helper::CommandBuffer commandBuffer(&m_deviceContext);
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
    m_deviceContext.queue.submit(submitInfo);
    m_deviceContext.queue.waitIdle();
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

    vk::helper::CommandBuffer commandBuffer(&m_deviceContext);
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
    m_deviceContext.queue.submit(submitInfo);
    m_deviceContext.queue.waitIdle();
}

VkPhysicalDevice Context::getVkPhysicalDevice() const noexcept
{
    return *m_deviceContext.physicalDevice;
}

VkDevice Context::getVkDevice() const noexcept
{
    return *m_deviceContext.device;
}

VkQueue Context::getVkQueue() const noexcept
{
    return *m_deviceContext.queue;
}

void Context::waitQueueIdle()
{
    m_deviceContext.queue.waitIdle();
}

}