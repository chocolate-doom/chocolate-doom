// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: deh_ptr.c 175 2005-10-08 20:54:16Z fraggle $
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
// Revision 1.4  2005/10/08 20:54:16  fraggle
// Proper dehacked error/warning framework.  Catch a load more errors.
//
// Revision 1.3  2005/10/03 11:08:16  fraggle
// Replace end of section functions with NULLs as they arent currently being
// used for anything.
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
// Parses Action Pointer entries in dehacked files
//
//-----------------------------------------------------------------------------

#include <stdlib.h>
#include <string.h>

#include "doomdef.h"
#include "doomtype.h"
#include "info.h"

#include "deh_defs.h"
#include "deh_io.h"
#include "deh_main.h"

static actionf_t codeptrs[NUMSTATES];

static void DEH_PointerInit(void)
{
    int i;
    
    // Initialise list of dehacked pointers

    for (i=0; i<NUMSTATES; ++i)
        codeptrs[i] = states[i].action;
}

static void *DEH_PointerStart(deh_context_t *context, char *line)
{
    int frame_number = 0;
    
    // FIXME: can the third argument here be something other than "Frame"
    // or are we ok?

    if (sscanf(line, "Pointer %*i (%*s %i)", &frame_number) != 1)
    {
        DEH_Warning(context, "Parse error on section start");
        return NULL;
    }

    if (frame_number < 0 || frame_number >= NUMSTATES)
    {
        DEH_Warning(context, "Invalid frame number: %i", frame_number);
        return NULL;
    }

    return &states[frame_number];
}

static void DEH_PointerParseLine(deh_context_t *context, char *line, void *tag)
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
    
//    printf("Set %s to %s for state\n", variable_name, value);

    // all values are integers

    ivalue = atoi(value);
    
    // set the appropriate field

    if (!strcasecmp(variable_name, "Codep frame"))
    {
        if (ivalue < 0 || ivalue >= NUMSTATES)
        {
            DEH_Warning(context, "Invalid state '%i'", ivalue);
        }
        else
        {        
            state->action = codeptrs[ivalue];
        }
    }
    else
    {
        DEH_Warning(context, "Unknown variable name '%s'", variable_name);
    }
}

deh_section_t deh_section_pointer =
{
    "Pointer",
    DEH_PointerInit,
    DEH_PointerStart,
    DEH_PointerParseLine,
    NULL,
};

