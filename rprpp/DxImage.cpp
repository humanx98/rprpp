#include "DxImage.h"
#include "Context.h"

namespace rprpp {

DxImage::DxImage(Context* context, const ImageDescription& desc, HANDLE dx11textureHandle)
    : Image(context)
{
    const ImageDescription& imageDescription = desc;
    vk::ImageUsageFlags usage = vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eStorage;
    vk::AccessFlags access = vk::AccessFlagBits::eNone;
    vk::ImageLayout layout = vk::ImageLayout::eUndefined;
    vk::PipelineStageFlags stages = vk::PipelineStageFlagBits::eTopOfPipe;

    vk::raii::Image image = createImage(context, imageDescription, usage);
    vk::raii::DeviceMemory memory = allocateDeviceMemory(context, &image, dx11textureHandle);
    vk::raii::ImageView view = createImageView(context, &image, imageDescription);

    m_imageDataPtr = std::make_unique<ImageData>(
        context,
        std::move(image),
        imageDescription,
        std::move(memory),
        std::move(view),
        usage,
        access,
        layout,
        stages);

    transitionImageLayout(
        vk::AccessFlagBits::eShaderRead | vk::AccessFlagBits::eShaderWrite,
        vk::ImageLayout::eGeneral,
        vk::PipelineStageFlagBits::eComputeShader);
}

vk::raii::Image DxImage::createImage(Context* context, const ImageDescription& desc, vk::ImageUsageFlags imageUsageFlags)
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
        imageUsageFlags,
        vk::SharingMode::eExclusive,
        nullptr,
        vk::ImageLayout::eUndefined,
        &externalMemoryInfo);
    return vk::raii::Image(context->deviceContext().device, imageInfo);
}

vk::raii::DeviceMemory DxImage::allocateDeviceMemory(Context* context, vk::raii::Image* image, HANDLE dx11textureHandle)
{
    vk::MemoryDedicatedRequirements memoryDedicatedRequirements;
    vk::MemoryRequirements2 memoryRequirements2({}, &memoryDedicatedRequirements);
    vk::ImageMemoryRequirementsInfo2 imageMemoryRequirementsInfo2(*(*image));
    (*context->deviceContext().device).getImageMemoryRequirements2(&imageMemoryRequirementsInfo2, &memoryRequirements2);
    vk::MemoryDedicatedAllocateInfo memoryDedicatedAllocateInfo(*(*image));
    vk::ImportMemoryWin32HandleInfoKHR importMemoryInfo(vk::ExternalMemoryHandleTypeFlagBits::eD3D11Texture,
        dx11textureHandle,
        nullptr,
        &memoryDedicatedAllocateInfo);
    uint32_t memoryType = vk::helper::findMemoryType(context->deviceContext().physicalDevice, memoryRequirements2.memoryRequirements.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal);
    vk::MemoryAllocateInfo memoryAllocateInfo(memoryRequirements2.memoryRequirements.size,
        memoryType,
        &importMemoryInfo);
    vk::raii::DeviceMemory memory = context->deviceContext().device.allocateMemory(memoryAllocateInfo);
    image->bindMemory(*memory, 0);

    return memory;
}

vk::raii::ImageView DxImage::createImageView(Context* context, vk::raii::Image* image, const ImageDescription& imageDescription)
{
    vk::ImageViewCreateInfo viewInfo({},
        *(*image),
        vk::ImageViewType::e2D,
        to_vk_format(imageDescription.format),
        {},
        { vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 });
    vk::raii::ImageView view(context->deviceContext().device, viewInfo);

    return view;
}

bool DxImage::IsStorage() const
{
    assert(m_imageDataPtr);
    return m_imageDataPtr->IsStorage();
}

bool DxImage::IsSampled() const
{
    assert(m_imageDataPtr);
    return m_imageDataPtr->IsSampled();
}

const ImageDescription& DxImage::description() const
{
    assert(m_imageDataPtr);
    return m_imageDataPtr->description();
}

const vk::raii::ImageView& DxImage::view() const
{
    assert(m_imageDataPtr);
    return m_imageDataPtr->view();
}

const vk::ImageLayout& DxImage::layout() const
{
    assert(m_imageDataPtr);
    return m_imageDataPtr->layout();
}

const vk::PipelineStageFlags& DxImage::stages() const
{
    assert(m_imageDataPtr);
    return m_imageDataPtr->stages();
}

const vk::AccessFlags& DxImage::access() const
{
    assert(m_imageDataPtr);
    return m_imageDataPtr->access();
}

const vk::Image& DxImage::image() const
{
    assert(m_imageDataPtr);
    return m_imageDataPtr->image();
}

void DxImage::updateLayout(vk::ImageLayout newLayout)
{
    assert(m_imageDataPtr);
    m_imageDataPtr->updateLayout(newLayout);
}

void DxImage::updateStages(vk::PipelineStageFlags newPipelineStageFlags)
{
    assert(m_imageDataPtr);
    m_imageDataPtr->updateStages(newPipelineStageFlags);
}

void DxImage::updateAccess(vk::AccessFlags newFlags)
{
    assert(m_imageDataPtr);
    m_imageDataPtr->updateAccess(newFlags);
}

}