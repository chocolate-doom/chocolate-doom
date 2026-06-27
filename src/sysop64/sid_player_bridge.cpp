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
//     C ABI bridge between the Sysop-64 Doom backend and the libsysop64 SID
//     player.
//

#include "sid_player_bridge.h"

#include "sysop_sid.h"

#include <algorithm>
#include <stdio.h>
#include <string>
#include <vector>

namespace {
SidPlayerHandle *g_sid_player = nullptr;
bool g_sid_loaded = false;
int g_sid_is_pal = 1;
std::string g_sid_title;

// Copy optional raster-timed digi writes from libsysop64 into the plain C
// buffer shape understood by the Sysop backend.
void copy_digi_writes(const std::vector<SidDigiWrite>& in,
                      DoomSidDigiWrite *out,
                      int max_out,
                      int *out_count) {
    int count = (int)in.size();
    if (out_count) {
        *out_count = count;
    }

    if (!out || max_out <= 0) {
        return;
    }

    count = std::min(count, max_out);
    for (int i = 0; i < count; ++i) {
        out[i].raster_line = in[i].raster_line;
        out[i].cycle = in[i].cycle;
        out[i].val = in[i].val;
        out[i].addr = in[i].addr;
    }
}

// Stop and destroy the library player so reloads and shutdowns leave no stale
// SID state behind.
void destroy_player(void) {
    if (g_sid_player) {
        sid_player_stop(g_sid_player);
        sid_player_destroy(g_sid_player);
        g_sid_player = nullptr;
    }

    g_sid_loaded = false;
    g_sid_title.clear();
}
}

// Load a SID tune, configure it for the detected PAL/NTSC machine, and prepare
// the player for repeated frame playback.
int doom_sid_init(const char *path, int is_pal) {
    char title[33];

    if (!path || !path[0]) {
        return 0;
    }

    destroy_player();

    g_sid_is_pal = is_pal != 0;
    g_sid_player = sid_player_create(g_sid_is_pal);
    if (!g_sid_player) {
        printf("[SID] Could not create SID player; music disabled.\n");
        return 0;
    }

    sid_player_set_machine_type(g_sid_player, g_sid_is_pal);
    sid_player_set_repeat(g_sid_player, -1);
    sid_player_set_duration_seconds(g_sid_player, -1.0f);

    // The Doom backend currently plays regular SID register streams. The
    // library can produce raster-timed digi writes too, but those need to be
    // merged with the bitmap scheduler before enabling them here.
    sid_player_set_digi_enabled(g_sid_player, 0);

    if (!sid_player_load(g_sid_player, path, 0)) {
        printf("[SID] %s not found; music disabled.\n", path);
        destroy_player();
        return 0;
    }

    title[0] = '\0';
    sid_player_get_title(g_sid_player, title, sizeof(title));
    g_sid_title = title;

    if (g_sid_title.empty()) {
        printf("[SID] Loaded %s\n", path);
    } else {
        printf("[SID] Loaded %s: %s\n", path, g_sid_title.c_str());
    }

    g_sid_loaded = true;
    return 1;
}

// Update the emulated SID machine timing after the Sysop backend identifies the
// real VIC model.
void doom_sid_set_machine(int is_pal) {
    g_sid_is_pal = is_pal != 0;

    if (g_sid_player) {
        sid_player_set_machine_type(g_sid_player, g_sid_is_pal);
    }
}

// Report whether a tune is currently loaded and ready to produce register
// writes.
int doom_sid_is_loaded(void) {
    return g_sid_loaded ? 1 : 0;
}

// Produce one SID player frame as a bounded list of C64 SID register writes,
// with optional digi writes copied out for future scheduler integration.
int doom_sid_play_frame(DoomSidWrite *writes,
                        int max_writes,
                        DoomSidDigiWrite *digi_writes,
                        int max_digi_writes,
                        int *digi_count) {
    int count = 0;

    if (digi_count) {
        *digi_count = 0;
    }

    if (!g_sid_loaded || !g_sid_player || !writes || max_writes <= 0) {
        return 0;
    }

    std::vector<SidPoke> out_pokes(max_writes);

    if (digi_writes && max_digi_writes > 0) {
        int out_digi_count = 0;
        std::vector<SidDigiWrite> out_digi(max_digi_writes);

        sid_player_play_frame_ex(g_sid_player,
                                 out_pokes.data(), max_writes, &count,
                                 out_digi.data(), max_digi_writes, &out_digi_count);

        out_digi.resize(std::max(0, std::min(out_digi_count, max_digi_writes)));
        copy_digi_writes(out_digi, digi_writes, max_digi_writes, digi_count);
    } else {
        sid_player_play_frame(g_sid_player, out_pokes.data(), max_writes, &count);
    }

    count = std::max(0, std::min(count, max_writes));
    for (int i = 0; i < count; ++i) {
        writes[i].addr = out_pokes[i].addr;
        writes[i].val = out_pokes[i].val;
    }

    return count;
}

// Stop playback and mark the bridge as unloaded.
void doom_sid_shutdown(void) {
    destroy_player();
}

// Return the title string reported by the loaded SID file, if any.
const char *doom_sid_title(void) {
    return g_sid_title.c_str();
}
