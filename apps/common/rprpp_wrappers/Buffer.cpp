#include "Buffer.h"

namespace rprpp::wrappers {

Buffer::Buffer(const Context& context, size_t size)
    : m_context(context.get())
    , m_size(size)
{
    RprPpError status;

    status = rprppContextCreateBuffer(m_context, size, &m_buffer);
    RPRPP_CHECK(status);
}

Buffer::~Buffer()
{
    RprPpError status;

    status = rprppContextDestroyBuffer(m_context, m_buffer);
    RPRPP_CHECK(status);
}

void* Buffer::map(size_t size)
{
    void* mapped = nullptr;
    RprPpError status;

    status = rprppBufferMap(m_buffer, size, &mapped);
    RPRPP_CHECK(status);
    return mapped;
}

void Buffer::unmap()
{
    RprPpError status;

    status = rprppBufferUnmap(m_buffer);
    RPRPP_CHECK(status);
}

size_t Buffer::size() const noexcept
{
    return m_size;
}

RprPpBuffer Buffer::get() const noexcept
{
    return m_buffer;
}

}
