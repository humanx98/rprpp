#pragma once

#include "Filter.h"
#include "rprpp/Image.h"
#include "rprpp/UniformObjectBuffer.h"
#include "rprpp/vk/CommandBuffer.h"
#include "rprpp/vk/DeviceContext.h"
#include "rprpp/vk/ShaderManager.h"

#include <memory>
#include <optional>

// we have a naive, not optimized 2d convolution and it has poor performance
// so instead we use 1d version
// #define USE_2D_CONVOLUTION

namespace rprpp::filters {

struct BloomParams {
    int kernelSize = 0;
    int kernelRadius = 0.0f;
    float intensity = 0.1f;
    float threshold = 0.0f;

    [[nodiscard]] size_t getKernelData2DBufferSizeInBytes() const noexcept
    {
        return kernelSize * kernelSize * sizeof(float);
    }

    [[nodiscard]] size_t getKernelData1DBufferSizeInBytes() const noexcept
    {
        return (kernelRadius + 1) * sizeof(float);
    }
};

class BloomFilter : public Filter {
public:
    explicit BloomFilter(Context* context) noexcept;

    vk::Semaphore run(std::optional<vk::Semaphore> waitSemaphore) override;
    void setInput(Image* img) override;
    void setOutput(Image* img) override;

    void setRadius(float radius) noexcept;
    void setIntensity(float intensity) noexcept;
    void setThreshold(float threshold) noexcept;

    [[nodiscard]] float getRadius() const noexcept;

    [[nodiscard]] float getIntensity() const noexcept;

    [[nodiscard]] float getThreshold() const noexcept;

private:
    void validateInputsAndOutput();
    void createShaderModules();
    void createDescriptorSet();
    void createComputePipelines();
    void recordComputeCommandBuffers();
    void generateGaussianKernel1d();
    void generateGaussianKernel2d();

    bool m_descriptorsDirty = true;
    bool m_kernelDirty = true;
    float m_radius = 0.0f;
    Image* m_input = nullptr;
    Image* m_output = nullptr;

    vk::helper::ShaderManager m_shaderManager;
    vk::raii::Semaphore m_finishedSemaphore;
    UniformObjectBuffer<BloomParams> m_ubo;
    vk::helper::CommandBuffer m_commandBuffer;
    std::unique_ptr<Buffer> m_threshold;
    std::unique_ptr<Buffer> m_tmpBuffer;
    std::unique_ptr<Buffer> m_kernelData;
    std::optional<vk::raii::DescriptorSetLayout> m_descriptorSetLayout;
    std::optional<vk::raii::DescriptorPool> m_descriptorPool;
    std::optional<vk::raii::DescriptorSet> m_descriptorSet;
    std::optional<vk::raii::PipelineLayout> m_pipelineLayout;

    std::optional<vk::raii::ShaderModule> m_thresholdShaderModule;
    std::optional<vk::raii::Pipeline> m_thresholdComputePipeline;
#if defined(USE_2D_CONVOLUTION)
    std::optional<vk::raii::ShaderModule> m_convolve2dShaderModule;
    std::optional<vk::raii::Pipeline> m_convolve2dComputePipeline;
#else
    std::optional<vk::raii::ShaderModule> m_convolve1dVerticalShaderModule;
    std::optional<vk::raii::Pipeline> m_convolve1dVerticalComputePipeline;
    std::optional<vk::raii::ShaderModule> m_convolve1dHorizontalShaderModule;
    std::optional<vk::raii::Pipeline> m_convolve1dHorizontalComputePipeline;
#endif
};

}