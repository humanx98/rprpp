#pragma once

#include "WRprPpContext.h"

class WRprPpBuffer {
public:
    WRprPpBuffer(const WRprPpContext& context, size_t size);
    ~WRprPpBuffer();

    void* map(size_t size);
    void unmap();
    size_t size() const noexcept;
    RprPpBuffer get() const noexcept;

    WRprPpBuffer(const WRprPpBuffer&) = delete;
    WRprPpBuffer& operator=(const WRprPpBuffer&) = delete;

private:
    RprPpContext m_context;
    RprPpBuffer m_buffer;
    size_t m_size;
};