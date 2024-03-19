#pragma once

#include "Filter.h"

namespace rprpp::wrappers::filters {

class ComposeColorShadowReflectionFilter : public Filter {
public:
    ComposeColorShadowReflectionFilter(const Context& context);

    void setAovOpacity(const Image& image);
    void setAovShadowCatcher(const Image& image);
    void setAovReflectionCatcher(const Image& image);
    void setAovMattePass(const Image& image);
    void setAovBackground(const Image& image);
    void setTileOffset(int x, int y);
    void setShadowIntensity(float shadowIntensity);
    void setNotRefractiveBackgroundColor(float x, float y, float z);
    void setNotRefractiveBackgroundColorWeight(float weight);
};

}