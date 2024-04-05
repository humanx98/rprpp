#include "ComposeOpacityShadowFilter.h"
#include "rprpp/Error.h"
#include "rprpp/rprpp.h"
#include "rprpp/vk/DescriptorBuilder.h"

constexpr int WorkgroupSize = 32;

namespace rprpp::filters {

constexpr vk::SamplerCreateInfo createSamplerInfo()
{
    vk::SamplerCreateInfo samplerInfo;
    samplerInfo.unnormalizedCoordinates = vk::True;
    samplerInfo.addressModeU = vk::SamplerAddressMode::eClampToEdge;
    samplerInfo.addressModeV = vk::SamplerAddressMode::eClampToEdge;

    return samplerInfo;
}

ComposeOpacityShadowFilter::ComposeOpacityShadowFilter(Context* context) 
    : Filter(context)
    , m_finishedSemaphore(deviceContext().device.createSemaphore({}))
    , m_ubo(context)
    , m_sampler(deviceContext().device, createSamplerInfo())
    , m_commandBuffer(&deviceContext())
{
}

void ComposeOpacityShadowFilter::createShaderModule()
{
    const std::unordered_map<std::string, std::string> macroDefinitions = {
        { "OUTPUT_FORMAT", to_glslformat(m_output->description().format) },
        { "AOVS_FORMAT", to_glslformat(m_aovOpacity->description().format) },
        { "WORKGROUP_SIZE", std::to_string(WorkgroupSize) },
        { "AOVS_ARE_SAMPLED_IMAGES", allAovsAreSampledImages() ? "1" : "0" }
    };
    m_shaderModule = m_shaderManager.getComposeOpacityShadowShader(deviceContext().device, macroDefinitions);
}

void ComposeOpacityShadowFilter::createDescriptorSet()
{
    vk::helper::DescriptorBuilder builder;
    vk::DescriptorBufferInfo uboDescriptorInfo(m_ubo.buffer(), 0, m_ubo.size()); // binding 0
    builder.bindUniformBuffer(&uboDescriptorInfo);

    vk::DescriptorImageInfo outputDescriptorInfo(nullptr, *m_output->view(), m_output->layout()); // binding 1
    builder.bindStorageImage(&outputDescriptorInfo);

    std::vector<Image*> aovs = {
        m_aovOpacity, // binding 2
        m_aovShadowCatcher, // binding 3
    };

    std::vector<vk::DescriptorImageInfo> descriptorImageInfos;
    descriptorImageInfos.reserve(aovs.size());
    for (auto img : aovs) {
        if (allAovsAreStoreImages()) {
            descriptorImageInfos.push_back(vk::DescriptorImageInfo(nullptr, *img->view(), img->layout()));
            builder.bindStorageImage(&descriptorImageInfos.back());
        } else if (allAovsAreSampledImages()) {
            descriptorImageInfos.push_back(vk::DescriptorImageInfo(*m_sampler, *img->view(), img->layout()));
            builder.bindCombinedImageSampler(&descriptorImageInfos.back());
        } else {
            throw InternalError("uknown image type");
        }
    }

    const std::vector<vk::DescriptorPoolSize>& poolSizes = builder.poolSizes();
    m_descriptorSetLayout = deviceContext().device.createDescriptorSetLayout(vk::DescriptorSetLayoutCreateInfo({}, builder.bindings()));
    m_descriptorPool = deviceContext().device.createDescriptorPool(vk::DescriptorPoolCreateInfo(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet, 1, poolSizes));
    m_descriptorSet = std::move(vk::raii::DescriptorSets(deviceContext().device, vk::DescriptorSetAllocateInfo(*m_descriptorPool.value(), *m_descriptorSetLayout.value())).front());

    builder.updateDescriptorSet(*m_descriptorSet.value());
    deviceContext().device.updateDescriptorSets(builder.writes(), nullptr);
}

void ComposeOpacityShadowFilter::recordComputeCommandBuffer()
{
    m_commandBuffer.get().begin(vk::CommandBufferBeginInfo(vk::CommandBufferUsageFlagBits::eSimultaneousUse));
    m_commandBuffer.get().bindPipeline(vk::PipelineBindPoint::eCompute, *m_computePipeline.value());
    m_commandBuffer.get().bindDescriptorSets(vk::PipelineBindPoint::eCompute, *m_pipelineLayout.value(), 0, *m_descriptorSet.value(), nullptr);
    int x = std::min((int)m_output->description().width - m_ubo.data().tileOffset[0], m_ubo.data().tileSize[0]);
    int y = std::min((int)m_output->description().height - m_ubo.data().tileOffset[1], m_ubo.data().tileSize[1]);
    m_commandBuffer.get().dispatch((uint32_t)ceil(x / float(WorkgroupSize)), (uint32_t)ceil(y / float(WorkgroupSize)), 1);
    m_commandBuffer.get().end();
}

void ComposeOpacityShadowFilter::createComputePipeline()
{
    vk::PipelineLayoutCreateInfo pipelineLayoutInfo({}, *m_descriptorSetLayout.value());
    m_pipelineLayout = vk::raii::PipelineLayout(deviceContext().device, pipelineLayoutInfo);

    vk::PipelineShaderStageCreateInfo shaderStageInfo({}, vk::ShaderStageFlagBits::eCompute, *m_shaderModule.value(), "main");
    vk::ComputePipelineCreateInfo pipelineInfo({}, shaderStageInfo, *m_pipelineLayout.value());
    m_computePipeline = deviceContext().device.createComputePipeline(nullptr, pipelineInfo);
}

bool ComposeOpacityShadowFilter::allAovsAreSampledImages() const noexcept
{
    return m_aovOpacity->IsSampled() && m_aovShadowCatcher->IsSampled();
}

bool ComposeOpacityShadowFilter::allAovsAreStoreImages() const noexcept
{
    return m_aovOpacity->IsStorage() && m_aovShadowCatcher->IsStorage();
}

void ComposeOpacityShadowFilter::validateInputsAndOutput()
{
    if (m_aovOpacity == nullptr) {
        throw InvalidParameter("aov opacity", "cannot be null");
    }

    if (m_aovShadowCatcher == nullptr) {
        throw InvalidParameter("aov shadow catcher", "cannot be null");
    }

    if (m_output == nullptr) {
        throw InvalidParameter("aov output", "cannot be null");
    }

    if (m_aovOpacity->description() != m_aovShadowCatcher->description()) {
        throw InvalidParameter("aovs", "all aovs should have the same image description");
    }

    if (!m_output->IsStorage()) {
        throw InvalidParameter("output", "output has to be created as storage images");
    }

    if (!allAovsAreStoreImages() && !allAovsAreSampledImages()) {
        throw InvalidParameter("aovs images", "all aovs images have to be created either as storage images or sampled images");
    }
}

vk::Semaphore ComposeOpacityShadowFilter::run(std::optional<vk::Semaphore> waitSemaphore)
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
    deviceContext().queue.submit(submitInfo);
    return *m_finishedSemaphore;
}

