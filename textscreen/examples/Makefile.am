
AM_CFLAGS =  -I$(top_srcdir)/src -I$(top_srcdir)/textscreen

noinst_PROGRAMS=guitest calculator

guitest_LDADD = ../libtextscreen.a @LDFLAGS@ @SDL_LIBS@
guitest_SOURCES = guitest.c

calculator_LDADD = ../libtextscreen.a @LDFLAGS@ @SDL_LIBS@
calculator_SOURCES = calculator.c

