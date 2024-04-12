file(READ shaders/bloom_horizontal.comp RPRPP_bloom_horizontal_SHADER_FILE_CONTENT)
file(READ shaders/bloom_vertical.comp RPRPP_bloom_vertical_SHADER_FILE_CONTENT)
file(READ shaders/compose_color_shadow_reflection.comp RPRPP_compose_color_shadow_reflection_SHADER_FILE_CONTENT)
file(READ shaders/compose_opacity_shadow.comp RPRPP_compose_opacity_shadow_SHADER_FILE_CONTENT)
file(READ shaders/tonemap.comp RPRPP_tonemap_SHADER_FILE_CONTENT)

string(REPLACE "\n" "\\n" RPRPP_bloom_horizontal_SHADER "${RPRPP_bloom_horizontal_SHADER_FILE_CONTENT}")
string(REPLACE "\n" "\\n" RPRPP_bloom_vertical_SHADER "${RPRPP_bloom_vertical_SHADER_FILE_CONTENT}")
string(REPLACE "\n" "\\n" RPRPP_compose_color_shadow_reflection_SHADER "${RPRPP_compose_color_shadow_reflection_SHADER_FILE_CONTENT}")
string(REPLACE "\n" "\\n" RPRPP_compose_opacity_shadow_SHADER "${RPRPP_compose_opacity_shadow_SHADER_FILE_CONTENT}")
string(REPLACE "\n" "\\n" RPRPP_tonemap_SHADER "${RPRPP_tonemap_SHADER_FILE_CONTENT}")

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

set(HEADERS
    filters/BloomFilter.h
    filters/ComposeColorShadowReflectionFilter.h
    filters/ComposeOpacityShadowFilter.h
    filters/DenoiserFilter.h
    filters/DenoiserGpuFilter.h
    filters/DenoiserCpuFilter.h
    filters/Filter.h
    filters/ToneMapFilter.h
    vk/CommandBuffer.h
    vk/DescriptorBuilder.h
    vk/DeviceContext.h
    vk/ShaderManager.h
    vk/vk.h
    VkSampledImage.h
    DxImage.h
    ImageSimple.h
    Context.h
    ContextObject.h
    ContextObjectContainer.h
    ContextObjectHash.h
    rprpp.h
    Error.h
    Buffer.h
    Image.h
    ImageFormat.h
    ImageDescription.h
    ImageData.h
    UniformObjectBuffer.h
    ContextObjectContainerHasher.h
    ContextObjectContainerEqual.h

)

set(SOURCES
    filters/BloomFilter.cpp
    filters/ComposeColorShadowReflectionFilter.cpp
    filters/ComposeOpacityShadowFilter.cpp
    filters/DenoiserFilter.cpp
    filters/DenoiserGpuFilter.cpp
    filters/DenoiserCpuFilter.cpp
    filters/Filter.cpp
    filters/ToneMapFilter.cpp
    vk/CommandBuffer.cpp
    vk/DescriptorBuilder.cpp
    vk/DeviceContext.cpp
    vk/ShaderManager.cpp
    ImageData.cpp
    DxImage.cpp
    ImageSimple.cpp
    VkSampledImage.cpp
    Context.cpp
    ContextObject.cpp
    ContextObjectContainer.cpp
    rprpp.cpp
    Error.cpp
    Buffer.cpp
    Image.cpp
    ImageDescription.cpp
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
    PRIVATE ${Boost_INCLUDE_DIRS}
)

target_link_libraries(rprpp
    PRIVATE Vulkan::Vulkan
    PRIVATE Vulkan::shaderc_combined
    PRIVATE OpenImageDenoise
    PRIVATE Boost::log
)
set_target_properties(rprpp PROPERTIES PUBLIC_HEADER "rprpp.h")

install(TARGETS rprpp
        RUNTIME PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/rprpp)

if (INSTALL_DEPENDENCIES)
    install(FILES ${CMAKE_SOURCE_DIR}/ThirdParty/oidn/bin/OpenImageDenoise.dll DESTINATION ${CMAKE_INSTALL_BINDIR})
    install(FILES ${CMAKE_SOURCE_DIR}/ThirdParty/oidn/bin/OpenImageDenoise_core.dll DESTINATION ${CMAKE_INSTALL_BINDIR})
    install(FILES ${CMAKE_SOURCE_DIR}/ThirdParty/oidn/bin/OpenImageDenoise_device_cpu.dll DESTINATION ${CMAKE_INSTALL_BINDIR})
    install(FILES ${CMAKE_SOURCE_DIR}/ThirdParty/oidn/bin/OpenImageDenoise_device_cuda.dll DESTINATION ${CMAKE_INSTALL_BINDIR})
    install(FILES ${CMAKE_SOURCE_DIR}/ThirdParty/oidn/bin/OpenImageDenoise_device_hip.dll DESTINATION ${CMAKE_INSTALL_BINDIR})
    install(FILES ${CMAKE_SOURCE_DIR}/ThirdParty/oneTBB/bin/tbb12.dll DESTINATION ${CMAKE_INSTALL_BINDIR})
    install(FILES ${CMAKE_SOURCE_DIR}/ThirdParty/oneTBB/bin/tbbmalloc.dll DESTINATION ${CMAKE_INSTALL_BINDIR})
endif()