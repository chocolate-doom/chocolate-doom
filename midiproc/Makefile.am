
AM_CFLAGS=-I$(top_srcdir)/src @SDLMIXER_CFLAGS@

if HAVE_WINDRES

noinst_PROGRAMS = @PROGRAM_PREFIX@midiproc

@PROGRAM_PREFIX@midiproc_LDADD = @SDLMIXER_LIBS@
@PROGRAM_PREFIX@midiproc_SOURCES = buffer.c buffer.h main.c proto.h

endif
