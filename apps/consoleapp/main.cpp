#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb/stb_image.h>
#include <stb/stb_image_write.h>

#include "common/HybridProRenderer.h"
#include "common/rprpp_wrappers/Buffer.h"
#include "common/rprpp_wrappers/Context.h"
#include "common/rprpp_wrappers/filters/BloomFilter.h"
#include "common/rprpp_wrappers/filters/ComposeColorShadowReflectionFilter.h"
#include "common/rprpp_wrappers/filters/ComposeOpacityShadowFilter.h"
#include "common/rprpp_wrappers/filters/DenoiserFilter.h"
#include "common/rprpp_wrappers/filters/ToneMapFilter.h"
#include <filesystem>
#include <iostream>

#define WIDTH 1200
#define HEIGHT 700
#define INTEROP true
#define DEVICE_ID 0
#define ITERATIONS 100

#ifdef NDEBUG
// please note that when we use frames in flight > 1
// hybridpro produces Validation Error with VK_OBJECT_TYPE_QUERY_POOL message looks like "query not reset. After query pool creation"
#define FRAMES_IN_FLIGHT 4
#else
#define FRAMES_IN_FLIGHT 1
#endif

void savePngImage(const char* filename, void* img, uint32_t width, uint32_t height, RprPpImageFormat format);
void copyRprFbToBuffer(HybridProRenderer& r, rprpp::wrappers::Buffer& buffer, rpr_aov aov);
void runWithInterop(const std::filesystem::path& exeDirPath, int device_id);
void runWithoutInterop(const std::filesystem::path& exeDirPath, int device_id);

int main(int argc, const char* argv[])
{
    std::cout << "ConsoleApp started..." << std::endl;
    try {
        uint32_t deviceCount;
        RPRPP_CHECK(rprppGetDeviceCount(&deviceCount));
        for (size_t i = 0; i < deviceCount; i++) {
            size_t size;
            RPRPP_CHECK(rprppGetDeviceInfo(i, RPRPP_DEVICE_INFO_NAME, nullptr, 0, &size));
            std::vector<char> deviceName;
            deviceName.resize(size);
            RPRPP_CHECK(rprppGetDeviceInfo(i, RPRPP_DEVICE_INFO_NAME, deviceName.data(), size, nullptr));
            std::cout << "Device id = " << i << ", name = " << std::string(deviceName.begin(), deviceName.end()) << std::endl;
        }

        std::filesystem::path exeDirPath = std::filesystem::path(argv[0]).parent_path();
        if (INTEROP) {
            runWithInterop(exeDirPath, DEVICE_ID);
        } else {
            runWithoutInterop(exeDirPath, DEVICE_ID);
        }

    } catch (const std::runtime_error& e) {
        printf("%s\n", e.what());
        return EXIT_FAILURE;
    }
    std::cout << "ConsoleApp finished..." << std::endl;
    return 0;
}

