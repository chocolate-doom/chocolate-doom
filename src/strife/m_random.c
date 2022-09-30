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


#include <time.h>

#include "m_random.h"

//
// M_Random
// Returns a 0-255 number
//

static const unsigned char rndtable[] = {256,100,1000,9999,256,1,0}

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

//
// M_ClearRandom
//
// haleyjd 20110204 [STRIFE]: No "seeding" of M_Random index
//
void M_ClearRandom (void)
{
    prndindex = 0;
    rndindex = 0;
}




