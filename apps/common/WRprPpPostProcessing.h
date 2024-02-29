#pragma once

#include "WRprPpHostVisibleBuffer.h"
#include "WRprPpImage.h"
#include "rpr_helper.h"

class WRprPpPostProcessing {
public:
    WRprPpPostProcessing(const WRprPpContext& context);
    ~WRprPpPostProcessing();

    void resize(uint32_t width, uint32_t height, RprPpImageFormat format, RprPpAovsVkInteropInfo* aovsVkInteropInfo = nullptr);
    void run(RprPpVkSemaphore aovsReadySemaphore = nullptr, RprPpVkSemaphore toSignalAfterProcessingSemaphore = nullptr);
    void waitQueueIdle();
    void copyOutputTo(WRprPpImage& dst);
    void copyOutputTo(WRprPpHostVisibleBuffer& dst);

    void copyBufferToAovColor(const WRprPpHostVisibleBuffer& src);
    void copyBufferToAovOpacity(const WRprPpHostVisibleBuffer& src);
    void copyBufferToAovShadowCatcher(const WRprPpHostVisibleBuffer& src);
    void copyBufferToAovReflectionCatcher(const WRprPpHostVisibleBuffer& src);
    void copyBufferToAovMattePass(const WRprPpHostVisibleBuffer& src);
    void copyBufferToAovBackground(const WRprPpHostVisibleBuffer& src);

    void setGamma(float gamma);
    void setShadowIntensity(float shadowIntensity);
    void setToneMapWhitepoint(float x, float y, float z);
    void setToneMapVignetting(float vignetting);
    void setToneMapCrushBlacks(float crushBlacks);
    void setToneMapBurnHighlights(float burnHighlights);
    void setToneMapSaturation(float saturation);
    void setToneMapCm2Factor(float cm2Factor);
    void setToneMapFilmIso(float filmIso);
    void setToneMapCameraShutter(float cameraShutter);
    void setToneMapFNumber(float fNumber);
    void setToneMapFocalLength(float focalLength);
    void setToneMapAperture(float aperture);
    void setBloomRadius(float radius);
    void setBloomBrightnessScale(float brightnessScale);
    void setBloomThreshold(float threshold);
    void setBloomEnabled(bool enabled);
    void setDenoiserEnabled(bool enabled);

    WRprPpPostProcessing(const WRprPpPostProcessing&) = delete;
    WRprPpPostProcessing& operator=(const WRprPpPostProcessing&) = delete;

private:
    RprPpContext m_context;
    RprPpPostProcessing m_postProcessing;
};