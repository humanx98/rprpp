#pragma once

#include "ContextObjectContainer.h"

#include "Buffer.h"
#include "Image.h"
#include "filters/BloomFilter.h"
#include "filters/ComposeColorShadowReflectionFilter.h"
#include "filters/ComposeOpacityShadowFilter.h"
#include "filters/DenoiserFilter.h"
#include "filters/Filter.h"
#include "filters/ToneMapFilter.h"
#include "oidn_helper.h"
#include "vk/DeviceContext.h"

#include <boost/noncopyable.hpp>

template <class T>
using map = std::unordered_map<T*, std::unique_ptr<T>>;

namespace rprpp {

class Context : public boost::noncopyable {
public:
    explicit Context(uint32_t deviceId, uint8_t luid[vk::LuidSize], uint8_t uuid[vk::UuidSize]);

    [[nodiscard]] VkPhysicalDevice getVkPhysicalDevice() const noexcept;

    [[nodiscard]] VkDevice getVkDevice() const noexcept;

    [[nodiscard]] VkQueue getVkQueue() const noexcept;

    void waitQueueIdle();

    [[nodiscard]] Buffer* createBuffer(size_t size);
    void destroyBuffer(Buffer* buffer);

    filters::BloomFilter* createBloomFilter();
    filters::ComposeColorShadowReflectionFilter* createComposeColorShadowReflectionFilter();
    filters::ComposeOpacityShadowFilter* createComposeOpacityShadowFilter();
    filters::ToneMapFilter* createToneMapFilter();
    filters::DenoiserFilter* createDenoiserFilter();

    void destroyFilter(filters::Filter* filter);

    Image* createImage(const ImageDescription& desc);
    Image* createFromVkSampledImage(vk::Image image, const ImageDescription& desc);
    Image* createImageFromDx11Texture(HANDLE dx11textureHandle, const ImageDescription& desc);
    void destroyImage(Image* image);
    void copyBufferToImage(Buffer* buffer, Image* image);
    void copyImageToBuffer(Image* image, Buffer* buffer);
    void copyImage(Image* src, Image* dst);

    [[nodiscard]] boost::uuids::uuid generateNextTag() { return m_objects.generateNextTag(); }

    [[nodiscard]] vk::helper::DeviceContext& deviceContext() noexcept { return m_deviceContext; }

    [[nodiscard]] const vk::helper::DeviceContext& deviceContext() const noexcept { return m_deviceContext; }

private:
    // order is matter. First should be cleared all m_objects, then denoiser dev, than main graph. dev
    vk::helper::DeviceContext m_deviceContext;
    oidn::DeviceRef m_denoiserDevice;
    ContextObjectContainer m_objects;
};

}