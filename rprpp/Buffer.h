#pragma once

#include "vk_helper.h"

namespace rprpp {

class Buffer {
public:
    Buffer(vk::helper::Buffer&&, size_t size) noexcept;

    Buffer(Buffer&&) = default;
    Buffer& operator=(Buffer&&) = default;

    Buffer(Buffer&) = delete;
    Buffer& operator=(const Buffer&) = delete;

    size_t size() const noexcept;
    const vk::helper::Buffer& get() const noexcept;
    void* map(size_t size);
    void unmap();

private:
    size_t m_size = 0;
    vk::helper::Buffer m_buffer;
};

}