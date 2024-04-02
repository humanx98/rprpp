#include "ComposeColorShadowReflectionFilter.h"

namespace rprpp::wrappers::filters {

ComposeColorShadowReflectionFilter::ComposeColorShadowReflectionFilter(const Context& _context)
: Filter(_context)
{
    RprPpError status;

    RprPpFilter filter;
    status = rprppContextCreateComposeColorShadowReflectionFilter(context(), &filter);
    RPRPP_CHECK(status);

    setFilter(filter);
}

void ComposeColorShadowReflectionFilter::setAovOpacity(const Image& img)
{
    RprPpError status;

    status = rprppComposeColorShadowReflectionFilterSetAovOpacity(filter(), img.get());
    RPRPP_CHECK(status);
}

void ComposeColorShadowReflectionFilter::setAovShadowCatcher(const Image& img)
{
    RprPpError status;

    status = rprppComposeColorShadowReflectionFilterSetAovShadowCatcher(filter(), img.get());
    RPRPP_CHECK(status);
}

void ComposeColorShadowReflectionFilter::setAovReflectionCatcher(const Image& img)
{
    RprPpError status;

    status = rprppComposeColorShadowReflectionFilterSetAovReflectionCatcher(filter(), img.get());
    RPRPP_CHECK(status);
}

void ComposeColorShadowReflectionFilter::setAovMattePass(const Image& img)
{
    RprPpError status;

    status = rprppComposeColorShadowReflectionFilterSetAovMattePass(filter(), img.get());
    RPRPP_CHECK(status);
}

void ComposeColorShadowReflectionFilter::setAovBackground(const Image& img)
{
    RprPpError status;

    status = rprppComposeColorShadowReflectionFilterSetAovBackground(filter(), img.get());
    RPRPP_CHECK(status);
}

void ComposeColorShadowReflectionFilter::setTileOffset(int x, int y)
{
    RprPpError status;

    status = rprppComposeColorShadowReflectionFilterSetTileOffset(filter(), x, y);
    RPRPP_CHECK(status);
}

void ComposeColorShadowReflectionFilter::setShadowIntensity(float shadowIntensity)
{
    RprPpError status;

    status = rprppComposeColorShadowReflectionFilterSetShadowIntensity(filter(), shadowIntensity);
    RPRPP_CHECK(status);
}

void ComposeColorShadowReflectionFilter::setNotRefractiveBackgroundColor(float x, float y, float z)
{
    RprPpError status;

    status = rprppComposeColorShadowReflectionFilterSetNotRefractiveBackgroundColor(filter(), x, y, z);
    RPRPP_CHECK(status);
}

void ComposeColorShadowReflectionFilter::setNotRefractiveBackgroundColorWeight(float weight)
{
    RprPpError status;

    status = rprppComposeColorShadowReflectionFilterSetNotRefractiveBackgroundColorWeight(filter(), weight);
    RPRPP_CHECK(status);
}


}