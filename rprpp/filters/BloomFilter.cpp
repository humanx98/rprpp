#include "BloomFilter.h"
#include "rprpp/Context.h"
#include "rprpp/Error.h"
#include "rprpp/ImageSimple.h"
#include "rprpp/rprpp.h"
#include "rprpp/vk/DescriptorBuilder.h"

constexpr int WorkgroupSize = 1024;

namespace rprpp::filters {

static int gaussianKernelDataSigma(const ImageDescription& description, float radius)
{
    float diagonal = sqrtf(description.width * description.width + description.height * description.height);
    return diagonal * std::min(1.f, radius);
}

static int gaussianKernelDataSize(const ImageDescription& description, float radius)
{
    float sigma = gaussianKernelDataSigma(description, radius);
    int kernelCenterOffset = std::max(1, int(4.0f * sigma + /*rounding*/ 0.5f));
    return kernelCenterOffset * 2 + 1;
}

BloomFilter::BloomFilter(Context* context) noexcept
    : Filter(context)
    , m_finishedSemaphore(deviceContext().device.createSemaphore({}))
    , m_ubo(UniformObjectBuffer<filters::BloomParams>(context))
    , m_commandBuffer(&deviceContext())
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

    if (m_input->description() != m_output->description()) {
        throw InvalidParameter("output and input", "output and input should have the same image description");
    }

    bool storage = m_input->IsStorage() && m_output->IsStorage();
    if (!storage) {
        throw InvalidParameter("output and input", "output and input images have to be created as storage images");
    }
}

void BloomFilter::createShaderModules()
{
    std::unordered_map<std::string, std::string> macroDefinitions = {
        { "OUTPUT_FORMAT", to_glslformat(m_output->description().format) },
        { "INPUT_FORMAT", to_glslformat(m_input->description().format) },
        { "WORKGROUP_SIZE", std::to_string(WorkgroupSize) },
    };
    m_thresholdShaderModule = m_shaderManager.getBloomThresholdShader(deviceContext().device, macroDefinitions);

#if defined(USE_2D_CONVOLUTION)
    m_convolve2dShaderModule = m_shaderManager.getBloomConvolve2dShader(deviceContext().device, macroDefinitions);
#else
    m_convolve1dVerticalShaderModule = m_shaderManager.getBloomConvolve1dShader(deviceContext().device, macroDefinitions);
    macroDefinitions["HORIZONTAL"] = "";
    m_convolve1dHorizontalShaderModule = m_shaderManager.getBloomConvolve1dShader(deviceContext().device, macroDefinitions);
#endif
}

