#include "ImageSimple.h"
#include "Context.h"

namespace rprpp {

vk::raii::Image ImageSimple::createImage(
    Context* context,
    const ImageDescription& desc,
    vk::ImageUsageFlags imageUsageFlags)
{
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
        vk::ImageLayout::eUndefined);

    return vk::raii::Image(context->deviceContext().device, imageInfo);
}

vk::raii::DeviceMemory ImageSimple::allocateDeviceMemory(Context* context, vk::raii::Image* image)
{
    vk::MemoryRequirements memRequirements = image->getMemoryRequirements();
    uint32_t memoryType = vk::helper::findMemoryType(context->deviceContext().physicalDevice, memRequirements.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal);
    vk::raii::DeviceMemory memory = context->deviceContext().device.allocateMemory(vk::MemoryAllocateInfo(memRequirements.size, memoryType));
    image->bindMemory(*memory, 0);

    return memory;
}

vk::raii::ImageView ImageSimple::createImageView(Context* context, vk::raii::Image* image, const ImageDescription& imageDescription)
{
    vk::ImageViewCreateInfo viewInfo({},
        *(*image),
        vk::ImageViewType::e2D,
        to_vk_format(imageDescription.format),
        {},
        { vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 });
    return vk::raii::ImageView(context->deviceContext().device, viewInfo);
}

ImageSimple::ImageSimple(Context* context, const ImageDescription& desc)
    : Image(context)
{
    const ImageDescription& imageDescription = desc;
    vk::ImageUsageFlags usage = vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eStorage;
    vk::AccessFlags access = vk::AccessFlagBits::eNone;
    vk::ImageLayout layout = vk::ImageLayout::eUndefined;
    vk::PipelineStageFlags stages = vk::PipelineStageFlagBits::eTopOfPipe;

    vk::raii::Image image = createImage(context, imageDescription, usage);
    vk::raii::DeviceMemory memory = allocateDeviceMemory(context, &image);
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
bool ImageSimple::IsStorage() const
{
    assert(m_imageDataPtr);
    return m_imageDataPtr->IsStorage();
}
bool ImageSimple::IsSampled() const
{
    assert(m_imageDataPtr);
    return m_imageDataPtr->IsSampled();
}

const ImageDescription& ImageSimple::description() const
{
    assert(m_imageDataPtr);
    return m_imageDataPtr->description();
}

const vk::raii::ImageView& ImageSimple::view() const
{
    assert(m_imageDataPtr);
    return m_imageDataPtr->view();
}

const vk::ImageLayout& ImageSimple::layout() const
{
    assert(m_imageDataPtr);
    return m_imageDataPtr->layout();
}

const vk::PipelineStageFlags& ImageSimple::stages() const
{
    assert(m_imageDataPtr);
    return m_imageDataPtr->stages();
}
const vk::AccessFlags& ImageSimple::access() const
{
    assert(m_imageDataPtr);
    return m_imageDataPtr->access();
}
const vk::Image& ImageSimple::image() const
{
    assert(m_imageDataPtr);
    return m_imageDataPtr->image();
}
void ImageSimple::updateLayout(vk::ImageLayout newLayout)
{
    assert(m_imageDataPtr);
    m_imageDataPtr->updateLayout(newLayout);
}
void ImageSimple::updateStages(vk::PipelineStageFlags newPipelineStageFlags)
{
    assert(m_imageDataPtr);
    m_imageDataPtr->updateStages(newPipelineStageFlags);
}
void ImageSimple::updateAccess(vk::AccessFlags newFlags)
{
    assert(m_imageDataPtr);
    m_imageDataPtr->updateAccess(newFlags);
}

} // namespace rprpp