#pragma once

#include "common.hpp"
#include <RadeonProRender.h>
#include <optional>
#include <vector>
#include <vulkan/vulkan_raii.hpp>
#include <windows.h>

const int WorkgroupSize = 32;

struct BindedBuffer {
    vk::raii::Buffer buffer;
    vk::raii::DeviceMemory memory;
};

struct BindedImage {
    vk::raii::Image image;
    vk::raii::DeviceMemory memory;
    vk::raii::ImageView view;
};

struct Aovs {
    BindedImage color;
    BindedImage opacity;
    BindedImage shadowCatcher;
    BindedImage reflectionCatcher;
    BindedImage mattePass;
    BindedImage background;
};

class PostProcessing {
private:
    bool m_enableValidationLayers = false;
    unsigned int m_width = 0;
    unsigned int m_height = 0;
    uint32_t m_queueFamilyIndex = 0;
    std::vector<const char*> m_enabledLayers;
    vk::raii::Context m_context;
    std::optional<vk::raii::Instance> m_instance;
    std::optional<vk::raii::DebugUtilsMessengerEXT> m_debugUtilMessenger;
    std::optional<vk::raii::PhysicalDevice> m_physicalDevice;
    std::optional<vk::raii::Device> m_device;
    std::optional<vk::raii::Queue> m_queue;
    std::optional<vk::raii::ShaderModule> m_shaderModule;
    std::optional<vk::raii::CommandPool> m_commandPool;
    std::optional<vk::raii::CommandBuffer> m_commandBuffer;
    std::optional<BindedBuffer> m_stagingAovBuffer;
    std::optional<BindedImage> m_outputDx11Texture;
    std::optional<Aovs> m_aovs;
    std::optional<vk::raii::DescriptorSetLayout> m_descriptorSetLayout;
    std::optional<vk::raii::DescriptorPool> m_descriptorPool;
    std::optional<vk::raii::DescriptorSet> m_descriptorSet;
    std::optional<vk::raii::PipelineLayout> m_pipelineLayout;
    std::optional<vk::raii::Pipeline> m_computePipeline;
    void createInstance();
    void findPhysicalDevice(GpuIndices gpuIndices);
    uint32_t getComputeQueueFamilyIndex();
    void createDevice();
    void createCommandBuffer();
    void createShaderModule(const Paths& paths);
    void createDescriptorSet();
    void createAovs();
    void createOutputDx11Texture(HANDLE sharedDx11TextureHandle);
    void createComputePipeline();
    uint32_t findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties);
    BindedBuffer createBuffer(vk::DeviceSize size,
        vk::BufferUsageFlags usage,
        vk::MemoryPropertyFlags properties);

    BindedImage createImage(uint32_t width,
        uint32_t height,
        vk::Format format,
        vk::ImageUsageFlags usage,
        vk::MemoryPropertyFlags properties);

    void transitionImageLayout(const vk::raii::Image& image,
        vk::ImageMemoryBarrier imageMemoryBarrier,
        vk::PipelineStageFlags srcStage,
        vk::PipelineStageFlags dstStage);
    void updateAov(const BindedImage& image, rpr_framebuffer rprfb);

public:
    PostProcessing(const Paths& paths,
        HANDLE sharedDx11TextureHandle,
        bool enableValidationLayers,
        unsigned int width,
        unsigned int height,
        GpuIndices gpuIndices);
    PostProcessing(PostProcessing&&) = default;
    PostProcessing& operator=(PostProcessing&&) = default;
    PostProcessing(PostProcessing&) = delete;
    PostProcessing& operator=(const PostProcessing&) = delete;
    void apply();

    inline VkPhysicalDevice getVkPhysicalDevice() const noexcept
    {
        return static_cast<VkPhysicalDevice>(**m_physicalDevice);
    }

    inline VkDevice getVkDevice() const noexcept
    {
        return static_cast<VkDevice>(**m_device);
    }

    inline void updateAovColor(rpr_framebuffer fb)
    {
        updateAov(m_aovs.value().color, fb);
    }

    inline void updateAovOpacity(rpr_framebuffer fb)
    {
        updateAov(m_aovs.value().opacity, fb);
    }

    inline void updateAovShadowCatcher(rpr_framebuffer fb)
    {
        updateAov(m_aovs.value().shadowCatcher, fb);
    }

    inline void updateAovReflectionCatcher(rpr_framebuffer fb)
    {
        updateAov(m_aovs.value().reflectionCatcher, fb);
    }

    inline void updateAovMattePass(rpr_framebuffer fb)
    {
        updateAov(m_aovs.value().mattePass, fb);
    }

    inline void updateAovBackground(rpr_framebuffer fb)
    {
        updateAov(m_aovs.value().background, fb);
    }
};