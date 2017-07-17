AUX_DIST_GEN =                          \
        $(ac_aux_dir)/install-sh        \
        $(ac_aux_dir)/missing

MSVC_FILES=                             \
        msvc/chocolate.sln              \
        msvc/config.h                   \
        msvc/doom.vcproj                \
        msvc/heretic.vcproj             \
        msvc/hexen.vcproj               \
        msvc/strife.vcproj              \
        msvc/inttypes.h                 \
        msvc/libopl.vcproj              \
        msvc/libpcsound.vcproj          \
        msvc/libtextscreen.vcproj       \
        msvc/README                     \
        msvc/server.vcproj              \
        msvc/setup.vcproj               \
        msvc/stdint.h                   \
        msvc/win_opendir.c              \
        msvc/win_opendir.h              \
        msvc/win32.rc

CODEBLOCKS_FILES=                              \
        codeblocks/chocolate.workspace         \
        codeblocks/config.h                    \
        codeblocks/doom.cbp                    \
        codeblocks/game-res.rc                 \
        codeblocks/heretic.cbp                 \
        codeblocks/hexen.cbp                   \
        codeblocks/strife.cbp                  \
        codeblocks/libopl.cbp                  \
        codeblocks/libpcsound.cbp              \
        codeblocks/libtextscreen.cbp           \
        codeblocks/README                      \
        codeblocks/server.cbp                  \
        codeblocks/setup.cbp                   \
        codeblocks/setup-res.rc

DOC_FILES=                              \
        README.md                       \
        README.Music.md                 \
        NEWS.md                         \
        PHILOSOPHY.md                   \
        ChangeLog

EXTRA_DIST=                             \
        $(AUX_DIST_GEN)                 \
        $(MSVC_FILES)                   \
        $(CODEBLOCKS_FILES)             \
        $(DOC_FILES)                    \
        NOT-BUGS.md                     \
        README.Strife.md                \
        .lvimrc                         \
        HACKING.md                      \
        TODO.md                         \
        rpm.spec

doomdocsdir = ${docdir}/../${PROGRAM_PREFIX}doom
doomdocs_DATA = $(DOC_FILES) NOT-BUGS.md

hereticdocsdir = ${docdir}/../${PROGRAM_PREFIX}heretic
hereticdocs_DATA = $(DOC_FILES)

hexendocsdir = ${docdir}/../${PROGRAM_PREFIX}hexen
hexendocs_DATA = $(DOC_FILES)

strifedocsdir = ${docdir}/../${PROGRAM_PREFIX}strife
strifedocs_DATA = $(DOC_FILES) README.Strife.md

MAINTAINERCLEANFILES =  $(AUX_DIST_GEN)

SUBDIRS=textscreen midiproc opl pcsound data src man

DIST_SUBDIRS=pkg $(SUBDIRS)

if HAVE_PYTHON

INSTALL : man/INSTALL.template man/simplecpp
	./man/simplecpp -DDOOM -DHERETIC -DHEXEN -DSTRIFE \
	                < man/INSTALL.template > $@

endif

