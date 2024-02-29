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

PostProcessing::PostProcessing(const std::shared_ptr<vk::helper::DeviceContext>& dctx,
    vk::raii::CommandPool&& commandPool,
    CommandBuffers&& commandBuffers,
    vk::helper::Buffer&& uboBuffer) noexcept
    : m_dctx(dctx)
    , m_commandPool(std::move(commandPool))
    , m_commandBuffers(std::move(commandBuffers))
    , m_uboBuffer(std::move(uboBuffer))
{
}

void PostProcessing::createShaderModule(ImageFormat outputFormat, bool aovsAreSampledImages)
{
    const std::unordered_map<std::string, std::string> macroDefinitions = {
        { "OUTPUT_FORMAT", to_glslformat(outputFormat) },
        { "WORKGROUP_SIZE", std::to_string(WorkgroupSize) },
        { "AOVS_ARE_SAMPLED_IMAGES", aovsAreSampledImages ? "1" : "0" }
    };
    m_shaderModule = m_shaderManager.get(m_dctx->device, macroDefinitions);
}

void PostProcessing::createImages(uint32_t width, uint32_t height, ImageFormat outputFormat, std::optional<AovsVkInteropInfo> aovsVkInteropInfo)
{
    vk::Format aovFormat = vk::Format::eR32G32B32A32Sfloat;
    if (aovsVkInteropInfo.has_value()) {
        vk::ImageViewCreateInfo viewInfo({},
            aovsVkInteropInfo.value().color,
            vk::ImageViewType::e2D,
            aovFormat,
            {},
            { vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 });
        vk::raii::ImageView color(m_dctx->device, viewInfo);

        viewInfo.setImage(aovsVkInteropInfo.value().opacity);
        vk::raii::ImageView opacity(m_dctx->device, viewInfo);

        viewInfo.setImage(aovsVkInteropInfo.value().shadowCatcher);
        vk::raii::ImageView shadowCatcher(m_dctx->device, viewInfo);

        viewInfo.setImage(aovsVkInteropInfo.value().reflectionCatcher);
        vk::raii::ImageView reflectionCatcher(m_dctx->device, viewInfo);

        viewInfo.setImage(aovsVkInteropInfo.value().mattePass);
        vk::raii::ImageView mattePass(m_dctx->device, viewInfo);

        viewInfo.setImage(aovsVkInteropInfo.value().background);
        vk::raii::ImageView background(m_dctx->device, viewInfo);

        vk::raii::Sampler sampler(m_dctx->device, vk::SamplerCreateInfo());

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
            .color = vk::helper::createImage(*m_dctx, width, height, aovFormat, aovUsage),
            .opacity = vk::helper::createImage(*m_dctx, width, height, aovFormat, aovUsage),
            .shadowCatcher = vk::helper::createImage(*m_dctx, width, height, aovFormat, aovUsage),
            .reflectionCatcher = vk::helper::createImage(*m_dctx, width, height, aovFormat, aovUsage),
            .mattePass = vk::helper::createImage(*m_dctx, width, height, aovFormat, aovUsage),
            .background = vk::helper::createImage(*m_dctx, width, height, aovFormat, aovUsage),
        };

        transitionImageLayout(aovs.color, aovAccess, vk::ImageLayout::eGeneral, vk::PipelineStageFlagBits::eComputeShader);
        transitionImageLayout(aovs.opacity, aovAccess, vk::ImageLayout::eGeneral, vk::PipelineStageFlagBits::eComputeShader);
        transitionImageLayout(aovs.shadowCatcher, aovAccess, vk::ImageLayout::eGeneral, vk::PipelineStageFlagBits::eComputeShader);
        transitionImageLayout(aovs.reflectionCatcher, aovAccess, vk::ImageLayout::eGeneral, vk::PipelineStageFlagBits::eComputeShader);
        transitionImageLayout(aovs.mattePass, aovAccess, vk::ImageLayout::eGeneral, vk::PipelineStageFlagBits::eComputeShader);
        transitionImageLayout(aovs.background, aovAccess, vk::ImageLayout::eGeneral, vk::PipelineStageFlagBits::eComputeShader);
        m_aovs = std::move(aovs);
    }

    m_outputImage = vk::helper::createImage(*m_dctx,
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
    m_commandBuffers.secondary.begin(vk::CommandBufferBeginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit));
    transitionImageLayout(m_commandBuffers.secondary, image, dstAccess, dstLayout, dstStage);
    m_commandBuffers.secondary.end();

    vk::SubmitInfo submitInfo(nullptr, nullptr, *m_commandBuffers.secondary);
    m_dctx->queue.submit(submitInfo);
    m_dctx->queue.waitIdle();
}

