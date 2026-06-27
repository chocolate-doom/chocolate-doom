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
//     Minimal SDL endian compatibility shim for the Sysop-64 build.
//

#ifndef SYSOP_SDL_ENDIAN_COMPAT_H
#define SYSOP_SDL_ENDIAN_COMPAT_H

#include <stdint.h>

#define SDL_LIL_ENDIAN 1234
#define SDL_BIG_ENDIAN 4321

#if defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#define SDL_BYTEORDER SDL_BIG_ENDIAN
#else
#define SDL_BYTEORDER SDL_LIL_ENDIAN
#endif

// Swap a 16-bit value for Chocolate Doom helpers that normally rely on SDL.
static inline uint16_t SDL_Swap16(uint16_t x)
{
    return (uint16_t)((x << 8) | (x >> 8));
}

// Swap a 32-bit value for little-endian WAD and savegame helpers.
static inline uint32_t SDL_Swap32(uint32_t x)
{
    return ((x & 0x000000ffU) << 24)
         | ((x & 0x0000ff00U) << 8)
         | ((x & 0x00ff0000U) >> 8)
         | ((x & 0xff000000U) >> 24);
}

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
#define SDL_SwapLE16(X) SDL_Swap16((uint16_t)(X))
#define SDL_SwapLE32(X) SDL_Swap32((uint32_t)(X))
#else
#define SDL_SwapLE16(X) ((uint16_t)(X))
#define SDL_SwapLE32(X) ((uint32_t)(X))
#endif

#endif
