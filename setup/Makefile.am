AM_CFLAGS = @SDL_CFLAGS@ -I../textscreen -I../src

gamesdir = $(prefix)/games

games_PROGRAMS = chocolate-setup

chocolate_setup_LDADD = @LDFLAGS@ @SDL_LIBS@ ../textscreen/libtextscreen.a
chocolate_setup_SOURCES = mainmenu.c keyboard.c

