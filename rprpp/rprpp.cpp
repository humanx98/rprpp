#include "rprpp.h"

#include "Context.h"
#include "Error.h"
#include "vk/DeviceContext.h"

#include <cassert>
#include <concepts>
#include <functional>
#include <mutex>
#include <optional>
#include <type_traits>
#include <utility>

#include <iostream>

static std::mutex Mutex;
static map<rprpp::Context> GlobalContextObjects;

template <class Function,
    class... Params>
[[nodiscard("Please, don't ignore result")]] inline auto safeCall(Function function, Params&&... params) noexcept -> std::expected<std::invoke_result_t<Function, Params&&...>, RprPpError>
{
    try {
        if constexpr (!std::is_same<std::invoke_result_t<Function, Params&&...>, void>::value) {
            return std::invoke(function, std::forward<Params>(params)...);
        } else {
            std::invoke(function, std::forward<Params>(params)...);
            return {};
        }
    } catch (const rprpp::Error& e) {
        std::cerr << e.what();
        return std::unexpected(static_cast<RprPpError>(e.getErrorCode()));
    } catch (const std::exception& e) {
        std::cerr << e.what();
        return std::unexpected(RPRPP_ERROR_INTERNAL_ERROR);
    } catch (...) {
        std::cerr << "unkown error";
        return std::unexpected(RPRPP_ERROR_INTERNAL_ERROR);
    }
}

#define check(status)        \
    if (!status.has_value()) \
        return status.error();

RprPpError rprppGetDeviceCount(uint32_t* deviceCount)
{
    auto result = safeCall(vk::helper::getDeviceCount);
    check(result);

    *deviceCount = *result;

    return RPRPP_SUCCESS;
}

RprPpError rprppGetDeviceInfo(uint32_t deviceId, RprPpDeviceInfo deviceInfo, void* data, size_t size, size_t* sizeRet)
{
    auto result = safeCall(vk::helper::getDeviceInfo, deviceId, static_cast<vk::helper::DeviceInfo>(deviceInfo), data, size, sizeRet);
    check(result);

    return RPRPP_SUCCESS;
}

RprPpError rprppCreateContext(uint32_t deviceId, RprPpContext* outContext)
{
    assert(outContext);

    auto result = safeCall([&]() {
        auto contextRef = std::make_unique<rprpp::Context>(deviceId);
        *outContext = contextRef.get();

        // avoid memleak
        std::lock_guard<std::mutex> lock(Mutex);
        GlobalContextObjects.emplace(contextRef.get(), std::move(contextRef));
    }); 
    check(result);

    return RPRPP_SUCCESS;
}

RprPpError rprppDestroyContext(RprPpContext context)
{
    if (!context) {
        std::cerr << "[WARING] context in null, but it should never be";
        return RPRPP_SUCCESS;
    }

    std::lock_guard<std::mutex> lock(Mutex);
    auto result = safeCall([context]() {
        GlobalContextObjects.erase(static_cast<rprpp::Context*>(context));
    });
    check(result);

    return RPRPP_SUCCESS;
}

RprPpError rprppContextCreateBloomFilter(RprPpContext context, RprPpFilter* outFilter)
{
    assert(context);
    assert(outFilter);

    auto result = safeCall([&] {
        rprpp::Context* ctx = static_cast<rprpp::Context*>(context);
        *outFilter = ctx->createBloomFilter();
    });
    check(result);

    return RPRPP_SUCCESS;
}

RprPpError rprppContextCreateComposeColorShadowReflectionFilter(RprPpContext context, RprPpFilter* outFilter)
{
    assert(context);
    assert(outFilter);

    auto result = safeCall([&] {
        rprpp::Context* ctx = static_cast<rprpp::Context*>(context);
        *outFilter = ctx->createComposeColorShadowReflectionFilter();
    });
    check(result);

    return RPRPP_SUCCESS;
}

RprPpError rprppContextCreateComposeOpacityShadowFilter(RprPpContext context, RprPpFilter* outFilter)
{
    assert(context);
    assert(outFilter);

    auto result = safeCall([&] {
        rprpp::Context* ctx = static_cast<rprpp::Context*>(context);
        *outFilter = ctx->createComposeOpacityShadowFilter();
    });
    check(result);

    return RPRPP_SUCCESS;
}

RprPpError rprppContextCreateDenoiserFilter(RprPpContext context, RprPpFilter* outFilter)
{
    assert(context);
    assert(outFilter);

    auto result = safeCall([&] {
        rprpp::Context* ctx = static_cast<rprpp::Context*>(context);
        *outFilter = ctx->createDenoiserFilter();
    });
    check(result);

    return RPRPP_SUCCESS;
}

RprPpError rprppContextCreateToneMapFilter(RprPpContext context, RprPpFilter* outFilter)
{
    assert(context);
    assert(outFilter);

    auto result = safeCall([&] {
        rprpp::Context* ctx = static_cast<rprpp::Context*>(context);
        *outFilter = ctx->createToneMapFilter();
    });
    check(result);

    return RPRPP_SUCCESS;
}

