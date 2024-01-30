#define VK_USE_PLATFORM_WIN32_KHR

#include "PostProcessing.hpp"
#include "vk_helper.hpp"
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

PostProcessing::PostProcessing(const Paths& paths,
    HANDLE sharedDx11TextureHandle,
    bool enableValidationLayers,
    unsigned int width,
    unsigned int height,
    GpuIndices gpuIndices)
{
    m_enableValidationLayers = enableValidationLayers;
    m_width = width;
    m_height = height;
    createInstance();
    findPhysicalDevice(gpuIndices);
    createDevice();
    createShaderModule(paths);
    createCommandBuffer();
    // createAovs();
    createOutputDx11Texture(sharedDx11TextureHandle);
    createDescriptorSets();
    createComputePipeline();
}

void PostProcessing::createInstance()
{
    std::vector<const char*> enabledExtensions;
    std::map<const char*, bool, cmp_str> foundExtensions = {
        { VK_EXT_DEBUG_UTILS_EXTENSION_NAME, false },
        { VK_KHR_EXTERNAL_MEMORY_CAPABILITIES_EXTENSION_NAME, false },
        { VK_KHR_EXTERNAL_SEMAPHORE_CAPABILITIES_EXTENSION_NAME, false },
    };
    std::optional<vk::DebugUtilsMessengerCreateInfoEXT> debugCreateInfo;
    if (m_enableValidationLayers) {
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

    if (m_enableValidationLayers) {
        m_debugUtilMessenger = m_instance.value().createDebugUtilsMessengerEXT(makeDebugUtilsMessengerCreateInfoEXT());
    }
}

void PostProcessing::findPhysicalDevice(GpuIndices gpuIndices)
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

        if (gpuIndices.vk == i) {
            m_physicalDevice = std::move(physicalDevices[i]);
        }
    }

    if (!m_physicalDevice.has_value()) {
        throw std::runtime_error("[PostProcessing.cpp] could not find a VkPhysicalDevice, gpuIndices.vk is out of range");
    }

    std::cout << "[PostProcessing.cpp] "
              << "Selected Device: " << m_physicalDevice.value().getProperties().deviceName << std::endl;
}

uint32_t PostProcessing::getComputeQueueFamilyIndex()
{
    auto queueFamilies = m_physicalDevice.value().getQueueFamilyProperties();
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
        { VK_EXT_SAMPLER_FILTER_MINMAX_EXTENSION_NAME, false },
        { VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME, false },
    };
    for (auto& prop : m_physicalDevice.value().enumerateDeviceExtensionProperties()) {
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
    deviceFeatures.samplerAnisotropy = true;

    vk::PhysicalDeviceVulkan12Features features12;
    features12.bufferDeviceAddress = true;
    features12.samplerFilterMinmax = true;
    vk::DeviceCreateInfo deviceCreateInfo({}, deviceQueueCreateInfo, m_enabledLayers, enabledExtensions, &deviceFeatures, &features12);
    m_device = m_physicalDevice.value().createDevice(deviceCreateInfo);
    m_queue = m_device.value().getQueue(m_queueFamilyIndex, 0);
}

void PostProcessing::createCommandBuffer()
{
    vk::CommandPoolCreateInfo cmdPoolInfo(vk::CommandPoolCreateFlagBits::eResetCommandBuffer, m_queueFamilyIndex);
    m_commandPool = vk::raii::CommandPool(m_device.value(), cmdPoolInfo);

    vk::CommandBufferAllocateInfo allocInfo(*m_commandPool.value(), vk::CommandBufferLevel::ePrimary, 1);
    m_commandBuffer = std::move(vk::raii::CommandBuffers(m_device.value(), allocInfo).front());
}

void PostProcessing::createShaderModule(const Paths& paths)
{
    std::ifstream fglsl(paths.postprocessingGlsl);
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

uint32_t PostProcessing::findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties)
{
    vk::PhysicalDeviceMemoryProperties memProperties = m_physicalDevice.value().getMemoryProperties();
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
    auto memory = m_device.value().allocateMemory(vk::MemoryAllocateInfo(memRequirements.size, memoryType));

    buffer.bindMemory(*memory, 0);

    return { std::move(buffer), std::move(memory) };
}

