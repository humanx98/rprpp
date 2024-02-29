#pragma once

#include "ImageFormat.h"
#include "vk_helper.h"

namespace rprpp {

struct ImageDescription {
    uint32_t width;
    uint32_t height;
    ImageFormat format;
};

class Image {
public:
    Image(vk::raii::Image&& image, vk::raii::DeviceMemory&& memory, vk::raii::ImageView&& view, const ImageDescription& desc) noexcept;
    Image(Image&&) = default;
    Image& operator=(Image&&) = default;

    static Image create(const vk::helper::DeviceContext& dctx, const ImageDescription& desc, vk::ImageUsageFlags usage);
    static Image createFromDx11Texture(const vk::helper::DeviceContext& dctx, HANDLE dx11textureHandle, const ImageDescription& desc, vk::ImageUsageFlags usage);
    const vk::raii::Image& get() const noexcept;
    const vk::raii::ImageView& view() const noexcept;
    const ImageDescription& description() const noexcept;

    vk::AccessFlags getAccess() const noexcept;
    void setAccess(vk::AccessFlags access) noexcept;

    vk::ImageLayout getLayout() const noexcept;
    void setLayout(vk::ImageLayout layout) noexcept;

    vk::PipelineStageFlags getPipelineStages() const noexcept;
    void setPipelineStages(vk::PipelineStageFlags stages) noexcept;

    Image(Image&) = delete;
    Image& operator=(const Image&) = delete;

private:
    vk::raii::Image m_image;
    vk::raii::DeviceMemory m_memory;
    vk::raii::ImageView m_view;
    ImageDescription m_description;
    vk::AccessFlags m_access = vk::AccessFlagBits::eNone;
    vk::ImageLayout m_layout = vk::ImageLayout::eUndefined;
    vk::PipelineStageFlags m_stages = vk::PipelineStageFlagBits::eTopOfPipe;
};

}