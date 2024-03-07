#include "Buffer.h"
#include "Error.h"

namespace rprpp {

Buffer::Buffer(vk::raii::Buffer&& buffer, vk::raii::DeviceMemory&& memory, size_t size) noexcept
    : m_buffer(std::move(buffer))
    , m_memory(std::move(memory))
    , m_size(size)
{
}

Buffer Buffer::create(const vk::helper::DeviceContext& dctx, vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties)
{
    vk::raii::Buffer buffer(dctx.device, vk::BufferCreateInfo({}, size, usage, vk::SharingMode::eExclusive));

    vk::MemoryRequirements memRequirements = buffer.getMemoryRequirements();
    uint32_t memoryType = vk::helper::findMemoryType(dctx.physicalDevice, memRequirements.memoryTypeBits, properties);
    auto memory = dctx.device.allocateMemory(vk::MemoryAllocateInfo(memRequirements.size, memoryType));

    buffer.bindMemory(*memory, 0);

    return Buffer(std::move(buffer), std::move(memory), size);
}

size_t Buffer::size() const noexcept
{
    return m_size;
}

vk::Buffer Buffer::get() const noexcept
{
    return *m_buffer;
}

void* Buffer::map(size_t size)
{
    if (size > m_size) {
        throw InvalidParameter("size", "the buffer is smaller than " + std::to_string(size));
    }

    return m_memory.mapMemory(0, size, {});
}

void Buffer::unmap()
{
    m_memory.unmapMemory();
}
}
