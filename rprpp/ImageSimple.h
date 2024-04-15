#pragma once

#include "Image.h"
#include "ImageData.h"

namespace rprpp {
class ImageSimple : public Image {
public:
    explicit ImageSimple(Context* context, const ImageDescription& desc);

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
    // constructors
    static vk::raii::Image createImage(Context* context, const ImageDescription& desc, vk::ImageUsageFlags imageUsageFlags);
    static vk::raii::DeviceMemory allocateDeviceMemory(Context* context, vk::raii::Image* image);
    static vk::raii::ImageView createImageView(Context* context, vk::raii::Image* image, const ImageDescription& imageDescription);

    std::unique_ptr<ImageData> m_imageDataPtr;
};
}