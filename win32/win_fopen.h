//
//  Copyright(C) 2021 Roman Fomin
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
// DESCRIPTION:
//      unicode paths for fopen() on Windows

#ifndef __WIN_D_FOPEN__
#define __WIN_D_FOPEN__

#ifdef _WIN32
#include <stdio.h>
#include <sys/stat.h>

FILE* D_fopen(const char *filename, const char *mode);
int D_remove(const char *path);
int D_rename(const char *oldname, const char *newname);
int D_stat(const char *path, struct stat *buffer);
int D_mkdir(const char *dirname);

#undef  fopen
#define fopen(n, m) D_fopen(n, m)

#undef  remove
#define remove(p) D_remove(p)

#undef  rename
#define rename(o, n) D_rename(o, n)

#undef  stat
#define stat(p, b) D_stat(p, b)

#undef  mkdir
#define mkdir(d) D_mkdir(d)
#endif

#endif
