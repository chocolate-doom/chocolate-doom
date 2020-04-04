add_library(pcsound STATIC EXCLUDE_FROM_ALL
            pcsound.c       pcsound.h
            pcsound_bsd.c
            pcsound_sdl.c
            pcsound_linux.c
            pcsound_win32.c
                            pcsound_internal.h
)

target_include_directories(pcsound
    INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}"
    PUBLIC "${PROJECT_BINARY_DIR}"
)

target_link_libraries(pcsound PUBLIC
    SDL2::mixer
)
