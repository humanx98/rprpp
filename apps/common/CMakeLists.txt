set(HEADERS
    HybridProRenderer.h
    rprpp_wrappers/helper.h
    rprpp_wrappers/Context.h
    rprpp_wrappers/Buffer.h
    rprpp_wrappers/Image.h
    rprpp_wrappers/PostProcessing.h
    rprpp_wrappers/PostProcessing.h
    rpr_helper.h
)
set(SOURCES
    HybridProRenderer.cpp
    rprpp_wrappers/helper.cpp
    rprpp_wrappers/Context.cpp
    rprpp_wrappers/Buffer.cpp
    rprpp_wrappers/Image.cpp
    rprpp_wrappers/PostProcessing.cpp
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

