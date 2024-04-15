#pragma once

#include "ImageFormat.h"

namespace rprpp {
struct ImageDescription {
    uint32_t width;
    uint32_t height;
    ImageFormat format;

    ImageDescription(const ImageDescription& other) = default;

    explicit ImageDescription(uint32_t w, uint32_t h, ImageFormat f);
    explicit ImageDescription(const RprPpImageDescription& desc);
};

bool operator==(const ImageDescription& imageDescription1, const ImageDescription& imageDescription2);

} // namespace rprpp