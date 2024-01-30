cmake_minimum_required(VERSION 3.28.1)
#   Allow GIT_SUBMODULES empty to mean no submodules
cmake_policy(SET CMP0097 NEW)
project(vkdx11interop LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_COMPILE_WARNING_AS_ERROR ON)

add_subdirectory(src)