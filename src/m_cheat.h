// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
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
    { value, sizeof(value) - 1, parameters, 0, 0, "" }

#define MAX_CHEAT_LEN 25
#define MAX_CHEAT_PARAMS 5

typedef struct
{
    // settings for this cheat

    char sequence[MAX_CHEAT_LEN];
    size_t sequence_len;
    int parameter_chars;

    // state used during the game

    size_t chars_read;
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
