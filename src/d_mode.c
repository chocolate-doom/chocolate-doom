// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// Copyright(C) 2005 Simon Howard
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
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
// 02111-1307, USA.
//
// Common code shared between the client and server
//


#include "doomtype.h"
#include "d_mode.h"

// Table of valid game modes

static struct {
    GameMission_t mission;
    GameMode_t mode;
} valid_modes[] = {
    { doom,        shareware },
    { doom,        registered },
    { doom,        retail },
    { doom2,       commercial },
    { pack_tnt,    commercial },
    { pack_plut,   commercial },
    { heretic,     shareware },
    { heretic,     registered },
    { hexen,       commercial },
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

// Table of valid versions

static struct {
    GameMission_t mission;
    GameVersion_t version;
} valid_versions[] = {
    { doom,     exe_doom_1_9 },
    { doom,     exe_ultimate },
    { doom,     exe_chex },
    { doom,     exe_final },
    { heretic,  exe_heretic_1_3 },
    { hexen,    exe_hexen_1_1 },
};

boolean D_ValidGameVersion(GameMission_t mission, GameVersion_t version)
{
    int i;

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

