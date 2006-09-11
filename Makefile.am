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
        codeblocks/textscreen.cbp

EXTRA_DIST=                             \
        $(AUX_DIST_GEN)                 \
        $(MSVC_FILES)                   \
        $(CODEBLOCKS_FILES)             \
        config.h                        \
        TODO                            \
        BUGS                            \
        data/chocolate-doom.png         \
        data/chocolate-doom.ico

MAINTAINERCLEANFILES =  $(AUX_DIST_GEN)

docdir=$(prefix)/share/doc/@PACKAGE@
SUBDIRS=textscreen src man setup

