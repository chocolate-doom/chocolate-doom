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
//     Minimal SDL standard-library compatibility shim for the Sysop-64 build.
//

#ifndef SYSOP_SDL_STDINC_COMPAT_H
#define SYSOP_SDL_STDINC_COMPAT_H

#include <stdlib.h>

// Chocolate Doom only needs SDL_qsort from SDL_stdinc here, so map it directly
// to the C runtime sorter.
#define SDL_qsort qsort

#endif