RprPpError rprppContextDestroyFilter(RprPpContext context, RprPpFilter filter)
{
    assert(context);
    assert(filter);

    auto result = safeCall([&] {
        rprpp::Context* ctx = static_cast<rprpp::Context*>(context);
        ctx->destroyFilter(static_cast<rprpp::filters::Filter*>(filter));
    });
    check(result);

    return RPRPP_SUCCESS;
}

RprPpError rprppContextCreateBuffer(RprPpContext context, size_t size, RprPpBuffer* outBuffer)
{
    assert(context);
    assert(outBuffer);

    auto result = safeCall([&] {
        rprpp::Context* ctx = static_cast<rprpp::Context*>(context);
        *outBuffer = ctx->createBuffer(size);
    });
    check(result);

    return RPRPP_SUCCESS;
}

RprPpError rprppContextDestroyBuffer(RprPpContext context, RprPpBuffer buffer)
{
    assert(context);
    assert(buffer);

    auto result = safeCall([&] {
        rprpp::Context* ctx = static_cast<rprpp::Context*>(context);
        ctx->destroyBuffer(static_cast<rprpp::Buffer*>(buffer));
    });
    check(result);

    return RPRPP_SUCCESS;
}

RprPpError rprppContextCreateImage(RprPpContext context, RprPpImageDescription description, RprPpImage* outImage)
{
    assert(context);
    assert(outImage);

    auto result = safeCall([&] {
        rprpp::Context* ctx = static_cast<rprpp::Context*>(context);
        *outImage = ctx->createImage(rprpp::ImageDescription(description));
    });
    check(result);

    return RPRPP_SUCCESS;
}

RprPpError rprppContextCreateImageFromVkSampledImage(RprPpContext context, RprPpVkImage vkSampledImage, RprPpImageDescription description, RprPpImage* outImage)
{
    assert(context);
    assert(outImage);

    auto result = safeCall([&] {
        rprpp::Context* ctx = static_cast<rprpp::Context*>(context);
        *outImage = ctx->createFromVkSampledImage(static_cast<vk::Image>(static_cast<VkImage>(vkSampledImage)), rprpp::ImageDescription(description));
    });
    check(result);

    return RPRPP_SUCCESS;
}

RprPpError rprppContextCreateImageFromDx11Texture(RprPpContext context, RprPpDx11Handle dx11textureHandle, RprPpImageDescription description, RprPpImage* outImage)
{
    assert(context);
    assert(outImage);

    auto result = safeCall([&] {
        rprpp::Context* ctx = static_cast<rprpp::Context*>(context);
        *outImage = ctx->createImageFromDx11Texture(dx11textureHandle, rprpp::ImageDescription(description));
    });
    check(result);

    return RPRPP_SUCCESS;
}

RprPpError rprppContextDestroyImage(RprPpContext context, RprPpImage image)
{
    assert(context);
    assert(image);

    auto result = safeCall([&] {
        rprpp::Context* ctx = static_cast<rprpp::Context*>(context);
        ctx->destroyImage(static_cast<rprpp::Image*>(image));
    });
    check(result);

    return RPRPP_SUCCESS;
}

RprPpError rprppContextCopyBufferToImage(RprPpContext context, RprPpBuffer buffer, RprPpImage image)
{
    assert(context);
    assert(image);
    assert(buffer);

    auto result = safeCall([&] {
        rprpp::Context* ctx = static_cast<rprpp::Context*>(context);
        ctx->copyBufferToImage(static_cast<rprpp::Buffer*>(buffer), static_cast<rprpp::Image*>(image));
    });
    check(result);

    return RPRPP_SUCCESS;
}

RprPpError rprppContextCopyImageToBuffer(RprPpContext context, RprPpImage image, RprPpBuffer buffer)
{
    assert(context);
    assert(image);
    assert(buffer);

    auto result = safeCall([&] {
        rprpp::Context* ctx = static_cast<rprpp::Context*>(context);
        ctx->copyImageToBuffer(static_cast<rprpp::Image*>(image), static_cast<rprpp::Buffer*>(buffer));
    });
    check(result);

    return RPRPP_SUCCESS;
}

RprPpError rprppContextCopyImage(RprPpContext context, RprPpImage src, RprPpImage dst)
{
    assert(context);
    assert(src);
    assert(dst);

    auto result = safeCall([&] {
        rprpp::Context* ctx = static_cast<rprpp::Context*>(context);
        ctx->copyImage(static_cast<rprpp::Image*>(src), static_cast<rprpp::Image*>(dst));
    });
    check(result);

    return RPRPP_SUCCESS;
}

RprPpError rprppContextGetVkPhysicalDevice(RprPpContext context, RprPpVkPhysicalDevice* physicalDevice)
{
    assert(context);
    assert(physicalDevice);

    auto result = safeCall([&] {
        rprpp::Context* ctx = static_cast<rprpp::Context*>(context);
        *physicalDevice = ctx->getVkPhysicalDevice();
    });
    check(result);

    return RPRPP_SUCCESS;
}

