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

void DenoiserFilter::setAovAlbedo(const Image& image)
{
    RprPpError status;

    status = rprppDenoiserFilterSetAovAlbedo(filter(), image.get());
    RPRPP_CHECK(status);
}

void DenoiserFilter::setAovNormal(const Image& image)
{
    RprPpError status;

    status = rprppDenoiserFilterSetAovNormal(filter(), image.get());
    RPRPP_CHECK(status);
}

}