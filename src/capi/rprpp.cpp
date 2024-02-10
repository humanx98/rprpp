#include "rprpp.h"
#include "../Error.h"
#include "../PostProcessing.h"
#include "../vk_helper.h"

RprPpError rprppGetDeviceCount(uint32_t* deviceCount)
{
    try {
        if (deviceCount != nullptr) {
            *deviceCount = vk::helper::getDeviceCount();
        }
    } catch (const rprpp::Error& e) {
        return e.errorCode;
    } catch (const std::exception& e) {
        return RPRPP_ERROR_INTERNAL_ERROR;
    }
    return RPRPP_SUCCESS;
}

RprPpError rprppGetDeviceInfo(uint32_t deviceId, RprPpDeviceInfo deviceInfo, void* data, size_t size, size_t* sizeRet)
{
    try {
        vk::helper::getDeviceInfo(deviceId, static_cast<vk::helper::DeviceInfo>(deviceInfo), data, size, sizeRet);
    } catch (const rprpp::Error& e) {
        return e.errorCode;
    } catch (const std::exception& e) {
        return RPRPP_ERROR_INTERNAL_ERROR;
    }
    return RPRPP_SUCCESS;
}

RprPpError rprppCreateContext(uint32_t deviceId, RprPpContext* outContext)
{
    try {
        if (outContext != nullptr) {
            *outContext = rprpp::PostProcessing::create(deviceId);
        }
    } catch (const rprpp::Error& e) {
        return e.errorCode;
    } catch (const std::exception& e) {
        return RPRPP_ERROR_INTERNAL_ERROR;
    }
    return RPRPP_SUCCESS;
}
RprPpError rprppDestroyContext(RprPpContext context)
{
    try {
        if (context != nullptr) {
            rprpp::PostProcessing* pp = (rprpp::PostProcessing*)context;
            delete pp;
        }
    } catch (const rprpp::Error& e) {
        return e.errorCode;
    } catch (const std::exception& e) {
        return RPRPP_ERROR_INTERNAL_ERROR;
    }
    return RPRPP_SUCCESS;
}

RprPpError rprppContextGetOutput(RprPpContext context, uint8_t* dst, size_t size, size_t* retSize)
{
    try {
        if (context != nullptr) {
            rprpp::PostProcessing* pp = (rprpp::PostProcessing*)context;
            pp->getOutput(dst, size, retSize);
        }
    } catch (const rprpp::Error& e) {
        return e.errorCode;
    } catch (const std::exception& e) {
        return RPRPP_ERROR_INTERNAL_ERROR;
    }
    return RPRPP_SUCCESS;
}

RprPpError rprppContextGetVkPhysicalDevice(RprPpContext context, RprPpVkHandle* physicalDevice)
{
    try {
        if (context != nullptr && physicalDevice != nullptr) {
            rprpp::PostProcessing* pp = (rprpp::PostProcessing*)context;
            *physicalDevice = pp->getVkPhysicalDevice();
        }
    } catch (const rprpp::Error& e) {
        return e.errorCode;
    } catch (const std::exception& e) {
        return RPRPP_ERROR_INTERNAL_ERROR;
    }
    return RPRPP_SUCCESS;
}

RprPpError rprppContextGetVkDevice(RprPpContext context, RprPpVkHandle* device)
{
    try {
        if (context != nullptr && device != nullptr) {
            rprpp::PostProcessing* pp = (rprpp::PostProcessing*)context;
            *device = pp->getVkDevice();
        }
    } catch (const rprpp::Error& e) {
        return e.errorCode;
    } catch (const std::exception& e) {
        return RPRPP_ERROR_INTERNAL_ERROR;
    }
    return RPRPP_SUCCESS;
}

RprPpError rprppContextResize(RprPpContext context, uint32_t width, uint32_t height, RprPpImageFormat format, RprPpDx11Handle sharedDx11TextureHandle)
{
    try {
        if (context != nullptr) {
            rprpp::PostProcessing* pp = (rprpp::PostProcessing*)context;
            pp->resize(width, height, static_cast<rprpp::ImageFormat>(format), sharedDx11TextureHandle);
        }
    } catch (const rprpp::Error& e) {
        return e.errorCode;
    } catch (const std::exception& e) {
        return RPRPP_ERROR_INTERNAL_ERROR;
    }
    return RPRPP_SUCCESS;
}

