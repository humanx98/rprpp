#pragma once

#include "rprpp/rprpp.h"
#include "vk_helper.h"

#include <optional>

namespace vk::helper {

struct DeviceContext {
    static DeviceContext create(uint32_t deviceId);
    vk::raii::CommandBuffer takeCommandBuffer();
    void returnCommandBuffer(vk::raii::CommandBuffer&& buffer);

    vk::raii::Context context;
    vk::helper::Instance instance;
    std::optional<vk::raii::DebugUtilsMessengerEXT> debugUtilMessenger;
    vk::raii::PhysicalDevice physicalDevice;
    vk::raii::Device device;
    vk::raii::Queue queue;
    uint32_t queueFamilyIndex;
    std::vector<vk::raii::CommandPool> commandPools;
    std::vector<vk::raii::CommandBuffer> commandBuffers;
};

}