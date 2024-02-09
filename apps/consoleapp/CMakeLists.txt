FetchContent_Declare(stb 
    GIT_REPOSITORY https://github.com/nothings/stb.git
    GIT_SHALLOW TRUE
    GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(stb)

set(HEADERS
    ../common/rpr_helper.h
    ../common/HybridProRenderer.h
    ../common/RprPostProcessing.h
)

set(SOURCES
    ../common/HybridProRenderer.cpp
    ../common/rpr_helper.cpp
)
add_executable(consoleapp ${SOURCES} ${HEADERS} main.cpp)

_copy_file_to_target(consoleapp ${Vulkan_SHADERC_DLL})
_copy_file_to_target(consoleapp ${RPRSDK_DIR}/binWin64/HybridPro.dll)
_copy_file_to_target(consoleapp ${RPRSDK_DIR}/binWin64/RadeonProRender64.dll)
_copy_file_to_target(consoleapp ${CMAKE_CURRENT_SOURCE_DIR}/../assets/teapot.obj)
_copy_file_to_target(consoleapp ${CMAKE_CURRENT_SOURCE_DIR}/../assets/envLightImage.exr)

target_include_directories(consoleapp 
    PRIVATE ${RPRSDK_DIR}/inc
    PRIVATE ${stb_SOURCE_DIR}
)

target_link_libraries(consoleapp 
    rprpp
    ${RPRSDK_DIR}/libWin64/RadeonProRender64.lib
)