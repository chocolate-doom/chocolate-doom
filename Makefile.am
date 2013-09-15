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

DOC_FILES=                              \
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

SUBDIRS=textscreen opl pcsound data src man

DIST_SUBDIRS=pkg $(SUBDIRS)

if HAVE_PYTHON

INSTALL : man/INSTALL.template man/simplecpp
	./man/simplecpp -DDOOM -DHERETIC -DHEXEN -DSTRIFE \
	                < man/INSTALL.template > $@

endif

