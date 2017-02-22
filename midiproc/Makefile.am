
AM_CFLAGS=-I$(top_srcdir)/src @SDLMIXER_CFLAGS@

if HAVE_WINDRES

noinst_PROGRAMS = midiproc

midiproc_LDADD = @SDLMIXER_LIBS@
midiproc_SOURCES = buffer.c buffer.h main.c

endif
