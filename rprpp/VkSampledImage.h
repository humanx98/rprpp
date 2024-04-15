#pragma once

#include "Image.h"

namespace rprpp {

// non-owning image
class VkSampledImage : public Image {
public:
    VkSampledImage(Context* context, vk::Image image, const ImageDescription& desc);

    [[nodiscard]] bool IsStorage() const override;

    [[nodiscard]] bool IsSampled() const override;

    [[nodiscard]] const ImageDescription& description() const override;

    [[nodiscard]] const vk::raii::ImageView& view() const override;

    [[nodiscard]] const vk::ImageLayout& layout() const override;

    [[nodiscard]] const vk::PipelineStageFlags& stages() const override;

    [[nodiscard]] const vk::AccessFlags& access() const override;

    [[nodiscard]] const vk::Image& image() const override;

protected:
    void updateLayout(vk::ImageLayout newLayout) override;
    void updateStages(vk::PipelineStageFlags newPipelineStageFlags) override;
    void updateAccess(vk::AccessFlags newFlags) override;

private:
    static vk::raii::ImageView createImageView(Context* context, vk::Image image, const ImageDescription& imageDescription);

    vk::Image m_notOwnedImage;
    ImageDescription m_description;
    vk::raii::ImageView m_view;

    vk::ImageUsageFlags m_usage;
    vk::AccessFlags m_access;
    vk::ImageLayout m_layout;
    vk::PipelineStageFlags m_stages;
};

}