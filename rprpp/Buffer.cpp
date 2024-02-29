#include "Buffer.h"
#include "Error.h"

namespace rprpp {

Buffer::Buffer(vk::helper::Buffer&& buffer, size_t size) noexcept
    : m_buffer(std::move(buffer))
    , m_size(size)
{
}

size_t Buffer::size() const noexcept
{
    return m_size;
}

const vk::helper::Buffer& Buffer::get() const noexcept
{
    return m_buffer;
}

void* Buffer::map(size_t size)
{
    if (size > m_size) {
        throw InvalidParameter("size", "the buffer is smaller than " + std::to_string(size));
    }

    return m_buffer.memory.mapMemory(0, size, {});
}

void Buffer::unmap()
{
    m_buffer.memory.unmapMemory();
}
}
