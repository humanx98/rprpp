#include "ImageData.h"

namespace rprpp {
ImageData::ImageData(Context* context,
    vk::raii::Image&& _image,
    ImageDescription _description,
    vk::raii::DeviceMemory&& _memory,
    vk::raii::ImageView&& _view,
    vk::ImageUsageFlags _usage,
    vk::AccessFlags _access,
    vk::ImageLayout _layout,
    vk::PipelineStageFlags _stages)
    : Image(context)
    , m_image(std::move(_image))
    , m_description(_description)
    , m_memory(std::move(_memory))
    , m_view(std::move(_view))
    , m_usage(_usage)
    , m_access(_access)
    , m_layout(_layout)
    , m_stages(_stages)
{
}

bool ImageData::IsStorage() const { return (m_usage & vk::ImageUsageFlagBits::eStorage) == vk::ImageUsageFlagBits::eStorage; }

bool ImageData::IsSampled() const { return (m_usage & vk::ImageUsageFlagBits::eSampled) == vk::ImageUsageFlagBits::eSampled; }

const ImageDescription& ImageData::description() const { return m_description; }

const vk::raii::ImageView& ImageData::view() const { return m_view; }

const vk::ImageLayout& ImageData::layout() const { return m_layout; }

const vk::PipelineStageFlags& ImageData::stages() const { return m_stages; }

const vk::AccessFlags& ImageData::access() const { return m_access; }

const vk::Image& ImageData::image() const { return *m_image; }

void ImageData::updateLayout(vk::ImageLayout newLayout) { m_layout = newLayout; }
void ImageData::updateStages(vk::PipelineStageFlags newPipelineStageFlags) { m_stages = newPipelineStageFlags; }
void ImageData::updateAccess(vk::AccessFlags newFlags) { m_access = newFlags; }
} // namespace rprpp