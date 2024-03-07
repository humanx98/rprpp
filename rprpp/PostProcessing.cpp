#include "PostProcessing.h"
#include "Error.h"
#include "rprpp.h"
#include "vk/DescriptorBuilder.h"

namespace rprpp {

constexpr int WorkgroupSize = 32;
constexpr int NumComponents = 4;

PostProcessing::PostProcessing(const std::shared_ptr<vk::helper::DeviceContext>& dctx, Buffer&& uboBuffer, vk::raii::Sampler&& sampler) noexcept
    : m_dctx(dctx)
    , m_uboBuffer(std::move(uboBuffer))
    , m_sampler(std::move(sampler))
    , m_commandBuffer(dctx->takeCommandBuffer())
{
}

PostProcessing::~PostProcessing()
{
    m_dctx->returnCommandBuffer(std::move(m_commandBuffer));
}

void PostProcessing::createShaderModule()
{
    const std::unordered_map<std::string, std::string> macroDefinitions = {
        { "OUTPUT_FORMAT", to_glslformat(m_output->description().format) },
        { "AOVS_FORMAT", to_glslformat(m_aovColor->description().format) },
        { "WORKGROUP_SIZE", std::to_string(WorkgroupSize) },
        { "AOVS_ARE_SAMPLED_IMAGES", m_aovColor->IsSampled() ? "1" : "0" }
    };
    m_shaderModule = m_shaderManager.get(m_dctx->device, macroDefinitions);
}

void PostProcessing::createDescriptorSet()
{
    vk::helper::DescriptorBuilder builder;
    vk::DescriptorBufferInfo uboDescriptoInfo(m_uboBuffer.get(), 0, sizeof(UniformBufferObject)); // binding 0
    builder.bindUniformBuffer(&uboDescriptoInfo);

    std::vector<Image*> images = {
        m_output, // binding 1
        m_aovColor, // binding 2
        m_aovOpacity, // binding 3
        m_aovShadowCatcher, // binding 4
        m_aovReflectionCatcher, // binding 5
        m_aovMattePass, // binding 6
        m_aovBackground, // binding 7
    };
    std::vector<vk::DescriptorImageInfo> descriptorImageInfos;
    descriptorImageInfos.reserve(images.size());
    for (auto img : images) {
        if (img->IsStorage()) {
            descriptorImageInfos.push_back(vk::DescriptorImageInfo(nullptr, img->view(), img->getLayout()));
            builder.bindStorageImage(&descriptorImageInfos.back());
        } else if (img->IsSampled()) {
            descriptorImageInfos.push_back(vk::DescriptorImageInfo(*m_sampler, img->view(), img->getLayout()));
            builder.bindCombinedImageSampler(&descriptorImageInfos.back());
        } else {
            throw InternalError("uknown image type");
        }
    }

    const std::vector<vk::DescriptorPoolSize>& poolSizes = builder.poolSizes();
    m_descriptorSetLayout = m_dctx->device.createDescriptorSetLayout(vk::DescriptorSetLayoutCreateInfo({}, builder.bindings()));
    m_descriptorPool = m_dctx->device.createDescriptorPool(vk::DescriptorPoolCreateInfo(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet, 1, poolSizes));
    m_descriptorSet = std::move(vk::raii::DescriptorSets(m_dctx->device, vk::DescriptorSetAllocateInfo(*m_descriptorPool.value(), *m_descriptorSetLayout.value())).front());

    builder.updateDescriptorSet(*m_descriptorSet.value());
    m_dctx->device.updateDescriptorSets(builder.writes(), nullptr);
}

void PostProcessing::recordComputeCommandBuffer()
{
    m_commandBuffer.begin(vk::CommandBufferBeginInfo(vk::CommandBufferUsageFlagBits::eSimultaneousUse));
    m_commandBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, *m_computePipeline.value());
    m_commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eCompute, *m_pipelineLayout.value(), 0, *m_descriptorSet.value(), nullptr);
    int x = std::min((int)m_output->description().width - m_ubo.tileOffset[0], m_ubo.tileSize[0]);
    int y = std::min((int)m_output->description().height - m_ubo.tileOffset[1], m_ubo.tileSize[1]);
    m_commandBuffer.dispatch((uint32_t)ceil(x / float(WorkgroupSize)), (uint32_t)ceil(y / float(WorkgroupSize)), 1);
    m_commandBuffer.end();
}

