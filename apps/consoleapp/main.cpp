#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image.h>
#include <stb_image_write.h>

#include "../common/HybridProRenderer.h"
#include "../common/RprPostProcessing.h"
#include <filesystem>
#include <iostream>

#define WIDTH 1200
#define HEIGHT 700

void savePngImage(const char* filename, uint8_t* img, uint32_t width, uint32_t height, RprPpImageFormat format);
void copyRprFbToPpStagingBuffer(HybridProRenderer& r, RprPostProcessing& pp, rpr_aov aov);

int main(int argc, const char* argv[])
{
    std::cout << "ConsoleApp started..." << std::endl;
    try {
        int deviceId = 0;
        std::filesystem::path exeDirPath = std::filesystem::path(argv[0]).parent_path();
        std::filesystem::path hybridproDll = exeDirPath / "HybridPro.dll";
        std::filesystem::path hybridproCacheDir = exeDirPath / "hybridpro_cache";
        std::filesystem::path assetsDir = exeDirPath;

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

        RprPpImageFormat format = RPRPP_IMAGE_FROMAT_R32G32B32A32_SFLOAT;
        RprPostProcessing postProcessing(deviceId);
        postProcessing.resize(WIDTH, HEIGHT, format);

        HybridProRenderer renderer(WIDTH,
            HEIGHT,
            deviceId,
            hybridproDll,
            hybridproCacheDir,
            assetsDir);

        std::vector<uint8_t> output;
        for (size_t i = 0; i <= 100; i++) {
            renderer.render(1);
            if (i == 0 || i == 100) {
                copyRprFbToPpStagingBuffer(renderer, postProcessing, RPR_AOV_COLOR);
                postProcessing.copyStagingBufferToAovColor();
                copyRprFbToPpStagingBuffer(renderer, postProcessing, RPR_AOV_OPACITY);
                postProcessing.copyStagingBufferToAovOpacity();
                copyRprFbToPpStagingBuffer(renderer, postProcessing, RPR_AOV_SHADOW_CATCHER);
                postProcessing.copyStagingBufferToAovShadowCatcher();
                copyRprFbToPpStagingBuffer(renderer, postProcessing, RPR_AOV_REFLECTION_CATCHER);
                postProcessing.copyStagingBufferToAovReflectionCatcher();
                copyRprFbToPpStagingBuffer(renderer, postProcessing, RPR_AOV_MATTE_PASS);
                postProcessing.copyStagingBufferToAovMattePass();
                copyRprFbToPpStagingBuffer(renderer, postProcessing, RPR_AOV_BACKGROUND);
                postProcessing.copyStagingBufferToAovBackground();
                postProcessing.run();

                size_t size;
                postProcessing.getOutput(nullptr, 0, &size);
                output.resize(size);
                postProcessing.getOutput(output.data(), size, nullptr);

                auto resultPath = exeDirPath / ("result_" + std::to_string(i) + ".png");
                std::filesystem::remove(resultPath);
                savePngImage(resultPath.string().c_str(), output.data(), WIDTH, HEIGHT, format);
            }
        }

    } catch (const std::runtime_error& e) {
        printf("%s\n", e.what());
        return EXIT_FAILURE;
    }
    std::cout << "ConsoleApp finished..." << std::endl;
    return 0;
}

void copyRprFbToPpStagingBuffer(HybridProRenderer& r, RprPostProcessing& pp, rpr_aov aov)
{
    size_t size;
    r.getAov(aov, nullptr, 0u, &size);
    void* data = pp.mapStagingBuffer(size);
    r.getAov(aov, data, size, nullptr);
    pp.unmapStagingBuffer();
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

void savePngImage(const char* filename, uint8_t* img, uint32_t width, uint32_t height, RprPpImageFormat format)
{
    size_t numComponents = 4;
    uint8_t* dst = img;
    std::vector<uint8_t> vDst;
    if (format != RPRPP_IMAGE_FROMAT_R8G8B8A8_UNORM) {
        vDst.resize(width * height * numComponents);
        dst = vDst.data();
        switch (format) {
        case RPRPP_IMAGE_FROMAT_R32G32B32A32_SFLOAT: {
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