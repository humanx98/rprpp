#include "PostProcessing.h"
#include "Error.h"
#include "rprpp.h"
#include "vk/DescriptorBuilder.h"
#include <vector>

namespace rprpp {

constexpr int WorkgroupSize = 32;
constexpr int NumComponents = 4;

PostProcessing::PostProcessing(const std::shared_ptr<vk::helper::DeviceContext>& dctx,
    filters::ComposeColorShadowReflectionFilter&& composeColorShadowReflectionFilter,
    filters::ComposeOpacityShadowFilter&& composeOpacityShadowFilter,
    filters::ToneMapFilter&& tonemapFilter,
    filters::BloomFilter&& bloomFilter) noexcept
    : m_dctx(dctx)
    , m_composeColorShadowReflectionFilter(std::move(composeColorShadowReflectionFilter))
    , m_composeOpacityShadowFilter(std::move(composeOpacityShadowFilter))
    , m_tonemapFilter(std::move(tonemapFilter))
    , m_bloomFilter(std::move(bloomFilter))
{
}

void PostProcessing::run(std::optional<vk::Semaphore> aovsReadySemaphore, std::optional<vk::Semaphore> processingFinishedSemaphore)
{
    vk::Semaphore filterFinished = m_composeColorShadowReflectionFilter.run(aovsReadySemaphore);
    if (m_bloomEnabled) {
        filterFinished = m_bloomFilter.run(filterFinished);
    }
    filterFinished = m_tonemapFilter.run(filterFinished);
    // filterFinished = m_composeOpacityShadowFilter.run(filterFinished);

    vk::PipelineStageFlags waitStage = vk::PipelineStageFlagBits::eComputeShader;
    vk::SubmitInfo submitInfo;
    submitInfo.setWaitDstStageMask(waitStage);
    submitInfo.setWaitSemaphores(filterFinished);

    if (processingFinishedSemaphore.has_value()) {
        submitInfo.setSignalSemaphores(processingFinishedSemaphore.value());
    }
    m_dctx->queue.submit(submitInfo);
}

void PostProcessing::setAovColor(Image* img)
{
    m_composeColorShadowReflectionFilter.setInput(img);
}

void PostProcessing::setAovOpacity(Image* img)
{
    m_composeColorShadowReflectionFilter.setAovOpacity(img);
    m_composeOpacityShadowFilter.setInput(img);
}

void PostProcessing::setAovShadowCatcher(Image* img)
{
    m_composeColorShadowReflectionFilter.setAovShadowCatcher(img);
    m_composeOpacityShadowFilter.setAovShadowCatcher(img);
}

void PostProcessing::setAovReflectionCatcher(Image* img)
{
    m_composeColorShadowReflectionFilter.setAovReflectionCatcher(img);
}

void PostProcessing::setAovMattePass(Image* img)
{
    m_composeColorShadowReflectionFilter.setAovMattePass(img);
}

void PostProcessing::setAovBackground(Image* img)
{
    m_composeColorShadowReflectionFilter.setAovBackground(img);
}

void PostProcessing::setOutput(Image* img)
{
    m_composeColorShadowReflectionFilter.setOutput(img);
    m_composeOpacityShadowFilter.setOutput(img);
    m_tonemapFilter.setInput(img);
    m_tonemapFilter.setOutput(img);
    m_bloomFilter.setInput(img);
    m_bloomFilter.setOutput(img);
}

void PostProcessing::setShadowIntensity(float shadowIntensity) noexcept
{
    m_composeColorShadowReflectionFilter.setShadowIntensity(shadowIntensity);
    m_composeOpacityShadowFilter.setShadowIntensity(shadowIntensity);
}

void PostProcessing::setTileOffset(uint32_t x, uint32_t y) noexcept
{
    m_composeColorShadowReflectionFilter.setTileOffset(x, y);
    m_composeOpacityShadowFilter.setTileOffset(x, y);
}

void PostProcessing::setGamma(float gamma) noexcept
{
    m_tonemapFilter.setGamma(gamma);
}

void PostProcessing::setToneMapWhitepoint(float x, float y, float z) noexcept
{
    m_tonemapFilter.setWhitepoint(x, y, z);
}

void PostProcessing::setToneMapVignetting(float vignetting) noexcept
{
    m_tonemapFilter.setVignetting(vignetting);
}

void PostProcessing::setToneMapCrushBlacks(float crushBlacks) noexcept
{
    m_tonemapFilter.setCrushBlacks(crushBlacks);
}

void PostProcessing::setToneMapBurnHighlights(float burnHighlights) noexcept
{
    m_tonemapFilter.setBurnHighlights(burnHighlights);
}

void PostProcessing::setToneMapSaturation(float saturation) noexcept
{
    m_tonemapFilter.setSaturation(saturation);
}

void PostProcessing::setToneMapCm2Factor(float cm2Factor) noexcept
{
    m_tonemapFilter.setCm2Factor(cm2Factor);
}

void PostProcessing::setToneMapFilmIso(float filmIso) noexcept
{
    m_tonemapFilter.setFilmIso(filmIso);
}

void PostProcessing::setToneMapCameraShutter(float cameraShutter) noexcept
{
    m_tonemapFilter.setCameraShutter(cameraShutter);
}

void PostProcessing::setToneMapFNumber(float fNumber) noexcept
{
    m_tonemapFilter.setFNumber(fNumber);
}

void PostProcessing::setToneMapFocalLength(float focalLength) noexcept
{
    m_tonemapFilter.setFocalLength(focalLength);
}

void PostProcessing::setToneMapAperture(float aperture) noexcept
{
    m_tonemapFilter.setAperture(aperture);
}

void PostProcessing::setBloomRadius(float radius) noexcept
{
    m_bloomFilter.setRadius(radius);
}

void PostProcessing::setBloomBrightnessScale(float brightnessScale) noexcept
{
    m_bloomFilter.setBrightnessScale(brightnessScale);
}

void PostProcessing::setBloomThreshold(float threshold) noexcept
{
    m_bloomFilter.setThreshold(threshold);
}

void PostProcessing::setBloomEnabled(bool enabled) noexcept
{
    m_bloomEnabled = enabled;
}

void PostProcessing::setDenoiserEnabled(bool enabled) noexcept
{
    m_denoiserEnabled = enabled;
}

void PostProcessing::getTileOffset(uint32_t& x, uint32_t& y) const noexcept
{
    m_composeColorShadowReflectionFilter.getTileOffset(x, y);
}

float PostProcessing::getShadowIntensity() const noexcept
{
    return m_composeColorShadowReflectionFilter.getShadowIntensity();
}

float PostProcessing::getGamma() const noexcept
{
    return m_tonemapFilter.getGamma();
}

void PostProcessing::getToneMapWhitepoint(float& x, float& y, float& z) const noexcept
{
    m_tonemapFilter.getWhitepoint(x, y, z);
}

float PostProcessing::getToneMapVignetting() const noexcept
{
    return m_tonemapFilter.getVignetting();
}

float PostProcessing::getToneMapCrushBlacks() const noexcept
{
    return m_tonemapFilter.getCrushBlacks();
}

float PostProcessing::getToneMapBurnHighlights() const noexcept
{
    return m_tonemapFilter.getBurnHighlights();
}

float PostProcessing::getToneMapSaturation() const noexcept
{
    return m_tonemapFilter.getSaturation();
}

float PostProcessing::getToneMapCm2Factor() const noexcept
{
    return m_tonemapFilter.getCm2Factor();
}

float PostProcessing::getToneMapFilmIso() const noexcept
{
    return m_tonemapFilter.getFilmIso();
}

float PostProcessing::getToneMapCameraShutter() const noexcept
{
    return m_tonemapFilter.getCameraShutter();
}

float PostProcessing::getToneMapFNumber() const noexcept
{
    return m_tonemapFilter.getFNumber();
}

float PostProcessing::getToneMapFocalLength() const noexcept
{
    return m_tonemapFilter.getFocalLength();
}

float PostProcessing::getToneMapAperture() const noexcept
{
    return m_tonemapFilter.getAperture();
}

float PostProcessing::getBloomRadius() const noexcept
{
    return m_bloomFilter.getRadius();
}

float PostProcessing::getBloomBrightnessScale() const noexcept
{
    return m_bloomFilter.getBrightnessScale();
}

float PostProcessing::getBloomThreshold() const noexcept
{
    return m_bloomFilter.getThreshold();
}

bool PostProcessing::getBloomEnabled() const noexcept
{
    return m_bloomEnabled;
}

bool PostProcessing::getDenoiserEnabled() const noexcept
{
    return m_denoiserEnabled;
}

}