void PostProcessing::createComputePipeline()
{
    vk::PipelineLayoutCreateInfo pipelineLayoutInfo({}, *m_descriptorSetLayout.value());
    m_pipelineLayout = vk::raii::PipelineLayout(m_dctx->device, pipelineLayoutInfo);

    vk::PipelineShaderStageCreateInfo shaderStageInfo({}, vk::ShaderStageFlagBits::eCompute, *m_shaderModule.value(), "main");
    vk::ComputePipelineCreateInfo pipelineInfo({}, shaderStageInfo, *m_pipelineLayout.value());
    m_computePipeline = m_dctx->device.createComputePipeline(nullptr, pipelineInfo);
}

void PostProcessing::validateInputsAndOutput()
{
    if (m_aovColor == nullptr) {
        throw InvalidParameter("aov color", "cannot be null");
    }

    if (m_aovOpacity == nullptr) {
        throw InvalidParameter("aov opacity", "cannot be null");
    }

    if (m_aovShadowCatcher == nullptr) {
        throw InvalidParameter("aov shadow catcher", "cannot be null");
    }

    if (m_aovReflectionCatcher == nullptr) {
        throw InvalidParameter("aov reflection catcher", "cannot be null");
    }

    if (m_aovMattePass == nullptr) {
        throw InvalidParameter("aov matte pass", "cannot be null");
    }

    if (m_aovBackground == nullptr) {
        throw InvalidParameter("aov background", "cannot be null");
    }

    if (m_output == nullptr) {
        throw InvalidParameter("aov output", "cannot be null");
    }

    if (m_aovColor->description() != m_aovOpacity->description()
        || m_aovColor->description() != m_aovShadowCatcher->description()
        || m_aovColor->description() != m_aovReflectionCatcher->description()
        || m_aovColor->description() != m_aovMattePass->description()
        || m_aovColor->description() != m_aovBackground->description()) {
        throw InvalidParameter("aovs", "all aovs should have the same image description");
    }

    if (!m_output->IsStorage()) {
        throw InvalidParameter("output", "output has to be created as storage images");
    }

    bool allStorageAovs = m_aovColor->IsStorage()
        && m_aovOpacity->IsStorage()
        && m_aovShadowCatcher->IsStorage()
        && m_aovReflectionCatcher->IsStorage()
        && m_aovMattePass->IsStorage()
        && m_aovBackground->IsStorage();

    bool allSampledAovs = m_aovColor->IsSampled()
        && m_aovOpacity->IsSampled()
        && m_aovShadowCatcher->IsSampled()
        && m_aovReflectionCatcher->IsSampled()
        && m_aovMattePass->IsSampled()
        && m_aovBackground->IsSampled();

    if (allStorageAovs && allSampledAovs) {
        throw InvalidParameter("aovs images", "all aovs images have to be created either as storage images or sampled images");
    }

    if (!allStorageAovs && !allSampledAovs) {
        throw InvalidParameter("aovs images", "all aovs images have to be created either as storage images or sampled images");
    }
}

