#pragma once

#include "HostVisibleBuffer.h"
#include "Image.h"
#include "PostProcessing.h"
#include "vk_helper.h"

#include <unordered_map>

template <class T>
using map = std::unordered_map<T*, std::unique_ptr<T>>;

namespace rprpp {

class Context {
public:
    Context(vk::helper::DeviceContext dctx);
    Context(Context&&) = default;
    Context& operator=(Context&&) = default;

    Context(Context&) = delete;
    Context& operator=(const Context&) = delete;

    static std::unique_ptr<Context> create(uint32_t deviceId);
    VkPhysicalDevice getVkPhysicalDevice() const noexcept;
    VkDevice getVkDevice() const noexcept;
    VkQueue getVkQueue() const noexcept;

    PostProcessing* createPostProcessing();
    void destroyPostProcessing(PostProcessing* pp);

    HostVisibleBuffer* createHostVisibleBuffer(size_t size);
    void destroyHostVisibleBuffer(HostVisibleBuffer* buffer);

    Image* createImageFromDx11Texture(HANDLE dx11textureHandle, const ImageDescription& desc);
    void destroyImage(Image* image);

private:
    std::shared_ptr<vk::helper::DeviceContext> m_deviceContext;
    map<PostProcessing> m_postProcessings;
    map<HostVisibleBuffer> m_hostVisibleBuffers;
    map<Image> m_images;
};

}