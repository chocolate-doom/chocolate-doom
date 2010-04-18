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
//-----------------------------------------------------------------------------
//
// Top-level dehacked definitions for Heretic dehacked (HHE).
//
//-----------------------------------------------------------------------------

#include <stdlib.h>
#include "deh_defs.h"
#include "deh_main.h"
#include "deh_htic.h"
#include "info.h"

char *deh_signatures[] =
{
    "Patch File for HHE v1.0",
    "Patch File for HHE v1.1",
    NULL
};

// Version number for patches.

deh_hhe_version_t deh_hhe_version = deh_hhe_1_0;

// deh_ammo.c:
extern deh_section_t deh_section_ammo;
// deh_frame.c:
extern deh_section_t deh_section_frame;
// deh_ptr.c:
extern deh_section_t deh_section_pointer;
// deh_sound.c
extern deh_section_t deh_section_sound;
// deh_htext.c:
extern deh_section_t deh_section_heretic_text;
// deh_thing.c:
extern deh_section_t deh_section_thing;
// deh_weapon.c:
extern deh_section_t deh_section_weapon;

//
// List of section types:
//

deh_section_t *deh_section_types[] =
{
    &deh_section_ammo,
    &deh_section_frame,
//    &deh_section_pointer, TODO
    &deh_section_sound,
    &deh_section_heretic_text,
    &deh_section_thing,
    &deh_section_weapon,
    NULL
};

int DEH_MapHereticFrameNumber(int frame)
{
    if (deh_hhe_version < deh_hhe_1_0)
    {
        // Between Heretic 1.0 and 1.2, two new frames
        // were added to the "states" table, to extend the "flame death"
        // animation displayed when the player is killed by fire.  Therefore,
        // we must map Heretic 1.0 frame numbers to corresponding indexes
        // for our state table.

        if (frame >= S_PLAY_FDTH19)
        {
            frame = (frame - S_PLAY_FDTH19) + S_BLOODYSKULL1;
        }
    }
    else
    {
        // After Heretic 1.2, three unused frames were removed from the
        // states table, unused phoenix rod frames.  Our state table includes
        // these missing states for backwards compatibility.  We must therefore
        // adjust frame numbers for v1.2/v1.3 to corresponding indexes for
        // our state table.

        if (frame >= S_PHOENIXFXIX_1)
        {
            frame = (frame - S_PHOENIXFXIX_1) + S_PHOENIXPUFF1;
        }
    }

    return frame;
}

