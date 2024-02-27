#include "PostProcessing.h"
#include "DescriptorBuilder.h"
#include "Error.h"
#include "rprpp.h"
#include <algorithm>

template <class... Ts>
struct overload : Ts... {
    using Ts::operator()...;
};
template <class... Ts>
overload(Ts...) -> overload<Ts...>; // line not needed in

namespace rprpp {

const int WorkgroupSize = 32;
const int NumComponents = 4;

PostProcessing::PostProcessing(vk::helper::DeviceContext&& dctx,
    vk::raii::CommandPool&& commandPool,
    vk::raii::CommandBuffer&& secondaryCommandBuffer,
    vk::raii::CommandBuffer&& computeCommandBuffer,
    vk::helper::Buffer&& uboBuffer) noexcept
    : m_dctx(std::move(dctx))
    , m_commandPool(std::move(commandPool))
    , m_secondaryCommandBuffer(std::move(secondaryCommandBuffer))
    , m_computeCommandBuffer(std::move(computeCommandBuffer))
    , m_uboBuffer(std::move(uboBuffer))
{
}

std::unique_ptr<PostProcessing> PostProcessing::create(uint32_t deviceId)
{
    vk::helper::DeviceContext dctx = vk::helper::createDeviceContext(deviceId);

    vk::CommandPoolCreateInfo cmdPoolInfo(vk::CommandPoolCreateFlagBits::eResetCommandBuffer, dctx.queueFamilyIndex);
    vk::raii::CommandPool commandPool = vk::raii::CommandPool(dctx.device, cmdPoolInfo);

    vk::CommandBufferAllocateInfo allocInfo(*commandPool, vk::CommandBufferLevel::ePrimary, 2);
    vk::raii::CommandBuffers commandBuffers(dctx.device, allocInfo);
    assert(commandBuffers.size() >= 2);

    vk::helper::Buffer uboBuffer = vk::helper::createBuffer(dctx,
        sizeof(UniformBufferObject),
        vk::BufferUsageFlagBits::eUniformBuffer | vk::BufferUsageFlagBits::eTransferDst,
        vk::MemoryPropertyFlagBits::eDeviceLocal);

    return std::make_unique<PostProcessing>(
        std::move(dctx),
        std::move(commandPool),
        std::move(commandBuffers[0]),
        std::move(commandBuffers[1]),
        std::move(uboBuffer));
}

void PostProcessing::createShaderModule(ImageFormat outputFormat, bool aovsAreSampledImages)
{
    const std::unordered_map<std::string, std::string> macroDefinitions = {
        { "OUTPUT_FORMAT", to_glslformat(outputFormat) },
        { "WORKGROUP_SIZE", std::to_string(WorkgroupSize) },
        { "AOVS_ARE_SAMPLED_IMAGES", aovsAreSampledImages ? "1" : "0" }
    };
    m_shaderModule = m_shaderManager.get(m_dctx.device, macroDefinitions);
}

void PostProcessing::createImages(uint32_t width, uint32_t height, ImageFormat outputFormat, std::optional<AovsVkInteropInfo> aovsVkInteropInfo)
{
    size_t stagingBufferSize = std::max(sizeof(UniformBufferObject), width * height * NumComponents * sizeof(float));
    m_stagingBuffer = vk::helper::createBuffer(m_dctx,
        stagingBufferSize,
        vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eTransferDst,
        vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

    vk::Format aovFormat = vk::Format::eR32G32B32A32Sfloat;
    if (aovsVkInteropInfo.has_value()) {
        vk::ImageViewCreateInfo viewInfo({},
            aovsVkInteropInfo.value().color,
            vk::ImageViewType::e2D,
            aovFormat,
            {},
            { vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 });
        vk::raii::ImageView color(m_dctx.device, viewInfo);

        viewInfo.setImage(aovsVkInteropInfo.value().opacity);
        vk::raii::ImageView opacity(m_dctx.device, viewInfo);

        viewInfo.setImage(aovsVkInteropInfo.value().shadowCatcher);
        vk::raii::ImageView shadowCatcher(m_dctx.device, viewInfo);

        viewInfo.setImage(aovsVkInteropInfo.value().reflectionCatcher);
        vk::raii::ImageView reflectionCatcher(m_dctx.device, viewInfo);

        viewInfo.setImage(aovsVkInteropInfo.value().mattePass);
        vk::raii::ImageView mattePass(m_dctx.device, viewInfo);

        viewInfo.setImage(aovsVkInteropInfo.value().background);
        vk::raii::ImageView background(m_dctx.device, viewInfo);

        vk::raii::Sampler sampler(m_dctx.device, vk::SamplerCreateInfo());

        m_aovs = InteropAovs {
            .color = std::move(color),
            .opacity = std::move(opacity),
            .shadowCatcher = std::move(shadowCatcher),
            .reflectionCatcher = std::move(reflectionCatcher),
            .mattePass = std::move(mattePass),
            .background = std::move(background),
            .sampler = std::move(sampler),
        };
    } else {
        vk::ImageUsageFlags aovUsage = vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eStorage;
        vk::AccessFlags aovAccess = vk::AccessFlagBits::eShaderRead;
        Aovs aovs = {
            .color = vk::helper::createImage(m_dctx, width, height, aovFormat, aovUsage),
            .opacity = vk::helper::createImage(m_dctx, width, height, aovFormat, aovUsage),
            .shadowCatcher = vk::helper::createImage(m_dctx, width, height, aovFormat, aovUsage),
            .reflectionCatcher = vk::helper::createImage(m_dctx, width, height, aovFormat, aovUsage),
            .mattePass = vk::helper::createImage(m_dctx, width, height, aovFormat, aovUsage),
            .background = vk::helper::createImage(m_dctx, width, height, aovFormat, aovUsage),
        };

        transitionImageLayout(aovs.color, aovAccess, vk::ImageLayout::eGeneral, vk::PipelineStageFlagBits::eComputeShader);
        transitionImageLayout(aovs.opacity, aovAccess, vk::ImageLayout::eGeneral, vk::PipelineStageFlagBits::eComputeShader);
        transitionImageLayout(aovs.shadowCatcher, aovAccess, vk::ImageLayout::eGeneral, vk::PipelineStageFlagBits::eComputeShader);
        transitionImageLayout(aovs.reflectionCatcher, aovAccess, vk::ImageLayout::eGeneral, vk::PipelineStageFlagBits::eComputeShader);
        transitionImageLayout(aovs.mattePass, aovAccess, vk::ImageLayout::eGeneral, vk::PipelineStageFlagBits::eComputeShader);
        transitionImageLayout(aovs.background, aovAccess, vk::ImageLayout::eGeneral, vk::PipelineStageFlagBits::eComputeShader);
        m_aovs = std::move(aovs);
    }

    m_outputImage = vk::helper::createImage(m_dctx,
        width,
        height,
        to_vk_format(outputFormat),
        vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eStorage);

    transitionImageLayout(m_outputImage.value(),
        vk::AccessFlagBits::eShaderRead | vk::AccessFlagBits::eShaderWrite,
        vk::ImageLayout::eGeneral,
        vk::PipelineStageFlagBits::eComputeShader);
}

void PostProcessing::transitionImageLayout(vk::helper::Image& image,
    vk::AccessFlags dstAccess,
    vk::ImageLayout dstLayout,
    vk::PipelineStageFlags dstStage)
{
    vk::ImageSubresourceRange subresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1);
    vk::ImageMemoryBarrier imageMemoryBarrier(image.access,
        dstAccess,
        image.layout,
        dstLayout,
        VK_QUEUE_FAMILY_IGNORED,
        VK_QUEUE_FAMILY_IGNORED,
        *image.image,
        subresourceRange);

    m_secondaryCommandBuffer.begin(vk::CommandBufferBeginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit));
    m_secondaryCommandBuffer.pipelineBarrier(image.stage,
        dstStage,
        {},
        nullptr,
        nullptr,
        imageMemoryBarrier);

