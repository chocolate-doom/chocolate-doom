//
// "Extension" implementation of errno.h for Windows CE.
//
// I (Simon Howard) release this file to the public domain.
//

#include <windows.h>

#include "errno.h"

// This should really be a thread-local variable.  Oh well.

static int my_errno;

int *_GetErrno()
{
    my_errno = GetLastError();
    return &my_errno;
}

