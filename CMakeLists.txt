cmake_minimum_required(VERSION 3.28.1)
project(rprpp LANGUAGES CXX VERSION 0.1)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_COMPILE_WARNING_AS_ERROR ON)

set(LIBS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/libs)
set(RPRSDK_DIR ${LIBS_DIR}/RadeonProRender)

include(FetchContent)

macro(_copy_file_to_target target thefile)
    add_custom_command(
        TARGET ${target} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${thefile}
        $<TARGET_FILE_DIR:${target}>
        VERBATIM
    )
endmacro()

add_subdirectory(src)
add_subdirectory(apps)