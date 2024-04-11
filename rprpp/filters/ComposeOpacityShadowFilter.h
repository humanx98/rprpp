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

struct ComposeOpacityShadowParams {
    int tileOffset[2] = { 0, 0 };
    int tileSize[2] = { 0, 0 };
    float shadowIntensity = 1.0f;
};

class ComposeOpacityShadowFilter : public Filter {
public:
    explicit ComposeOpacityShadowFilter(Context* context);

    vk::Semaphore run(std::optional<vk::Semaphore> waitSemaphore) override;
    void setOutput(Image* img) override;
    void setInput(Image* img) override;
    void setAovShadowCatcher(Image* img) noexcept;

    void setTileOffset(uint32_t x, uint32_t y) noexcept;
    void setShadowIntensity(float shadowIntensity) noexcept;

    void getTileOffset(uint32_t& x, uint32_t& y) const noexcept;
    float getShadowIntensity() const noexcept;

private:
    bool allAovsAreSampledImages() const noexcept;
    bool allAovsAreStoreImages() const noexcept;
    void validateInputsAndOutput();
    void createShaderModule();
    void createDescriptorSet();
    void createComputePipeline();
    void recordComputeCommandBuffer();

    bool m_descriptorsDirty = true;
    Image* m_aovColor = nullptr;
    Image* m_aovOpacity = nullptr;
    Image* m_aovShadowCatcher = nullptr;
    Image* m_aovReflectionCatcher = nullptr;
    Image* m_aovMattePass = nullptr;
    Image* m_aovBackground = nullptr;
    Image* m_output = nullptr;
    vk::helper::ShaderManager m_shaderManager;
    vk::raii::Semaphore m_finishedSemaphore;
    UniformObjectBuffer<ComposeOpacityShadowParams> m_ubo;
    vk::raii::Sampler m_sampler;
    vk::helper::CommandBuffer m_commandBuffer;
    std::optional<vk::raii::ShaderModule> m_shaderModule;
    std::optional<vk::raii::DescriptorSetLayout> m_descriptorSetLayout;
    std::optional<vk::raii::DescriptorPool> m_descriptorPool;
    std::optional<vk::raii::DescriptorSet> m_descriptorSet;
    std::optional<vk::raii::PipelineLayout> m_pipelineLayout;
    std::optional<vk::raii::Pipeline> m_computePipeline;
};

} // namespace rprpp