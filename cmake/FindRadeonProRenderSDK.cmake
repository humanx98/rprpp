find_path(RPR_INCLUDE_DIR 
	NAME RadeonProRender.h
	HINTS /usr/include /usr/local/include 
	"${RPR_SDK_ROOT}"
	ENV RPR_SDK_ROOT
    	PATH_SUFFIXES RadeonProRender/inc
)

find_library(RPR_LIBRARY 
	NAMES
	RadeonProRender64
	HINTS /usr/lib64 /usr/loca/lib64 
	"${RPR_SDK_ROOT}"
	ENV RPR_SDK_ROOT
	PATH_SUFFIXES RadeonProRender/binUbuntu18 
	              RadeonProRender/libWin64
)
find_path(RPR_HIP_KERNELS_DIR
	NAME AllPreCompilations.json
	HINTS /usr/share/RadeonProRender/hipbin
	"${RPR_SDK_ROOT}/hipbin"
	"${RPR_HIPBIN}"
	ENV RPR_SDK_ROOT 
	ENV RPR_HIPBIN
)

set(RadeonProRender_DLLS "")
if (WIN32)
	find_file(RadeonProRenderSDK_radeonprorender_dll 
		NAMES "RadeonProRender64.dll"
		HINTS
		"${RPR_SDK_ROOT}"
		ENV RPR_SDK_ROOT
		PATH_SUFFIXES RadeonProRender/binWin64)

	if (RadeonProRenderSDK_radeonprorender_dll)
		list(APPEND RadeonProRender_DLLS ${RadeonProRenderSDK_radeonprorender_dll})
	endif()
endif()

function(process_rpr_version)
	if(RPR_INCLUDE_DIR)
		file(READ "${RPR_INCLUDE_DIR}/RadeonProRender.h" HEADER_CONTENT)
		
		string(REGEX MATCHALL "#define RPR_VERSION_MAJOR[ \t\r\n]+([0-9]+)" VERSION_MAJOR_STR "${HEADER_CONTENT}")
		set(RPR_VERSION_MAJOR ${CMAKE_MATCH_1})

		string(REGEX MATCHALL "#define RPR_VERSION_MINOR[ \t\r\n]+([0-9]+)" VERSION_MINOR_STR "${HEADER_CONTENT}")
		set(RPR_VERSION_MINOR ${CMAKE_MATCH_1})

		string(REGEX MATCHALL "#define RPR_VERSION_REVISION[ \t\r\n]+([0-9]+)" REVISION_MINOR_STR "${HEADER_CONTENT}")
		set(RPR_VERSION_REVISION ${CMAKE_MATCH_1})

		unset(HEADER_CONTENT)
	endif()
	set(RPR_VERSION_STRING "${RPR_VERSION_MAJOR}.${RPR_VERSION_MINOR}.${RPR_VERSION_REVISION}" PARENT_SCOPE)
endfunction()

macro(find_tahoe)
	find_library(RPR_TAHOE_LIBRARY 
		NAMES
		Tahoe64
		HINTS /usr/lib64 /usr/local/lib64
		"${RPR_SDK_ROOT}"
		ENV RPR_SDK_ROOT
		PATH_SUFFIXES RadeonProRender/binUbuntu18
	     		      RadeonProRender/libWin64
	)

	if (WIN32)
		find_file(RadeonProRenderSDK_tahoe_dll 
			NAMES "Tahoe.dll"
			HINTS
			"${RPR_SDK_ROOT}"
			ENV RPR_SDK_ROOT
			PATH_SUFFIXES RadeonProRender/binWin64)
		if (RadeonProRenderSDK_tahoe_dll)
			list(APPEND RadeonProRender_DLLS ${RadeonProRenderSDK_tahoe_dll})
		endif()
	endif()


	if(NOT TARGET RadeonProRenderSDK::tahoe)
    		add_library(RadeonProRenderSDK::tahoe INTERFACE IMPORTED)		
    		set_target_properties(RadeonProRenderSDK::tahoe PROPERTIES
			#INTERFACE_INCLUDE_DIRECTORIES "${RPR_INCLUDE_DIR}"
			INTERFACE_LINK_LIBRARIES "${RPR_TAHOE_LIBRARY}")
    	endif()
endmacro()

macro(find_northstar)
	find_library(RPR_NORTHSTAR_LIBRARY 
		NAMES
		Northstar64
		HINTS /usr/lib64 /usr/local/lib64
		"${RPR_SDK_ROOT}"
		ENV RPR_SDK_ROOT
		PATH_SUFFIXES RadeonProRender/binUbuntu18
	     		      RadeonProRender/libWin64

	)

	if (WIN32)
		find_file(RadeonProRenderSDK_northstar_dll 
			NAMES "Northstar64.dll"
			HINTS
			"${RPR_SDK_ROOT}"
			ENV RPR_SDK_ROOT
			PATH_SUFFIXES RadeonProRender/binWin64)
		if (RadeonProRenderSDK_northstar_dll)
			list(APPEND RadeonProRender_DLLS ${RadeonProRenderSDK_northstar_dll})
		endif()
	endif()

	if(NOT TARGET RadeonProRenderSDK::northstar)
    		add_library(RadeonProRenderSDK::northstar INTERFACE IMPORTED)		
    		set_target_properties(RadeonProRenderSDK::northstar PROPERTIES
			INTERFACE_LINK_LIBRARIES "${RPR_NORTHSTAR_LIBRARY}")
	endif()
