#include "ComposeColorShadowReflectionFilter.h"
#include "rprpp/Error.h"
#include "rprpp/rprpp.h"
#include "rprpp/vk/DescriptorBuilder.h"

constexpr int WorkgroupSize = 32;

namespace rprpp::filters {

ComposeColorShadowReflectionFilter::ComposeColorShadowReflectionFilter(vk::helper::DeviceContext* dctx,
    UniformObjectBuffer<ComposeColorShadowReflectionParams>&& ubo,
    vk::raii::Sampler&& sampler) noexcept
    : m_dctx(dctx)
    , m_finishedSemaphore(dctx->device.createSemaphore({}))
    , m_ubo(std::move(ubo))
    , m_sampler(std::move(sampler))
    , m_commandBuffer(dctx)
{
}

void ComposeColorShadowReflectionFilter::createShaderModule()
{
    const std::unordered_map<std::string, std::string> macroDefinitions = {
        { "OUTPUT_FORMAT", to_glslformat(m_output->description().format) },
        { "AOVS_FORMAT", to_glslformat(m_aovColor->description().format) },
        { "WORKGROUP_SIZE", std::to_string(WorkgroupSize) },
        { "AOVS_ARE_SAMPLED_IMAGES", allAovsAreSampledImages() ? "1" : "0" }
    };
    m_shaderModule = m_shaderManager.getComposeColorShadowReflectionShader(m_dctx->device, macroDefinitions);
}

void ComposeColorShadowReflectionFilter::createDescriptorSet()
{
    vk::helper::DescriptorBuilder builder;
    vk::DescriptorBufferInfo uboDescriptorInfo(m_ubo.buffer(), 0, m_ubo.size()); // binding 0
    builder.bindUniformBuffer(&uboDescriptorInfo);

    vk::DescriptorImageInfo outputDescriptorInfo(nullptr, m_output->view(), m_output->getLayout()); // binding 1
    builder.bindStorageImage(&outputDescriptorInfo);

    std::vector<Image*> aovs = {
        m_aovColor, // binding 2
        m_aovOpacity, // binding 3
        m_aovShadowCatcher, // binding 4
        m_aovReflectionCatcher, // binding 5
        m_aovMattePass, // binding 6
        m_aovBackground, // binding 7
    };

    std::vector<vk::DescriptorImageInfo> descriptorImageInfos;
    descriptorImageInfos.reserve(aovs.size());
    for (auto img : aovs) {
        if (allAovsAreStoreImages()) {
            descriptorImageInfos.push_back(vk::DescriptorImageInfo(nullptr, img->view(), img->getLayout()));
            builder.bindStorageImage(&descriptorImageInfos.back());
        } else if (allAovsAreSampledImages()) {
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

void ComposeColorShadowReflectionFilter::recordComputeCommandBuffer()
{
    m_commandBuffer.get().begin(vk::CommandBufferBeginInfo(vk::CommandBufferUsageFlagBits::eSimultaneousUse));
    m_commandBuffer.get().bindPipeline(vk::PipelineBindPoint::eCompute, *m_computePipeline.value());
    m_commandBuffer.get().bindDescriptorSets(vk::PipelineBindPoint::eCompute, *m_pipelineLayout.value(), 0, *m_descriptorSet.value(), nullptr);
    int x = std::min((int)m_output->description().width - m_ubo.data().tileOffset[0], m_ubo.data().tileSize[0]);
    int y = std::min((int)m_output->description().height - m_ubo.data().tileOffset[1], m_ubo.data().tileSize[1]);
    m_commandBuffer.get().dispatch((uint32_t)ceil(x / float(WorkgroupSize)), (uint32_t)ceil(y / float(WorkgroupSize)), 1);
    m_commandBuffer.get().end();
}

void ComposeColorShadowReflectionFilter::createComputePipeline()
{
    vk::PipelineLayoutCreateInfo pipelineLayoutInfo({}, *m_descriptorSetLayout.value());
    m_pipelineLayout = vk::raii::PipelineLayout(m_dctx->device, pipelineLayoutInfo);

    vk::PipelineShaderStageCreateInfo shaderStageInfo({}, vk::ShaderStageFlagBits::eCompute, *m_shaderModule.value(), "main");
    vk::ComputePipelineCreateInfo pipelineInfo({}, shaderStageInfo, *m_pipelineLayout.value());
    m_computePipeline = m_dctx->device.createComputePipeline(nullptr, pipelineInfo);
}

bool ComposeColorShadowReflectionFilter::allAovsAreSampledImages() const noexcept
{
    return m_aovColor->IsSampled()
        && m_aovOpacity->IsSampled()
        && m_aovShadowCatcher->IsSampled()
        && m_aovReflectionCatcher->IsSampled()
        && m_aovMattePass->IsSampled()
        && m_aovBackground->IsSampled();
}

bool ComposeColorShadowReflectionFilter::allAovsAreStoreImages() const noexcept
{
    return m_aovColor->IsStorage()
        && m_aovOpacity->IsStorage()
        && m_aovShadowCatcher->IsStorage()
        && m_aovReflectionCatcher->IsStorage()
        && m_aovMattePass->IsStorage()
        && m_aovBackground->IsStorage();
}

void ComposeColorShadowReflectionFilter::validateInputsAndOutput()
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

    if (!allAovsAreStoreImages() && !allAovsAreSampledImages()) {
        throw InvalidParameter("aovs images", "all aovs images have to be created either as storage images or sampled images");
    }
}

vk::Semaphore ComposeColorShadowReflectionFilter::run(std::optional<vk::Semaphore> waitSemaphore)
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

    if (m_ubo.dirty()) {
        m_ubo.update();
        recordComputeCommandBuffer();
    }

    vk::PipelineStageFlags waitStage = vk::PipelineStageFlagBits::eAllCommands;
    vk::SubmitInfo submitInfo;
    if (waitSemaphore.has_value()) {
        submitInfo.setWaitDstStageMask(waitStage);
        submitInfo.setWaitSemaphores(waitSemaphore.value());
    }

    submitInfo.setSignalSemaphores(*m_finishedSemaphore);
    submitInfo.setCommandBuffers(*m_commandBuffer.get());
    m_dctx->queue.submit(submitInfo);
    return *m_finishedSemaphore;
}

void ComposeColorShadowReflectionFilter::setInput(Image* img) noexcept
{
    m_aovColor = img;
    m_descriptorsDirty = true;

    if (img != nullptr) {
        m_ubo.data().tileSize[0] = img->description().width;
        m_ubo.data().tileSize[1] = img->description().height;
        m_ubo.markDirty();
    }
}

void ComposeColorShadowReflectionFilter::setAovOpacity(Image* img) noexcept
{
    m_aovOpacity = img;
    m_descriptorsDirty = true;
}

void ComposeColorShadowReflectionFilter::setAovShadowCatcher(Image* img) noexcept
{
    m_aovShadowCatcher = img;
    m_descriptorsDirty = true;
}

void ComposeColorShadowReflectionFilter::setAovReflectionCatcher(Image* img) noexcept
{
    m_aovReflectionCatcher = img;
    m_descriptorsDirty = true;
}

void ComposeColorShadowReflectionFilter::setAovMattePass(Image* img) noexcept
{
    m_aovMattePass = img;
    m_descriptorsDirty = true;
}

void ComposeColorShadowReflectionFilter::setAovBackground(Image* img) noexcept
{
    m_aovBackground = img;
    m_descriptorsDirty = true;
}

void ComposeColorShadowReflectionFilter::setOutput(Image* img) noexcept
{
    m_output = img;
    m_descriptorsDirty = true;
}

void ComposeColorShadowReflectionFilter::setShadowIntensity(float shadowIntensity) noexcept
{
    m_ubo.data().shadowIntensity = shadowIntensity;
    m_ubo.markDirty();
}

void ComposeColorShadowReflectionFilter::setTileOffset(uint32_t x, uint32_t y) noexcept
{
    m_ubo.data().tileOffset[0] = x;
    m_ubo.data().tileOffset[1] = y;
    m_ubo.markDirty();
}

void ComposeColorShadowReflectionFilter::setNotRefractiveBackgroundColor(float x, float y, float z)
{
    m_ubo.data().notRefractiveBackgroundColor[0] = x;
    m_ubo.data().notRefractiveBackgroundColor[1] = y;
    m_ubo.data().notRefractiveBackgroundColor[2] = z;
    m_ubo.markDirty();
}

void ComposeColorShadowReflectionFilter::setNotRefractiveBackgroundColorWeight(float weight)
{
    m_ubo.data().notRefractiveBackgroundColorWeight = weight;
    m_ubo.markDirty();
}

float ComposeColorShadowReflectionFilter::getNotRefractiveBackgroundColorWeight()
{
    return m_ubo.data().notRefractiveBackgroundColorWeight;
}

void ComposeColorShadowReflectionFilter::getNotRefractiveBackgroundColor(float& x, float& y, float& z)
{
    x = m_ubo.data().notRefractiveBackgroundColor[0];
    y = m_ubo.data().notRefractiveBackgroundColor[1];
    z = m_ubo.data().notRefractiveBackgroundColor[2];
}

void ComposeColorShadowReflectionFilter::getTileOffset(uint32_t& x, uint32_t& y) const noexcept
{
    x = m_ubo.data().tileOffset[0];
    y = m_ubo.data().tileOffset[1];
}

float ComposeColorShadowReflectionFilter::getShadowIntensity() const noexcept
{
    return m_ubo.data().shadowIntensity;
}

}
