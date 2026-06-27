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
//     Minimal SDL event compatibility shim for the Sysop-64 build.
//

#ifndef SDL_H
#define SDL_H

// Minimal SDL compatibility for the sysop-64 build.
// The sysop backend does not compile SDL input, but Chocolate's shared
// i_input.h exposes SDL_Event pointers in a couple of prototypes.
typedef struct SDL_Event
{
    int type;
} SDL_Event;

#endif // SDL_H
