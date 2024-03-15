#include "ComposeOpacityShadowFilter.h"

namespace rprpp::wrappers::filters {

ComposeOpacityShadowFilter::ComposeOpacityShadowFilter(const Context& context)
    : Filter(context)
{
    RprPpError status;

    status = rprppContextCreateComposeOpacityShadowFilter(m_context, &m_filter);
    RPRPP_CHECK(status);
}

void ComposeOpacityShadowFilter::setAovShadowCatcher(const Image& img)
{
    RprPpError status;

    status = rprppComposeOpacityShadowFilterSetAovShadowCatcher(m_filter, img.get());
    RPRPP_CHECK(status);
}

void ComposeOpacityShadowFilter::setTileOffset(int x, int y)
{
    RprPpError status;

    status = rprppComposeOpacityShadowFilterSetTileOffset(m_filter, x, y);
    RPRPP_CHECK(status);
}

void ComposeOpacityShadowFilter::setShadowIntensity(float shadowIntensity)
{
    RprPpError status;

    status = rprppComposeOpacityShadowFilterSetShadowIntensity(m_filter, shadowIntensity);
    RPRPP_CHECK(status);
}

}