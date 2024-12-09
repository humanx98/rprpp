cmake_minimum_required(VERSION 3.26.1)
project(rprpp LANGUAGES CXX VERSION 2.1.0)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_COMPILE_WARNING_AS_ERROR ON)

option(RPRPP_EXPORT_API "Export dll symbols" ON)
option(BUILD_APPS "Build testing applications" ON)
option(FORCE_COLORED_OUTPUT "Always produce ANSI-colored output (GNU/Clang only)." FALSE)
option(INSTALL_DEPENDENCIES "Install ThirdParty dependencies (dlls)" TRUE)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")
set(OpenImageDenoise_DIR "${CMAKE_SOURCE_DIR}/ThirdParty/oidn/lib/cmake/OpenImageDenoise-2.3.1")

include("${CMAKE_SOURCE_DIR}/cmake/CompileOptions.cmake")
include("${CMAKE_SOURCE_DIR}/cmake/WindowsFileVersion.cmake")
include("${CMAKE_SOURCE_DIR}/cmake/CopyFileToTarget.cmake")
include("${CMAKE_SOURCE_DIR}/cmake/VulkanTools.cmake")

set(RPR_SDK_ROOT "${CMAKE_SOURCE_DIR}/ThirdParty/RadeonProRenderSDK")

find_package(Boost REQUIRED COMPONENTS system log program_options)

find_package(Vulkan COMPONENTS shaderc_combined REQUIRED)
find_package(Stb REQUIRED)
find_package(RadeonProRenderSDK 3.1.0 REQUIRED COMPONENTS hybridpro)
find_package(OpenImageDenoise REQUIRED)

check_vulkan(${Vulkan_LIBRARY})

include(GNUInstallDirs)
include(FetchContent)
set(FETCHCONTENT_QUIET FALSE)

# workaround for dll's copies
if(WIN32)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
endif()

add_subdirectory(rprpp)
if (BUILD_APPS)
    add_subdirectory(stbimpl)
    add_subdirectory(apps)
endif()