RprPpError rprppContextGetVkDevice(RprPpContext context, RprPpVkDevice* device)
{
    assert(context);
    assert(device);

    auto result = safeCall([&] {
        rprpp::Context* ctx = static_cast<rprpp::Context*>(context);
        *device = ctx->getVkDevice();
    });
    check(result);

    return RPRPP_SUCCESS;
}

RprPpError rprppContextGetVkQueue(RprPpContext context, RprPpVkQueue* queue)
{
    assert(context);
    assert(queue);

    auto result = safeCall([&] {
        rprpp::Context* ctx = static_cast<rprpp::Context*>(context);
        *queue = ctx->getVkQueue();
    });
    check(result);

    return RPRPP_SUCCESS;
}

RprPpError rprppContextWaitQueueIdle(RprPpContext context)
{
    assert(context);

    auto result = safeCall([&] {
        rprpp::Context* ctx = static_cast<rprpp::Context*>(context);
        ctx->waitQueueIdle();
    });
    check(result);

    return RPRPP_SUCCESS;
}

// Filter
RprPpError rprppFilterRun(RprPpFilter filter, RprPpVkSemaphore waitSemaphore, RprPpVkSemaphore* finishedSemaphore)
{
    assert(filter);

    auto result = safeCall([&] {
        rprpp::filters::Filter* f = static_cast<rprpp::filters::Filter*>(filter);

        std::optional<vk::Semaphore> waitSemaphoreOptional;
        if (waitSemaphore != nullptr) {
            waitSemaphoreOptional = static_cast<vk::Semaphore>(static_cast<VkSemaphore>(waitSemaphore));
        }

        *finishedSemaphore = (VkSemaphore)f->run(waitSemaphoreOptional);
    });
    check(result);

    return RPRPP_SUCCESS;
}

RprPpError rprppFilterSetInput(RprPpFilter filter, RprPpImage image)
{
    assert(filter);

    auto result = safeCall([&] {
        rprpp::filters::Filter* f = static_cast<rprpp::filters::Filter*>(filter);
        f->setInput(static_cast<rprpp::Image*>(image));
    });
    check(result);

    return RPRPP_SUCCESS;
}

RprPpError rprppFilterSetOutput(RprPpFilter filter, RprPpImage image)
{
    assert(filter);

    auto result = safeCall([&] {
        rprpp::filters::Filter* f = static_cast<rprpp::filters::Filter*>(filter);
        f->setOutput(static_cast<rprpp::Image*>(image));
    });
    check(result);

    return RPRPP_SUCCESS;
}

RprPpError rprppBloomFilterSetRadius(RprPpFilter filter, float radius)
{
    assert(filter);

    auto result = safeCall([&] {
        rprpp::filters::BloomFilter* f = static_cast<rprpp::filters::BloomFilter*>(filter);
        f->setRadius(radius);
    });
    check(result);

    return RPRPP_SUCCESS;
}

RprPpError rprppBloomFilterSetBrightnessScale(RprPpFilter filter, float brightnessScale)
{
    assert(filter);

    auto result = safeCall([&] {
        rprpp::filters::BloomFilter* f = static_cast<rprpp::filters::BloomFilter*>(filter);
        f->setBrightnessScale(brightnessScale);
    });
    check(result);

    return RPRPP_SUCCESS;
}

RprPpError rprppBloomFilterSetThreshold(RprPpFilter filter, float threshold)
{
    assert(filter);

    auto result = safeCall([&] {
        rprpp::filters::BloomFilter* f = static_cast<rprpp::filters::BloomFilter*>(filter);
        f->setThreshold(threshold);
    });
    check(result);

    return RPRPP_SUCCESS;
}

RprPpError rprppBloomFilterGetRadius(RprPpFilter filter, float* radius)
{
    assert(filter);

    auto result = safeCall([&] {
        rprpp::filters::BloomFilter* f = static_cast<rprpp::filters::BloomFilter*>(filter);

        if (radius != nullptr) {
            *radius = f->getRadius();
        }
    });
    check(result);

    return RPRPP_SUCCESS;
}

RprPpError rprppBloomFilterGetBrightnessScale(RprPpFilter filter, float* brightnessScale)
{
    assert(filter);

    auto result = safeCall([&] {
        rprpp::filters::BloomFilter* f = static_cast<rprpp::filters::BloomFilter*>(filter);

        if (brightnessScale != nullptr) {
            *brightnessScale = f->getBrightnessScale();
        }
    });
    check(result);

    return RPRPP_SUCCESS;
}

RprPpError rprppBloomFilterGetThreshold(RprPpFilter filter, float* threshold)
{
    assert(filter);

    auto result = safeCall([&] {
        rprpp::filters::BloomFilter* f = static_cast<rprpp::filters::BloomFilter*>(filter);

        if (threshold != nullptr) {
            *threshold = f->getThreshold();
        }
    });
    check(result);

    return RPRPP_SUCCESS;
}

