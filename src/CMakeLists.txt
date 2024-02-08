set(HEADERS
    capi/rprpp.h
    common.h
    vk_helper.h
    vk.h
    DescriptorBuilder.h
    Error.h
    ShaderManager.h
    PostProcessing.h
)

set(SOURCES
    capi/rprpp.cpp
    vk_helper.cpp
    PostProcessing.cpp
)
add_library(rprpp STATIC ${SOURCES} ${HEADERS})

target_include_directories(rprpp 
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/capi/.
    PRIVATE ${Vulkan_INCLUDE_DIR}
)

target_link_libraries(rprpp 
    PRIVATE ${Vulkan_LIBRARY}
    PRIVATE ${Vulkan_SHADERC_LIB}
)