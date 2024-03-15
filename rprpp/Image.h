#pragma once

#include "ImageFormat.h"
#include "rprpp.h"
#include "vk/DeviceContext.h"

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

class Image {
public:
    Image(vk::raii::Image&& image, vk::raii::DeviceMemory&& memory, vk::raii::ImageView&& view, vk::ImageUsageFlags usage, const ImageDescription& desc) noexcept;
    Image(vk::Image image, vk::raii::ImageView&& view, const ImageDescription& desc) noexcept;
    Image(Image&&) noexcept = default;
    Image& operator=(Image&&) noexcept = default;

    static Image create(vk::helper::DeviceContext& dctx, const ImageDescription& desc);
    static Image createFromVkSampledImage(const vk::helper::DeviceContext& dctx, vk::Image image, const ImageDescription& desc);
    static Image createFromDx11Texture(vk::helper::DeviceContext& dctx, HANDLE dx11textureHandle, const ImageDescription& desc);
    static void transitionImageLayout(vk::helper::DeviceContext& dctx,
        Image& image,
        vk::AccessFlags dstAccess,
        vk::ImageLayout dstLayout,
        vk::PipelineStageFlags dstStage);
    static void transitionImageLayout(const vk::raii::CommandBuffer& commandBuffer,
        Image& image,
        vk::AccessFlags dstAccess,
        vk::ImageLayout dstLayout,
        vk::PipelineStageFlags dstStage);

    const vk::Image get() const noexcept;
    const vk::ImageView view() const noexcept;
    vk::ImageUsageFlags usage() const noexcept;
    bool IsStorage() const noexcept;
    bool IsSampled() const noexcept;
    const ImageDescription& description() const noexcept;

    vk::AccessFlags getAccess() const noexcept;
    void setAccess(vk::AccessFlags access) noexcept;

    vk::ImageLayout getLayout() const noexcept;
    void setLayout(vk::ImageLayout layout) noexcept;

    vk::PipelineStageFlags getPipelineStages() const noexcept;
    void setPipelineStages(vk::PipelineStageFlags stages) noexcept;

    Image(const Image&) = delete;
    Image& operator=(const Image&) = delete;

private:
    std::optional<vk::raii::Image> m_image;
    std::optional<vk::raii::DeviceMemory> m_memory;
    std::optional<vk::Image> m_notOwnedImage;
    vk::raii::ImageView m_view;
    ImageDescription m_description;
    vk::ImageUsageFlags m_usage;
    vk::AccessFlags m_access = vk::AccessFlagBits::eNone;
    vk::ImageLayout m_layout = vk::ImageLayout::eUndefined;
    vk::PipelineStageFlags m_stages = vk::PipelineStageFlagBits::eTopOfPipe;
};

}