// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: deh_misc.c 175 2005-10-08 20:54:16Z fraggle $
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
// Revision 1.1  2005/10/04 22:10:32  fraggle
// Dehacked "Misc" section parser (currently a dummy)
//
//
//-----------------------------------------------------------------------------
//
// Parses "Misc" sections in dehacked files
//
//-----------------------------------------------------------------------------

#include "doomdef.h"
#include "doomtype.h"
#include "deh_defs.h"
#include "deh_io.h"

static void *DEH_MiscStart(deh_context_t *context, char *line)
{
    DEH_Warning(context, "Dehacked 'Misc' sections are not supported yet.");
    return NULL;
}

static void DEH_MiscParseLine(deh_context_t *context, char *line, void *tag)
{
}

deh_section_t deh_section_misc =
{
    "Misc",
    NULL,
    DEH_MiscStart,
    DEH_MiscParseLine,
    NULL,
};

