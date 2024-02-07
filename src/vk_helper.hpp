#pragma once

#define VK_USE_PLATFORM_WIN32_KHR
#include <optional>
#include <vulkan/vulkan_raii.hpp>
#include <windows.h>

namespace vk::helper {

struct DeviceContext {
    vk::raii::Context context;
    vk::raii::Instance instance;
    std::optional<vk::raii::DebugUtilsMessengerEXT> debugUtilMessenger;
    vk::raii::PhysicalDevice physicalDevice;
    vk::raii::Device device;
    vk::raii::Queue queue;
    uint32_t queueFamilyIndex;
};

struct Buffer {
    vk::raii::Buffer buffer;
    vk::raii::DeviceMemory memory;
};

struct Image {
    vk::raii::Image image;
    vk::raii::DeviceMemory memory;
    vk::raii::ImageView view;
    uint32_t width;
    uint32_t height;
    vk::AccessFlags access;
    vk::ImageLayout layout;
    vk::PipelineStageFlags stage;
};

DeviceContext createDeviceContext(bool enableValidationLayers, uint32_t deviceId);

Image createImage(const DeviceContext& dctx,
    uint32_t width,
    uint32_t height,
    vk::Format format,
    vk::ImageUsageFlags usage,
    HANDLE sharedDx11TextureHandle = nullptr);

Buffer createBuffer(const DeviceContext& dctx,
    vk::DeviceSize size,
    vk::BufferUsageFlags usage,
    vk::MemoryPropertyFlags properties);

}