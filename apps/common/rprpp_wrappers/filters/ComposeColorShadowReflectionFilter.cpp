#include "ComposeColorShadowReflectionFilter.h"

namespace rprpp::wrappers::filters {

ComposeColorShadowReflectionFilter::ComposeColorShadowReflectionFilter(const Context& context)
    : Filter(context)
{
    RprPpError status;

    status = rprppContextCreateComposeColorShadowReflectionFilter(m_context, &m_filter);
    RPRPP_CHECK(status);
}

void ComposeColorShadowReflectionFilter::setAovOpacity(const Image& img)
{
    RprPpError status;

    status = rprppComposeColorShadowReflectionFilterSetAovOpacity(m_filter, img.get());
    RPRPP_CHECK(status);
}

void ComposeColorShadowReflectionFilter::setAovShadowCatcher(const Image& img)
{
    RprPpError status;

    status = rprppComposeColorShadowReflectionFilterSetAovShadowCatcher(m_filter, img.get());
    RPRPP_CHECK(status);
}

void ComposeColorShadowReflectionFilter::setAovReflectionCatcher(const Image& img)
{
    RprPpError status;

    status = rprppComposeColorShadowReflectionFilterSetAovReflectionCatcher(m_filter, img.get());
    RPRPP_CHECK(status);
}

void ComposeColorShadowReflectionFilter::setAovMattePass(const Image& img)
{
    RprPpError status;

    status = rprppComposeColorShadowReflectionFilterSetAovMattePass(m_filter, img.get());
    RPRPP_CHECK(status);
}

void ComposeColorShadowReflectionFilter::setAovBackground(const Image& img)
{
    RprPpError status;

    status = rprppComposeColorShadowReflectionFilterSetAovBackground(m_filter, img.get());
    RPRPP_CHECK(status);
}

void ComposeColorShadowReflectionFilter::setTileOffset(int x, int y)
{
    RprPpError status;

    status = rprppComposeColorShadowReflectionFilterSetTileOffset(m_filter, x, y);
    RPRPP_CHECK(status);
}

void ComposeColorShadowReflectionFilter::setShadowIntensity(float shadowIntensity)
{
    RprPpError status;

    status = rprppComposeColorShadowReflectionFilterSetShadowIntensity(m_filter, shadowIntensity);
    RPRPP_CHECK(status);
}

void ComposeColorShadowReflectionFilter::setNotRefractiveBackgroundColor(float x, float y, float z)
{
    RprPpError status;

    status = rprppComposeColorShadowReflectionFilterSetNotRefractiveBackgroundColor(m_filter, x, y, z);
    RPRPP_CHECK(status);
}

void ComposeColorShadowReflectionFilter::setNotRefractiveBackgroundColorWeight(float weight)
{
    RprPpError status;

    status = rprppComposeColorShadowReflectionFilterSetNotRefractiveBackgroundColorWeight(m_filter, weight);
    RPRPP_CHECK(status);
}


}