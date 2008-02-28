AUX_DIST_GEN =                          \
        $(ac_aux_dir)/install-sh        \
        $(ac_aux_dir)/missing

MSVC_FILES=                             \
        msvc/README                     \
        msvc/config.h                   \
        msvc/inttypes.h                 \
        msvc/stdint.h                   \
        msvc/win32.rc                   \
        msvc/ChocolateDoom.sln          \
        msvc/ChocolateDoom.vcproj

CODEBLOCKS_FILES=                              \
	codeblocks/config.h                    \
	codeblocks/game.cbp                    \
	codeblocks/game-res.rc                 \
	codeblocks/libpcsound.cbp              \
	codeblocks/main.workspace              \
	codeblocks/README                      \
	codeblocks/server.cbp                  \
	codeblocks/setup.cbp                   \
	codeblocks/setup-res.rc                \
	codeblocks/textscreen.cbp

DATA_FILES=                   \
        data/doom.ico         \
        data/doom.png         \
        data/setup.ico        \
        data/setup.png        \
        data/convert-icon

EXTRA_DIST=                             \
        $(AUX_DIST_GEN)                 \
        $(MSVC_FILES)                   \
        $(CODEBLOCKS_FILES)             \
        $(DATA_FILES)                   \
        config.h                        \
        CMDLINE                         \
        HACKING                         \
        TODO                            \
        BUGS

MAINTAINERCLEANFILES =  $(AUX_DIST_GEN)

docdir=$(prefix)/share/doc/@PACKAGE@
SUBDIRS=textscreen pcsound src man setup

if HAVE_PYTHON

CMDLINE : src/
	./man/docgen -p man/CMDLINE.template src/ > $@

endif

