#include "DenoiserFilter.h"

namespace rprpp::wrappers::filters {

DenoiserFilter::DenoiserFilter(const Context& context)
    : Filter(context)
{
    RprPpError status;

    status = rprppContextCreateDenoiserFilter(m_context, &m_filter);
    RPRPP_CHECK(status);
}

}