    m_secondaryCommandBuffer.end();

    vk::SubmitInfo submitInfo(nullptr, nullptr, *m_secondaryCommandBuffer);
    m_dctx.queue.submit(submitInfo);
    m_dctx.queue.waitIdle();

    image.access = dstAccess;
    image.layout = dstLayout;
    image.stage = dstStage;
}

void PostProcessing::createDescriptorSet()
{
    DescriptorBuilder builder;
    vk::DescriptorImageInfo outputDescriptorInfo(nullptr, *m_outputImage->view, vk::ImageLayout::eGeneral); // binding 0
    builder.bindStorageImage(&outputDescriptorInfo);

    std::vector<vk::DescriptorImageInfo> descriptorImageInfos;
    std::visit(overload {
                   [&](const Aovs& arg) {
                       descriptorImageInfos.push_back(vk::DescriptorImageInfo(nullptr, *arg.color.view, vk::ImageLayout::eGeneral)); // binding 1
                       descriptorImageInfos.push_back(vk::DescriptorImageInfo(nullptr, *arg.opacity.view, vk::ImageLayout::eGeneral)); // binding 2
                       descriptorImageInfos.push_back(vk::DescriptorImageInfo(nullptr, *arg.shadowCatcher.view, vk::ImageLayout::eGeneral)); // binding 3
                       descriptorImageInfos.push_back(vk::DescriptorImageInfo(nullptr, *arg.reflectionCatcher.view, vk::ImageLayout::eGeneral)); // binding 4
                       descriptorImageInfos.push_back(vk::DescriptorImageInfo(nullptr, *arg.mattePass.view, vk::ImageLayout::eGeneral)); // binding 5
                       descriptorImageInfos.push_back(vk::DescriptorImageInfo(nullptr, *arg.background.view, vk::ImageLayout::eGeneral)); // binding 6
                       for (auto& dii : descriptorImageInfos) {
                           builder.bindStorageImage(&dii);
                       }
                   },
                   [&](const InteropAovs& arg) {
                       descriptorImageInfos.push_back(vk::DescriptorImageInfo(*arg.sampler, *arg.color, vk::ImageLayout::eShaderReadOnlyOptimal)); // binding 1
                       descriptorImageInfos.push_back(vk::DescriptorImageInfo(*arg.sampler, *arg.opacity, vk::ImageLayout::eShaderReadOnlyOptimal)); // binding 2
                       descriptorImageInfos.push_back(vk::DescriptorImageInfo(*arg.sampler, *arg.shadowCatcher, vk::ImageLayout::eShaderReadOnlyOptimal)); // binding 3
                       descriptorImageInfos.push_back(vk::DescriptorImageInfo(*arg.sampler, *arg.reflectionCatcher, vk::ImageLayout::eShaderReadOnlyOptimal)); // binding 4
                       descriptorImageInfos.push_back(vk::DescriptorImageInfo(*arg.sampler, *arg.mattePass, vk::ImageLayout::eShaderReadOnlyOptimal)); // binding 5
                       descriptorImageInfos.push_back(vk::DescriptorImageInfo(*arg.sampler, *arg.background, vk::ImageLayout::eShaderReadOnlyOptimal)); // binding 6
                       for (auto& dii : descriptorImageInfos) {
                           builder.bindCombinedImageSampler(&dii);
                       }
                   } },
        m_aovs.value());

    vk::DescriptorBufferInfo uboDescriptoInfo = vk::DescriptorBufferInfo(*m_uboBuffer.buffer, 0, sizeof(UniformBufferObject)); // binding 7
    builder.bindUniformBuffer(&uboDescriptoInfo);

    const std::vector<vk::DescriptorPoolSize>& poolSizes = builder.poolSizes();
    m_descriptorSetLayout = m_dctx.device.createDescriptorSetLayout(vk::DescriptorSetLayoutCreateInfo({}, builder.bindings()));
    m_descriptorPool = m_dctx.device.createDescriptorPool(vk::DescriptorPoolCreateInfo(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet, 1, poolSizes));
    m_descriptorSet = std::move(vk::raii::DescriptorSets(m_dctx.device, vk::DescriptorSetAllocateInfo(*m_descriptorPool.value(), *m_descriptorSetLayout.value())).front());

    builder.updateDescriptorSet(*m_descriptorSet.value());
    m_dctx.device.updateDescriptorSets(builder.writes(), nullptr);
}

