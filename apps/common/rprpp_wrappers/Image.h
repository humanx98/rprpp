#pragma once

#include "Context.h"

namespace rprpp::wrappers {

class Image {
public:
    Image(const Context& context, RprPpDx11Handle dx11textureHandle, const RprPpImageDescription& description);
    ~Image();

    RprPpImage get() const noexcept;
    const RprPpImageDescription& description() const noexcept;

    Image(const Image&) = delete;
    Image& operator=(const Image&) = delete;

private:
    RprPpContext m_context;
    RprPpImage m_image;
    RprPpImageDescription m_description;
};

}