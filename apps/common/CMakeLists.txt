set(HEADERS
    HybridProRenderer.h
    WRprPpContext.h
    WRprPpBuffer.h
    WRprPpImage.h
    WRprPpPostProcessing.h
    rpr_helper.h
)
set(SOURCES
    HybridProRenderer.cpp
    WRprPpContext.cpp
    WRprPpBuffer.cpp
    WRprPpImage.cpp
    WRprPpPostProcessing.cpp
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

