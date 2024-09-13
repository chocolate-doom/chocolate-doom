//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2005-2014 Simon Howard
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
// DESCRIPTION:
//      Miscellaneous.
//    


#ifndef __M_MISC__
#define __M_MISC__

#include <stdio.h>
#include <stdarg.h>
#include <sys/stat.h>

#include "doomtype.h"

#ifdef _WIN32
wchar_t *M_ConvertUtf8ToWide(const char *str);
char *M_ConvertWideToUtf8(const wchar_t *wstr);
#endif

FILE *M_fopen(const char *filename, const char *mode);
int M_remove(const char *path);
int M_rename(const char *oldname, const char *newname);
int M_stat(const char *path, struct stat *buf);
char *M_getenv(const char *name);
boolean M_WriteFile(const char *name, const void *source, int length);
int M_ReadFile(const char *name, byte **buffer);
void M_MakeDirectory(const char *dir);
char *M_TempFile(const char *s);
boolean M_FileExists(const char *file);
char *M_FileCaseExists(const char *file);
long M_FileLength(FILE *handle);
boolean M_StrToInt(const char *str, int *result);
char *M_DirName(const char *path);
const char *M_BaseName(const char *path);
void M_ExtractFileBase(const char *path, char *dest);
void M_ForceUppercase(char *text);
void M_ForceLowercase(char *text);
const char *M_StrCaseStr(const char *haystack, const char *needle);
char *M_StringDuplicate(const char *orig);
boolean M_StringCopy(char *dest, const char *src, size_t dest_size);
boolean M_StringConcat(char *dest, const char *src, size_t dest_size);
char *M_StringReplace(const char *haystack, const char *needle,
                      const char *replacement);
char *M_StringJoin(const char *s, ...);
boolean M_StringStartsWith(const char *s, const char *prefix);
boolean M_StringEndsWith(const char *s, const char *suffix);
int M_vsnprintf(char *buf, size_t buf_len, const char *s, va_list args);
int M_snprintf(char *buf, size_t buf_len, const char *s, ...) PRINTF_ATTR(3, 4);
void M_NormalizeSlashes(char *str);


// debugging code to check there are no loops in a linked list
// disabled unless explicitly requested
#ifdef DEBUG_LINKED_LISTS


#define LINKED_LIST_CHECK_NO_CYCLE(list_type, list, next_member)  \
    do                                                            \
    {                                                             \
        if (list != NULL) {                                       \
            list_type *slow, *fast;                               \
            slow = list;                                          \
            fast = list->next_member;                             \
            while (fast) {                                        \
                if (!fast->next_member) {                         \
                    break;                                        \
                }                                                 \
                fast = fast->next_member->next_member;            \
                slow = slow->next_member;                         \
                if (slow == fast) {                               \
                    fprintf(stderr, "loop in linked list " # list " in %s:%d", __FILE__, __LINE__); \
                    __builtin_trap();                             \
                }                                                 \
            }                                                     \
        }                                                         \
    } while (0)                                                   \



#else  // DEBUG_LINKED_LISTS


#define LINKED_LIST_CHECK_NO_CYCLE(list_type, list, next_member)  \
    do                                                            \
    {                                                             \
    } while (0)                                                   \


#endif  // DEBUG_LINKED_LISTS


#endif

