add_executable(consoleapp ${SOURCES} ${HEADERS} main.cpp)

_copy_file_to_target(consoleapp ${Vulkan_SHADERC_DLL})
_copy_file_to_target(consoleapp ${RPRSDK_DIR}/binWin64/HybridPro.dll)
_copy_file_to_target(consoleapp ${RPRSDK_DIR}/binWin64/RadeonProRender64.dll)
_copy_file_to_target(consoleapp ${CMAKE_CURRENT_SOURCE_DIR}/../assets/teapot.obj)
_copy_file_to_target(consoleapp ${CMAKE_CURRENT_SOURCE_DIR}/../assets/envLightImage.exr)

target_link_libraries(consoleapp 
	appcommon
)