RprPpError rprppContextRun(RprPpContext context)
{
    try {
        if (context != nullptr) {
            rprpp::PostProcessing* pp = (rprpp::PostProcessing*)context;
            pp->run();
        }
    } catch (const rprpp::Error& e) {
        return e.errorCode;
    } catch (const std::exception& e) {
        return RPRPP_ERROR_INTERNAL_ERROR;
    }
    return RPRPP_SUCCESS;
}

RprPpError rprppContextMapStagingBuffer(RprPpContext context, size_t size, void** data)
{
    try {
        if (context != nullptr && data != nullptr) {
            rprpp::PostProcessing* pp = (rprpp::PostProcessing*)context;
            *data = pp->mapStagingBuffer(size);
        }
    } catch (const rprpp::Error& e) {
        return e.errorCode;
    } catch (const std::exception& e) {
        return RPRPP_ERROR_INTERNAL_ERROR;
    }
    return RPRPP_SUCCESS;
}

RprPpError rprppContextUnmapStagingBuffer(RprPpContext context)
{
    try {
        if (context != nullptr) {
            rprpp::PostProcessing* pp = (rprpp::PostProcessing*)context;
            pp->unmapStagingBuffer();
        }
    } catch (const rprpp::Error& e) {
        return e.errorCode;
    } catch (const std::exception& e) {
        return RPRPP_ERROR_INTERNAL_ERROR;
    }
    return RPRPP_SUCCESS;
}

RprPpError rprppContextCopyStagingBufferToAovColor(RprPpContext context)
{
    try {
        if (context != nullptr) {
            rprpp::PostProcessing* pp = (rprpp::PostProcessing*)context;
            pp->copyStagingBufferToAovColor();
        }
    } catch (const rprpp::Error& e) {
        return e.errorCode;
    } catch (const std::exception& e) {
        return RPRPP_ERROR_INTERNAL_ERROR;
    }
    return RPRPP_SUCCESS;
}

RprPpError rprppContextCopyStagingBufferToAovOpacity(RprPpContext context)
{
    try {
        if (context != nullptr) {
            rprpp::PostProcessing* pp = (rprpp::PostProcessing*)context;
            pp->copyStagingBufferToAovOpacity();
        }
    } catch (const rprpp::Error& e) {
        return e.errorCode;
    } catch (const std::exception& e) {
        return RPRPP_ERROR_INTERNAL_ERROR;
    }
    return RPRPP_SUCCESS;
}

RprPpError rprppContextCopyStagingBufferToAovShadowCatcher(RprPpContext context)
{
    try {
        if (context != nullptr) {
            rprpp::PostProcessing* pp = (rprpp::PostProcessing*)context;
            pp->copyStagingBufferToAovShadowCatcher();
        }
    } catch (const rprpp::Error& e) {
        return e.errorCode;
    } catch (const std::exception& e) {
        return RPRPP_ERROR_INTERNAL_ERROR;
    }
    return RPRPP_SUCCESS;
}

RprPpError rprppContextCopyStagingBufferToAovReflectionCatcher(RprPpContext context)
{
    try {
        if (context != nullptr) {
            rprpp::PostProcessing* pp = (rprpp::PostProcessing*)context;
            pp->copyStagingBufferToAovReflectionCatcher();
        }
    } catch (const rprpp::Error& e) {
        return e.errorCode;
    } catch (const std::exception& e) {
        return RPRPP_ERROR_INTERNAL_ERROR;
    }
    return RPRPP_SUCCESS;
}

RprPpError rprppContextCopyStagingBufferToAovMattePass(RprPpContext context)
{
    try {
        if (context != nullptr) {
            rprpp::PostProcessing* pp = (rprpp::PostProcessing*)context;
            pp->copyStagingBufferToAovMattePass();
        }
    } catch (const rprpp::Error& e) {
        return e.errorCode;
    } catch (const std::exception& e) {
        return RPRPP_ERROR_INTERNAL_ERROR;
    }
    return RPRPP_SUCCESS;
}

RprPpError rprppContextCopyStagingBufferToAovBackground(RprPpContext context)
{
    try {
        if (context != nullptr) {
            rprpp::PostProcessing* pp = (rprpp::PostProcessing*)context;
            pp->copyStagingBufferToAovBackground();
        }
    } catch (const rprpp::Error& e) {
        return e.errorCode;
    } catch (const std::exception& e) {
        return RPRPP_ERROR_INTERNAL_ERROR;
    }
    return RPRPP_SUCCESS;
}