void ComposeOpacityShadowFilter::setInput(Image* img)
{
    m_aovOpacity = img;
    m_descriptorsDirty = true;

    if (img != nullptr) {
        m_ubo.data().tileSize[0] = img->description().width;
        m_ubo.data().tileSize[1] = img->description().height;
        m_ubo.markDirty();
    }
}

void ComposeOpacityShadowFilter::setAovShadowCatcher(Image* img) noexcept
{
    m_aovShadowCatcher = img;
    m_descriptorsDirty = true;
}

void ComposeOpacityShadowFilter::setOutput(Image* img)
{
    m_output = img;
    m_descriptorsDirty = true;
}

void ComposeOpacityShadowFilter::setTileOffset(uint32_t x, uint32_t y) noexcept
{
    m_ubo.data().tileOffset[0] = x;
    m_ubo.data().tileOffset[1] = y;
    m_ubo.markDirty();
}

void ComposeOpacityShadowFilter::setShadowIntensity(float shadowIntensity) noexcept
{
    m_ubo.data().shadowIntensity = shadowIntensity;
    m_ubo.markDirty();
}

void ComposeOpacityShadowFilter::getTileOffset(uint32_t& x, uint32_t& y) const noexcept
{
    x = m_ubo.data().tileOffset[0];
    y = m_ubo.data().tileOffset[1];
}

float ComposeOpacityShadowFilter::getShadowIntensity() const noexcept
{
    return m_ubo.data().shadowIntensity;
}

}
