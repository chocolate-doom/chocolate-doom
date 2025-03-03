add_library(setup STATIC
            accessibility.c     accessibility.h
            compatibility.c     compatibility.h
            display.c           display.h
            joystick.c          joystick.h
            keyboard.c          keyboard.h
            mainmenu.c
            mode.c              mode.h
            mouse.c             mouse.h
            multiplayer.c       multiplayer.h
            sound.c             sound.h
            execute.c           execute.h
            txt_joyaxis.c       txt_joyaxis.h
            txt_joybinput.c     txt_joybinput.h
            txt_keyinput.c      txt_keyinput.h
            txt_mouseinput.c    txt_mouseinput.h)

target_include_directories(setup PRIVATE "../" "${CMAKE_CURRENT_BINARY_DIR}/../../")

if (DEFINED EMSCRIPTEN)
    set(SDL_FLAGS "-s USE_SDL=2")
    if(ENABLE_SDL2_MIXER)
        set(SDL_FLAGS "${SDL_FLAGS} -s USE_SDL_MIXER=2")
    endif()
else()
    set(SDL_LINK_LIBS, "SDL2::SDL2")
    if(ENABLE_SDL2_MIXER)
        list(APPEND SDL_LINK_LIBS SDL2_mixer::SDL2_mixer)
    endif()
endif()
target_link_libraries(setup textscreen ${SDL_LINK_LIBS})
set_target_properties(setup PROPERTIES COMPILE_FLAGS "${SDL_FLAGS}")
set_target_properties(setup PROPERTIES LINK_FLAGS "${SDL_FLAGS}")
