#pragma once

#include "rprpp.h"
#include "vk.h"

namespace rprpp {

enum class ImageFormat {
    eR8G8B8A8Unorm = RPRPP_IMAGE_FROMAT_R8G8B8A8_UNORM,
    eR32G32B32A32Sfloat = RPRPP_IMAGE_FROMAT_R32G32B32A32_SFLOAT,
    eB8G8R8A8Unorm = RPRPP_IMAGE_FROMAT_B8G8R8A8_UNORM,
};

inline vk::Format to_vk_format(ImageFormat from)
{
    switch (from) {
    case ImageFormat::eR8G8B8A8Unorm:
        return vk::Format::eR8G8B8A8Unorm;
    case ImageFormat::eB8G8R8A8Unorm:
        return vk::Format::eB8G8R8A8Unorm;
    case ImageFormat::eR32G32B32A32Sfloat:
    default:
        return vk::Format::eR32G32B32A32Sfloat;
    }
}

inline size_t to_pixel_size(ImageFormat from)
{
    switch (from) {
    case ImageFormat::eR8G8B8A8Unorm:
    case ImageFormat::eB8G8R8A8Unorm:
        return 4 * sizeof(uint8_t);
    case ImageFormat::eR32G32B32A32Sfloat:
    default:
        return 4 * sizeof(float);
    }
}

inline std::string to_glslformat(ImageFormat from)
{
    switch (from) {
    case ImageFormat::eR8G8B8A8Unorm:
    case ImageFormat::eB8G8R8A8Unorm:
        return "rgba8";
    case ImageFormat::eR32G32B32A32Sfloat:
    default:
        return "rgba32f";
    }
}

}