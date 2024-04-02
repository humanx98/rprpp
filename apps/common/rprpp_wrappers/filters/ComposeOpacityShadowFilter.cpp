#include "ComposeOpacityShadowFilter.h"

namespace rprpp::wrappers::filters {

ComposeOpacityShadowFilter::ComposeOpacityShadowFilter(const Context& _context)
: Filter(_context)
{
    RprPpError status;

    RprPpFilter filter;
    status = rprppContextCreateComposeOpacityShadowFilter(context(), &filter);
    RPRPP_CHECK(status);

    setFilter(filter);
}

void ComposeOpacityShadowFilter::setAovShadowCatcher(const Image& img)
{
    RprPpError status;

    status = rprppComposeOpacityShadowFilterSetAovShadowCatcher(filter(), img.get());
    RPRPP_CHECK(status);
}

void ComposeOpacityShadowFilter::setTileOffset(int x, int y)
{
    RprPpError status;

    status = rprppComposeOpacityShadowFilterSetTileOffset(filter(), x, y);
    RPRPP_CHECK(status);
}

void ComposeOpacityShadowFilter::setShadowIntensity(float shadowIntensity)
{
    RprPpError status;

    status = rprppComposeOpacityShadowFilterSetShadowIntensity(filter(), shadowIntensity);
    RPRPP_CHECK(status);
}

}