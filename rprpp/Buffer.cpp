#include "Buffer.h"
#include "Error.h"
#include "Context.h"

namespace rprpp {

Buffer::Buffer(Context* parent, vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties)
: ContextObject(parent), 
  m_size(size), 
  m_buffer(createBuffer(context()->deviceContext(), size, usage)),
  m_memory(allocateMemory(context()->deviceContext(), properties))
{
    m_buffer.bindMemory(*m_memory, 0);
}

vk::raii::Buffer Buffer::createBuffer(const vk::helper::DeviceContext& dctx, vk::DeviceSize size, vk::BufferUsageFlags usage)
{
    return vk::raii::Buffer(dctx.device, vk::BufferCreateInfo({}, size, usage, vk::SharingMode::eExclusive));
}

vk::raii::DeviceMemory Buffer::allocateMemory(const vk::helper::DeviceContext& dctx, const vk::MemoryPropertyFlags& properties)
{
    vk::MemoryRequirements memRequirements = m_buffer.getMemoryRequirements();
    uint32_t memoryType = vk::helper::findMemoryType(dctx.physicalDevice, memRequirements.memoryTypeBits, properties);
    return dctx.device.allocateMemory(vk::MemoryAllocateInfo(memRequirements.size, memoryType));
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
