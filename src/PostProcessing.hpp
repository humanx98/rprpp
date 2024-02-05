#pragma once

#include <filesystem>
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
    uint32_t width;
    uint32_t height;
    vk::AccessFlags access;
    vk::ImageLayout layout;
    vk::PipelineStageFlags stage;
};

struct Aovs {
    BindedImage color;
    BindedImage opacity;
    BindedImage shadowCatcher;
    BindedImage reflectionCatcher;
    BindedImage mattePass;
    BindedImage background;
};

struct ToneMap {
    float whitepoint[3] = { 1.0f, 1.0f, 1.0f };
    float vignetting = 0.0f;
    float crushBlacks = 0.0f;
    float burnHighlights = 1.0f;
    float saturation = 1.0f;
    float cm2Factor = 1.0f;
    float filmIso = 100.0f;
    float cameraShutter = 8.0f;
    float fNumber = 2.0f;
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
};

class PostProcessing {
private:
    uint32_t m_width = 0;
    uint32_t m_height = 0;
    uint32_t m_queueFamilyIndex = 0;
    bool m_uboDirty = true;
    UniformBufferObject m_ubo;
    std::vector<const char*> m_enabledLayers;
    vk::raii::Context m_context;
    std::optional<vk::raii::Instance> m_instance;
    std::optional<vk::raii::DebugUtilsMessengerEXT> m_debugUtilMessenger;
    std::optional<vk::raii::PhysicalDevice> m_physicalDevice;
    std::optional<vk::raii::Device> m_device;
    std::optional<vk::raii::Queue> m_queue;
    std::optional<vk::raii::ShaderModule> m_shaderModule;
    std::optional<vk::raii::CommandPool> m_commandPool;
    std::optional<vk::raii::CommandBuffer> m_secondaryCommandBuffer;
    std::optional<vk::raii::CommandBuffer> m_computeCommandBuffer;
    std::optional<BindedBuffer> m_stagingUboBuffer;
    std::optional<BindedBuffer> m_uboBuffer;
    std::optional<BindedBuffer> m_stagingAovBuffer;
    std::optional<BindedImage> m_outputDx11Texture;
    std::optional<Aovs> m_aovs;
    std::optional<vk::raii::DescriptorSetLayout> m_descriptorSetLayout;
    std::optional<vk::raii::DescriptorPool> m_descriptorPool;
    std::optional<vk::raii::DescriptorSet> m_descriptorSet;
    std::optional<vk::raii::PipelineLayout> m_pipelineLayout;
    std::optional<vk::raii::Pipeline> m_computePipeline;
    void createInstance(bool enableValidationLayers);
    void findPhysicalDevice(uint32_t deviceId);
    uint32_t getComputeQueueFamilyIndex();
    void createDevice();
    void createCommandBuffers();
    void createShaderModule(const std::filesystem::path& shaderPath);
    void createDescriptorSet();
    void createUbo();
    void createAovs(uint32_t width, uint32_t height);
    void createOutputDx11Texture(HANDLE sharedDx11TextureHandle, uint32_t width, uint32_t height);
    void createComputePipeline();
    void recordComputeCommandBuffer(uint32_t width, uint32_t height);
    uint32_t findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties);
    BindedBuffer createBuffer(vk::DeviceSize size,
        vk::BufferUsageFlags usage,
        vk::MemoryPropertyFlags properties);

    BindedImage createImage(uint32_t width,
        uint32_t height,
        vk::Format format,
        vk::ImageUsageFlags usage,
        vk::MemoryPropertyFlags properties);

    void transitionImageLayout(BindedImage& image,
        vk::AccessFlags dstAccess,
        vk::ImageLayout dstLayout,
        vk::PipelineStageFlags dstStage);
    void updateAov(BindedImage& image, rpr_framebuffer rprfb);
    void updateUbo();

public:
    PostProcessing(HANDLE sharedDx11TextureHandle,
        bool enableValidationLayers,
        uint32_t width,
        uint32_t height,
        uint32_t deviceId,
        const std::filesystem::path& shaderPath);
    // PostProcessing(bool enableValidationLayers,
    //     uint32_t width,
    //     uint32_t height,
    //     uint32_t deviceId,
    //     const std::filesystem::path& shaderPath) : PostProcessing(nullptr, enableValidationLayers, width, height, deviceId, shaderPath)
    // {
    // }
    PostProcessing(PostProcessing&&) = default;
    PostProcessing& operator=(PostProcessing&&) = default;
    PostProcessing(PostProcessing&) = delete;
    PostProcessing& operator=(const PostProcessing&) = delete;
    void resize(HANDLE sharedDx11TextureHandle, uint32_t width, uint32_t height);
    void run();

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

    inline void setGamma(float gamma)
    {
        m_ubo.invGamma = 1.0f / (gamma > 0.00001f ? gamma : 1.0f);
        m_uboDirty = true;
    }

    inline void setShadowIntensity(float shadowIntensity)
    {
        m_ubo.shadowIntensity = shadowIntensity;
        m_uboDirty = true;
    }

    inline void setToneMapWhitepoint(float x, float y, float z)
    {
        m_ubo.tonemap.whitepoint[0] = x;
        m_ubo.tonemap.whitepoint[1] = y;
        m_ubo.tonemap.whitepoint[2] = z;
        m_uboDirty = true;
    }

    inline void setToneMapWhitepoint(float vignetting)
    {
        m_ubo.tonemap.vignetting = vignetting;
        m_uboDirty = true;
    }

    inline void setToneMapCrushBlacks(float crushBlacks)
    {
        m_ubo.tonemap.crushBlacks = crushBlacks;
        m_uboDirty = true;
    }

    inline void setToneMapBurnHighlights(float burnHighlights)
    {
        m_ubo.tonemap.burnHighlights = burnHighlights;
        m_uboDirty = true;
    }

    inline void setToneMapSaturation(float saturation)
    {
        m_ubo.tonemap.saturation = saturation;
        m_uboDirty = true;
    }

    inline void setToneMapCm2Factor(float cm2Factor)
    {
        m_ubo.tonemap.cm2Factor = cm2Factor;
        m_uboDirty = true;
    }

    inline void setToneMapFilmIso(float filmIso)
    {
        m_ubo.tonemap.filmIso = filmIso;
        m_uboDirty = true;
    }

    inline void setToneMapCameraShutter(float cameraShutter)
    {
        m_ubo.tonemap.cameraShutter = cameraShutter;
        m_uboDirty = true;
    }

    inline void setToneMapFNumber(float fNumber)
    {
        m_ubo.tonemap.fNumber = fNumber;
        m_uboDirty = true;
    }

    inline void setToneMapFocalLength(float focalLength)
    {
        m_ubo.tonemap.focalLength = focalLength;
        m_uboDirty = true;
    }

    inline void setToneMapAperture(float aperture)
    {
        m_ubo.tonemap.aperture = aperture;
        m_uboDirty = true;
    }

    inline void setBloomRadius(float radius)
    {
        m_ubo.bloom.radius = radius;
        m_uboDirty = true;
    }

    inline void setBloomBrightnessScale(float brightnessScale)
    {
        m_ubo.bloom.brightnessScale = brightnessScale;
        m_uboDirty = true;
    }

    inline void setBloomThreshold(float threshold)
    {
        m_ubo.bloom.threshold = threshold;
        m_uboDirty = true;
    }

    inline void setBloomEnabled(bool enabled)
    {
        m_ubo.bloom.enabled = enabled ? 1 : 0;
        m_uboDirty = true;
    }
};