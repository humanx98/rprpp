#pragma once

#include "ImageFormat.hpp"
#include "ShaderManager.hpp"
#include "vk_helper.hpp"
#include <RadeonProRender.h>
#include <filesystem>
#include <optional>
#include <vector>

namespace rprpp {

const int WorkgroupSize = 32;
const int NumComponents = 4;

struct Aovs {
    vk::helper::Image color;
    vk::helper::Image opacity;
    vk::helper::Image shadowCatcher;
    vk::helper::Image reflectionCatcher;
    vk::helper::Image mattePass;
    vk::helper::Image background;
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
    HANDLE m_sharedDx11TextureHandle = nullptr;
    uint32_t m_width = 0;
    uint32_t m_height = 0;
    ImageFormat m_outputImageFormat = ImageFormat::eR32G32B32A32Sfloat;
    bool m_uboDirty = true;
    UniformBufferObject m_ubo;
    std::vector<const char*> m_enabledLayers;
    std::filesystem::path m_shaderPath;
    ShaderManager m_shaderManager;
    vk::helper::DeviceContext m_dctx;
    vk::raii::CommandPool m_commandPool;
    vk::raii::CommandBuffer m_secondaryCommandBuffer;
    vk::raii::CommandBuffer m_computeCommandBuffer;
    vk::helper::Buffer m_uboBuffer;
    std::optional<vk::raii::ShaderModule> m_shaderModule;
    std::optional<vk::helper::Buffer> m_stagingBuffer;
    std::optional<vk::helper::Image> m_outputImage;
    std::optional<Aovs> m_aovs;
    std::optional<vk::raii::DescriptorSetLayout> m_descriptorSetLayout;
    std::optional<vk::raii::DescriptorPool> m_descriptorPool;
    std::optional<vk::raii::DescriptorSet> m_descriptorSet;
    std::optional<vk::raii::PipelineLayout> m_pipelineLayout;
    std::optional<vk::raii::Pipeline> m_computePipeline;
    void createShaderModule(ImageFormat outputFormat);
    void createDescriptorSet();
    void createImages(uint32_t width, uint32_t height, ImageFormat outputFormat, HANDLE sharedDx11TextureHandle);
    void createComputePipeline();
    void recordComputeCommandBuffer(uint32_t width, uint32_t height);
    void transitionImageLayout(vk::helper::Image& image,
        vk::AccessFlags dstAccess,
        vk::ImageLayout dstLayout,
        vk::PipelineStageFlags dstStage);
    void updateAov(vk::helper::Image& image, rpr_framebuffer rprfb);
    void updateUbo();

public:
    PostProcessing(vk::helper::DeviceContext dctx,
        vk::raii::CommandPool commandPool,
        vk::raii::CommandBuffer secondaryCommandBuffer,
        vk::raii::CommandBuffer computeCommandBuffer,
        vk::helper::Buffer uboBuffer,
        const std::filesystem::path& shaderPath);
    PostProcessing(PostProcessing&&) = default;
    PostProcessing& operator=(PostProcessing&&) = default;
    PostProcessing(PostProcessing&) = delete;
    PostProcessing& operator=(const PostProcessing&) = delete;
    static PostProcessing create(bool enableValidationLayers, uint32_t deviceId, const std::filesystem::path& shaderPath);
    void resize(uint32_t width, uint32_t height, ImageFormat format, HANDLE sharedDx11TextureHandle = nullptr);
    void getOutput(uint8_t* dst, size_t size, size_t* retSize);
    void run();

    inline VkPhysicalDevice getVkPhysicalDevice() const noexcept
    {
        return static_cast<VkPhysicalDevice>(*m_dctx.physicalDevice);
    }

    inline VkDevice getVkDevice() const noexcept
    {
        return static_cast<VkDevice>(*m_dctx.device);
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

}