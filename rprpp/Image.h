#pragma once

#include "vk_helper.h"
#include "ImageFormat.h"

namespace rprpp {

struct ImageDescription {
    uint32_t width;
    uint32_t height;
    ImageFormat format;
};

class Image {
public:
    Image(vk::helper::Image&& image, const ImageDescription& desc) noexcept;

    Image(Image&&) = default;
    Image& operator=(Image&&) = default;

    vk::helper::Image& get() noexcept;
    const ImageDescription& description() const noexcept;

    Image(Image&) = delete;
    Image& operator=(const Image&) = delete;

private:
    ImageDescription m_description;
    vk::helper::Image m_image;
};

}