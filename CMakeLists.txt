set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# X_VCPKG_APPLOCAL_DEPS_INSTALL automatically installs dependencies if vcpkg is
# used.
set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)
set(X_VCPKG_APPLOCAL_DEPS_INSTALL ON)

cmake_minimum_required(VERSION 3.7.2)
project("Chocolate Doom" VERSION 3.0.0 LANGUAGES C)

# Autotools variables
set(top_srcdir ${CMAKE_CURRENT_SOURCE_DIR})
set(top_builddir ${CMAKE_CURRENT_BINARY_DIR})

# AC_INIT variables
set(PACKAGE_NAME "${PROJECT_NAME}")
set(PACKAGE_TARNAME "chocolate-doom")
set(PACKAGE_VERSION "${PROJECT_VERSION}")
set(PACKAGE_STRING "${PROJECT_NAME} ${PROJECT_VERSION}")
set(PACKAGE_BUGREPORT "chocolate-doom-dev-list@chocolate-doom.org")

string(REGEX REPLACE " Doom$" "" PACKAGE_SHORTNAME "${PACKAGE_NAME}")
set(PACKAGE_COPYRIGHT "Copyright (C) 1993-2022")
set(PACKAGE_LICENSE "GNU General Public License, version 2")

# Any settings that should apply to all targets in this directory and all
# subdirectories should go here.  Use judiciously.
if(MSVC)
    add_definitions("/D_CRT_SECURE_NO_WARNINGS" "/D_CRT_SECURE_NO_DEPRECATE"
                    "/D_CRT_NONSTDC_NO_DEPRECATE")
else()
    add_compile_options("-Wall" "-Wdeclaration-after-statement"
                        "-Wredundant-decls")
endif()

if(VCPKG_INSTALLED_DIR)
    set(VCPKG_DIR TRUE)
endif()

if(VCPKG_DIR)
    set(ENABLE_SDL2_NET TRUE)
    set(ENABLE_SDL2_MIXER TRUE)

    find_package(SDL2 CONFIG REQUIRED)
    find_package(SDL2_mixer CONFIG REQUIRED)
    find_package(SDL2_net CONFIG REQUIRED)
else()
    option(ENABLE_SDL2_NET "Enable SDL2_net" On)
    option(ENABLE_SDL2_MIXER "Enable SDL2_mixer" On)

    find_package(SDL2 2.0.7)
    if(ENABLE_SDL2_MIXER)
        find_package(SDL2_mixer 2.0.2)
    else()
        add_compile_definitions(DISABLE_SDL2MIXER=1)
    endif()

    if(ENABLE_SDL2_NET)
        find_package(SDL2_net 2.0.0)
    else()
        add_compile_definitions(DISABLE_SDL2NET=1)
    endif()
endif()

# Check for libsamplerate.
find_package(samplerate)
if(SAMPLERATE_FOUND)
    set(HAVE_LIBSAMPLERATE TRUE)
endif()

# Check for libpng.
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

# Without a hyphen. This is used for the bash-completion scripts.
string(TOLOWER "${PACKAGE_SHORTNAME}" PROGRAM_SPREFIX)

# With a hyphen, used almost everywhere else.
set(PROGRAM_PREFIX "${PROGRAM_SPREFIX}-")

configure_file(cmake/config.h.cin config.h)

configure_file(src/doom-res.rc.in src/doom-res.rc)
configure_file(src/heretic-res.rc.in src/heretic-res.rc)
configure_file(src/hexen-res.rc.in src/hexen-res.rc)
configure_file(src/setup-res.rc.in src/setup-res.rc)
configure_file(src/setup/setup-manifest.xml.in src/setup/setup-manifest.xml)
configure_file(src/strife-res.rc.in src/strife-res.rc)

foreach(SUBDIR textscreen opl pcsound src)
    add_subdirectory("${SUBDIR}")
endforeach()

# Generate distribution packages with CPack.
if(VCPKG_DIR)
    set(CPACK_GENERATOR ZIP)
    set(CPACK_STRIP_FILES TRUE)
    set(CPACK_ARCHIVE_COMPONENT_INSTALL ON)
    if(CMAKE_SIZEOF_VOID_P STREQUAL 8)
        set(SYS_POSTFIX "win64")
    else()
        set(SYS_POSTFIX "win32")
    endif()
    set(CPACK_ARCHIVE_DOOM_FILE_NAME "${PROGRAM_PREFIX}doom-${PROJECT_VERSION}-${SYS_POSTFIX}")
    set(CPACK_ARCHIVE_HERETIC_FILE_NAME "${PROGRAM_PREFIX}heretic-${PROJECT_VERSION}-${SYS_POSTFIX}")
    set(CPACK_ARCHIVE_HEXEN_FILE_NAME "${PROGRAM_PREFIX}hexen-${PROJECT_VERSION}-${SYS_POSTFIX}")
    include(CPack)
endif()
