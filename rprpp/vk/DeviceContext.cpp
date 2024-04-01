#include "DeviceContext.h"
#include "rprpp/Error.h"
#include <iostream>

namespace vk::helper {

namespace {

    struct VulkanInstance {
        vk::raii::Instance instance;
        std::vector<const char*> enabledLayers;
    };

    VKAPI_ATTR VkBool32 debugUtilsMessengerCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
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

    bool validateRequiredExtensions(const std::vector<const char*>& extensions, const std::vector<const char*>& requiredExtensions)
    {
        // validate that all required extensions are present in extensions container
        return std::all_of(requiredExtensions.begin(), requiredExtensions.end(), [&extensions](const char* requiredExtensionName) {
            auto sameExtensionNames = [&requiredExtensionName](const char* extensionName) -> bool {
                return std::strcmp(requiredExtensionName, extensionName) == 0;
            };

            auto iter = std::find_if(extensions.begin(), extensions.end(), sameExtensionNames);
            return iter != extensions.end();
        });
    } // validateReqFunction

    VulkanInstance createInstance(const vk::raii::Context& context, bool enableValidationLayers)
    {
        static const std::vector<const char*> requiredExtensions {
            VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
            VK_KHR_EXTERNAL_MEMORY_CAPABILITIES_EXTENSION_NAME,
            VK_KHR_EXTERNAL_SEMAPHORE_CAPABILITIES_EXTENSION_NAME
        };

        std::vector<const char*> availableExtensions;
        std::vector<const char*> enabledLayers;

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
                throw rprpp::InternalError("Layer VK_LAYER_KHRONOS_validation not supported\n");
            }

            debugCreateInfo = makeDebugUtilsMessengerCreateInfoEXT();
        }
        const auto& extensionProperties = context.enumerateInstanceExtensionProperties();
        availableExtensions.reserve(extensionProperties.size());

        for (const VkExtensionProperties& prop : extensionProperties) {
            availableExtensions.push_back(prop.extensionName);
        }

        if (!validateRequiredExtensions(availableExtensions, requiredExtensions)) {
            throw rprpp::InternalError("Instance Required Extensions not found");
        }

        vk::ApplicationInfo applicationInfo("AppName", 1, "EngineName", 1, VK_API_VERSION_1_2);
        vk::InstanceCreateInfo instanceCreateInfo(
            {},
            &applicationInfo,
            enabledLayers,
            requiredExtensions,
            debugCreateInfo.has_value() ? &debugCreateInfo.value() : nullptr);

