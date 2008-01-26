
gamesdir = $(prefix)/games

AM_CFLAGS = @SDL_CFLAGS@ -I../textscreen -I../src -DINSTALL_DIR="\"$(gamesdir)\""

games_PROGRAMS = strawberry-setup

strawberry_setup_LDADD = ../textscreen/libtextscreen.a @LDFLAGS@ @SDL_LIBS@
SOURCE_FILES =                                  \
    compatibility.c   compatibility.h           \
    configfile.c      configfile.h              \
    display.c         display.h                 \
    joystick.c        joystick.h                \
    keyboard.c        keyboard.h                \
    m_argv.c          m_argv.h                  \
    mainmenu.c                                  \
    mouse.c           mouse.h                   \
    multiplayer.c     multiplayer.h             \
    sound.c           sound.h                   \
    execute.c         execute.h                 \
    txt_joybinput.c   txt_joybinput.h           \
    txt_keyinput.c    txt_keyinput.h            \
    txt_mouseinput.c  txt_mouseinput.h

EXTRA_DIST=                                     \
    chocolate_setup_icon.c

if HAVE_WINDRES
strawberry_setup_SOURCES=$(SOURCE_FILES) chocolate-setup-res.rc
else
strawberry_setup_SOURCES=$(SOURCE_FILES)
endif

.rc.o:
	$(WINDRES) $^ -o $@
%.o : %.rc
	$(WINDRES) $^ -o $@

if HAVE_PYTHON

chocolate_setup_icon.c : ../data/chocolate-setup.png
	../data/convert-icon $^ $@

endif

