//
// Copyright(C) 2020 Simon Howard
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
//
// This is a set of safe C functions that are intended to eliminate common
// errors; in particular they replace or wrap the following functions from
// the C standard library:
//
// Standard function     Instead use
//    malloc()        |  X_Alloc() or X_AllocArray()
//    realloc()       |  X_ReallocArray()
//    sprintf         |  X_snprintf()
//    snprintf        |  X_snprintf()
//    vsprintf        |  X_vsnprintf()
//    vsnprintf       |  X_vsnprintf()
//    strcpy()        |  X_StringCopy()
//    strncpy()       |  X_StringCopy()
//    strcat()        |  X_StringConcat()
//    strncat()       |  X_StringConcat()
//    strdup()        |  X_StringDuplicate()
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include <inttypes.h>

#include "safe.h"

static void ErrorAbort(const char *s, ...)
{
    va_list args;

    va_start(args, s);
    vfprintf(stderr, s, args);
    va_end(args);
    exit(1);
}

static x_abort_function_t abort_function = ErrorAbort;

void X_SetAbortFunction(x_abort_function_t fn)
{
    abort_function = fn;
}

// Safe version of strdup() that checks the string was successfully
// allocated.
char *X_StringDuplicate(const char *orig)
{
    char *result;

    result = strdup(orig);

    if (result == NULL)
    {
        abort_function("Failed allocation; strdup(%" PRIuPTR " bytes)=NULL\n",
                       strlen(orig));
    }

    return result;
}

// Safe string copy function that works like OpenBSD's strlcpy().
// Returns true if the string was not truncated.
int X_StringCopy(char *dest, const char *src, size_t dest_size)
{
    size_t len;

    if (dest_size >= 1)
    {
        dest[dest_size - 1] = '\0';
        strncpy(dest, src, dest_size - 1);
    }
    else
    {
        return 0;
    }

    len = strlen(dest);
    return src[len] == '\0';
}

// Safe string concat function that works like OpenBSD's strlcat().
// Returns true if string not truncated.
int X_StringConcat(char *dest, const char *src, size_t dest_size)
{
    size_t offset;

    offset = strlen(dest);
    if (offset > dest_size)
    {
        offset = dest_size;
    }

    return X_StringCopy(dest + offset, src, dest_size - offset);
}

// On Windows, vsnprintf() is _vsnprintf().
#ifdef _WIN32
#if _MSC_VER < 1400 /* not needed for Visual Studio 2008 */
#define vsnprintf _vsnprintf
#endif
#endif

// Safe, portable vsnprintf().
int X_vsnprintf(char *buf, size_t buf_len, const char *s, va_list args)
{
    int result;

    if (buf_len < 1)
    {
        return 0;
    }

    // Windows (and other OSes?) has a vsnprintf() that doesn't always
    // append a trailing \0. So we must do it, and write into a buffer
    // that is one byte shorter; otherwise this function is unsafe.
    result = vsnprintf(buf, buf_len, s, args);

    // If truncated, change the final char in the buffer to a \0.
    // A negative result indicates a truncated buffer on Windows.
    if (result < 0 || result >= buf_len)
    {
        buf[buf_len - 1] = '\0';
        result = buf_len - 1;
    }

    return result;
}

// Safe, portable snprintf().
int X_snprintf(char *buf, size_t buf_len, const char *s, ...)
{
    va_list args;
    int result;
    va_start(args, s);
    result = X_vsnprintf(buf, buf_len, s, args);
    va_end(args);
    return result;
}

void *X_CheckedCalloc(size_t count, size_t size)
{
    void *result = calloc(count, size);

    if (result == NULL)
    {
        abort_function("Failed allocation; "
                       "calloc(%" PRIuPTR ", %" PRIuPTR ")=NULL\n",
                       count, size);
    }

    return result;
}

void *X_CheckedRealloc(void *ptr, size_t count, size_t size)
{
    size_t new_size;
    void *result;

    new_size = count * size;
    result = realloc(ptr, new_size);
    if (result == NULL)
    {
        abort_function("Failed allocation; realloc(%p, %" PRIuPTR ")=NULL\n",
                       ptr, new_size);
    }

    return result;
}