        return {
            .instance = vk::raii::Instance(context, instanceCreateInfo),
            .enabledLayers = enabledLayers
        };
    } // createInstance

    std::vector<const char*> getRayTracingExtensions()
    {
        return {
            VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME,
            VK_KHR_PIPELINE_LIBRARY_EXTENSION_NAME,
            VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME,
            VK_KHR_RAY_QUERY_EXTENSION_NAME,
        };
    }

    vk::raii::Device createDevice(const vk::raii::PhysicalDevice& physicalDevice,
        const std::vector<const char*>& enabledLayers,
        const std::vector<vk::DeviceQueueCreateInfo>& queueInfos)
    {
        std::vector<const char*> requiredExtensions {
            VK_KHR_EXTERNAL_MEMORY_EXTENSION_NAME,
            // VK_KHR_EXTERNAL_SEMAPHORE_EXTENSION_NAME, // we don't use it right now
            VK_KHR_EXTERNAL_MEMORY_WIN32_EXTENSION_NAME,
            // VK_KHR_EXTERNAL_SEMAPHORE_WIN32_EXTENSION_NAME, // we don't use it right now
            VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME,

            // for hybridpro
            VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME,
            VK_KHR_DRAW_INDIRECT_COUNT_EXTENSION_NAME,
            VK_EXT_SHADER_SUBGROUP_BALLOT_EXTENSION_NAME,
            VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME,
            VK_KHR_MAINTENANCE3_EXTENSION_NAME,
            VK_KHR_SPIRV_1_4_EXTENSION_NAME,
        };

        //  for hybridpro
        std::vector<const char*> rayTracingExtensions = getRayTracingExtensions();

        std::vector<const char*> availableExtensions;

        const std::vector<vk::ExtensionProperties> extensionProperties = physicalDevice.enumerateDeviceExtensionProperties();
        availableExtensions.reserve(extensionProperties.size());
        for (const vk::ExtensionProperties& property : extensionProperties) {
            availableExtensions.push_back(property.extensionName);
        }

        if (!validateRequiredExtensions(availableExtensions, requiredExtensions)) {
            throw rprpp::InternalError("Physical Device Required Extensions not found");
        }

        bool supportHardwareRT = validateRequiredExtensions(availableExtensions, rayTracingExtensions);
        if (supportHardwareRT) {
            requiredExtensions.insert(requiredExtensions.end(), rayTracingExtensions.begin(), rayTracingExtensions.end());
        }

        auto supportedFeatures = physicalDevice.getFeatures2<
            vk::PhysicalDeviceFeatures2,
            vk::PhysicalDeviceVulkan11Features,
            vk::PhysicalDeviceVulkan12Features,
            vk::PhysicalDeviceAccelerationStructureFeaturesKHR,
            vk::PhysicalDeviceRayQueryFeaturesKHR>();

        vk::PhysicalDeviceAccelerationStructureFeaturesKHR accelerationStructureFeatures;
        accelerationStructureFeatures.accelerationStructure = supportedFeatures.get<vk::PhysicalDeviceAccelerationStructureFeaturesKHR>().accelerationStructure;

        vk::PhysicalDeviceRayQueryFeaturesKHR rayQueryFetures;
        rayQueryFetures.rayQuery = supportedFeatures.get<vk::PhysicalDeviceRayQueryFeaturesKHR>().rayQuery;

        vk::PhysicalDeviceVulkan12Features features12;
        features12.drawIndirectCount = vk::True;
        features12.shaderFloat16 = supportedFeatures.get<vk::PhysicalDeviceVulkan12Features>().shaderFloat16;
        features12.descriptorIndexing = vk::True;
        features12.shaderSampledImageArrayNonUniformIndexing = vk::True;
        features12.shaderStorageBufferArrayNonUniformIndexing = vk::True;
        features12.samplerFilterMinmax = supportedFeatures.get<vk::PhysicalDeviceVulkan12Features>().samplerFilterMinmax;
        features12.bufferDeviceAddress = vk::True;

        vk::PhysicalDeviceVulkan11Features features11;
        features11.storageBuffer16BitAccess = supportedFeatures.get<vk::PhysicalDeviceVulkan11Features>().storageBuffer16BitAccess;

        vk::PhysicalDeviceFeatures2 features2;
        features2.features.independentBlend = vk::True;
        features2.features.geometryShader = vk::True;
        features2.features.multiDrawIndirect = vk::True;
        features2.features.wideLines = vk::True;
        features2.features.samplerAnisotropy = vk::True;
        features2.features.vertexPipelineStoresAndAtomics = vk::True;
        features2.features.fragmentStoresAndAtomics = vk::True;
        features2.features.shaderStorageImageExtendedFormats = vk::True;
        features2.features.shaderFloat64 = vk::True;
        features2.features.shaderInt64 = supportedFeatures.get<vk::PhysicalDeviceFeatures2>().features.shaderInt64;
        features2.features.shaderInt16 = vk::True;

        accelerationStructureFeatures.pNext = &rayQueryFetures;
        rayQueryFetures.pNext = &features12;
        features12.pNext = &features11;
        features11.pNext = &features2;

        vk::DeviceCreateInfo deviceCreateInfo({},
            queueInfos,
            enabledLayers,
            requiredExtensions,
            nullptr,
            supportHardwareRT ? (void*)&accelerationStructureFeatures : (void*)&features12);
        return physicalDevice.createDevice(deviceCreateInfo);
    }
} // namespace

// -------------------------------------------------------------------
// Public functions implementations
// -------------------------------------------------------------------

uint32_t findMemoryType(const vk::raii::PhysicalDevice& physicalDevice, uint32_t typeFilter, vk::MemoryPropertyFlags properties)
{
    vk::PhysicalDeviceMemoryProperties memProperties = physicalDevice.getMemoryProperties();
    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    throw rprpp::InternalError("Failed to find suitable memory type!");
}

uint32_t getDeviceCount()
{
    vk::raii::Context context;
    VulkanInstance vulkanInstance = createInstance(context, false);
    vk::raii::PhysicalDevices physicalDevices(vulkanInstance.instance);
    return physicalDevices.size();
}

void getDeviceInfo(uint32_t deviceId, DeviceInfo info, void* data, size_t size, size_t* sizeRet)
{
    vk::raii::Context context;
    VulkanInstance vulkanInstance = createInstance(context, false);
    vk::raii::PhysicalDevices physicalDevices(vulkanInstance.instance);

    if (physicalDevices.size() <= deviceId) {
        throw rprpp::InvalidDevice(deviceId);
    }

    vk::raii::PhysicalDevice physicalDevice = std::move(physicalDevices[deviceId]);

    auto props2 = physicalDevice.getProperties2<vk::PhysicalDeviceProperties2, vk::PhysicalDeviceIDProperties>();
    vk::PhysicalDeviceProperties props = props2.get<vk::PhysicalDeviceProperties2>().properties;
    vk::PhysicalDeviceIDProperties idprops = props2.get<vk::PhysicalDeviceIDProperties>();

    switch (info) {
    case DeviceInfo::eName: {
        size_t len = std::strlen(props.deviceName) + 1;
        if (sizeRet != nullptr) {
            *sizeRet = len;
        }

        if (data != nullptr && size <= len) {
            std::strcpy((char*)data, props.deviceName);
        }
        break;
    }
    case DeviceInfo::eLUID: {
        size_t len = VK_LUID_SIZE;
        if (sizeRet != nullptr) {
            *sizeRet = len;
        }

        if (data != nullptr && size <= len) {
            std::memcpy(data, idprops.deviceLUID, len);
        }
        break;
    }
    case DeviceInfo::eSupportHardwareRayTracing: {
        size_t len = sizeof(uint32_t);
        if (sizeRet != nullptr) {
            *sizeRet = len;
        }

        if (data != nullptr && size <= len) {
            std::vector<const char*> availableExtensions;
            const std::vector<vk::ExtensionProperties> extensionProperties = physicalDevice.enumerateDeviceExtensionProperties();
            availableExtensions.reserve(extensionProperties.size());
            for (const vk::ExtensionProperties& property : extensionProperties) {
                availableExtensions.push_back(property.extensionName);
            }
            std::vector<const char*> rayTracingExtensions = getRayTracingExtensions();
            uint32_t supportHardwareRT = validateRequiredExtensions(availableExtensions, rayTracingExtensions)
                ? RPRPP_TRUE
                : RPRPP_FALSE;
            std::memcpy(data, &supportHardwareRT, len);
        }
        break;
    }
    default:
        throw rprpp::InvalidParameter("deviceInfo", "Not supported device info type");
    }
}

