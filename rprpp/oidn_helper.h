#pragma once

#include <OpenImageDenoise/oidn.hpp>

namespace oidn::helper {

oidn::DeviceRef createDevice(uint8_t luid[OIDN_LUID_SIZE], uint8_t uuid[OIDN_UUID_SIZE]);

}