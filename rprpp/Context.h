#pragma once

#include "Buffer.h"
#include "Image.h"
#include "PostProcessing.h"
#include "vk/DeviceContext.h"

#include <unordered_map>

template <class T>
using map = std::unordered_map<T*, std::unique_ptr<T>>;

namespace rprpp {

class Context {
public:
    Context(const std::shared_ptr<vk::helper::DeviceContext>& dctx);
    Context(Context&&) = default;
    Context& operator=(Context&&) = default;

    Context(Context&) = delete;
    Context& operator=(const Context&) = delete;

    static std::unique_ptr<Context> create(uint32_t deviceId);
    VkPhysicalDevice getVkPhysicalDevice() const noexcept;
    VkDevice getVkDevice() const noexcept;
    VkQueue getVkQueue() const noexcept;
    void waitQueueIdle();

    PostProcessing* createPostProcessing();
    void destroyPostProcessing(PostProcessing* pp);

    Buffer* createBuffer(size_t size);
    void destroyBuffer(Buffer* buffer);

    Image* createImage(const ImageDescription& desc);
    Image* createFromVkSampledImage(vk::Image image, const ImageDescription& desc);
    Image* createImageFromDx11Texture(HANDLE dx11textureHandle, const ImageDescription& desc);
    void destroyImage(Image* image);
    void copyBufferToImage(Buffer* buffer, Image* image);
    void copyImageToBuffer(Image* image, Buffer* buffer);
    void copyImage(Image* src, Image* dst);

private:
    std::shared_ptr<vk::helper::DeviceContext> m_deviceContext;
    map<PostProcessing> m_postProcessings;
    map<Buffer> m_buffers;
    map<Image> m_images;
};

}