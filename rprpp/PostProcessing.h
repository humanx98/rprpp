#pragma once

#include "Buffer.h"
#include "Image.h"
#include "ImageFormat.h"
#include "vk/DeviceContext.h"
#include "vk/ShaderManager.h"

#include <memory>
#include <optional>
#include <vector>

namespace rprpp {

struct ToneMap {
    float whitepoint[3] = { 1.0f, 1.0f, 1.0f };
    float vignetting = 0.0f;
    float crushBlacks = 0.0f;
    float burnHighlights = 1.0f;
    float saturation = 1.0f;
    float cm2Factor = 1.0f;
    float filmIso = 100.0f;
    float cameraShutter = 1.0f;
    float fNumber = 1.0f;
    float focalLength = 1.0f;
    float aperture = 0.024f; // hardcoded in swviz
    // paddings, needed to make a correct memory allignment
    float _padding0; // int enabled; TODO: probably we won't need this field
    float _padding1;
    float _padding2;
};

struct Bloom {
    float radius;
    float brightnessScale;
    float threshold;
    int enabled = 0;
};

struct UniformBufferObject {
    ToneMap tonemap;
    Bloom bloom;
    float shadowIntensity = 1.0f;
    float invGamma = 1.0f;
    int tileOffsetX = 0;
    int tileOffsetY = 0;
};

class PostProcessing {
public:
    PostProcessing(const std::shared_ptr<vk::helper::DeviceContext>& dctx, Buffer&& uboBuffer, vk::raii::Sampler&& sampler) noexcept;
    ~PostProcessing();

    PostProcessing(PostProcessing&&) = default;
    PostProcessing& operator=(PostProcessing&&) = default;

    PostProcessing(PostProcessing&) = delete;
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

    float getGamma() const noexcept;
    float getShadowIntensity() const noexcept;
    void getToneMapWhitepoint(float& x, float& y, float& z) noexcept;
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
    void validateInputsAndOutput();
    void createShaderModule();
    void createDescriptorSet();
    void createComputePipeline();
    void recordComputeCommandBuffer();

    bool m_uboDirty = true;
    bool m_denoiserEnabled = false;
    UniformBufferObject m_ubo;
    bool m_descriptorsDirty = true;
    Image* m_aovColor;
    Image* m_aovOpacity;
    Image* m_aovShadowCatcher;
    Image* m_aovReflectionCatcher;
    Image* m_aovMattePass;
    Image* m_aovBackground;
    Image* m_output;
    vk::helper::ShaderManager m_shaderManager;
    std::shared_ptr<vk::helper::DeviceContext> m_dctx;
    Buffer m_uboBuffer;
    vk::raii::Sampler m_sampler;
    vk::raii::CommandBuffer m_commandBuffer;
    std::optional<vk::raii::ShaderModule> m_shaderModule;
    std::optional<vk::raii::DescriptorSetLayout> m_descriptorSetLayout;
    std::optional<vk::raii::DescriptorPool> m_descriptorPool;
    std::optional<vk::raii::DescriptorSet> m_descriptorSet;
    std::optional<vk::raii::PipelineLayout> m_pipelineLayout;
    std::optional<vk::raii::Pipeline> m_computePipeline;
};

} // namespace rprpp