void PostProcessing::createComputePipeline()
{
    vk::PipelineLayoutCreateInfo pipelineLayoutInfo({}, *m_descriptorSetLayout.value());
    m_pipelineLayout = vk::raii::PipelineLayout(m_dctx.device, pipelineLayoutInfo);

    vk::PipelineShaderStageCreateInfo shaderStageInfo({}, vk::ShaderStageFlagBits::eCompute, *m_shaderModule.value(), "main");
    vk::ComputePipelineCreateInfo pipelineInfo({}, shaderStageInfo, *m_pipelineLayout.value());
    m_computePipeline = m_dctx.device.createComputePipeline(nullptr, pipelineInfo);
}

void PostProcessing::recordComputeCommandBuffer(uint32_t width, uint32_t height)
{
    m_computeCommandBuffer.begin(vk::CommandBufferBeginInfo(vk::CommandBufferUsageFlagBits::eSimultaneousUse));
    m_computeCommandBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, *m_computePipeline.value());
    m_computeCommandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eCompute,
        *m_pipelineLayout.value(),
        0,
        *m_descriptorSet.value(),
        nullptr);
    m_computeCommandBuffer.dispatch((uint32_t)ceil(width / float(WorkgroupSize)), (uint32_t)ceil(height / float(WorkgroupSize)), 1);
    m_computeCommandBuffer.end();
}