void runWithInterop(const std::filesystem::path& exeDirPath, int deviceId)
{
    std::filesystem::path hybridproDll = exeDirPath / "HybridPro.dll";
    std::filesystem::path hybridproCacheDir = exeDirPath / "hybridpro_cache";
    std::filesystem::path assetsDir = exeDirPath;

    RprPpImageFormat format = RPRPP_IMAGE_FROMAT_R32G32B32A32_SFLOAT;
    rprpp::wrappers::Context ppContext(deviceId);
    rprpp::wrappers::filters::BloomFilter bloomFilter(ppContext);
    rprpp::wrappers::filters::ComposeColorShadowReflectionFilter composeColorShadowReflectionFilter(ppContext);
    rprpp::wrappers::filters::DenoiserFilter denoiserFilter(ppContext);
    rprpp::wrappers::filters::ToneMapFilter tonemapFilter(ppContext);
    rprpp::wrappers::Buffer buffer(ppContext, WIDTH * HEIGHT * rprpp::wrappers::to_pixel_size(format));

    std::vector<RprPpVkFence> fences;
    std::vector<RprPpVkSemaphore> frameBuffersReleaseSemaphores;
    for (uint32_t i = 0; i < FRAMES_IN_FLIGHT; i++) {
        RprPpVkSemaphore semaphore;
        RPRPP_CHECK(rprppVkCreateSemaphore(ppContext.getVkDevice(), &semaphore));
        frameBuffersReleaseSemaphores.push_back(semaphore);

        RprPpVkFence fence;
        RPRPP_CHECK(rprppVkCreateFence(ppContext.getVkDevice(), RPRPP_TRUE, &fence));
        fences.push_back(fence);
    }

    // set frame buffers realese to signal state
    RprPpVkSubmitInfo submitInfo;
    submitInfo.waitSemaphoreCount = 0;
    submitInfo.pSignalSemaphores = frameBuffersReleaseSemaphores.data();
    submitInfo.signalSemaphoreCount = frameBuffersReleaseSemaphores.size();
    RPRPP_CHECK(rprppVkQueueSubmit(ppContext.getVkQueue(), submitInfo, nullptr));

    HybridProInteropInfo aovsInteropInfo = HybridProInteropInfo {
        .physicalDevice = ppContext.getVkPhysicalDevice(),
        .device = ppContext.getVkDevice(),
        .framesInFlight = FRAMES_IN_FLIGHT,
        .frameBuffersReleaseSemaphores = frameBuffersReleaseSemaphores.data(),
    };

    HybridProRenderer renderer(deviceId, aovsInteropInfo, hybridproDll, hybridproCacheDir, assetsDir);
    auto frameBuffersReadySemaphores = renderer.getFrameBuffersReadySemaphores();

    renderer.resize(WIDTH, HEIGHT);

    RprPpImageDescription outputDesc = {
        .width = WIDTH,
        .height = HEIGHT,
        .format = format,
    };
    rprpp::wrappers::Image output = rprpp::wrappers::Image::create(ppContext, outputDesc);
    RprPpImageDescription aovsDesc = {
        .width = WIDTH,
        .height = HEIGHT,
        .format = RPRPP_IMAGE_FROMAT_R32G32B32A32_SFLOAT,
    };
    rprpp::wrappers::Image aovColor = rprpp::wrappers::Image::createFromVkSampledImage(ppContext, (RprPpVkImage)renderer.getAovVkImage(RPR_AOV_COLOR), aovsDesc);
    rprpp::wrappers::Image aovOpacity = rprpp::wrappers::Image::createFromVkSampledImage(ppContext, (RprPpVkImage)renderer.getAovVkImage(RPR_AOV_OPACITY), aovsDesc);
    rprpp::wrappers::Image aovShadowCatcher = rprpp::wrappers::Image::createFromVkSampledImage(ppContext, (RprPpVkImage)renderer.getAovVkImage(RPR_AOV_SHADOW_CATCHER), aovsDesc);
    rprpp::wrappers::Image aovReflectionCatcher = rprpp::wrappers::Image::createFromVkSampledImage(ppContext, (RprPpVkImage)renderer.getAovVkImage(RPR_AOV_REFLECTION_CATCHER), aovsDesc);
    rprpp::wrappers::Image aovMattePass = rprpp::wrappers::Image::createFromVkSampledImage(ppContext, (RprPpVkImage)renderer.getAovVkImage(RPR_AOV_MATTE_PASS), aovsDesc);
    rprpp::wrappers::Image aovBackground = rprpp::wrappers::Image::createFromVkSampledImage(ppContext, (RprPpVkImage)renderer.getAovVkImage(RPR_AOV_BACKGROUND), aovsDesc);

    composeColorShadowReflectionFilter.setOutput(output);
    composeColorShadowReflectionFilter.setInput(aovColor);
    composeColorShadowReflectionFilter.setAovOpacity(aovOpacity);
    composeColorShadowReflectionFilter.setAovShadowCatcher(aovShadowCatcher);
    composeColorShadowReflectionFilter.setAovReflectionCatcher(aovReflectionCatcher);
    composeColorShadowReflectionFilter.setAovMattePass(aovMattePass);
    composeColorShadowReflectionFilter.setAovBackground(aovBackground);

    denoiserFilter.setOutput(output);
    denoiserFilter.setInput(output);

    bloomFilter.setOutput(output);
    bloomFilter.setInput(output);
    bloomFilter.setRadius(0.03f);
    bloomFilter.setThreshold(0.0f);
    bloomFilter.setIntensity(0.2f);

    tonemapFilter.setOutput(output);
    tonemapFilter.setInput(output);
    tonemapFilter.setFocalLength(renderer.getFocalLength() / 1000.0f);

    uint32_t currentFrame = 0;
    for (size_t i = 0; i < ITERATIONS; i++) {
        renderer.render();
        renderer.flushFrameBuffers();

        RprPpVkFence fence = fences[currentFrame];
        RPRPP_CHECK(rprppVkWaitForFences(ppContext.getVkDevice(), 1, &fence, true, UINT64_MAX));
        RPRPP_CHECK(rprppVkResetFences(ppContext.getVkDevice(), 1, &fence));

        uint32_t semaphoreIndex = renderer.getSemaphoreIndex();
        RprPpVkSemaphore aovsReadySemaphore = frameBuffersReadySemaphores[semaphoreIndex];
        RprPpVkSemaphore aovReleasedSemaphore = frameBuffersReleaseSemaphores[semaphoreIndex];

        RprPpVkSemaphore filterFinished = composeColorShadowReflectionFilter.run(aovsReadySemaphore);
        filterFinished = denoiserFilter.run(filterFinished);
        filterFinished = bloomFilter.run(filterFinished);
        filterFinished = tonemapFilter.run(filterFinished);

        RprPpVkSubmitInfo submitInfo;
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = &filterFinished;
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = &aovReleasedSemaphore;
        RPRPP_CHECK(rprppVkQueueSubmit(ppContext.getVkQueue(), submitInfo, fence));
        currentFrame = (currentFrame + 1) % FRAMES_IN_FLIGHT;

        if (i == 0 || i == ITERATIONS - 1) {
            ppContext.waitQueueIdle();
            size_t size;
            ppContext.copyImageToBuffer(output.get(), buffer.get());

            auto resultPath = exeDirPath / ("result_with_interop_" + std::to_string(i) + ".png");
            std::filesystem::remove(resultPath);
            savePngImage(resultPath.string().c_str(), buffer.map(buffer.size()), WIDTH, HEIGHT, format);
            buffer.unmap();
        }
    }

    ppContext.waitQueueIdle();

    for (auto f : fences) {
        RPRPP_CHECK(rprppVkDestroyFence(ppContext.getVkDevice(), f));
    }

    for (auto s : frameBuffersReleaseSemaphores) {
        RPRPP_CHECK(rprppVkDestroySemaphore(ppContext.getVkDevice(), s));
    }
}

