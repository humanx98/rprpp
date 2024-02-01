#include "Dx11App.hpp"
#include "HybridProRenderer.hpp"
#include <iostream>

#define WIDTH 1200
#define HEIGHT 700

int main(int argc, const char* argv[])
{
    GpuIndices gpus = { .dx11 = 0, .vk = 0 };
    std::cout << "VkDx11Interop App started..." << std::endl;
    std::filesystem::path exeDirPath = std::filesystem::path(argv[0]).parent_path();
    Paths paths = {
        .hybridproDll = exeDirPath / "HybridPro.dll",
        .hybridproCacheDir = exeDirPath / "hybridpro_cache",
        .assetsDir = exeDirPath,
        .postprocessingGlsl = exeDirPath / "post_processing.comp"
    };

    Dx11App app(WIDTH, HEIGHT, paths, gpus);
    try {
        app.run();
    } catch (const std::runtime_error& e) {
        printf("%s\n", e.what());
        return EXIT_FAILURE;
    }
    std::cout << "VkDx11Interop App finished..." << std::endl;
    return 0;
}