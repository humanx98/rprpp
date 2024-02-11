set(HEADERS
	rprpp.h)

set(SOURCES
	rprpp.cpp)

add_library(capi SHARED ${HEADERS} ${SOURCES})
target_compile_definitions(capi PRIVATE RPRPP_EXPORT_API=${RPRPP_EXPORT_API})
_copy_file_to_target(capi ${Vulkan_SHADERC_DLL})

target_include_directories(capi 
	PRIVATE ${CMAKE_SOURCE_DIR}/src
	PRIVATE ${Vulkan_INCLUDE_DIR})

target_link_libraries(capi 
	PRIVATE rprpp)
