#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "OpenImageDenoiseAMD_core" for configuration "Release"
set_property(TARGET OpenImageDenoiseAMD_core APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(OpenImageDenoiseAMD_core PROPERTIES
  IMPORTED_IMPLIB_RELEASE "${_IMPORT_PREFIX}/lib/OpenImageDenoiseAMD_core.lib"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/bin/OpenImageDenoiseAMD_core.dll"
  )

list(APPEND _cmake_import_check_targets OpenImageDenoiseAMD_core )
list(APPEND _cmake_import_check_files_for_OpenImageDenoiseAMD_core "${_IMPORT_PREFIX}/lib/OpenImageDenoiseAMD_core.lib" "${_IMPORT_PREFIX}/bin/OpenImageDenoiseAMD_core.dll" )

# Import target "OpenImageDenoiseAMD" for configuration "Release"
set_property(TARGET OpenImageDenoiseAMD APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(OpenImageDenoiseAMD PROPERTIES
  IMPORTED_IMPLIB_RELEASE "${_IMPORT_PREFIX}/lib/OpenImageDenoiseAMD.lib"
  IMPORTED_LINK_DEPENDENT_LIBRARIES_RELEASE "OpenImageDenoiseAMD_core"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/bin/OpenImageDenoiseAMD.dll"
  )

list(APPEND _cmake_import_check_targets OpenImageDenoiseAMD )
list(APPEND _cmake_import_check_files_for_OpenImageDenoiseAMD "${_IMPORT_PREFIX}/lib/OpenImageDenoiseAMD.lib" "${_IMPORT_PREFIX}/bin/OpenImageDenoiseAMD.dll" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
