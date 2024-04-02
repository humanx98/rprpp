#pragma once

#include "vk/DeviceContext.h"
#include "ContextObject.h"

namespace rprpp {

class Buffer : public ContextObject {
public:
    explicit Buffer(Context* parent, vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties);

    size_t size() const noexcept { return m_size; }
    vk::Buffer get() const noexcept;
    void* map(size_t size);
    void unmap();
private:
    vk::raii::Buffer createBuffer(const vk::helper::DeviceContext& dctx, vk::DeviceSize size, vk::BufferUsageFlags usage);
    vk::raii::DeviceMemory allocateMemory(const vk::helper::DeviceContext& dctx, const vk::MemoryPropertyFlags& properties);

    size_t m_size;
    vk::raii::Buffer m_buffer;
    vk::raii::DeviceMemory m_memory;
};

}