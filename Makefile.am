AUX_DIST_GEN =                          \
        $(ac_aux_dir)/install-sh        \
        $(ac_aux_dir)/missing

CMAKE_FILES=                                   \
        CMakeLists.txt                         \
        cmake/FindSDL2.cmake                   \
        cmake/FindSDL2_mixer.cmake             \
        cmake/FindSDL2_net.cmake               \
        cmake/FindFluidSynth.cmake             \
        cmake/Findm.cmake                      \
        cmake/FindSampleRate.cmake             \
        cmake/config.h.cin

DOC_FILES=                              \
        COPYING.md                      \
        README.md                       \
        README.Music.md                 \
        NEWS.md                         \
        PHILOSOPHY.md                   \
        ChangeLog

EXTRA_DIST=                             \
        $(AUX_DIST_GEN)                 \
        $(CMAKE_FILES)                  \
        $(DOC_FILES)                    \
        NOT-BUGS.md                     \
        README.Strife.md                \
        .lvimrc                         \
        HACKING.md                      \
        TODO.md                         \
        rpm.spec                        \
        win32/win_opendir.c             \
        win32/win_opendir.h

doomdocsdir = ${docdir}/../${PROGRAM_PREFIX}doom
doomdocs_DATA = $(DOC_FILES) NOT-BUGS.md

hereticdocsdir = ${docdir}/../${PROGRAM_PREFIX}heretic
hereticdocs_DATA = $(DOC_FILES)

hexendocsdir = ${docdir}/../${PROGRAM_PREFIX}hexen
hexendocs_DATA = $(DOC_FILES)

strifedocsdir = ${docdir}/../${PROGRAM_PREFIX}strife
strifedocs_DATA = $(DOC_FILES) README.Strife.md

MAINTAINERCLEANFILES =  $(AUX_DIST_GEN)

SUBDIRS=textscreen opl pcsound data src man

DIST_SUBDIRS=pkg $(SUBDIRS)

if HAVE_DOC

INSTALL : man/INSTALL.template man/simplecpp
	$(srcdir)/man/simplecpp -DDOOM -DHERETIC -DHEXEN -DSTRIFE \
            -DLONG_GAME_NAME="@PACKAGE_SHORTNAME@ Doom" \
            -DLONG_EXE_NAME="@PROGRAM_PREFIX@doom" \
	                < $(srcdir)/man/INSTALL.template > $@

endif
