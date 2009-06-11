
AM_CFLAGS = -I.. -I$(top_builddir)/src

noinst_PROGRAMS=guitest calculator

guitest_LDADD = ../libtextscreen.a @LDFLAGS@ @SDL_LIBS@
guitest_SOURCES = guitest.c

calculator_LDADD = ../libtextscreen.a @LDFLAGS@ @SDL_LIBS@
calculator_SOURCES = calculator.c

