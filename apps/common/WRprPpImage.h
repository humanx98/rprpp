#pragma once

#include "WRprPpContext.h"

class WRprPpImage {
public:
    WRprPpImage(const WRprPpContext& context, RprPpDx11Handle dx11textureHandle, const RprPpImageDescription& description);
    ~WRprPpImage();

    RprPpImage get() const noexcept;
    const RprPpImageDescription& description() const noexcept;

    WRprPpImage(const WRprPpImage&) = delete;
    WRprPpImage& operator=(const WRprPpImage&) = delete;

private:
    RprPpContext m_context;
    RprPpImage m_image;
    RprPpImageDescription m_description;
};