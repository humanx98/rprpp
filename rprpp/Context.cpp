#include "Context.h"
#include "Error.h"

namespace rprpp {

Context::Context(vk::helper::DeviceContext dctx)
    : m_deviceContext(std::make_shared<vk::helper::DeviceContext>(std::move(dctx)))
{
}

std::unique_ptr<Context> Context::create(uint32_t deviceId)
{
    return std::make_unique<Context>(vk::helper::createDeviceContext(deviceId));
}

PostProcessing* Context::createPostProcessing()
{
    vk::CommandPoolCreateInfo cmdPoolInfo(vk::CommandPoolCreateFlagBits::eResetCommandBuffer, m_deviceContext->queueFamilyIndex);
    vk::raii::CommandPool commandPool = vk::raii::CommandPool(m_deviceContext->device, cmdPoolInfo);

    vk::CommandBufferAllocateInfo allocInfo(*commandPool, vk::CommandBufferLevel::ePrimary, 2);
    vk::raii::CommandBuffers commandBuffers(m_deviceContext->device, allocInfo);
    assert(commandBuffers.size() >= 2);

    vk::helper::Buffer uboBuffer = vk::helper::createBuffer(*m_deviceContext,
        sizeof(UniformBufferObject),
        vk::BufferUsageFlagBits::eUniformBuffer,
        vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

    auto pp = std::make_unique<PostProcessing>(
        m_deviceContext,
        std::move(commandPool),
        CommandBuffers {
            .compute = std::move(commandBuffers[0]),
            .secondary = std::move(commandBuffers[1]),
        },
        std::move(uboBuffer));

    PostProcessing* ptr = pp.get();
    m_postProcessings.emplace(ptr, std::move(pp));
    return ptr;
}

void Context::destroyPostProcessing(PostProcessing* pp)
{
    m_postProcessings.erase(pp);
}

HostVisibleBuffer* Context::createHostVisibleBuffer(size_t size)
{
    auto usage = vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eTransferDst;
    auto props = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;
    auto buffer = std::make_unique<HostVisibleBuffer>(
        vk::helper::createBuffer(*m_deviceContext, size, usage, props),
        size);

    HostVisibleBuffer* ptr = buffer.get();
    m_hostVisibleBuffers.emplace(ptr, std::move(buffer));
    return ptr;
}

void Context::destroyHostVisibleBuffer(HostVisibleBuffer* buffer)
{
    m_hostVisibleBuffers.erase(buffer);
}

Image* Context::createImageFromDx11Texture(HANDLE dx11textureHandle, const ImageDescription& desc)
{
    if (dx11textureHandle == nullptr) {
        throw InvalidParameter("dx11textureHandle", "Cannot be null");
    }

    // we set vk::ImageUsageFlagBits::eStorage in order to remove warnings in validation layer
    auto image = std::make_unique<Image>(
        vk::helper::createImageFromDx11Texture(*m_deviceContext,
            dx11textureHandle,
            desc.width,
            desc.height,
            to_vk_format(desc.format),
            vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eStorage),
        desc);

    Image* ptr = image.get();
    m_images.emplace(ptr, std::move(image));
    return ptr;
}

void Context::destroyImage(Image* image)
{
    m_images.erase(image);
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

}