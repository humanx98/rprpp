#include "vk_helper.hpp"
#include "common.hpp"
#include <map>
#include <iostream>

namespace vk::helper {

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

std::pair<vk::raii::Instance, std::vector<const char*>> createInstance(const vk::raii::Context& context, bool enableValidationLayers)
{
    std::vector<const char*> enabledExtensions;
    std::vector<const char*> enabledLayers;
    std::map<const char*, bool, cmp_str> foundExtensions = {
        { VK_EXT_DEBUG_UTILS_EXTENSION_NAME, false },
        { VK_KHR_EXTERNAL_MEMORY_CAPABILITIES_EXTENSION_NAME, false },
        { VK_KHR_EXTERNAL_SEMAPHORE_CAPABILITIES_EXTENSION_NAME, false },
    };
    std::optional<vk::DebugUtilsMessengerCreateInfoEXT> debugCreateInfo;
    if (enableValidationLayers) {
        bool foundValidationLayer = false;
        for (auto& prop : context.enumerateInstanceLayerProperties()) {
            if (strcmp("VK_LAYER_KHRONOS_validation", prop.layerName) == 0) {
                foundValidationLayer = true;
                enabledLayers.push_back("VK_LAYER_KHRONOS_validation");
                break;
            }
        }

        if (!foundValidationLayer) {
            throw std::runtime_error("[PostProcessing.cpp] Layer VK_LAYER_KHRONOS_validation not supported\n");
        }

        for (VkExtensionProperties& prop : context.enumerateInstanceExtensionProperties()) {
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
        enabledLayers,
        enabledExtensions,
        debugCreateInfo.has_value() ? &debugCreateInfo.value() : nullptr);
    return std::make_pair(vk::raii::Instance(context, instanceCreateInfo), enabledLayers);
}

vk::raii::PhysicalDevice findPhysicalDevice(const vk::raii::Instance& instance, uint32_t deviceId)
{
    vk::raii::PhysicalDevices physicalDevices(instance);
    if (physicalDevices.empty()) {
        throw std::runtime_error("[PostProcessing.cpp] could not find a device with vulkan support");
    }
    std::cout << "[PostProcessing.cpp] "
              << "All Physical devices:" << std::endl;
    std::optional<vk::raii::PhysicalDevice> physicalDevice;
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
            physicalDevice = std::move(physicalDevices[i]);
        }
    }

    if (!physicalDevice.has_value()) {
        throw std::runtime_error("[PostProcessing.cpp] could not find a VkPhysicalDevice, gpuIndices.vk is out of range");
    }

    std::cout << "[PostProcessing.cpp] "
              << "Selected Device: " << physicalDevice->getProperties().deviceName << std::endl;

    return physicalDevice.value();
}

vk::raii::Device createDevice(const vk::raii::PhysicalDevice& physicalDevice, 
    const std::vector<const char*>& enabledLayers, 
    const std::vector<vk::DeviceQueueCreateInfo>& queueInfos)
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
    for (auto& prop : physicalDevice.enumerateDeviceExtensionProperties()) {
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

    vk::PhysicalDeviceFeatures deviceFeatures;
    deviceFeatures.samplerAnisotropy = true; // for hybridpro
    vk::PhysicalDeviceVulkan12Features features12;
    features12.bufferDeviceAddress = true; // for hybridpro
    features12.samplerFilterMinmax = true; // for hybridpro
    vk::DeviceCreateInfo deviceCreateInfo({}, queueInfos, enabledLayers, enabledExtensions, &deviceFeatures, &features12);
    return physicalDevice.createDevice(deviceCreateInfo);
}

DeviceContext createDeviceContext(bool enableValidationLayers, uint32_t deviceId)
{
    vk::raii::Context context;
    auto instanceAndValidationLayers = createInstance(context, enableValidationLayers);
    vk::raii::Instance instance = std::move(instanceAndValidationLayers.first);
    auto enabledLayers = std::move(instanceAndValidationLayers.second);
    std::optional<vk::raii::DebugUtilsMessengerEXT> debugUtilMessenger;
    if (enableValidationLayers) {
        debugUtilMessenger = instance.createDebugUtilsMessengerEXT(makeDebugUtilsMessengerCreateInfoEXT());
    }

    vk::raii::PhysicalDevice physicalDevice = findPhysicalDevice(instance, deviceId);

    auto queueFamilies = physicalDevice.getQueueFamilyProperties();
    uint32_t queueFamilyIndex = 0;
    for (; queueFamilyIndex < queueFamilies.size(); ++queueFamilyIndex) {
        if (queueFamilies[queueFamilyIndex].queueCount > 0 
            && (queueFamilies[queueFamilyIndex].queueFlags & vk::QueueFlagBits::eCompute)
            && (queueFamilies[queueFamilyIndex].queueFlags & vk::QueueFlagBits::eTransfer)) {
            break;
        }
    }

    if (queueFamilyIndex == queueFamilies.size()) {
        throw std::runtime_error("[PostProcessing.cpp] could not find a queue family that supports operations");
    }

    float queuePriority = 1.0f;
    vk::raii::Device device = createDevice(physicalDevice, 
        enabledLayers, 
        { vk::DeviceQueueCreateInfo({}, queueFamilyIndex, 1, &queuePriority) });
    vk::raii::Queue queue = device.getQueue(queueFamilyIndex, 0);

    return {  
        std::move(context),
        std::move(instance),
        std::move(debugUtilMessenger),
        std::move(physicalDevice),
        std::move(device),
        std::move(queue),
        queueFamilyIndex
    };
}

