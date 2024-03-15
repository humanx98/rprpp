#include "Image.h"
#include "vk/CommandBuffer.h"

namespace rprpp {

ImageDescription::ImageDescription(uint32_t w, uint32_t h, ImageFormat f)
    : width(w)
    , height(h)
    , format(f)
{
}

ImageDescription::ImageDescription(const RprPpImageDescription& desc)
    : width(desc.width)
    , height(desc.height)
    , format(static_cast<rprpp::ImageFormat>(desc.format))
{
}

Image::Image(vk::raii::Image&& image, vk::raii::DeviceMemory&& memory, vk::raii::ImageView&& view, vk::ImageUsageFlags usage, const ImageDescription& desc) noexcept
    : m_image(std::move(image))
    , m_memory(std::move(memory))
    , m_view(std::move(view))
    , m_usage(usage)
    , m_description(desc)
{
}

Image::Image(vk::Image image, vk::raii::ImageView&& view, const ImageDescription& desc) noexcept
    : m_notOwnedImage(image)
    , m_view(std::move(view))
    , m_description(desc)
    , m_usage(vk::ImageUsageFlagBits::eSampled)
    , m_layout(vk::ImageLayout::eShaderReadOnlyOptimal)
    , m_access(vk::AccessFlagBits::eShaderRead)
    , m_stages(vk::PipelineStageFlagBits::eComputeShader)
{
}

Image Image::create(vk::helper::DeviceContext& dctx, const ImageDescription& desc)
{
    vk::ImageUsageFlags usage = vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eStorage;
    vk::ImageCreateInfo imageInfo({},
        vk::ImageType::e2D,
        to_vk_format(desc.format),
        { desc.width, desc.height, 1 },
        1,
        1,
        vk::SampleCountFlagBits::e1,
        vk::ImageTiling::eOptimal,
        usage,
        vk::SharingMode::eExclusive,
        nullptr,
        vk::ImageLayout::eUndefined);
    vk::raii::Image image(dctx.device, imageInfo);

    vk::MemoryRequirements memRequirements = image.getMemoryRequirements();
    uint32_t memoryType = vk::helper::findMemoryType(dctx.physicalDevice, memRequirements.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal);
    vk::raii::DeviceMemory memory = dctx.device.allocateMemory(vk::MemoryAllocateInfo(memRequirements.size, memoryType));
    image.bindMemory(*memory, 0);

    vk::ImageViewCreateInfo viewInfo({},
        *image,
        vk::ImageViewType::e2D,
        to_vk_format(desc.format),
        {},
        { vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 });
    vk::raii::ImageView view(dctx.device, viewInfo);

    Image result(std::move(image), std::move(memory), std::move(view), usage, desc);
    vk::AccessFlags access = vk::AccessFlagBits::eShaderRead | vk::AccessFlagBits::eShaderWrite;
    transitionImageLayout(dctx, result, access, vk::ImageLayout::eGeneral, vk::PipelineStageFlagBits::eComputeShader);
    return result;
}

Image Image::createFromVkSampledImage(const vk::helper::DeviceContext& dctx, vk::Image image, const ImageDescription& desc)
{
    vk::ImageViewCreateInfo viewInfo({},
        image,
        vk::ImageViewType::e2D,
        to_vk_format(desc.format),
        {},
        { vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 });
    vk::raii::ImageView view(dctx.device, viewInfo);

    return Image(image, std::move(view), desc);
}

Image Image::createFromDx11Texture(vk::helper::DeviceContext& dctx, HANDLE dx11textureHandle, const ImageDescription& desc)
{
    vk::ImageUsageFlags usage = vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eStorage;
    vk::ExternalMemoryImageCreateInfo externalMemoryInfo = vk::ExternalMemoryImageCreateInfo(vk::ExternalMemoryHandleTypeFlagBits::eD3D11Texture);

    vk::ImageCreateInfo imageInfo({},
        vk::ImageType::e2D,
        to_vk_format(desc.format),
        { desc.width, desc.height, 1 },
        1,
        1,
        vk::SampleCountFlagBits::e1,
        vk::ImageTiling::eOptimal,
        usage,
        vk::SharingMode::eExclusive,
        nullptr,
        vk::ImageLayout::eUndefined,
        &externalMemoryInfo);
    vk::raii::Image image(dctx.device, imageInfo);

    vk::MemoryDedicatedRequirements memoryDedicatedRequirements;
    vk::MemoryRequirements2 memoryRequirements2({}, &memoryDedicatedRequirements);
    vk::ImageMemoryRequirementsInfo2 imageMemoryRequirementsInfo2(*image);
    (*dctx.device).getImageMemoryRequirements2(&imageMemoryRequirementsInfo2, &memoryRequirements2);
    vk::MemoryDedicatedAllocateInfo memoryDedicatedAllocateInfo(*image);
    vk::ImportMemoryWin32HandleInfoKHR importMemoryInfo(vk::ExternalMemoryHandleTypeFlagBits::eD3D11Texture,
        dx11textureHandle,
        nullptr,
        &memoryDedicatedAllocateInfo);
    uint32_t memoryType = vk::helper::findMemoryType(dctx.physicalDevice, memoryRequirements2.memoryRequirements.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal);
    vk::MemoryAllocateInfo memoryAllocateInfo(memoryRequirements2.memoryRequirements.size,
        memoryType,
        &importMemoryInfo);
    vk::raii::DeviceMemory memory = dctx.device.allocateMemory(memoryAllocateInfo);

    image.bindMemory(*memory, 0);

    vk::ImageViewCreateInfo viewInfo({},
        *image,
        vk::ImageViewType::e2D,
        to_vk_format(desc.format),
        {},
        { vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 });
    vk::raii::ImageView view(dctx.device, viewInfo);

    Image result(std::move(image), std::move(memory), std::move(view), usage, desc);
    vk::AccessFlags access = vk::AccessFlagBits::eShaderRead | vk::AccessFlagBits::eShaderWrite;
    transitionImageLayout(dctx, result, access, vk::ImageLayout::eGeneral, vk::PipelineStageFlagBits::eComputeShader);
    return result;
}

void Image::transitionImageLayout(vk::helper::DeviceContext& deviceContext,
    Image& image,
    vk::AccessFlags dstAccess,
    vk::ImageLayout dstLayout,
    vk::PipelineStageFlags dstStage)
{
    vk::raii::CommandBuffer commandBuffer = deviceContext.takeCommandBuffer();
    commandBuffer.begin(vk::CommandBufferBeginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit));
    transitionImageLayout(commandBuffer, image, dstAccess, dstLayout, dstStage);
    commandBuffer.end();

