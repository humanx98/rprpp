#pragma once

#include "vk_helper.h"

namespace rprpp {

class HostVisibleBuffer {
public:
    HostVisibleBuffer(vk::helper::Buffer&&, size_t size) noexcept;

    HostVisibleBuffer(HostVisibleBuffer&&) = default;
    HostVisibleBuffer& operator=(HostVisibleBuffer&&) = default;

    HostVisibleBuffer(HostVisibleBuffer&) = delete;
    HostVisibleBuffer& operator=(const HostVisibleBuffer&) = delete;

    size_t size() const noexcept;
    const vk::raii::Buffer& buffer() const noexcept;
    void* map(size_t size);
    void unmap();

private:
    size_t m_size = 0;
    vk::helper::Buffer m_buffer;
};

}