#pragma once

#include "ImageDescription.h"
#include "rprpp.h"
#include "vk/DeviceContext.h"
#include "ContextObject.h"

namespace rprpp {

class Image : public ContextObject {
public:
    explicit Image(Context* context, const ImageDescription& desc);

    // transition functions
    void transitionImageLayout(
        vk::AccessFlags newAccessFlags,
        vk::ImageLayout newImageLayout,
        vk::PipelineStageFlags newPipelineStageFlags);

    void transitionImageLayout(
        const vk::raii::CommandBuffer& commandBuffer,
        vk::AccessFlags newAccessFlags,
        vk::ImageLayout newImageLayout,
        vk::PipelineStageFlags newPipelineStageFlags);

    [[nodiscard]]
    bool IsStorage() const noexcept {  return (m_usage & vk::ImageUsageFlagBits::eStorage) == vk::ImageUsageFlagBits::eStorage; }

    [[nodiscard]]
    bool IsSampled() const noexcept { return (m_usage & vk::ImageUsageFlagBits::eSampled) == vk::ImageUsageFlagBits::eSampled; }

    [[nodiscard]]
    const ImageDescription& description() const noexcept { return m_description; }

    [[nodiscard]]
    const vk::raii::ImageView& view() const noexcept { return m_view; }

    [[nodiscard]]
    const vk::ImageLayout& layout() const noexcept { return m_layout; }

    [[nodiscard]]
    const vk::PipelineStageFlags& stages() const noexcept { return m_stages; }

    [[nodiscard]]
    const vk::AccessFlags& access() const noexcept { return m_access; }

    [[nodiscard]]
    const vk::raii::Image& image() const noexcept { return m_image; }

private:
    // constructors
    static vk::raii::Image createImage(Context* context,
        const ImageDescription& desc,
        vk::ImageUsageFlags imageUsageFlags);
    static vk::raii::DeviceMemory allocateDeviceMemory(Context* context, vk::raii::Image* image);
    static vk::raii::ImageView createImageView(Context* context, vk::raii::Image* image, const ImageDescription& imageDescription);

    ImageDescription m_description;
    vk::ImageUsageFlags m_usage;

    vk::raii::Image m_image;
    vk::raii::DeviceMemory m_memory;
    vk::raii::ImageView m_view;

    vk::AccessFlags m_access;
    vk::ImageLayout m_layout;
    vk::PipelineStageFlags m_stages;
};

} // namespace