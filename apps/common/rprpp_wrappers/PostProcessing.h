#pragma once

#include "Buffer.h"
#include "Image.h"
#include "helper.h"

namespace rprpp::wrappers {

class PostProcessing {
public:
    PostProcessing(const Context& context);
    ~PostProcessing();

    void run(RprPpVkSemaphore aovsReadySemaphore = nullptr, RprPpVkSemaphore toSignalAfterProcessingSemaphore = nullptr);
    void waitQueueIdle();
    void setAovColor(const Image& img);
    void setAovOpacity(const Image& img);
    void setAovShadowCatcher(const Image& img);
    void setAovReflectionCatcher(const Image& img);
    void setAovMattePass(const Image& img);
    void setAovBackground(const Image& img);
    void setOutput(const Image& img);

    void setTileOffset(int x, int y);
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

    PostProcessing(const PostProcessing&) = delete;
    PostProcessing& operator=(const PostProcessing&) = delete;

private:
    RprPpContext m_context;
    RprPpPostProcessing m_postProcessing;
};

}