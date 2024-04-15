#include "oidn_helper.h"

#include <boost/log/trivial.hpp>

namespace oidn::helper {

namespace {

    void printAllDenoiserDevices(int maxDevicesNumber)
    {
        for (int deviceId = 0; deviceId < maxDevicesNumber; ++deviceId) {
            BOOST_LOG_TRIVIAL(info) << "Denoiser Device id = " << deviceId << ", name = " << oidnGetPhysicalDeviceString(deviceId, "name");
        }
    }

}

oidn::DeviceRef createDevice(uint8_t luid[OIDN_LUID_SIZE], uint8_t uuid[OIDN_UUID_SIZE])
{
    BOOST_LOG_TRIVIAL(trace) << "oidn::helper::createDevice";

    int numPhysicalDevices = oidn::getNumPhysicalDevices();
    printAllDenoiserDevices(numPhysicalDevices);

    int deviceId = -1;
    int cpuId = -1;
    for (int i = 0; i < numPhysicalDevices; i++) {
        oidn::PhysicalDeviceRef physicalDevice(i);
        if (physicalDevice.get<bool>("luidSupported")) {
            oidn::LUID oidnLUID = physicalDevice.get<oidn::LUID>("luid");
            if (std::equal(std::begin(oidnLUID.bytes), std::end(oidnLUID.bytes), luid)) {
                deviceId = i;
                break;
            }
        }

        if (physicalDevice.get<bool>("uuidSupported")) {
            oidn::UUID oidnUUID = physicalDevice.get<oidn::UUID>("uuid");
            if (std::equal(std::begin(oidnUUID.bytes), std::end(oidnUUID.bytes), uuid)) {
                deviceId = i;
                break;
            }
        }

        if (physicalDevice.get<oidn::DeviceType>("type") == oidn::DeviceType::CPU) {
            cpuId = i;
        }
    }

    // cpu fallback
    if (deviceId < 0) {
        deviceId = cpuId;
    }

    BOOST_LOG_TRIVIAL(info) << "Initialize Selected Denoiser Device id = " << deviceId << ", name = " << oidnGetPhysicalDeviceString(deviceId, "name");
    oidn::DeviceRef device = oidn::newDevice(deviceId);
    device.commit();

    const char* errorMessage;
    if (device.getError(errorMessage) != oidn::Error::None) {
        BOOST_LOG_TRIVIAL(error) << errorMessage;
        throw std::runtime_error(errorMessage);
    }

    return device;
}

}