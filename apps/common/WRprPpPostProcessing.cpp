#include "WRprPpPostProcessing.h"

WRprPpPostProcessing::WRprPpPostProcessing(const WRprPpContext& context)
    : m_context(context.get())
{
    RprPpError status;

    status = rprppContextCreatePostProcessing(m_context, &m_postProcessing);
    RPRPP_CHECK(status);
}

WRprPpPostProcessing::~WRprPpPostProcessing()
{
    RprPpError status;

    status = rprppContextDestroyPostProcessing(m_context, m_postProcessing);
    RPRPP_CHECK(status);
}

void WRprPpPostProcessing::resize(uint32_t width, uint32_t height, RprPpImageFormat format, RprPpAovsVkInteropInfo* aovsVkInteropInfo)
{
    RprPpError status;

    status = rprppPostProcessingResize(m_postProcessing, width, height, format, aovsVkInteropInfo);
    RPRPP_CHECK(status);
}

void WRprPpPostProcessing::run(RprPpVkSemaphore aovsReadySemaphore, RprPpVkSemaphore toSignalAfterProcessingSemaphore)
{
    RprPpError status;

    status = rprppPostProcessingRun(m_postProcessing, aovsReadySemaphore, toSignalAfterProcessingSemaphore);
    RPRPP_CHECK(status);
}

void WRprPpPostProcessing::waitQueueIdle()
{
    RprPpError status;

    status = rprppPostProcessingWaitQueueIdle(m_postProcessing);
    RPRPP_CHECK(status);
}

void WRprPpPostProcessing::copyOutputTo(WRprPpImage& dst)
{
    RprPpError status;

    status = rprppPostProcessingCopyOutputToImage(m_postProcessing, dst.get());
    RPRPP_CHECK(status);
}

void WRprPpPostProcessing::copyOutputTo(WRprPpBuffer& dst)
{
    RprPpError status;

    status = rprppPostProcessingCopyOutputToBuffer(m_postProcessing, dst.get());
    RPRPP_CHECK(status);
}

void WRprPpPostProcessing::copyBufferToAovColor(const WRprPpBuffer& src)
{
    RprPpError status;

    status = rprppPostProcessingCopyBufferToAovColor(m_postProcessing, src.get());
    RPRPP_CHECK(status);
}

void WRprPpPostProcessing::copyBufferToAovOpacity(const WRprPpBuffer& src)
{
    RprPpError status;

    status = rprppPostProcessingCopyBufferToAovOpacity(m_postProcessing, src.get());
    RPRPP_CHECK(status);
}

void WRprPpPostProcessing::copyBufferToAovShadowCatcher(const WRprPpBuffer& src)
{
    RprPpError status;

    status = rprppPostProcessingCopyBufferToAovShadowCatcher(m_postProcessing, src.get());
    RPRPP_CHECK(status);
}

void WRprPpPostProcessing::copyBufferToAovReflectionCatcher(const WRprPpBuffer& src)
{
    RprPpError status;

    status = rprppPostProcessingCopyBufferToAovReflectionCatcher(m_postProcessing, src.get());
    RPRPP_CHECK(status);
}

void WRprPpPostProcessing::copyBufferToAovMattePass(const WRprPpBuffer& src)
{
    RprPpError status;

    status = rprppPostProcessingCopyBufferToAovMattePass(m_postProcessing, src.get());
    RPRPP_CHECK(status);
}

void WRprPpPostProcessing::copyBufferToAovBackground(const WRprPpBuffer& src)
{
    RprPpError status;

    status = rprppPostProcessingCopyBufferToAovBackground(m_postProcessing, src.get());
    RPRPP_CHECK(status);
}

void WRprPpPostProcessing::setGamma(float gamma)
{
    RprPpError status;

    status = rprppPostProcessingSetGamma(m_postProcessing, gamma);
    RPRPP_CHECK(status);
}

void WRprPpPostProcessing::setShadowIntensity(float shadowIntensity)
{
    RprPpError status;

    status = rprppPostProcessingSetShadowIntensity(m_postProcessing, shadowIntensity);
    RPRPP_CHECK(status);
}

