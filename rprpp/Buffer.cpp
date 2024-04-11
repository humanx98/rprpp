#include "Buffer.h"
#include "Context.h"
#include "Error.h"

namespace rprpp {

Buffer::Buffer(Context* parent, vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties, bool win32Exportable)
    : ContextObject(parent)
    , m_size(size)
    , m_buffer(createBuffer(context()->deviceContext(), size, usage, win32Exportable))
    , m_memory(allocateMemory(context()->deviceContext(), properties, win32Exportable))
{
    m_buffer.bindMemory(*m_memory, 0);
}

vk::raii::Buffer Buffer::createBuffer(const vk::helper::DeviceContext& dctx, vk::DeviceSize size, vk::BufferUsageFlags usage, bool win32Exportable)
{
    vk::BufferCreateInfo info({}, size, usage, vk::SharingMode::eExclusive);
    vk::ExternalMemoryBufferCreateInfo externalInfo(vk::ExternalMemoryHandleTypeFlagBits::eOpaqueWin32);
    if (win32Exportable) {
        info.pNext = &externalInfo;
    }

    return vk::raii::Buffer(dctx.device, info);
}

vk::raii::DeviceMemory Buffer::allocateMemory(const vk::helper::DeviceContext& dctx, const vk::MemoryPropertyFlags& properties, bool win32Exportable)
{
    vk::MemoryRequirements memRequirements = m_buffer.getMemoryRequirements();
    uint32_t memoryType = vk::helper::findMemoryType(dctx.physicalDevice, memRequirements.memoryTypeBits, properties);
    vk::MemoryAllocateInfo info(memRequirements.size, memoryType);
    vk::ExportMemoryAllocateInfo exportInfo(vk::ExternalMemoryHandleTypeFlagBits::eOpaqueWin32);
    if (win32Exportable) {
        info.pNext = &exportInfo;
    }
    return dctx.device.allocateMemory(info);
}

vk::Buffer Buffer::get() const noexcept
{
    return *m_buffer;
}

vk::DeviceMemory Buffer::memory() const noexcept
{
    return *m_memory;
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
