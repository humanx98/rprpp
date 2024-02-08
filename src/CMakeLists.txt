set(HEADERS
    capi/rprpp.h
    common.hpp
    vk_helper.hpp
    vk.hpp
    DescriptorBuilder.hpp
    Error.hpp
    ShaderManager.hpp
    PostProcessing.hpp
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