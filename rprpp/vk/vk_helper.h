#pragma once

#include "vk.h"
#include <boost/noncopyable.hpp>

namespace vk::helper {

class Instance {
public:
    explicit Instance(vk::raii::Instance&& instance, std::vector<const char*>&& enabledLayers);
    Instance(Instance&&) noexcept = default;
    Instance& operator=(Instance&&) noexcept = default;
    Instance(const Instance&) = delete;
    Instance& operator=(const Instance&) = delete;

    [[nodiscard]] vk::raii::Instance& get() noexcept
    {
        return m_instance;
    }

    [[nodiscard]] const std::vector<const char*>& enabledLayers() const noexcept
    {
        return m_enabledLayers;
    }

private:
    vk::raii::Instance m_instance;
    std::vector<const char*> m_enabledLayers;
};

vk::DebugUtilsMessengerCreateInfoEXT makeDebugUtilsMessengerCreateInfoEXT();
bool validateRequiredExtensions(const std::vector<const char*>& extensions, const std::vector<const char*>& requiredExtensions);
std::vector<const char*> getRayTracingExtensions();
uint32_t getDeviceCount();
uint32_t findMemoryType(const vk::raii::PhysicalDevice& physicalDevice, uint32_t typeFilter, vk::MemoryPropertyFlags properties);
Instance createInstance(const vk::raii::Context& context, bool enableValidationLayers);
vk::raii::Device createDevice(const vk::raii::PhysicalDevice& physicalDevice,
    const std::vector<const char*>& enabledLayers,
    const std::vector<vk::DeviceQueueCreateInfo>& queueInfos);

}