void WRprPpPostProcessing::setToneMapWhitepoint(float x, float y, float z)
{
    RprPpError status;

    status = rprppPostProcessingSetToneMapWhitepoint(m_postProcessing, x, y, z);
    RPRPP_CHECK(status);
}

void WRprPpPostProcessing::setToneMapVignetting(float vignetting)
{
    RprPpError status;

    status = rprppPostProcessingSetToneMapVignetting(m_postProcessing, vignetting);
    RPRPP_CHECK(status);
}

void WRprPpPostProcessing::setToneMapCrushBlacks(float crushBlacks)
{
    RprPpError status;

    status = rprppPostProcessingSetToneMapCrushBlacks(m_postProcessing, crushBlacks);
    RPRPP_CHECK(status);
}

void WRprPpPostProcessing::setToneMapBurnHighlights(float burnHighlights)
{
    RprPpError status;

    status = rprppPostProcessingSetToneMapBurnHighlights(m_postProcessing, burnHighlights);
    RPRPP_CHECK(status);
}

void WRprPpPostProcessing::setToneMapSaturation(float saturation)
{
    RprPpError status;

    status = rprppPostProcessingSetToneMapSaturation(m_postProcessing, saturation);
    RPRPP_CHECK(status);
}

void WRprPpPostProcessing::setToneMapCm2Factor(float cm2Factor)
{
    RprPpError status;

    status = rprppPostProcessingSetToneMapCm2Factor(m_postProcessing, cm2Factor);
    RPRPP_CHECK(status);
}

void WRprPpPostProcessing::setToneMapFilmIso(float filmIso)
{
    RprPpError status;

    status = rprppPostProcessingSetToneMapFilmIso(m_postProcessing, filmIso);
    RPRPP_CHECK(status);
}

void WRprPpPostProcessing::setToneMapCameraShutter(float cameraShutter)
{
    RprPpError status;

    status = rprppPostProcessingSetToneMapCameraShutter(m_postProcessing, cameraShutter);
    RPRPP_CHECK(status);
}

void WRprPpPostProcessing::setToneMapFNumber(float fNumber)
{
    RprPpError status;

    status = rprppPostProcessingSetToneMapFNumber(m_postProcessing, fNumber);
    RPRPP_CHECK(status);
}

void WRprPpPostProcessing::setToneMapFocalLength(float focalLength)
{
    RprPpError status;

    status = rprppPostProcessingSetToneMapFocalLength(m_postProcessing, focalLength);
    RPRPP_CHECK(status);
}

void WRprPpPostProcessing::setToneMapAperture(float aperture)
{
    RprPpError status;

    status = rprppPostProcessingSetToneMapAperture(m_postProcessing, aperture);
    RPRPP_CHECK(status);
}

void WRprPpPostProcessing::setBloomRadius(float radius)
{
    RprPpError status;

    status = rprppPostProcessingSetBloomRadius(m_postProcessing, radius);
    RPRPP_CHECK(status);
}

void WRprPpPostProcessing::setBloomBrightnessScale(float brightnessScale)
{
    RprPpError status;

    status = rprppPostProcessingSetBloomBrightnessScale(m_postProcessing, brightnessScale);
    RPRPP_CHECK(status);
}

void WRprPpPostProcessing::setBloomThreshold(float threshold)
{
    RprPpError status;

    status = rprppPostProcessingSetBloomThreshold(m_postProcessing, threshold);
    RPRPP_CHECK(status);
}

void WRprPpPostProcessing::setBloomEnabled(bool enabled)
{
    RprPpError status;

    status = rprppPostProcessingSetBloomEnabled(m_postProcessing, enabled ? RPRPP_TRUE : RPRPP_FALSE);
    RPRPP_CHECK(status);
}

void WRprPpPostProcessing::setDenoiserEnabled(bool enabled)
{
    RprPpError status;

    status = rprppPostProcessingSetDenoiserEnabled(m_postProcessing, enabled ? RPRPP_TRUE : RPRPP_FALSE);
    RPRPP_CHECK(status);
}
