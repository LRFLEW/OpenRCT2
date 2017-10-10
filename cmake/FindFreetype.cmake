# TODO: This module can be deleted with CMake 3.10, which supports import targets
# for Freetype

include(FindPackageHandleStandardArgs)

find_library(FREETYPE_LIBRARY NAMES libfreetype.a Freetype)
find_path(FREETYPE_INCLUDE_DIR ft2build.h PATH_SUFFIXES freetype2)

if(MINGW)
    find_dependency(BZip2)
endif()

add_library(Freetype::Freetype IMPORTED UNKNOWN)

set_target_properties(Freetype::Freetype PROPERTIES
    IMPORTED_LOCATION "${FREETYPE_LIBRARY}"
    INTERFACE_INCLUDE_DIRECTORIES "${FREETYPE_INCLUDE_DIR}"
    )

find_package_handle_standard_args(Freetype
    "Unable to find required Freetype library"
    FREETYPE_LIBRARY FREETYPE_INCLUDE_DIR
    )

if(MINGW)
    set_property(TARGET Freetype::Freetype APPEND PROPERTY INTERFACE_LINK_LIBRARIES BZip2::BZip2)
endif()