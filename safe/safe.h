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
// Safe C functions
//

#ifndef SAFE_C_FUNCTIONS_H
#define SAFE_C_FUNCTIONS_H

#include <stdarg.h>
#include <stddef.h>

typedef void (*x_abort_function_t)(const char *s, ...);

#ifdef __GNUC__

#define X_PRINTF_ATTR(fmt, first) __attribute__((format(printf, fmt, first)))
#define X_PRINTF_ARG_ATTR(x) __attribute__((format_arg(x)))
#define X_NORETURN __attribute__((noreturn))

#else

#define X_PRINTF_ATTR(fmt, first)
#define X_PRINTF_ARG_ATTR(x)
#define X_NORETURN

#endif

void X_SetAbortFunction(x_abort_function_t fn);

// Use instead of strdup(); aborts on error:
char *X_StringDuplicate(const char *orig);

// Use instead of strcpy() or strncpy(); always includes a NUL terminator
int X_StringCopy(char *dest, const char *src, size_t dest_size);

// Use instead of strcat() or strncat(); always includes a NUL terminator
int X_StringConcat(char *dest, const char *src, size_t dest_size);

// Use instead of vsnprintf(); always includes a NUL terminator.
int X_vsnprintf(char *buf, size_t buf_len, const char *s, va_list args);

// Use instead of snprintf(); always includes a NUL terminator.
int X_snprintf(char *buf, size_t buf_len, const char *s, ...) X_PRINTF_ATTR(3, 4);

#define X_AllocArray(t, cnt) \
    ((t *) X_CheckedCalloc((cnt), sizeof(t)))
void *X_CheckedCalloc(size_t count, size_t size);

#define X_ReallocArray(ptr, t, cnt) \
    ((t *) X_CheckedRealloc((ptr), (cnt), sizeof(t)))
void *X_CheckedRealloc(void *ptr, size_t count, size_t size);

#define X_Alloc(t) X_AllocArray(t, 1)

#endif

