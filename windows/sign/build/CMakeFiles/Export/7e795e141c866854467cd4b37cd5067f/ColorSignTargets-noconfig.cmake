#----------------------------------------------------------------
# Generated CMake target import file.
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "ColorSign::colorsign" for configuration ""
set_property(TARGET ColorSign::colorsign APPEND PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)
set_target_properties(ColorSign::colorsign PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_NOCONFIG "CXX"
  IMPORTED_LOCATION_NOCONFIG "${_IMPORT_PREFIX}/lib/libcolorsign.a"
  )

list(APPEND _cmake_import_check_targets ColorSign::colorsign )
list(APPEND _cmake_import_check_files_for_ColorSign::colorsign "${_IMPORT_PREFIX}/lib/libcolorsign.a" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
