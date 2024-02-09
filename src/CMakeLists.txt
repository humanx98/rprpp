find_package(Vulkan REQUIRED)
get_filename_component(Vulkan_LIBRARY_DIR ${Vulkan_LIBRARY} DIRECTORY)
find_file(Vulkan_SHADERC_LIB NAMES shaderc_shared.lib HINTS ${Vulkan_LIBRARY_DIR})
find_file(Vulkan_SHADERC_DLL NAMES shaderc_shared.dll HINTS ${Vulkan_LIBRARY_DIR}/../Bin)
if(NOT Vulkan_SHADERC_DLL)
    message(FATAL_ERROR "Windows platform requires VulkanSDK with shaderc_shared.lib/dll (since SDK 1.2.135.0)")
endif()

file(READ shaders/post_processing.comp RPRPP_POSTPROCESSING_SHADER_FILE_CONTENT)
string(REPLACE "\n" "\\n" RPRPP_POSTPROCESSING_SHADER "${RPRPP_POSTPROCESSING_SHADER_FILE_CONTENT}")

configure_file(rprppconfig.h.in rprppconfig.h)

set(HEADERS
    capi/rprpp.h
    common.h
    vk_helper.h
    vk.h
    DescriptorBuilder.h
    Error.h
    PostProcessing.h
    ShaderManager.h
)

set(SOURCES
    capi/rprpp.cpp
    vk_helper.cpp
    PostProcessing.cpp
    ShaderManager.cpp
)

add_library(rprpp SHARED ${SOURCES} ${HEADERS})
target_compile_definitions(rprpp PRIVATE RPRPP_EXPORT_API)
_copy_file_to_target(rprpp ${Vulkan_SHADERC_DLL})

target_include_directories(rprpp 
    PRIVATE ${CMAKE_CURRENT_BINARY_DIR}
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/capi
    PRIVATE ${Vulkan_INCLUDE_DIR}
)

target_link_libraries(rprpp 
    PRIVATE ${Vulkan_LIBRARY}
    PRIVATE ${Vulkan_SHADERC_LIB}
)