void PostProcessing::transitionImageLayout(vk::raii::CommandBuffer& commandBuffer, vk::helper::Image& image,
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

    commandBuffer.pipelineBarrier(image.stage,
        dstStage,
        {},
        nullptr,
        nullptr,
        imageMemoryBarrier);

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
    m_descriptorSetLayout = m_dctx->device.createDescriptorSetLayout(vk::DescriptorSetLayoutCreateInfo({}, builder.bindings()));
    m_descriptorPool = m_dctx->device.createDescriptorPool(vk::DescriptorPoolCreateInfo(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet, 1, poolSizes));
    m_descriptorSet = std::move(vk::raii::DescriptorSets(m_dctx->device, vk::DescriptorSetAllocateInfo(*m_descriptorPool.value(), *m_descriptorSetLayout.value())).front());

    builder.updateDescriptorSet(*m_descriptorSet.value());
    m_dctx->device.updateDescriptorSets(builder.writes(), nullptr);
}

void PostProcessing::createComputePipeline()
{
    vk::PipelineLayoutCreateInfo pipelineLayoutInfo({}, *m_descriptorSetLayout.value());
    m_pipelineLayout = vk::raii::PipelineLayout(m_dctx->device, pipelineLayoutInfo);

    vk::PipelineShaderStageCreateInfo shaderStageInfo({}, vk::ShaderStageFlagBits::eCompute, *m_shaderModule.value(), "main");
    vk::ComputePipelineCreateInfo pipelineInfo({}, shaderStageInfo, *m_pipelineLayout.value());
    m_computePipeline = m_dctx->device.createComputePipeline(nullptr, pipelineInfo);
}

void PostProcessing::recordComputeCommandBuffer(uint32_t width, uint32_t height)
{
    m_commandBuffers.compute.begin(vk::CommandBufferBeginInfo(vk::CommandBufferUsageFlagBits::eSimultaneousUse));
    m_commandBuffers.compute.bindPipeline(vk::PipelineBindPoint::eCompute, *m_computePipeline.value());
    m_commandBuffers.compute.bindDescriptorSets(vk::PipelineBindPoint::eCompute,
        *m_pipelineLayout.value(),
        0,
        *m_descriptorSet.value(),
        nullptr);
    m_commandBuffers.compute.dispatch((uint32_t)ceil(width / float(WorkgroupSize)), (uint32_t)ceil(height / float(WorkgroupSize)), 1);
    m_commandBuffers.compute.end();
}

void PostProcessing::copyBufferToAov(const HostVisibleBuffer& src, vk::helper::Image& dst)
{
    vk::AccessFlags oldAccess = dst.access;
    vk::ImageLayout oldLayout = dst.layout;
    vk::PipelineStageFlags oldStage = dst.stage;

    m_commandBuffers.secondary.begin(vk::CommandBufferBeginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit));
    transitionImageLayout(m_commandBuffers.secondary,
        dst,
        vk::AccessFlagBits::eTransferWrite,
        vk::ImageLayout::eTransferDstOptimal,
        vk::PipelineStageFlagBits::eTransfer);
    {

        vk::ImageSubresourceLayers imageSubresource(vk::ImageAspectFlagBits::eColor, 0, 0, 1);
        vk::BufferImageCopy region(0, 0, 0, imageSubresource, { 0, 0, 0 }, { dst.width, dst.height, 1 });
        m_commandBuffers.secondary.copyBufferToImage(*src.get().buffer,
            *dst.image,
            vk::ImageLayout::eTransferDstOptimal,
            region);
    }
    transitionImageLayout(m_commandBuffers.secondary,
        dst,
        oldAccess,
        oldLayout,
        oldStage);

    m_commandBuffers.secondary.end();

    vk::SubmitInfo submitInfo(nullptr, nullptr, *m_commandBuffers.secondary);
    m_dctx->queue.submit(submitInfo);
    m_dctx->queue.waitIdle();
}

void PostProcessing::updateUbo()
{
    void* data = m_uboBuffer.memory.mapMemory(0, sizeof(UniformBufferObject), {});
    std::memcpy(data, &m_ubo, sizeof(UniformBufferObject));
    m_uboBuffer.memory.unmapMemory();
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

void PostProcessing::copyOutputTo(HostVisibleBuffer& dst)
{
    size_t size = m_width * m_height * to_pixel_size(m_outputImageFormat);
    if (dst.size() < size) {
        throw InvalidParameter("dst", "The provided buffer is smaller than output image");
    }

    vk::AccessFlags oldAccess = m_outputImage->access;
    vk::ImageLayout oldLayout = m_outputImage->layout;
    vk::PipelineStageFlags oldStage = m_outputImage->stage;

    m_commandBuffers.secondary.begin(vk::CommandBufferBeginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit));
    transitionImageLayout(m_commandBuffers.secondary, m_outputImage.value(),
        vk::AccessFlagBits::eTransferRead,
        vk::ImageLayout::eTransferSrcOptimal,
        vk::PipelineStageFlagBits::eTransfer);
    {
        vk::ImageSubresourceLayers imageSubresource(vk::ImageAspectFlagBits::eColor, 0, 0, 1);
        vk::BufferImageCopy region(0, 0, 0, imageSubresource, { 0, 0, 0 }, { m_outputImage->width, m_outputImage->height, 1 });
        m_commandBuffers.secondary.copyImageToBuffer(*m_outputImage->image,
            vk::ImageLayout::eTransferSrcOptimal,
            *dst.get().buffer,
            region);
    }
    transitionImageLayout(m_commandBuffers.secondary, m_outputImage.value(), oldAccess, oldLayout, oldStage);
    m_commandBuffers.secondary.end();

    vk::SubmitInfo submitInfo(nullptr, nullptr, *m_commandBuffers.secondary);
    m_dctx->queue.submit(submitInfo);
    m_dctx->queue.waitIdle();
}

