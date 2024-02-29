#include "Image.h"

namespace rprpp {

Image::Image(vk::helper::Image&& image, const ImageDescription& desc) noexcept
    : m_image(std::move(image))
    , m_description(desc)
{
}

const ImageDescription& Image::description() const noexcept
{
    return m_description;
}

vk::helper::Image& Image::get() noexcept
{
    return m_image;
}

}