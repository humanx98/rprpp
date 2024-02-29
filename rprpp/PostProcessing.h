#pragma once

#include "Buffer.h"
#include "Image.h"
#include "ImageFormat.h"
#include "ShaderManager.h"
#include "vk_helper.h"

#include <memory>
#include <optional>
#include <variant>
#include <vector>

namespace rprpp {

struct AovsVkInteropInfo {
    VkImage color;
    VkImage opacity;
    VkImage shadowCatcher;
    VkImage reflectionCatcher;
    VkImage mattePass;
    VkImage background;

    friend bool operator==(const AovsVkInteropInfo&, const AovsVkInteropInfo&) = default;
    friend bool operator!=(const AovsVkInteropInfo&, const AovsVkInteropInfo&) = default;
};

struct InteropAovs {
    vk::raii::ImageView color;
    vk::raii::ImageView opacity;
    vk::raii::ImageView shadowCatcher;
    vk::raii::ImageView reflectionCatcher;
    vk::raii::ImageView mattePass;
    vk::raii::ImageView background;
    vk::raii::Sampler sampler;
};

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
};

struct CommandBuffers {
    vk::raii::CommandBuffer compute;
    vk::raii::CommandBuffer secondary;
};

class PostProcessing {
public:
    PostProcessing(const std::shared_ptr<vk::helper::DeviceContext>& dctx,
        vk::raii::CommandPool&& commandPool,
        CommandBuffers&& commandBuffers,
        vk::helper::Buffer&& uboBuffer) noexcept;

    PostProcessing(PostProcessing&&) = default;
    PostProcessing& operator=(PostProcessing&&) = default;

    PostProcessing(PostProcessing&) = delete;
    PostProcessing& operator=(const PostProcessing&) = delete;

    void resize(uint32_t width, uint32_t height, ImageFormat format, std::optional<AovsVkInteropInfo> aovsVkInteropInfo);
    void copyOutputTo(Buffer& dst);
    void copyOutputTo(Image& image);
    void run(std::optional<vk::Semaphore> aovsReadySemaphore, std::optional<vk::Semaphore> toSignalAfterProcessingSemaphore);
    void waitQueueIdle();

    void copyBufferToAovColor(const Buffer& src);
    void copyBufferToAovOpacity(const Buffer& src);
    void copyBufferToAovShadowCatcher(const Buffer& src);
    void copyBufferToAovReflectionCatcher(const Buffer& src);
    void copyBufferToAovMattePass(const Buffer& src);
    void copyBufferToAovBackground(const Buffer& src);

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
    void createShaderModule(ImageFormat outputFormat, bool sampledAovs);
    void createDescriptorSet();
    void createImages(uint32_t width, uint32_t height, ImageFormat outputFormat, std::optional<AovsVkInteropInfo> aovsVkInteropInfo);
    void createComputePipeline();
    void recordComputeCommandBuffer(uint32_t width, uint32_t height);
    void transitionImageLayout(vk::helper::Image& image,
        vk::AccessFlags dstAccess,
        vk::ImageLayout dstLayout,
        vk::PipelineStageFlags dstStage);
    void transitionImageLayout(vk::raii::CommandBuffer& commandBuffer, vk::helper::Image& image,
        vk::AccessFlags dstAccess,
        vk::ImageLayout dstLayout,
        vk::PipelineStageFlags dstStage);
    void copyBufferToAov(const Buffer& src, vk::helper::Image& dst);
    void updateUbo();

    uint32_t m_width = 0;
    uint32_t m_height = 0;
    ImageFormat m_outputImageFormat = ImageFormat::eR32G32B32A32Sfloat;
    std::optional<AovsVkInteropInfo> m_aovsVkInteropInfo;
    bool m_uboDirty = true;
    bool m_denoiserEnabled = false;
    UniformBufferObject m_ubo;
    std::vector<const char*> m_enabledLayers;
    ShaderManager m_shaderManager;
    std::shared_ptr<vk::helper::DeviceContext> m_dctx;
    vk::raii::CommandPool m_commandPool;
    CommandBuffers m_commandBuffers;
    vk::helper::Buffer m_uboBuffer;
    std::optional<vk::raii::ShaderModule> m_shaderModule;
    std::optional<vk::helper::Image> m_outputImage;
    std::optional<std::variant<Aovs, InteropAovs>> m_aovs;
    std::optional<vk::raii::DescriptorSetLayout> m_descriptorSetLayout;
    std::optional<vk::raii::DescriptorPool> m_descriptorPool;
    std::optional<vk::raii::DescriptorSet> m_descriptorSet;
    std::optional<vk::raii::PipelineLayout> m_pipelineLayout;
    std::optional<vk::raii::Pipeline> m_computePipeline;
};

} // namespace rprpp