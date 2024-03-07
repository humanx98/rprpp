#pragma once

#include "helper.h"
#include "rprpp/rprpp.h"

namespace rprpp::wrappers {

class Context {
public:
    Context(uint32_t deviceId);
    ~Context();

    RprPpVkPhysicalDevice getVkPhysicalDevice() const noexcept;
    RprPpVkDevice getVkDevice() const noexcept;
    RprPpVkQueue getVkQueue() const noexcept;
    void waitQueueIdle();
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