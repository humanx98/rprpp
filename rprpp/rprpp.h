#ifndef __RPRPP_H
#define __RPRPP_H

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
    RPRPP_DEVICE_INFO_UUID = 2,
    RPRPP_DEVICE_INFO_SUPPORT_HARDWARE_RAY_TRACING = 3,
    RPRPP_DEVICE_INFO_SUPPORT_GPU_DENOISER = 4,
} RprPpDeviceInfo;

typedef enum RprPpImageFormat {
    RPRPP_IMAGE_FROMAT_R8G8B8A8_UNORM = 0,
    RPRPP_IMAGE_FROMAT_R32G32B32A32_SFLOAT = 1,
    RPRPP_IMAGE_FROMAT_B8G8R8A8_UNORM = 2,
} RprPpImageFormat;

typedef unsigned int RprPpBool;
typedef void* RprPpContext;
typedef void* RprPpFilter;
typedef void* RprPpBuffer;
typedef void* RprPpImage;
typedef void* RprPpDx11Handle;
typedef void* RprPpVkFence;
typedef void* RprPpVkSemaphore;
typedef void* RprPpVkPhysicalDevice;
typedef void* RprPpVkDevice;
typedef void* RprPpVkQueue;
typedef void* RprPpVkImage;

typedef struct RprPpImageDescription {
    unsigned int width;
    unsigned int height;
    RprPpImageFormat format;
} RprPpImageDescription;

typedef struct RprPpVkSubmitInfo {
    unsigned int waitSemaphoreCount;
    RprPpVkSemaphore* pWaitSemaphores;
    unsigned int signalSemaphoreCount;
    RprPpVkSemaphore* pSignalSemaphores;
} RprPpVkSubmitInfo;