void runWithoutInterop(const std::filesystem::path& exeDirPath, int deviceId)
{
    std::filesystem::path hybridproDll = exeDirPath / "HybridPro.dll";
    std::filesystem::path hybridproCacheDir = exeDirPath / "hybridpro_cache";
    std::filesystem::path assetsDir = exeDirPath;

    HybridProRenderer renderer(deviceId, std::nullopt, hybridproDll, hybridproCacheDir, assetsDir);
    renderer.resize(WIDTH, HEIGHT);

    RprPpImageFormat format = RPRPP_IMAGE_FROMAT_R8G8B8A8_UNORM;
    rprpp::wrappers::Context ppContext(deviceId);
    rprpp::wrappers::filters::BloomFilter bloomFilter(ppContext);
    rprpp::wrappers::filters::ComposeColorShadowReflectionFilter composeColorShadowReflectionFilter(ppContext);
    rprpp::wrappers::filters::DenoiserFilter denoiserFilter(ppContext);
    rprpp::wrappers::filters::ToneMapFilter tonemapFilter(ppContext);
    // this buffer should handle hdr for aovs and hdr/ldr for output
    rprpp::wrappers::Buffer buffer(ppContext, WIDTH * HEIGHT * 4 * sizeof(float));

    RprPpImageDescription outputDesc = {
        .width = WIDTH,
        .height = HEIGHT,
        .format = format,
    };
    rprpp::wrappers::Image output = rprpp::wrappers::Image::create(ppContext, outputDesc);
    RprPpImageDescription aovsDesc = {
        .width = WIDTH,
        .height = HEIGHT,
        .format = RPRPP_IMAGE_FROMAT_R32G32B32A32_SFLOAT,
    };
    rprpp::wrappers::Image aovColor = rprpp::wrappers::Image::create(ppContext, aovsDesc);
    rprpp::wrappers::Image aovOpacity = rprpp::wrappers::Image::create(ppContext, aovsDesc);
    rprpp::wrappers::Image aovShadowCatcher = rprpp::wrappers::Image::create(ppContext, aovsDesc);
    rprpp::wrappers::Image aovReflectionCatcher = rprpp::wrappers::Image::create(ppContext, aovsDesc);
    rprpp::wrappers::Image aovMattePass = rprpp::wrappers::Image::create(ppContext, aovsDesc);
    rprpp::wrappers::Image aovBackground = rprpp::wrappers::Image::create(ppContext, aovsDesc);

    composeColorShadowReflectionFilter.setOutput(output);
    composeColorShadowReflectionFilter.setInput(aovColor);
    composeColorShadowReflectionFilter.setAovOpacity(aovOpacity);
    composeColorShadowReflectionFilter.setAovShadowCatcher(aovShadowCatcher);
    composeColorShadowReflectionFilter.setAovReflectionCatcher(aovReflectionCatcher);
    composeColorShadowReflectionFilter.setAovMattePass(aovMattePass);
    composeColorShadowReflectionFilter.setAovBackground(aovBackground);

    denoiserFilter.setOutput(output);
    denoiserFilter.setInput(output);

    bloomFilter.setOutput(output);
    bloomFilter.setInput(output);
    bloomFilter.setRadius(0.03f);
    bloomFilter.setThreshold(0.0f);
    bloomFilter.setIntensity(0.2f);

    tonemapFilter.setOutput(output);
    tonemapFilter.setInput(output);
    tonemapFilter.setFocalLength(renderer.getFocalLength() / 1000.0f);

    for (size_t i = 0; i < ITERATIONS; i++) {
        renderer.render();

        copyRprFbToBuffer(renderer, buffer, RPR_AOV_COLOR);
        ppContext.copyBufferToImage(buffer.get(), aovColor.get());
        copyRprFbToBuffer(renderer, buffer, RPR_AOV_OPACITY);
        ppContext.copyBufferToImage(buffer.get(), aovOpacity.get());
        copyRprFbToBuffer(renderer, buffer, RPR_AOV_SHADOW_CATCHER);
        ppContext.copyBufferToImage(buffer.get(), aovShadowCatcher.get());
        copyRprFbToBuffer(renderer, buffer, RPR_AOV_REFLECTION_CATCHER);
        ppContext.copyBufferToImage(buffer.get(), aovReflectionCatcher.get());
        copyRprFbToBuffer(renderer, buffer, RPR_AOV_MATTE_PASS);
        ppContext.copyBufferToImage(buffer.get(), aovMattePass.get());
        copyRprFbToBuffer(renderer, buffer, RPR_AOV_BACKGROUND);
        ppContext.copyBufferToImage(buffer.get(), aovBackground.get());

        RprPpVkSemaphore filterFinished = composeColorShadowReflectionFilter.run();
        filterFinished = denoiserFilter.run(filterFinished);
        filterFinished = bloomFilter.run(filterFinished);
        filterFinished = tonemapFilter.run(filterFinished);
        ppContext.waitQueueIdle();

        if (i == 0 || i == ITERATIONS - 1) {
            ppContext.copyImageToBuffer(output.get(), buffer.get());

            auto resultPath = exeDirPath / ("result_without_interop_" + std::to_string(i) + ".png");
            std::filesystem::remove(resultPath);
            savePngImage(resultPath.string().c_str(), buffer.map(rprpp::wrappers::to_pixel_size(format) * WIDTH * HEIGHT), WIDTH, HEIGHT, format);
            buffer.unmap();
        }
    }
}

