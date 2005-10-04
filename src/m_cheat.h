// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: m_cheat.h 162 2005-10-04 21:41:42Z fraggle $
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
// DESCRIPTION:
//	Cheat code checking.
//
//-----------------------------------------------------------------------------


#ifndef __M_CHEAT__
#define __M_CHEAT__

//
// CHEAT SEQUENCE PACKAGE
//

// declaring a cheat

#define CHEAT(value, parameters) \
    { value, sizeof(value) - 1, parameters, 0, 0 }

#define MAX_CHEAT_LEN 15
#define MAX_CHEAT_PARAMS 5

typedef struct
{
    // settings for this cheat

    char sequence[MAX_CHEAT_LEN];
    int sequence_len;
    int parameter_chars;

    // state used during the game

    int chars_read;
    int param_chars_read;
    char parameter_buf[MAX_CHEAT_PARAMS];
} cheatseq_t;

int
cht_CheckCheat
( cheatseq_t*		cht,
  char			key );


void
cht_GetParam
( cheatseq_t*		cht,
  char*			buffer );


#endif
//-----------------------------------------------------------------------------
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
//-----------------------------------------------------------------------------
