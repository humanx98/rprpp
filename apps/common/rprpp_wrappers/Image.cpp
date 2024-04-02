#include "Image.h"
#include <utility>

namespace rprpp::wrappers {

Image::Image(const Context& context, RprPpImage image, const RprPpImageDescription& description) noexcept
    : m_context(context.get())
    , m_image(image)
    , m_description(description)
{
}

Image::Image(Image&& other) noexcept
    : m_context(std::exchange(other.m_context, nullptr))
    , m_image(std::exchange(other.m_image, nullptr))
    , m_description(other.m_description)
{
}

Image& Image::operator=(Image&& other) noexcept
{
    std::swap(m_context, other.m_context);
    std::swap(m_image, other.m_image);
    std::swap(m_description, other.m_description);
    return *this;
}

Image Image::create(const Context& context, const RprPpImageDescription& description)
{
    RprPpError status;
    RprPpImage image;

    status = rprppContextCreateImage(context.get(), description, &image);
    RPRPP_CHECK(status);

    return Image(context, image, description);
}

Image Image::createFromVkSampledImage(const Context& context, RprPpVkImage vkSampledImage, const RprPpImageDescription& description)
{
    RprPpError status;
    RprPpImage image;

    status = rprppContextCreateImageFromVkSampledImage(context.get(), vkSampledImage, description, &image);
    RPRPP_CHECK(status);

    return Image(context, image, description);
}

Image Image::createImageFromDx11Texture(const Context& context, RprPpDx11Handle dx11textureHandle, const RprPpImageDescription& description)
{
    RprPpError status;
    RprPpImage image;

    status = rprppContextCreateImageFromDx11Texture(context.get(), dx11textureHandle, description, &image);
    RPRPP_CHECK(status);

    return Image(context, image, description);
}

Image::~Image()
{
    if (!m_image)
        return;

    RprPpError status;

    status = rprppContextDestroyImage(m_context, m_image);
    RPRPP_CHECK(status);

#ifndef NDEBUG
    m_image = nullptr;
#endif
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