RprPpError rprppComposeColorShadowReflectionFilterSetAovOpacity(RprPpFilter filter, RprPpImage image)
{
    assert(filter);

    auto result = safeCall([&] {
        rprpp::filters::ComposeColorShadowReflectionFilter* f = static_cast<rprpp::filters::ComposeColorShadowReflectionFilter*>(filter);
        f->setAovOpacity(static_cast<rprpp::Image*>(image));
    });
    check(result);

    return RPRPP_SUCCESS;
}

RprPpError rprppComposeColorShadowReflectionFilterSetAovShadowCatcher(RprPpFilter filter, RprPpImage image)
{
    assert(filter);

    auto result = safeCall([&] {
        rprpp::filters::ComposeColorShadowReflectionFilter* f = static_cast<rprpp::filters::ComposeColorShadowReflectionFilter*>(filter);
        f->setAovShadowCatcher(static_cast<rprpp::Image*>(image));
    });
    check(result);

    return RPRPP_SUCCESS;
}

RprPpError rprppComposeColorShadowReflectionFilterSetAovReflectionCatcher(RprPpFilter filter, RprPpImage image)
{
    assert(filter);

    auto result = safeCall([&] {
        rprpp::filters::ComposeColorShadowReflectionFilter* f = static_cast<rprpp::filters::ComposeColorShadowReflectionFilter*>(filter);
        f->setAovReflectionCatcher(static_cast<rprpp::Image*>(image));
    });
    check(result);

    return RPRPP_SUCCESS;
}

RprPpError rprppComposeColorShadowReflectionFilterSetAovMattePass(RprPpFilter filter, RprPpImage image)
{
    assert(filter);

    auto result = safeCall([&] {
        rprpp::filters::ComposeColorShadowReflectionFilter* f = static_cast<rprpp::filters::ComposeColorShadowReflectionFilter*>(filter);
        f->setAovMattePass(static_cast<rprpp::Image*>(image));
    });
    check(result);

    return RPRPP_SUCCESS;
}

RprPpError rprppComposeColorShadowReflectionFilterSetAovBackground(RprPpFilter filter, RprPpImage image)
{
    assert(filter);

    auto result = safeCall([&] {
        rprpp::filters::ComposeColorShadowReflectionFilter* f = static_cast<rprpp::filters::ComposeColorShadowReflectionFilter*>(filter);
        f->setAovBackground(static_cast<rprpp::Image*>(image));
    });
    check(result);

    return RPRPP_SUCCESS;
}

RprPpError rprppComposeColorShadowReflectionFilterSetTileOffset(RprPpFilter filter, uint32_t x, uint32_t y)
{
    assert(filter);

    auto result = safeCall([&] {
        rprpp::filters::ComposeColorShadowReflectionFilter* f = static_cast<rprpp::filters::ComposeColorShadowReflectionFilter*>(filter);
        f->setTileOffset(x, y);
    });
    check(result);

    return RPRPP_SUCCESS;
}

RprPpError rprppComposeColorShadowReflectionFilterSetShadowIntensity(RprPpFilter filter, float shadowIntensity)
{
    assert(filter);

    auto result = safeCall([&] {
        rprpp::filters::ComposeColorShadowReflectionFilter* f = static_cast<rprpp::filters::ComposeColorShadowReflectionFilter*>(filter);
        f->setShadowIntensity(shadowIntensity);
    });
    check(result);

    return RPRPP_SUCCESS;
}

RprPpError rprppComposeColorShadowReflectionFilterSetNotRefractiveBackgroundColor(RprPpFilter filter, float x, float y, float z)
{
    assert(filter);

    auto result = safeCall([&] {
        rprpp::filters::ComposeColorShadowReflectionFilter* f = static_cast<rprpp::filters::ComposeColorShadowReflectionFilter*>(filter);
        f->setNotRefractiveBackgroundColor(x, y, z);
    });
    check(result);

    return RPRPP_SUCCESS;
}

RprPpError rprppComposeColorShadowReflectionFilterSetNotRefractiveBackgroundColorWeight(RprPpFilter filter, float weight)
{
    assert(filter);

    auto result = safeCall([&] {
        rprpp::filters::ComposeColorShadowReflectionFilter* f = static_cast<rprpp::filters::ComposeColorShadowReflectionFilter*>(filter);
        f->setNotRefractiveBackgroundColorWeight(weight);
    });
    check(result);

    return RPRPP_SUCCESS;
}

RprPpError rprppComposeColorShadowReflectionFilterGetTileOffset(RprPpFilter filter, uint32_t* x, uint32_t* y)
{
    assert(filter);

    auto result = safeCall([&] {
        rprpp::filters::ComposeColorShadowReflectionFilter* f = static_cast<rprpp::filters::ComposeColorShadowReflectionFilter*>(filter);
        uint32_t xy[2];
        f->getTileOffset(xy[0], xy[1]);

        if (x != nullptr) {
            *x = xy[0];
        }

        if (y != nullptr) {
            *y = xy[1];
        }
    });
    check(result);

    return RPRPP_SUCCESS;
}

