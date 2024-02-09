set(HEADERS
	rprpp.h
)

set(SOURCES 
	rprpp.cpp
)
add_library(capi STATIC ${HEADERS} ${SOURCES})
target_include_directories(capi 
    PUBLIC ${CMAKE_SOURCE_DIR}/src
    PUBLIC ${Vulkan_INCLUDE_DIR}
)

target_link_libraries(capi
    PUBLIC ${Vulkan_LIBRARY}
    PUBLIC ${Vulkan_SHADERC_LIB}
)