DeviceContext createDeviceContext(uint32_t deviceId)
{
#if NDEBUG
    const bool enableValidationLayers = false;
#else
    const bool enableValidationLayers = true;
#endif

    vk::raii::Context context;
    VulkanInstance vulkanInstance = createInstance(context, enableValidationLayers);
    std::optional<vk::raii::DebugUtilsMessengerEXT> debugUtilMessenger;
    if (enableValidationLayers) {
        debugUtilMessenger = vulkanInstance.instance.createDebugUtilsMessengerEXT(makeDebugUtilsMessengerCreateInfoEXT());
    }

    vk::raii::PhysicalDevices physicalDevices(vulkanInstance.instance);
    if (physicalDevices.size() <= deviceId) {
        throw rprpp::InvalidDevice(deviceId);
    }
    vk::raii::PhysicalDevice physicalDevice = std::move(physicalDevices[deviceId]);

    std::optional<uint32_t> computeQueueFamilyIndex;
    bool computeOnGraphics = false;
    std::vector<vk::DeviceQueueCreateInfo> queueInfos;
    float queuePriority = 1.0f;
    auto queueFamilies = physicalDevice.getQueueFamilyProperties();
    for (uint32_t i = 0; i < queueFamilies.size(); ++i) {
        queueInfos.push_back(vk::DeviceQueueCreateInfo({}, i, 1, &queuePriority));
        bool supportsCompute = queueFamilies[i].queueCount > 0 && queueFamilies[i].queueFlags & vk::QueueFlagBits::eCompute;
        if (supportsCompute && !computeQueueFamilyIndex.has_value()) {
            computeQueueFamilyIndex = i;
            computeOnGraphics = (queueFamilies[i].queueFlags & vk::QueueFlagBits::eGraphics) == vk::QueueFlagBits::eGraphics;
        } else if (supportsCompute && computeQueueFamilyIndex.has_value() && computeOnGraphics) {
            // prefer to use compute queue without graphics support
            computeQueueFamilyIndex = i;
            computeOnGraphics = false;
        }
    }

    if (!computeQueueFamilyIndex.has_value()) {
        throw rprpp::InternalError("Could not find a queue family that supports compute and transfer operations");
    }

    vk::raii::Device device = createDevice(physicalDevice,
        vulkanInstance.enabledLayers,
        queueInfos);
    vk::raii::Queue queue = device.getQueue(computeQueueFamilyIndex.value(), 0);

    return {
        std::move(context),
        std::move(vulkanInstance.instance),
        std::move(debugUtilMessenger),
        std::move(physicalDevice),
        std::move(device),
        std::move(queue),
        computeQueueFamilyIndex.value()
    };
}

vk::raii::CommandBuffer DeviceContext::takeCommandBuffer()
{
    constexpr uint32_t numberOfPreallocatedBuffers = 12;
    if (commandBuffers.empty()) {
        vk::CommandPoolCreateInfo cmdPoolInfo(vk::CommandPoolCreateFlagBits::eResetCommandBuffer, queueFamilyIndex);
        vk::raii::CommandPool pool(device, cmdPoolInfo);

        vk::CommandBufferAllocateInfo allocInfo(*pool, vk::CommandBufferLevel::ePrimary, numberOfPreallocatedBuffers);
        vk::raii::CommandBuffers buffers(device, allocInfo);
        commandBuffers.insert(commandBuffers.end(), std::move_iterator(buffers.begin()), std::move_iterator(buffers.end()));
        commandPools.push_back(std::move(pool));
    }

    vk::raii::CommandBuffer buf = std::move(commandBuffers.back());
    commandBuffers.pop_back();
    return buf;
}

void DeviceContext::returnCommandBuffer(vk::raii::CommandBuffer buffer)
{
    commandBuffers.push_back(std::move(buffer));
}

}
