cmake_minimum_required(VERSION 3.26.1)
project(vkdx11_interop LANGUAGES CXX VERSION 1.0.1)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_COMPILE_WARNING_AS_ERROR ON)

option(RPRPP_EXPORT_API "Export dll symbols" 1)

# Get the latest abbreviated commit hash of the working branch
execute_process(
  COMMAND git log -1 --format=%h
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  OUTPUT_VARIABLE GIT_COMMIT_HASH
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

set(LIBS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/libs)
set(RPRSDK_DIR ${LIBS_DIR}/RadeonProRender)

find_package(Vulkan REQUIRED)
get_filename_component(Vulkan_LIBRARY_DIR ${Vulkan_LIBRARY} DIRECTORY)
find_file(Vulkan_SHADERC_LIB NAMES shaderc_shared.lib HINTS ${Vulkan_LIBRARY_DIR})
find_file(Vulkan_SHADERC_DLL NAMES shaderc_shared.dll HINTS ${Vulkan_LIBRARY_DIR}/../Bin)
if(NOT Vulkan_SHADERC_DLL)
    message(FATAL_ERROR "Windows platform requires VulkanSDK with shaderc_shared.lib/dll (since SDK 1.2.135.0)")
endif()

include(FetchContent)
set(FETCHCONTENT_QUIET FALSE)

macro(_copy_file_to_target target thefile)
    add_custom_command(
        TARGET ${target} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${thefile}
        $<TARGET_FILE_DIR:${target}>
        VERBATIM
    )
endmacro()

add_subdirectory(vkdx11_interop)
add_subdirectory(rprpp)
add_subdirectory(apps)
