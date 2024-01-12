macro(_copy_files_to_target target thefiles)
    foreach (FFF ${thefiles} )
        add_custom_command(
            TARGET ${target} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${FFF}
            $<TARGET_FILE_DIR:${target}>
            VERBATIM
        )
    endforeach()
endmacro()

include(FetchContent)

FetchContent_Declare(glfw GIT_REPOSITORY https://github.com/glfw/glfw)
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

FetchContent_Declare(lodepng GIT_REPOSITORY https://github.com/lvandeve/lodepng.git)
FetchContent_MakeAvailable(lodepng)
add_library(lodepng STATIC ${lodepng_SOURCE_DIR}/lodepng.cpp ${lodepng_SOURCE_DIR}/lodepng.h)
target_include_directories(lodepng PUBLIC ${lodepng_SOURCE_DIR})

set(HEADERS)
set(SOURCES)
add_executable (vkdx11_interop ${SOURCES} ${HEADERS} main.cpp)

_copy_files_to_target(vkdx11_interop "${Vulkan_SHADERC_DLL}")

target_include_directories(vkdx11_interop 
    PRIVATE ${Vulkan_INCLUDE_DIR}
)

target_link_libraries(vkdx11_interop 
    glfw
    lodepng
    ${Vulkan_LIBRARY}
    ${Vulkan_SHADERC_LIB}
)