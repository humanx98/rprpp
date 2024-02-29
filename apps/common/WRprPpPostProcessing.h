#pragma once

#include "WRprPpBuffer.h"
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
    void copyOutputTo(WRprPpBuffer& dst);

    void copyBufferToAovColor(const WRprPpBuffer& src);
    void copyBufferToAovOpacity(const WRprPpBuffer& src);
    void copyBufferToAovShadowCatcher(const WRprPpBuffer& src);
    void copyBufferToAovReflectionCatcher(const WRprPpBuffer& src);
    void copyBufferToAovMattePass(const WRprPpBuffer& src);
    void copyBufferToAovBackground(const WRprPpBuffer& src);

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