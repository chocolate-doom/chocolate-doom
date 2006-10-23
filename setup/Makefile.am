
gamesdir = $(prefix)/games

AM_CFLAGS = @SDL_CFLAGS@ -I../textscreen -I../src -DINSTALL_DIR="\"$(gamesdir)\""

games_PROGRAMS = chocolate-setup

chocolate_setup_LDADD = @LDFLAGS@ @SDL_LIBS@ ../textscreen/libtextscreen.a
chocolate_setup_SOURCES =                       \
    compatibility.c   compatibility.h           \
    configfile.c      configfile.h              \
    display.c         display.h                 \
    keyboard.c        keyboard.h                \
    m_argv.c          m_argv.h                  \
    mainmenu.c                                  \
    mouse.c           mouse.h                   \
    multiplayer.c     multiplayer.h             \
    sound.c           sound.h                   \
    execute.c         execute.h                 \
    txt_keyinput.c    txt_keyinput.h            \
    txt_mouseinput.c  txt_mouseinput.h

