FetchContent_Declare(stb 
    GIT_REPOSITORY https://github.com/nothings/stb.git
    GIT_SHALLOW TRUE
    GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(stb)

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
	PUBLIC ${RPRSDK_DIR}/inc
	PUBLIC ${stb_SOURCE_DIR}
)

target_link_libraries(appcommon PUBLIC 
    rprpp
    vkdx11_interop
    ${RPRSDK_DIR}/libWin64/RadeonProRender64.lib
 )

