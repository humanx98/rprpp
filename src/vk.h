#pragma once

#include <windows.h>
#undef min
#undef max
#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan_raii.hpp>