# FindSDL2_net.cmake
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
set(SDL2_NET_DIR "${SDL2_NET_DIR}" CACHE PATH "Location of SDL2_net library directory")

# Use pkg-config to find library locations in *NIX environments.
find_package(PkgConfig QUIET)
if(PKG_CONFIG_FOUND)
    pkg_search_module(PC_SDL2_NET QUIET SDL2_net)
endif()

# Find the include directory.
find_path(SDL2_NET_INCLUDE_DIR "SDL_net.h"
    HINTS "${SDL2_NET_DIR}/include" ${PC_SDL2_NET_INCLUDE_DIRS})

# Find the version.  Taken and modified from CMake's FindSDL.cmake.
if(SDL2_NET_INCLUDE_DIR AND EXISTS "${SDL2_NET_INCLUDE_DIR}/SDL_net.h")
    file(STRINGS "${SDL2_NET_INCLUDE_DIR}/SDL_net.h" SDL2_NET_VERSION_MAJOR_LINE REGEX "^#define[ \t]+SDL_NET_MAJOR_VERSION[ \t]+[0-9]+$")
    file(STRINGS "${SDL2_NET_INCLUDE_DIR}/SDL_net.h" SDL2_NET_VERSION_MINOR_LINE REGEX "^#define[ \t]+SDL_NET_MINOR_VERSION[ \t]+[0-9]+$")
    file(STRINGS "${SDL2_NET_INCLUDE_DIR}/SDL_net.h" SDL2_NET_VERSION_PATCH_LINE REGEX "^#define[ \t]+SDL_NET_PATCHLEVEL[ \t]+[0-9]+$")
    string(REGEX REPLACE "^#define[ \t]+SDL_NET_MAJOR_VERSION[ \t]+([0-9]+)$" "\\1" SDL2_NET_VERSION_MAJOR "${SDL2_NET_VERSION_MAJOR_LINE}")
    string(REGEX REPLACE "^#define[ \t]+SDL_NET_MINOR_VERSION[ \t]+([0-9]+)$" "\\1" SDL2_NET_VERSION_MINOR "${SDL2_NET_VERSION_MINOR_LINE}")
    string(REGEX REPLACE "^#define[ \t]+SDL_NET_PATCHLEVEL[ \t]+([0-9]+)$" "\\1" SDL2_NET_VERSION_PATCH "${SDL2_NET_VERSION_PATCH_LINE}")
    set(SDL2_NET_VERSION "${SDL2_NET_VERSION_MAJOR}.${SDL2_NET_VERSION_MINOR}.${SDL2_NET_VERSION_PATCH}")
    unset(SDL2_NET_VERSION_MAJOR_LINE)
    unset(SDL2_NET_VERSION_MINOR_LINE)
    unset(SDL2_NET_VERSION_PATCH_LINE)
    unset(SDL2_NET_VERSION_MAJOR)
    unset(SDL2_NET_VERSION_MINOR)
    unset(SDL2_NET_VERSION_PATCH)
endif()

# Find the library.
if(CMAKE_SIZEOF_VOID_P STREQUAL 8)
    find_library(SDL2_NET_LIBRARY "SDL2_net"
        HINTS "${SDL2_NET_DIR}/lib/x64" ${PC_SDL2_NET_LIBRARY_DIRS})
else()
    find_library(SDL2_NET_LIBRARY "SDL2_net"
        HINTS "${SDL2_NET_DIR}/lib/x86" ${PC_SDL2_NET_LIBRARY_DIRS})
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(SDL2_net
    FOUND_VAR SDL2_NET_FOUND
    REQUIRED_VARS SDL2_NET_INCLUDE_DIR SDL2_NET_LIBRARY
    VERSION_VAR SDL2_NET_VERSION
)

if(SDL2_NET_FOUND)
    # Imported target.
    add_library(SDL2::net UNKNOWN IMPORTED)
    set_target_properties(SDL2::net PROPERTIES
                          INTERFACE_COMPILE_OPTIONS "${PC_SDL2_NET_CFLAGS_OTHER}"
                          INTERFACE_INCLUDE_DIRECTORIES "${SDL2_NET_INCLUDE_DIR}"
                          INTERFACE_LINK_LIBRARIES SDL2::SDL2
                          IMPORTED_LOCATION "${SDL2_NET_LIBRARY}")
endif()
