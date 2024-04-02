#include "WithAovsInteropApp.h"
#include <iostream>

#include <OpenImageDenoise/oidn.h>

constexpr int WIDTH = 2000;
constexpr int HEIGHT = 1000;
constexpr int RENDERED_ITERATIONS = 16;
constexpr int DEVICE_ID = 0;
// please note that when we use frames in flight > 1
// hybridpro produces Validation Error with VK_OBJECT_TYPE_QUERY_POOL message looks like "query not reset. After query pool creation"
constexpr int FRAMES_IN_FLIGHT = 4;

DeviceInfo getDeviceInfoOf(int index)
{
    RprPpError status;
    size_t size;

    status = rprppGetDeviceInfo(index, RPRPP_DEVICE_INFO_NAME, nullptr, 0, &size);
    RPRPP_CHECK(status);

    std::vector<char> deviceName(size);

    status = rprppGetDeviceInfo(index, RPRPP_DEVICE_INFO_NAME, deviceName.data(), size, nullptr);
    RPRPP_CHECK(status);

    status = rprppGetDeviceInfo(index, RPRPP_DEVICE_INFO_LUID, nullptr, 0, &size);
    RPRPP_CHECK(status);

    std::vector<uint8_t> deviceLUID(size);

    status = rprppGetDeviceInfo(index, RPRPP_DEVICE_INFO_LUID, deviceLUID.data(), size, nullptr);
    RPRPP_CHECK(status);

    uint32_t supportHardwareRT;
    status = rprppGetDeviceInfo(index, RPRPP_DEVICE_INFO_SUPPORT_HARDWARE_RAY_TRACING, &supportHardwareRT, sizeof(supportHardwareRT), nullptr);
    RPRPP_CHECK(status);

    return {
        .index = index,
        .name = std::string(deviceName.begin(), deviceName.end()),
        .LUID = deviceLUID,
        .supportHardwareRT = supportHardwareRT == RPRPP_TRUE ? true : false,
    };
}

int main(int argc, const char* argv[]) try
{

    OIDNDevice oidnDevice = oidnNewDevice(OIDN_DEVICE_TYPE_HIP);

    RprPpError status;
    std::cout << "GlfwApp started..." << std::endl;
    std::filesystem::path exeDirPath = std::filesystem::path(argv[0]).parent_path();
    Paths paths = {
        .hybridproDll = exeDirPath / "HybridPro.dll",
        .hybridproCacheDir = exeDirPath / "hybridpro_cache",
        .assetsDir = exeDirPath,
    };

    std::vector<DeviceInfo> deviceInfos;
    uint32_t deviceCount;

    status = rprppGetDeviceCount(&deviceCount);
    RPRPP_CHECK(status);

    for (int i = 0; i < deviceCount; i++) {
        deviceInfos.push_back(getDeviceInfoOf(i));
        std::cout << "Device id = " << i << ", name = " << deviceInfos[i].name;
        if (deviceInfos[i].supportHardwareRT) {
            std::cout << ", support Hardware Ray Tracing.";
        }
        std::cout << "\n";
    }

    if (DEVICE_ID >= deviceCount) {
        throw std::runtime_error("There is no device with index = " + std::to_string(DEVICE_ID));
    }

    WithAovsInteropApp app(WIDTH, HEIGHT, RENDERED_ITERATIONS, FRAMES_IN_FLIGHT, paths, deviceInfos.at(DEVICE_ID));
    app.run();

    std::cout << "GlfwApp finished...\n";

    return EXIT_SUCCESS;
} catch (const std::exception& error) {
    std::cerr << error.what() << "\n";
    return EXIT_FAILURE;
}