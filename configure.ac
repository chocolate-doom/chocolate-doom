AC_INIT(Crispy Doom, 7.0.0,
        fabian@greffrath.com, crispy-doom)

PACKAGE_SHORTNAME=${PACKAGE_NAME% Doom}
PACKAGE_SHORTDESC="Limit-removing enhanced-resolution Doom source port"
PACKAGE_COPYRIGHT="Copyright (C) 1993-2024"
PACKAGE_LICENSE="GNU General Public License, version 2"
PACKAGE_MAINTAINER="Fabian Greffrath"
PACKAGE_URL="http://fabiangreffrath.github.io/crispy-doom"
PACKAGE_RDNS="io.github.fabiangreffrath"
PACKAGE_BUGREPORT="https://github.com/fabiangreffrath/crispy-doom/issues"

dnl Check for Emscripten environment so we can set necessary environment variables
AC_ARG_ENABLE(
  [emscripten],
  [AS_HELP_STRING([--enable-emscripten=@<:@no/yes@:>@],[compile to JavaScript using Emscripten])],,
  [AS_CASE([$CC],
           [*emcc.py], [enable_emscripten="yes"],
           [*emcc], [enable_emscripten="yes"],
           [enable_emscripten="no"]
	  )
 ]
)

AS_IF([test "x$enable_emscripten" = "xyes"],
  AC_MSG_NOTICE([auto-detected use of Emscripten])
  SDL_CFLAGS="-s USE_SDL=2"
  SDL_LIBS="-s USE_SDL=2"
  SDLMIXER_CFLAGS="-s USE_SDL=2 -s USE_SDL_MIXER=2"
  SDLMIXER_LIBS="-s USE_SDL=2 -s USE_SDL_MIXER=2"
  SDLNET_CFLAGS="-s USE_SDL=2 -s USE_SDL_NET=2"
  SDLNET_LIBS="-s USE_SDL=2 -s USE_SDL_NET=2"
  EM_LDFLAGS="-s ALLOW_MEMORY_GROWTH=1 -s ASYNCIFY=1 -s EXPORTED_RUNTIME_METHODS=[ccall,cwrap,FS,ENV,PATH,ERRNO_CODES] -s FORCE_FILESYSTEM"
)

AC_CONFIG_AUX_DIR(autotools)
AC_CANONICAL_HOST

orig_CFLAGS="$CFLAGS"

AM_PROG_AR
AC_PROG_CC_C99
AC_PROG_RANLIB
AC_CHECK_PROG(HAVE_PYTHON, python3, true, false)

OPT_LEVEL=2

# If this is gcc, we have some options we'd like to turn on.  Turn on 
# optimisation and debugging symbols.

if test "$GCC" = "yes"
then
        WARNINGS="-Wall -Wdeclaration-after-statement -Wredundant-decls"
        CFLAGS="-O$OPT_LEVEL -g $WARNINGS $orig_CFLAGS"
fi

AC_CANONICAL_TARGET
TARGET_OS=""
AS_CASE([$target_cpu],
        [x86_64],
        [
            TARGET_OS="x64"
        ],
        [aarch64*|arm*],
        [
            TARGET_OS="arm"
        ],
        [i?86|arm*],
        [
            TARGET_OS="32"
        ])
AC_SUBST([TARGET_OS])

PKG_CHECK_MODULES(SDL, [sdl2 >= 2.0.14])
# Check for SDL2_mixer
AC_ARG_ENABLE([sdl2mixer],
AS_HELP_STRING([--disable-sdl2mixer], [Disable SDL2_mixer support])
)
AS_IF([test "x$enable_sdl2mixer" != xno], [
    PKG_CHECK_MODULES(SDLMIXER, [SDL2_mixer >= 2.0.2])], [
    AC_DEFINE([DISABLE_SDL2MIXER], [1], [SDL2_mixer disabled])
])

# Check for networking
AC_ARG_ENABLE([sdl2net],
AS_HELP_STRING([--disable-sdl2net], [Disable SDL2_net support])
)
AS_IF([test "x$enable_sdl2net" != xno], [
    PKG_CHECK_MODULES(SDLNET, [SDL2_net >= 2.0.0])], [
    AC_DEFINE([DISABLE_SDL2NET], [1], [SDL2_net disabled])
])