RprPpError rprppComposeColorShadowReflectionFilterGetShadowIntensity(RprPpFilter filter, float* shadowIntensity)
{
    assert(filter);

    auto result = safeCall([&] {
        rprpp::filters::ComposeColorShadowReflectionFilter* f = static_cast<rprpp::filters::ComposeColorShadowReflectionFilter*>(filter);

        if (shadowIntensity != nullptr) {
            *shadowIntensity = f->getShadowIntensity();
        }
    });
    check(result);

    return RPRPP_SUCCESS;
}

RprPpError rprppComposeColorShadowReflectionFilterGetNotRefractiveBackgroundColor(RprPpFilter filter, float* x, float* y, float* z)
{
    assert(filter);

    auto result = safeCall([&] {
        rprpp::filters::ComposeColorShadowReflectionFilter* f = static_cast<rprpp::filters::ComposeColorShadowReflectionFilter*>(filter);
        float color[3];
        f->getNotRefractiveBackgroundColor(color[0], color[1], color[2]);
        
        if (x != nullptr) {
            *x = color[0];
        }

        if (y != nullptr) {
            *y = color[1];
        }

        if (z != nullptr) {
            *z = color[2];
        }
    });
    check(result);

    return RPRPP_SUCCESS;
}

RprPpError rprppComposeColorShadowReflectionFilterGetNotRefractiveBackgroundColorWeight(RprPpFilter filter, float* weight)
{
    assert(filter);

    auto result = safeCall([&] {
        rprpp::filters::ComposeColorShadowReflectionFilter* f = static_cast<rprpp::filters::ComposeColorShadowReflectionFilter*>(filter);

        if (weight != nullptr) {
            *weight = f->getNotRefractiveBackgroundColorWeight();
        }
    });
    check(result);

    return RPRPP_SUCCESS;
}

RprPpError rprppComposeOpacityShadowFilterSetAovShadowCatcher(RprPpFilter filter, RprPpImage image)
{
    assert(filter);

    auto result = safeCall([&] {
        rprpp::filters::ComposeOpacityShadowFilter* f = static_cast<rprpp::filters::ComposeOpacityShadowFilter*>(filter);
        f->setAovShadowCatcher(static_cast<rprpp::Image*>(image));
    });
    check(result);

    return RPRPP_SUCCESS;
}

RprPpError rprppComposeOpacityShadowFilterSetTileOffset(RprPpFilter filter, uint32_t x, uint32_t y)
{
    assert(filter);

    auto result = safeCall([&] {
        rprpp::filters::ComposeOpacityShadowFilter* f = static_cast<rprpp::filters::ComposeOpacityShadowFilter*>(filter);
        f->setTileOffset(x, y);
    });
    check(result);

    return RPRPP_SUCCESS;
}

RprPpError rprppComposeOpacityShadowFilterSetShadowIntensity(RprPpFilter filter, float shadowIntensity)
{
    assert(filter);

    auto result = safeCall([&] {
        rprpp::filters::ComposeOpacityShadowFilter* f = static_cast<rprpp::filters::ComposeOpacityShadowFilter*>(filter);
        f->setShadowIntensity(shadowIntensity);
    });
    check(result);

    return RPRPP_SUCCESS;
}

RprPpError rprppComposeOpacityShadowFilterGetTileOffset(RprPpFilter filter, uint32_t* x, uint32_t* y)
{
    assert(filter);

    auto result = safeCall([&] {
        rprpp::filters::ComposeOpacityShadowFilter* f = static_cast<rprpp::filters::ComposeOpacityShadowFilter*>(filter);
        uint32_t xy[2];
        f->getTileOffset(xy[0], xy[1]);

        if (x != nullptr) {
            *x = xy[0];
        }

        if (y != nullptr) {
            *y = xy[1];
        }
    });
    check(result);

    return RPRPP_SUCCESS;
}

RprPpError rprppComposeOpacityShadowFilterGetShadowIntensity(RprPpFilter filter, float* shadowIntensity)
{
    assert(filter);

    auto result = safeCall([&] {
        rprpp::filters::ComposeOpacityShadowFilter* f = static_cast<rprpp::filters::ComposeOpacityShadowFilter*>(filter);

        if (shadowIntensity != nullptr) {
            *shadowIntensity = f->getShadowIntensity();
        }
    });
    check(result);

    return RPRPP_SUCCESS;
}

RprPpError rprppToneMapFilterSetWhitepoint(RprPpFilter filter, float x, float y, float z)
{
    assert(filter);

    auto result = safeCall([&] {
        rprpp::filters::ToneMapFilter* f = static_cast<rprpp::filters::ToneMapFilter*>(filter);
        f->setWhitepoint(x, y, z);
    });
    check(result);

    return RPRPP_SUCCESS;
}

