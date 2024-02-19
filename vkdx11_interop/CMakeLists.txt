set(HEADERS
	vkdx11_interop.h
)

set(SOURCES
	vkdx11_interop.cpp
)

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

add_library(vkdx11_interop SHARED ${HEADERS} ${SOURCES} 
        ${CMAKE_CURRENT_BINARY_DIR}/version_info.h
	${CMAKE_CURRENT_BINARY_DIR}/version.rc
)
target_compile_definitions(vkdx11_interop PRIVATE 
	${CMAKE_CURRENT_BINARY_DIR}/version_info.h
	${CMAKE_CURRENT_BINARY_DIR}/version.rc
)
if(RPRPP_EXPORT_API)
	target_compile_definitions(vkdx11_interop PRIVATE RPRPP_EXPORT_API)
endif()

_copy_file_to_target(vkdx11_interop ${Vulkan_SHADERC_DLL})

target_include_directories(vkdx11_interop 
	PRIVATE ${Vulkan_INCLUDE_DIR}
)

target_link_libraries(vkdx11_interop 
	PRIVATE rprpp
)

set_target_properties(vkdx11_interop PROPERTIES PUBLIC_HEADER "vkdx11_interop.h")

install(TARGETS vkdx11_interop RUNTIME PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/vkdx11_interop)
