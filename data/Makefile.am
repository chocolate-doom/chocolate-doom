
EXTRA_DIST=              \
        README           \
        doom.ico         \
        doom.png         \
        heretic.ico      \
        heretic.png      \
        hexen.ico        \
        hexen.png        \
        setup.ico        \
        setup.png        \
        strife.ico       \
        strife.png       \
        wadfile.png      \
        convert-icon

iconsdir = $(prefix)/share/icons/hicolor/128x128/apps
icons_DATA = @PROGRAM_PREFIX@doom.png    \
             @PROGRAM_PREFIX@heretic.png \
             @PROGRAM_PREFIX@hexen.png   \
             @PROGRAM_PREFIX@setup.png   \
             @PROGRAM_PREFIX@strife.png

@PROGRAM_PREFIX@doom.png : doom.png
	cp $(top_srcdir)/data/doom.png $@

@PROGRAM_PREFIX@heretic.png : heretic.png
	cp $(top_srcdir)/data/heretic.png $@

@PROGRAM_PREFIX@hexen.png : hexen.png
	cp $(top_srcdir)/data/hexen.png $@

@PROGRAM_PREFIX@setup.png : setup.png
	cp $(top_srcdir)/data/setup.png $@

@PROGRAM_PREFIX@strife.png : strife.png
	cp $(top_srcdir)/data/strife.png $@

CLEANFILES = $(icons_DATA)
