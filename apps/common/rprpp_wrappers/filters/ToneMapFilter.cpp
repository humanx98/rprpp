#include "ToneMapFilter.h"

namespace rprpp::wrappers::filters {

ToneMapFilter::ToneMapFilter(const Context& context)
    : Filter(context)
{
    RprPpError status;

    status = rprppContextCreateToneMapFilter(m_context, &m_filter);
    RPRPP_CHECK(status);
}

void ToneMapFilter::setGamma(float gamma)
{
    RprPpError status;

    status = rprppToneMapFilterSetGamma(m_filter, gamma);
    RPRPP_CHECK(status);
}

void ToneMapFilter::setWhitepoint(float x, float y, float z)
{
    RprPpError status;

    status = rprppToneMapFilterSetWhitepoint(m_filter, x, y, z);
    RPRPP_CHECK(status);
}

void ToneMapFilter::setVignetting(float vignetting)
{
    RprPpError status;

    status = rprppToneMapFilterSetVignetting(m_filter, vignetting);
    RPRPP_CHECK(status);
}

void ToneMapFilter::setCrushBlacks(float crushBlacks)
{
    RprPpError status;

    status = rprppToneMapFilterSetCrushBlacks(m_filter, crushBlacks);
    RPRPP_CHECK(status);
}

void ToneMapFilter::setBurnHighlights(float burnHighlights)
{
    RprPpError status;

    status = rprppToneMapFilterSetBurnHighlights(m_filter, burnHighlights);
    RPRPP_CHECK(status);
}

void ToneMapFilter::setSaturation(float saturation)
{
    RprPpError status;

    status = rprppToneMapFilterSetSaturation(m_filter, saturation);
    RPRPP_CHECK(status);
}

void ToneMapFilter::setCm2Factor(float cm2Factor)
{
    RprPpError status;

    status = rprppToneMapFilterSetCm2Factor(m_filter, cm2Factor);
    RPRPP_CHECK(status);
}

void ToneMapFilter::setFilmIso(float filmIso)
{
    RprPpError status;

    status = rprppToneMapFilterSetFilmIso(m_filter, filmIso);
    RPRPP_CHECK(status);
}

void ToneMapFilter::setCameraShutter(float cameraShutter)
{
    RprPpError status;

    status = rprppToneMapFilterSetCameraShutter(m_filter, cameraShutter);
    RPRPP_CHECK(status);
}

void ToneMapFilter::setFNumber(float fNumber)
{
    RprPpError status;

    status = rprppToneMapFilterSetFNumber(m_filter, fNumber);
    RPRPP_CHECK(status);
}

void ToneMapFilter::setFocalLength(float focalLength)
{
    RprPpError status;

    status = rprppToneMapFilterSetFocalLength(m_filter, focalLength);
    RPRPP_CHECK(status);
}

void ToneMapFilter::setAperture(float aperture)
{
    RprPpError status;

    status = rprppToneMapFilterSetAperture(m_filter, aperture);
    RPRPP_CHECK(status);
}

}