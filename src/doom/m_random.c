//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2005-2014 Simon Howard
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
// DESCRIPTION:
//	Random number LUT.
//

//
// DeHackEd support for RNG ;)
//
#include "deh_main.h"
#include "deh_misc.h"


//
// M_Random
// Returns a 0-255 number
//

static const unsigned char rndtable[] = deh_rngtable;

int	rndindex = 0;
int	prndindex = 0;

// Which one is deterministic?
int P_Random (void)
{
    prndindex = (prndindex+1)&0xff;
    return rndtable[prndindex];
}

int M_Random (void)
{
    rndindex = (rndindex+1)&0xff;
    return rndtable[rndindex];
}

void M_ClearRandom (void)
{
    rndindex = prndindex = 0;
}

// inspired by the same routine in Eternity, thanks haleyjd
int P_SubRandom (void)
{
    int r = P_Random();
    return r - P_Random();
}
