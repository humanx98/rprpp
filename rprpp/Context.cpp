#include "Context.h"
#include "Error.h"
#include "filters/BloomFilter.h"
#include "filters/ComposeColorShadowReflectionFilter.h"
#include "filters/ComposeOpacityShadowFilter.h"
#include "filters/DenoiserCpuFilter.h"
#include "filters/DenoiserGpuFilter.h"
#include "filters/ToneMapFilter.h"
#include "DxImage.h"
#include "VkSampledImage.h"
#include "ImageSimple.h"

#include <boost/log/trivial.hpp>

namespace rprpp {

Context::Context(uint32_t deviceId, uint8_t luid[vk::LuidSize], uint8_t uuid[vk::UuidSize])
    : m_deviceContext(vk::helper::createDeviceContext(deviceId)), m_denoiserDevice(createDenoiserDevice(luid, uuid))
{
}

oidn::DeviceRef Context::createDenoiserDevice(uint8_t luid[vk::LuidSize], uint8_t uuid[vk::UuidSize])
{
    BOOST_LOG_TRIVIAL(trace) << "Context::createDenoiserDevice";

    int numPhysicalDevices = oidn::getNumPhysicalDevices();
    int deviceId = -1;
    int cpuId = -1;
    for (int i = 0; i < numPhysicalDevices; i++) {
        oidn::PhysicalDeviceRef physicalDevice(i);
        if (physicalDevice.get<bool>("luidSupported")) {
            oidn::LUID oidnLUID =  physicalDevice.get<oidn::LUID>("luid");
            if (std::equal(std::begin(oidnLUID.bytes), std::end(oidnLUID.bytes), luid))
            {
                deviceId = i;
                break;
            }
        }
        
        if (physicalDevice.get<bool>("uuidSupported")) {
            oidn::UUID oidnUUID = physicalDevice.get<oidn::UUID>("uuid");
            if (std::equal(std::begin(oidnUUID.bytes), std::end(oidnUUID.bytes), uuid)) {
                deviceId = i;
                break;
            }
        }

        if (physicalDevice.get<oidn::DeviceType>("type") == oidn::DeviceType::CPU) {
            cpuId = i;
        }
    }

    // cpu fallback
    if (deviceId < 0) {
        deviceId = cpuId;
    }
    
    BOOST_LOG_TRIVIAL(info) << "Initialize Selected Denoiser Device id = " << deviceId << ", name = " << oidnGetPhysicalDeviceString(deviceId, "name");
    oidn::DeviceRef device = oidn::newDevice(deviceId);
    device.commit();

    const char* errorMessage;
    if (device.getError(errorMessage) != oidn::Error::None) {
        BOOST_LOG_TRIVIAL(error) << errorMessage;
        throw std::runtime_error(errorMessage);
    }


    return device;
}

filters::BloomFilter* Context::createBloomFilter()
{
    return m_objects.emplaceCastReturn<filters::BloomFilter>(this);
}

filters::ComposeColorShadowReflectionFilter* Context::createComposeColorShadowReflectionFilter()
{
    return m_objects.emplaceCastReturn<filters::ComposeColorShadowReflectionFilter>(this);
}

filters::ComposeOpacityShadowFilter* Context::createComposeOpacityShadowFilter()
{
    return m_objects.emplaceCastReturn<filters::ComposeOpacityShadowFilter>(this);
}

filters::DenoiserFilter* Context::createDenoiserFilter()
{
    auto externalMemoryTypes = m_denoiserDevice.get<oidn::ExternalMemoryTypeFlags>("externalMemoryTypes");
    if ((externalMemoryTypes & oidn::ExternalMemoryTypeFlag::OpaqueWin32) == oidn::ExternalMemoryTypeFlag::OpaqueWin32) {
        return m_objects.emplaceCastReturn<filters::DenoiserGpuFilter>(this, m_denoiserDevice);
    } else {
        return m_objects.emplaceCastReturn<filters::DenoiserCpuFilter>(this, m_denoiserDevice);
    }
}

filters::ToneMapFilter* Context::createToneMapFilter()
{
    return m_objects.emplaceCastReturn<filters::ToneMapFilter>(this);
}

void Context::destroyFilter(filters::Filter* filter)
{
    m_objects.erase(filter);
}

Buffer* Context::createBuffer(size_t size)
{
    auto usage = vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eTransferDst;
    auto props = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;

    return m_objects.emplaceCastReturn<Buffer>(this, size, usage, props);
}

void Context::destroyBuffer(Buffer* buffer)
{
    m_objects.erase(buffer);
}

Image* Context::createImage(const ImageDescription& desc)
{
    return m_objects.emplaceCastReturn<ImageSimple>(this, desc);
}

Image* Context::createFromVkSampledImage(vk::Image vkSampledImage, const ImageDescription& desc)
{
    return m_objects.emplaceCastReturn<VkSampledImage>(this, vkSampledImage, desc);
}

Image* Context::createImageFromDx11Texture(HANDLE dx11textureHandle, const ImageDescription& desc)
{
    assert(dx11textureHandle);
    return m_objects.emplaceCastReturn<DxImage>(this, desc, dx11textureHandle);
}

void Context::destroyImage(Image* image)
{
    m_objects.erase(image);
}

void Context::copyBufferToImage(Buffer* buffer, Image* image)
{
    size_t size = image->description().width * image->description().height * to_pixel_size(image->description().format);
    if (buffer->size() < size) {
        throw InvalidParameter("buffer", "The provided buffer doesn't fit destination image");
    }

    vk::AccessFlags oldAccess = image->access();
    vk::ImageLayout oldLayout = image->layout();
    vk::PipelineStageFlags oldStage = image->stages();

    vk::helper::CommandBuffer commandBuffer(&m_deviceContext);
    commandBuffer.get().begin(vk::CommandBufferBeginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit));
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

