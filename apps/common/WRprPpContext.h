#pragma once

#include "rpr_helper.h"
#include "rprpp/rprpp.h"

class WRprPpContext {
public:
    WRprPpContext(uint32_t deviceId);
    ~WRprPpContext();

    RprPpVkPhysicalDevice getVkPhysicalDevice() const noexcept;
    RprPpVkDevice getVkDevice() const noexcept;
    RprPpVkQueue getVkQueue() const noexcept;
    RprPpContext get() const noexcept;

    WRprPpContext(const WRprPpContext&) = delete;
    WRprPpContext& operator=(const WRprPpContext&) = delete;

private:
    RprPpContext m_context;
};