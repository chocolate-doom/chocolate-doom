// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: m_cheat.c 162 2005-10-04 21:41:42Z fraggle $
//
// Copyright(C) 1993-1996 Id Software, Inc.
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
// Revision 1.3  2005/10/04 21:41:42  fraggle
// Rewrite cheats code.  Add dehacked cheat replacement.
//
// Revision 1.2  2005/07/23 16:44:55  fraggle
// Update copyright to GNU GPL
//
// Revision 1.1.1.1  2005/07/23 16:20:31  fraggle
// Initial import
//
//
// DESCRIPTION:
//	Cheat sequence checking.
//
//-----------------------------------------------------------------------------


static const char
rcsid[] = "$Id: m_cheat.c 162 2005-10-04 21:41:42Z fraggle $";

#include <string.h>

#include "doomtype.h"
#include "m_cheat.h"

//
// CHEAT SEQUENCE PACKAGE
//

//
// Called in st_stuff module, which handles the input.
// Returns a 1 if the cheat was successful, 0 if failed.
//
int
cht_CheckCheat
( cheatseq_t*	cht,
  char		key )
{
    // if we make a short sequence on a cheat with parameters, this 
    // will not work in vanilla doom.  behave the same.

    if (cht->parameter_chars > 0 && strlen(cht->sequence) < cht->sequence_len)
        return false;
    
    if (cht->chars_read < strlen(cht->sequence))
    {
        // still reading characters from the cheat code
        // and verifying.  reset back to the beginning 
        // if a key is wrong

        if (key == cht->sequence[cht->chars_read])
            ++cht->chars_read;
        else
            cht->chars_read = 0;
        
        cht->param_chars_read = 0;
    }
    else if (cht->param_chars_read < cht->parameter_chars)
    {
        // we have passed the end of the cheat sequence and are 
        // entering parameters now 
        
        cht->parameter_buf[cht->param_chars_read] = key;
        
        ++cht->param_chars_read;
    }

    if (cht->chars_read >= strlen(cht->sequence)
     && cht->param_chars_read >= cht->parameter_chars)
    {
        cht->chars_read = cht->param_chars_read = 0;

        return true;
    }
    
    // cheat not matched yet

    return false;
}

void
cht_GetParam
( cheatseq_t*	cht,
  char*		buffer )
{
    memcpy(buffer, cht->parameter_buf, cht->parameter_chars);
}


