file(READ shaders/post_processing.comp RPRPP_POSTPROCESSING_SHADER_FILE_CONTENT)
string(REPLACE "\n" "\\n" RPRPP_POSTPROCESSING_SHADER "${RPRPP_POSTPROCESSING_SHADER_FILE_CONTENT}")

configure_file(rprpp_config.h.in rprpp_config.h)

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/version_info.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/version_info.h
    @ONLY
)
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/version.rc.in
    ${CMAKE_CURRENT_BINARY_DIR}/version.rc
    @ONLY
)

set(SHADERS
    shaders/post_processing.comp
)

set(HEADERS
    vk/DescriptorBuilder.h
    vk/DeviceContext.h
    vk/ShaderManager.h
    vk/vk.h
    Context.h
    rprpp.h
    Error.h
    Buffer.h
    Image.h
    ImageFormat.h
    PostProcessing.h
)

set(SOURCES
    vk/DescriptorBuilder.cpp
    vk/DeviceContext.cpp
    vk/ShaderManager.cpp
    Context.cpp
    rprpp.cpp
    Error.cpp
    Buffer.cpp
    Image.cpp
    PostProcessing.cpp
)

add_library(rprpp SHARED 
    ${HEADERS}
    ${SOURCES}
    ${CMAKE_CURRENT_BINARY_DIR}/version_info.h
    ${CMAKE_CURRENT_BINARY_DIR}/version.rc
)
if(RPRPP_EXPORT_API)
    target_compile_definitions(rprpp PRIVATE RPRPP_EXPORT_API)
endif()

target_include_directories(rprpp
    PUBLIC ${CMAKE_SOURCE_DIR}
    PRIVATE ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(rprpp
    PRIVATE Vulkan::Vulkan
    PRIVATE Vulkan::shaderc_combined
)
set_target_properties(rprpp PROPERTIES PUBLIC_HEADER "rprpp.h")

install(TARGETS rprpp RUNTIME PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/rprpp)