void PostProcessing::run(std::optional<vk::Semaphore> aovsReadySemaphore, std::optional<vk::Semaphore> processingFinishedSemaphore)
{
    validateInputsAndOutput();

    if (m_descriptorsDirty) {
        m_computePipeline.reset();
        m_pipelineLayout.reset();
        m_descriptorSet.reset();
        m_descriptorPool.reset();
        m_descriptorSetLayout.reset();
        m_shaderModule.reset();

        createShaderModule();
        createDescriptorSet();
        createComputePipeline();
        m_descriptorsDirty = false;
    }

    if (m_uboDirty) {
        void* data = m_uboBuffer.map(sizeof(UniformBufferObject));
        std::memcpy(data, &m_ubo, sizeof(UniformBufferObject));
        m_uboBuffer.unmap();
        recordComputeCommandBuffer();
        m_uboDirty = false;
    }

    vk::PipelineStageFlags waitStage = vk::PipelineStageFlagBits::eAllCommands;
    vk::SubmitInfo submitInfo;
    if (aovsReadySemaphore.has_value()) {
        submitInfo.setWaitDstStageMask(waitStage);
        submitInfo.setWaitSemaphores(aovsReadySemaphore.value());
    }
    if (processingFinishedSemaphore.has_value()) {
        submitInfo.setSignalSemaphores(processingFinishedSemaphore.value());
    }
    submitInfo.setCommandBuffers(*m_commandBuffer);
    m_dctx->queue.submit(submitInfo);
}

void PostProcessing::setAovColor(Image* img)
{
    m_aovColor = img;
    m_descriptorsDirty = true;

    if (img != nullptr) {
        m_uboDirty = true;
        m_ubo.tileSize[0] = img->description().width;
        m_ubo.tileSize[1] = img->description().height;
    }
}

void PostProcessing::setAovOpacity(Image* img)
{
    m_aovOpacity = img;
    m_descriptorsDirty = true;
}

void PostProcessing::setAovShadowCatcher(Image* img)
{
    m_aovShadowCatcher = img;
    m_descriptorsDirty = true;
}

void PostProcessing::setAovReflectionCatcher(Image* img)
{
    m_aovReflectionCatcher = img;
    m_descriptorsDirty = true;
}

void PostProcessing::setAovMattePass(Image* img)
{
    m_aovMattePass = img;
    m_descriptorsDirty = true;
}

void PostProcessing::setAovBackground(Image* img)
{
    m_aovBackground = img;
    m_descriptorsDirty = true;
}

void PostProcessing::setOutput(Image* img)
{
    m_output = img;
    m_descriptorsDirty = true;
}

void PostProcessing::setGamma(float gamma) noexcept
{
    m_ubo.invGamma = 1.0f / (gamma > 0.00001f ? gamma : 1.0f);
    m_uboDirty = true;
}

void PostProcessing::setShadowIntensity(float shadowIntensity) noexcept
{
    m_ubo.shadowIntensity = shadowIntensity;
    m_uboDirty = true;
}

void PostProcessing::setTileOffset(uint32_t x, uint32_t y) noexcept
{
    m_ubo.tileOffset[0] = x;
    m_ubo.tileOffset[1] = y;
    m_uboDirty = true;
}

void PostProcessing::setToneMapWhitepoint(float x, float y, float z) noexcept
{
    m_ubo.tonemap.whitepoint[0] = x;
    m_ubo.tonemap.whitepoint[1] = y;
    m_ubo.tonemap.whitepoint[2] = z;
    m_uboDirty = true;
}

void PostProcessing::setToneMapVignetting(float vignetting) noexcept
{
    m_ubo.tonemap.vignetting = vignetting;
    m_uboDirty = true;
}

void PostProcessing::setToneMapCrushBlacks(float crushBlacks) noexcept
{
    m_ubo.tonemap.crushBlacks = crushBlacks;
    m_uboDirty = true;
}

void PostProcessing::setToneMapBurnHighlights(float burnHighlights) noexcept
{
    m_ubo.tonemap.burnHighlights = burnHighlights;
    m_uboDirty = true;
}

void PostProcessing::setToneMapSaturation(float saturation) noexcept
{
    m_ubo.tonemap.saturation = saturation;
    m_uboDirty = true;
}

void PostProcessing::setToneMapCm2Factor(float cm2Factor) noexcept
{
    m_ubo.tonemap.cm2Factor = cm2Factor;
    m_uboDirty = true;
}

void PostProcessing::setToneMapFilmIso(float filmIso) noexcept
{
    m_ubo.tonemap.filmIso = filmIso;
    m_uboDirty = true;
}