RprPpError rprppToneMapFilterSetVignetting(RprPpFilter filter, float vignetting)
{
    assert(filter);

    auto result = safeCall([&] {
        rprpp::filters::ToneMapFilter* f = static_cast<rprpp::filters::ToneMapFilter*>(filter);
        f->setVignetting(vignetting);
    });
    check(result);

    return RPRPP_SUCCESS;
}

RprPpError rprppToneMapFilterSetCrushBlacks(RprPpFilter filter, float crushBlacks)
{
    assert(filter);

    auto result = safeCall([&] {
        rprpp::filters::ToneMapFilter* f = static_cast<rprpp::filters::ToneMapFilter*>(filter);
        f->setCrushBlacks(crushBlacks);
    });
    check(result);

    return RPRPP_SUCCESS;
}

RprPpError rprppToneMapFilterSetBurnHighlights(RprPpFilter filter, float burnHighlights)
{
    assert(filter);

    auto result = safeCall([&] {
        rprpp::filters::ToneMapFilter* f = static_cast<rprpp::filters::ToneMapFilter*>(filter);
        f->setBurnHighlights(burnHighlights);
    });
    check(result);

    return RPRPP_SUCCESS;
}

RprPpError rprppToneMapFilterSetSaturation(RprPpFilter filter, float saturation)
{
    assert(filter);

    auto result = safeCall([&] {
        rprpp::filters::ToneMapFilter* f = static_cast<rprpp::filters::ToneMapFilter*>(filter);
        f->setSaturation(saturation);
    });
    check(result);

    return RPRPP_SUCCESS;
}

RprPpError rprppToneMapFilterSetCm2Factor(RprPpFilter filter, float cm2Factor)
{
    assert(filter);

    auto result = safeCall([&] {
        rprpp::filters::ToneMapFilter* f = static_cast<rprpp::filters::ToneMapFilter*>(filter);
        f->setCm2Factor(cm2Factor);
    });
    check(result);

    return RPRPP_SUCCESS;
}

RprPpError rprppToneMapFilterSetFilmIso(RprPpFilter filter, float filmIso)
{
    assert(filter);

    auto result = safeCall([&] {
        rprpp::filters::ToneMapFilter* f = static_cast<rprpp::filters::ToneMapFilter*>(filter);
        f->setFilmIso(filmIso);
    });
    check(result);

    return RPRPP_SUCCESS;
}

RprPpError rprppToneMapFilterSetCameraShutter(RprPpFilter filter, float cameraShutter)
{
    assert(filter);

    auto result = safeCall([&] {
        rprpp::filters::ToneMapFilter* f = static_cast<rprpp::filters::ToneMapFilter*>(filter);
        f->setCameraShutter(cameraShutter);
    });
    check(result);

    return RPRPP_SUCCESS;
}

RprPpError rprppToneMapFilterSetFNumber(RprPpFilter filter, float fNumber)
{
    assert(filter);

    auto result = safeCall([&] {
        rprpp::filters::ToneMapFilter* f = static_cast<rprpp::filters::ToneMapFilter*>(filter);
        f->setFNumber(fNumber);
    });
    check(result);

    return RPRPP_SUCCESS;
}

RprPpError rprppToneMapFilterSetFocalLength(RprPpFilter filter, float focalLength)
{
    assert(filter);

    auto result = safeCall([&] {
        rprpp::filters::ToneMapFilter* f = static_cast<rprpp::filters::ToneMapFilter*>(filter);
        f->setFocalLength(focalLength);
    });
    check(result);

    return RPRPP_SUCCESS;
}

RprPpError rprppToneMapFilterSetAperture(RprPpFilter filter, float aperture)
{
    assert(filter);

    auto result = safeCall([&] {
        rprpp::filters::ToneMapFilter* f = static_cast<rprpp::filters::ToneMapFilter*>(filter);
        f->setAperture(aperture);
    });
    check(result);

    return RPRPP_SUCCESS;
}

RprPpError rprppToneMapFilterSetGamma(RprPpFilter filter, float gamma)
{
    assert(filter);

    auto result = safeCall([&] {
        rprpp::filters::ToneMapFilter* f = static_cast<rprpp::filters::ToneMapFilter*>(filter);
        f->setGamma(gamma);
    });
    check(result);

    return RPRPP_SUCCESS;
}

RprPpError rprppToneMapFilterGetWhitepoint(RprPpFilter filter, float* x, float* y, float* z)
{
    assert(filter);

    auto result = safeCall([&] {
        rprpp::filters::ToneMapFilter* f = static_cast<rprpp::filters::ToneMapFilter*>(filter);
        float whitepoint[3];
        f->getWhitepoint(whitepoint[0], whitepoint[1], whitepoint[2]);

        if (x != nullptr) {
            *x = whitepoint[0];
        }

        if (y != nullptr) {
            *y = whitepoint[1];
        }

        if (z != nullptr) {
            *z = whitepoint[2];
        }
    });
    check(result);

    return RPRPP_SUCCESS;
}

