add_library(opl STATIC EXCLUDE_FROM_ALL
            opl_internal.h
            opl.c           opl.h
            opl_linux.c
            opl_obsd.c
            opl_queue.c     opl_queue.h
            opl_sdl.c
            opl_timer.c     opl_timer.h
            opl_win32.c
            ioperm_sys.c    ioperm_sys.h
            opl3.c          opl3.h
)

target_include_directories(opl
    INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}"
    PRIVATE "${PROJECT_BINARY_DIR}"
)

target_link_libraries(opl PUBLIC
    SDL2::mixer
)