void PostProcessing::setToneMapCameraShutter(float cameraShutter) noexcept
{
    m_ubo.tonemap.cameraShutter = cameraShutter;
    m_uboDirty = true;
}

void PostProcessing::setToneMapFNumber(float fNumber) noexcept
{
    m_ubo.tonemap.fNumber = fNumber;
    m_uboDirty = true;
}

void PostProcessing::setToneMapFocalLength(float focalLength) noexcept
{
    m_ubo.tonemap.focalLength = focalLength;
    m_uboDirty = true;
}

void PostProcessing::setToneMapAperture(float aperture) noexcept
{
    m_ubo.tonemap.aperture = aperture;
    m_uboDirty = true;
}

void PostProcessing::setBloomRadius(float radius) noexcept
{
    m_ubo.bloom.radius = radius;
    m_uboDirty = true;
}

void PostProcessing::setBloomBrightnessScale(float brightnessScale) noexcept
{
    m_ubo.bloom.brightnessScale = brightnessScale;
    m_uboDirty = true;
}

void PostProcessing::setBloomThreshold(float threshold) noexcept
{
    m_ubo.bloom.threshold = threshold;
    m_uboDirty = true;
}

void PostProcessing::setBloomEnabled(bool enabled) noexcept
{
    m_ubo.bloom.enabled = enabled ? RPRPP_TRUE : RPRPP_FALSE;
    m_uboDirty = true;
}

void PostProcessing::setDenoiserEnabled(bool enabled) noexcept
{
    m_denoiserEnabled = enabled;
}

void PostProcessing::getTileOffset(uint32_t& x, uint32_t& y) noexcept
{
    x = m_ubo.tileOffset[0];
    y = m_ubo.tileOffset[1];
}

float PostProcessing::getGamma() const noexcept
{
    return 1.0f / (m_ubo.invGamma > 0.00001f ? m_ubo.invGamma : 1.0f);
}

float PostProcessing::getShadowIntensity() const noexcept
{
    return m_ubo.shadowIntensity;
}

void PostProcessing::getToneMapWhitepoint(float& x, float& y, float& z) noexcept
{
    x = m_ubo.tonemap.whitepoint[0];
    y = m_ubo.tonemap.whitepoint[1];
    z = m_ubo.tonemap.whitepoint[2];
}

float PostProcessing::getToneMapVignetting() const noexcept
{
    return m_ubo.tonemap.vignetting;
}

float PostProcessing::getToneMapCrushBlacks() const noexcept
{
    return m_ubo.tonemap.crushBlacks;
}

float PostProcessing::getToneMapBurnHighlights() const noexcept
{
    return m_ubo.tonemap.burnHighlights;
}

float PostProcessing::getToneMapSaturation() const noexcept
{
    return m_ubo.tonemap.saturation;
}

float PostProcessing::getToneMapCm2Factor() const noexcept
{
    return m_ubo.tonemap.cm2Factor;
}

float PostProcessing::getToneMapFilmIso() const noexcept
{
    return m_ubo.tonemap.filmIso;
}

float PostProcessing::getToneMapCameraShutter() const noexcept
{
    return m_ubo.tonemap.cameraShutter;
}

float PostProcessing::getToneMapFNumber() const noexcept
{
    return m_ubo.tonemap.fNumber;
}

float PostProcessing::getToneMapFocalLength() const noexcept
{
    return m_ubo.tonemap.focalLength;
}

float PostProcessing::getToneMapAperture() const noexcept
{
    return m_ubo.tonemap.aperture;
}

float PostProcessing::getBloomRadius() const noexcept
{
    return m_ubo.bloom.radius;
}

float PostProcessing::getBloomBrightnessScale() const noexcept
{
    return m_ubo.bloom.brightnessScale;
}

float PostProcessing::getBloomThreshold() const noexcept
{
    return m_ubo.bloom.threshold;
}

bool PostProcessing::getBloomEnabled() const noexcept
{
    return m_ubo.bloom.enabled == RPRPP_TRUE;
}

bool PostProcessing::getDenoiserEnabled() const noexcept
{
    return m_denoiserEnabled;
}

}
