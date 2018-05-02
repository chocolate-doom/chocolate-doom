# Currently works with the following generators:
# - Visual Studio

set(SDL2_DIR "${SDL2_DIR}" CACHE PATH "Location of SDL2 library directory")

find_path(SDL2_INCLUDE_DIR "SDL_version.h"
          PATHS "${SDL2_DIR}/include")

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

if(CMAKE_SIZEOF_VOID_P STREQUAL 8)
    find_library(SDL2_LIBRARY "SDL2"
                 PATHS "${SDL2_DIR}/lib/x64")
    find_library(SDL2_MAIN_LIBRARY "SDL2main"
                 PATHS "${SDL2_DIR}/lib/x64")
else()
    find_library(SDL2_LIBRARY "SDL2"
                 PATHS "${SDL2_DIR}/lib/x86")
    find_library(SDL2_MAIN_LIBRARY "SDL2main"
                 PATHS "${SDL2_DIR}/lib/x86")
endif()
set(SDL2_LIBRARIES "${SDL2_MAIN_LIBRARY}" "${SDL2_LIBRARY}")

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(SDL2
    FOUND_VAR SDL2_FOUND
    REQUIRED_VARS SDL2_INCLUDE_DIR SDL2_LIBRARIES
    VERSION_VAR SDL2_VERSION
)

if(SDL2_FOUND)
    add_library(SDL2::SDL2 UNKNOWN IMPORTED)
    set_target_properties(SDL2::SDL2 PROPERTIES
                          INTERFACE_INCLUDE_DIRECTORIES "${SDL2_INCLUDE_DIR}"
                          IMPORTED_LOCATION "${SDL2_LIBRARY}")
    add_library(SDL2::SDL2main UNKNOWN IMPORTED)
    set_target_properties(SDL2::SDL2main PROPERTIES
                          IMPORTED_LOCATION "${SDL2_MAIN_LIBRARY}")
endif()
