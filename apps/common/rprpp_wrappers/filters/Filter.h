#pragma once

#include "../Context.h"
#include "../Image.h"
#include "../helper.h"

namespace rprpp::wrappers::filters {

class Filter {
public:
    virtual ~Filter();

    RprPpVkSemaphore run(RprPpVkSemaphore waitSemaphore = nullptr);
    void setInput(const Image& image);
    void setOutput(const Image& image);

    [[nodiscard]]
    RprPpFilter get() const noexcept;

    Filter(const Filter&)            = delete;
    Filter& operator=(const Filter&) = delete;
protected:
    // only constructed by child filters
    explicit Filter(const Context& context);
    void setFilter(RprPpFilter filter);

    [[nodiscard]]
    RprPpContext context() const noexcept { return m_context; }

    [[nodiscard]]
    RprPpFilter filter() const noexcept { return m_filter; }

private:
    RprPpContext m_context;
    RprPpFilter m_filter;
};

}