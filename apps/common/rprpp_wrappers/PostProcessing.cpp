#include "PostProcessing.h"

namespace rprpp::wrappers {

PostProcessing::PostProcessing(const Context& context)
    : m_context(context.get())
{
    RprPpError status;

    status = rprppContextCreatePostProcessing(m_context, &m_postProcessing);
    RPRPP_CHECK(status);
}

PostProcessing::~PostProcessing()
{
    RprPpError status;

    status = rprppContextDestroyPostProcessing(m_context, m_postProcessing);
    RPRPP_CHECK(status);
}

void PostProcessing::run(RprPpVkSemaphore aovsReadySemaphore, RprPpVkSemaphore toSignalAfterProcessingSemaphore)
{
    RprPpError status;

    status = rprppPostProcessingRun(m_postProcessing, aovsReadySemaphore, toSignalAfterProcessingSemaphore);
    RPRPP_CHECK(status);
}

void PostProcessing::setOutput(const Image& img)
{
    RprPpError status;

    status = rprppPostProcessingSetOutput(m_postProcessing, img.get());
    RPRPP_CHECK(status);
}

void PostProcessing::setAovColor(const Image& img)
{
    RprPpError status;

    status = rprppPostProcessingSetAovColor(m_postProcessing, img.get());
    RPRPP_CHECK(status);
}

void PostProcessing::setAovOpacity(const Image& img)
{
    RprPpError status;

    status = rprppPostProcessingSetAovOpacity(m_postProcessing, img.get());
    RPRPP_CHECK(status);
}

void PostProcessing::setAovShadowCatcher(const Image& img)
{
    RprPpError status;

    status = rprppPostProcessingSetAovShadowCatcher(m_postProcessing, img.get());
    RPRPP_CHECK(status);
}

void PostProcessing::setAovReflectionCatcher(const Image& img)
{
    RprPpError status;

    status = rprppPostProcessingSetAovReflectionCatcher(m_postProcessing, img.get());
    RPRPP_CHECK(status);
}

void PostProcessing::setAovMattePass(const Image& img)
{
    RprPpError status;

    status = rprppPostProcessingSetAovMattePass(m_postProcessing, img.get());
    RPRPP_CHECK(status);
}

void PostProcessing::setAovBackground(const Image& img)
{
    RprPpError status;

    status = rprppPostProcessingSetAovBackground(m_postProcessing, img.get());
    RPRPP_CHECK(status);
}

void PostProcessing::setTileOffset(int x, int y)
{
    RprPpError status;

    status = rprppPostProcessingSetTileOffset(m_postProcessing, x, y);
    RPRPP_CHECK(status);
}

void PostProcessing::setGamma(float gamma)
{
    RprPpError status;

    status = rprppPostProcessingSetGamma(m_postProcessing, gamma);
    RPRPP_CHECK(status);
}

void PostProcessing::setShadowIntensity(float shadowIntensity)
{
    RprPpError status;

    status = rprppPostProcessingSetShadowIntensity(m_postProcessing, shadowIntensity);
    RPRPP_CHECK(status);
}

void PostProcessing::setToneMapWhitepoint(float x, float y, float z)
{
    RprPpError status;

    status = rprppPostProcessingSetToneMapWhitepoint(m_postProcessing, x, y, z);
    RPRPP_CHECK(status);
}

void PostProcessing::setToneMapVignetting(float vignetting)
{
    RprPpError status;

    status = rprppPostProcessingSetToneMapVignetting(m_postProcessing, vignetting);
    RPRPP_CHECK(status);
}

void PostProcessing::setToneMapCrushBlacks(float crushBlacks)
{
    RprPpError status;

    status = rprppPostProcessingSetToneMapCrushBlacks(m_postProcessing, crushBlacks);
    RPRPP_CHECK(status);
}

void PostProcessing::setToneMapBurnHighlights(float burnHighlights)
{
    RprPpError status;

    status = rprppPostProcessingSetToneMapBurnHighlights(m_postProcessing, burnHighlights);
    RPRPP_CHECK(status);
}

void PostProcessing::setToneMapSaturation(float saturation)
{
    RprPpError status;

    status = rprppPostProcessingSetToneMapSaturation(m_postProcessing, saturation);
    RPRPP_CHECK(status);
}

void PostProcessing::setToneMapCm2Factor(float cm2Factor)
{
    RprPpError status;

    status = rprppPostProcessingSetToneMapCm2Factor(m_postProcessing, cm2Factor);
    RPRPP_CHECK(status);
}

void PostProcessing::setToneMapFilmIso(float filmIso)
{
    RprPpError status;

    status = rprppPostProcessingSetToneMapFilmIso(m_postProcessing, filmIso);
    RPRPP_CHECK(status);
}

void PostProcessing::setToneMapCameraShutter(float cameraShutter)
{
    RprPpError status;

    status = rprppPostProcessingSetToneMapCameraShutter(m_postProcessing, cameraShutter);
    RPRPP_CHECK(status);
}

void PostProcessing::setToneMapFNumber(float fNumber)
{
    RprPpError status;

    status = rprppPostProcessingSetToneMapFNumber(m_postProcessing, fNumber);
    RPRPP_CHECK(status);
}

void PostProcessing::setToneMapFocalLength(float focalLength)
{
    RprPpError status;

    status = rprppPostProcessingSetToneMapFocalLength(m_postProcessing, focalLength);
    RPRPP_CHECK(status);
}

void PostProcessing::setToneMapAperture(float aperture)
{
    RprPpError status;

    status = rprppPostProcessingSetToneMapAperture(m_postProcessing, aperture);
    RPRPP_CHECK(status);
}

void PostProcessing::setBloomRadius(float radius)
{
    RprPpError status;

    status = rprppPostProcessingSetBloomRadius(m_postProcessing, radius);
    RPRPP_CHECK(status);
}

void PostProcessing::setBloomBrightnessScale(float brightnessScale)
{
    RprPpError status;

    status = rprppPostProcessingSetBloomBrightnessScale(m_postProcessing, brightnessScale);
    RPRPP_CHECK(status);
}

void PostProcessing::setBloomThreshold(float threshold)
{
    RprPpError status;

    status = rprppPostProcessingSetBloomThreshold(m_postProcessing, threshold);
    RPRPP_CHECK(status);
}

void PostProcessing::setBloomEnabled(bool enabled)
{
    RprPpError status;

    status = rprppPostProcessingSetBloomEnabled(m_postProcessing, enabled ? RPRPP_TRUE : RPRPP_FALSE);
    RPRPP_CHECK(status);
}

void PostProcessing::setDenoiserEnabled(bool enabled)
{
    RprPpError status;

    status = rprppPostProcessingSetDenoiserEnabled(m_postProcessing, enabled ? RPRPP_TRUE : RPRPP_FALSE);
    RPRPP_CHECK(status);
}

}
