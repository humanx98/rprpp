cmake_minimum_required(VERSION 3.26.1)
project(rprpp LANGUAGES CXX VERSION 1.0.1)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_COMPILE_WARNING_AS_ERROR ON)

option(RPRPP_EXPORT_API "Export dll symbols" ON)

# Get the latest abbreviated commit hash of the working branch
execute_process(
  COMMAND git log -1 --format=%h
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  OUTPUT_VARIABLE GIT_COMMIT_HASH
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

set(LIBS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/libs)
set(RPRSDK_DIR ${LIBS_DIR}/RadeonProRender)

find_package(Vulkan COMPONENTS shaderc_combined REQUIRED)
get_filename_component(Vulkan_LIBRARY_DIR ${Vulkan_LIBRARY} DIRECTORY)
find_file(Vulkan_SHADERC_COMBINEDD_LIB NAMES shaderc_combinedd.lib HINTS ${Vulkan_LIBRARY_DIR})
if(NOT Vulkan_SHADERC_COMBINEDD_LIB)
  message(FATAL_ERROR "Install 'Shader Toolchain Debug Symbols - 64 bit' with VulkanSDK in order to have shaderc_combinedd.lib")
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

add_subdirectory(rprpp)
add_subdirectory(apps)
