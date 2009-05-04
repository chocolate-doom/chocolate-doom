
AM_CFLAGS = @SDL_CFLAGS@ -I../../textscreen -I.. -DINSTALL_DIR="\"$(gamesdir)\""

noinst_LIBRARIES = libsetup.a

SOURCE_FILES =                                  \
    compatibility.c   compatibility.h           \
    display.c         display.h                 \
    joystick.c        joystick.h                \
    keyboard.c        keyboard.h                \
    mainmenu.c                                  \
    mode.c            mode.h                    \
    mouse.c           mouse.h                   \
    multiplayer.c     multiplayer.h             \
    sound.c           sound.h                   \
    execute.c         execute.h                 \
    txt_joybinput.c   txt_joybinput.h           \
    txt_keyinput.c    txt_keyinput.h            \
    txt_mouseinput.c  txt_mouseinput.h

libsetup_a_SOURCES = $(SOURCE_FILES)

EXTRA_DIST=                                     \
    setup_icon.c

