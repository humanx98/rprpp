#include "DenoiserFilter.h"

namespace rprpp::wrappers::filters {

DenoiserFilter::DenoiserFilter(const Context& _context)
    : Filter(_context)
{
    RprPpError status;

    RprPpFilter filter;
    status = rprppContextCreateDenoiserFilter(context(), &filter);
    RPRPP_CHECK(status);

    setFilter(filter);
}

}