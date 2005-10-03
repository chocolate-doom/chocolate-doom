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
// Parses Text substitution sections in dehacked files
//
//-----------------------------------------------------------------------------

#include "doomdef.h"
#include "doomtype.h"
#include "deh_defs.h"

static void *DEH_TextStart(deh_context_t *context, char *line)
{
    return NULL;
}

static void DEH_TextParseLine(deh_context_t *context, char *line, void *tag)
{
}

deh_section_t deh_section_text =
{
    "Text",
    NULL,
    DEH_TextStart,
    DEH_TextParseLine,
    NULL,
};

