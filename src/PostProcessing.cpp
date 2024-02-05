#define VK_USE_PLATFORM_WIN32_KHR

#include "common.hpp"
#include "PostProcessing.hpp"
#include "vk_helper.hpp"
#include "rpr_helper.hpp"
#include <fstream>
#include <iostream>
#include <map>
#include <shaderc/shaderc.hpp>

static VKAPI_ATTR VkBool32 VKAPI_CALL debugUtilsMessengerCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
{
    std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
    return VK_FALSE;
}

vk::DebugUtilsMessengerCreateInfoEXT makeDebugUtilsMessengerCreateInfoEXT()
{
    return { {},
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError,
        vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation,
        &debugUtilsMessengerCallback };
}

PostProcessing::PostProcessing(HANDLE sharedDx11TextureHandle,
    bool enableValidationLayers,
    uint32_t width,
    uint32_t height,
    uint32_t deviceId,
    const std::filesystem::path& shaderPath)
{
    createInstance(enableValidationLayers);
    findPhysicalDevice(deviceId);
    createDevice();
    createShaderModule(shaderPath);
    createCommandBuffers();
    createUbo();
    resize(sharedDx11TextureHandle, width, height);
}

void PostProcessing::createInstance(bool enableValidationLayers)
{
    std::vector<const char*> enabledExtensions;
    std::map<const char*, bool, cmp_str> foundExtensions = {
        { VK_EXT_DEBUG_UTILS_EXTENSION_NAME, false },
        { VK_KHR_EXTERNAL_MEMORY_CAPABILITIES_EXTENSION_NAME, false },
        { VK_KHR_EXTERNAL_SEMAPHORE_CAPABILITIES_EXTENSION_NAME, false },
    };
    std::optional<vk::DebugUtilsMessengerCreateInfoEXT> debugCreateInfo;
    if (enableValidationLayers) {
        bool foundValidationLayer = false;
        for (auto& prop : m_context.enumerateInstanceLayerProperties()) {
            if (strcmp("VK_LAYER_KHRONOS_validation", prop.layerName) == 0) {
                foundValidationLayer = true;
                m_enabledLayers.push_back("VK_LAYER_KHRONOS_validation");
                break;
            }
        }

        if (!foundValidationLayer) {
            throw std::runtime_error("[PostProcessing.cpp] Layer VK_LAYER_KHRONOS_validation not supported\n");
        }

        for (VkExtensionProperties& prop : m_context.enumerateInstanceExtensionProperties()) {
            auto it = foundExtensions.find(prop.extensionName);
            if (it != foundExtensions.end()) {
                it->second = true;
                enabledExtensions.push_back(it->first);
            }
        }

        for (auto const& fe : foundExtensions) {
            if (!fe.second) {
                throw std::runtime_error("[PostProcessing.cpp] Instance Extension " + std::string(fe.first) + " not supported\n");
            }
        }

        debugCreateInfo = makeDebugUtilsMessengerCreateInfoEXT();
    }

    vk::ApplicationInfo applicationInfo("AppName", 1, "EngineName", 1, VK_API_VERSION_1_2);
    vk::InstanceCreateInfo instanceCreateInfo(
        {},
        &applicationInfo,
        m_enabledLayers,
        enabledExtensions,
        debugCreateInfo.has_value() ? &debugCreateInfo.value() : nullptr);
    m_instance = vk::raii::Instance(m_context, instanceCreateInfo);

    if (enableValidationLayers) {
        m_debugUtilMessenger = m_instance->createDebugUtilsMessengerEXT(makeDebugUtilsMessengerCreateInfoEXT());
    }
}

