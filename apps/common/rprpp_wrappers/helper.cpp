#include "helper.h"

#include <cassert>
#include <iostream>
#include <sstream>

void process_error(RprPpError errorCode, const char* fileName, int line)
{
    std::stringstream stream;
    stream << "ERROR detected - program will stop\n";
    stream << "file = " << fileName << "\n";
    stream << "line = " << line << "\n";
    stream << "RPRPP error code = " << errorCode << "\n";

    throw std::runtime_error(stream.str());
}

size_t rprpp::wrappers::to_pixel_size(RprPpImageFormat from)
{
    switch (from) {
    case RPRPP_IMAGE_FROMAT_R8G8B8A8_UNORM:
    case RPRPP_IMAGE_FROMAT_B8G8R8A8_UNORM:
        return 4 * sizeof(uint8_t);
    case RPRPP_IMAGE_FROMAT_R32G32B32A32_SFLOAT:
        return 4 * sizeof(float);
    default:
        std::cerr << "Unsupported image format" << std::endl;
        assert(0);
        return 0;
    }
}