//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2005-2014 Simon Howard
// Copyright(C) 2026 Sysop-64 contributors
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
//     Minimal SDL filesystem compatibility shim for the Sysop-64 build.
//

#ifndef SYSOP_SDL_FILESYSTEM_COMPAT_H
#define SYSOP_SDL_FILESYSTEM_COMPAT_H

#include <stdlib.h>
#include <string.h>

// Return the current directory as the preference path so Sysop builds keep
// config/save files beside the executable unless Chocolate Doom overrides them.
static inline char *SDL_GetPrefPath(const char *org, const char *app)
{
    const char *path = "./";
    size_t len = strlen(path) + 1;
    char *result = (char *)malloc(len);
    (void)org;
    (void)app;
    if (result != NULL) {
        memcpy(result, path, len);
    }
    return result;
}

// Match SDL's allocator API with the C runtime allocator used by this shim.
static inline void SDL_free(void *ptr)
{
    free(ptr);
}

#endif
