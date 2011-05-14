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

DOC_FILES=                              \
        CMDLINE                         \
        README                          \
        README.OPL                      \
        NEWS                            \
        ChangeLog                       \
        NOT-BUGS

EXTRA_DIST=                             \
        $(AUX_DIST_GEN)                 \
        $(MSVC_FILES)                   \
        $(CODEBLOCKS_FILES)             \
        $(DOC_FILES)                    \
        .lvimrc                         \
        HACKING                         \
        TODO                            \
        rpm.spec

docdir=$(prefix)/share/doc/@PACKAGE@
doc_DATA=$(DOC_FILES)

MAINTAINERCLEANFILES =  $(AUX_DIST_GEN)

SUBDIRS=wince textscreen opl pcsound data src man setup
DIST_SUBDIRS=pkg $(SUBDIRS)

if HAVE_PYTHON

noinst_DATA=CMDLINE

CMDLINE : src/
	./man/docgen -p man/CMDLINE.template src/ > $@

INSTALL : man/INSTALL.template man/simplecpp
	./man/simplecpp < man/INSTALL.template > $@

endif

