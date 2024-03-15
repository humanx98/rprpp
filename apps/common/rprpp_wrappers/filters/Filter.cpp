#include "Filter.h"

namespace rprpp::wrappers::filters {

Filter::Filter(const Context& context)
    : m_context(context.get())
{
}

Filter::~Filter()
{
    RprPpError status;

    status = rprppContextDestroyFilter(m_context, m_filter);
    RPRPP_CHECK(status);
}

RprPpVkSemaphore Filter::run(RprPpVkSemaphore waitSemaphore)
{
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

}