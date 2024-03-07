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

void Context::waitQueueIdle()
{
    RprPpError status;

    status = rprppContextWaitQueueIdle(m_context);
    RPRPP_CHECK(status);
}

RprPpContext Context::get() const noexcept
{
    return m_context;
}

void Context::copyImageToBuffer(RprPpImage image, RprPpBuffer buffer)
{
    RprPpError status;

    status = rprppContextCopyImageToBuffer(m_context, image, buffer);
    RPRPP_CHECK(status);
}

void Context::copyBufferToImage(RprPpBuffer buffer, RprPpImage image)
{
    RprPpError status;

    status = rprppContextCopyBufferToImage(m_context, buffer, image);
    RPRPP_CHECK(status);
}

void Context::copyImage(RprPpImage src, RprPpImage dst)
{
    RprPpError status;

    status = rprppContextCopyImage(m_context, src, dst);
    RPRPP_CHECK(status);
}

}