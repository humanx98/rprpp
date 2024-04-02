#include "ToneMapFilter.h"

namespace rprpp::wrappers::filters {

ToneMapFilter::ToneMapFilter(const Context& _context)
    : Filter(_context)
{
    RprPpError status;

    RprPpFilter filter;
    status = rprppContextCreateToneMapFilter(context(), &filter);
    RPRPP_CHECK(status);

    setFilter(filter);
}

void ToneMapFilter::setGamma(float gamma)
{
    RprPpError status;

    status = rprppToneMapFilterSetGamma(filter(), gamma);
    RPRPP_CHECK(status);
}

void ToneMapFilter::setWhitepoint(float x, float y, float z)
{
    RprPpError status;

    status = rprppToneMapFilterSetWhitepoint(filter(), x, y, z);
    RPRPP_CHECK(status);
}

void ToneMapFilter::setVignetting(float vignetting)
{
    RprPpError status;

    status = rprppToneMapFilterSetVignetting(filter(), vignetting);
    RPRPP_CHECK(status);
}

void ToneMapFilter::setCrushBlacks(float crushBlacks)
{
    RprPpError status;

    status = rprppToneMapFilterSetCrushBlacks(filter(), crushBlacks);
    RPRPP_CHECK(status);
}

void ToneMapFilter::setBurnHighlights(float burnHighlights)
{
    RprPpError status;

    status = rprppToneMapFilterSetBurnHighlights(filter(), burnHighlights);
    RPRPP_CHECK(status);
}

void ToneMapFilter::setSaturation(float saturation)
{
    RprPpError status;

    status = rprppToneMapFilterSetSaturation(filter(), saturation);
    RPRPP_CHECK(status);
}

void ToneMapFilter::setCm2Factor(float cm2Factor)
{
    RprPpError status;

    status = rprppToneMapFilterSetCm2Factor(filter(), cm2Factor);
    RPRPP_CHECK(status);
}

void ToneMapFilter::setFilmIso(float filmIso)
{
    RprPpError status;

    status = rprppToneMapFilterSetFilmIso(filter(), filmIso);
    RPRPP_CHECK(status);
}

void ToneMapFilter::setCameraShutter(float cameraShutter)
{
    RprPpError status;

    status = rprppToneMapFilterSetCameraShutter(filter(), cameraShutter);
    RPRPP_CHECK(status);
}

void ToneMapFilter::setFNumber(float fNumber)
{
    RprPpError status;

    status = rprppToneMapFilterSetFNumber(filter(), fNumber);
    RPRPP_CHECK(status);
}

void ToneMapFilter::setFocalLength(float focalLength)
{
    RprPpError status;

    status = rprppToneMapFilterSetFocalLength(filter(), focalLength);
    RPRPP_CHECK(status);
}

void ToneMapFilter::setAperture(float aperture)
{
    RprPpError status;

    status = rprppToneMapFilterSetAperture(filter(), aperture);
    RPRPP_CHECK(status);
}

}