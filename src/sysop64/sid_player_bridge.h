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
//     Public C interface for SID playback used by the Sysop-64 backend.
//

#ifndef DOOM_SID_PLAYER_BRIDGE_H
#define DOOM_SID_PLAYER_BRIDGE_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct DoomSidWrite {
    uint16_t addr;
    uint8_t val;
} DoomSidWrite;

typedef struct DoomSidDigiWrite {
    uint16_t raster_line;
    uint8_t cycle;
    uint8_t val;
    uint16_t addr;
} DoomSidDigiWrite;

// Load a SID tune and initialize the bridge for PAL or NTSC playback.
int doom_sid_init(const char *path, int is_pal);
// Change PAL/NTSC timing without reloading the tune.
void doom_sid_set_machine(int is_pal);
// Return nonzero when a SID tune is loaded.
int doom_sid_is_loaded(void);
// Generate one frame of SID register writes for the C backend to poke.
int doom_sid_play_frame(DoomSidWrite *writes,
                        int max_writes,
                        DoomSidDigiWrite *digi_writes,
                        int max_digi_writes,
                        int *digi_count);
// Stop playback and release the loaded tune state.
void doom_sid_shutdown(void);
// Return the loaded SID title, or an empty string when none was available.
const char *doom_sid_title(void);

#ifdef __cplusplus
}
#endif

#endif