void PostProcessing::findPhysicalDevice(uint32_t deviceId)
{
    vk::raii::PhysicalDevices physicalDevices(m_instance.value());
    if (physicalDevices.empty()) {
        throw std::runtime_error("[PostProcessing.cpp] could not find a device with vulkan support");
    }
    std::cout << "[PostProcessing.cpp] "
              << "All Physical devices:" << std::endl;
    for (size_t i = 0; i < physicalDevices.size(); i++) {
        auto props = physicalDevices[i].getProperties();
        std::cout << "[PostProcessing.cpp] "
                  << "\t" << i << ". " << props.deviceName;

        switch (props.deviceType) {
        case vk::PhysicalDeviceType::eCpu: {
            std::cout << ", deviceType = CPU";
            break;
        }
        case vk::PhysicalDeviceType::eDiscreteGpu: {
            std::cout << ", deviceType = dGPU";
            break;
        }
        case vk::PhysicalDeviceType::eIntegratedGpu: {
            std::cout << ", deviceType = iGPU";
            break;
        }
        }
        std::cout << std::endl;

        if (deviceId == i) {
            m_physicalDevice = std::move(physicalDevices[i]);
        }
    }

    if (!m_physicalDevice.has_value()) {
        throw std::runtime_error("[PostProcessing.cpp] could not find a VkPhysicalDevice, gpuIndices.vk is out of range");
    }

    std::cout << "[PostProcessing.cpp] "
              << "Selected Device: " << m_physicalDevice->getProperties().deviceName << std::endl;
}

uint32_t PostProcessing::getComputeQueueFamilyIndex()
{
    auto queueFamilies = m_physicalDevice->getQueueFamilyProperties();
    uint32_t i = 0;
    for (; i < queueFamilies.size(); ++i) {
        if (queueFamilies[i].queueCount > 0 && (queueFamilies[i].queueFlags & vk::QueueFlagBits::eCompute)) {
            break;
        }
    }

    if (i == queueFamilies.size()) {
        throw std::runtime_error("[PostProcessing.cpp] could not find a queue family that supports operations");
    }

    return i;
}

void PostProcessing::createDevice()
{
    std::vector<const char*> enabledExtensions;
    std::map<const char*, bool, cmp_str> foundExtensions = {
        { VK_KHR_EXTERNAL_MEMORY_EXTENSION_NAME, false },
        { VK_KHR_EXTERNAL_SEMAPHORE_EXTENSION_NAME, false },
        { VK_KHR_EXTERNAL_MEMORY_WIN32_EXTENSION_NAME, false },
        { VK_KHR_EXTERNAL_SEMAPHORE_WIN32_EXTENSION_NAME, false },
        { VK_EXT_SAMPLER_FILTER_MINMAX_EXTENSION_NAME, false }, //  for hybridpro
        { VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME, false }, // for hybridpro
    };
    for (auto& prop : m_physicalDevice->enumerateDeviceExtensionProperties()) {
        auto it = foundExtensions.find(prop.extensionName);
        if (it != foundExtensions.end()) {
            it->second = true;
            enabledExtensions.push_back(it->first);
        }
    }

    for (auto const& fe : foundExtensions) {
        if (!fe.second) {
            throw std::runtime_error("[PostProcessing.cpp] Device Extension " + std::string(fe.first) + " not supported\n");
        }
    }

    m_queueFamilyIndex = getComputeQueueFamilyIndex();
    float queuePriority = 1.0f;
    vk::DeviceQueueCreateInfo deviceQueueCreateInfo({}, m_queueFamilyIndex, 1, &queuePriority);

    vk::PhysicalDeviceFeatures deviceFeatures;
    deviceFeatures.samplerAnisotropy = true; // for hybridpro

    vk::PhysicalDeviceVulkan12Features features12;
    features12.bufferDeviceAddress = true; // for hybridpro
    features12.samplerFilterMinmax = true; // for hybridpro
    vk::DeviceCreateInfo deviceCreateInfo({}, deviceQueueCreateInfo, m_enabledLayers, enabledExtensions, &deviceFeatures, &features12);
    m_device = m_physicalDevice->createDevice(deviceCreateInfo);
    m_queue = m_device->getQueue(m_queueFamilyIndex, 0);
}

