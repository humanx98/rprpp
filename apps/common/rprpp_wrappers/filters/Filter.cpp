#include "Filter.h"
#include <cassert>

namespace rprpp::wrappers::filters {

Filter::Filter(const Context& context)
: m_context(context.get()),
  m_filter(nullptr)
{
}

Filter::~Filter()
{
    if (!m_filter)
        return;

    RprPpError status;
    status = rprppContextDestroyFilter(m_context, m_filter);
    RPRPP_CHECK(status);

#ifndef NDEBUG
    m_context = nullptr;
    m_filter = nullptr;
#endif
}

RprPpVkSemaphore Filter::run(RprPpVkSemaphore waitSemaphore)
{
    assert(m_context);
    assert(m_filter);

    RprPpError status;
    RprPpVkSemaphore finishedSemaphore;

    status = rprppFilterRun(m_filter, waitSemaphore, &finishedSemaphore);
    RPRPP_CHECK(status);

    return finishedSemaphore;
}

void Filter::setInput(const Image& image)
{
    RprPpError status;

    status = rprppFilterSetInput(m_filter, image.get());
    RPRPP_CHECK(status);
}

void Filter::setOutput(const Image& image)
{
    RprPpError status;

    status = rprppFilterSetOutput(m_filter, image.get());
    RPRPP_CHECK(status);
}

RprPpFilter Filter::get() const noexcept
{
    return m_filter;
}


void Filter::setFilter(RprPpFilter filter)
{
    assert(filter);
    m_filter = filter;
}

}