# Check for bash-completion.
AC_ARG_ENABLE([bash-completion],
AS_HELP_STRING([--disable-bash-completion], [Disable bash-completion])
)
AS_IF([test "x$enable_bash_completion" != xno], [
    AS_IF([test "x$HAVE_PYTHON" = xfalse], [
        AC_MSG_WARN([Building bash-completion requires Python, but Python not found])
        enable_bash_completion=no
    ])
])

# Check for doc.
AC_ARG_ENABLE([doc],
AS_HELP_STRING([--disable-doc], [Disable documentation])
)
AS_IF([test "x$enable_doc" != xno], [
    AS_IF([test "x$HAVE_PYTHON" = xfalse], [
        AC_MSG_WARN([Building documentation requires Python, but Python not found])
        enable_doc=no
    ])
])

# Check for fonts.
AC_ARG_ENABLE([fonts],
AS_HELP_STRING([--disable-fonts], [Disable fonts])
)
AS_IF([test "x$enable_fonts" != xno], [
    AS_IF([test "x$HAVE_PYTHON" = xfalse], [
        AC_MSG_WARN([Building fonts require Python, but Python not found])
        enable_fonts=no
    ])
])

# Check for icons.
AC_ARG_ENABLE([icons],
AS_HELP_STRING([--disable-icons], [Disable icons])
)
AS_IF([test "x$enable_icons" != xno], [
    AS_IF([test "x$HAVE_PYTHON" = xfalse], [
        AC_MSG_WARN([Building icons require Python, but Python not found])
        enable_icons=no
    ])
])

# [Crispy] Check for zlib.
AC_ARG_WITH([zlib],
AS_HELP_STRING([--without-zlib],
    [Build without zlib @<:@default=check@:>@]),
[],
[
    [with_zlib=check]
])
AS_IF([test "x$with_zlib" != xno], [
    PKG_CHECK_MODULES(LIBZ, zlib, [
        AC_DEFINE([HAVE_LIBZ], [1], [zlib installed])
    ], [
        AS_IF([test "x$with_zlib" != xcheck], [AC_MSG_FAILURE(
            [--with-zlib was given, but test for zlib failed])
        ])
    ])
])

# Check for memory pool debugging.
AC_ARG_ENABLE([zpool],
AS_HELP_STRING([--disable-zpool], [Disable memory pools for debugging])
)
AS_IF([test "x$enable_zpool" != xno], [], [
    AC_DEFINE([DISABLE_ZPOOL], [1], [Memory pooling disabled])
])

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
    PKG_CHECK_MODULES(PNG, libpng >= 1.2.50, [
        AC_DEFINE([HAVE_LIBPNG], [1], [libpng installed])
    ], [
        AS_IF([test "x$with_libpng" != xcheck], [AC_MSG_FAILURE(
            [--with-libpng was given, but test for libpng failed])
        ])
    ])
])

# Check for fluidsynth.
AC_ARG_WITH([fluidsynth],
AS_HELP_STRING([--without-fluidsynth],
    [Build without fluidsynth @<:@default=check@:>@]),
[],
[
    [with_fluidsynth=check]
])
AS_IF([test "x$with_fluidsynth" != xno] && [test "x$enable_sdl2mixer" != xno], [
    PKG_CHECK_MODULES(FLUIDSYNTH, fluidsynth >= 2.2.0, [
        AC_DEFINE([HAVE_FLUIDSYNTH], [1], [fluidsynth installed])
    ], [
        AS_IF([test "x$with_fluidsynth" != xcheck], [AC_MSG_FAILURE(
            [--with-fluidsynth was given, but test for fluidsynth failed])
        ])
    ])
])

# [crispy] true-color rendering as a compile-time option
AC_ARG_ENABLE([truecolor],
AS_HELP_STRING([--enable-truecolor],
    [true-color rendering (experimental) @<:@default=no@:>@]))
AS_IF([test "x$enable_truecolor" = "xyes"], [
    AC_DEFINE([CRISPY_TRUECOLOR], [1], [true-color rendering])
])

