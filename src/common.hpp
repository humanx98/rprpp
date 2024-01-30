#pragma once

#include <cstring>
#include <filesystem>

struct GpuIndices {
    int dx11 = 0;
    int vk = 0;
};

struct cmp_str {
    bool operator()(char const* a, char const* b) const
    {
        return std::strcmp(a, b) < 0;
    }
};

struct Paths {
    std::filesystem::path hybridproDll;
    std::filesystem::path hybridproCacheDir;
    std::filesystem::path assetsDir;
    std::filesystem::path postprocessingGlsl;
};
