#pragma once

#include <OpenImageDenoise/oidn.hpp>

namespace rprpp
{
    oidn::DeviceRef createOidnDevice(uint8_t luid[OIDN_LUID_SIZE], uint8_t uuid[OIDN_UUID_SIZE]);
}