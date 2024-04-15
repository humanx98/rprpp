#include "VkSampledImage.h"
#include "Context.h"

namespace rprpp {
VkSampledImage::VkSampledImage(rprpp::Context* context, vk::Image image, const rprpp::ImageDescription& desc)
    : Image(context)
    , m_notOwnedImage(image)
    , m_description(desc)
    , m_view(createImageView(context, image, desc))
    , m_usage(vk::ImageUsageFlagBits::eSampled)
    , m_access(vk::AccessFlagBits::eShaderRead)
    , m_layout(vk::ImageLayout::eShaderReadOnlyOptimal)
    , m_stages(vk::PipelineStageFlagBits::eComputeShader)
{
}

vk::raii::ImageView VkSampledImage::createImageView(Context* context, vk::Image image, const ImageDescription& imageDescription)
{
    vk::ImageViewCreateInfo viewInfo({},
        image,
        vk::ImageViewType::e2D,
        to_vk_format(imageDescription.format),
        {},
        { vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 });

    vk::raii::ImageView view(context->deviceContext().device, viewInfo);
    return view;
}

[[nodiscard]] bool VkSampledImage::IsStorage() const
{
    return (m_usage & vk::ImageUsageFlagBits::eStorage) == vk::ImageUsageFlagBits::eStorage;
}

[[nodiscard]] bool VkSampledImage::IsSampled() const
{
    return (m_usage & vk::ImageUsageFlagBits::eSampled) == vk::ImageUsageFlagBits::eSampled;
}

const ImageDescription& VkSampledImage::description() const
{
    return m_description;
}

const vk::raii::ImageView& VkSampledImage::view() const
{
    return m_view;
}

const vk::ImageLayout& VkSampledImage::layout() const
{
    return m_layout;
}
const vk::PipelineStageFlags& VkSampledImage::stages() const
{
    return m_stages;
}
const vk::AccessFlags& VkSampledImage::access() const
{
    return m_access;
}

const vk::Image& VkSampledImage::image() const
{
    return m_notOwnedImage;
}

void VkSampledImage::updateLayout(vk::ImageLayout newLayout)
{
    m_layout = newLayout;
}

void VkSampledImage::updateStages(vk::PipelineStageFlags newPipelineStageFlags)
{
    m_stages = newPipelineStageFlags;
}
void VkSampledImage::updateAccess(vk::AccessFlags newFlags)
{
    m_access = newFlags;
}

} // namespace