set(HEADERS
    common.hpp
    rpr_helper.hpp
    vk_helper.hpp
    DescriptorBuilder.hpp
    ShaderManager.hpp
    PostProcessing.hpp
)

set(SOURCES 
    vk_helper.cpp
    PostProcessing.cpp
)
add_library(rprpp STATIC ${SOURCES} ${HEADERS})

target_include_directories(rprpp 
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/.
    PUBLIC ${Vulkan_INCLUDE_DIR}
    PUBLIC "${RPRSDK_DIR}/inc"
)

target_link_libraries(rprpp 
    PUBLIC ${Vulkan_LIBRARY}
    PRIVATE ${Vulkan_SHADERC_LIB}
    PUBLIC ${RPRSDK_DIR}/libWin64/RadeonProRender64.lib
)