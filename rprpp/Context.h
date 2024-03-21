#pragma once

#include "Buffer.h"
#include "Image.h"
#include "filters/BloomFilter.h"
#include "filters/ComposeColorShadowReflectionFilter.h"
#include "filters/ComposeOpacityShadowFilter.h"
#include "filters/DenoiserFilter.h"
#include "filters/Filter.h"
#include "filters/ToneMapFilter.h"
#include "vk/DeviceContext.h"

#include <unordered_map>

#include <OpenImageDenoise/oidn.hpp>

#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_hash.hpp>
#include <boost/noncopyable.hpp>

#include <variant>


template <class T>
using map = std::unordered_map<T*, std::unique_ptr<T>>;

namespace rprpp {

class ContextObject;

class Context : public boost::noncopyable {
public:
    explicit Context(uint32_t deviceId);

    VkPhysicalDevice getVkPhysicalDevice() const noexcept;
    VkDevice getVkDevice() const noexcept;
    VkQueue getVkQueue() const noexcept;
    void waitQueueIdle();

    Buffer* createBuffer(size_t size);
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

    boost::uuids::random_generator& tagGenerator() noexcept {   return m_tagGenerator; }
private:
    boost::uuids::random_generator m_tagGenerator;

    vk::helper::DeviceContext m_deviceContext;
    oidn::DeviceRef m_oidnDevice;

    map<filters::Filter> m_filters;
    map<Buffer> m_buffers;
    map<Image> m_images;

    using resource_t = std::variant<Buffer, Image>;

    std::unordered_map<boost::uuids::uuid, resource_t, boost::hash<boost::uuids::uuid> > m_childrens;
};

}