void PostProcessing::copyStagingBufferToAov(vk::helper::Image& image)
{
    // image transitions
    vk::AccessFlags oldAccess = image.access;
    vk::ImageLayout oldLayout = image.layout;
    vk::PipelineStageFlags oldStage = image.stage;
    transitionImageLayout(image,
        vk::AccessFlagBits::eTransferWrite,
        vk::ImageLayout::eTransferDstOptimal,
        vk::PipelineStageFlagBits::eTransfer);
    {
        m_secondaryCommandBuffer.begin(vk::CommandBufferBeginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit));

        vk::ImageSubresourceLayers imageSubresource(vk::ImageAspectFlagBits::eColor, 0, 0, 1);
        vk::BufferImageCopy region(0, 0, 0, imageSubresource, { 0, 0, 0 }, { image.width, image.height, 1 });
        m_secondaryCommandBuffer.copyBufferToImage(*m_stagingBuffer->buffer,
            *image.image,
            vk::ImageLayout::eTransferDstOptimal,
            region);

        m_secondaryCommandBuffer.end();

        vk::SubmitInfo submitInfo(nullptr, nullptr, *m_secondaryCommandBuffer);
        m_dctx.queue.submit(submitInfo);
        m_dctx.queue.waitIdle();
    }
    transitionImageLayout(image,
        oldAccess,
        oldLayout,
        oldStage);
}

void* PostProcessing::mapStagingBuffer(size_t size)
{
    return m_stagingBuffer->memory.mapMemory(0, size, {});
}

void PostProcessing::unmapStagingBuffer()
{
    m_stagingBuffer->memory.unmapMemory();
}

void PostProcessing::updateUbo()
{
    void* data = m_stagingBuffer->memory.mapMemory(0, sizeof(UniformBufferObject), {});
    std::memcpy(data, &m_ubo, sizeof(UniformBufferObject));
    m_stagingBuffer->memory.unmapMemory();

    m_secondaryCommandBuffer.begin(vk::CommandBufferBeginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit));
    m_secondaryCommandBuffer.copyBuffer(*m_stagingBuffer->buffer, *m_uboBuffer.buffer, vk::BufferCopy(0, 0, sizeof(UniformBufferObject)));
    m_secondaryCommandBuffer.end();

    vk::SubmitInfo submitInfo(nullptr, nullptr, *m_secondaryCommandBuffer);
    m_dctx.queue.submit(submitInfo);
    m_dctx.queue.waitIdle();
}

void PostProcessing::resize(uint32_t width, uint32_t height, ImageFormat format, std::optional<AovsVkInteropInfo> aovsVkInteropInfo)
{
    if (m_width == width
        && m_height == height
        && m_outputImageFormat == format
        && m_aovsVkInteropInfo == aovsVkInteropInfo) {
        return;
    }

    m_computePipeline.reset();
    m_pipelineLayout.reset();
    m_descriptorSet.reset();
    m_descriptorPool.reset();
    m_descriptorSetLayout.reset();
    m_aovs.reset();
    m_outputImage.reset();
    m_stagingBuffer.reset();

    if (m_outputImageFormat != format || m_aovsVkInteropInfo.has_value() != aovsVkInteropInfo.has_value() || !m_shaderModule.has_value()) {
        m_shaderModule.reset();
        createShaderModule(format, aovsVkInteropInfo.has_value());
    }

    if (width > 0 && height > 0) {
        createImages(width, height, format, aovsVkInteropInfo);
        createDescriptorSet();
        createComputePipeline();
        recordComputeCommandBuffer(width, height);
    }

    m_width = width;
    m_height = height;
    m_outputImageFormat = format;
    m_aovsVkInteropInfo = aovsVkInteropInfo;
}

