#include "WRprPpContext.h"

WRprPpContext::WRprPpContext(uint32_t deviceId)
{
    RprPpError status;

    status = rprppCreateContext(deviceId, &m_context);
    RPRPP_CHECK(status);
}

WRprPpContext::~WRprPpContext()
{
    RprPpError status;

    status = rprppDestroyContext(m_context);
    RPRPP_CHECK(status);
}

RprPpVkPhysicalDevice WRprPpContext::getVkPhysicalDevice() const noexcept
{
    RprPpError status;
    RprPpVkPhysicalDevice vkhandle = nullptr;

    status = rprppContextGetVkPhysicalDevice(m_context, &vkhandle);
    RPRPP_CHECK(status);
    return vkhandle;
}

RprPpVkDevice WRprPpContext::getVkDevice() const noexcept
{
    RprPpError status;
    RprPpVkDevice vkhandle = nullptr;

    status = rprppContextGetVkDevice(m_context, &vkhandle);
    RPRPP_CHECK(status);
    return vkhandle;
}

RprPpVkQueue WRprPpContext::getVkQueue() const noexcept
{
    RprPpError status;
    RprPpVkQueue vkhandle = nullptr;

    status = rprppContextGetVkQueue(m_context, &vkhandle);
    RPRPP_CHECK(status);
    return vkhandle;
}

RprPpContext WRprPpContext::get() const noexcept
{
    return m_context;
}
