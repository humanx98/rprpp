#ifndef __RPRPP_H
#define __RPRPP_H

#include <stdint.h>

#ifdef _WIN32
#ifdef RPRPP_EXPORT_API
#define RPRPP_API __declspec(dllexport)
#else
#define RPRPP_API __declspec(dllimport)
#endif
#else
#define RPRPP_API __attribute__((visibility("default")))
#endif

#define RPRPP_TRUE 1u
#define RPRPP_FALSE 0u

typedef enum RprPpError {
    RPRPP_SUCCESS = 0,
    RPRPP_ERROR_INTERNAL_ERROR = -1,
    RPRPP_ERROR_INVALID_DEVICE = -2,
    RPRPP_ERROR_INVALID_PARAMETER = -3,
    RPRPP_ERROR_SHADER_COMPILATION = -4,
    RPRPP_ERROR_INVALID_OPERATION = -5,
} RprPpError;

typedef enum RprPpDeviceInfo {
    RPRPP_DEVICE_INFO_NAME = 0,
    RPRPP_DEVICE_INFO_LUID = 1,
    RPRPP_DEVICE_INFO_SUPPORT_HARDWARE_RAY_TRACING = 2,
} RprPpDeviceInfo;

typedef enum RprPpImageFormat {
    RPRPP_IMAGE_FROMAT_R8G8B8A8_UNORM = 0,
    RPRPP_IMAGE_FROMAT_R32G32B32A32_SFLOAT = 1,
    RPRPP_IMAGE_FROMAT_B8G8R8A8_UNORM = 2,
} RprPpImageFormat;

typedef uint32_t RprPpBool;
typedef void* RprPpContext;
typedef void* RprPpPostProcessing;
typedef void* RprPpBuffer;
typedef void* RprPpImage;
typedef void* RprPpDx11Handle;
typedef void* RprPpVkFence;
typedef void* RprPpVkSemaphore;
typedef void* RprPpVkPhysicalDevice;
typedef void* RprPpVkDevice;
typedef void* RprPpVkQueue;
typedef void* RprPpVkImage;

typedef struct RprPpAovsVkInteropInfo {
    RprPpVkImage color;
    RprPpVkImage opacity;
    RprPpVkImage shadowCatcher;
    RprPpVkImage reflectionCatcher;
    RprPpVkImage mattePass;
    RprPpVkImage background;
} RprPpVkInteropAovs;

typedef struct RprPpImageDescription {
    uint32_t width;
    uint32_t height;
    RprPpImageFormat format;
} RprPpImageDescription;

