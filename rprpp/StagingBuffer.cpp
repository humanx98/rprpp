#include "StagingBuffer.h"
#include <cassert>

namespace rprpp {
StagingBuffer::StagingBuffer(vk::raii::DeviceMemory* deviceMemory, vk::DeviceSize offset, vk::DeviceSize size, vk::MemoryMapFlags flags )
 : m_deviceMemory(deviceMemory)
{
    assert(m_deviceMemory);

    m_mappedMemory = m_deviceMemory->mapMemory(offset, size, flags);
    assert(m_mappedMemory != nullptr);
}

StagingBuffer::StagingBuffer(StagingBuffer&& stagingBuffer) noexcept
: m_deviceMemory(stagingBuffer.m_deviceMemory), 
  m_mappedMemory(nullptr)
{
    std::swap(m_mappedMemory, stagingBuffer.m_mappedMemory);
}

StagingBuffer& StagingBuffer::operator=(StagingBuffer&& stagingBuffer) noexcept
{
    if (m_mappedMemory) {
        assert(m_deviceMemory);
        unmap();
    }

    m_deviceMemory = nullptr;
    assert(m_mappedMemory == nullptr);

    std::swap(m_deviceMemory, stagingBuffer.m_deviceMemory);
    std::swap(m_mappedMemory, stagingBuffer.m_mappedMemory);

    return *this;
}

void StagingBuffer::unmap() noexcept
{
    assert(m_deviceMemory);
    assert(m_mappedMemory != nullptr);

    m_deviceMemory->unmapMemory();
    m_mappedMemory = nullptr;
}

StagingBuffer::~StagingBuffer()
{
    if (m_mappedMemory)
        unmap();
}

} // namespace
