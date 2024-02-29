#include "Image.h"

namespace rprpp {

Image::Image(vk::raii::Image&& image, vk::raii::DeviceMemory&& memory, vk::raii::ImageView&& view, const ImageDescription& desc) noexcept
    : m_image(std::move(image))
    , m_memory(std::move(memory))
    , m_view(std::move(view))
    , m_description(desc)
{
}

Image Image::create(const vk::helper::DeviceContext& dctx, const ImageDescription& desc, vk::ImageUsageFlags usage)
{
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

    return Image(std::move(image), std::move(memory), std::move(view), desc);
}

Image Image::createFromDx11Texture(const vk::helper::DeviceContext& dctx, HANDLE dx11textureHandle, const ImageDescription& desc, vk::ImageUsageFlags usage)
{
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

    return Image(std::move(image), std::move(memory), std::move(view), desc);
}

const ImageDescription& Image::description() const noexcept
{
    return m_description;
}

const vk::raii::Image& Image::get() const noexcept
{
    return m_image;
}

const vk::raii::ImageView& Image::view() const noexcept
{
    return m_view;
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