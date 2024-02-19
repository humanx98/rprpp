set(HEADERS
	rprpp.h)

set(SOURCES
	rprpp.cpp)

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


add_library(vkdx11_interop SHARED ${HEADERS} ${SOURCES})
target_compile_definitions(vkdx11_interop PRIVATE 
	${CMAKE_CURRENT_BINARY_DIR}/version_info.h
	${CMAKE_CURRENT_BINARY_DIR}/version.rc)
if(RPRPP_EXPORT_API)
	target_compile_definitions(vkdx11_interop PRIVATE RPRPP_EXPORT_API}
endif()

_copy_file_to_target(vkdx11_interop ${Vulkan_SHADERC_DLL})

target_include_directories(vkdx11_interop 
	PRIVATE ${CMAKE_SOURCE_DIR}/src
	PRIVATE ${Vulkan_INCLUDE_DIR})

target_link_libraries(vkdx11_interop 
	PRIVATE rprpp)

set_target_properties(mylib PROPERTIES PUBLIC_HEADER "rprpp.h")

#install(TARGETS vkdx11_interop RUNTIME PUBLIC_HEADER rprpp.h)
