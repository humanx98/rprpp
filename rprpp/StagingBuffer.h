#pragma once

#include "vk_helper.h"

namespace rprpp {

class StagingBuffer {
public:
    explicit StagingBuffer(vk::raii::DeviceMemory* deviceMemory, vk::DeviceSize offset,
        vk::DeviceSize size,
        vk::MemoryMapFlags flags = {});

    StagingBuffer(StagingBuffer&& stagingBuffer) noexcept;
    StagingBuffer& operator=(StagingBuffer&& stagingBuffer) noexcept;

    ~StagingBuffer();

    void unmap() noexcept;

    void* data() const noexcept { return m_mappedMemory; }

    StagingBuffer(const StagingBuffer&) = delete;
    StagingBuffer& operator=(const StagingBuffer&) = delete;
private:
    vk::raii::DeviceMemory* m_deviceMemory;
    void* m_mappedMemory;
};

} // namespace
