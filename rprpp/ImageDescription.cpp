#include "ImageDescription.h"

namespace rprpp
{
ImageDescription::ImageDescription(uint32_t w, uint32_t h, ImageFormat f)
    : width(w)
    , height(h)
    , format(f)
{
}

ImageDescription::ImageDescription(const RprPpImageDescription& desc)
    : width(desc.width)
    , height(desc.height)
    , format(static_cast<rprpp::ImageFormat>(desc.format))
{
}

bool operator==(const ImageDescription& imageDescription1, const ImageDescription& imageDescription2)
{
    return imageDescription1.width == imageDescription2.width &&
           imageDescription1.height == imageDescription2.height &&
           imageDescription1.format == imageDescription2.format;
}

}
