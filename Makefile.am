AUX_DIST_GEN =                          \
        $(ac_aux_dir)/config.guess      \
        $(ac_aux_dir)/config.sub        \
        $(ac_aux_dir)/install-sh        \
        $(ac_aux_dir)/missing           \
        $(ac_aux_dir)/mkinstalldirs

MSVC_FILES=                             \
        msvc/README                     \
        msvc/config.h

CODEBLOCKS_FILES=                              \
        codeblocks/README                      \
        codeblocks/chocolate-doom.cbp          \
        codeblocks/chocolate-doom.workspace    \
        codeblocks/chocolate-server.cbp        \
        codeblocks/chocolate-setup.cbp        \
        codeblocks/chocolate-doom-res.rc       \
        codeblocks/textscreen.cbp

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
SUBDIRS=textscreen src man setup

CMDLINE : src/
	./man/docgen -p $? > $@