void copyRprFbToBuffer(HybridProRenderer& r, rprpp::wrappers::Buffer& buffer, rpr_aov aov)
{
    size_t size;
    r.getAov(aov, nullptr, 0u, &size);
    r.getAov(aov, buffer.map(size), size, nullptr);
    buffer.unmap();
}

inline uint8_t floatToByte(float value)
{
    if (value >= 1.0f) {
        return 255;
    }
    if (value <= 0.0f) {
        return 0;
    }
    return roundf(value * 255.0f);
}

void savePngImage(const char* filename, void* img, uint32_t width, uint32_t height, RprPpImageFormat format)
{
    size_t numComponents = 4;
    uint8_t* dst = (uint8_t*)img;
    std::vector<uint8_t> vDst;
    if (format != RPRPP_IMAGE_FROMAT_R8G8B8A8_UNORM) {
        switch (format) {
        case RPRPP_IMAGE_FROMAT_R32G32B32A32_SFLOAT: {
            vDst.resize(width * height * numComponents);
            dst = vDst.data();

            float* hdrImg = (float*)img;
            for (size_t i = 0; i < width * height * numComponents; i++) {
                dst[i] = floatToByte(hdrImg[i]);
            }
            /* code */
            break;
        }
        default:
            throw std::runtime_error("not implemented image format conversation");
        }
    }

    stbi_write_png(filename, width, height, numComponents, dst, width * numComponents);
}