BindedImage PostProcessing::createImage(uint32_t width,
    uint32_t height,
    vk::Format format,
    vk::ImageUsageFlags usage,
    vk::MemoryPropertyFlags properties)
{
    auto image = m_device.value().createImage(vk::ImageCreateInfo({},
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
    auto memory = m_device.value().allocateMemory(vk::MemoryAllocateInfo(memRequirements.size, memoryType));

    image.bindMemory(*memory, 0);

    auto view = m_device.value().createImageView(vk::ImageViewCreateInfo({},
        *image,
        vk::ImageViewType::e2D,
        format,
        {},
        { vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 }));

    return { std::move(image), std::move(memory), std::move(view) };
}

void PostProcessing::createAovs()
{
    m_stagingAovBuffer = createBuffer(m_width * m_height * 4 * sizeof(float),
        vk::BufferUsageFlagBits::eTransferSrc,
        vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

    vk::Format aovFormat = vk::Format::eR32G32B32A32Sfloat;
    vk::ImageUsageFlags aovUsage = vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eStorage;
    m_aovs = {
        .color = createImage(m_width, m_height, aovFormat, aovUsage, vk::MemoryPropertyFlagBits::eDeviceLocal),
        .opacity = createImage(m_width, m_height, aovFormat, aovUsage, vk::MemoryPropertyFlagBits::eDeviceLocal),
        .shadowCatcher = createImage(m_width, m_height, aovFormat, aovUsage, vk::MemoryPropertyFlagBits::eDeviceLocal),
        .reflectionCatcher = createImage(m_width, m_height, aovFormat, aovUsage, vk::MemoryPropertyFlagBits::eDeviceLocal),
        .mattePass = createImage(m_width, m_height, aovFormat, aovUsage, vk::MemoryPropertyFlagBits::eDeviceLocal),
        .background = createImage(m_width, m_height, aovFormat, aovUsage, vk::MemoryPropertyFlagBits::eDeviceLocal),
    };
}

void PostProcessing::createOutputDx11Texture(HANDLE sharedDx11TextureHandle)
{
    vk::Format format = vk::Format::eR8G8B8A8Unorm;
    vk::ExternalMemoryImageCreateInfo externalMemoryInfo(vk::ExternalMemoryHandleTypeFlagBits::eD3D11Texture);
    vk::ImageCreateInfo imageInfo({},
        vk::ImageType::e2D,
        format,
        { m_width, m_height, 1 },
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
    vk::raii::DeviceMemory memory = m_device.value().allocateMemory(memoryAllocateInfo);

    image.bindMemory(*memory, 0);

    vk::ImageViewCreateInfo viewInfo({},
        *image,
        vk::ImageViewType::e2D,
        format,
        {},
        { vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 });
    vk::raii::ImageView view(m_device.value(), viewInfo);

    transitionImageLayout(image, vk::ImageLayout::eUndefined, vk::ImageLayout::eGeneral);
    m_outputDx11Texture = { std::move(image), std::move(memory), std::move(view) };
}

void PostProcessing::transitionImageLayout(const vk::raii::Image& image, vk::ImageLayout oldLayout, vk::ImageLayout newLayout)
{
    m_commandBuffer.value().begin(vk::CommandBufferBeginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit));

    vk::ImageSubresourceRange imageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1);
    vk::ImageMemoryBarrier imageMemoryBarrier(vk::AccessFlagBits::eNone,
        vk::AccessFlagBits::eShaderRead,
        oldLayout,
        newLayout,
        VK_QUEUE_FAMILY_IGNORED,
        VK_QUEUE_FAMILY_IGNORED,
        *image,
        imageSubresourceRange);
    m_commandBuffer.value().pipelineBarrier(vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eComputeShader, {}, nullptr, nullptr, imageMemoryBarrier);

    m_commandBuffer.value().end();

    vk::SubmitInfo submitInfo(nullptr, nullptr, *m_commandBuffer.value());
    m_queue.value().submit(submitInfo);
    m_queue.value().waitIdle();
}

void PostProcessing::createDescriptorSets()
{
    vk::DescriptorImageInfo descriptorImageInfo(nullptr, *m_outputDx11Texture.value().view, vk::ImageLayout::eGeneral);
    vk::helper::DescriptorBuilder builder;
    builder.bindStorageImage(&descriptorImageInfo);
    /*builder.bindStorageImage();
    builder.bindStorageImage();
    builder.bindStorageImage();*/

    auto poolSizes = builder.poolSizes();
    m_descriptorSetLayout = m_device.value().createDescriptorSetLayout(vk::DescriptorSetLayoutCreateInfo({}, builder.bindings));
    m_descriptorPool = m_device.value().createDescriptorPool(vk::DescriptorPoolCreateInfo(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet, 1, poolSizes));
    m_descriptorSet = std::move(vk::raii::DescriptorSets(m_device.value(), vk::DescriptorSetAllocateInfo(*m_descriptorPool.value(), *m_descriptorSetLayout.value())).front());

    for (auto& w : builder.writes) {
        w.dstSet = *m_descriptorSet.value();
    }
    m_device.value().updateDescriptorSets(builder.writes, nullptr);
}

void PostProcessing::createComputePipeline()
{
    vk::PipelineLayoutCreateInfo pipelineLayoutInfo({}, *m_descriptorSetLayout.value());
    m_pipelineLayout = vk::raii::PipelineLayout(m_device.value(), pipelineLayoutInfo);

    vk::PipelineShaderStageCreateInfo shaderStageInfo({}, vk::ShaderStageFlagBits::eCompute, *m_shaderModule.value(), "main");
    vk::ComputePipelineCreateInfo pipelineInfo({}, shaderStageInfo, *m_pipelineLayout.value());
    m_computePipeline = m_device.value().createComputePipeline(nullptr, pipelineInfo);
}

void PostProcessing::updateAovColor(const std::vector<uint8_t>& aovbuff)
{
}

void PostProcessing::updateAovOpacity(const std::vector<uint8_t>& aovbuff)
{
}

void PostProcessing::updateAovShadowCatcher(const std::vector<uint8_t>& aovbuff)
{
}

void PostProcessing::updateAovReflectionCatcher(const std::vector<uint8_t>& aovbuff)
{
}

void PostProcessing::updateAovMattePass(const std::vector<uint8_t>& aovbuff)
{
}

void PostProcessing::updateAovBackground(const std::vector<uint8_t>& aovbuff)
{
}

void PostProcessing::apply()
{
    m_commandBuffer.value().begin({});
    m_commandBuffer.value().bindPipeline(vk::PipelineBindPoint::eCompute, *m_computePipeline.value());
    m_commandBuffer.value().bindDescriptorSets(vk::PipelineBindPoint::eCompute,
        *m_pipelineLayout.value(),
        0,
        *m_descriptorSet.value(),
        nullptr);
    m_commandBuffer.value().dispatch((uint32_t)ceil(m_width / float(WorkgroupSize)), (uint32_t)ceil(m_height / float(WorkgroupSize)), 1);
    m_commandBuffer.value().end();

    vk::SubmitInfo submitInfo(nullptr, nullptr, *m_commandBuffer.value());
    m_queue.value().submit(submitInfo);
    m_queue.value().waitIdle();
}