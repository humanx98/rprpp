#pragma once

#include "StagingBuffer.h"
#include "rpr_helper.h"

#include <vulkan/vulkan.h>

class RprPostProcessing {
public:
    RprPostProcessing(uint32_t deviceId);
    ~RprPostProcessing();

    StagingBuffer mapStagingBuffer(size_t size);

    void resize(uint32_t width,
        uint32_t height,
        RprPpImageFormat format,
        RprPpDx11Handle outputDx11TextureHandle = nullptr,
        RprPpAovsVkInteropInfo* aovsVkInteropInfo = nullptr);
    void getOutput(uint8_t* dst, size_t size, size_t* retSize);
    void run(RprPpVkSemaphore aovsReadySemaphore = nullptr, RprPpVkSemaphore toSignalAfterProcessingSemaphore = nullptr);

    void waitQueueIdle();
    VkPhysicalDevice getVkPhysicalDevice() const noexcept;
    VkDevice getVkDevice() const noexcept;
    VkQueue getVkQueue() const noexcept;

    void copyStagingBufferToAovColor();
    void copyStagingBufferToAovOpacity();
    void copyStagingBufferToAovShadowCatcher();
    void copyStagingBufferToAovReflectionCatcher();
    void copyStagingBufferToAovMattePass();
    void copyStagingBufferToAovBackground();

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

    RprPostProcessing(const RprPostProcessing&) = delete;
    RprPostProcessing& operator=(const RprPostProcessing&) = delete;

private:
    RprPpContext m_context;
};