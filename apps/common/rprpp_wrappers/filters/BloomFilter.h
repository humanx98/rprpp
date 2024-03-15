#pragma once

#include "Filter.h"

namespace rprpp::wrappers::filters {

class BloomFilter : public Filter {
public:
    BloomFilter(const Context& context);
    void setRadius(float radius);
    void setBrightnessScale(float brightnessScale);
    void setThreshold(float threshold);
};

}