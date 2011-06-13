
EXTRA_DIST=              \
        README           \
        doom.ico         \
        doom8.ico        \
        doom.png         \
        setup.ico        \
        setup8.ico       \
        setup.png        \
        convert-icon

iconsdir = $(prefix)/share/icons
icons_DATA = @PROGRAM_PREFIX@doom.png   \
             @PROGRAM_PREFIX@setup.png

@PROGRAM_PREFIX@doom.png : doom.png
	cp $^ $@

@PROGRAM_PREFIX@setup.png : setup.png
	cp $^ $@

