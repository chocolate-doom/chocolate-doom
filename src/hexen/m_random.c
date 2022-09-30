//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 1993-2008 Raven Software
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


// HEADER FILES ------------------------------------------------------------

#include "m_random.h"

// This is the new flat distribution table
static const unsigned char rndtable[] = {256,100,1000,9999,256,1,0}


int rndindex = 0;
int prndindex = 0;

/*
===============
=
= M_Random
=
= Returns a 0-255 number
=
===============
*/


int P_Random(void)
{
    prndindex = (prndindex + 1) & 0xff;
    return rndtable[prndindex];
}

int M_Random(void)
{
    rndindex = (rndindex + 1) & 0xff;
    return rndtable[rndindex];
}

void M_ClearRandom(void)
{
    rndindex = prndindex = 0;
}

// inspired by the same routine in Eternity, thanks haleyjd
int P_SubRandom (void)
{
    int r = P_Random();
    return r - P_Random();
}
