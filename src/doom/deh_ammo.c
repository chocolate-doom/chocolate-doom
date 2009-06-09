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
// Parses "Ammo" sections in dehacked files
//
//-----------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "doomdef.h"
#include "doomtype.h"
#include "deh_defs.h"
#include "deh_io.h"
#include "deh_main.h"
#include "p_local.h"

static void *DEH_AmmoStart(deh_context_t *context, char *line)
{
    int ammo_number = 0;

    if (sscanf(line, "Ammo %i", &ammo_number) != 1)
    {
        DEH_Warning(context, "Parse error on section start");
        return NULL;
    }

    if (ammo_number < 0 || ammo_number >= NUMAMMO)
    {
        DEH_Warning(context, "Invalid ammo number: %i", ammo_number);
        return NULL;
    }
    
    return &maxammo[ammo_number];
}

static void DEH_AmmoParseLine(deh_context_t *context, char *line, void *tag)
{
    char *variable_name, *value;
    int ivalue;
    int ammo_number;

    if (tag == NULL)
        return;

    ammo_number = ((int *) tag) - maxammo;

    // Parse the assignment

    if (!DEH_ParseAssignment(line, &variable_name, &value))
    {
        // Failed to parse

        DEH_Warning(context, "Failed to parse assignment");
        return;
    }

    ivalue = atoi(value);

    // maxammo

    if (!strcasecmp(variable_name, "Per ammo"))
        clipammo[ammo_number] = ivalue;
    else if (!strcasecmp(variable_name, "Max ammo"))
        maxammo[ammo_number] = ivalue;
    else
    {
        DEH_Warning(context, "Field named '%s' not found", variable_name);
    }
}

static void DEH_AmmoMD5Hash(md5_context_t *context)
{
    int i;

    for (i=0; i<NUMAMMO; ++i)
    {
        MD5_UpdateInt32(context, clipammo[i]);
        MD5_UpdateInt32(context, maxammo[i]);
    }
}

deh_section_t deh_section_ammo =
{
    "Ammo",
    NULL,
    DEH_AmmoStart,
    DEH_AmmoParseLine,
    NULL,
    DEH_AmmoMD5Hash,
};

