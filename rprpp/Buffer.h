#pragma once

#include "vk/DeviceContext.h"
#include "ContextObject.h"

namespace rprpp {

class Buffer : public ContextObject {
public:
    explicit Buffer(Context* parent, vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties, bool oidnExportable = false);

    [[nodiscard]]
    size_t size() const noexcept { return m_size; }

    [[nodiscard]]
    vk::Buffer get() const noexcept;

    [[nodiscard]] 
    vk::DeviceMemory memory() const noexcept;

    [[nodiscard]]
    void* map(size_t size);

    void unmap();
private:
    vk::raii::Buffer createBuffer(const vk::helper::DeviceContext& dctx, vk::DeviceSize size, vk::BufferUsageFlags usage, bool oidnExportable = false);
    vk::raii::DeviceMemory allocateMemory(const vk::helper::DeviceContext& dctx, const vk::MemoryPropertyFlags& properties, bool oidnExportable = false);

    size_t m_size;
    vk::raii::Buffer m_buffer;
    vk::raii::DeviceMemory m_memory;
};

}