#include "BloomFilter.h"
#include "rprpp/Error.h"
#include "rprpp/rprpp.h"
#include "rprpp/vk/DescriptorBuilder.h"
#include "rprpp/Context.h"
#include "rprpp/ImageSimple.h"

constexpr int WorkgroupSize = 32;

namespace rprpp::filters {

BloomFilter::BloomFilter(Context* context) noexcept
    : Filter(context)
    , m_verticalFinishedSemaphore(deviceContext().device.createSemaphore({}))
    , m_horizontalFinishedSemaphore(deviceContext().device.createSemaphore({}))
    , m_ubo(UniformObjectBuffer<filters::BloomParams>(context))
    , m_verticalCommandBuffer(&deviceContext())
    , m_horizontalCommandBuffer(&deviceContext())
{
}

void BloomFilter::validateInputsAndOutput()
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

void BloomFilter::createShaderModules()
{
    const std::unordered_map<std::string, std::string> macroDefinitions = {
        { "OUTPUT_FORMAT", to_glslformat(m_output->description().format) },
        { "INPUT_FORMAT", to_glslformat(m_input->description().format) },
        { "WORKGROUP_SIZE", std::to_string(WorkgroupSize) },
    };
    m_verticalShaderModule = m_shaderManager.getBloomVerticalShader(deviceContext().device, macroDefinitions);
    m_horizontalShaderModule = m_shaderManager.getBloomHorizontalShader(deviceContext().device, macroDefinitions);
}

void BloomFilter::createDescriptorSet()
{
    assert(m_tmpImage);

    vk::helper::DescriptorBuilder builder;
    vk::DescriptorBufferInfo uboDescriptorInfo(m_ubo.buffer(), 0, m_ubo.size()); // binding 0
    builder.bindUniformBuffer(&uboDescriptorInfo);

    vk::DescriptorImageInfo outputDescriptorInfo(nullptr, *m_output->view(), m_output->layout()); // binding 1
    builder.bindStorageImage(&outputDescriptorInfo);

    vk::DescriptorImageInfo tmpImageDescriptorInfo(nullptr, *m_tmpImage->view(), m_tmpImage->layout()); // binding 2
    builder.bindStorageImage(&tmpImageDescriptorInfo);

    vk::DescriptorImageInfo inputDescriptorInfo(nullptr, *m_input->view(), m_input->layout()); // binding 3
    builder.bindStorageImage(&inputDescriptorInfo);

    const std::vector<vk::DescriptorPoolSize>& poolSizes = builder.poolSizes();
    m_descriptorSetLayout = deviceContext().device.createDescriptorSetLayout(vk::DescriptorSetLayoutCreateInfo({}, builder.bindings()));
    m_descriptorPool = deviceContext().device.createDescriptorPool(vk::DescriptorPoolCreateInfo(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet, 1, poolSizes));
    m_descriptorSet = std::move(vk::raii::DescriptorSets(deviceContext().device, vk::DescriptorSetAllocateInfo(*m_descriptorPool.value(), *m_descriptorSetLayout.value())).front());

    builder.updateDescriptorSet(*m_descriptorSet.value());
    deviceContext().device.updateDescriptorSets(builder.writes(), nullptr);
}

void BloomFilter::recordComputeCommandBuffers()
{
    uint32_t x = m_output->description().width;
    uint32_t y = m_output->description().height;

    m_verticalCommandBuffer.get().begin(vk::CommandBufferBeginInfo(vk::CommandBufferUsageFlagBits::eSimultaneousUse));
    m_verticalCommandBuffer.get().bindPipeline(vk::PipelineBindPoint::eCompute, *m_verticalComputePipeline.value());
    m_verticalCommandBuffer.get().bindDescriptorSets(vk::PipelineBindPoint::eCompute, *m_pipelineLayout.value(), 0, *m_descriptorSet.value(), nullptr);
    m_verticalCommandBuffer.get().dispatch((uint32_t)ceil(x / float(WorkgroupSize)), (uint32_t)ceil(y / float(WorkgroupSize)), 1);
    m_verticalCommandBuffer.get().end();

    m_horizontalCommandBuffer.get().begin(vk::CommandBufferBeginInfo(vk::CommandBufferUsageFlagBits::eSimultaneousUse));
    m_horizontalCommandBuffer.get().bindPipeline(vk::PipelineBindPoint::eCompute, *m_horizontalComputePipeline.value());
    m_horizontalCommandBuffer.get().bindDescriptorSets(vk::PipelineBindPoint::eCompute, *m_pipelineLayout.value(), 0, *m_descriptorSet.value(), nullptr);
    m_horizontalCommandBuffer.get().dispatch((uint32_t)ceil(x / float(WorkgroupSize)), (uint32_t)ceil(y / float(WorkgroupSize)), 1);
    m_horizontalCommandBuffer.get().end();
}

void BloomFilter::createComputePipelines()
{
    vk::PipelineLayoutCreateInfo pipelineLayoutInfo({}, *m_descriptorSetLayout.value());
    m_pipelineLayout = vk::raii::PipelineLayout(deviceContext().device, pipelineLayoutInfo);
    // vertical
    {
        vk::PipelineShaderStageCreateInfo shaderStageInfo({}, vk::ShaderStageFlagBits::eCompute, *m_verticalShaderModule.value(), "main");
        vk::ComputePipelineCreateInfo pipelineInfo({}, shaderStageInfo, *m_pipelineLayout.value());
        m_verticalComputePipeline = deviceContext().device.createComputePipeline(nullptr, pipelineInfo);
    }
    // horizontal
    {
        vk::PipelineShaderStageCreateInfo shaderStageInfo({}, vk::ShaderStageFlagBits::eCompute, *m_horizontalShaderModule.value(), "main");
        vk::ComputePipelineCreateInfo pipelineInfo({}, shaderStageInfo, *m_pipelineLayout.value());
        m_horizontalComputePipeline = deviceContext().device.createComputePipeline(nullptr, pipelineInfo);
    }
}

vk::Semaphore BloomFilter::run(std::optional<vk::Semaphore> waitSemaphore)
{
    validateInputsAndOutput();

    if (m_descriptorsDirty) {
        m_verticalComputePipeline.reset();
        m_horizontalComputePipeline.reset();
        m_pipelineLayout.reset();
        m_descriptorSet.reset();
        m_descriptorPool.reset();
        m_descriptorSetLayout.reset();
        m_verticalShaderModule.reset();
        m_horizontalShaderModule.reset();

        if (!m_tmpImage || m_tmpImage->description() != m_input->description()) {
            m_tmpImage.reset();
            ImageDescription desc(m_input->description().width, m_input->description().height, ImageFormat::eR32G32B32A32Sfloat);
            m_tmpImage = std::make_unique<ImageSimple>(context(), desc);
        }

        createShaderModules();
        createDescriptorSet();
        createComputePipelines();
        recordComputeCommandBuffers();
        m_descriptorsDirty = false;
    }

    if (m_ubo.dirty()) {
        m_ubo.data().radiusInPixel = (uint32_t)(m_input->description().width * m_ubo.data().radius);
        m_ubo.update();
    }

    // vertical
    {
        vk::SubmitInfo submitInfo;
        vk::PipelineStageFlags waitStage = vk::PipelineStageFlagBits::eAllCommands;
        if (waitSemaphore.has_value()) {
            submitInfo.setWaitDstStageMask(waitStage);
            submitInfo.setWaitSemaphores(waitSemaphore.value());
        }

        submitInfo.setSignalSemaphores(*m_verticalFinishedSemaphore);
        submitInfo.setCommandBuffers(*m_verticalCommandBuffer.get());
        deviceContext().queue.submit(submitInfo);
    }
    // horizontal
    {
        vk::SubmitInfo submitInfo;
        vk::PipelineStageFlags waitStage = vk::PipelineStageFlagBits::eComputeShader;
        submitInfo.setWaitDstStageMask(waitStage);
        submitInfo.setWaitSemaphores(*m_verticalFinishedSemaphore);

        submitInfo.setSignalSemaphores(*m_horizontalFinishedSemaphore);
        submitInfo.setCommandBuffers(*m_horizontalCommandBuffer.get());
        deviceContext().queue.submit(submitInfo);
    }
    return *m_horizontalFinishedSemaphore;
}

void BloomFilter::setInput(Image* image) noexcept
{
    m_input = image;
    m_descriptorsDirty = true;
}

void BloomFilter::setOutput(Image* image) noexcept
{
    m_output = image;
    m_descriptorsDirty = true;
}

void BloomFilter::setRadius(float radius) noexcept
{
    m_ubo.data().radius = radius;
    m_ubo.markDirty();
}

void BloomFilter::setBrightnessScale(float brightnessScale) noexcept
{
    m_ubo.data().brightnessScale = brightnessScale;
    m_ubo.markDirty();
}

void BloomFilter::setThreshold(float threshold) noexcept
{
    m_ubo.data().threshold = threshold;
    m_ubo.markDirty();
}

float BloomFilter::getRadius() const noexcept
{
    return m_ubo.data().radius;
}

float BloomFilter::getBrightnessScale() const noexcept
{
    return m_ubo.data().brightnessScale;
}

float BloomFilter::getThreshold() const noexcept
{
    return m_ubo.data().threshold;
}

}