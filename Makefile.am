AUX_DIST_GEN =                          \
        $(ac_aux_dir)/install-sh        \
        $(ac_aux_dir)/missing

MSVC_FILES=                             \
        msvc/chocolate.sln              \
        msvc/config.h                   \
        msvc/doom.vcproj                \
        msvc/heretic.vcproj             \
        msvc/hexen.vcproj               \
        msvc/inttypes.h                 \
        msvc/libpcsound.vcproj          \
        msvc/libtextscreen.vcproj       \
        msvc/README                     \
        msvc/server.vcproj              \
        msvc/setup.vcproj               \
        msvc/stdint.h                   \
        msvc/win32.rc

CODEBLOCKS_FILES=                              \
        codeblocks/chocolate.workspace         \
        codeblocks/config.h                    \
        codeblocks/doom.cbp                    \
        codeblocks/game-res.rc                 \
        codeblocks/heretic.cbp                 \
        codeblocks/hexen.cbp                   \
        codeblocks/libpcsound.cbp              \
        codeblocks/libtextscreen.cbp           \
        codeblocks/README                      \
        codeblocks/server.cbp                  \
        codeblocks/setup.cbp                   \
        codeblocks/setup-res.rc

DATA_FILES=                   \
        data/README           \
        data/doom.ico         \
        data/doom8.ico        \
        data/doom.png         \
        data/setup.ico        \
        data/setup8.ico       \
        data/setup.png        \
        data/convert-icon

EXTRA_DIST=                             \
        $(AUX_DIST_GEN)                 \
        $(MSVC_FILES)                   \
        $(CODEBLOCKS_FILES)             \
        $(DATA_FILES)                   \
        .lvimrc                         \
        config.h                        \
        CMDLINE                         \
        HACKING                         \
        TODO                            \
        BUGS                            \
        rpm.spec

MAINTAINERCLEANFILES =  $(AUX_DIST_GEN)

docdir=$(prefix)/share/doc/@PACKAGE@
SUBDIRS=wince textscreen pcsound src man 
DIST_SUBDIRS=pkg $(SUBDIRS)

if HAVE_PYTHON

noinst_DATA=CMDLINE

CMDLINE : src/
	./man/docgen -p man/CMDLINE.template src/ > $@

endif

