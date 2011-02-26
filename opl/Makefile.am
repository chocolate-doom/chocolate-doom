
AM_CFLAGS=@SDLMIXER_CFLAGS@

SUBDIRS = . examples

noinst_LIBRARIES=libopl.a

libopl_a_SOURCES =                                \
                            opl_internal.h        \
        opl.c               opl.h                 \
        opl_linux.c                               \
        opl_obsd.c                                \
        opl_queue.c         opl_queue.h           \
        opl_sdl.c                                 \
        opl_timer.c         opl_timer.h           \
        opl_win32.c                               \
        ioperm_sys.c        ioperm_sys.h          \
        dbopl.c             dbopl.h

