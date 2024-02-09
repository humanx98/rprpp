#ifndef __RPRPP_H
#define __RPRPP_H

#include <stdint.h>

#define RPRPP_TRUE 1u
#define RPRPP_FALSE 0u

typedef enum RprPpError {
    RPRPP_SUCCESS = 0,
    RPRPP_ERROR_INTERNAL_ERROR = 1,
    RPRPP_ERROR_INVALID_DEVICE = 2,
    RPRPP_ERROR_INVALID_PARAMETER = 3,
    RPRPP_ERROR_SHADER_COMPILATION = 4,
} RprPpError;

typedef enum RprPpDeviceInfo {
    RPRPP_DEVICE_INFO_NAME = 0,
} RprPpDeviceInfo;

typedef enum RprPpImageFormat {
    RPRPP_IMAGE_FROMAT_R8G8B8A8_UNORM = 0,
    RPRPP_IMAGE_FROMAT_R32G32B32A32_SFLOAT = 1,
} RprPpImageFormat;

typedef void* RprPpContext;
typedef void* RprPpDx11Handle;
typedef void* RprPpVkHandle;

RprPpError rprppGetDeviceCount(uint32_t* deviceCount);
RprPpError rprppGetDeviceInfo(uint32_t deviceId, RprPpDeviceInfo deviceInfo, void* data, size_t size, size_t* sizeRet);

RprPpError rprppCreateContext(uint32_t enableValidationLayer, uint32_t deviceId, RprPpContext* outContext);
RprPpError rprppDestroyContext(RprPpContext context);

RprPpError rprppContextGetOutput(RprPpContext context, uint8_t* dst, size_t size, size_t* retSize);
RprPpError rprppContextGetVkPhysicalDevice(RprPpContext context, RprPpVkHandle* physicalDevice);
RprPpError rprppContextGetVkDevice(RprPpContext context, RprPpVkHandle* device);
RprPpError rprppContextResize(RprPpContext context, uint32_t width, uint32_t height, RprPpImageFormat format, RprPpDx11Handle sharedDx11TextureHandle);
RprPpError rprppContextRun(RprPpContext context);

RprPpError rprppContextMapStagingBuffer(RprPpContext context, size_t size, void** data);
RprPpError rprppContextUnmapStagingBuffer(RprPpContext context);
RprPpError rprppContextCopyStagingBufferToAovColor(RprPpContext context);
RprPpError rprppContextCopyStagingBufferToAovOpacity(RprPpContext context);
RprPpError rprppContextCopyStagingBufferToAovShadowCatcher(RprPpContext context);
RprPpError rprppContextCopyStagingBufferToAovReflectionCatcher(RprPpContext context);
RprPpError rprppContextCopyStagingBufferToAovMattePass(RprPpContext context);
RprPpError rprppContextCopyStagingBufferToAovBackground(RprPpContext context);

RprPpError rprppContextSetToneMapWhitepoint(RprPpContext context, float x, float y, float z);
RprPpError rprppContextSetToneMapVignetting(RprPpContext context, float vignetting);
RprPpError rprppContextSetToneMapCrushBlacks(RprPpContext context, float crushBlacks);
RprPpError rprppContextSetToneMapBurnHighlights(RprPpContext context, float burnHighlights);
RprPpError rprppContextSetToneMapSaturation(RprPpContext context, float saturation);
RprPpError rprppContextSetToneMapCm2Factor(RprPpContext context, float cm2Factor);
RprPpError rprppContextSetToneMapFilmIso(RprPpContext context, float filmIso);
RprPpError rprppContextSetToneMapCameraShutter(RprPpContext context, float cameraShutter);
RprPpError rprppContextSetToneMapFNumber(RprPpContext context, float fNumber);
RprPpError rprppContextSetToneMapFocalLength(RprPpContext context, float focalLength);
RprPpError rprppContextSetToneMapAperture(RprPpContext context, float aperture);
RprPpError rprppContextSetBloomRadius(RprPpContext context, float radius);
RprPpError rprppContextSetBloomBrightnessScale(RprPpContext context, float brightnessScale);
RprPpError rprppContextSetBloomThreshold(RprPpContext context, float threshold);
RprPpError rprppContextSetBloomEnabled(RprPpContext context, uint32_t enabled);
RprPpError rprppContextSetGamma(RprPpContext context, float gamma);
RprPpError rprppContextSetShadowIntensity(RprPpContext context, float shadowIntensity);
RprPpError rprppContextSetDenoiserEnabled(RprPpContext context, uint32_t enabled);

#endif