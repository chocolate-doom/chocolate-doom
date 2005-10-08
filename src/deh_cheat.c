// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: deh_cheat.c 175 2005-10-08 20:54:16Z fraggle $
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
// $Log$
// Revision 1.2  2005/10/08 20:54:16  fraggle
// Proper dehacked error/warning framework.  Catch a load more errors.
//
// Revision 1.1  2005/10/04 21:41:42  fraggle
// Rewrite cheats code.  Add dehacked cheat replacement.
//
// Revision 1.2  2005/10/03 11:08:16  fraggle
// Replace end of section functions with NULLs as they arent currently being
// used for anything.
//
// Revision 1.1  2005/10/02 23:49:01  fraggle
// The beginnings of dehacked support
//
//
//-----------------------------------------------------------------------------
//
// Parses "Cheat" sections in dehacked files
//
//-----------------------------------------------------------------------------

#include "doomdef.h"
#include "doomtype.h"

#include "deh_defs.h"
#include "deh_io.h"
#include "deh_main.h"
#include "am_map.h"
#include "st_stuff.h"

typedef struct 
{
    char *name;
    cheatseq_t *seq;
} deh_cheat_t;

static deh_cheat_t allcheats[] =
{
    {"Change music",        &cheat_mus },
    {"Chainsaw",            &cheat_choppers },
    {"God mode",            &cheat_god },
    {"Ammo & Keys",         &cheat_ammo },
    {"Ammo",                &cheat_ammonokey },
    {"No Clipping 1",       &cheat_noclip },
    {"No Clipping 2",       &cheat_commercial_noclip },
    {"Invincibility",       &cheat_powerup[0] },
    {"Berserk",             &cheat_powerup[1] },
    {"Invisibility",        &cheat_powerup[2] },
    {"Radiation Suit",      &cheat_powerup[3] },
    {"Auto-map",            &cheat_powerup[4] },
    {"Lite-Amp Goggles",    &cheat_powerup[5] },
    {"BEHOLD menu",         &cheat_powerup[6] },
    {"Level Warp",          &cheat_clev },
    {"Player Position",     &cheat_mypos },
    {"Map cheat",           &cheat_amap },
};

static deh_cheat_t *FindCheatByName(char *name)
{
    int i;
    
    for (i=0; i<sizeof(allcheats) / sizeof(*allcheats); ++i)
    {
        if (!strcasecmp(allcheats[i].name, name))
            return &allcheats[i];
    }

    return NULL;
}

static void *DEH_CheatStart(deh_context_t *context, char *line)
{
    return NULL;
}

static void DEH_CheatParseLine(deh_context_t *context, char *line, void *tag)
{
    deh_cheat_t *cheat;
    char *variable_name;
    unsigned char *value;
    int i;

    if (!DEH_ParseAssignment(line, &variable_name, (char **) &value))
    {
        // Failed to parse

        DEH_Warning(context, "Failed to parse assignment");
        return;
    }

    cheat = FindCheatByName(variable_name);

    if (cheat == NULL)
    {
        DEH_Warning(context, "Unknown cheat '%s'", variable_name);
        return;
    }

    // write the value into the cheat sequence

    for (i=0; 
         i<cheat->seq->sequence_len && value[i] != 0 && value[i] != 0xff; 
         ++i)
        cheat->seq->sequence[i] = value[i];

    cheat->seq->sequence[i] = '\0';
}

deh_section_t deh_section_cheat =
{
    "Cheat",
    NULL,
    DEH_CheatStart,
    DEH_CheatParseLine,
    NULL,
};

