cmake_minimum_required(VERSION 3.2)
project("Chocolate Doom" VERSION 2.2.0)

set(PROGRAM_PREFIX "chocolate-")
set(PROJECT_BUGREPORT "fraggle@gmail.com")
set(PROJECT_COPYRIGHT "Copyright (C) 1993-2015")
set(PROJECT_LICENSE "GNU General Public License, version 2")
set(WINDOWS_RC_VERSION "2,2,0,0")

include(TestBigEndian)
test_big_endian(WORDS_BIGENDIAN)

find_package(SDL)
find_package(SDL_mixer)
find_package(SDL_net)

configure_file("cmake/config.h.in" "${CMAKE_CURRENT_BINARY_DIR}/config.h")
configure_file("cmake/resource.rc.in" "${CMAKE_CURRENT_BINARY_DIR}/src/resource.rc")

add_subdirectory(opl)
add_subdirectory(pcsound)
add_subdirectory(textscreen)

add_subdirectory(src)
