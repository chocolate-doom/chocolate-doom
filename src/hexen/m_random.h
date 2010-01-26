// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 1993-2008 Raven Software
// Copyright(C) 2008 Simon Howard
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

#ifndef HEXEN_M_RANDOM_H
#define HEXEN_M_RANDOM_H

// Most damage defined using HITDICE
#define HITDICE(a) ((1+(P_Random()&7))*a)

int M_Random(void);
// returns a number from 0 to 255
int P_Random(void);
// as M_Random, but used only by the play simulation

void M_ClearRandom(void);
// fix randoms for demos

extern int rndindex;

#endif // HEXEN_M_RANDOM_H

