include_directories(
    "${CMAKE_CURRENT_BINARY_DIR}/.."
    ${SDL_INCLUDE_DIR}
    ${SDL_MIXER_INCLUDE_DIR})
add_library(pcsound STATIC
            pcsound.c           pcsound.h
            pcsound_bsd.c
            pcsound_sdl.c
            pcsound_linux.c
            pcsound_win32.c
                                pcsound_internal.h)
