#include "Context.h"

namespace rprpp::wrappers {

Context::Context(uint32_t deviceId)
{
    RprPpError status;

    status = rprppCreateContext(deviceId, &m_context);
    RPRPP_CHECK(status);
}

Context::~Context()
{
    RprPpError status;

    status = rprppDestroyContext(m_context);
    RPRPP_CHECK(status);
}

RprPpVkPhysicalDevice Context::getVkPhysicalDevice() const noexcept
{
    RprPpError status;
    RprPpVkPhysicalDevice vkhandle = nullptr;

    status = rprppContextGetVkPhysicalDevice(m_context, &vkhandle);
    RPRPP_CHECK(status);
    return vkhandle;
}

RprPpVkDevice Context::getVkDevice() const noexcept
{
    RprPpError status;
    RprPpVkDevice vkhandle = nullptr;

    status = rprppContextGetVkDevice(m_context, &vkhandle);
    RPRPP_CHECK(status);
    return vkhandle;
}

RprPpVkQueue Context::getVkQueue() const noexcept
{
    RprPpError status;
    RprPpVkQueue vkhandle = nullptr;

    status = rprppContextGetVkQueue(m_context, &vkhandle);
    RPRPP_CHECK(status);
    return vkhandle;
}

RprPpContext Context::get() const noexcept
{
    return m_context;
}

}