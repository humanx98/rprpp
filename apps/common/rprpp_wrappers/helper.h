#pragma once

#include "rprpp/rprpp.h"

#define RPRPP_CHECK(x)                           \
    {                                            \
        if ((x) != RPRPP_SUCCESS) {              \
            process_error(x, __FILE__, __LINE__);\
        }                                        \
    }

void process_error(RprPpError errorCode, const char* fileName, int line);

namespace rprpp::wrappers {

size_t to_pixel_size(RprPpImageFormat from);

}