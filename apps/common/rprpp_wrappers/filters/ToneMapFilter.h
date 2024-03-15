#pragma once

#include "Filter.h"

namespace rprpp::wrappers::filters {

class ToneMapFilter : public Filter {
public:
    ToneMapFilter(const Context& context);
    void setGamma(float gamma);
    void setWhitepoint(float x, float y, float z);
    void setVignetting(float vignetting);
    void setCrushBlacks(float crushBlacks);
    void setBurnHighlights(float burnHighlights);
    void setSaturation(float saturation);
    void setCm2Factor(float cm2Factor);
    void setFilmIso(float filmIso);
    void setCameraShutter(float cameraShutter);
    void setFNumber(float fNumber);
    void setFocalLength(float focalLength);
    void setAperture(float aperture);
};

}