FetchContent_Declare(glfw 
    GIT_REPOSITORY https://github.com/glfw/glfw
    GIT_TAG 3.3.9
    GIT_SHALLOW TRUE
    GIT_PROGRESS TRUE
)
set(GLFW_BUILD_EXAMPLES OFF CACHE INTERNAL "Glfw build examples")
set(GLFW_BUILD_TESTS OFF CACHE INTERNAL "Glfw build tests")
set(GLFW_BUILD_DOCS OFF CACHE INTERNAL "Glfw build docs")
set(GLFW_INSTALL OFF CACHE INTERNAL "Glfw install")
FetchContent_MakeAvailable(glfw)

set(HEADERS
    ../common/rpr_helper.h
    ../common/HybridProRenderer.h
    ../common/RprPostProcessing.h
    Dx11App.h
)

set(SOURCES
    ../common/HybridProRenderer.cpp
    ../common/rpr_helper.cpp
    Dx11App.cpp
)
add_executable(glfwapp ${SOURCES} ${HEADERS} main.cpp)

_copy_file_to_target(glfwapp ${Vulkan_SHADERC_DLL})
_copy_file_to_target(glfwapp ${RPRSDK_DIR}/binWin64/HybridPro.dll)
_copy_file_to_target(glfwapp ${RPRSDK_DIR}/binWin64/RadeonProRender64.dll)
_copy_file_to_target(glfwapp ${CMAKE_CURRENT_SOURCE_DIR}/../assets/teapot.obj)
_copy_file_to_target(glfwapp ${CMAKE_CURRENT_SOURCE_DIR}/../assets/envLightImage.exr)
_copy_file_to_target(glfwapp ${CMAKE_CURRENT_SOURCE_DIR}/../assets/post_processing.comp)

target_include_directories(glfwapp
    PRIVATE ${RPRSDK_DIR}/inc
)

target_link_libraries(glfwapp 
    glfw 
    rprpp
    ${RPRSDK_DIR}/libWin64/RadeonProRender64.lib
)