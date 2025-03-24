#pragma once

#include "Filter.h"

namespace rprpp::wrappers::filters {

class BloomFilter : public Filter {
public:
    explicit BloomFilter(const Context& context);
    void setRadius(float radius);
    void setIntensity(float intensity);
    void setThreshold(float threshold);
};

}