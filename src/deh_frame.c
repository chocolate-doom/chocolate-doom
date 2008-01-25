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
// Parses "Frame" sections in dehacked files
//
//-----------------------------------------------------------------------------

#include <stdlib.h>

#include "doomdef.h"
#include "doomtype.h"
#include "d_items.h"
#include "info.h"

#include "deh_defs.h"
#include "deh_io.h"
#include "deh_main.h"
#include "deh_mapping.h"

DEH_BEGIN_MAPPING(state_mapping, state_t)
  DEH_MAPPING("Sprite number",    sprite)
  DEH_MAPPING("Sprite subnumber", frame)
  DEH_MAPPING("Duration",         tics)
  DEH_MAPPING("Next frame",       nextstate)
  DEH_MAPPING("Unknown 1",        misc1)
  DEH_MAPPING("Unknown 2",        misc2)
  DEH_UNSUPPORTED_MAPPING("Codep frame")
DEH_END_MAPPING

static void *DEH_FrameStart(deh_context_t *context, char *line)
{
    int frame_number = 0;
    state_t *state;
    
    if (sscanf(line, "Frame %i", &frame_number) != 1)
    {
        DEH_Warning(context, "Parse error on section start");
        return NULL;
    }
    
    if (frame_number < 0 || frame_number >= NUMSTATES)
    {
        DEH_Warning(context, "Invalid frame number: %i", frame_number);
        return NULL;
    }

    if (frame_number >= DEH_VANILLA_NUMSTATES) 
    {
        DEH_Warning(context, "Attempt to modify frame %i: this will cause "
                             "problems in Vanilla dehacked.", frame_number);
    }

    state = &states[frame_number];

    return state;
}

// Simulate a frame overflow: Doom has 967 frames in the states[] array, but
// DOS dehacked internally only allocates memory for 966.  As a result, 
// attempts to set frame 966 (the last frame) will overflow the dehacked
// array and overwrite the weaponinfo[] array instead.
//
// This is noticable in Batman Doom where it is impossible to switch weapons
// away from the fist once selected.

static void DEH_FrameOverflow(deh_context_t *context, char *varname, int value)
{
    if (!strcasecmp(varname, "Duration"))
    {
        weaponinfo[0].ammo = value;
    }
    else if (!strcasecmp(varname, "Codep frame")) 
    {
        weaponinfo[0].upstate = value;
    }
    else if (!strcasecmp(varname, "Next frame")) 
    {
        weaponinfo[0].downstate = value;
    }
    else if (!strcasecmp(varname, "Unknown 1"))
    {
        weaponinfo[0].readystate = value;
    }
    else if (!strcasecmp(varname, "Unknown 2"))
    {
        weaponinfo[0].atkstate = value;
    }
    else
    {
        DEH_Error(context, "Unable to simulate frame overflow: field '%s'",
                  varname);
    }
}

static void DEH_FrameParseLine(deh_context_t *context, char *line, void *tag)
{
    state_t *state;
    char *variable_name, *value;
    int ivalue;
    
    if (tag == NULL)
       return;

    state = (state_t *) tag;

    // Parse the assignment

    if (!DEH_ParseAssignment(line, &variable_name, &value))
    {
        // Failed to parse

        DEH_Warning(context, "Failed to parse assignment");
        return;
    }
    
    // all values are integers

    ivalue = atoi(value);
    
    if (state == &states[NUMSTATES - 1])
    {
        DEH_FrameOverflow(context, variable_name, ivalue);
    }
    else
    {
        // set the appropriate field

        DEH_SetMapping(context, &state_mapping, state, variable_name, ivalue);
    }
}

static void DEH_FrameMD5Sum(md5_context_t *context)
{
    int i;

    for (i=0; i<NUMSTATES; ++i)
    {
        DEH_StructMD5Sum(context, &state_mapping, &states[i]);
    }
}

deh_section_t deh_section_frame =
{
    "Frame",
    NULL,
    DEH_FrameStart,
    DEH_FrameParseLine,
    NULL,
    DEH_FrameMD5Sum,
};

