#include "WRprPpHostVisibleBuffer.h"

WRprPpHostVisibleBuffer::WRprPpHostVisibleBuffer(const WRprPpContext& context, size_t size)
{
    RprPpError status;
    m_context = context.get();
    m_size = size;

    status = rprppContextCreateHostVisibleBuffer(m_context, size, &m_buffer);
    RPRPP_CHECK(status);
}

WRprPpHostVisibleBuffer::~WRprPpHostVisibleBuffer()
{
    RprPpError status;

    status = rprppContextDestroyHostVisibleBuffer(m_context, m_buffer);
    RPRPP_CHECK(status);
}

void* WRprPpHostVisibleBuffer::map(size_t size)
{
    void* mapped = nullptr;
    RprPpError status;

    status = rprppHostVisibleBufferMap(m_buffer, size, &mapped);
    RPRPP_CHECK(status);
    return mapped;
}

void WRprPpHostVisibleBuffer::unmap()
{
    RprPpError status;

    status = rprppHostVisibleBufferUnmap(m_buffer);
    RPRPP_CHECK(status);
}

size_t WRprPpHostVisibleBuffer::size() const noexcept
{
    return m_size;
}

RprPpHostVisibleBuffer WRprPpHostVisibleBuffer::get() const noexcept
{
    return m_buffer;
}