RprPpError rprppContextSetToneMapWhitepoint(RprPpContext context, float x, float y, float z)
{
    try {
        if (context != nullptr) {
            rprpp::PostProcessing* pp = (rprpp::PostProcessing*)context;
            pp->setToneMapWhitepoint(x, y, z);
        }
    } catch (const rprpp::Error& e) {
        return e.errorCode;
    } catch (const std::exception& e) {
        return RPRPP_ERROR_INTERNAL_ERROR;
    }
    return RPRPP_SUCCESS;
}

RprPpError rprppContextSetToneMapVignetting(RprPpContext context, float vignetting)
{
    try {
        if (context != nullptr) {
            rprpp::PostProcessing* pp = (rprpp::PostProcessing*)context;
            pp->setToneMapVignetting(vignetting);
        }
    } catch (const rprpp::Error& e) {
        return e.errorCode;
    } catch (const std::exception& e) {
        return RPRPP_ERROR_INTERNAL_ERROR;
    }
    return RPRPP_SUCCESS;
}

RprPpError rprppContextSetToneMapCrushBlacks(RprPpContext context, float crushBlacks)
{
    try {
        if (context != nullptr) {
            rprpp::PostProcessing* pp = (rprpp::PostProcessing*)context;
            pp->setToneMapCrushBlacks(crushBlacks);
        }
    } catch (const rprpp::Error& e) {
        return e.errorCode;
    } catch (const std::exception& e) {
        return RPRPP_ERROR_INTERNAL_ERROR;
    }
    return RPRPP_SUCCESS;
}

RprPpError rprppContextSetToneMapBurnHighlights(RprPpContext context, float burnHighlights)
{
    try {
        if (context != nullptr) {
            rprpp::PostProcessing* pp = (rprpp::PostProcessing*)context;
            pp->setToneMapBurnHighlights(burnHighlights);
        }
    } catch (const rprpp::Error& e) {
        return e.errorCode;
    } catch (const std::exception& e) {
        return RPRPP_ERROR_INTERNAL_ERROR;
    }
    return RPRPP_SUCCESS;
}

RprPpError rprppContextSetToneMapSaturation(RprPpContext context, float saturation)
{
    try {
        if (context != nullptr) {
            rprpp::PostProcessing* pp = (rprpp::PostProcessing*)context;
            pp->setToneMapSaturation(saturation);
        }
    } catch (const rprpp::Error& e) {
        return e.errorCode;
    } catch (const std::exception& e) {
        return RPRPP_ERROR_INTERNAL_ERROR;
    }
    return RPRPP_SUCCESS;
}

RprPpError rprppContextSetToneMapCm2Factor(RprPpContext context, float cm2Factor)
{
    try {
        if (context != nullptr) {
            rprpp::PostProcessing* pp = (rprpp::PostProcessing*)context;
            pp->setToneMapCm2Factor(cm2Factor);
        }
    } catch (const rprpp::Error& e) {
        return e.errorCode;
    } catch (const std::exception& e) {
        return RPRPP_ERROR_INTERNAL_ERROR;
    }
    return RPRPP_SUCCESS;
}

RprPpError rprppContextSetToneMapFilmIso(RprPpContext context, float filmIso)
{
    try {
        if (context != nullptr) {
            rprpp::PostProcessing* pp = (rprpp::PostProcessing*)context;
            pp->setToneMapFilmIso(filmIso);
        }
    } catch (const rprpp::Error& e) {
        return e.errorCode;
    } catch (const std::exception& e) {
        return RPRPP_ERROR_INTERNAL_ERROR;
    }
    return RPRPP_SUCCESS;
}

RprPpError rprppContextSetToneMapCameraShutter(RprPpContext context, float cameraShutter)
{
    try {
        if (context != nullptr) {
            rprpp::PostProcessing* pp = (rprpp::PostProcessing*)context;
            pp->setToneMapCameraShutter(cameraShutter);
        }
    } catch (const rprpp::Error& e) {
        return e.errorCode;
    } catch (const std::exception& e) {
        return RPRPP_ERROR_INTERNAL_ERROR;
    }
    return RPRPP_SUCCESS;
}

RprPpError rprppContextSetToneMapFNumber(RprPpContext context, float fNumber)
{
    try {
        if (context != nullptr) {
            rprpp::PostProcessing* pp = (rprpp::PostProcessing*)context;
            pp->setToneMapFNumber(fNumber);
        }
    } catch (const rprpp::Error& e) {
        return e.errorCode;
    } catch (const std::exception& e) {
        return RPRPP_ERROR_INTERNAL_ERROR;
    }
    return RPRPP_SUCCESS;
}

