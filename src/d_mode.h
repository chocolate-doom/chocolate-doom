// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// Copyright(C) 1993-1996 Id Software, Inc.
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
// DESCRIPTION:
//  Internally used data structures for virtually everything,
//   lots of other stuff.
//
//-----------------------------------------------------------------------------

#ifndef __D_MODE__
#define __D_MODE__

#include "doomtype.h"

// The "mission" controls what game we are playing.

typedef enum
{
    doom,            // Doom 1
    doom2,           // Doom 2
    pack_tnt,        // Final Doom: TNT: Evilution
    pack_plut,       // Final Doom: The Plutonia Experiment
    heretic,         // Heretic 
    hexen,           // Hexen

    none
} GameMission_t;

// The "mode" allows more accurate specification of the game mode we are
// in: eg. shareware vs. registered.  So doom1.wad and doom.wad are the
// same mission, but a different mode.

typedef enum
{
    shareware,       // Doom/Heretic shareware
    registered,      // Doom/Heretic registered
    commercial,      // Doom II/Hexen
    retail,          // Ultimate Doom
    indetermined     // Unknown.
} GameMode_t;

// What version are we emulating?

typedef enum
{
    exe_doom_1_9,    // Doom 1.9: used for shareware, registered and commercial
    exe_ultimate,    // Ultimate Doom (retail)
    exe_final,       // Final Doom
    exe_chex,        // Chex Quest executable (based on Final Doom)

    exe_heretic_1_3, // Heretic 1.3

    exe_hexen_1_1    // Hexen 1.1
} GameVersion_t;

// Skill level.

typedef enum
{
    sk_noitems = -1,        // the "-skill 0" hack
    sk_baby = 0,
    sk_easy,
    sk_medium,
    sk_hard,
    sk_nightmare
} skill_t;

boolean D_ValidGameMode(GameMission_t mission, GameMode_t mode);
boolean D_ValidGameVersion(GameMission_t mission, GameVersion_t version);

#endif /* #ifndef __D_MODE__ */

