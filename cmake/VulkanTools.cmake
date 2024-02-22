function(check_vulkan vklib)
	get_filename_component(Vulkan_LIBRARY_DIR ${vklib} DIRECTORY)
	find_file(Vulkan_SHADERC_COMBINEDD_LIB NAMES shaderc_combinedd.lib HINTS ${Vulkan_LIBRARY_DIR})
	if(NOT Vulkan_SHADERC_COMBINEDD_LIB)
	  message(FATAL_ERROR "Install 'Shader Toolchain Debug Symbols - 64 bit' with VulkanSDK in order to have shaderc_combinedd.lib")
	endif()
endfunction()


