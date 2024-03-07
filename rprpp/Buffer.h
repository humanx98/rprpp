#pragma once

#include "vk/DeviceContext.h"

namespace rprpp {

class Buffer {
public:
    Buffer(vk::raii::Buffer&&, vk::raii::DeviceMemory&& memory, size_t size) noexcept;
    Buffer(Buffer&&) = default;
    Buffer& operator=(Buffer&&) = default;

    static Buffer create(const vk::helper::DeviceContext& dctx, vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties);
    size_t size() const noexcept;
    vk::Buffer get() const noexcept;
    void* map(size_t size);
    void unmap();

    Buffer(Buffer&) = delete;
    Buffer& operator=(const Buffer&) = delete;

private:
    size_t m_size = 0;
    vk::raii::Buffer m_buffer;
    vk::raii::DeviceMemory m_memory;
};

}