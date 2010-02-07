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

#include <stdio.h>
#include <stdlib.h>

#include "doomtype.h"
#include "info.h"

#include "deh_defs.h"
#include "deh_io.h"
#include "deh_main.h"
#include "deh_mapping.h"
#include "deh_htic.h"

DEH_BEGIN_MAPPING(state_mapping, state_t)
  DEH_MAPPING("Sprite number",    sprite)
  DEH_MAPPING("Sprite subnumber", frame)
  DEH_MAPPING("Duration",         tics)
  DEH_MAPPING("Next frame",       nextstate)
  DEH_MAPPING("Unknown 1",        misc1)
  DEH_MAPPING("Unknown 2",        misc2)
  DEH_UNSUPPORTED_MAPPING("Action pointer")
DEH_END_MAPPING

static void *DEH_FrameStart(deh_context_t *context, char *line)
{
    int frame_number = 0;
    int mapped_frame_number;
    state_t *state;

    if (sscanf(line, "Frame %i", &frame_number) != 1)
    {
        DEH_Warning(context, "Parse error on section start");
        return NULL;
    }

    // Map the HHE frame number (which assumes a Heretic 1.0 state table)
    // to the internal frame number (which is is the Heretic 1.3 state table):

    mapped_frame_number = DEH_MapHereticFrameNumber(frame_number);

    if (mapped_frame_number < 0 || mapped_frame_number >= DEH_HERETIC_NUMSTATES)
    {
        DEH_Warning(context, "Invalid frame number: %i", frame_number);
        return NULL;
    }

    state = &states[mapped_frame_number];

    return state;
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

    // "Next frame" numbers need to undergo mapping.

    if (!strcasecmp(variable_name, "Next frame"))
    {
        ivalue = DEH_MapHereticFrameNumber(ivalue);
    }

    DEH_SetMapping(context, &state_mapping, state, variable_name, ivalue);
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

