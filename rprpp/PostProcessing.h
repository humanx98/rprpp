#pragma once

#include "Image.h"
#include "UniformObjectBuffer.h"
#include "filters/BloomFilter.h"
#include "filters/ComposeColorShadowReflectionFilter.h"
#include "filters/ComposeOpacityShadowFilter.h"
#include "filters/ToneMapFilter.h"
#include "vk/DeviceContext.h"
#include "vk/ShaderManager.h"

#include <memory>
#include <optional>

namespace rprpp {

class PostProcessing {
public:
    PostProcessing(const std::shared_ptr<vk::helper::DeviceContext>& dctx,
        filters::ComposeColorShadowReflectionFilter&& composeColorShadowReflectionFilter,
        filters::ComposeOpacityShadowFilter&& composeOpacityShadowFilter,
        filters::ToneMapFilter&& tonemapFilter,
        filters::BloomFilter&& bloomFilter) noexcept;

    PostProcessing(PostProcessing&&) noexcept = default;
    PostProcessing& operator=(PostProcessing&&) noexcept = default;

    PostProcessing(const PostProcessing&) = delete;
    PostProcessing& operator=(const PostProcessing&) = delete;

    void run(std::optional<vk::Semaphore> aovsReadySemaphore, std::optional<vk::Semaphore> processingFinishedSemaphore);
    void setAovColor(Image* img);
    void setAovOpacity(Image* img);
    void setAovShadowCatcher(Image* img);
    void setAovReflectionCatcher(Image* img);
    void setAovMattePass(Image* img);
    void setAovBackground(Image* img);
    void setOutput(Image* img);

    void setTileOffset(uint32_t x, uint32_t y) noexcept;
    void setGamma(float gamma) noexcept;
    void setShadowIntensity(float shadowIntensity) noexcept;
    void setToneMapWhitepoint(float x, float y, float z) noexcept;
    void setToneMapVignetting(float vignetting) noexcept;
    void setToneMapCrushBlacks(float crushBlacks) noexcept;
    void setToneMapBurnHighlights(float burnHighlights) noexcept;
    void setToneMapSaturation(float saturation) noexcept;
    void setToneMapCm2Factor(float cm2Factor) noexcept;
    void setToneMapFilmIso(float filmIso) noexcept;
    void setToneMapCameraShutter(float cameraShutter) noexcept;
    void setToneMapFNumber(float fNumber) noexcept;
    void setToneMapFocalLength(float focalLength) noexcept;
    void setToneMapAperture(float aperture) noexcept;
    void setBloomRadius(float radius) noexcept;
    void setBloomBrightnessScale(float brightnessScale) noexcept;
    void setBloomThreshold(float threshold) noexcept;
    void setBloomEnabled(bool enabled) noexcept;
    void setDenoiserEnabled(bool enabled) noexcept;

    void getTileOffset(uint32_t& x, uint32_t& y) const noexcept;
    float getGamma() const noexcept;
    float getShadowIntensity() const noexcept;
    void getToneMapWhitepoint(float& x, float& y, float& z) const noexcept;
    float getToneMapVignetting() const noexcept;
    float getToneMapCrushBlacks() const noexcept;
    float getToneMapBurnHighlights() const noexcept;
    float getToneMapSaturation() const noexcept;
    float getToneMapCm2Factor() const noexcept;
    float getToneMapFilmIso() const noexcept;
    float getToneMapCameraShutter() const noexcept;
    float getToneMapFNumber() const noexcept;
    float getToneMapFocalLength() const noexcept;
    float getToneMapAperture() const noexcept;
    float getBloomRadius() const noexcept;
    float getBloomBrightnessScale() const noexcept;
    float getBloomThreshold() const noexcept;
    bool getBloomEnabled() const noexcept;
    bool getDenoiserEnabled() const noexcept;

private:
    bool m_denoiserEnabled = false;
    bool m_bloomEnabled = false;
    std::shared_ptr<vk::helper::DeviceContext> m_dctx;
    filters::ComposeColorShadowReflectionFilter m_composeColorShadowReflectionFilter;
    filters::ComposeOpacityShadowFilter m_composeOpacityShadowFilter;
    filters::ToneMapFilter m_tonemapFilter;
    filters::BloomFilter m_bloomFilter;
};

} // namespace rprpp