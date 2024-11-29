#include "NoAovsInteropApp.h"
#include "WithAovsInteropApp.h"

#include <boost/program_options.hpp>
#include <boost/program_options/parsers.hpp>

#include <iostream>

const int FB_WIDTH = 2000;
const int FB_HEIGHT = 1000;
const int FB_RENDERED_ITERATIONS = 16;
const bool FB_INTEROP = true;
const size_t FB_DEVICE_ID = 0;
#ifdef NDEBUG
// please note that when we use frames in flight > 1
// hybridpro produces Validation Error with VK_OBJECT_TYPE_QUERY_POOL message looks like "query not reset. After query pool creation"
const unsigned int FB_FRAMES_IN_FLIGHT = 4;
#else
const unsigned int FB_FRAMES_IN_FLIGHT = 1;
#endif

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

    uint32_t supportGpuDenoiser;
    RPRPP_CHECK(rprppGetDeviceInfo(index, RPRPP_DEVICE_INFO_SUPPORT_GPU_DENOISER, &supportGpuDenoiser, sizeof(supportGpuDenoiser), nullptr));
    return {
        .index = index,
        .name = std::string(deviceName.begin(), deviceName.end()),
        .LUID = deviceLUID,
        .supportHardwareRT = supportHardwareRT == RPRPP_TRUE ? true : false,
        .supportGpuDenoiser = supportGpuDenoiser == RPRPP_TRUE ? true : false,
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

    bool useInterop;
    int width;
    int height;
    int render_iterations;
    size_t deviceId;
    unsigned int framesInFlight;

    boost::program_options::options_description genericOptions("generic");
    genericOptions.add_options()
        ("help,h", "produce help message")
        ("interop,b", boost::program_options::value<bool>(&useInterop)->default_value(FB_INTEROP), "use vulkan interop")
        ("width,w", boost::program_options::value<int>(&width)->default_value(FB_WIDTH), "framebuffer width")
        ("height,h", boost::program_options::value<int>(&height)->default_value(FB_HEIGHT), "framebuffer height")
        ("iterations,i", boost::program_options::value<int>(&render_iterations)->default_value(FB_RENDERED_ITERATIONS), "iterations")
        ("device,d", boost::program_options::value<size_t>(&deviceId)->default_value(FB_DEVICE_ID), "device id")
        ("frames,f", boost::program_options::value<unsigned int>(&framesInFlight)->default_value(FB_FRAMES_IN_FLIGHT), "frames in flight")
        ("verbosity,v", boost::program_options::value<std::string>(), "verbosity");

    boost::program_options::options_description cmdline_options;
    cmdline_options.add(genericOptions);

    boost::program_options::variables_map vm;
    boost::program_options::store(boost::program_options::parse_command_line(argc, argv, cmdline_options), vm);

    if (vm.contains("help")) {
        std::cout << cmdline_options << "\n";
        return 0;
    }
    boost::program_options::notify(vm);

    if (vm.contains("verbosity")) {
        RPRPP_CHECK(rprppSetLogVerbosity(vm["verbosity"].as<std::string>().c_str()));
    } else {
        RPRPP_CHECK(rprppSetLogVerbosity("trace"));
    }

    std::vector<DeviceInfo> deviceInfos;
    uint32_t deviceCount;
    RPRPP_CHECK(rprppGetDeviceCount(&deviceCount));
    for (int i = 0; i < deviceCount; i++) {
        deviceInfos.push_back(getDeviceInfoOf(i));
        std::cout << "Device id = " << i << ", name = " << deviceInfos[i].name;
        if (deviceInfos[i].supportHardwareRT) {
            std::cout << ", support Hardware Ray Tracing";
        }

        if (deviceInfos[i].supportGpuDenoiser) {
            std::cout << ", support Gpu Denoiser";
        }
        std::cout << std::endl;
    }

    if (deviceId >= deviceCount) {
        std::cerr << "There is no device with index = " << deviceId << "\n";
        return EXIT_FAILURE;
    }

    try {
        if(useInterop) {
            WithAovsInteropApp app(width, height, render_iterations, framesInFlight, paths, deviceInfos.at(deviceId));
            app.run();
        }
        else {
            NoAovsInteropApp app(width, height, render_iterations, paths, deviceInfos.at(deviceId));
            app.run();
        }
    } catch (const std::exception& e) {
        std::cerr << e.what() << "\n";
        return EXIT_FAILURE;
    }

    std::cout << "GlfwApp finished...\n";

    return 0;
}