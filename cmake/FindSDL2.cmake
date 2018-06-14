# FindSDL2.cmake
#
# Copyright (c) 2018, Alex Mayfield <alexmax2742@gmail.com>
# All rights reserved.
# 
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#     * Redistributions of source code must retain the above copyright
#       notice, this list of conditions and the following disclaimer.
#     * Redistributions in binary form must reproduce the above copyright
#       notice, this list of conditions and the following disclaimer in the
#       documentation and/or other materials provided with the distribution.
#     * Neither the name of the <organization> nor the
#       names of its contributors may be used to endorse or promote products
#       derived from this software without specific prior written permission.
# 
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
# DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
# ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
# Currently works with the following generators:
# - Unix Makefiles
# - Ninja
# - Visual Studio

# Cache variable that allows you to point CMake at a directory containing
# an extracted development library.
set(SDL2_DIR "${SDL2_DIR}" CACHE PATH "Location of SDL2 library directory")

# Use pkg-config to find library locations in *NIX environments.
find_package(PkgConfig QUIET)
if(PKG_CONFIG_FOUND)
    pkg_search_module(PC_SDL2 QUIET sdl2)
endif()

# Find the include directory.
find_path(SDL2_INCLUDE_DIR "SDL_version.h"
    HINTS "${SDL2_DIR}/include" ${PC_SDL2_INCLUDE_DIRS})

# Find the version.  Taken and modified from CMake's FindSDL.cmake.
if(SDL2_INCLUDE_DIR AND EXISTS "${SDL2_INCLUDE_DIR}/SDL_version.h")
    file(STRINGS "${SDL2_INCLUDE_DIR}/SDL_version.h" SDL2_VERSION_MAJOR_LINE REGEX "^#define[ \t]+SDL_MAJOR_VERSION[ \t]+[0-9]+$")
    file(STRINGS "${SDL2_INCLUDE_DIR}/SDL_version.h" SDL2_VERSION_MINOR_LINE REGEX "^#define[ \t]+SDL_MINOR_VERSION[ \t]+[0-9]+$")
    file(STRINGS "${SDL2_INCLUDE_DIR}/SDL_version.h" SDL2_VERSION_PATCH_LINE REGEX "^#define[ \t]+SDL_PATCHLEVEL[ \t]+[0-9]+$")
    string(REGEX REPLACE "^#define[ \t]+SDL_MAJOR_VERSION[ \t]+([0-9]+)$" "\\1" SDL2_VERSION_MAJOR "${SDL2_VERSION_MAJOR_LINE}")
    string(REGEX REPLACE "^#define[ \t]+SDL_MINOR_VERSION[ \t]+([0-9]+)$" "\\1" SDL2_VERSION_MINOR "${SDL2_VERSION_MINOR_LINE}")
    string(REGEX REPLACE "^#define[ \t]+SDL_PATCHLEVEL[ \t]+([0-9]+)$" "\\1" SDL2_VERSION_PATCH "${SDL2_VERSION_PATCH_LINE}")
    set(SDL2_VERSION "${SDL2_VERSION_MAJOR}.${SDL2_VERSION_MINOR}.${SDL2_VERSION_PATCH}")
    unset(SDL2_VERSION_MAJOR_LINE)
    unset(SDL2_VERSION_MINOR_LINE)
    unset(SDL2_VERSION_PATCH_LINE)
    unset(SDL2_VERSION_MAJOR)
    unset(SDL2_VERSION_MINOR)
    unset(SDL2_VERSION_PATCH)
endif()

# Find the SDL2 and SDL2main libraries
if(CMAKE_SIZEOF_VOID_P STREQUAL 8)
    find_library(SDL2_LIBRARY "SDL2"
        HINTS "${SDL2_DIR}/lib/x64" ${PC_SDL2_LIBRARY_DIRS})
    find_library(SDL2_MAIN_LIBRARY "SDL2main"
        HINTS "${SDL2_DIR}/lib/x64" ${PC_SDL2_LIBRARY_DIRS})
else()
    find_library(SDL2_LIBRARY "SDL2"
        HINTS "${SDL2_DIR}/lib/x86" ${PC_SDL2_LIBRARY_DIRS})
    find_library(SDL2_MAIN_LIBRARY "SDL2main"
        HINTS "${SDL2_DIR}/lib/x86" ${PC_SDL2_LIBRARY_DIRS})
endif()
set(SDL2_LIBRARIES "${SDL2_MAIN_LIBRARY}" "${SDL2_LIBRARY}")

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(SDL2
    FOUND_VAR SDL2_FOUND
    REQUIRED_VARS SDL2_INCLUDE_DIR SDL2_LIBRARIES
    VERSION_VAR SDL2_VERSION
)

if(SDL2_FOUND)
    # SDL2 imported target.
    add_library(SDL2::SDL2 UNKNOWN IMPORTED)
    set_target_properties(SDL2::SDL2 PROPERTIES
                          INTERFACE_COMPILE_OPTIONS "${PC_SDL2_CFLAGS_OTHER}"
                          INTERFACE_INCLUDE_DIRECTORIES "${SDL2_INCLUDE_DIR}"
                          IMPORTED_LOCATION "${SDL2_LIBRARY}")

    # SDL2main imported target.
    add_library(SDL2::SDL2main UNKNOWN IMPORTED)
    set_target_properties(SDL2::SDL2main PROPERTIES
                          IMPORTED_LOCATION "${SDL2_MAIN_LIBRARY}")
endif()
