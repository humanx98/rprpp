#pragma once

#include "Filter.h"

namespace rprpp::wrappers::filters {

class ComposeOpacityShadowFilter : public Filter {
public:
    ComposeOpacityShadowFilter(const Context& context);

    void setAovShadowCatcher(const Image& image);
    void setTileOffset(int x, int y);
    void setShadowIntensity(float shadowIntensity);
};

}