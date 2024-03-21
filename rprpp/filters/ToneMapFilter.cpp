#include "ToneMapFilter.h"
#include "rprpp/Error.h"
#include "rprpp/rprpp.h"
#include "rprpp/vk/DescriptorBuilder.h"

constexpr int WorkgroupSize = 32;

namespace rprpp::filters {

ToneMapFilter::ToneMapFilter(vk::helper::DeviceContext* dctx,
    UniformObjectBuffer<ToneMapParams>&& ubo) noexcept
    : m_dctx(dctx)
    , m_finishedSemaphore(dctx->device.createSemaphore({}))
    , m_ubo(std::move(ubo))
    , m_commandBuffer(dctx)
{
}

void ToneMapFilter::createShaderModule()
{
    const std::unordered_map<std::string, std::string> macroDefinitions = {
        { "OUTPUT_FORMAT", to_glslformat(m_output->description().format) },
        { "INPUT_FORMAT", to_glslformat(m_input->description().format) },
        { "WORKGROUP_SIZE", std::to_string(WorkgroupSize) },
    };
    m_shaderModule = m_shaderManager.getToneMapShader(m_dctx->device, macroDefinitions);
}

void ToneMapFilter::createDescriptorSet()
{
    vk::helper::DescriptorBuilder builder;
    vk::DescriptorBufferInfo uboDescriptorInfo(m_ubo.buffer(), 0, m_ubo.size()); // binding 0
    builder.bindUniformBuffer(&uboDescriptorInfo);

    vk::DescriptorImageInfo outputDescriptorInfo(nullptr, m_output->view(), m_output->getLayout()); // binding 1
    builder.bindStorageImage(&outputDescriptorInfo);

    vk::DescriptorImageInfo inputDescriptorInfo(nullptr, m_input->view(), m_input->getLayout()); // binding 2
    builder.bindStorageImage(&inputDescriptorInfo);

    const std::vector<vk::DescriptorPoolSize>& poolSizes = builder.poolSizes();
    m_descriptorSetLayout = m_dctx->device.createDescriptorSetLayout(vk::DescriptorSetLayoutCreateInfo({}, builder.bindings()));
    m_descriptorPool = m_dctx->device.createDescriptorPool(vk::DescriptorPoolCreateInfo(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet, 1, poolSizes));
    m_descriptorSet = std::move(vk::raii::DescriptorSets(m_dctx->device, vk::DescriptorSetAllocateInfo(*m_descriptorPool.value(), *m_descriptorSetLayout.value())).front());

    builder.updateDescriptorSet(*m_descriptorSet.value());
    m_dctx->device.updateDescriptorSets(builder.writes(), nullptr);
}

void ToneMapFilter::recordComputeCommandBuffer()
{
    uint32_t x = m_output->description().width;
    uint32_t y = m_output->description().height;

    m_commandBuffer.get().begin(vk::CommandBufferBeginInfo(vk::CommandBufferUsageFlagBits::eSimultaneousUse));
    m_commandBuffer.get().bindPipeline(vk::PipelineBindPoint::eCompute, *m_computePipeline.value());
    m_commandBuffer.get().bindDescriptorSets(vk::PipelineBindPoint::eCompute, *m_pipelineLayout.value(), 0, *m_descriptorSet.value(), nullptr);
    m_commandBuffer.get().dispatch((uint32_t)ceil(x / float(WorkgroupSize)), (uint32_t)ceil(y / float(WorkgroupSize)), 1);
    m_commandBuffer.get().end();
}

void ToneMapFilter::createComputePipeline()
{
    vk::PipelineLayoutCreateInfo pipelineLayoutInfo({}, *m_descriptorSetLayout.value());
    m_pipelineLayout = vk::raii::PipelineLayout(m_dctx->device, pipelineLayoutInfo);

    vk::PipelineShaderStageCreateInfo shaderStageInfo({}, vk::ShaderStageFlagBits::eCompute, *m_shaderModule.value(), "main");
    vk::ComputePipelineCreateInfo pipelineInfo({}, shaderStageInfo, *m_pipelineLayout.value());
    m_computePipeline = m_dctx->device.createComputePipeline(nullptr, pipelineInfo);
}

void ToneMapFilter::validateInputsAndOutput()
{
    if (m_input == nullptr) {
        throw InvalidParameter("input", "cannot be null");
    }

    if (m_output == nullptr) {
        throw InvalidParameter("output", "cannot be null");
    }

    if (m_input->description().width != m_output->description().width
        || m_input->description().height != m_output->description().height) {
        throw InvalidParameter("output and input", "output and input should have the same image description");
    }

    bool storage = m_input->IsStorage() && m_output->IsStorage();
    if (!storage) {
        throw InvalidParameter("output and input", "output and input images have to be created as storage images");
    }
}

vk::Semaphore ToneMapFilter::run(std::optional<vk::Semaphore> waitSemaphore)
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

void ToneMapFilter::setInput(Image* image) noexcept
{
    m_input = image;
    m_descriptorsDirty = true;
}

void ToneMapFilter::setOutput(Image* image) noexcept
{
    m_output = image;
    m_descriptorsDirty = true;
}

void ToneMapFilter::setGamma(float gamma) noexcept
{
    m_ubo.data().invGamma = 1.0f / (gamma > 0.00001f ? gamma : 1.0f);
    m_ubo.markDirty();
}

void ToneMapFilter::setWhitepoint(float x, float y, float z) noexcept
{
    m_ubo.data().whitepoint[0] = x;
    m_ubo.data().whitepoint[1] = y;
    m_ubo.data().whitepoint[2] = z;
    m_ubo.markDirty();
}

void ToneMapFilter::setVignetting(float vignetting) noexcept
{
    m_ubo.data().vignetting = vignetting;
    m_ubo.markDirty();
}

void ToneMapFilter::setCrushBlacks(float crushBlacks) noexcept
{
    m_ubo.data().crushBlacks = crushBlacks;
    m_ubo.markDirty();
}

void ToneMapFilter::setBurnHighlights(float burnHighlights) noexcept
{
    m_ubo.data().burnHighlights = burnHighlights;
    m_ubo.markDirty();
}

void ToneMapFilter::setSaturation(float saturation) noexcept
{
    m_ubo.data().saturation = saturation;
    m_ubo.markDirty();
}

void ToneMapFilter::setCm2Factor(float cm2Factor) noexcept
{
    m_ubo.data().cm2Factor = cm2Factor;
    m_ubo.markDirty();
}

void ToneMapFilter::setFilmIso(float filmIso) noexcept
{
    m_ubo.data().filmIso = filmIso;
    m_ubo.markDirty();
}

void ToneMapFilter::setCameraShutter(float cameraShutter) noexcept
{
    m_ubo.data().cameraShutter = cameraShutter;
    m_ubo.markDirty();
}

void ToneMapFilter::setFNumber(float fNumber) noexcept
{
    m_ubo.data().fNumber = fNumber;
    m_ubo.markDirty();
}

void ToneMapFilter::setFocalLength(float focalLength) noexcept
{
    m_ubo.data().focalLength = focalLength;
    m_ubo.markDirty();
}

void ToneMapFilter::setAperture(float aperture) noexcept
{
    m_ubo.data().aperture = aperture;
    m_ubo.markDirty();
}

float ToneMapFilter::getGamma() const noexcept
{
    return 1.0f / (m_ubo.data().invGamma > 0.00001f ? m_ubo.data().invGamma : 1.0f);
}

void ToneMapFilter::getWhitepoint(float& x, float& y, float& z) const noexcept
{
    x = m_ubo.data().whitepoint[0];
    y = m_ubo.data().whitepoint[1];
    z = m_ubo.data().whitepoint[2];
}

float ToneMapFilter::getVignetting() const noexcept
{
    return m_ubo.data().vignetting;
}

float ToneMapFilter::getCrushBlacks() const noexcept
{
    return m_ubo.data().crushBlacks;
}

float ToneMapFilter::getBurnHighlights() const noexcept
{
    return m_ubo.data().burnHighlights;
}

float ToneMapFilter::getSaturation() const noexcept
{
    return m_ubo.data().saturation;
}

float ToneMapFilter::getCm2Factor() const noexcept
{
    return m_ubo.data().cm2Factor;
}

float ToneMapFilter::getFilmIso() const noexcept
{
    return m_ubo.data().filmIso;
}

float ToneMapFilter::getCameraShutter() const noexcept
{
    return m_ubo.data().cameraShutter;
}

float ToneMapFilter::getFNumber() const noexcept
{
    return m_ubo.data().fNumber;
}

float ToneMapFilter::getFocalLength() const noexcept
{
    return m_ubo.data().focalLength;
}

float ToneMapFilter::getAperture() const noexcept
{
    return m_ubo.data().aperture;
}

}
