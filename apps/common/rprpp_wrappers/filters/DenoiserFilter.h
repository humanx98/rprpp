#pragma once

#include "Filter.h"

namespace rprpp::wrappers::filters {

class DenoiserFilter : public Filter {
public:
    explicit DenoiserFilter(const Context& context);
};

}