endmacro()
macro(find_hybrid)
	find_library(RPR_HYBRID_LIBRARY 
		NAMES
		Hybrid
		HINTS /usr/lib64 /usr/local/lib64
		"${RPR_SDK_ROOT}"
		ENV RPR_SDK_ROOT
		PATH_SUFFIXES RadeonProRender/binUbuntu18
	     		      RadeonProRender/libWin64
	)

	if (WIN32)
		find_file(RadeonProRenderSDK_hybrid_dll 
			NAMES "Hybrid.dll"
			HINTS
			"${RPR_SDK_ROOT}"
			ENV RPR_SDK_ROOT
			PATH_SUFFIXES RadeonProRender/binWin64)
		if (RadeonProRenderSDK_hybrid_dll)
			list(APPEND RadeonProRender_DLLS ${RadeonProRenderSDK_hybrid_dll})
		endif()
	endif()


	if(NOT TARGET RadeonProRenderSDK::hybrid)
    	add_library(RadeonProRenderSDK::hybrid INTERFACE IMPORTED)		
    	set_target_properties(RadeonProRenderSDK::hybrid PROPERTIES
			INTERFACE_LINK_LIBRARIES "${RPR_HYBRID_LIBRARY}")
    endif()
endmacro()

macro(find_hybridpro)
	find_library(RPR_HYBRIDPRO_LIBRARY
			NAMES
			HybridPro
			HINTS /usr/lib64 /usr/local/lib64
			"${RPR_SDK_ROOT}"
			ENV RPR_SDK_ROOT
			PATH_SUFFIXES RadeonProRender/binUbuntu18
			RadeonProRender/libWin64
			)

	if (WIN32)
		find_file(RadeonProRenderSDK_hybridpro_dll
				NAMES "HybridPro.dll"
				HINTS
				"${RPR_SDK_ROOT}"
				ENV RPR_SDK_ROOT
				PATH_SUFFIXES RadeonProRender/binWin64)
		if (RadeonProRenderSDK_hybridpro_dll)
			list(APPEND RadeonProRender_DLLS ${RadeonProRenderSDK_hybridpro_dll})
		endif()
	endif()


	if(NOT TARGET RadeonProRenderSDK::hybridpro)
		add_library(RadeonProRenderSDK::hybridpro INTERFACE IMPORTED)
		set_target_properties(RadeonProRenderSDK::hybridpro PROPERTIES
				INTERFACE_LINK_LIBRARIES "${RPR_HYBRIDPRO_LIBRARY}")
	endif()
endmacro()

process_rpr_version()


foreach(component IN LISTS RadeonProRenderSDK_FIND_COMPONENTS)
	string(TOUPPER "${component}" component_upcase)

	if (component STREQUAL "tahoe")
		find_tahoe()
	elseif(component STREQUAL "northstar")
		find_northstar()
	elseif(component STREQUAL "hybrid")
		find_hybrid()
	elseif(component STREQUAL "hybridpro")
		find_hybridpro()
	else()
		message(WARNING "${component} is not a valid RadeonProRender component")
		set(RadeonProRenderSDK_${component}_FOUND false)
	endif()

	if(TARGET RadeonProRenderSDK::${component})
 		set(RadeonProRenderSDK_${component}_FOUND TRUE)
	else()
	    	set(RadeonProRenderSDK_${component}_FOUND FALSE)
	endif()


endforeach()
unset(component)

find_package_handle_standard_args(RadeonProRenderSDK
	REQUIRED_VARS RPR_LIBRARY RPR_INCLUDE_DIR RPR_HIP_KERNELS_DIR
    VERSION_VAR RPR_VERSION_STRING
    HANDLE_COMPONENTS)

if(RadeonProRenderSDK_FOUND)
	set(RadeonProRenderSDK_INCLUDE_DIRS ${RPR_INCLUDE_DIR})
	set(RadeonProRenderSDK_LIBRARIES ${RPR_LIBRARY})
	set(RadeonProRenderSDK_HIP_KERNELS_DIRS ${RPR_HIP_KERNELS_DIR})
	
	if(NOT TARGET RadeonProRenderSDK::RPR)
		add_library(RadeonProRenderSDK::RPR IMPORTED SHARED)
		set_target_properties(RadeonProRenderSDK::RPR PROPERTIES
			INTERFACE_INCLUDE_DIRECTORIES ${RPR_INCLUDE_DIR}
			IMPORTED_IMPLIB ${RPR_LIBRARY}
			IMPORTED_LOCATION "${RadeonProRender_DLLS}"
			)
	endif()
endif()

# ---------------------------------------------------------
# util functions
# ---------------------------------------------------------
function(populate_rprsdk target)
        foreach(dll ${RadeonProRender_DLLS})
                get_filename_component(file_name "${dll}" NAME)
                add_custom_command(
                        TARGET ${target} POST_BUILD
                        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                        ${dll}
                        $<TARGET_FILE_DIR:${target}>)
        endforeach()
endfunction()

function(populate_rprsdk_kernels target)
        file(GLOB hipkernels "${RadeonProRenderSDK_HIP_KERNELS_DIRS}/*")
        foreach(hipkernel ${hipkernels})
                get_filename_component(file_name "${hipkernel}" NAME)
                add_custom_command(
                        TARGET ${target} POST_BUILD
                        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                        "${hipkernel}"
                        "$<TARGET_FILE_DIR:${target}>/hipbin/${file_name}")
                        
        endforeach()
endfunction()

