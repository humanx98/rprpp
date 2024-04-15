#pragma once

#include "ContextObject.h"
#include "ImageDescription.h"

namespace rprpp {

class Image : public ContextObject {
public:
    explicit Image(Context* context);

    [[nodiscard]] virtual bool IsStorage() const = 0;

    [[nodiscard]] virtual bool IsSampled() const = 0;

    [[nodiscard]] virtual const ImageDescription& description() const = 0;

    [[nodiscard]] virtual const vk::raii::ImageView& view() const = 0;

    [[nodiscard]] virtual const vk::ImageLayout& layout() const = 0;

    [[nodiscard]] virtual const vk::PipelineStageFlags& stages() const = 0;

    [[nodiscard]] virtual const vk::AccessFlags& access() const = 0;

    [[nodiscard]] virtual const vk::Image& image() const = 0;

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

protected:
    virtual void updateLayout(vk::ImageLayout newLayout) = 0;
    virtual void updateStages(vk::PipelineStageFlags newPipelineStageFlags) = 0;
    virtual void updateAccess(vk::AccessFlags newFlags) = 0;
};

} // namespace