void BloomFilter::createDescriptorSet()
{
    assert(m_threshold);
    assert(m_kernelData);
    assert(m_tmpBuffer);

    vk::helper::DescriptorBuilder builder;
    vk::DescriptorBufferInfo uboDescriptorInfo(m_ubo.buffer(), 0, m_ubo.size()); // binding 0
    builder.bindUniformBuffer(&uboDescriptorInfo);

    vk::DescriptorBufferInfo kernelDataDescriptorInfo(m_kernelData->get(), 0, m_kernelData->size()); // binding 1
    builder.bindStorageBuffer(&kernelDataDescriptorInfo);

    vk::DescriptorBufferInfo tmpBufferDescriptorInfo(m_tmpBuffer->get(), 0, m_tmpBuffer->size()); // binding 2
    builder.bindStorageBuffer(&tmpBufferDescriptorInfo);

    vk::DescriptorBufferInfo thresholdDescriptorInfo(m_threshold->get(), 0, m_threshold->size()); // binding 3
    builder.bindStorageBuffer(&thresholdDescriptorInfo);

    vk::DescriptorImageInfo outputDescriptorInfo(nullptr, *m_output->view(), m_output->layout()); // binding 4
    builder.bindStorageImage(&outputDescriptorInfo);

    vk::DescriptorImageInfo inputDescriptorInfo(nullptr, *m_input->view(), m_input->layout()); // binding 5
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
    uint32_t pixelsCount = m_output->description().width * m_output->description().height;

    m_commandBuffer.get().begin(vk::CommandBufferBeginInfo(vk::CommandBufferUsageFlagBits::eSimultaneousUse));
    {
        m_commandBuffer.get().bindPipeline(vk::PipelineBindPoint::eCompute, *m_thresholdComputePipeline.value());
        m_commandBuffer.get().bindDescriptorSets(vk::PipelineBindPoint::eCompute, *m_pipelineLayout.value(), 0, *m_descriptorSet.value(), nullptr);
        m_commandBuffer.get().dispatch((uint32_t)ceil(pixelsCount / float(WorkgroupSize)), 1, 1);

        vk::BufferMemoryBarrier thresholdBufferBarrier(
            vk::AccessFlagBits::eShaderWrite,
            vk::AccessFlagBits::eShaderRead,
            VK_QUEUE_FAMILY_IGNORED,
            VK_QUEUE_FAMILY_IGNORED,
            m_threshold->get(),
            0,
            m_threshold->size()
        );
        m_commandBuffer.get().pipelineBarrier(vk::PipelineStageFlagBits::eComputeShader, vk::PipelineStageFlagBits::eComputeShader, {}, nullptr, thresholdBufferBarrier, nullptr);

#if defined(USE_2D_CONVOLUTION)
        m_commandBuffer.get().bindPipeline(vk::PipelineBindPoint::eCompute, *m_convolve2dComputePipeline.value());
        m_commandBuffer.get().bindDescriptorSets(vk::PipelineBindPoint::eCompute, *m_pipelineLayout.value(), 0, *m_descriptorSet.value(), nullptr);
        m_commandBuffer.get().dispatch((uint32_t)ceil(pixelsCount / float(WorkgroupSize)), 1, 1);
#else

        m_commandBuffer.get().bindPipeline(vk::PipelineBindPoint::eCompute, *m_convolve1dVerticalComputePipeline.value());
        m_commandBuffer.get().bindDescriptorSets(vk::PipelineBindPoint::eCompute, *m_pipelineLayout.value(), 0, *m_descriptorSet.value(), nullptr);
        m_commandBuffer.get().dispatch((uint32_t)ceil(pixelsCount / float(WorkgroupSize)), 1, 1);

        vk::BufferMemoryBarrier tmpBufferBarrier(
            vk::AccessFlagBits::eShaderWrite,
            vk::AccessFlagBits::eShaderRead,
            VK_QUEUE_FAMILY_IGNORED,
            VK_QUEUE_FAMILY_IGNORED,
            m_tmpBuffer->get(),
            0,
            m_tmpBuffer->size()
       );
        m_commandBuffer.get().pipelineBarrier(vk::PipelineStageFlagBits::eComputeShader, vk::PipelineStageFlagBits::eComputeShader, {}, nullptr, tmpBufferBarrier, nullptr);
        m_commandBuffer.get().bindPipeline(vk::PipelineBindPoint::eCompute, *m_convolve1dHorizontalComputePipeline.value());
        m_commandBuffer.get().bindDescriptorSets(vk::PipelineBindPoint::eCompute, *m_pipelineLayout.value(), 0, *m_descriptorSet.value(), nullptr);
        m_commandBuffer.get().dispatch((uint32_t)ceil(pixelsCount / float(WorkgroupSize)), 1, 1);

#endif
    }
    m_commandBuffer.get().end();
}