#ifdef __cplusplus
extern "C" {
#endif

RPRPP_API RprPpError rprppInitialize();
RPRPP_API RprPpError rprppDestroy();

RPRPP_API RprPpError rprppGetDeviceCount(unsigned int* deviceCount);
RPRPP_API RprPpError rprppGetDeviceInfo(unsigned int deviceId, RprPpDeviceInfo deviceInfo, void* data, size_t size, size_t* sizeRet);
RPRPP_API RprPpError rprppCreateContext(unsigned int deviceId, RprPpContext* outContext);
RPRPP_API RprPpError rprppDestroyContext(RprPpContext context);
RPRPP_API RprPpError rprppContextCreateBloomFilter(RprPpContext context, RprPpFilter* outFilter);
RPRPP_API RprPpError rprppContextCreateComposeColorShadowReflectionFilter(RprPpContext context, RprPpFilter* outFilter);
RPRPP_API RprPpError rprppContextCreateComposeOpacityShadowFilter(RprPpContext context, RprPpFilter* outFilter);
RPRPP_API RprPpError rprppContextCreateDenoiserFilter(RprPpContext context, RprPpFilter* outFilter);
RPRPP_API RprPpError rprppContextCreateToneMapFilter(RprPpContext context, RprPpFilter* outFilter);
RPRPP_API RprPpError rprppContextDestroyFilter(RprPpContext context, RprPpFilter filter);
RPRPP_API RprPpError rprppContextCreateBuffer(RprPpContext context, size_t size, RprPpBuffer* outBuffer);
RPRPP_API RprPpError rprppContextDestroyBuffer(RprPpContext context, RprPpBuffer buffer);
RPRPP_API RprPpError rprppContextCreateImage(RprPpContext context, RprPpImageDescription description, RprPpImage* outImage);
RPRPP_API RprPpError rprppContextCreateImageFromVkSampledImage(RprPpContext context, RprPpVkImage vkSampledImage, RprPpImageDescription description, RprPpImage* outImage);
RPRPP_API RprPpError rprppContextCreateImageFromDx11Texture(RprPpContext context, RprPpDx11Handle dx11textureHandle, RprPpImageDescription description, RprPpImage* outImage);
RPRPP_API RprPpError rprppContextDestroyImage(RprPpContext context, RprPpImage image);
RPRPP_API RprPpError rprppContextCopyBufferToImage(RprPpContext context, RprPpBuffer buffer, RprPpImage image);
RPRPP_API RprPpError rprppContextCopyImageToBuffer(RprPpContext context, RprPpImage image, RprPpBuffer buffer);
RPRPP_API RprPpError rprppContextCopyImage(RprPpContext context, RprPpImage src, RprPpImage dst);
RPRPP_API RprPpError rprppContextGetVkPhysicalDevice(RprPpContext context, RprPpVkPhysicalDevice* physicalDevice);
RPRPP_API RprPpError rprppContextGetVkDevice(RprPpContext context, RprPpVkDevice* device);
RPRPP_API RprPpError rprppContextGetVkQueue(RprPpContext context, RprPpVkQueue* queue);
RPRPP_API RprPpError rprppContextWaitQueueIdle(RprPpContext context);

// Filter
RPRPP_API RprPpError rprppFilterRun(RprPpFilter filter, RprPpVkSemaphore waitSemaphore, RprPpVkSemaphore* finishedSemaphore);
RPRPP_API RprPpError rprppFilterSetInput(RprPpFilter filter, RprPpImage image);
RPRPP_API RprPpError rprppFilterSetOutput(RprPpFilter filter, RprPpImage image);
// Bloom Filter
RPRPP_API RprPpError rprppBloomFilterGetRadius(RprPpFilter filter, float* radius);
RPRPP_API RprPpError rprppBloomFilterGetBrightnessScale(RprPpFilter filter, float* brightnessScale);
RPRPP_API RprPpError rprppBloomFilterGetThreshold(RprPpFilter filter, float* threshold);
RPRPP_API RprPpError rprppBloomFilterSetRadius(RprPpFilter filter, float radius);
RPRPP_API RprPpError rprppBloomFilterSetBrightnessScale(RprPpFilter filter, float brightnessScale);
RPRPP_API RprPpError rprppBloomFilterSetThreshold(RprPpFilter filter, float threshold);
// ComposeColorShadowReflection Filter
RPRPP_API RprPpError rprppComposeColorShadowReflectionFilterSetAovOpacity(RprPpFilter filter, RprPpImage image);
RPRPP_API RprPpError rprppComposeColorShadowReflectionFilterSetAovShadowCatcher(RprPpFilter filter, RprPpImage image);
RPRPP_API RprPpError rprppComposeColorShadowReflectionFilterSetAovReflectionCatcher(RprPpFilter filter, RprPpImage image);
RPRPP_API RprPpError rprppComposeColorShadowReflectionFilterSetAovMattePass(RprPpFilter filter, RprPpImage image);
RPRPP_API RprPpError rprppComposeColorShadowReflectionFilterSetAovBackground(RprPpFilter filter, RprPpImage image);
RPRPP_API RprPpError rprppComposeColorShadowReflectionFilterGetTileOffset(RprPpFilter filter, unsigned int* x, unsigned int* y);
RPRPP_API RprPpError rprppComposeColorShadowReflectionFilterGetShadowIntensity(RprPpFilter filter, float* shadowIntensity);
RPRPP_API RprPpError rprppComposeColorShadowReflectionFilterGetNotRefractiveBackgroundColor(RprPpFilter filter, float* x, float* y, float* z);
RPRPP_API RprPpError rprppComposeColorShadowReflectionFilterGetNotRefractiveBackgroundColorWeight(RprPpFilter filter, float* weight);
RPRPP_API RprPpError rprppComposeColorShadowReflectionFilterSetTileOffset(RprPpFilter filter, unsigned int x, unsigned int y);
RPRPP_API RprPpError rprppComposeColorShadowReflectionFilterSetNotRefractiveBackgroundColor(RprPpFilter filter, float x, float y, float z);
RPRPP_API RprPpError rprppComposeColorShadowReflectionFilterSetNotRefractiveBackgroundColorWeight(RprPpFilter filter, float weight);
RPRPP_API RprPpError rprppComposeColorShadowReflectionFilterSetShadowIntensity(RprPpFilter filter, float shadowIntensity);
// ComposeOpacityShadow Filter
RPRPP_API RprPpError rprppComposeOpacityShadowFilterSetAovShadowCatcher(RprPpFilter filter, RprPpImage image);
RPRPP_API RprPpError rprppComposeOpacityShadowFilterGetTileOffset(RprPpFilter filter, unsigned int* x, unsigned int* y);
RPRPP_API RprPpError rprppComposeOpacityShadowFilterGetShadowIntensity(RprPpFilter filter, float* shadowIntensity);
RPRPP_API RprPpError rprppComposeOpacityShadowFilterSetTileOffset(RprPpFilter filter, unsigned int x, unsigned int y);
RPRPP_API RprPpError rprppComposeOpacityShadowFilterSetShadowIntensity(RprPpFilter filter, float shadowIntensity);
// ToneMap Filter
RPRPP_API RprPpError rprppToneMapFilterSetWhitepoint(RprPpFilter filter, float x, float y, float z);
RPRPP_API RprPpError rprppToneMapFilterSetVignetting(RprPpFilter filter, float vignetting);
RPRPP_API RprPpError rprppToneMapFilterSetCrushBlacks(RprPpFilter filter, float crushBlacks);
RPRPP_API RprPpError rprppToneMapFilterSetBurnHighlights(RprPpFilter filter, float burnHighlights);
RPRPP_API RprPpError rprppToneMapFilterSetSaturation(RprPpFilter filter, float saturation);
RPRPP_API RprPpError rprppToneMapFilterSetCm2Factor(RprPpFilter filter, float cm2Factor);
RPRPP_API RprPpError rprppToneMapFilterSetFilmIso(RprPpFilter filter, float filmIso);
RPRPP_API RprPpError rprppToneMapFilterSetCameraShutter(RprPpFilter filter, float cameraShutter);
RPRPP_API RprPpError rprppToneMapFilterSetFNumber(RprPpFilter filter, float fNumber);
RPRPP_API RprPpError rprppToneMapFilterSetFocalLength(RprPpFilter filter, float focalLength);
RPRPP_API RprPpError rprppToneMapFilterSetAperture(RprPpFilter filter, float aperture);
RPRPP_API RprPpError rprppToneMapFilterSetGamma(RprPpFilter filter, float gamma);
RPRPP_API RprPpError rprppToneMapFilterGetWhitepoint(RprPpFilter filter, float* x, float* y, float* z);
RPRPP_API RprPpError rprppToneMapFilterGetVignetting(RprPpFilter filter, float* vignetting);
RPRPP_API RprPpError rprppToneMapFilterGetCrushBlacks(RprPpFilter filter, float* crushBlacks);
RPRPP_API RprPpError rprppToneMapFilterGetBurnHighlights(RprPpFilter filter, float* burnHighlights);
RPRPP_API RprPpError rprppToneMapFilterGetSaturation(RprPpFilter filter, float* saturation);
RPRPP_API RprPpError rprppToneMapFilterGetCm2Factor(RprPpFilter filter, float* cm2Factor);
RPRPP_API RprPpError rprppToneMapFilterGetFilmIso(RprPpFilter filter, float* filmIso);
RPRPP_API RprPpError rprppToneMapFilterGetCameraShutter(RprPpFilter filter, float* cameraShutter);
RPRPP_API RprPpError rprppToneMapFilterGetFNumber(RprPpFilter filter, float* fNumber);
RPRPP_API RprPpError rprppToneMapFilterGetFocalLength(RprPpFilter filter, float* focalLength);
RPRPP_API RprPpError rprppToneMapFilterGetAperture(RprPpFilter filter, float* aperture);
RPRPP_API RprPpError rprppToneMapFilterGetGamma(RprPpFilter filter, float* gamma);
// Denoiser Filter
RPRPP_API RprPpError rprppDenoiserFilterSetAovAlbedo(RprPpFilter filter, RprPpImage image);
RPRPP_API RprPpError rprppDenoiserFilterSetAovNormal(RprPpFilter filter, RprPpImage image);

// buffer functions
RPRPP_API RprPpError rprppBufferMap(RprPpBuffer buffer, size_t size, void** outdata);
RPRPP_API RprPpError rprppBufferUnmap(RprPpBuffer buffer);

// vk functions
RPRPP_API RprPpError rprppVkCreateSemaphore(RprPpVkDevice device, RprPpVkSemaphore* outSemaphore);
RPRPP_API RprPpError rprppVkDestroySemaphore(RprPpVkDevice device, RprPpVkSemaphore semaphore);

RPRPP_API RprPpError rprppVkCreateFence(RprPpVkDevice device, RprPpBool signaled, RprPpVkFence* outFence);
RPRPP_API RprPpError rprppVkDestroyFence(RprPpVkDevice device, RprPpVkFence fence);
RPRPP_API RprPpError rprppVkWaitForFences(RprPpVkDevice device, unsigned int fenceCount, RprPpVkFence* pFences, RprPpBool waitAll, unsigned long long timeout);
RPRPP_API RprPpError rprppVkResetFences(RprPpVkDevice device, unsigned int fenceCount, RprPpVkFence* pFences);
RPRPP_API RprPpError rprppVkQueueSubmit(RprPpVkQueue queue, RprPpVkSubmitInfo submit, RprPpVkFence fence);

#ifdef __cplusplus
} // extern "C"
#endif

#endif
