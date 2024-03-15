#include "CommandBuffer.h"

namespace vk::helper {

CommandBuffer::CommandBuffer(const std::shared_ptr<DeviceContext>& deviceContext)
    : m_deviceContext(deviceContext)
    , m_commandBuffer(deviceContext->takeCommandBuffer())
{
}

CommandBuffer::CommandBuffer(CommandBuffer&& other) noexcept
{
    m_deviceContext = std::move(other.m_deviceContext);
    m_commandBuffer = std::move(other.m_commandBuffer);
    other.m_commandBuffer.reset();
}

CommandBuffer& CommandBuffer::operator=(CommandBuffer&& other) noexcept
{
    std::swap(m_deviceContext, other.m_deviceContext);
    std::swap(m_commandBuffer, other.m_commandBuffer);
    return *this;
}

CommandBuffer::~CommandBuffer()
{
    if (m_commandBuffer.has_value()) {
        m_deviceContext->returnCommandBuffer(std::move(m_commandBuffer.value()));
    }
}

}