void PostProcessing::getOutput(uint8_t* dst, size_t size, size_t* retSize)
{
    if (retSize != nullptr) {
        *retSize = m_width * m_height * to_pixel_size(m_outputImageFormat);
    }

    if (dst != nullptr && size > 0) {
        vk::AccessFlags oldAccess = m_outputImage->access;
        vk::ImageLayout oldLayout = m_outputImage->layout;
        vk::PipelineStageFlags oldStage = m_outputImage->stage;
        transitionImageLayout(m_outputImage.value(),
            vk::AccessFlagBits::eTransferRead,
            vk::ImageLayout::eTransferSrcOptimal,
            vk::PipelineStageFlagBits::eTransfer);
        {
            m_secondaryCommandBuffer.begin(vk::CommandBufferBeginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit));

            vk::ImageSubresourceLayers imageSubresource(vk::ImageAspectFlagBits::eColor, 0, 0, 1);
            vk::BufferImageCopy region(0, 0, 0, imageSubresource, { 0, 0, 0 }, { m_outputImage->width, m_outputImage->height, 1 });
            m_secondaryCommandBuffer.copyImageToBuffer(*m_outputImage->image,
                vk::ImageLayout::eTransferSrcOptimal,
                *m_stagingBuffer->buffer,
                region);

            m_secondaryCommandBuffer.end();

            vk::SubmitInfo submitInfo(nullptr, nullptr, *m_secondaryCommandBuffer);
            m_dctx.queue.submit(submitInfo);
            m_dctx.queue.waitIdle();
        }

        transitionImageLayout(m_outputImage.value(), oldAccess, oldLayout, oldStage);

        void* data = m_stagingBuffer->memory.mapMemory(0, size, {});
        std::memcpy(dst, data, size);
        m_stagingBuffer->memory.unmapMemory();
    }
}

void PostProcessing::run(std::optional<vk::Semaphore> aovsReadySemaphore, std::optional<vk::Semaphore> toSignalAfterProcessingSemaphore)
{
    if (m_computePipeline.has_value()) {
        if (m_uboDirty) {
            updateUbo();
            m_uboDirty = false;
        }

        vk::PipelineStageFlags waitStage = vk::PipelineStageFlagBits::eAllCommands;
        vk::SubmitInfo submitInfo;
        if (aovsReadySemaphore.has_value()) {
            submitInfo.setWaitDstStageMask(waitStage);
            submitInfo.setWaitSemaphores(aovsReadySemaphore.value());
        }
        if (toSignalAfterProcessingSemaphore.has_value()) {
            submitInfo.setSignalSemaphores(toSignalAfterProcessingSemaphore.value());
        }
        submitInfo.setCommandBuffers(*m_computeCommandBuffer);
        m_dctx.queue.submit(submitInfo);
    }
}

void PostProcessing::waitQueueIdle()
{
    m_dctx.queue.waitIdle();
}

