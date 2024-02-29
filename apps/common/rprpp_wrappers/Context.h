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
    RprPpContext get() const noexcept;

    Context(const Context&) = delete;
    Context& operator=(const Context&) = delete;

private:
    RprPpContext m_context;
};

}