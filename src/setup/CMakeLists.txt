add_executable(setup WIN32
            compatibility.c     compatibility.h
            display.c           display.h
            joystick.c          joystick.h
            keyboard.c          keyboard.h
            mainmenu.c
            mode.c              mode.h
            mouse.c             mouse.h
            multiplayer.c       multiplayer.h
            setup_icon.c
            sound.c             sound.h
            execute.c           execute.h
            txt_joyaxis.c       txt_joyaxis.h
            txt_joybinput.c     txt_joybinput.h
            txt_keyinput.c      txt_keyinput.h
            txt_mouseinput.c    txt_mouseinput.h
)

convert_icon("${PROJECT_DATADIR}/setup.png" setup_icon.c COPY "${CMAKE_CURRENT_SOURCE_DIR}/setup_icon.c")

if(WIN32)
    target_sources(setup PRIVATE
        "${CMAKE_CURRENT_BINARY_DIR}/../setup-res.rc"
    )
endif()

target_link_libraries(setup PRIVATE
    setupcommon
    textscreen
)

set_target_properties(setup PROPERTIES
    OUTPUT_NAME "${PROGRAM_PREFIX}setup"
)

if(MSVC)
    set_target_properties(setup PROPERTIES
        LINK_FLAGS "/MANIFEST:NO"
    )
endif()
