#pragma once

#include "Filter.h"
#include "rprpp/Image.h"
#include "rprpp/UniformObjectBuffer.h"
#include "rprpp/vk/CommandBuffer.h"
#include "rprpp/vk/DeviceContext.h"
#include "rprpp/vk/ShaderManager.h"

#include <memory>
#include <optional>
#include <vector>

namespace rprpp::filters {

struct ToneMapParams {
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
    float aperture = 0.024f;
    float shadowIntensity = 1.0f;
    float invGamma = 1.0f;
};

class ToneMapFilter : public Filter {
public:
    ToneMapFilter(const std::shared_ptr<vk::helper::DeviceContext>& dctx,
        UniformObjectBuffer<ToneMapParams>&& ubo) noexcept;

    ToneMapFilter(ToneMapFilter&&) noexcept = default;
    ToneMapFilter& operator=(ToneMapFilter&&) noexcept = default;

    ToneMapFilter(const ToneMapFilter&) = delete;
    ToneMapFilter& operator=(const ToneMapFilter&) = delete;

    vk::Semaphore run(std::optional<vk::Semaphore> waitSemaphore) override;

    void setInput(Image* img) noexcept override;
    void setOutput(Image* img) noexcept override;

    void setGamma(float gamma) noexcept;
    void setWhitepoint(float x, float y, float z) noexcept;
    void setVignetting(float vignetting) noexcept;
    void setCrushBlacks(float crushBlacks) noexcept;
    void setBurnHighlights(float burnHighlights) noexcept;
    void setSaturation(float saturation) noexcept;
    void setCm2Factor(float cm2Factor) noexcept;
    void setFilmIso(float filmIso) noexcept;
    void setCameraShutter(float cameraShutter) noexcept;
    void setFNumber(float fNumber) noexcept;
    void setFocalLength(float focalLength) noexcept;
    void setAperture(float aperture) noexcept;

    float getGamma() const noexcept;
    void getWhitepoint(float& x, float& y, float& z) const noexcept;
    float getVignetting() const noexcept;
    float getCrushBlacks() const noexcept;
    float getBurnHighlights() const noexcept;
    float getSaturation() const noexcept;
    float getCm2Factor() const noexcept;
    float getFilmIso() const noexcept;
    float getCameraShutter() const noexcept;
    float getFNumber() const noexcept;
    float getFocalLength() const noexcept;
    float getAperture() const noexcept;

private:
    void validateInputsAndOutput();
    void createShaderModule();
    void createDescriptorSet();
    void createComputePipeline();
    void recordComputeCommandBuffer();

    bool m_descriptorsDirty = true;
    Image* m_input = nullptr;
    Image* m_output = nullptr;
    vk::helper::ShaderManager m_shaderManager;
    std::shared_ptr<vk::helper::DeviceContext> m_dctx;
    vk::raii::Semaphore m_finishedSemaphore;
    UniformObjectBuffer<ToneMapParams> m_ubo;
    vk::helper::CommandBuffer m_commandBuffer;
    std::optional<vk::raii::ShaderModule> m_shaderModule;
    std::optional<vk::raii::DescriptorSetLayout> m_descriptorSetLayout;
    std::optional<vk::raii::DescriptorPool> m_descriptorPool;
    std::optional<vk::raii::DescriptorSet> m_descriptorSet;
    std::optional<vk::raii::PipelineLayout> m_pipelineLayout;
    std::optional<vk::raii::Pipeline> m_computePipeline;
};

} // namespace rprpp