RprPpError rprppContextSetToneMapFocalLength(RprPpContext context, float focalLength)
{
    try {
        if (context != nullptr) {
            rprpp::PostProcessing* pp = (rprpp::PostProcessing*)context;
            pp->setToneMapFocalLength(focalLength);
        }
    } catch (const rprpp::Error& e) {
        return e.errorCode;
    } catch (const std::exception& e) {
        return RPRPP_ERROR_INTERNAL_ERROR;
    }
    return RPRPP_SUCCESS;
}

RprPpError rprppContextSetToneMapAperture(RprPpContext context, float aperture)
{
    try {
        if (context != nullptr) {
            rprpp::PostProcessing* pp = (rprpp::PostProcessing*)context;
            pp->setToneMapAperture(aperture);
        }
    } catch (const rprpp::Error& e) {
        return e.errorCode;
    } catch (const std::exception& e) {
        return RPRPP_ERROR_INTERNAL_ERROR;
    }
    return RPRPP_SUCCESS;
}

RprPpError rprppContextSetBloomRadius(RprPpContext context, float radius)
{
    try {
        if (context != nullptr) {
            rprpp::PostProcessing* pp = (rprpp::PostProcessing*)context;
            pp->setBloomRadius(radius);
        }
    } catch (const rprpp::Error& e) {
        return e.errorCode;
    } catch (const std::exception& e) {
        return RPRPP_ERROR_INTERNAL_ERROR;
    }
    return RPRPP_SUCCESS;
}

RprPpError rprppContextSetBloomBrightnessScale(RprPpContext context, float brightnessScale)
{
    try {
        if (context != nullptr) {
            rprpp::PostProcessing* pp = (rprpp::PostProcessing*)context;
            pp->setBloomBrightnessScale(brightnessScale);
        }
    } catch (const rprpp::Error& e) {
        return e.errorCode;
    } catch (const std::exception& e) {
        return RPRPP_ERROR_INTERNAL_ERROR;
    }
    return RPRPP_SUCCESS;
}

RprPpError rprppContextSetBloomThreshold(RprPpContext context, float threshold)
{
    try {
        if (context != nullptr) {
            rprpp::PostProcessing* pp = (rprpp::PostProcessing*)context;
            pp->setBloomThreshold(threshold);
        }
    } catch (const rprpp::Error& e) {
        return e.errorCode;
    } catch (const std::exception& e) {
        return RPRPP_ERROR_INTERNAL_ERROR;
    }
    return RPRPP_SUCCESS;
}

RprPpError rprppContextSetBloomEnabled(RprPpContext context, uint32_t enabled)
{
    try {
        if (context != nullptr) {
            rprpp::PostProcessing* pp = (rprpp::PostProcessing*)context;
            pp->setBloomEnabled(RPRPP_TRUE == enabled);
        }
    } catch (const rprpp::Error& e) {
        return e.errorCode;
    } catch (const std::exception& e) {
        return RPRPP_ERROR_INTERNAL_ERROR;
    }
    return RPRPP_SUCCESS;
}

RprPpError rprppContextSetGamma(RprPpContext context, float gamma)
{
    try {
        if (context != nullptr) {
            rprpp::PostProcessing* pp = (rprpp::PostProcessing*)context;
            pp->setGamma(gamma);
        }
    } catch (const rprpp::Error& e) {
        return e.errorCode;
    } catch (const std::exception& e) {
        return RPRPP_ERROR_INTERNAL_ERROR;
    }
    return RPRPP_SUCCESS;
}

RprPpError rprppContextSetShadowIntensity(RprPpContext context, float shadowIntensity)
{
    try {
        if (context != nullptr) {
            rprpp::PostProcessing* pp = (rprpp::PostProcessing*)context;
            pp->setShadowIntensity(shadowIntensity);
        }
    } catch (const rprpp::Error& e) {
        return e.errorCode;
    } catch (const std::exception& e) {
        return RPRPP_ERROR_INTERNAL_ERROR;
    }
    return RPRPP_SUCCESS;
}

RprPpError rprppContextSetDenoiserEnabled(RprPpContext context, uint32_t enabled)
{
    try {
        if (context != nullptr) {
            rprpp::PostProcessing* pp = (rprpp::PostProcessing*)context;
            pp->setDenoiserEnabled(RPRPP_TRUE == enabled);
        }
    } catch (const rprpp::Error& e) {
        return e.errorCode;
    } catch (const std::exception& e) {
        return RPRPP_ERROR_INTERNAL_ERROR;
    }
    return RPRPP_SUCCESS;
}