uint32_t findMemoryType(const vk::raii::PhysicalDevice physicalDevice, uint32_t typeFilter, vk::MemoryPropertyFlags properties)
{
    vk::PhysicalDeviceMemoryProperties memProperties = physicalDevice.getMemoryProperties();
    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    throw std::runtime_error("failed to find suitable memory type!");
}

vk::raii::DeviceMemory allocateImageMemory(const DeviceContext& dctx, 
    const vk::raii::Image& image, 
    HANDLE sharedDx11TextureHandle)
{
    if (sharedDx11TextureHandle != nullptr) {
        vk::MemoryDedicatedRequirements memoryDedicatedRequirements;
        vk::MemoryRequirements2 memoryRequirements2({}, &memoryDedicatedRequirements);
        vk::ImageMemoryRequirementsInfo2 imageMemoryRequirementsInfo2(*image);
        (*dctx.device).getImageMemoryRequirements2(&imageMemoryRequirementsInfo2, &memoryRequirements2);
        vk::MemoryDedicatedAllocateInfo memoryDedicatedAllocateInfo(*image);
        vk::ImportMemoryWin32HandleInfoKHR importMemoryInfo(vk::ExternalMemoryHandleTypeFlagBits::eD3D11Texture,
            sharedDx11TextureHandle,
            nullptr,
            &memoryDedicatedAllocateInfo);
        uint32_t memoryType = findMemoryType(dctx.physicalDevice, memoryRequirements2.memoryRequirements.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal);
        vk::MemoryAllocateInfo memoryAllocateInfo(memoryRequirements2.memoryRequirements.size,
            memoryType,
            &importMemoryInfo);
        return dctx.device.allocateMemory(memoryAllocateInfo);
    } else {
        vk::MemoryRequirements memRequirements = image.getMemoryRequirements();
        uint32_t memoryType = findMemoryType(dctx.physicalDevice, memRequirements.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal);
        return dctx.device.allocateMemory(vk::MemoryAllocateInfo(memRequirements.size, memoryType));
    }
}

Image createImage(const DeviceContext& dctx,
    uint32_t width,
    uint32_t height,
    vk::Format format,
    vk::ImageUsageFlags usage,
    HANDLE sharedDx11TextureHandle)
{
    std::optional<vk::ExternalMemoryImageCreateInfo> externalMemoryInfo;
    if (sharedDx11TextureHandle != nullptr) {
        externalMemoryInfo = vk::ExternalMemoryImageCreateInfo(vk::ExternalMemoryHandleTypeFlagBits::eD3D11Texture);
    }

    vk::ImageCreateInfo imageInfo({},
        vk::ImageType::e2D,
        format,
        { width, height, 1 },
        1,
        1,
        vk::SampleCountFlagBits::e1,
        vk::ImageTiling::eOptimal,
        usage,
        vk::SharingMode::eExclusive,
        nullptr,
        vk::ImageLayout::eUndefined,
        externalMemoryInfo.has_value() ? &externalMemoryInfo.value() : nullptr);
    vk::raii::Image vkimage(dctx.device, imageInfo);

    auto memory = allocateImageMemory(dctx, vkimage, sharedDx11TextureHandle);
    vkimage.bindMemory(*memory, 0);

    vk::ImageViewCreateInfo viewInfo({},
        *vkimage,
        vk::ImageViewType::e2D,
        format,
        {},
        { vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 });
    vk::raii::ImageView view(dctx.device, viewInfo);

    Image image = {
        .image = std::move(vkimage),
        .memory = std::move(memory),
        .view = std::move(view),
        .width = width,
        .height = height,
        .access = vk::AccessFlagBits::eNone,
        .layout = vk::ImageLayout::eUndefined,
        .stage = vk::PipelineStageFlagBits::eTopOfPipe
    };

    return image;
}

Buffer createBuffer(const DeviceContext& dctx,
    vk::DeviceSize size,
    vk::BufferUsageFlags usage,
    vk::MemoryPropertyFlags properties)
{
    vk::raii::Buffer buffer(dctx.device, vk::BufferCreateInfo({}, size, usage, vk::SharingMode::eExclusive));

    vk::MemoryRequirements memRequirements = buffer.getMemoryRequirements();
    uint32_t memoryType = findMemoryType(dctx.physicalDevice, memRequirements.memoryTypeBits, properties);
    auto memory = dctx.device.allocateMemory(vk::MemoryAllocateInfo(memRequirements.size, memoryType));

    buffer.bindMemory(*memory, 0);

    return { std::move(buffer), std::move(memory) };
}

}