    vk::AccessFlags oldAccess = image->access();
    vk::ImageLayout oldLayout = image->layout();
    vk::PipelineStageFlags oldStage = image->stages();

    vk::helper::CommandBuffer commandBuffer(&m_deviceContext);
    commandBuffer.get().begin(vk::CommandBufferBeginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit));
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

    vk::AccessFlags oldDstAccess = dst->access();
    vk::ImageLayout oldDstLayout = dst->layout();
    vk::PipelineStageFlags oldDstStage = dst->stages();

    vk::AccessFlags oldSrcAccess = src->access();
    vk::ImageLayout oldSrcLayout = src->layout();
    vk::PipelineStageFlags oldSrcStage = src->stages();

    vk::helper::CommandBuffer commandBuffer(&m_deviceContext);
    commandBuffer.get().begin(vk::CommandBufferBeginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit));
    dst->transitionImageLayout(commandBuffer.get(),
        vk::AccessFlagBits::eTransferWrite,
        vk::ImageLayout::eTransferDstOptimal,
        vk::PipelineStageFlagBits::eTransfer);
    src->transitionImageLayout(commandBuffer.get(),
        vk::AccessFlagBits::eTransferRead,
        vk::ImageLayout::eTransferSrcOptimal,
        vk::PipelineStageFlagBits::eTransfer);
    {
        vk::ImageSubresourceLayers imageSubresource(vk::ImageAspectFlagBits::eColor, 0, 0, 1);
        vk::ImageCopy region(imageSubresource, { 0, 0, 0 }, imageSubresource, { 0, 0, 0 }, { src->description().width, src->description().height, 1 });
        commandBuffer.get().copyImage(src->image(), vk::ImageLayout::eTransferSrcOptimal, dst->image(), vk::ImageLayout::eTransferDstOptimal, region);
    }
    src->transitionImageLayout(commandBuffer.get(), oldSrcAccess, oldSrcLayout, oldSrcStage);
    dst->transitionImageLayout(commandBuffer.get(), oldDstAccess, oldDstLayout, oldDstStage);
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