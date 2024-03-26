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

#include <boost/uuid/uuid_generators.hpp>
//#include <boost/uuid/uuid_hash.hpp>
#include <boost/noncopyable.hpp>

#include <OpenImageDenoise/oidn.hpp>

#include <unordered_set>
#include "ContextObject.h"


template <class T>
using map = std::unordered_map<T*, std::unique_ptr<T>>;

namespace rprpp {

class ContextObject;

class ContextObjectContainer 
{
    struct ContextObjectEqual
    {
        using is_transparent = void;

        bool operator()(const ContextObjectRef& obj1, const ContextObjectRef& obj2) const
        {
            return *obj1 == *obj2;
        }

        bool operator()(boost::uuids::uuid uuid, const ContextObjectRef& obj2) const
        {
            return uuid == obj2->tag();
        }

    };

    struct ContextObjectHasher 
    {
        using hash_type = std::hash<ContextObject>;
        using is_transparent = void;

        hash_type hasher;

        size_t operator()(const ContextObjectRef& obj) const
        {
            return hasher(*obj);
        }

        size_t operator()(boost::uuids::uuid uuid) const
        {
            return hasher(uuid);
        }
    };

public:
    template <class T, class... Params>
    auto emplace(Params&&... params)
    {
        return m_objects.emplace(std::make_unique<T>(std::forward<Params>(params)...));
    }

    template <class T, class... Params>
    T* emplaceCastReturn(Params&&... params)
    {
        const auto iter = emplace<T>(std::forward<Params>(params)...);
        return static_cast<T*>(iter.first->get());
    }

    auto erase(boost::uuids::uuid uuid) 
    {
        auto iter = m_objects.find(uuid);
        return m_objects.erase(iter);
    }

    [[warning("Performance penalty")]]
    void erase(ContextObject* address) 
    {
        for (auto iter = m_objects.begin(); iter != m_objects.end(); ++iter) {
            const ContextObject* ptr = iter->get();
            if (!ptr)
                continue;

            if (ptr == address) {
                break;
            }

        }
    }




 private:
    std::unordered_set<ContextObjectRef, ContextObjectHasher, ContextObjectEqual> m_objects;
};


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

    boost::uuids::uuid generateNextTag() { return m_tagGenerator(); }
    vk::helper::DeviceContext& deviceContext() noexcept { return m_deviceContext; }
    const vk::helper::DeviceContext& deviceContext() const noexcept { return m_deviceContext; }

private:
    boost::uuids::random_generator m_tagGenerator;

    vk::helper::DeviceContext m_deviceContext;
    oidn::DeviceRef m_oidnDevice;

    ContextObjectContainer m_objects;

};

}