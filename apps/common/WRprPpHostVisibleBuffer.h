#pragma once

#include "WRprPpContext.h"

class WRprPpHostVisibleBuffer {
public:
    WRprPpHostVisibleBuffer(const WRprPpContext& context, size_t size);
    ~WRprPpHostVisibleBuffer();

    void* map(size_t size);
    void unmap();
    size_t size() const noexcept;
    RprPpHostVisibleBuffer get() const noexcept;

    WRprPpHostVisibleBuffer(const WRprPpHostVisibleBuffer&) = delete;
    WRprPpHostVisibleBuffer& operator=(const WRprPpHostVisibleBuffer&) = delete;

private:
    RprPpContext m_context;
    RprPpHostVisibleBuffer m_buffer;
    size_t m_size;
};