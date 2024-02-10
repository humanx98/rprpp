set(HEADERS
	rprpp.h)

set(SOURCES
	rprpp.cpp)

add_library(capi STATIC ${HEADERS} ${SOURCES})

target_include_directories(capi 
	PRIVATE ${CMAKE_SOURCE_DIR}/src
	PRIVATE ${Vulkan_INCLUDE_DIR})

target_link_libraries(capi 
	PRIVATE rprpp)