void BloomFilter::createComputePipelines()
{
    vk::PipelineLayoutCreateInfo pipelineLayoutInfo({}, *m_descriptorSetLayout.value());
    m_pipelineLayout = vk::raii::PipelineLayout(deviceContext().device, pipelineLayoutInfo);
    // threshold
    {
        vk::PipelineShaderStageCreateInfo shaderStageInfo({}, vk::ShaderStageFlagBits::eCompute, *m_thresholdShaderModule.value(), "main");
        vk::ComputePipelineCreateInfo pipelineInfo({}, shaderStageInfo, *m_pipelineLayout.value());
        m_thresholdComputePipeline = deviceContext().device.createComputePipeline(nullptr, pipelineInfo);
    }
#if defined(USE_2D_CONVOLUTION)
    // convolve2d
    {
        vk::PipelineShaderStageCreateInfo shaderStageInfo({}, vk::ShaderStageFlagBits::eCompute, *m_convolve2dShaderModule.value(), "main");
        vk::ComputePipelineCreateInfo pipelineInfo({}, shaderStageInfo, *m_pipelineLayout.value());
        m_convolve2dComputePipeline = deviceContext().device.createComputePipeline(nullptr, pipelineInfo);
    }
#else
    // convolve1d vertical
    {
        vk::PipelineShaderStageCreateInfo shaderStageInfo({}, vk::ShaderStageFlagBits::eCompute, *m_convolve1dVerticalShaderModule.value(), "main");
        vk::ComputePipelineCreateInfo pipelineInfo({}, shaderStageInfo, *m_pipelineLayout.value());
        m_convolve1dVerticalComputePipeline = deviceContext().device.createComputePipeline(nullptr, pipelineInfo);
    }
    // convolve1d horizontal
    {
        vk::PipelineShaderStageCreateInfo shaderStageInfo({}, vk::ShaderStageFlagBits::eCompute, *m_convolve1dHorizontalShaderModule.value(), "main");
        vk::ComputePipelineCreateInfo pipelineInfo({}, shaderStageInfo, *m_pipelineLayout.value());
        m_convolve1dHorizontalComputePipeline = deviceContext().device.createComputePipeline(nullptr, pipelineInfo);
    }
#endif
}

void BloomFilter::generateGaussianKernel1d()
{
    float* mappedKernelData = static_cast<float*>(m_kernelData->map(m_ubo.data().getKernelData1DBufferSizeInBytes()));
    float sigma = gaussianKernelDataSigma(m_input->description(), m_radius);
    float sum = 0.0f;
    const float distNormalization = -1.0f / (2.0f * sigma * sigma);
    for (int i = 0; i < m_ubo.data().kernelRadius + 1; i++) {
        const float distSq = i * i;
        const float result = expf(distNormalization * distSq);
        mappedKernelData[i] = result;
        sum += result;
    }

    // Normalize weights so they sum up to 1.0.
    const float normalization = 0.5f / sum;
    for (int i = 0; i < m_ubo.data().kernelSize; i++) {
        mappedKernelData[i] *= normalization;
    }

    m_kernelData->unmap();
}

void BloomFilter::generateGaussianKernel2d()
{
    float* mappedKernelData = static_cast<float*>(m_kernelData->map(m_ubo.data().getKernelData2DBufferSizeInBytes()));
    float sigma = gaussianKernelDataSigma(m_input->description(), m_radius);
    float sum = 0.0f;
    const float distNormalization = -1.0f / (2.0f * sigma * sigma);
    const float xShift = -0.5f * (float)(m_ubo.data().kernelSize - 1);
    const float yShift = -0.5f * (float)(m_ubo.data().kernelSize - 1);

    for (int j = 0; j < m_ubo.data().kernelSize; j++) {
        const float y = yShift + (float)j;
        for (int i = 0; i < m_ubo.data().kernelSize; i++) {
            const float x = xShift + (float)i;
            const float distSq = x * x + y * y;
            const float result = expf(distNormalization * distSq);
            mappedKernelData[j * m_ubo.data().kernelSize + i] = result;
            sum += result;
        }
    }

    // Normalize weights so they sum up to 1.0.
    const float normalization = 1.0f / sum;
    for (int j = 0; j < m_ubo.data().kernelSize; j++) {
        for (int i = 0; i < m_ubo.data().kernelSize; i++) {
            mappedKernelData[j * m_ubo.data().kernelSize + i] *= normalization;
        }
    }

    m_kernelData->unmap();
}

