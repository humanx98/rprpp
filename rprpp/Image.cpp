#include "Image.h"
#include "vk/CommandBuffer.h"
#include "Context.h"

namespace rprpp {

vk::raii::Image Image::createImage(
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

vk::raii::DeviceMemory Image::allocateDeviceMemory(Context* context, vk::raii::Image* image)
{
    vk::MemoryRequirements memRequirements = image->getMemoryRequirements();
    uint32_t memoryType = vk::helper::findMemoryType(context->deviceContext().physicalDevice, memRequirements.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal);
    vk::raii::DeviceMemory memory = context->deviceContext().device.allocateMemory(vk::MemoryAllocateInfo(memRequirements.size, memoryType));
    image->bindMemory(*memory, 0);

    return memory;
}

vk::raii::ImageView Image::createImageView(Context* context, vk::raii::Image* image, const ImageDescription& imageDescription)
{
    vk::ImageViewCreateInfo viewInfo({},
            *(*image),
            vk::ImageViewType::e2D,
            to_vk_format(imageDescription.format),
            {},
            { vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 });
    return vk::raii::ImageView(context->deviceContext().device, viewInfo);
}

Image::Image(Context* context, const ImageDescription& desc)
    : ContextObject(context)
    , m_description(desc)
    , m_usage(vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eStorage)
    , m_image(createImage(context, desc, m_usage))
    , m_memory(allocateDeviceMemory(context, &m_image))
    , m_view(createImageView(context, &m_image, m_description))
    , m_access(vk::AccessFlagBits::eNone)
    , m_layout(vk::ImageLayout::eUndefined)
    , m_stages(vk::PipelineStageFlagBits::eTopOfPipe)
{
    transitionImageLayout(
        vk::AccessFlagBits::eShaderRead | vk::AccessFlagBits::eShaderWrite,
        vk::ImageLayout::eGeneral,
        vk::PipelineStageFlagBits::eComputeShader);
}

void Image::transitionImageLayout(vk::AccessFlags dstAccessFlags, vk::ImageLayout dstImageLayout, vk::PipelineStageFlags dstPipelineStageFlags)
{
    vk::raii::CommandBuffer commandBuffer = deviceContext().takeCommandBuffer();
    commandBuffer.begin(vk::CommandBufferBeginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit));
    transitionImageLayout(commandBuffer, dstAccessFlags, dstImageLayout, dstPipelineStageFlags);
    commandBuffer.end();

    vk::SubmitInfo submitInfo(nullptr, nullptr, *commandBuffer);
    deviceContext().queue.submit(submitInfo);
    deviceContext().queue.waitIdle();
    deviceContext().returnCommandBuffer(std::move(commandBuffer));
}

void Image::transitionImageLayout(const vk::raii::CommandBuffer& commandBuffer,
    vk::AccessFlags dstAccessFlags,
    vk::ImageLayout dstImageLayout,
    vk::PipelineStageFlags dstPipelineStageFlags)
{
    vk::ImageSubresourceRange subresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1);

    vk::ImageMemoryBarrier imageMemoryBarrier(m_access,
        dstAccessFlags,
        m_layout,
        dstImageLayout,
        VK_QUEUE_FAMILY_IGNORED,
        VK_QUEUE_FAMILY_IGNORED,
        *m_image,
        subresourceRange);

    commandBuffer.pipelineBarrier(m_stages,
        dstPipelineStageFlags,
        {},
        nullptr,
        nullptr,
        imageMemoryBarrier);

    m_access = dstAccessFlags;
    m_layout = dstImageLayout;
    m_stages = dstPipelineStageFlags;
}

} // namespace rprpp