void PostProcessing::createCommandBuffers()
{
    vk::CommandPoolCreateInfo cmdPoolInfo(vk::CommandPoolCreateFlagBits::eResetCommandBuffer, m_queueFamilyIndex);
    m_commandPool = vk::raii::CommandPool(m_device.value(), cmdPoolInfo);

    vk::CommandBufferAllocateInfo allocInfo(*m_commandPool.value(), vk::CommandBufferLevel::ePrimary, 2);
    vk::raii::CommandBuffers commandBuffers(m_device.value(), allocInfo);
    m_secondaryCommandBuffer = std::move(commandBuffers[0]);
    m_computeCommandBuffer = std::move(commandBuffers[1]);
}

void PostProcessing::createShaderModule(const std::filesystem::path& shaderPath)
{
    std::ifstream fglsl(shaderPath);
    std::string computeShaderGlsl((std::istreambuf_iterator<char>(fglsl)), std::istreambuf_iterator<char>());

    shaderc::Compiler compiler;
    shaderc::CompileOptions options;
    options.SetOptimizationLevel(shaderc_optimization_level_performance);
    shaderc::SpvCompilationResult spv = compiler.CompileGlslToSpv(
        computeShaderGlsl,
        shaderc_glsl_compute_shader,
        "shader",
        options);
    if (spv.GetCompilationStatus() != shaderc_compilation_status_success) {
        std::cerr << spv.GetErrorMessage();
    }
    vk::ShaderModuleCreateInfo shaderModuleInfo({}, std::distance(spv.cbegin(), spv.cend()) * sizeof(uint32_t), spv.cbegin());
    m_shaderModule = vk::raii::ShaderModule(m_device.value(), shaderModuleInfo);
}

void PostProcessing::createUbo()
{
    m_uboDirty = true;
    m_stagingUboBuffer = createBuffer(sizeof(UniformBufferObject),
        vk::BufferUsageFlagBits::eTransferSrc,
        vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
    m_uboBuffer = createBuffer(sizeof(UniformBufferObject),
        vk::BufferUsageFlagBits::eUniformBuffer | vk::BufferUsageFlagBits::eTransferDst,
        vk::MemoryPropertyFlagBits::eDeviceLocal);
}

uint32_t PostProcessing::findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties)
{
    vk::PhysicalDeviceMemoryProperties memProperties = m_physicalDevice->getMemoryProperties();
    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    throw std::runtime_error("failed to find suitable memory type!");
}

BindedBuffer PostProcessing::createBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties)
{
    vk::raii::Buffer buffer(m_device.value(), vk::BufferCreateInfo({}, size, usage, vk::SharingMode::eExclusive));

    vk::MemoryRequirements memRequirements = buffer.getMemoryRequirements();
    uint32_t memoryType = findMemoryType(memRequirements.memoryTypeBits, properties);
    auto memory = m_device->allocateMemory(vk::MemoryAllocateInfo(memRequirements.size, memoryType));

    buffer.bindMemory(*memory, 0);

    return { std::move(buffer), std::move(memory) };
}

BindedImage PostProcessing::createImage(uint32_t width,
    uint32_t height,
    vk::Format format,
    vk::ImageUsageFlags usage,
    vk::MemoryPropertyFlags properties)
{
    auto image = m_device->createImage(vk::ImageCreateInfo({},
        vk::ImageType::e2D,
        format,
        { width, height, 1 },
        1,
        1,
        vk::SampleCountFlagBits::e1,
        vk::ImageTiling::eOptimal,
        usage));

    vk::MemoryRequirements memRequirements = image.getMemoryRequirements();
    uint32_t memoryType = findMemoryType(memRequirements.memoryTypeBits, properties);
    auto memory = m_device->allocateMemory(vk::MemoryAllocateInfo(memRequirements.size, memoryType));

    image.bindMemory(*memory, 0);

    auto view = m_device->createImageView(vk::ImageViewCreateInfo({},
        *image,
        vk::ImageViewType::e2D,
        format,
        {},
        { vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 }));

    BindedImage bindedImage = {
        .image = std::move(image),
        .memory = std::move(memory),
        .view = std::move(view),
        .width = width,
        .height = height,
        .access = vk::AccessFlagBits::eNone,
        .layout = vk::ImageLayout::eUndefined,
        .stage = vk::PipelineStageFlagBits::eTopOfPipe
    };
    transitionImageLayout(bindedImage,
        vk::AccessFlagBits::eShaderRead,
        vk::ImageLayout::eGeneral,
        vk::PipelineStageFlagBits::eComputeShader);

    return bindedImage;
}

