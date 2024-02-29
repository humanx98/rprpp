#pragma once

#include "Context.h"

namespace rprpp::wrappers {

class Buffer {
public:
    Buffer(const Context& context, size_t size);
    ~Buffer();

    void* map(size_t size);
    void unmap();
    size_t size() const noexcept;
    RprPpBuffer get() const noexcept;

    Buffer(const Buffer&) = delete;
    Buffer& operator=(const Buffer&) = delete;

private:
    RprPpContext m_context;
    RprPpBuffer m_buffer;
    size_t m_size;
};

}