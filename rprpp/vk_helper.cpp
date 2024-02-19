#include "vk_helper.h"
#include "Error.h"
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

    void validateRequiredExtensions(const std::vector<const char*>& extensions, const std::vector<const char*>& requiredExtensions)
    {
        // validate that all required extensions are present in extensions container
        std::for_each(requiredExtensions.begin(), requiredExtensions.end(), [&extensions](const char* requiredExtensionName) {
            auto sameExtensionNames = [&requiredExtensionName](const char* extensionName) -> bool {
                return std::strcmp(requiredExtensionName, extensionName) == 0;
            };

            auto iter = std::find_if(extensions.begin(), extensions.end(), sameExtensionNames);
            if (iter == extensions.end()) {
                throw rprpp::InternalError("Required Extension " + std::string(requiredExtensionName) + " not found");
            }
        });
    } // validateReqFunction

    VulkanInstance createInstance(const vk::raii::Context& context, bool enableValidationLayers)
    {
        static const std::vector<const char*> requiredExtensions {
            VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
            VK_KHR_EXTERNAL_MEMORY_CAPABILITIES_EXTENSION_NAME,
            VK_KHR_EXTERNAL_SEMAPHORE_CAPABILITIES_EXTENSION_NAME
        };

        std::vector<const char*> extensions;
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
        extensions.reserve(extensionProperties.size());

        for (const VkExtensionProperties& prop : extensionProperties) {
            extensions.push_back(prop.extensionName);
        }

        validateRequiredExtensions(extensions, requiredExtensions);

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

    vk::raii::Device createDevice(const vk::raii::PhysicalDevice& physicalDevice,
        const std::vector<const char*>& enabledLayers,
        const std::vector<vk::DeviceQueueCreateInfo>& queueInfos)
    {
        const static std::vector<const char*> requiredExtensions {
            VK_KHR_EXTERNAL_MEMORY_EXTENSION_NAME,
            // VK_KHR_EXTERNAL_SEMAPHORE_EXTENSION_NAME, // we don't use it right now
            VK_KHR_EXTERNAL_MEMORY_WIN32_EXTENSION_NAME,
            // VK_KHR_EXTERNAL_SEMAPHORE_WIN32_EXTENSION_NAME, // we don't use it right now
            VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME,

            //  for hybridpro
            VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME,
            VK_EXT_SHADER_SUBGROUP_BALLOT_EXTENSION_NAME,
            VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME,
            VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME,
            VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME,
            VK_KHR_DRAW_INDIRECT_COUNT_EXTENSION_NAME,
            VK_KHR_MAINTENANCE3_EXTENSION_NAME,
            VK_KHR_PIPELINE_LIBRARY_EXTENSION_NAME,
            VK_KHR_RAY_QUERY_EXTENSION_NAME,
            VK_KHR_SPIRV_1_4_EXTENSION_NAME,
        };

        std::vector<const char*> extensions;

        const std::vector<vk::ExtensionProperties> extensionProperties = physicalDevice.enumerateDeviceExtensionProperties();
        extensions.reserve(extensionProperties.size());
        for (const vk::ExtensionProperties& property : extensionProperties) {
            extensions.push_back(property.extensionName);
        }

        validateRequiredExtensions(extensions, requiredExtensions);

        vk::PhysicalDeviceAccelerationStructureFeaturesKHR accelerationStructureFeatures;
        accelerationStructureFeatures.accelerationStructure = vk::True;

        vk::PhysicalDeviceRayQueryFeaturesKHR rayQueryFetures;
        rayQueryFetures.rayQuery = vk::True;

        vk::PhysicalDeviceVulkan12Features features12;
        features12.drawIndirectCount = vk::True;
        features12.shaderFloat16 = vk::True;
        features12.descriptorIndexing = vk::True;
        features12.shaderSampledImageArrayNonUniformIndexing = vk::True;
        features12.shaderStorageBufferArrayNonUniformIndexing = vk::True;
        features12.samplerFilterMinmax = vk::True;
        features12.bufferDeviceAddress = vk::True;

        vk::PhysicalDeviceVulkan11Features features11;
        features11.storageBuffer16BitAccess = vk::True;

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
        features2.features.shaderInt64 = vk::True;
        features2.features.shaderInt16 = vk::True;

        accelerationStructureFeatures.pNext = &rayQueryFetures;
        rayQueryFetures.pNext = &features12;
        features12.pNext = &features11;
        features11.pNext = &features2;

        vk::DeviceCreateInfo deviceCreateInfo({}, queueInfos, enabledLayers, requiredExtensions, nullptr, &accelerationStructureFeatures);
        return physicalDevice.createDevice(deviceCreateInfo);
    }

    uint32_t findMemoryType(const vk::raii::PhysicalDevice physicalDevice, uint32_t typeFilter, vk::MemoryPropertyFlags properties)
    {
        vk::PhysicalDeviceMemoryProperties memProperties = physicalDevice.getMemoryProperties();
        for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
            if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
                return i;
            }
        }

        throw rprpp::InternalError("Failed to find suitable memory type!");
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

} // namespace

// -------------------------------------------------------------------
// Public functions implementations
// -------------------------------------------------------------------

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
    vk::PhysicalDeviceProperties props = physicalDevice.getProperties();

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
    // auto enabledLayers = std::move(instanceAndValidationLayers.second);
    std::optional<vk::raii::DebugUtilsMessengerEXT> debugUtilMessenger;
    if (enableValidationLayers) {
        debugUtilMessenger = vulkanInstance.instance.createDebugUtilsMessengerEXT(makeDebugUtilsMessengerCreateInfoEXT());
    }

    vk::raii::PhysicalDevices physicalDevices(vulkanInstance.instance);
    if (physicalDevices.size() <= deviceId) {
        throw rprpp::InvalidDevice(deviceId);
    }
    vk::raii::PhysicalDevice physicalDevice = std::move(physicalDevices[deviceId]);

    std::optional<uint32_t> queueFamilyIndex;
    std::vector<vk::DeviceQueueCreateInfo> queueInfos;
    float queuePriority = 1.0f;
    auto queueFamilies = physicalDevice.getQueueFamilyProperties();
    for (uint32_t i = 0; i < queueFamilies.size(); ++i) {
        queueInfos.push_back(vk::DeviceQueueCreateInfo({}, i, 1, &queuePriority));
        if (!queueFamilyIndex.has_value()
            && queueFamilies[i].queueCount > 0
            && (queueFamilies[i].queueFlags & vk::QueueFlagBits::eCompute)
            && (queueFamilies[i].queueFlags & vk::QueueFlagBits::eTransfer)) {
            queueFamilyIndex = i;
        }
    }

    if (!queueFamilyIndex.has_value()) {
        throw rprpp::InternalError("Could not find a queue family that supports compute and transfer operations");
    }

    vk::raii::Device device = createDevice(physicalDevice,
        vulkanInstance.enabledLayers,
        queueInfos);
    vk::raii::Queue queue = device.getQueue(queueFamilyIndex.value(), 0);

    return {
        std::move(context),
        std::move(vulkanInstance.instance),
        std::move(debugUtilMessenger),
        std::move(physicalDevice),
        std::move(device),
        std::move(queue),
        queueFamilyIndex.value()
    };
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
