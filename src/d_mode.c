//
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
//
// DESCRIPTION:
//   Functions and definitions relating to the game type and operational
//   mode.
//

#include "doomtype.h"
#include "d_mode.h"

// Valid game mode/mission combinations, with the number of
// episodes/maps for each.

static struct
{
    GameMission_t mission;
    GameMode_t mode;
    int episode;
    int map;
} valid_modes[] = {
    { pack_chex, retail,     1, 5 },
    { doom,      shareware,  1, 9 },
    { doom,      registered, 3, 9 },
    { doom,      retail,     5, 9 }, // SIGIL
    { doom2,     commercial, 1, 32 },
    { pack_tnt,  commercial, 1, 32 },
    { pack_plut, commercial, 1, 32 },
    { pack_hacx, commercial, 1, 32 },
    { heretic,   shareware,  1, 9 },
    { heretic,   registered, 3, 9 },
    { heretic,   retail,     5, 9 },
    { hexen,     commercial, 1, 60 },
    { strife,    commercial, 1, 34 },
};

// Check that a gamemode+gamemission received over the network is valid.

boolean D_ValidGameMode(GameMission_t mission, GameMode_t mode)
{
    int i;

    for (i=0; i<arrlen(valid_modes); ++i)
    {
        if (valid_modes[i].mode == mode && valid_modes[i].mission == mission)
        {
            return true;
        }
    }

    return false;
}

boolean D_ValidEpisodeMap(GameMission_t mission, GameMode_t mode,
                          int episode, int map)
{
    int i;

    // Hacks for Heretic secret episodes

    if (mission == heretic)
    {
        if (mode == retail && episode == 6)
        {
            return map >= 1 && map <= 3;
        }
        else if (mode == registered && episode == 4)
        {
            return map == 1;
        }
    }

    // Find the table entry for this mission/mode combination.

    for (i=0; i<arrlen(valid_modes); ++i) 
    {
        if (mission == valid_modes[i].mission
         && mode == valid_modes[i].mode)
        {
            return episode >= 1 && episode <= valid_modes[i].episode
                && map >= 1 && map <= valid_modes[i].map;
        }
    }

    // Unknown mode/mission combination

    return false;
}

// Get the number of valid episodes for the specified mission/mode.

int D_GetNumEpisodes(GameMission_t mission, GameMode_t mode)
{
    int episode;

    episode = 1;

    while (D_ValidEpisodeMap(mission, mode, episode, 1))
    {
        ++episode;
    }

    return episode - 1;
}

// Table of valid versions

static struct {
    GameMission_t mission;
    NetGameVersion_t version;
} valid_versions[] = {
    { doom,     net_doom_1_0 },
    { doom,     net_doom_1_1 },
    { doom,     net_doom_1_2 },
    { doom,     net_doom_1_666 },
    { doom,     net_doom_1_7 },
    { doom,     net_doom_1_8 },
    { doom,     net_doom_1_9 },
    { doom,     net_hacx },
    { doom,     net_ultimate },
    { doom,     net_final },
    { doom,     net_final2 },
    { doom,     net_chex },
    { heretic,  net_heretic_1_3 },
    { hexen,    net_hexen_1_1 },
    { strife,   net_strife_1_2 },
    { strife,   net_strife_1_31 },
};

boolean D_ValidNetGameVersion(GameMission_t mission, NetGameVersion_t version)
{
    int i;

    // All Doom variants can use the Doom versions.

    if (mission == doom2 || mission == pack_plut || mission == pack_tnt
     || mission == pack_hacx || mission == pack_chex)
    {
        mission = doom;
    }

    for (i=0; i<arrlen(valid_versions); ++i) 
    {
        if (valid_versions[i].mission == mission 
         && valid_versions[i].version == version)
        {
            return true;
        }
    }

    return false;
}

// Order of these is the same as in GameVersion_t
const NetGameVersion_t game_to_net_version_table[] =
{
    net_doom_1_0,
    net_doom_1_1,
    net_doom_1_2,
    net_doom_1_666,
    net_doom_1_7,
    net_doom_1_8,
    net_doom_1_9,
    net_hacx,
    net_ultimate,
    net_final,
    net_final2,
    net_chex,

    net_heretic_1_3,

    net_hexen_1_1,
    net_strife_1_2,
    net_strife_1_31
};

NetGameVersion_t D_NetGameVersion(int gameversion)
{
    return game_to_net_version_table[gameversion];
}

// Does this mission type use ExMy form, rather than MAPxy form?

boolean D_IsEpisodeMap(GameMission_t mission)
{
    switch (mission)
    {
        case doom:
        case heretic:
        case pack_chex:
            return true;

        case none:
        case hexen:
        case doom2:
        case pack_hacx:
        case pack_tnt:
        case pack_plut:
        case strife:
        default:
            return false;
    }
}

char *D_GameMissionString(GameMission_t mission)
{
    switch (mission)
    {
        case none:
        default:
            return "none";
        case doom:
            return "doom";
        case doom2:
            return "doom2";
        case pack_tnt:
            return "tnt";
        case pack_plut:
            return "plutonia";
        case pack_hacx:
            return "hacx";
        case pack_chex:
            return "chex";
        case heretic:
            return "heretic";
        case hexen:
            return "hexen";
        case strife:
            return "strife";
    }
}

