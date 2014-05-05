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
//	The status bar widget code.
//


#include <stdio.h>
#include <ctype.h>

#include "deh_main.h"
#include "doomdef.h"

#include "z_zone.h"
#include "v_video.h"

#include "i_swap.h"
#include "i_system.h"

#include "w_wad.h"

#include "st_stuff.h"
#include "st_lib.h"
#include "r_local.h"


// in AM_map.c
extern boolean		automapactive; 




//
// Hack display negative frags.
//  Loads and store the stminus lump.
//
patch_t*		sttminus;

void STlib_init(void)
{
    // haleyjd 08/28/10: [STRIFE] STTMINUS -> STCFN045
    sttminus = (patch_t *) W_CacheLumpName(DEH_String("STCFN045"), PU_STATIC);
}


// 
// STlib_initNum
//
// haleyjd 09/01/10: [STRIFE]
// * Rogue removed the "on" member of st_number_t.
void
STlib_initNum
( st_number_t*		n,
  int			x,
  int			y,
  patch_t**		pl,
  int*			num,
  int			width )
{
    n->x	= x;
    n->y	= y;
    n->width	= width;
    n->num	= num;
    n->p	= pl;
}


// 
// STlib_drawNum
//
// A fairly efficient way to draw a number
//  based on differences from the old number.
// Note: worth the trouble?
//
// haleyjd 09/01/10: [STRIFE]
// * Rogue removed the "refresh" parameter and caching code
//
void
STlib_drawNum
( st_number_t*  n)
{
    int         numdigits = n->width;
    int         num = *n->num;

    int         w = SHORT(n->p[0]->width) + 1; // [STRIFE] +1
    int         x = n->x;

    int         neg;

    neg = num < 0;

    if (neg)
    {
        if (numdigits == 2 && num < -9)
            num = -9;
        else if (numdigits == 3 && num < -99)
            num = -99;

        num = -num;
    }

    /* haleyjd 09/01/10: [STRIFE] Widget caching system removed by Rogue
    // clear the area
    x = n->x - numdigits*w;

    if (n->y - ST_Y < 0)
        I_Error("drawNum: n->y - ST_Y < 0");

    V_CopyRect(x, n->y - ST_Y, st_backing_screen, w*numdigits, h, x, n->y);
    */

    // if non-number, do not draw it
    if (num == 1994)
        return;

    x = n->x;

    // in the special case of 0, you draw 0
    if (!num)
        V_DrawPatch(x - w, n->y, n->p[ 0 ]);

    // draw the new number
    while (num && numdigits--)
    {
        x -= w;
        V_DrawPatch(x, n->y, n->p[ num % 10 ]);
        num /= 10;
    }

    // draw a minus sign if necessary
    if (neg)
        V_DrawPatch(x - 8, n->y, sttminus);
}


// 
// STlib_drawNumPositive
//
// haleyjd 09/01/10: [STRIFE] New function.
// * Mostly the same as STlib_drawNum, except doesn't draw negatives.
//
void
STlib_drawNumPositive
( st_number_t*  n)
{
    int         numdigits = n->width;
    int         num = *n->num;

    int         w = SHORT(n->p[0]->width) + 1; // [STRIFE] +1
    int         x = n->x;

    // Don't draw negative values.
    if (num < 0)
        num = 0;

    // if non-number, do not draw it
    if (num == 1994)
        return;

    x = n->x;

    // in the special case of 0, you draw 0
    if (!num)
        V_DrawPatch(x - w, n->y, n->p[ 0 ]);

    // draw the new number
    while (num && numdigits--)
    {
        x -= w;
        V_DrawPatch(x, n->y, n->p[ num % 10 ]);
        num /= 10;
    }
}


// haleyjd 09/01/10: [STRIFE] All other functions were removed.
/*
void
STlib_updateNum
( st_number_t*		n,
  boolean		refresh )
{
    if (*n->on) STlib_drawNum(n, refresh);
}


//
void
STlib_initPercent
( st_percent_t*		p,
  int			x,
  int			y,
  patch_t**		pl,
  int*			num,
  boolean*		on,
  patch_t*		percent )
{
    STlib_initNum(&p->n, x, y, pl, num, on, 3);
    p->p = percent;
}




void
STlib_updatePercent
( st_percent_t*		per,
  int			refresh )
{
    if (refresh && *per->n.on)
	V_DrawPatch(per->n.x, per->n.y, per->p);
    
    STlib_updateNum(&per->n, refresh);
}



void
STlib_initMultIcon
( st_multicon_t*	i,
  int			x,
  int			y,
  patch_t**		il,
  int*			inum,
  boolean*		on )
{
    i->x	= x;
    i->y	= y;
    i->oldinum 	= -1;
    i->inum	= inum;
    i->on	= on;
    i->p	= il;
}



void
STlib_updateMultIcon
( st_multicon_t*	mi,
  boolean		refresh )
{
    int			w;
    int			h;
    int			x;
    int			y;

    if (*mi->on
	&& (mi->oldinum != *mi->inum || refresh)
	&& (*mi->inum!=-1))
    {
	if (mi->oldinum != -1)
	{
	    x = mi->x - SHORT(mi->p[mi->oldinum]->leftoffset);
	    y = mi->y - SHORT(mi->p[mi->oldinum]->topoffset);
	    w = SHORT(mi->p[mi->oldinum]->width);
	    h = SHORT(mi->p[mi->oldinum]->height);

	    if (y - ST_Y < 0)
		I_Error("updateMultIcon: y - ST_Y < 0");

	    V_CopyRect(x, y-ST_Y, st_backing_screen, w, h, x, y);
	}
	V_DrawPatch(mi->x, mi->y, mi->p[*mi->inum]);
	mi->oldinum = *mi->inum;
    }
}



void
STlib_initBinIcon
( st_binicon_t*		b,
  int			x,
  int			y,
  patch_t*		i,
  boolean*		val,
  boolean*		on )
{
    b->x	= x;
    b->y	= y;
    b->oldval	= false;
    b->val	= val;
    b->on	= on;
    b->p	= i;
}



void
STlib_updateBinIcon
( st_binicon_t*		bi,
  boolean		refresh )
{
    int			x;
    int			y;
    int			w;
    int			h;

    if (*bi->on
     && (bi->oldval != *bi->val || refresh))
    {
	x = bi->x - SHORT(bi->p->leftoffset);
	y = bi->y - SHORT(bi->p->topoffset);
	w = SHORT(bi->p->width);
	h = SHORT(bi->p->height);

	if (y - ST_Y < 0)
	    I_Error("updateBinIcon: y - ST_Y < 0");

	if (*bi->val)
	    V_DrawPatch(bi->x, bi->y, bi->p);
	else
	    V_CopyRect(x, y-ST_Y, st_backing_screen, w, h, x, y);

	bi->oldval = *bi->val;
    }

}
*/

