
noinst_LIBRARIES=libc_wince.a

if WINDOWS_CE

libc_wince_a_SOURCES =                            \
        libc_wince.h                              \
        env.c            env.h                    \
	errno.c          errno.h                  \
        fileops.c        fileops.h

else

libc_wince_a_SOURCES = dummy.c

endif

