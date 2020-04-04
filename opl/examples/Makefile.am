
AM_CFLAGS = \
    -I$(top_srcdir)/opl  \
    -I$(top_srcdir)/safe

noinst_PROGRAMS=droplay

droplay_LDADD = \
    $(top_srcdir)/opl/libopl.a \
    $(top_srcdir)/safe/libsafe.a \
    @LDFLAGS@ @SDL_LIBS@ @SDLMIXER_LIBS@
droplay_SOURCES = droplay.c

