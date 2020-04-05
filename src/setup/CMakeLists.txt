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

doom_install(TARGETS setup)

generate_install_desktop(Setup
    SRCDIR "${CMAKE_CURRENT_SOURCE_DIR}"
)

add_custom_target(create_setups ALL)

set(SETUP_BIN_DIR "${CMAKE_CURRENT_BINARY_DIR}" CACHE INTERNAL "setup binary dir")

function(create_setup_link NAME)
    set(SETUP_NAME "${PROGRAM_PREFIX}${NAME}-setup${CMAKE_EXECUTABLE_SUFFIX}")
    add_custom_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${SETUP_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E create_symlink "$<TARGET_FILE:setup>" "${CMAKE_CURRENT_BINARY_DIR}/${SETUP_NAME}"
    )
    add_custom_target("create_setup_${NAME}" DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/${SETUP_NAME}")
    add_dependencies(create_setups "create_setup_${NAME}")

    cmake_policy(SET CMP0087 NEW)
    install(CODE "file(CREATE_LINK \"$<TARGET_FILE_BASE_NAME:setup>\" \"\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR}/${SETUP_NAME}\" SYMBOLIC)")
endfunction()
