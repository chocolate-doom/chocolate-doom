
AM_CFLAGS = @SDL_CFLAGS@ -I.. -I../../src

noinst_PROGRAMS=guitest calculator

guitest_LDADD = @LDFLAGS@ @SDL_LIBS@ ../libtextscreen.a
guitest_SOURCES = guitest.c

calculator_LDADD = @LDFLAGS@ @SDL_LIBS@ ../libtextscreen.a
calculator_SOURCES = calculator.c