RprPpError rprppToneMapFilterGetVignetting(RprPpFilter filter, float* vignetting)
{
    assert(filter);

    auto result = safeCall([&] {
        rprpp::filters::ToneMapFilter* f = static_cast<rprpp::filters::ToneMapFilter*>(filter);

        if (vignetting != nullptr) {
            *vignetting = f->getVignetting();
        }
    });
    check(result);

    return RPRPP_SUCCESS;
}

RprPpError rprppToneMapFilterGetCrushBlacks(RprPpFilter filter, float* crushBlacks)
{
    assert(filter);

    auto result = safeCall([&] {
        rprpp::filters::ToneMapFilter* f = static_cast<rprpp::filters::ToneMapFilter*>(filter);

        if (crushBlacks != nullptr) {
            *crushBlacks = f->getCrushBlacks();
        }
    });
    check(result);

    return RPRPP_SUCCESS;
}

RprPpError rprppToneMapFilterGetBurnHighlights(RprPpFilter filter, float* burnHighlights)
{
    assert(filter);

    auto result = safeCall([&] {
        rprpp::filters::ToneMapFilter* f = static_cast<rprpp::filters::ToneMapFilter*>(filter);

        if (burnHighlights != nullptr) {
            *burnHighlights = f->getBurnHighlights();
        }
    });
    check(result);

    return RPRPP_SUCCESS;
}

RprPpError rprppToneMapFilterGetSaturation(RprPpFilter filter, float* saturation)
{
    assert(filter);

    auto result = safeCall([&] {
        rprpp::filters::ToneMapFilter* f = static_cast<rprpp::filters::ToneMapFilter*>(filter);

        if (saturation != nullptr) {
            *saturation = f->getSaturation();
        }
    });
    check(result);

    return RPRPP_SUCCESS;
}

RprPpError rprppToneMapFilterGetCm2Factor(RprPpFilter filter, float* cm2Factor)
{
    assert(filter);

    auto result = safeCall([&] {
        rprpp::filters::ToneMapFilter* f = static_cast<rprpp::filters::ToneMapFilter*>(filter);

        if (cm2Factor != nullptr) {
            *cm2Factor = f->getCm2Factor();
        }
    });
    check(result);

    return RPRPP_SUCCESS;
}

RprPpError rprppToneMapFilterGetFilmIso(RprPpFilter filter, float* filmIso)
{
    assert(filter);

    auto result = safeCall([&] {
        rprpp::filters::ToneMapFilter* f = static_cast<rprpp::filters::ToneMapFilter*>(filter);

        if (filmIso != nullptr) {
            *filmIso = f->getFilmIso();
        }
    });
    check(result);

    return RPRPP_SUCCESS;
}

RprPpError rprppToneMapFilterGetCameraShutter(RprPpFilter filter, float* cameraShutter)
{
    assert(filter);

    auto result = safeCall([&] {
        rprpp::filters::ToneMapFilter* f = static_cast<rprpp::filters::ToneMapFilter*>(filter);

        if (cameraShutter != nullptr) {
            *cameraShutter = f->getCameraShutter();
        }
    });
    check(result);

    return RPRPP_SUCCESS;
}

RprPpError rprppToneMapFilterGetFNumber(RprPpFilter filter, float* fNumber)
{
    assert(filter);

    auto result = safeCall([&] {
        rprpp::filters::ToneMapFilter* f = static_cast<rprpp::filters::ToneMapFilter*>(filter);

        if (fNumber != nullptr) {
            *fNumber = f->getFNumber();
        }
    });
    check(result);

    return RPRPP_SUCCESS;
}

RprPpError rprppToneMapFilterGetFocalLength(RprPpFilter filter, float* focalLength)
{
    assert(filter);

    auto result = safeCall([&] {
        rprpp::filters::ToneMapFilter* f = static_cast<rprpp::filters::ToneMapFilter*>(filter);

        if (focalLength != nullptr) {
            *focalLength = f->getFocalLength();
        }
    });
    check(result);

    return RPRPP_SUCCESS;
}

RprPpError rprppToneMapFilterGetAperture(RprPpFilter filter, float* aperture)
{
    assert(filter);

    auto result = safeCall([&] {
        rprpp::filters::ToneMapFilter* f = static_cast<rprpp::filters::ToneMapFilter*>(filter);

        if (aperture != nullptr) {
            *aperture = f->getAperture();
        }
    });
    check(result);

    return RPRPP_SUCCESS;
}

RprPpError rprppToneMapFilterGetGamma(RprPpFilter filter, float* gamma)
{
    assert(filter);

    auto result = safeCall([&] {
        rprpp::filters::ToneMapFilter* f = static_cast<rprpp::filters::ToneMapFilter*>(filter);

        if (gamma != nullptr) {
            *gamma = f->getGamma();
        }
    });
    check(result);

    return RPRPP_SUCCESS;
}

