#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image.h>
#include <stb_image_write.h>

#include "../common/HybridProRenderer.hpp"
#include <PostProcessing.hpp>
#include <filesystem>
#include <iostream>

#define WIDTH 1200
#define HEIGHT 700

void savePngImage(const char* filename, uint8_t* img, uint32_t width, uint32_t height, rprpp::ImageFormat format);

int main(int argc, const char* argv[])
{
    std::cout << "ConsoleApp started..." << std::endl;
    try {
        int deviceId = 0;
        std::filesystem::path exeDirPath = std::filesystem::path(argv[0]).parent_path();
        std::filesystem::path hybridproDll = exeDirPath / "HybridPro.dll";
        std::filesystem::path hybridproCacheDir = exeDirPath / "hybridpro_cache";
        std::filesystem::path assetsDir = exeDirPath;
        std::filesystem::path postprocessingGlsl = exeDirPath / "post_processing.comp";

        rprpp::ImageFormat format = rprpp::ImageFormat::eR8G8B8A8Unorm;
        rprpp::PostProcessing postProcessing(true, deviceId, postprocessingGlsl);
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
                postProcessing.updateAovColor(renderer.getAov(RPR_AOV_COLOR));
                postProcessing.updateAovOpacity(renderer.getAov(RPR_AOV_OPACITY));
                postProcessing.updateAovShadowCatcher(renderer.getAov(RPR_AOV_SHADOW_CATCHER));
                postProcessing.updateAovReflectionCatcher(renderer.getAov(RPR_AOV_REFLECTION_CATCHER));
                postProcessing.updateAovMattePass(renderer.getAov(RPR_AOV_MATTE_PASS));
                postProcessing.updateAovBackground(renderer.getAov(RPR_AOV_BACKGROUND));
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

void savePngImage(const char* filename, uint8_t* img, uint32_t width, uint32_t height, rprpp::ImageFormat format)
{
    size_t numComponents = 4;
    uint8_t* dst = img;
    std::vector<uint8_t> vDst;
    if (format != rprpp::ImageFormat::eR8G8B8A8Unorm) {
        vDst.resize(width * height * numComponents);
        dst = vDst.data();
        switch (format) {
        case rprpp::ImageFormat::eR32G32B32A32Sfloat: {
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