void PostProcessing::createAovs(uint32_t width, uint32_t height)
{
    m_stagingAovBuffer = createBuffer(width * height * 4 * sizeof(float),
        vk::BufferUsageFlagBits::eTransferSrc,
        vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

    vk::Format aovFormat = vk::Format::eR32G32B32A32Sfloat;
    vk::ImageUsageFlags aovUsage = vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eStorage;
    m_aovs = {
        .color = createImage(width, height, aovFormat, aovUsage, vk::MemoryPropertyFlagBits::eDeviceLocal),
        .opacity = createImage(width, height, aovFormat, aovUsage, vk::MemoryPropertyFlagBits::eDeviceLocal),
        .shadowCatcher = createImage(width, height, aovFormat, aovUsage, vk::MemoryPropertyFlagBits::eDeviceLocal),
        .reflectionCatcher = createImage(width, height, aovFormat, aovUsage, vk::MemoryPropertyFlagBits::eDeviceLocal),
        .mattePass = createImage(width, height, aovFormat, aovUsage, vk::MemoryPropertyFlagBits::eDeviceLocal),
        .background = createImage(width, height, aovFormat, aovUsage, vk::MemoryPropertyFlagBits::eDeviceLocal),
    };
}

void PostProcessing::createOutputDx11Texture(HANDLE sharedDx11TextureHandle, uint32_t width, uint32_t height)
{
    vk::Format format = vk::Format::eR8G8B8A8Unorm;
    vk::ExternalMemoryImageCreateInfo externalMemoryInfo(vk::ExternalMemoryHandleTypeFlagBits::eD3D11Texture);
    vk::ImageCreateInfo imageInfo({},
        vk::ImageType::e2D,
        format,
        { width, height, 1 },
        1,
        1,
        vk::SampleCountFlagBits::e1,
        vk::ImageTiling::eOptimal,
        vk::ImageUsageFlagBits::eStorage,
        vk::SharingMode::eExclusive,
        nullptr,
        vk::ImageLayout::eUndefined,
        &externalMemoryInfo);
    vk::raii::Image image(m_device.value(), imageInfo);

    vk::MemoryDedicatedRequirements memoryDedicatedRequirements;
    vk::MemoryRequirements2 memoryRequirements2({}, &memoryDedicatedRequirements);
    vk::ImageMemoryRequirementsInfo2 imageMemoryRequirementsInfo2(*image);
    (*m_device.value()).getImageMemoryRequirements2(&imageMemoryRequirementsInfo2, &memoryRequirements2);
    vk::MemoryDedicatedAllocateInfo memoryDedicatedAllocateInfo(*image);
    vk::ImportMemoryWin32HandleInfoKHR importMemoryInfo(vk::ExternalMemoryHandleTypeFlagBits::eD3D11Texture,
        sharedDx11TextureHandle,
        nullptr,
        &memoryDedicatedAllocateInfo);
    vk::MemoryAllocateInfo memoryAllocateInfo(memoryRequirements2.memoryRequirements.size,
        findMemoryType(memoryRequirements2.memoryRequirements.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal),
        &importMemoryInfo);
    vk::raii::DeviceMemory memory = m_device->allocateMemory(memoryAllocateInfo);

    image.bindMemory(*memory, 0);

    vk::ImageViewCreateInfo viewInfo({},
        *image,
        vk::ImageViewType::e2D,
        format,
        {},
        { vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 });
    vk::raii::ImageView view(m_device.value(), viewInfo);
    m_outputDx11Texture = {
        .image = std::move(image),
        .memory = std::move(memory),
        .view = std::move(view),
        .width = width,
        .height = height,
        .access = vk::AccessFlagBits::eNone,
        .layout = vk::ImageLayout::eUndefined,
        .stage = vk::PipelineStageFlagBits::eTopOfPipe
    };
    transitionImageLayout(m_outputDx11Texture.value(),
        vk::AccessFlagBits::eShaderWrite | vk::AccessFlagBits::eShaderRead,
        vk::ImageLayout::eGeneral,
        vk::PipelineStageFlagBits::eComputeShader);
}

void PostProcessing::transitionImageLayout(BindedImage& image,
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

    m_secondaryCommandBuffer->begin(vk::CommandBufferBeginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit));
    m_secondaryCommandBuffer->pipelineBarrier(image.stage,
        dstStage,
        {},
        nullptr,
        nullptr,
        imageMemoryBarrier);

    m_secondaryCommandBuffer->end();

    vk::SubmitInfo submitInfo(nullptr, nullptr, *m_secondaryCommandBuffer.value());
    m_queue->submit(submitInfo);
    m_queue->waitIdle();

    image.access = dstAccess;
    image.layout = dstLayout;
    image.stage = dstStage;
}

