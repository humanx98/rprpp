add_subdirectory(capi)

file(READ shaders/post_processing.comp RPRPP_POSTPROCESSING_SHADER_FILE_CONTENT)
string(REPLACE "\n" "\\n" RPRPP_POSTPROCESSING_SHADER "${RPRPP_POSTPROCESSING_SHADER_FILE_CONTENT}")

configure_file(rprppconfig.h.in rprppconfig.h)

set(HEADERS
    common.h
    vk_helper.h
    vk.h
    DescriptorBuilder.h
    Error.h
    PostProcessing.h
    ShaderManager.h
)

set(SOURCES
    vk_helper.cpp
    PostProcessing.cpp
    ShaderManager.cpp
)

add_library(rprpp STATIC ${SOURCES} ${HEADERS})

target_include_directories(rprpp 
    PUBLIC ${CMAKE_CURRENT_BINARY_DIR}
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/capi
    PUBLIC ${Vulkan_INCLUDE_DIR}
)

target_link_libraries(rprpp 
    PUBLIC ${Vulkan_LIBRARY}
    PUBLIC ${Vulkan_SHADERC_LIB}
    PUBLIC capi
)
