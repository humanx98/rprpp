#include "Image.h"
#include "Context.h"
#include "vk/CommandBuffer.h"

namespace rprpp {

Image::Image(Context* context)
    : ContextObject(context)
{
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

    vk::ImageMemoryBarrier imageMemoryBarrier(access(),
        dstAccessFlags,
        layout(),
        dstImageLayout,
        VK_QUEUE_FAMILY_IGNORED,
        VK_QUEUE_FAMILY_IGNORED,
        image(),
        subresourceRange);

    commandBuffer.pipelineBarrier(stages(),
        dstPipelineStageFlags,
        {},
        nullptr,
        nullptr,
        imageMemoryBarrier);

    updateAccess(dstAccessFlags);
    updateLayout(dstImageLayout);
    updateStages(dstPipelineStageFlags);
}

} // namespace rprpp