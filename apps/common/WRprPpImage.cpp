#include "WRprPpImage.h"

WRprPpImage::WRprPpImage(const WRprPpContext& context, RprPpDx11Handle dx11textureHandle, const RprPpImageDescription& description)
    : m_context(context.get())
    , m_description(description)
{
    RprPpError status;

    status = rprppContextCreateImageFromDx11Texture(m_context, dx11textureHandle, m_description, &m_image);
    RPRPP_CHECK(status);
}


WRprPpImage::~WRprPpImage()
{
    RprPpError status;

    status = rprppContextDestroyImage(m_context, m_image);
    RPRPP_CHECK(status);
}

RprPpImage WRprPpImage::get() const noexcept
{
    return m_image;
}

const RprPpImageDescription& WRprPpImage::description() const noexcept
{
    return m_description;
}