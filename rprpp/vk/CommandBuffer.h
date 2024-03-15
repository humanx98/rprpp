#pragma once

#include "DeviceContext.h"

namespace vk::helper {

class CommandBuffer {
public:
    CommandBuffer(const std::shared_ptr<DeviceContext>& deviceContext);
    ~CommandBuffer();
    CommandBuffer(CommandBuffer&&) noexcept;
    CommandBuffer& operator=(CommandBuffer&&) noexcept;
    vk::raii::CommandBuffer& get() { return m_commandBuffer.value(); }

private:
    std::shared_ptr<DeviceContext> m_deviceContext;
    std::optional<vk::raii::CommandBuffer> m_commandBuffer;
};

}