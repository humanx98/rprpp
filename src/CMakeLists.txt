macro(_copy_file_to_target target thefile)
    add_custom_command(
        TARGET ${target} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${thefile}
        $<TARGET_FILE_DIR:${target}>
        VERBATIM
    )
endmacro()

include(FetchContent)
set(FETCHCONTENT_QUIET FALSE)

FetchContent_Declare(glfw 
    GIT_REPOSITORY https://github.com/glfw/glfw
    GIT_TAG 3.3.9
    GIT_SHALLOW TRUE
    GIT_PROGRESS TRUE
)
set(GLFW_BUILD_EXAMPLES OFF CACHE INTERNAL "Glfw build examples")
set(GLFW_BUILD_TESTS OFF CACHE INTERNAL "Glfw build tests")
set(GLFW_BUILD_DOCS OFF CACHE INTERNAL "Glfw build docs")
set(GLFW_INSTALL OFF CACHE INTERNAL "Glfw install")
FetchContent_MakeAvailable(glfw)

find_package(Vulkan REQUIRED)
get_filename_component(Vulkan_LIBRARY_DIR ${Vulkan_LIBRARY} DIRECTORY)
find_file(Vulkan_SHADERC_LIB NAMES shaderc_shared.lib HINTS ${Vulkan_LIBRARY_DIR})
find_file(Vulkan_SHADERC_DLL NAMES shaderc_shared.dll HINTS ${Vulkan_LIBRARY_DIR}/../Bin)
if(NOT Vulkan_SHADERC_DLL)
    message(FATAL_ERROR "Windows platform requires VulkanSDK with shaderc_shared.lib/dll (since SDK 1.2.135.0)")
endif()

FetchContent_Declare(rprsdk 
    GIT_REPOSITORY https://github.com/GPUOpen-LibrariesAndSDKs/RadeonProRenderSDK.git
    GIT_TAG v3.1.4
    GIT_SUBMODULES ""
    GIT_SHALLOW TRUE
    GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(rprsdk)

set(HEADERS
    Dx11App.hpp
    rpr_helper.hpp
    vk_helper.hpp
    HybridProRenderer.hpp
    PostProcessing.hpp
)

set(SOURCES 
    HybridProRenderer.cpp
    PostProcessing.cpp
)
add_executable (vkdx11_interop ${SOURCES} ${HEADERS} main.cpp)

_copy_file_to_target(vkdx11_interop "${Vulkan_SHADERC_DLL}")
_copy_file_to_target(vkdx11_interop "${rprsdk_SOURCE_DIR}/RadeonProRender/binWin64/HybridPro.dll")
_copy_file_to_target(vkdx11_interop "${rprsdk_SOURCE_DIR}/RadeonProRender/binWin64/RadeonProRender64.dll")
_copy_file_to_target(vkdx11_interop ${CMAKE_CURRENT_SOURCE_DIR}/assets/teapot.obj)
_copy_file_to_target(vkdx11_interop ${CMAKE_CURRENT_SOURCE_DIR}/assets/envLightImage.exr)
_copy_file_to_target(vkdx11_interop ${CMAKE_CURRENT_SOURCE_DIR}/assets/post_processing.comp)

target_include_directories(vkdx11_interop 
    PRIVATE ${Vulkan_INCLUDE_DIR}
    PRIVATE ${rprsdk_SOURCE_DIR}/RadeonProRender/inc
)

target_link_libraries(vkdx11_interop 
    glfw
    ${Vulkan_LIBRARY}
    ${Vulkan_SHADERC_LIB}
    ${rprsdk_SOURCE_DIR}/RadeonProRender/libWin64/RadeonProRender64.lib
)