RprPpError rprppBufferMap(RprPpBuffer buffer, size_t size, void** outdata)
{
    assert(buffer);

    auto result = safeCall([&] {
        rprpp::Buffer* buff = static_cast<rprpp::Buffer*>(buffer);

        if (outdata != nullptr) {
            *outdata = buff->map(size);
        }
    });
    check(result);

    return RPRPP_SUCCESS;
}

RprPpError rprppBufferUnmap(RprPpBuffer buffer)
{
    assert(buffer);

    auto result = safeCall([&] {
        rprpp::Buffer* buff = static_cast<rprpp::Buffer*>(buffer);
        buff->unmap();
    });
    check(result);

    return RPRPP_SUCCESS;
}

RprPpError rprppVkCreateSemaphore(RprPpVkDevice device, RprPpVkSemaphore* outSemaphore)
{
    assert(device);

    auto result = safeCall([&] {
        vk::Device vkdevice = static_cast<VkDevice>(device);
        *outSemaphore = (VkSemaphore)vkdevice.createSemaphore({});
    });
    check(result);

    return RPRPP_SUCCESS;
}

RprPpError rprppVkDestroySemaphore(RprPpVkDevice device, RprPpVkSemaphore semaphore)
{
    assert(device);

    auto result = safeCall([&] {
        vk::Device vkdevice = static_cast<VkDevice>(device);
        vk::Semaphore vksemaphore = static_cast<VkSemaphore>(semaphore);
        vkdevice.destroySemaphore(vksemaphore);
    });
    check(result);

    return RPRPP_SUCCESS;
}

RprPpError rprppVkCreateFence(RprPpVkDevice device, RprPpBool signaled, RprPpVkFence* outFence)
{
    assert(device);

    auto result = safeCall([&] {
        vk::Device vkdevice = static_cast<VkDevice>(device);
        vk::FenceCreateInfo info;
        if (signaled == RPRPP_TRUE) {
            info.flags = vk::FenceCreateFlagBits::eSignaled;
        }
        *outFence = (VkFence)vkdevice.createFence(info);
    });
    check(result);

    return RPRPP_SUCCESS;
}

RprPpError rprppVkDestroyFence(RprPpVkDevice device, RprPpVkFence fence)
{
    assert(device);

    auto result = safeCall([&] {
        vk::Device vkdevice = static_cast<VkDevice>(device);
        vk::Fence vkfence = static_cast<VkFence>(fence);
        vkdevice.destroyFence(vkfence);
    });
    check(result);

    return RPRPP_SUCCESS;
}

RprPpError rprppVkWaitForFences(RprPpVkDevice rprppDevice, uint32_t fenceCount, RprPpVkFence* rprppFences, RprPpBool waitAll, uint64_t timeout)
{
    assert(rprppDevice);

    auto result = safeCall([&] {
        vk::Device device = static_cast<VkDevice>(rprppDevice);
        vk::Fence* pFences = reinterpret_cast<vk::Fence*>(rprppFences);
        vk::resultCheck(device.waitForFences(fenceCount, pFences, waitAll, timeout), "rprppVkWaitForFences");
    });
    check(result);

    return RPRPP_SUCCESS;
}

RprPpError rprppVkResetFences(RprPpVkDevice rprppDevice, uint32_t fenceCount, RprPpVkFence* rprppFences)
{
    assert(rprppDevice);

    auto result = safeCall([&] {
        vk::Device device = static_cast<VkDevice>(rprppDevice);
        vk::Fence* pFences = reinterpret_cast<vk::Fence*>(rprppFences);
        vk::resultCheck(device.resetFences(fenceCount, pFences), "rprppVkResetFences");
    });
    check(result);

    return RPRPP_SUCCESS;
}

RprPpError rprppVkQueueSubmit(RprPpVkQueue rprppQueue, RprPpVkSubmitInfo rprppSubmitInfo, RprPpVkFence rprppFence)
{
    assert(rprppQueue);

    auto result = safeCall([&] {
        vk::Queue queue = static_cast<VkQueue>(rprppQueue);
        vk::Fence fence = static_cast<VkFence>(rprppFence);

        std::vector<vk::PipelineStageFlags> waitDstStageMask;
        waitDstStageMask.resize(rprppSubmitInfo.waitSemaphoreCount, vk::PipelineStageFlagBits::eAllCommands);

        vk::SubmitInfo submitInfo;
        submitInfo.pWaitDstStageMask = waitDstStageMask.data();
        submitInfo.pWaitSemaphores = reinterpret_cast<vk::Semaphore*>(rprppSubmitInfo.pWaitSemaphores);
        submitInfo.waitSemaphoreCount = rprppSubmitInfo.waitSemaphoreCount;
        submitInfo.pSignalSemaphores = reinterpret_cast<vk::Semaphore*>(rprppSubmitInfo.pSignalSemaphores);
        submitInfo.signalSemaphoreCount = rprppSubmitInfo.signalSemaphoreCount;
        vk::resultCheck(queue.submit(1, &submitInfo, static_cast<vk::Fence>(fence)), "rprppVkQueueSubmit");
    });
    check(result);

    return RPRPP_SUCCESS;
}
