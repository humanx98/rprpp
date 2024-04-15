#include "DeviceContext.h"
#include "vk_helper.h"
#include "rprpp/Error.h"

namespace vk::helper {

DeviceContext DeviceContext::create(uint32_t deviceId)
{
#if NDEBUG
    const bool enableValidationLayers = false;
#else
    const bool enableValidationLayers = true;
#endif

    vk::raii::Context context;
    Instance instance = createInstance(context, enableValidationLayers);
    std::optional<vk::raii::DebugUtilsMessengerEXT> debugUtilMessenger;
    if (enableValidationLayers) {
        debugUtilMessenger = instance.get().createDebugUtilsMessengerEXT(vk::helper::makeDebugUtilsMessengerCreateInfoEXT());
    }

    vk::raii::PhysicalDevices physicalDevices(instance.get());
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
        instance.enabledLayers(),
        queueInfos);
    vk::raii::Queue queue = device.getQueue(computeQueueFamilyIndex.value(), 0);

    return {
        std::move(context),
        std::move(instance),
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

void DeviceContext::returnCommandBuffer(vk::raii::CommandBuffer&& buffer)
{
    commandBuffers.push_back(std::move(buffer));
}

}
