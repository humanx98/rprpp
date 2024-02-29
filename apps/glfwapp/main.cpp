#include "NoAovsInteropApp.h"
#include "WithAovsInteropApp.h"
#include <iostream>

#define WIDTH 2000
#define HEIGHT 1000
#define RENDERED_ITERATIONS 16
#define INTEROP true
#define DEVICE_ID 0
// please note that when we use frames in flight > 1
// hybridpro produces Validation Error with VK_OBJECT_TYPE_QUERY_POOL message looks like "query not reset. After query pool creation"
#define FRAMES_IN_FLIGHT 4

DeviceInfo getDeviceInfoOf(int index)
{
    size_t size;
    RPRPP_CHECK(rprppGetDeviceInfo(index, RPRPP_DEVICE_INFO_NAME, nullptr, 0, &size));
    std::vector<char> deviceName;
    deviceName.resize(size);
    RPRPP_CHECK(rprppGetDeviceInfo(index, RPRPP_DEVICE_INFO_NAME, deviceName.data(), size, nullptr));

    RPRPP_CHECK(rprppGetDeviceInfo(index, RPRPP_DEVICE_INFO_LUID, nullptr, 0, &size));
    std::vector<uint8_t> deviceLUID;
    deviceLUID.resize(size);
    RPRPP_CHECK(rprppGetDeviceInfo(index, RPRPP_DEVICE_INFO_LUID, deviceLUID.data(), size, nullptr));

    uint32_t supportHardwareRT;
    RPRPP_CHECK(rprppGetDeviceInfo(index, RPRPP_DEVICE_INFO_SUPPORT_HARDWARE_RAY_TRACING, &supportHardwareRT, sizeof(supportHardwareRT), nullptr));
    return {
        .index = index,
        .name = std::string(deviceName.begin(), deviceName.end()),
        .LUID = deviceLUID,
        .supportHardwareRT = supportHardwareRT == RPRPP_TRUE ? true : false,
    };
}

int main(int argc, const char* argv[])
{
    std::cout << "GlfwApp started..." << std::endl;
    std::filesystem::path exeDirPath = std::filesystem::path(argv[0]).parent_path();
    Paths paths = {
        .hybridproDll = exeDirPath / "HybridPro.dll",
        .hybridproCacheDir = exeDirPath / "hybridpro_cache",
        .assetsDir = exeDirPath,
    };

    std::vector<DeviceInfo> deviceInfos;
    uint32_t deviceCount;
    RPRPP_CHECK(rprppGetDeviceCount(&deviceCount));
    for (int i = 0; i < deviceCount; i++) {
        deviceInfos.push_back(getDeviceInfoOf(i));
        std::cout << "Device id = " << i << ", name = " << deviceInfos[i].name;
        if (deviceInfos[i].supportHardwareRT) {
            std::cout << ", support Hardware Ray Tracing.";
        }
        std::cout << std::endl;
    }

    if (DEVICE_ID >= deviceCount) {
        throw std::runtime_error("There is no device with index = " + std::to_string(DEVICE_ID));
    }

#if INTEROP
    WithAovsInteropApp app(WIDTH, HEIGHT, RENDERED_ITERATIONS, FRAMES_IN_FLIGHT, paths, deviceInfos.at(DEVICE_ID));
#else
    NoAovsInteropApp app(WIDTH, HEIGHT, RENDERED_ITERATIONS, paths, deviceInfos.at(DEVICE_ID));
#endif
    try {
        app.run();
    } catch (const std::runtime_error& e) {
        printf("%s\n", e.what());
        return EXIT_FAILURE;
    }
    std::cout << "GlfwApp finished..." << std::endl;
    return 0;
}