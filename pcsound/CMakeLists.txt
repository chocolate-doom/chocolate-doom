add_library(pcsound STATIC
            pcsound.c       pcsound.h
            pcsound_bsd.c
            pcsound_sdl.c
            pcsound_linux.c
            pcsound_win32.c
                            pcsound_internal.h)
target_include_directories(pcsound
                           INTERFACE "."
                           PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/../")
if (DEFINED EMSCRIPTEN)
    if(ENABLE_SDL2_MIXER)
        set_target_properties(pcsound PROPERTIES COMPILE_FLAGS "-s USE_SDL=2 -s USE_SDL_MIXER=2")
        set_target_properties(pcsound PROPERTIES LINK_FLAGS "-s USE_SDL=2 -s USE_SDL_MIXER=2")
    else()
        set_target_properties(pcsound PROPERTIES COMPILE_FLAGS "-s USE_SDL=2")
        set_target_properties(pcsound PROPERTIES LINK_FLAGS "-s USE_SDL=2")
    endif()
else()
    target_link_libraries(pcsound SDL2::SDL2)
    if(ENABLE_SDL2_MIXER)
        target_link_libraries(pcsound SDL2_mixer::SDL2_mixer)
    endif()
endif()
