#pragma once

#include "helper.h"
#include "rprpp/rprpp.h"

#include <cstdint>

namespace rprpp::wrappers {

class Context {
public:
    explicit Context(uint32_t deviceId);
    ~Context();

    [[nodiscard]]
    RprPpVkPhysicalDevice getVkPhysicalDevice() const noexcept;

    [[nodiscard]]
    RprPpVkDevice getVkDevice() const noexcept;

    [[nodiscard]]
    RprPpVkQueue getVkQueue() const noexcept;

    void waitQueueIdle();

    [[nodiscard]]
    RprPpContext get() const noexcept;

    void copyBufferToImage(RprPpBuffer buffer, RprPpImage image);
    void copyImageToBuffer(RprPpImage image, RprPpBuffer buffer);
    void copyImage(RprPpImage src, RprPpImage dst);

    Context(const Context&) = delete;
    Context& operator=(const Context&) = delete;
private:
    RprPpContext m_context;
};

}