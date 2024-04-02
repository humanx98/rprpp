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
    WithAovsInteropApp.h
)

set(SOURCES
    WithAovsInteropApp.cpp
)
add_executable(oidnapp ${SOURCES} ${HEADERS} main.cpp)

target_link_libraries(oidnapp 
    glfw 
    appcommon
    OpenImageDenoise
    
)
# automatically copy all dll's deps.
add_custom_command(TARGET oidnapp POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy -t $<TARGET_FILE_DIR:oidnapp> $<TARGET_RUNTIME_DLLS:oidnapp>
  COMMAND_EXPAND_LISTS
)
# copy hip kernels
populate_rprsdk_kernels(oidnapp)

# copy assets
_copy_file_to_target(oidnapp ${CMAKE_SOURCE_DIR}/apps/assets/teapot.obj)
_copy_file_to_target(oidnapp ${CMAKE_SOURCE_DIR}/apps/assets/envLightImage.exr)