void PostProcessing::createDescriptorSet()
{
    vk::helper::DescriptorBuilder builder;
    std::vector<vk::DescriptorImageInfo> descriptorImageInfos = {
        vk::DescriptorImageInfo(nullptr, *m_outputDx11Texture->view, vk::ImageLayout::eGeneral), // binding 0
        vk::DescriptorImageInfo(nullptr, *m_aovs->color.view, vk::ImageLayout::eGeneral), // binding 1
        vk::DescriptorImageInfo(nullptr, *m_aovs->opacity.view, vk::ImageLayout::eGeneral), // binding 2
        vk::DescriptorImageInfo(nullptr, *m_aovs->shadowCatcher.view, vk::ImageLayout::eGeneral), // binding 3
        vk::DescriptorImageInfo(nullptr, *m_aovs->reflectionCatcher.view, vk::ImageLayout::eGeneral), // binding 4
        vk::DescriptorImageInfo(nullptr, *m_aovs->mattePass.view, vk::ImageLayout::eGeneral), // binding 5
        vk::DescriptorImageInfo(nullptr, *m_aovs->background.view, vk::ImageLayout::eGeneral), // binding 6
    };

    for (auto& dii : descriptorImageInfos) {
        builder.bindStorageImage(&dii);
    }

    vk::DescriptorBufferInfo uboDescriptoInfo = vk::DescriptorBufferInfo(*m_uboBuffer->buffer, 0, sizeof(UniformBufferObject)); // binding 7
    builder.bindUniformBuffer(&uboDescriptoInfo);

    auto poolSizes = builder.poolSizes();
    m_descriptorSetLayout = m_device->createDescriptorSetLayout(vk::DescriptorSetLayoutCreateInfo({}, builder.bindings));
    m_descriptorPool = m_device->createDescriptorPool(vk::DescriptorPoolCreateInfo(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet, 1, poolSizes));
    m_descriptorSet = std::move(vk::raii::DescriptorSets(m_device.value(), vk::DescriptorSetAllocateInfo(*m_descriptorPool.value(), *m_descriptorSetLayout.value())).front());

    for (auto& w : builder.writes) {
        w.dstSet = *m_descriptorSet.value();
    }
    m_device->updateDescriptorSets(builder.writes, nullptr);
}

void PostProcessing::createComputePipeline()
{
    vk::PipelineLayoutCreateInfo pipelineLayoutInfo({}, *m_descriptorSetLayout.value());
    m_pipelineLayout = vk::raii::PipelineLayout(m_device.value(), pipelineLayoutInfo);

    vk::PipelineShaderStageCreateInfo shaderStageInfo({}, vk::ShaderStageFlagBits::eCompute, *m_shaderModule.value(), "main");
    vk::ComputePipelineCreateInfo pipelineInfo({}, shaderStageInfo, *m_pipelineLayout.value());
    m_computePipeline = m_device->createComputePipeline(nullptr, pipelineInfo);
}

