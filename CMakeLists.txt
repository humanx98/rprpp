cmake_minimum_required(VERSION 3.26.1)
#   Allow GIT_SUBMODULES empty to mean no submodules
cmake_policy(SET CMP0097 NEW)
project(rprpp LANGUAGES CXX VERSION 0.1)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_COMPILE_WARNING_AS_ERROR ON)

include(FetchContent)
set(FETCHCONTENT_QUIET FALSE)

set(LIBS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/libs)

find_package(Vulkan REQUIRED)
get_filename_component(Vulkan_LIBRARY_DIR ${Vulkan_LIBRARY} DIRECTORY)
find_file(Vulkan_SHADERC_LIB NAMES shaderc_shared.lib HINTS ${Vulkan_LIBRARY_DIR})
find_file(Vulkan_SHADERC_DLL NAMES shaderc_shared.dll HINTS ${Vulkan_LIBRARY_DIR}/../Bin)
if(NOT Vulkan_SHADERC_DLL)
    message(FATAL_ERROR "Windows platform requires VulkanSDK with shaderc_shared.lib/dll (since SDK 1.2.135.0)")
endif()

# Fetching rprsdk takes about 5min
# 
# FetchContent_Declare(rprsdk 
#     GIT_REPOSITORY https://github.com/GPUOpen-LibrariesAndSDKs/RadeonProRenderSDK.git
#     GIT_TAG v3.1.4
#     GIT_SUBMODULES ""
#     GIT_SHALLOW TRUE
#     GIT_PROGRESS TRUE
# )
# FetchContent_MakeAvailable(rprsdk)
# set(RPRSDK_DIR ${rprsdk_SOURCE_DIR}/RadeonProRender) 
set(RPRSDK_DIR ${LIBS_DIR}/RadeonProRender)

add_subdirectory(capi)
add_subdirectory(src)
add_subdirectory(apps)
