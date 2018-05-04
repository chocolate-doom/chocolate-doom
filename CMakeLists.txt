set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

cmake_minimum_required(VERSION 3.7.2)
project("Chocolate Doom" VERSION 3.0.0 LANGUAGES C)

# AC_INIT variables
set(PACKAGE_NAME "${PROJECT_NAME}")
set(PACKAGE_TARNAME "chocolate-doom")
set(PACKAGE_STRING "${PROJECT_NAME} ${PROJECT_VERSION}")

string(REGEX REPLACE " Doom$" "" PACKAGE_SHORTNAME "${PACKAGE_NAME}")

find_package(SDL2 2.0.1)
find_package(SDL2_mixer 2.0.0)
find_package(SDL2_net 2.0.0)

include(CheckFunctionExists)
check_function_exists(strcasecmp HAVE_DECL_STRCASECMP)
check_function_exists(strncasecmp HAVE_DECL_STRNCASECMP)

# Without a hyphen. This is used for the bash-completion scripts.
string(TOLOWER "${PACKAGE_SHORTNAME}" PROGRAM_SPREFIX)

# With a hyphen, used almost everywhere else.
set(PROGRAM_PREFIX "${PROGRAM_SPREFIX}-")

configure_file(config.cmake.h config.h)

foreach(SUBDIR textscreen midiproc opl pcsound src)
    add_subdirectory("${SUBDIR}")
endforeach()
