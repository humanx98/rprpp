#pragma once

#include "DeviceContext.h"

namespace vk::helper {

class CommandBuffer {
public:
    CommandBuffer(DeviceContext* deviceContext);
    ~CommandBuffer();

    vk::raii::CommandBuffer& get() noexcept { return m_commandBuffer; }
    const vk::raii::CommandBuffer& get() const noexcept { return m_commandBuffer; }

private:
    DeviceContext* m_deviceContext;
    vk::raii::CommandBuffer m_commandBuffer;
};

}