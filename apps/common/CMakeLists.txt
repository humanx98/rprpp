set(HEADERS
    StagingBuffer.h
    HybridProRenderer.h
    RprPostProcessing.h
    rpr_helper.h
)
set(SOURCES
    StagingBuffer.cpp
    HybridProRenderer.cpp
    RprPostProcessing.cpp
    rpr_helper.cpp
)

add_library(appcommon STATIC ${HEADERS} ${SOURCES})
target_include_directories(appcommon PUBLIC
    PUBLIC ${CMAKE_SOURCE_DIR}/apps
)

target_link_libraries(appcommon PUBLIC
    RadeonProRenderSDK::RPR
    rprpp
    stbimpl
)