void PostProcessing::recordComputeCommandBuffer(uint32_t width, uint32_t height)
{
    m_computeCommandBuffer->begin({});
    m_computeCommandBuffer->bindPipeline(vk::PipelineBindPoint::eCompute, *m_computePipeline.value());
    m_computeCommandBuffer->bindDescriptorSets(vk::PipelineBindPoint::eCompute,
        *m_pipelineLayout.value(),
        0,
        *m_descriptorSet.value(),
        nullptr);
    m_computeCommandBuffer->dispatch((uint32_t)ceil(width / float(WorkgroupSize)), (uint32_t)ceil(height / float(WorkgroupSize)), 1);
    m_computeCommandBuffer->end();
}

void PostProcessing::updateAov(BindedImage& image, rpr_framebuffer rprfb)
{
    // copy rpr aov to vk staging buffer
    size_t size;
    RPR_CHECK(rprFrameBufferGetInfo(rprfb, RPR_FRAMEBUFFER_DATA, 0, nullptr, &size));
    void* data = m_stagingAovBuffer->memory.mapMemory(0, size, {});
    RPR_CHECK(rprFrameBufferGetInfo(rprfb, RPR_FRAMEBUFFER_DATA, size, data, nullptr));
    m_stagingAovBuffer->memory.unmapMemory();

    // aov image transitions
    transitionImageLayout(image,
        vk::AccessFlagBits::eTransferWrite,
        vk::ImageLayout::eTransferDstOptimal,
        vk::PipelineStageFlagBits::eTransfer);
    {
        m_secondaryCommandBuffer->begin(vk::CommandBufferBeginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit));

        vk::ImageSubresourceLayers imageSubresource(vk::ImageAspectFlagBits::eColor, 0, 0, 1);
        vk::BufferImageCopy region(0, 0, 0, imageSubresource, { 0, 0, 0 }, { image.width, image.height, 1 });
        m_secondaryCommandBuffer->copyBufferToImage(*m_stagingAovBuffer->buffer,
            *image.image,
            vk::ImageLayout::eTransferDstOptimal,
            region);

        m_secondaryCommandBuffer->end();

        vk::SubmitInfo submitInfo(nullptr, nullptr, *m_secondaryCommandBuffer.value());
        m_queue->submit(submitInfo);
        m_queue->waitIdle();
    }
    transitionImageLayout(image,
        vk::AccessFlagBits::eShaderRead,
        vk::ImageLayout::eGeneral,
        vk::PipelineStageFlagBits::eComputeShader);
}

void PostProcessing::updateUbo()
{
    void* data = m_stagingUboBuffer->memory.mapMemory(0, sizeof(UniformBufferObject), {});
    std::memcpy(data, &m_ubo, sizeof(UniformBufferObject));
    m_stagingUboBuffer->memory.unmapMemory();

    m_secondaryCommandBuffer->begin(vk::CommandBufferBeginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit));
    m_secondaryCommandBuffer->copyBuffer(*m_stagingUboBuffer->buffer, *m_uboBuffer->buffer, vk::BufferCopy(0, 0, sizeof(UniformBufferObject)));
    m_secondaryCommandBuffer->end();

    vk::SubmitInfo submitInfo(nullptr, nullptr, *m_secondaryCommandBuffer.value());
    m_queue->submit(submitInfo);
    m_queue->waitIdle();
}

void PostProcessing::resize(HANDLE sharedDx11TextureHandle, uint32_t width, uint32_t height)
{
    if (sharedDx11TextureHandle == nullptr) {
        return;
    }

    if (m_width != width || m_height != height) {
        m_computePipeline.reset();
        m_pipelineLayout.reset();
        m_descriptorSet.reset();
        m_descriptorPool.reset();
        m_descriptorSetLayout.reset();
        m_aovs.reset();
        m_outputDx11Texture.reset();
        m_stagingAovBuffer.reset();
        createAovs(width, height);
        createOutputDx11Texture(sharedDx11TextureHandle, width, height);
        createDescriptorSet();
        createComputePipeline();
        recordComputeCommandBuffer(width, height);
        m_width = width;
        m_height = height;
    }
}

void PostProcessing::run()
{
    if (m_uboDirty) {
        updateUbo();
        m_uboDirty = false;
    }

    vk::SubmitInfo submitInfo(nullptr, nullptr, *m_computeCommandBuffer.value());
    m_queue->submit(submitInfo);
    m_queue->waitIdle();
}