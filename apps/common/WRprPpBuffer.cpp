#include "WRprPpBuffer.h"

WRprPpBuffer::WRprPpBuffer(const WRprPpContext& context, size_t size)
    : m_context(context.get())
    , m_size(size)
{
    RprPpError status;

    status = rprppContextCreateBuffer(m_context, size, &m_buffer);
    RPRPP_CHECK(status);
}

WRprPpBuffer::~WRprPpBuffer()
{
    RprPpError status;

    status = rprppContextDestroyBuffer(m_context, m_buffer);
    RPRPP_CHECK(status);
}

void* WRprPpBuffer::map(size_t size)
{
    void* mapped = nullptr;
    RprPpError status;

    status = rprppBufferMap(m_buffer, size, &mapped);
    RPRPP_CHECK(status);
    return mapped;
}

void WRprPpBuffer::unmap()
{
    RprPpError status;

    status = rprppBufferUnmap(m_buffer);
    RPRPP_CHECK(status);
}

size_t WRprPpBuffer::size() const noexcept
{
    return m_size;
}

RprPpBuffer WRprPpBuffer::get() const noexcept
{
    return m_buffer;
}
