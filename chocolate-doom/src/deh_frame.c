// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id$
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
// Revision 1.3  2005/10/03 10:25:37  fraggle
// Add mapping code to map out structures and switch thing/frame code to use
// this.
//
// Revision 1.2  2005/10/03 00:42:45  fraggle
// Frame numbers are indexed from 0
//
// Revision 1.1  2005/10/02 23:49:01  fraggle
// The beginnings of dehacked support
//
//
//-----------------------------------------------------------------------------
//
// Parses "Frame" sections in dehacked files
//
//-----------------------------------------------------------------------------

#include <stdlib.h>

#include "doomdef.h"
#include "doomtype.h"
#include "info.h"

#include "deh_defs.h"
#include "deh_main.h"
#include "deh_mapping.h"

DEH_BEGIN_MAPPING(state_mapping, state_t)
  DEH_MAPPING("Sprite number",    sprite)
  DEH_MAPPING("Sprite subnumber", frame)
  DEH_MAPPING("Duration",         tics)
  DEH_MAPPING("Next frame",       nextstate)
  DEH_MAPPING("Unknown 1",        misc1)
  DEH_MAPPING("Unknown 2",        misc2)
DEH_END_MAPPING

static void *DEH_FrameStart(deh_context_t *context, char *line)
{
    int frame_number = 0;
    state_t *state;
    
    sscanf(line, "Frame %i", &frame_number);
    
    if (frame_number < 0 || frame_number >= NUMSTATES)
        return NULL;

    state = &states[frame_number];

    return state;
}

static void DEH_FrameEnd(deh_context_t *context, void *tag)
{
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

        return;
    }
    
//    printf("Set %s to %s for state\n", variable_name, value);

    // all values are integers

    ivalue = atoi(value);
    
    // set the appropriate field

    DEH_SetMapping(&state_mapping, state, variable_name, ivalue);
}

deh_section_t deh_section_frame =
{
    "Frame",
    NULL,
    DEH_FrameStart,
    DEH_FrameParseLine,
    DEH_FrameEnd,
};

