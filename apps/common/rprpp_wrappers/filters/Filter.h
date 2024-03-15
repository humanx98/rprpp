#pragma once

#include "../Context.h"
#include "../Image.h"
#include "../helper.h"

namespace rprpp::wrappers::filters {

class Filter {
public:
    Filter(const Context& context);
    ~Filter();

    RprPpVkSemaphore run(RprPpVkSemaphore waitSemaphore);
    void setInput(const Image& image);
    void setOutput(const Image& image);
    RprPpFilter get() const noexcept;

    Filter(const Filter&) = delete;
    Filter& operator=(const Filter&) = delete;

protected:
    RprPpContext m_context;
    RprPpFilter m_filter;
};

}