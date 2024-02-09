#include "Dx11App.h"
#include <iostream>

#define WIDTH 1200
#define HEIGHT 700

int main(int argc, const char* argv[])
{
    GpuIndices gpus = { .dx11 = 0, .vk = 0 };
    std::cout << "GlfwApp started..." << std::endl;
    std::filesystem::path exeDirPath = std::filesystem::path(argv[0]).parent_path();
    Paths paths = {
        .hybridproDll = exeDirPath / "HybridPro.dll",
        .hybridproCacheDir = exeDirPath / "hybridpro_cache",
        .assetsDir = exeDirPath,
    };

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

    Dx11App app(WIDTH, HEIGHT, paths, gpus);
    try {
        app.run();
    } catch (const std::runtime_error& e) {
        printf("%s\n", e.what());
        return EXIT_FAILURE;
    }
    std::cout << "GlfwApp finished..." << std::endl;
    return 0;
}