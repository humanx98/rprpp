#pragma once

#include "Image.h"

namespace rprpp
{

class ImageData : public Image {
public:
    ImageData(Context* context,
        vk::raii::Image&& _image,
        ImageDescription _description,
        vk::raii::DeviceMemory&& _memory,
        vk::raii::ImageView&& _view,
        vk::ImageUsageFlags _usage,
        vk::AccessFlags _access,
        vk::ImageLayout _layout,
        vk::PipelineStageFlags _stages);

    [[nodiscard]]
    bool IsStorage() const override;

    [[nodiscard]]
    bool IsSampled() const override;

    [[nodiscard]]
    const ImageDescription& description() const override;

    [[nodiscard]]
    const vk::raii::ImageView& view() const override;

    [[nodiscard]]
    const vk::ImageLayout& layout() const override;

    [[nodiscard]]
    const vk::PipelineStageFlags& stages() const override;

    [[nodiscard]]
    const vk::AccessFlags& access() const override;

    [[nodiscard]]
    const vk::Image& image() const override;

    void updateLayout(vk::ImageLayout newLayout) override ;
    void updateStages(vk::PipelineStageFlags newPipelineStageFlags) override ;
    void updateAccess(vk::AccessFlags newFlags) override;

private:
    vk::raii::Image m_image;
    ImageDescription m_description;
    vk::raii::DeviceMemory m_memory;
    vk::raii::ImageView m_view;

    vk::ImageUsageFlags m_usage;
    vk::AccessFlags m_access;
    vk::ImageLayout m_layout;
    vk::PipelineStageFlags m_stages;
};

} // namespace rprpp