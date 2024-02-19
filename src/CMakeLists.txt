file(READ shaders/post_processing.comp RPRPP_POSTPROCESSING_SHADER_FILE_CONTENT)
string(REPLACE "\n" "\\n" RPRPP_POSTPROCESSING_SHADER "${RPRPP_POSTPROCESSING_SHADER_FILE_CONTENT}")

configure_file(rprppconfig.h.in rprppconfig.h)

set(SHADERS
    shaders/post_processing.comp
)

set(HEADERS
    Error.h
    common.h
    vk_helper.h
    vk.h
    DescriptorBuilder.h
    PostProcessing.h
    ShaderManager.h
)

set(SOURCES
    Error.cpp
    vk_helper.cpp
    DescriptorBuilder.cpp
    PostProcessing.cpp
    ShaderManager.cpp
)

add_library(rprpp STATIC ${SOURCES} ${HEADERS} ${SHADERS})

target_include_directories(rprpp 
    PUBLIC ${CMAKE_SOURCE_DIR}
    PUBLIC ${CMAKE_CURRENT_BINARY_DIR}
    PUBLIC ${Vulkan_INCLUDE_DIR}
)

target_link_libraries(rprpp 
    PUBLIC ${Vulkan_LIBRARY}
    PUBLIC ${Vulkan_SHADERC_LIB}
)
