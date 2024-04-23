set(HEADERS
    HybridProRenderer.h
    rprpp_wrappers/filters/BloomFilter.h
    rprpp_wrappers/filters/ComposeColorShadowReflectionFilter.h
    rprpp_wrappers/filters/ComposeOpacityShadowFilter.h
    rprpp_wrappers/filters/DenoiserFilter.h
    rprpp_wrappers/filters/Filter.h
    rprpp_wrappers/filters/ToneMapFilter.h
    rprpp_wrappers/helper.h
    rprpp_wrappers/Context.h
    rprpp_wrappers/Buffer.h
    rprpp_wrappers/Image.h
    rpr_helper.h
)
set(SOURCES
    HybridProRenderer.cpp
    rprpp_wrappers/filters/BloomFilter.cpp
    rprpp_wrappers/filters/ComposeColorShadowReflectionFilter.cpp
    rprpp_wrappers/filters/ComposeOpacityShadowFilter.cpp
    rprpp_wrappers/filters/DenoiserFilter.cpp
    rprpp_wrappers/filters/Filter.cpp
    rprpp_wrappers/filters/ToneMapFilter.cpp
    rprpp_wrappers/helper.cpp
    rprpp_wrappers/Context.cpp
    rprpp_wrappers/Buffer.cpp
    rprpp_wrappers/Image.cpp
    rpr_helper.cpp
)

add_library(appcommon STATIC ${HEADERS} ${SOURCES})
target_include_directories(appcommon PUBLIC
    PUBLIC ${CMAKE_SOURCE_DIR}/apps
)

target_link_libraries(appcommon
	PUBLIC RadeonProRenderSDK::RPR
	PUBLIC stbimpl
	PUBLIC rprpp
    PUBLIC Boost::program_options
)

