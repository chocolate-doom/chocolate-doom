
dnl
dnl SDL workaround autoconf macros, by Simon Howard.
dnl I release the contents of this file to the public domain.
dnl

dnl Macro to check if autoconf's compile tests have been broken by
dnl SDL.  Tries to build the simplest possible program, and if it
dnl fails, calls the given block.

AC_DEFUN([AC_CHECK_SDL_BREAKAGE], [
    AC_LINK_IFELSE(AC_LANG_PROGRAM([], []), [], [
        $1
    ])
])

dnl Macro to work around SDL redefining main.  The provided block
dnl is run with main #defined to SDL_main via a compiler switch
dnl if autoconf tests are found to be broken.

AC_DEFUN([AC_SDL_MAIN_WORKAROUND], [
    sdl_workaround_saved_CFLAGS="$CFLAGS"

    AC_CHECK_SDL_BREAKAGE([
        CFLAGS="$CFLAGS -Dmain=SDL_main"
    ])

    AC_CHECK_SDL_BREAKAGE([
        AC_MSG_ERROR([Autoconf checks broken by SDL, and can't figure out how to fix them.])
    ])

    $1

    CFLAGS="$sdl_workaround_saved_CFLAGS"
])

