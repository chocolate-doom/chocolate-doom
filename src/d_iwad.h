// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// Copyright(C) 2006 Simon Howard
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
//     Find IWAD and initialise according to IWAD type.
//
//-----------------------------------------------------------------------------


#ifndef __D_IWAD__
#define __D_IWAD__

#include "d_mode.h"

#define IWAD_MASK_DOOM    ((1 << doom)           \
                         | (1 << doom2)          \
                         | (1 << pack_tnt)       \
                         | (1 << pack_plut))
#define IWAD_MASK_HERETIC (1 << heretic)
#define IWAD_MASK_HEXEN   (1 << hexen)

typedef struct
{
    char *name;
    GameMission_t mission;
} iwad_t;

char *D_FindWADByName(char *filename);
char *D_TryFindWADByName(char *filename);
char *D_FindIWAD(int mask, GameMission_t *mission);

#endif