vk::Semaphore BloomFilter::run(std::optional<vk::Semaphore> waitSemaphore)
{
    validateInputsAndOutput();

    if (m_descriptorsDirty) {
        m_thresholdComputePipeline.reset();
        m_thresholdShaderModule.reset();
#if defined(USE_2D_CONVOLUTION)
        m_convolve2dComputePipeline.reset();
        m_convolve2dShaderModule.reset();
#else
        m_convolve1dVerticalComputePipeline.reset();
        m_convolve1dVerticalShaderModule.reset();
        m_convolve1dHorizontalComputePipeline.reset();
        m_convolve1dHorizontalShaderModule.reset();
#endif
        m_pipelineLayout.reset();
        m_descriptorSet.reset();
        m_descriptorPool.reset();
        m_descriptorSetLayout.reset();

        size_t thresholdSize = sizeof(float) * 4 * m_input->description().width * m_input->description().height;
        if (!m_threshold || m_threshold->size() != thresholdSize) {
            m_threshold.reset();
            m_threshold = std::make_unique<Buffer>(context(), thresholdSize, vk::BufferUsageFlagBits::eStorageBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal);

            m_tmpBuffer.reset();
            m_tmpBuffer = std::make_unique<Buffer>(context(), thresholdSize, vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferDst, vk::MemoryPropertyFlagBits::eDeviceLocal);
        }

        // m_radius cannot be more than 1.0f
        size_t maxKernelDataSize = gaussianKernelDataSize(m_input->description(), 1.0f);
        size_t maxKernelRadius = (maxKernelDataSize - 1) / 2;
        size_t kernelDataBufferSizeInBytes = 
#if defined(USE_2D_CONVOLUTION)
            maxKernelDataSize * maxKernelDataSize * sizeof(float);
#else
            (maxKernelDataSize + 1) * sizeof(float);
#endif
        if (!m_kernelData || m_kernelData->size() < kernelDataBufferSizeInBytes) {
            m_kernelData.reset();
            m_kernelData = std::make_unique<Buffer>(context(), kernelDataBufferSizeInBytes, vk::BufferUsageFlagBits::eStorageBuffer, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
            m_kernelDirty = true;
        }

        createShaderModules();
        createDescriptorSet();
        createComputePipelines();
        recordComputeCommandBuffers();
        m_descriptorsDirty = false;
    }

    if (m_ubo.dirty() || m_kernelDirty) {
        m_ubo.data().kernelSize = gaussianKernelDataSize(m_input->description(), m_radius);
        m_ubo.data().kernelRadius = (m_ubo.data().kernelSize - 1) / 2;
        m_ubo.update();
    }

    if (m_kernelDirty) {
#if defined(USE_2D_CONVOLUTION)
        generateGaussianKernel2d();
#else
        generateGaussianKernel1d();
#endif
        m_kernelDirty = false;
    }

    // threshold
    {
        vk::SubmitInfo submitInfo;
        vk::PipelineStageFlags waitStage = vk::PipelineStageFlagBits::eAllCommands;
        if (waitSemaphore.has_value()) {
            submitInfo.setWaitDstStageMask(waitStage);
            submitInfo.setWaitSemaphores(waitSemaphore.value());
        }

        submitInfo.setSignalSemaphores(*m_finishedSemaphore);
        submitInfo.setCommandBuffers(*m_commandBuffer.get());
        deviceContext().queue.submit(submitInfo);
    }
    return *m_finishedSemaphore;
}

void BloomFilter::setInput(Image* image)
{
    m_input = image;
    m_descriptorsDirty = true;
    m_kernelDirty = true;
}

void BloomFilter::setOutput(Image* image)
{        
    m_output = image;
    m_descriptorsDirty = true;
}

void BloomFilter::setRadius(float radius) noexcept
{
    m_radius = radius;
    m_ubo.markDirty();
    m_kernelDirty = true;
}

void BloomFilter::setIntensity(float intensity) noexcept
{
    m_ubo.data().intensity = intensity;
    m_ubo.markDirty();
}

void BloomFilter::setThreshold(float threshold) noexcept
{
    m_ubo.data().threshold = threshold;
    m_ubo.markDirty();
}

float BloomFilter::getRadius() const noexcept
{
    return m_radius;
}

float BloomFilter::getIntensity() const noexcept
{
    return m_ubo.data().intensity;
}

float BloomFilter::getThreshold() const noexcept
{
    return m_ubo.data().threshold;
}

}