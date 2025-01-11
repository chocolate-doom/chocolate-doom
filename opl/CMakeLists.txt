add_library(opl STATIC
            opl_internal.h
            opl.c           opl.h
            opl_linux.c
            opl_obsd.c
            opl_queue.c     opl_queue.h
            opl_sdl.c
            opl_timer.c     opl_timer.h
            opl_win32.c
            ioperm_sys.c    ioperm_sys.h
            opl3.c          opl3.h)
target_include_directories(opl
                           INTERFACE "."
                           PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/../")
if (DEFINED EMSCRIPTEN)
    if(ENABLE_SDL2_MIXER)
        set_target_properties(opl PROPERTIES COMPILE_FLAGS "-s USE_SDL=2 -s USE_SDL_MIXER=2")
        set_target_properties(opl PROPERTIES LINK_FLAGS "-s USE_SDL=2 -s USE_SDL_MIXER=2")
    else()
        set_target_properties(opl PROPERTIES COMPILE_FLAGS "-s USE_SDL=2")
        set_target_properties(opl PROPERTIES LINK_FLAGS "-s USE_SDL=2")
    endif()
else()
    target_link_libraries(opl SDL2::SDL2)
    if(ENABLE_SDL2_MIXER)
        target_link_libraries(opl SDL2_mixer::SDL2_mixer)
    endif()
endif()
