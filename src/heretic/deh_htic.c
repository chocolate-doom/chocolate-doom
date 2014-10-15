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
// Top-level dehacked definitions for Heretic dehacked (HHE).
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "deh_defs.h"
#include "deh_main.h"
#include "deh_htic.h"
#include "info.h"
#include "m_argv.h"

char *deh_signatures[] =
{
    "Patch File for HHE v1.0",
    "Patch File for HHE v1.1",
    NULL
};

static char *hhe_versions[] =
{
    "1.0", "1.2", "1.3"
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

static void SetHHEVersionByName(char *name)
{
    int i;

    for (i=0; i<arrlen(hhe_versions); ++i)
    {
        if (!strcmp(hhe_versions[i], name))
        {
            deh_hhe_version = i;
            return;
        }
    }

    fprintf(stderr, "Unknown Heretic version: %s\n", name);
    fprintf(stderr, "Valid versions:\n");

    for (i=0; i<arrlen(hhe_versions); ++i)
    {
        fprintf(stderr, "\t%s\n", hhe_versions[i]);
    }
}

// Initialize Heretic(HHE)-specific dehacked bits.

void DEH_HereticInit(void)
{
    int i;

    //!
    // @arg <version>
    // @category mod
    //
    // Select the Heretic version number that was used to generate the
    // HHE patch to be loaded.  Patches for each of the Vanilla
    // Heretic versions (1.0, 1.2, 1.3) can be loaded, but the correct
    // version number must be specified.

    i = M_CheckParm("-hhever");

    if (i > 0)
    {
        SetHHEVersionByName(myargv[i + 1]);
    }

    // For v1.0 patches, we must apply a slight change to the states[]
    // table.  The table was changed between 1.0 and 1.3 to add two extra
    // frames to the player "burning death" animation.
    //
    // If we are using a v1.0 patch, we must change the table to cut
    // these out again.

    if (deh_hhe_version < deh_hhe_1_2)
    {
        states[S_PLAY_FDTH18].nextstate = S_NULL;
    }
}

int DEH_MapHereticThingType(int type)
{
    // Heretic 1.0 had an extra entry in the mobjinfo table that was removed
    // in later versions. This has been added back into the table for
    // compatibility. However, it also means that if we're loading a patch
    // for a later version, we need to translate to the index used internally.

    if (deh_hhe_version > deh_hhe_1_0)
    {
        if (type >= MT_PHOENIXFX_REMOVED)
        {
            ++type;
        }
    }

    return type;
}

int DEH_MapHereticFrameNumber(int frame)
{
    if (deh_hhe_version < deh_hhe_1_2)
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

void DEH_SuggestHereticVersion(deh_hhe_version_t version)
{
    fprintf(stderr,
    "\n"
    "This patch may be for version %s. You are currently running in\n"
    "Heretic %s mode. For %s mode, add this to your command line:\n"
    "\n"
    "\t-hhever %s\n"
    "\n",
    hhe_versions[version],
    hhe_versions[deh_hhe_version],
    hhe_versions[version],
    hhe_versions[version]);
}

