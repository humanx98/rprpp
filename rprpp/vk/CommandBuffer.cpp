#include "CommandBuffer.h"

namespace vk::helper {

CommandBuffer::CommandBuffer(DeviceContext* deviceContext)
    : m_deviceContext(deviceContext)
    , m_commandBuffer(deviceContext->takeCommandBuffer())
{
}

CommandBuffer::~CommandBuffer()
{
    m_deviceContext->returnCommandBuffer(std::move(m_commandBuffer));
}

}