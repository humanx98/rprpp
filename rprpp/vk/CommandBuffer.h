#pragma once

#include "DeviceContext.h"

namespace vk::helper {

class CommandBuffer {
public:
    explicit CommandBuffer(DeviceContext* deviceContext);
    ~CommandBuffer();

    [[nodiscard]]
    vk::raii::CommandBuffer& get() noexcept { return m_commandBuffer; }

    [[nodiscard]]
    const vk::raii::CommandBuffer& get() const noexcept { return m_commandBuffer; }

private:
    DeviceContext* m_deviceContext;
    vk::raii::CommandBuffer m_commandBuffer;
};

}