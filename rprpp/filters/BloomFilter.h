#pragma once

#include "Filter.h"
#include "rprpp/Image.h"
#include "rprpp/UniformObjectBuffer.h"
#include "rprpp/vk/CommandBuffer.h"
#include "rprpp/vk/DeviceContext.h"
#include "rprpp/vk/ShaderManager.h"

#include <memory>
#include <optional>

namespace rprpp::filters {

struct BloomParams {
    int radiusInPixel = 0;
    float radius = 0.0f;
    float brightnessScale = 0.1f;
    float threshold = 0.0f;
};

class BloomFilter : public Filter {
public:
    explicit BloomFilter(Context* context) noexcept;

    vk::Semaphore run(std::optional<vk::Semaphore> waitSemaphore) override;
    void setInput(Image* img) override;
    void setOutput(Image* img) override;

    void setRadius(float radius) noexcept;
    void setBrightnessScale(float brightnessScale) noexcept;
    void setThreshold(float threshold) noexcept;

    [[nodiscard]]
    float getRadius() const noexcept;

    [[nodiscard]]
    float getBrightnessScale() const noexcept;

    [[nodiscard]]
    float getThreshold() const noexcept;

private:
    void validateInputsAndOutput();
    void createShaderModules();
    void createDescriptorSet();
    void createComputePipelines();
    void recordComputeCommandBuffers();

    bool m_descriptorsDirty = true;
    Image* m_input = nullptr;
    Image* m_output = nullptr;

    vk::helper::ShaderManager m_shaderManager;
    vk::raii::Semaphore m_verticalFinishedSemaphore;
    vk::raii::Semaphore m_horizontalFinishedSemaphore;
    UniformObjectBuffer<BloomParams> m_ubo;
    vk::helper::CommandBuffer m_verticalCommandBuffer;
    vk::helper::CommandBuffer m_horizontalCommandBuffer;
    std::unique_ptr<Image> m_tmpImage;
    std::optional<vk::raii::ShaderModule> m_verticalShaderModule;
    std::optional<vk::raii::ShaderModule> m_horizontalShaderModule;
    std::optional<vk::raii::DescriptorSetLayout> m_descriptorSetLayout;
    std::optional<vk::raii::DescriptorPool> m_descriptorPool;
    std::optional<vk::raii::DescriptorSet> m_descriptorSet;
    std::optional<vk::raii::PipelineLayout> m_pipelineLayout;
    std::optional<vk::raii::Pipeline> m_verticalComputePipeline;
    std::optional<vk::raii::Pipeline> m_horizontalComputePipeline;
};

}