find_path(Stb_INCLUDE_DIR 
	NAME stb_image.h 
	HINTS ThirdParty
	PATH_SUFFIXES stb)

find_package_handle_standard_args(Stb REQUIRED_VARS Stb_INCLUDE_DIR)