void PostProcessing::copyOutputTo(Image& image)
{
    if (!m_outputImage.has_value()) {
        return;
    }

    m_commandBuffers.secondary.begin(vk::CommandBufferBeginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit));
    transitionImageLayout(
        m_commandBuffers.secondary,
        image.get(),
        vk::AccessFlagBits::eTransferWrite,
        vk::ImageLayout::eTransferDstOptimal,
        vk::PipelineStageFlagBits::eTransfer);

    vk::AccessFlags oldAccess = m_outputImage->access;
    vk::ImageLayout oldLayout = m_outputImage->layout;
    vk::PipelineStageFlags oldStage = m_outputImage->stage;
    transitionImageLayout(
        m_commandBuffers.secondary,
        m_outputImage.value(),
        vk::AccessFlagBits::eTransferRead,
        vk::ImageLayout::eTransferSrcOptimal,
        vk::PipelineStageFlagBits::eTransfer);
    {
        vk::ImageSubresourceLayers imageSubresource(vk::ImageAspectFlagBits::eColor, 0, 0, 1);
        vk::ImageCopy region(imageSubresource, { 0, 0, 0 }, imageSubresource, { 0, 0, 0 }, { m_outputImage->width, m_outputImage->height, 1 });
        m_commandBuffers.secondary.copyImage(*m_outputImage->image, vk::ImageLayout::eTransferSrcOptimal, *image.get().image, vk::ImageLayout::eTransferDstOptimal, region);
    }
    transitionImageLayout(m_commandBuffers.secondary, m_outputImage.value(), oldAccess, oldLayout, oldStage);
    m_commandBuffers.secondary.end();

    vk::SubmitInfo submitInfo(nullptr, nullptr, *m_commandBuffers.secondary);
    m_dctx->queue.submit(submitInfo);
    m_dctx->queue.waitIdle();
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
        submitInfo.setCommandBuffers(*m_commandBuffers.compute);
        m_dctx->queue.submit(submitInfo);
    }
}

void PostProcessing::waitQueueIdle()
{
    m_dctx->queue.waitIdle();
}

void PostProcessing::copyBufferToAovColor(const HostVisibleBuffer& src)
{
    if (m_aovsVkInteropInfo.has_value()) {
        throw InvalidOperation("copyBufferToAovColor cannot be called when vkinterop is used");
    }
    std::visit(overload {
                   [&](Aovs& arg) { copyBufferToAov(src, arg.color); },
                   [](InteropAovs& args) {} },
        m_aovs.value());
}

void PostProcessing::copyBufferToAovOpacity(const HostVisibleBuffer& src)
{
    if (m_aovsVkInteropInfo.has_value()) {
        throw InvalidOperation("copyBufferToAovOpacity cannot be called when vkinterop is used");
    }
    std::visit(overload {
                   [&](Aovs& arg) { copyBufferToAov(src, arg.opacity); },
                   [](InteropAovs& args) {} },
        m_aovs.value());
}

void PostProcessing::copyBufferToAovShadowCatcher(const HostVisibleBuffer& src)
{
    if (m_aovsVkInteropInfo.has_value()) {
        throw InvalidOperation("copyBufferToAovShadowCatcher cannot be called when vkinterop is used");
    }
    std::visit(overload {
                   [&](Aovs& arg) { copyBufferToAov(src, arg.shadowCatcher); },
                   [](InteropAovs& args) {} },
        m_aovs.value());
}

void PostProcessing::copyBufferToAovReflectionCatcher(const HostVisibleBuffer& src)
{
    if (m_aovsVkInteropInfo.has_value()) {
        throw InvalidOperation("copyBufferToAovReflectionCatcher cannot be called when vkinterop is used");
    }
    std::visit(overload {
                   [&](Aovs& arg) { copyBufferToAov(src, arg.reflectionCatcher); },
                   [](InteropAovs& args) {} },
        m_aovs.value());
}

void PostProcessing::copyBufferToAovMattePass(const HostVisibleBuffer& src)
{
    if (m_aovsVkInteropInfo.has_value()) {
        throw InvalidOperation("copyBufferToAovMattePass cannot be called when vkinterop is used");
    }
    std::visit(overload {
                   [&](Aovs& arg) { copyBufferToAov(src, arg.mattePass); },
                   [](InteropAovs& args) {} },
        m_aovs.value());
}

void PostProcessing::copyBufferToAovBackground(const HostVisibleBuffer& src)
{
    if (m_aovsVkInteropInfo.has_value()) {
        throw InvalidOperation("copyBufferToAovBackground cannot be called when vkinterop is used");
    }
    std::visit(overload {
                   [&](Aovs& arg) { copyBufferToAov(src, arg.background); },
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
