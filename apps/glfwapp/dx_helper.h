#pragma once

#include <filesystem>
#include <winerror.h>

void process_dx_error(HRESULT res, const char* file, int line);

#define DX_CHECK(f) if (!SUCCEEDED((HRESULT)f)) process_dx_error(f, __FILE__, __LINE__);

struct DeviceInfo {
    int index;
    std::string name;
    std::vector<uint8_t> LUID;
    bool supportHardwareRT;
    bool supportGpuDenoiser;
};

struct Paths {
    std::filesystem::path hybridproDll;
    std::filesystem::path hybridproCacheDir;
    std::filesystem::path assetsDir;
};