    vk::SubmitInfo submitInfo(nullptr, nullptr, *commandBuffer);
    deviceContext.queue.submit(submitInfo);
    deviceContext.queue.waitIdle();
    deviceContext.returnCommandBuffer(std::move(commandBuffer));
}

void Image::transitionImageLayout(const vk::raii::CommandBuffer& commandBuffer,
    Image& image,
    vk::AccessFlags dstAccess,
    vk::ImageLayout dstLayout,
    vk::PipelineStageFlags dstStage)
{
    vk::ImageSubresourceRange subresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1);
    vk::ImageMemoryBarrier imageMemoryBarrier(image.getAccess(),
        dstAccess,
        image.getLayout(),
        dstLayout,
        VK_QUEUE_FAMILY_IGNORED,
        VK_QUEUE_FAMILY_IGNORED,
        image.get(),
        subresourceRange);

    commandBuffer.pipelineBarrier(image.getPipelineStages(),
        dstStage,
        {},
        nullptr,
        nullptr,
        imageMemoryBarrier);

    image.setAccess(dstAccess);
    image.setLayout(dstLayout);
    image.setPipelineStages(dstStage);
}

const ImageDescription& Image::description() const noexcept
{
    return m_description;
}

vk::ImageUsageFlags Image::usage() const noexcept
{
    return m_usage;
}

bool Image::IsStorage() const noexcept
{
    return (m_usage & vk::ImageUsageFlagBits::eStorage) == vk::ImageUsageFlagBits::eStorage;
}

bool Image::IsSampled() const noexcept
{
    return (m_usage & vk::ImageUsageFlagBits::eSampled) == vk::ImageUsageFlagBits::eSampled;
}

const vk::Image Image::get() const noexcept
{
    return m_image.has_value() ? *m_image.value() : m_notOwnedImage.value();
}

const vk::ImageView Image::view() const noexcept
{
    return *m_view;
}

vk::AccessFlags Image::getAccess() const noexcept
{
    return m_access;
}

void Image::setAccess(vk::AccessFlags access) noexcept
{
    m_access = access;
}

vk::ImageLayout Image::getLayout() const noexcept
{
    return m_layout;
}

void Image::setLayout(vk::ImageLayout layout) noexcept
{
    m_layout = layout;
}

vk::PipelineStageFlags Image::getPipelineStages() const noexcept
{
    return m_stages;
}

void Image::setPipelineStages(vk::PipelineStageFlags stages) noexcept
{
    m_stages = stages;
}

}