#pragma once

#include "ImageFormat.h"
#include "rprpp.h"
#include "vk/DeviceContext.h"
#include "ContextObject.h"

namespace rprpp {

struct ImageDescription {
    uint32_t width;
    uint32_t height;
    ImageFormat format;

    ImageDescription(uint32_t w, uint32_t h, ImageFormat f);
    ImageDescription(const RprPpImageDescription& desc);

    friend bool operator==(const ImageDescription&, const ImageDescription&) = default;
    friend bool operator!=(const ImageDescription&, const ImageDescription&) = default;
};

class Image : public ContextObject {
public:
    Image(Context* context, vk::raii::Image&& image, vk::raii::DeviceMemory&& memory, vk::raii::ImageView&& view, vk::ImageUsageFlags usage, const ImageDescription& desc) noexcept;
    Image(Context* context, vk::Image image, vk::raii::ImageView&& view, const ImageDescription& desc) noexcept;

    explicit Image(Context* context, const ImageDescription& desc);

    static Image createFromVkSampledImage(Context* context, vk::Image image, const ImageDescription& desc);
    static Image createFromDx11Texture(Context* context, HANDLE dx11textureHandle, const ImageDescription& desc);

    void transitionImageLayout();
    void transitionImageLayout(const vk::raii::CommandBuffer& commandBuffer);

    const vk::Image get() const noexcept;
    const vk::ImageView view() const noexcept;
    vk::ImageUsageFlags usage() const noexcept;
    bool IsStorage() const noexcept;
    bool IsSampled() const noexcept;
    const ImageDescription& description() const noexcept;

    vk::AccessFlags getAccess() const noexcept;
    vk::ImageLayout getLayout() const noexcept;

    vk::PipelineStageFlags getPipelineStages() const noexcept;
    void setPipelineStages(vk::PipelineStageFlags stages) noexcept;

private:
    vk::raii::Image createImage(const ImageDescription& desc, vk::ImageUsageFlags imageUsageFlags);
    vk::raii::DeviceMemory allocateDeviceMemory();
    vk::raii::ImageView createImageView();

    ImageDescription m_description;
    vk::ImageUsageFlags m_usage;

    vk::raii::Image m_image;
    vk::raii::DeviceMemory m_memory;
    vk::Image m_notOwnedImage;
    vk::raii::ImageView m_view;
    vk::AccessFlags m_access = vk::AccessFlagBits::eNone;
    vk::ImageLayout m_layout = vk::ImageLayout::eUndefined;
    vk::PipelineStageFlags m_stages = vk::PipelineStageFlagBits::eTopOfPipe;
};

}