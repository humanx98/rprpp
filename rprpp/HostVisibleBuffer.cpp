#include "HostVisibleBuffer.h"
#include "Error.h"

namespace rprpp {

HostVisibleBuffer::HostVisibleBuffer(vk::helper::Buffer&& buffer, size_t size) noexcept
    : m_buffer(std::move(buffer))
    , m_size(size)
{
}

size_t HostVisibleBuffer::size() const noexcept
{
    return m_size;
}

const vk::raii::Buffer& HostVisibleBuffer::buffer() const noexcept
{
    return m_buffer.buffer;
}

void* HostVisibleBuffer::map(size_t size)
{
    if (size > m_size) {
        throw InvalidParameter("size", "the buffer is smaller than " + std::to_string(size));
    }

    return m_buffer.memory.mapMemory(0, size, {});
}

void HostVisibleBuffer::unmap()
{
    m_buffer.memory.unmapMemory();
}
}
