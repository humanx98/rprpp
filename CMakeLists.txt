cmake_minimum_required(VERSION 3.28.1)
#   Allow GIT_SUBMODULES empty to mean no submodules
cmake_policy(SET CMP0097 NEW)
project(vkdx11interop LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_COMPILE_WARNING_AS_ERROR ON)

include(FetchContent)
set(FETCHCONTENT_QUIET FALSE)

# Fetching rprsdk takes about 5min
# 
# FetchContent_Declare(rprsdk 
#     GIT_REPOSITORY https://github.com/GPUOpen-LibrariesAndSDKs/RadeonProRenderSDK.git
#     GIT_TAG v3.1.4
#     GIT_SUBMODULES ""
#     GIT_SHALLOW TRUE
#     GIT_PROGRESS TRUE
# )
# FetchContent_MakeAvailable(rprsdk)
# set(RPRSDK ${rprsdk_SOURCE_DIR}/RadeonProRender) 
set(RPRSDK ${CMAKE_CURRENT_SOURCE_DIR}/libs/RadeonProRender)


add_subdirectory(src)