set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

cmake_minimum_required(VERSION 3.9.3)
project("Chocolate Doom" VERSION 3.0.0 LANGUAGES C)

find_package(SDL2 2.0.1)
find_package(SDL2_mixer 2.0.0)
find_package(SDL2_net 2.0.0)

add_subdirectory(textscreen)