#ifdef __cplusplus
extern "C" {
#endif

RPRPP_API RprPpError rprppGetDeviceCount(uint32_t* deviceCount);
RPRPP_API RprPpError rprppGetDeviceInfo(uint32_t deviceId, RprPpDeviceInfo deviceInfo, void* data, size_t size, size_t* sizeRet);

RPRPP_API RprPpError rprppCreateContext(uint32_t deviceId, RprPpContext* outContext);
RPRPP_API RprPpError rprppDestroyContext(RprPpContext context);
RPRPP_API RprPpError rprppContextCreatePostProcessing(RprPpContext context, RprPpPostProcessing* outpp);
RPRPP_API RprPpError rprppContextDestroyPostProcessing(RprPpContext context, RprPpPostProcessing pp);
RPRPP_API RprPpError rprppContextCreateBuffer(RprPpContext context, size_t size, RprPpBuffer* outBuffer);
RPRPP_API RprPpError rprppContextDestroyBuffer(RprPpContext context, RprPpBuffer buffer);
RPRPP_API RprPpError rprppContextCreateImageFromDx11Texture(RprPpContext context, RprPpDx11Handle dx11textureHandle, RprPpImageDescription description, RprPpImage* outImage);
RPRPP_API RprPpError rprppContextDestroyImage(RprPpContext context, RprPpImage image);
RPRPP_API RprPpError rprppContextGetVkPhysicalDevice(RprPpContext context, RprPpVkPhysicalDevice* physicalDevice);
RPRPP_API RprPpError rprppContextGetVkDevice(RprPpContext context, RprPpVkDevice* device);
RPRPP_API RprPpError rprppContextGetVkQueue(RprPpContext context, RprPpVkQueue* queue);

// post processing functions
RPRPP_API RprPpError rprppPostProcessingResize(RprPpPostProcessing processing, uint32_t width, uint32_t height, RprPpImageFormat format, RprPpAovsVkInteropInfo* aovsVkInteropInfo);
RPRPP_API RprPpError rprppPostProcessingRun(RprPpPostProcessing processing, RprPpVkSemaphore aovsReadySemaphore, RprPpVkSemaphore toSignalAfterProcessingSemaphore);
RPRPP_API RprPpError rprppPostProcessingWaitQueueIdle(RprPpPostProcessing processing);
RPRPP_API RprPpError rprppPostProcessingCopyOutputToImage(RprPpPostProcessing processing, RprPpImage dst);
RPRPP_API RprPpError rprppPostProcessingCopyOutputToBuffer(RprPpPostProcessing processing, RprPpBuffer dst);

RPRPP_API RprPpError rprppPostProcessingCopyBufferToAovColor(RprPpPostProcessing processing, RprPpBuffer src);
RPRPP_API RprPpError rprppPostProcessingCopyBufferToAovOpacity(RprPpPostProcessing processing, RprPpBuffer src);
RPRPP_API RprPpError rprppPostProcessingCopyBufferToAovShadowCatcher(RprPpPostProcessing processing, RprPpBuffer src);
RPRPP_API RprPpError rprppPostProcessingCopyBufferToAovReflectionCatcher(RprPpPostProcessing processing, RprPpBuffer src);
RPRPP_API RprPpError rprppPostProcessingCopyBufferToAovMattePass(RprPpPostProcessing processing, RprPpBuffer src);
RPRPP_API RprPpError rprppPostProcessingCopyBufferToAovBackground(RprPpPostProcessing processing, RprPpBuffer src);

RPRPP_API RprPpError rprppPostProcessingSetToneMapWhitepoint(RprPpPostProcessing processing, float x, float y, float z);
RPRPP_API RprPpError rprppPostProcessingSetToneMapVignetting(RprPpPostProcessing processing, float vignetting);
RPRPP_API RprPpError rprppPostProcessingSetToneMapCrushBlacks(RprPpPostProcessing processing, float crushBlacks);
RPRPP_API RprPpError rprppPostProcessingSetToneMapBurnHighlights(RprPpPostProcessing processing, float burnHighlights);
RPRPP_API RprPpError rprppPostProcessingSetToneMapSaturation(RprPpPostProcessing processing, float saturation);
RPRPP_API RprPpError rprppPostProcessingSetToneMapCm2Factor(RprPpPostProcessing processing, float cm2Factor);
RPRPP_API RprPpError rprppPostProcessingSetToneMapFilmIso(RprPpPostProcessing processing, float filmIso);
RPRPP_API RprPpError rprppPostProcessingSetToneMapCameraShutter(RprPpPostProcessing processing, float cameraShutter);
RPRPP_API RprPpError rprppPostProcessingSetToneMapFNumber(RprPpPostProcessing processing, float fNumber);
RPRPP_API RprPpError rprppPostProcessingSetToneMapFocalLength(RprPpPostProcessing processing, float focalLength);
RPRPP_API RprPpError rprppPostProcessingSetToneMapAperture(RprPpPostProcessing processing, float aperture);
RPRPP_API RprPpError rprppPostProcessingSetBloomRadius(RprPpPostProcessing processing, float radius);
RPRPP_API RprPpError rprppPostProcessingSetBloomBrightnessScale(RprPpPostProcessing processing, float brightnessScale);
RPRPP_API RprPpError rprppPostProcessingSetBloomThreshold(RprPpPostProcessing processing, float threshold);
RPRPP_API RprPpError rprppPostProcessingSetBloomEnabled(RprPpPostProcessing processing, RprPpBool enabled);
RPRPP_API RprPpError rprppPostProcessingSetGamma(RprPpPostProcessing processing, float gamma);
RPRPP_API RprPpError rprppPostProcessingSetShadowIntensity(RprPpPostProcessing processing, float shadowIntensity);
RPRPP_API RprPpError rprppPostProcessingSetDenoiserEnabled(RprPpPostProcessing processing, RprPpBool enabled);

RPRPP_API RprPpError rprppPostProcessingGetToneMapWhitepoint(RprPpPostProcessing processing, float* x, float* y, float* z);
RPRPP_API RprPpError rprppPostProcessingGetToneMapVignetting(RprPpPostProcessing processing, float* vignetting);
RPRPP_API RprPpError rprppPostProcessingGetToneMapCrushBlacks(RprPpPostProcessing processing, float* crushBlacks);
RPRPP_API RprPpError rprppPostProcessingGetToneMapBurnHighlights(RprPpPostProcessing processing, float* burnHighlights);
RPRPP_API RprPpError rprppPostProcessingGetToneMapSaturation(RprPpPostProcessing processing, float* saturation);
RPRPP_API RprPpError rprppPostProcessingGetToneMapCm2Factor(RprPpPostProcessing processing, float* cm2Factor);
RPRPP_API RprPpError rprppPostProcessingGetToneMapFilmIso(RprPpPostProcessing processing, float* filmIso);
RPRPP_API RprPpError rprppPostProcessingGetToneMapCameraShutter(RprPpPostProcessing processing, float* cameraShutter);
RPRPP_API RprPpError rprppPostProcessingGetToneMapFNumber(RprPpPostProcessing processing, float* fNumber);
RPRPP_API RprPpError rprppPostProcessingGetToneMapFocalLength(RprPpPostProcessing processing, float* focalLength);
RPRPP_API RprPpError rprppPostProcessingGetToneMapAperture(RprPpPostProcessing processing, float* aperture);
RPRPP_API RprPpError rprppPostProcessingGetBloomRadius(RprPpPostProcessing processing, float* radius);
RPRPP_API RprPpError rprppPostProcessingGetBloomBrightnessScale(RprPpPostProcessing processing, float* brightnessScale);
RPRPP_API RprPpError rprppPostProcessingGetBloomThreshold(RprPpPostProcessing processing, float* threshold);
RPRPP_API RprPpError rprppPostProcessingGetBloomEnabled(RprPpPostProcessing processing, RprPpBool* enabled);
RPRPP_API RprPpError rprppPostProcessingGetGamma(RprPpPostProcessing processing, float* gamma);
RPRPP_API RprPpError rprppPostProcessingGetShadowIntensity(RprPpPostProcessing processing, float* shadowIntensity);
RPRPP_API RprPpError rprppPostProcessingGetDenoiserEnabled(RprPpPostProcessing processing, RprPpBool* enabled);

// buffer functions
RPRPP_API RprPpError rprppBufferMap(RprPpBuffer buffer, size_t size, void** outdata);
RPRPP_API RprPpError rprppBufferUnmap(RprPpBuffer buffer);

// vk functions
RPRPP_API RprPpError rprppVkCreateSemaphore(RprPpVkDevice device, RprPpVkSemaphore* outSemaphore);
RPRPP_API RprPpError rprppVkDestroySemaphore(RprPpVkDevice device, RprPpVkSemaphore semaphore);

RPRPP_API RprPpError rprppVkCreateFence(RprPpVkDevice device, RprPpBool signaled, RprPpVkFence* outFence);
RPRPP_API RprPpError rprppVkDestroyFence(RprPpVkDevice device, RprPpVkFence fence);
RPRPP_API RprPpError rprppVkWaitForFences(RprPpVkDevice device, uint32_t fenceCount, RprPpVkFence* pFences, RprPpBool waitAll, uint64_t timeout);
RPRPP_API RprPpError rprppVkResetFences(RprPpVkDevice device, uint32_t fenceCount, RprPpVkFence* pFences);

RPRPP_API RprPpError rprppVkQueueSubmitWaitAndSignal(RprPpVkQueue queue, RprPpVkSemaphore waitSemaphore, RprPpVkSemaphore signalSemaphore, RprPpVkFence fence);

#ifdef __cplusplus
}
#endif

#endif
