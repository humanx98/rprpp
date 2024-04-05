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
    dx_helper.h
    NoAovsInteropApp.h
    WithAovsInteropApp.h
)

set(SOURCES
    NoAovsInteropApp.cpp
    WithAovsInteropApp.cpp
)
add_executable(glfwapp ${SOURCES} ${HEADERS} main.cpp)

target_link_libraries(glfwapp
    PUBLIC glfw
    PUBLIC appcommon
)
# automatically copy all dll's deps.
#add_custom_command(TARGET glfwapp POST_BUILD
#  COMMAND ${CMAKE_COMMAND} -E copy -t $<TARGET_FILE_DIR:glfwapp> $<TARGET_RUNTIME_DLLS:glfwapp>
#  COMMAND_EXPAND_LISTS
#)
add_custom_target(glfwapp_dlls
  COMMAND ${CMAKE_COMMAND} -E copy -t $<TARGET_FILE_DIR:glfwapp> $<TARGET_RUNTIME_DLLS:glfwapp>
  COMMAND_EXPAND_LISTS
)

add_dependencies(glfwapp glfwapp_dlls)
add_dependencies(rprpp glfwapp_dlls)

# copy hip kernels
populate_rprsdk_kernels(glfwapp)

# copy assets
_copy_file_to_target(glfwapp ${CMAKE_SOURCE_DIR}/apps/assets/teapot.obj)
_copy_file_to_target(glfwapp ${CMAKE_SOURCE_DIR}/apps/assets/envLightImage.exr)

# bug in oidn cmake. Manually copy backend dll's
_copy_file_to_target(glfwapp ${CMAKE_SOURCE_DIR}/ThirdParty/oidn/bin/OpenImageDenoise_device_cpu.dll)
_copy_file_to_target(glfwapp ${CMAKE_SOURCE_DIR}/ThirdParty/oidn/bin/OpenImageDenoise_device_hip.dll)


