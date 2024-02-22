set(HEADERS stbimpl.h)
set(SOURCES stbimpl.cpp)

add_library(stbimpl STATIC ${SOURCES} ${HEADERS})
target_include_directories(stbimpl PUBLIC "${CMAKE_SOURCE_DIR}/ThirdParty")
