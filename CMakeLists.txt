list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(ParseAutoconfigure)

cmake_minimum_required(VERSION 3.7.2)
project("${PACKAGE_NAME}"
    VERSION "${PACKAGE_VERSION}"
    DESCRIPTION "${PACKAGE_SHORTDESC}"
    HOMEPAGE_URL "${PACKAGE_URL}"
    LANGUAGES C
)

# Any settings that should apply to all targets in this directory and all
# subdirectories should go here.  Use judiciously.
if(MSVC)
    add_definitions(
        _CRT_SECURE_NO_WARNINGS
        _CRT_SECURE_NO_DEPRECATE
        _CRT_NONSTDC_NO_DEPRECATE
    )
else()
    add_compile_options(
        -Wall
        $<$<COMPILE_LANGUAGE:C>:-Wdeclaration-after-statement>
        -Wredundant-decls
    )
endif()

find_package(SDL2 2.0.1 REQUIRED)
find_package(SDL2_mixer 2.0.0 REQUIRED)
find_package(SDL2_net 2.0.0 REQUIRED)

find_package(samplerate)
if(SAMPLERATE_FOUND)
    set(HAVE_LIBSAMPLERATE TRUE)
endif()

find_package(PNG)
if(PNG_FOUND)
    set(HAVE_LIBPNG TRUE)
endif()

find_package(m)

include(CheckSymbolExists)
include(CheckIncludeFile)
check_symbol_exists(strcasecmp "strings.h" HAVE_DECL_STRCASECMP)
check_symbol_exists(strncasecmp "strings.h" HAVE_DECL_STRNCASECMP)
check_include_file("dirent.h" HAVE_DIRENT_H)

string(CONCAT WINDOWS_RC_VERSION "${PROJECT_VERSION_MAJOR}, "
    "${PROJECT_VERSION_MINOR}, ${PROJECT_VERSION_PATCH}, 0")

configure_file(cmake/config.h.cin config.h)

configure_file(src/resource.rc.in src/resource.rc)
configure_file(src/setup-res.rc.in src/setup-res.rc)
configure_file(src/setup/setup-manifest.xml.in src/setup/setup-manifest.xml)

add_subdirectory(textscreen)
add_subdirectory(midiproc)
add_subdirectory(opl)
add_subdirectory(pcsound)
add_subdirectory(src)