void PostProcessing::copyOutputToDx11Texture(HANDLE dx11textureHandle)
{
    if (!m_outputImage.has_value()) {
        return;
    }

    if (dx11textureHandle == nullptr) {
        throw InvalidParameter("dx11textureHandle", "Cannot be null");
    }

    // we set vk::ImageUsageFlagBits::eStorage in order to remove warnings in validation layer
    auto externalDx11Image = vk::helper::createImageFromDx11Texture(m_dctx,
        dx11textureHandle,
        m_outputImage->width,
        m_outputImage->height,
        to_vk_format(m_outputImageFormat),
        vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eStorage);

    transitionImageLayout(externalDx11Image,
        vk::AccessFlagBits::eTransferWrite,
        vk::ImageLayout::eTransferDstOptimal,
        vk::PipelineStageFlagBits::eTransfer);

    vk::AccessFlags oldAccess = m_outputImage->access;
    vk::ImageLayout oldLayout = m_outputImage->layout;
    vk::PipelineStageFlags oldStage = m_outputImage->stage;
    transitionImageLayout(m_outputImage.value(),
        vk::AccessFlagBits::eTransferRead,
        vk::ImageLayout::eTransferSrcOptimal,
        vk::PipelineStageFlagBits::eTransfer);
    {
        m_secondaryCommandBuffer.begin(vk::CommandBufferBeginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit));

        vk::ImageSubresourceLayers imageSubresource(vk::ImageAspectFlagBits::eColor, 0, 0, 1);
        vk::ImageCopy region(imageSubresource, { 0, 0, 0 }, imageSubresource, { 0, 0, 0 }, { m_outputImage->width, m_outputImage->height, 1 });
        m_secondaryCommandBuffer.copyImage(*m_outputImage->image, vk::ImageLayout::eTransferSrcOptimal, *externalDx11Image.image, vk::ImageLayout::eTransferDstOptimal, region);

        m_secondaryCommandBuffer.end();

        vk::SubmitInfo submitInfo(nullptr, nullptr, *m_secondaryCommandBuffer);
        m_dctx.queue.submit(submitInfo);
        m_dctx.queue.waitIdle();
    }

    transitionImageLayout(m_outputImage.value(), oldAccess, oldLayout, oldStage);
}

VkPhysicalDevice PostProcessing::getVkPhysicalDevice() const noexcept
{
    return static_cast<VkPhysicalDevice>(*m_dctx.physicalDevice);
}

VkDevice PostProcessing::getVkDevice() const noexcept
{
    return static_cast<VkDevice>(*m_dctx.device);
}

VkQueue PostProcessing::getVkQueue() const noexcept
{
    return static_cast<VkQueue>(*m_dctx.queue);
}

void PostProcessing::copyStagingBufferToAovColor()
{
    if (m_aovsVkInteropInfo.has_value()) {
        throw InvalidOperation("copyStagingBuffer cannot be called when vkinterop is used");
    }
    std::visit(overload {
                   [&](Aovs& arg) { copyStagingBufferToAov(arg.color); },
                   [](InteropAovs& args) {} },
        m_aovs.value());
}

void PostProcessing::copyStagingBufferToAovOpacity()
{
    if (m_aovsVkInteropInfo.has_value()) {
        throw InvalidOperation("copyStagingBuffer cannot be called when vkinterop is used");
    }
    std::visit(overload {
                   [&](Aovs& arg) { copyStagingBufferToAov(arg.opacity); },
                   [](InteropAovs& args) {} },
        m_aovs.value());
}

void PostProcessing::copyStagingBufferToAovShadowCatcher()
{
    if (m_aovsVkInteropInfo.has_value()) {
        throw InvalidOperation("copyStagingBuffer cannot be called when vkinterop is used");
    }
    std::visit(overload {
                   [&](Aovs& arg) { copyStagingBufferToAov(arg.shadowCatcher); },
                   [](InteropAovs& args) {} },
        m_aovs.value());
}

void PostProcessing::copyStagingBufferToAovReflectionCatcher()
{
    if (m_aovsVkInteropInfo.has_value()) {
        throw InvalidOperation("copyStagingBuffer cannot be called when vkinterop is used");
    }
    std::visit(overload {
                   [&](Aovs& arg) { copyStagingBufferToAov(arg.reflectionCatcher); },
                   [](InteropAovs& args) {} },
        m_aovs.value());
}

void PostProcessing::copyStagingBufferToAovMattePass()
{
    if (m_aovsVkInteropInfo.has_value()) {
        throw InvalidOperation("copyStagingBuffer cannot be called when vkinterop is used");
    }
    std::visit(overload {
                   [&](Aovs& arg) { copyStagingBufferToAov(arg.mattePass); },
                   [](InteropAovs& args) {} },
        m_aovs.value());
}

void PostProcessing::copyStagingBufferToAovBackground()
{
    if (m_aovsVkInteropInfo.has_value()) {
        throw InvalidOperation("copyStagingBuffer cannot be called when vkinterop is used");
    }
    std::visit(overload {
                   [&](Aovs& arg) { copyStagingBufferToAov(arg.background); },
                   [](InteropAovs& args) {} },
        m_aovs.value());
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
