#pragma once

#include "rprpp/rprpp.h"
#include "vk.h"

namespace vk::helper {

enum class DeviceInfo {
    eName = RPRPP_DEVICE_INFO_NAME,
    eLUID = RPRPP_DEVICE_INFO_LUID,
    eSupportHardwareRayTracing = RPRPP_DEVICE_INFO_SUPPORT_HARDWARE_RAY_TRACING,
};

struct DeviceContext {
    vk::raii::Context context;
    vk::raii::Instance instance;
    std::optional<vk::raii::DebugUtilsMessengerEXT> debugUtilMessenger;
    vk::raii::PhysicalDevice physicalDevice;
    vk::raii::Device device;
    vk::raii::Queue queue;
    uint32_t queueFamilyIndex;
    std::vector<vk::raii::CommandPool> commandPools;
    std::vector<vk::raii::CommandBuffer> commandBuffers;

    vk::raii::CommandBuffer takeCommandBuffer();
    void returnCommandBuffer(vk::raii::CommandBuffer&& buffer);
};

void getDeviceInfo(uint32_t deviceId, DeviceInfo info, void* data, size_t size, size_t* sizeRet);
uint32_t getDeviceCount();

uint32_t findMemoryType(const vk::raii::PhysicalDevice& physicalDevice, uint32_t typeFilter, vk::MemoryPropertyFlags properties);
DeviceContext createDeviceContext(uint32_t deviceId);

}