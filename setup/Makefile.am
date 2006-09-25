AM_CFLAGS = @SDL_CFLAGS@ -I../textscreen -I../src

gamesdir = $(prefix)/games

games_PROGRAMS = chocolate-setup

chocolate_setup_LDADD = @LDFLAGS@ @SDL_LIBS@ ../textscreen/libtextscreen.a
chocolate_setup_SOURCES =                       \
    compatibility.c                             \
    display.c                                   \
    keyboard.c                                  \
    mainmenu.c                                  \
    mouse.c                                     \
    multiplayer.c                               \
    sound.c                                     \
    testconfig.c      testconfig.h              \
    txt_keyinput.c    txt_keyinput.h            \
    txt_mouseinput.c  txt_mouseinput.h

