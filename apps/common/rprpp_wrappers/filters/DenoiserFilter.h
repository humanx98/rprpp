#pragma once

#include "Filter.h"

namespace rprpp::wrappers::filters {

class DenoiserFilter : public Filter {
public:
    explicit DenoiserFilter(const Context& context);
    void setAovAlbedo(const Image& image);
    void setAovNormal(const Image& image);
};

}