# TODO: We currently link everything against libraries that don't need it.
# Use the specific library CFLAGS/LIBS variables instead of setting them here.
CFLAGS="$CFLAGS $SDL_CFLAGS ${SAMPLERATE_CFLAGS:-} ${PNG_CFLAGS:-} ${FLUIDSYNTH_CFLAGS:-} ${LIBZ_CFLAGS:-}"
LDFLAGS="$LDFLAGS $SDL_LIBS ${SAMPLERATE_LIBS:-} ${PNG_LIBS:-} ${FLUIDSYNTH_LIBS:-} ${LIBZ_LIBS:-}"
case "$host" in
  *-*-mingw* | *-*-cygwin* | *-*-msvc* )
    LDFLAGS="$LDFLAGS -lwinmm -lshlwapi"
    ;;
  *)
esac

AC_CHECK_LIB(m, log)

# [crispy] use stdlib's qsort() function for sorting the vissprites[] array
AC_CHECK_FUNCS(qsort)

AC_CHECK_HEADERS([dirent.h linux/kd.h dev/isa/spkrio.h dev/speaker/speaker.h])
AC_CHECK_FUNCS(mmap ioperm)
AC_CHECK_DECLS([strcasecmp, strncasecmp], [], [], [[#include <strings.h>]])

# OpenBSD I/O i386 library for I/O port access.
# (64 bit has the same thing with a different name!)

AC_CHECK_LIB(i386, i386_iopl)
AC_CHECK_LIB(amd64, amd64_iopl)

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

AC_ARG_ENABLE([werror], AS_HELP_STRING([--enable-werror], [Treat warnings as errors]))

AS_IF([test "x$enable_werror" = "xyes"], [
        CFLAGS="$CFLAGS -Werror"
])

AM_CONDITIONAL(HAVE_WINDRES, test "$WINDRES" != "")
AM_CONDITIONAL(HAVE_BASH_COMPLETION, [test "x$enable_bash_completion" != xno])
AM_CONDITIONAL(HAVE_DOC, [test "x$enable_doc" != xno])
AM_CONDITIONAL(HAVE_FONTS, [test "x$enable_fonts" != xno])
AM_CONDITIONAL(HAVE_ICONS, [test "x$enable_icons" != xno])
AM_CONDITIONAL(HAVE_ZPOOL, [test "x$enable_zpool" != xno])

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

AC_CONFIG_HEADERS(config.h:config.hin)

AC_SUBST(WINDOWS_RC_VERSION)
AC_SUBST(SDLMIXER_CFLAGS)
AC_SUBST(SDLMIXER_LIBS)
AC_SUBST(EM_LDFLAGS)

AC_SUBST(SDLNET_CFLAGS)
AC_SUBST(SDLNET_LIBS)

AC_SUBST(ac_aux_dir)

AC_SUBST(PACKAGE_SHORTNAME)
AC_SUBST(PACKAGE_SHORTDESC)
AC_SUBST(PACKAGE_COPYRIGHT)
AC_SUBST(PACKAGE_LICENSE)
AC_SUBST(PACKAGE_MAINTAINER)
AC_SUBST(PACKAGE_URL)
AC_SUBST(PACKAGE_RDNS)
AC_SUBST(PACKAGE_BUGREPORT)

dnl Shut up the datarootdir warnings.
AC_DEFUN([AC_DATAROOTDIR_CHECKED])

AS_IF([test "x$enable_emscripten" = "xyes"],
  EXEEXT=".html"
)

AC_CONFIG_FILES([
Makefile
man/Makefile
man/bash-completion/Makefile
man/bash-completion/doom.template
man/bash-completion/heretic.template
man/bash-completion/hexen.template
man/bash-completion/strife.template
opl/Makefile
opl/examples/Makefile
pcsound/Makefile
pkg/Makefile
pkg/config.make
pkg/osx/Info.plist
rpm.spec
data/Makefile
src/Makefile
src/doom-res.rc
src/Doom.metainfo.xml
src/Doom.desktop
src/Doom_Screensaver.desktop
src/doom/Makefile
src/heretic-res.rc
src/Heretic.metainfo.xml
src/Heretic.desktop
src/heretic/Makefile
src/hexen-res.rc
src/Hexen.metainfo.xml
src/Hexen.desktop
src/hexen/Makefile
src/setup-res.rc
src/setup/Makefile
src/setup/Setup.desktop
src/setup/setup-manifest.xml
src/strife-res.rc
src/Strife.metainfo.xml
src/Strife.desktop
src/strife/Makefile
textscreen/Makefile
textscreen/examples/Makefile
textscreen/fonts/Makefile
])
AC_OUTPUT
