AC_INIT(Chocolate Doom, 3.0.1,
        chocolate-doom-dev-list@chocolate-doom.org, chocolate-doom)

PACKAGE_SHORTNAME=${PACKAGE_NAME% Doom}
PACKAGE_SHORTDESC="Conservative source port"
PACKAGE_COPYRIGHT="Copyright (C) 1993-2017"
PACKAGE_LICENSE="GNU General Public License, version 2"
PACKAGE_MAINTAINER="Simon Howard"
PACKAGE_URL="https://www.chocolate-doom.org/"
PACKAGE_ISSUES="https://github.com/chocolate-doom/chocolate-doom/issues"

AC_CONFIG_AUX_DIR(autotools)
AC_CANONICAL_HOST

orig_CFLAGS="$CFLAGS"

AC_PROG_CC
AC_PROG_RANLIB
AC_CHECK_PROG(HAVE_PYTHON, python, true, false)

OPT_LEVEL=2

# If this is gcc, we have some options we'd like to turn on.  Turn on 
# optimisation and debugging symbols.

if test "$GCC" = "yes"
then
        WARNINGS="-Wall -Wdeclaration-after-statement -Wredundant-decls"
        CFLAGS="-O$OPT_LEVEL -g $WARNINGS $orig_CFLAGS"
fi

PKG_CHECK_MODULES(SDL, [sdl2 >= 2.0.1])
PKG_CHECK_MODULES(SDLMIXER, [SDL2_mixer >= 2.0.0])
PKG_CHECK_MODULES(SDLNET, [SDL2_net >= 2.0.0])

# Check for libsamplerate.
AC_ARG_WITH([libsamplerate],
AS_HELP_STRING([--without-libsamplerate],
    [Build without libsamplerate @<:@default=check@:>@]),
[],
[
    [with_libsamplerate=check]
])
AS_IF([test "x$with_libsamplerate" != xno], [
    PKG_CHECK_MODULES(SAMPLERATE, samplerate >= 0.1.8, [
        AC_DEFINE([HAVE_LIBSAMPLERATE], [1], [libsamplerate installed])
    ], [
        AS_IF([test "x$with_libsamplerate" != xcheck], [AC_MSG_FAILURE(
            [--with-libsamplerate was given, but test for libsamplerate failed])
        ])
    ])
])

# Check for libpng.
AC_ARG_WITH([libpng],
AS_HELP_STRING([--without-libpng],
    [Build without libpng @<:@default=check@:>@]),
[],
[
    [with_libpng=check]
])
AS_IF([test "x$with_libpng" != xno], [
    PKG_CHECK_MODULES(PNG, libpng >= 1.6.10, [
        AC_DEFINE([HAVE_LIBPNG], [1], [libpng installed])
    ], [
        AS_IF([test "x$with_libpng" != xcheck], [AC_MSG_FAILURE(
            [--with-libpng was given, but test for libpng failed])
        ])
    ])
])

# TODO: We currently link everything against libraries that don't need it.
# Use the specific library CFLAGS/LIBS variables instead of setting them here.
CFLAGS="$CFLAGS $SDL_CFLAGS ${SAMPLERATE_CFLAGS:-} ${PNG_CFLAGS:-}"
LDFLAGS="$LDFLAGS $SDL_LIBS ${SAMPLERATE_LIBS:-} ${PNG_LIBS:-}"
AC_CHECK_LIB(m, log)

AC_CHECK_HEADERS([linux/kd.h dev/isa/spkrio.h dev/speaker/speaker.h])
AC_CHECK_FUNCS(mmap ioperm)
AC_CHECK_DECLS([strcasecmp, strncasecmp], [], [], [[#include <strings.h>]])

# OpenBSD I/O i386 library for I/O port access.
# (64 bit has the same thing with a different name!)

AC_CHECK_LIB(i386, i386_iopl)
AC_CHECK_LIB(amd64, amd64_iopl)

AC_ARG_WITH([bashcompletiondir],
    AS_HELP_STRING([--with-bashcompletiondir=DIR], [Bash completion directory]),
    [],
    [AS_IF([$($PKG_CONFIG --exists bash-completion 2> /dev/null)],
        [bashcompletiondir=$($PKG_CONFIG --variable=completionsdir bash-completion)],
	[bashcompletiondir=${datadir}/bash-completion/completions])])

case "$host" in
  *-*-mingw* | *-*-cygwin* | *-*-msvc* )
    AC_CHECK_TOOL(WINDRES, windres, )
    ;;
  *)
    WINDRES=
    ;;
esac

AC_CHECK_TOOL(OBJDUMP, objdump, )
AC_CHECK_TOOL(STRIP, strip, )

AM_CONDITIONAL(HAVE_WINDRES, test "$WINDRES" != "")
AM_CONDITIONAL(HAVE_PYTHON, $HAVE_PYTHON)

dnl Automake v1.8.0 is required, please upgrade!

AM_INIT_AUTOMAKE([1.8.0 foreign])
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

WINDOWS_RC_VERSION=`echo $PACKAGE_VERSION | sed 's/-.*//; s/\./, /g; s/$/, 0/'`

dnl Without a hyphen. This is used for the bash-completion scripts.
PROGRAM_SPREFIX=$(echo $PACKAGE_SHORTNAME | tr A-Z a-z)

dnl With a hyphen, used almost everywhere else.
PROGRAM_PREFIX=${PROGRAM_SPREFIX}-

AC_SUBST(PROGRAM_PREFIX)
AC_DEFINE_UNQUOTED(PROGRAM_PREFIX, "$PROGRAM_PREFIX",
                   Change this when you create your awesome forked version)
AC_SUBST(PROGRAM_SPREFIX)

AM_CONFIG_HEADER(config.h:config.hin)

AC_SUBST(WINDOWS_RC_VERSION)
AC_SUBST(SDLMIXER_CFLAGS)
AC_SUBST(SDLMIXER_LIBS)

AC_SUBST(SDLNET_CFLAGS)
AC_SUBST(SDLNET_LIBS)

AC_SUBST(ac_aux_dir)

AC_SUBST(PACKAGE_SHORTNAME)
AC_SUBST(PACKAGE_SHORTDESC)
AC_SUBST(PACKAGE_COPYRIGHT)
AC_SUBST(PACKAGE_LICENSE)
AC_SUBST(PACKAGE_MAINTAINER)
AC_SUBST(PACKAGE_URL)
AC_SUBST(PACKAGE_ISSUES)

AC_SUBST(bashcompletiondir)

dnl Shut up the datarootdir warnings.
AC_DEFUN([AC_DATAROOTDIR_CHECKED])

AC_OUTPUT([
Makefile
man/Makefile
man/bash-completion/Makefile
man/bash-completion/doom.template
man/bash-completion/heretic.template
man/bash-completion/hexen.template
man/bash-completion/strife.template
midiproc/Makefile
opl/Makefile
opl/examples/Makefile
pcsound/Makefile
pkg/Makefile
pkg/config.make
pkg/osx/Info.plist
rpm.spec
data/Makefile
src/Makefile
src/doom.appdata.xml
src/doom.desktop
src/doom-screensaver.desktop
src/doom/Makefile
src/heretic.appdata.xml
src/heretic.desktop
src/heretic/Makefile
src/hexen.appdata.xml
src/hexen.desktop
src/hexen/Makefile
src/resource.rc
src/setup-res.rc
src/setup/Makefile
src/setup/setup.desktop
src/setup/setup-manifest.xml
src/strife.appdata.xml
src/strife.desktop
src/strife/Makefile
textscreen/Makefile
textscreen/examples/Makefile
textscreen/fonts/Makefile
])

