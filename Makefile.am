AUX_DIST_GEN =                          \
        $(ac_aux_dir)/install-sh        \
        $(ac_aux_dir)/missing

MSVC_FILES=                             \
        msvc/README                     \
        msvc/config.h                   \
        msvc/inttypes.h                 \
        msvc/stdint.h

CODEBLOCKS_FILES=                              \
        codeblocks/README                      \
        codeblocks/chocolate-doom.cbp          \
        codeblocks/chocolate-doom.workspace    \
        codeblocks/chocolate-server.cbp        \
        codeblocks/chocolate-setup.cbp         \
        codeblocks/chocolate-doom-res.rc       \
        codeblocks/textscreen.cbp              \
        codeblocks/chocolate-setup-res.rc      \
        codeblocks/config.h                    \
        codeblocks/libpcsound.cbp

DATA_FILES=                             \
        data/chocolate-doom.ico         \
        data/chocolate-doom.png         \
        data/chocolate-setup.ico        \
        data/chocolate-setup.png        \
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
	./man/docgen -p src/ > $@

endif

