#include "Image.h"

namespace rprpp::wrappers {

Image::Image(const Context& context, RprPpDx11Handle dx11textureHandle, const RprPpImageDescription& description)
    : m_context(context.get())
    , m_description(description)
{
    RprPpError status;

    status = rprppContextCreateImageFromDx11Texture(m_context, dx11textureHandle, m_description, &m_image);
    RPRPP_CHECK(status);
}

Image::~Image()
{
    RprPpError status;

    status = rprppContextDestroyImage(m_context, m_image);
    RPRPP_CHECK(status);
}

RprPpImage Image::get() const noexcept
{
    return m_image;
}

const RprPpImageDescription& Image::description() const noexcept
{
    return m_description;
}

}