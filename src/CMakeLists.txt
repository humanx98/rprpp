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
add_library(rprpp STATIC ${SOURCES} ${HEADERS})

target_include_directories(rprpp 
    PRIVATE ${CMAKE_CURRENT_BINARY_DIR}
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/capi
    PRIVATE ${Vulkan_INCLUDE_DIR}
)

target_link_libraries(rprpp 
    PRIVATE ${Vulkan_LIBRARY}
    PRIVATE ${Vulkan_SHADERC_LIB}
)