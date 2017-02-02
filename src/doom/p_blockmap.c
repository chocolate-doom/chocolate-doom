//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2005-2014 Simon Howard
// Copyright(C) 2017 Fabian Greffrath
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
//	[crispy] Create Blockmap
//

#include <math.h>
#include "doomdata.h"
#include "r_defs.h"
#include "r_state.h"
#include "p_local.h"
#include "i_system.h"
#include "z_zone.h"

#define BLOCKSIZE 128

typedef struct
{
	int x, y, w, h;
} rect_t;

rect_t worldbounds;

int32_t	datalist[0x10000], *data_p;

float	xl, xh, yl, yh;

static int	BSPPointOnSide (vertex_t *p, line_t *l)
{
	float	dx,dy;
	float	left, right;
	float	a,b,c,d;

	if (!l->dx)
	{
		if (p->x > l->v1->x-2 && p->x < l->v1->x+2)
			return -1;
		if (p->x < l->v1->x)
			return l->dy > 0;
		return l->dy < 0;
	}
	if (!l->dy)
	{
		if (p->y > l->v1->y-2 && p->y < l->v1->y+2)
			return -1;
		if (p->y < l->v1->y)
			return l->dx < 0;
		return l->dx > 0;
	}

	dx = l->v1->x - p->x;
	dy = l->v1->y - p->y;
	a = l->dx*l->dx + l->dy*l->dy;
	b = 2*(l->dx*dx + l->dy*dy);
	c = dx*dx+dy*dy - 2*2;		// 2 unit radius
	d = b*b - 4*a*c;
	if (d>0)
		return -1;		// within four pixels of line

	dx = p->x - l->v1->x;
	dy = p->y - l->v1->y;

	left = l->dy * dx;
	right = dy * l->dx;

	if ( fabs (left-right) < 0.5 )	// allow slop
		return -1;		// on line
	if (right < left)
		return 0;		// front side
	return 1;			// back side
}

static boolean	LineContact (line_t *wl)
{
	vertex_t		*p1, *p2, pt1, pt2 , pd;
	float		lxl, lxh, lyl, lyh;
	line_t	ld;
	int			s1,s2;

	ld.v1 = &pd;

	p1 = wl->v1;
	p2 = wl->v2;
	ld.v1->x = p1->x;
	ld.v1->y = p1->y;
	ld.dx = p2->x - p1->x;
	ld.dy = p2->y - p1->y;

	if (p1->x < p2->x)
	{
		lxl = p1->x;
		lxh = p2->x;
	}
	else
	{
		lxl = p2->x;
		lxh = p1->x;
	}
	if (p1->y < p2->y)
	{
		lyl = p1->y;
		lyh = p2->y;
	}
	else
	{
		lyl = p2->y;
		lyh = p1->y;
	}

	if (lxl >= xh || lxh < xl || lyl >= yh || lyh < yl)
		return false;	// no bbox intersections

	if ( ld.dy / ld.dx > 0)
	{	// positive slope
		pt1.x = xl;
		pt1.y = yh;
		pt2.x = xh;
		pt2.y = yl;
	}
	else
	{	// negetive slope
		pt1.x = xh;
		pt1.y = yh;
		pt2.x = xl;
		pt2.y = yl;
	}

	s1 = BSPPointOnSide (&pt1, &ld);
	s2 = BSPPointOnSide (&pt2, &ld);

	return s1 != s2;
}

static void GenerateBlockList (int x, int y)
{
	line_t	*wl;
	int			i;

	*data_p++ = 0;		// leave space for thing links

	xl = bmaporgx + x*BLOCKSIZE;
	xh = xl+BLOCKSIZE;
	yl = bmaporgy + y*BLOCKSIZE;
	yh = yl+BLOCKSIZE;

	wl = lines;

	for (i=0 ; i<numlines ; i++,wl++)
	{
		if (wl->v1->x == wl->v2->x)
		{	// vertical
			if (wl->v1->x < xl || wl->v1->x >= xh)
				continue;
			if (wl->v1->y < wl->v2->y)
			{
				if (wl->v1->y >= yh || wl->v2->y < yl)
					continue;
			}
			else
			{
				if (wl->v2->y >= yh || wl->v1->y < yl)
					continue;
			}
			*data_p++ = i;
			continue;
		}
		if (wl->v1->y == wl->v2->y)
		{	// horizontal
			if (wl->v1->y < yl || wl->v1->y >= yh)
				continue;
			if (wl->v1->x < wl->v2->x)
			{
				if (wl->v1->x >= xh || wl->v2->x < xl)
					continue;
			}
			else
			{
				if (wl->v2->x >= xh || wl->v1->x < xl)
					continue;
			}
			*data_p++ = i;
			continue;
		}
		// diagonal
		if (LineContact (wl) ) 
			*data_p++ = i;
	}

	*data_p++ = -1;		// end of list marker
}

static void IDRectFromPoints(rect_t *rect, vertex_t const *p1, vertex_t const *p2 )
{
	if (p1->x < p2->x)
	{
		rect->x = p1->x;
		rect->w = p2->x - p1->x + 1;
	}
	else
	{
		rect->x = p2->x;
		rect->w = p1->x - p2->x + 1;
	}

	if (p1->y < p2->y)
	{
		rect->y = p1->y;
		rect->h = p2->y - p1->y + 1;
	}
	else
	{
		rect->y = p2->y;
		rect->h = p1->y - p2->y + 1;
	}
}

static void IDEnclosePoint (rect_t *rect, vertex_t const *point)
{
	float	right, top;

	right = rect->x + rect->w - 1;
	top = rect->y + rect->h - 1;

	if (point->x < rect->x)
		rect->x = point->x;
	if (point->y < rect->y)
		rect->y = point->y;
	if (point->x > right)
		right = point->x;
	if (point->y > top)
		top = point->y;

	rect->w = right - rect->x + 1;
	rect->h = top - rect->y + 1;
}

static void BoundLineStore (rect_t *r)
{
	int	i,c;
	line_t	*line_p;

	c = numlines;
	if (!c)
		I_Error ("BoundLineStore: empty list");

	line_p = lines;
	IDRectFromPoints (r, line_p->v1, line_p->v2);

	for (i=1 ; i<c ; i++)
	{
		line_p = &lines[i];
		IDEnclosePoint (r, line_p->v1);
		IDEnclosePoint (r, line_p->v2);
	}
}

void SaveBlocks (void)
{
	int32_t		x,y, len;
	int32_t	*pointer_p;

	BoundLineStore (&worldbounds);
	worldbounds.x -= 8;
	worldbounds.y -= 8;
	worldbounds.w += 16;
	worldbounds.h += 16;

	bmapwidth = (worldbounds.w+BLOCKSIZE-1)/BLOCKSIZE;
	bmapheight = (worldbounds.h+BLOCKSIZE-1)/BLOCKSIZE;
	bmaporgx = worldbounds.x;
	bmaporgy = worldbounds.y;

	pointer_p = datalist;
	*pointer_p++ = bmaporgx;
	*pointer_p++ = bmaporgy;
	*pointer_p++ = bmapwidth;
	*pointer_p++ = bmaporgy;

	data_p = pointer_p + bmapwidth*bmapheight;

	for (y=0 ; y<bmapheight ; y++)
		for (x=0 ; x<bmapwidth ; x++)
		{
			len = data_p - datalist;
			*pointer_p++ = len;
			GenerateBlockList (x,y);
		}

	len = 2*(data_p-datalist);

	printf ("blockmap: (%i, %i) = %i\n",bmapwidth, bmapheight, len);

	// [crispy] copied over from P_LoadBlockMap()
	{
		const int count = sizeof(*blocklinks) * bmapwidth * bmapheight;
		blocklinks = Z_Malloc(count, PU_LEVEL, 0);
		memset(blocklinks, 0, count);
		blockmap = blockmaplump+4;
	}
}


// [crispy] taken from mbfsrc/P_SETUP.C:547-707, slightly adapted
/*
static void P_CreateBlockMap(void)
{
  register int i;
  fixed_t minx = INT_MAX, miny = INT_MAX, maxx = INT_MIN, maxy = INT_MIN;

  // First find limits of map

  for (i=0; i<numvertexes; i++)
    {
      if (vertexes[i].x >> FRACBITS < minx)
	minx = vertexes[i].x >> FRACBITS;
      else
	if (vertexes[i].x >> FRACBITS > maxx)
	  maxx = vertexes[i].x >> FRACBITS;
      if (vertexes[i].y >> FRACBITS < miny)
	miny = vertexes[i].y >> FRACBITS;
      else
	if (vertexes[i].y >> FRACBITS > maxy)
	  maxy = vertexes[i].y >> FRACBITS;
    }

  // Save blockmap parameters

  bmaporgx = minx << FRACBITS;
  bmaporgy = miny << FRACBITS;
  bmapwidth  = ((maxx-minx) >> MAPBTOFRAC) + 1;
  bmapheight = ((maxy-miny) >> MAPBTOFRAC) + 1;

  // Compute blockmap, which is stored as a 2d array of variable-sized lists.
  //
  // Pseudocode:
  //
  // For each linedef:
  //
  //   Map the starting and ending vertices to blocks.
  //
  //   Starting in the starting vertex's block, do:
  //
  //     Add linedef to current block's list, dynamically resizing it.
  //
  //     If current block is the same as the ending vertex's block, exit loop.
  //
  //     Move to an adjacent block by moving towards the ending block in
  //     either the x or y direction, to the block which contains the linedef.

  {
    typedef struct { int n, nalloc, *list; } bmap_t;  // blocklist structure
    unsigned tot = bmapwidth * bmapheight;            // size of blockmap
    bmap_t *bmap = calloc(sizeof *bmap, tot);         // array of blocklists
    int x, y, adx, ady, bend;

    for (i=0; i < numlines; i++)
      {
	int dx, dy, diff, b;

	// starting coordinates
	if (crispy_fliplevels)
	{
	    x = (lines[i].v2->x >> FRACBITS) - minx;
	    y = (lines[i].v2->y >> FRACBITS) - miny;
	}
	else
	{
	    x = (lines[i].v1->x >> FRACBITS) - minx;
	    y = (lines[i].v1->y >> FRACBITS) - miny;
	}

	// x-y deltas
	adx = lines[i].dx >> FRACBITS, dx = adx < 0 ? -1 : 1;
	ady = lines[i].dy >> FRACBITS, dy = ady < 0 ? -1 : 1;

	// difference in preferring to move across y (>0) instead of x (<0)
	diff = !adx ? 1 : !ady ? -1 :
	  (((x >> MAPBTOFRAC) << MAPBTOFRAC) +
	   (dx > 0 ? MAPBLOCKUNITS-1 : 0) - x) * (ady = abs(ady)) * dx -
	  (((y >> MAPBTOFRAC) << MAPBTOFRAC) +
	   (dy > 0 ? MAPBLOCKUNITS-1 : 0) - y) * (adx = abs(adx)) * dy;

	// starting block, and pointer to its blocklist structure
	b = (y >> MAPBTOFRAC)*bmapwidth + (x >> MAPBTOFRAC);

	// ending block
	if (crispy_fliplevels)
	{
	    bend = (((lines[i].v1->y >> FRACBITS) - miny) >> MAPBTOFRAC) *
	        bmapwidth + (((lines[i].v1->x >> FRACBITS) - minx) >> MAPBTOFRAC);
	}
	else
	{
	    bend = (((lines[i].v2->y >> FRACBITS) - miny) >> MAPBTOFRAC) *
	        bmapwidth + (((lines[i].v2->x >> FRACBITS) - minx) >> MAPBTOFRAC);
	}

	// delta for pointer when moving across y
	dy *= bmapwidth;

	// deltas for diff inside the loop
	adx <<= MAPBTOFRAC;
	ady <<= MAPBTOFRAC;

	// Now we simply iterate block-by-block until we reach the end block.
	while ((unsigned) b < tot)    // failsafe -- should ALWAYS be true
	  {
	    // Increase size of allocated list if necessary
	    if (bmap[b].n >= bmap[b].nalloc)
	      bmap[b].list = crispy_realloc(bmap[b].list,
				     (bmap[b].nalloc = bmap[b].nalloc ?
				      bmap[b].nalloc*2 : 8)*sizeof*bmap->list);

	    // Add linedef to end of list
	    bmap[b].list[bmap[b].n++] = i;

	    // If we have reached the last block, exit
	    if (b == bend)
	      break;

	    // Move in either the x or y direction to the next block
	    if (diff < 0)
	      diff += ady, b += dx;
	    else
	      diff -= adx, b += dy;
	  }
      }

    // Compute the total size of the blockmap.
    //
    // Compression of empty blocks is performed by reserving two offset words
    // at tot and tot+1.
    //
    // 4 words, unused if this routine is called, are reserved at the start.

    {
      int count = tot+6;  // we need at least 1 word per block, plus reserved's

      for (i = 0; i < tot; i++)
	if (bmap[i].n)
	  count += bmap[i].n + 2; // 1 header word + 1 trailer word + blocklist

      // Allocate blockmap lump with computed count
      blockmaplump = Z_Malloc(sizeof(*blockmaplump) * count, PU_LEVEL, 0);
    }

    // Now compress the blockmap.
    {
      int ndx = tot += 4;         // Advance index to start of linedef lists
      bmap_t *bp = bmap;          // Start of uncompressed blockmap

      blockmaplump[ndx++] = 0;    // Store an empty blockmap list at start
      blockmaplump[ndx++] = -1;   // (Used for compression)

      for (i = 4; i < tot; i++, bp++)
	if (bp->n)                                      // Non-empty blocklist
	  {
	    blockmaplump[blockmaplump[i] = ndx++] = 0;  // Store index & header
	    do
	      blockmaplump[ndx++] = bp->list[--bp->n];  // Copy linedef list
	    while (bp->n);
	    blockmaplump[ndx++] = -1;                   // Store trailer
	    free(bp->list);                             // Free linedef list
	  }
	else            // Empty blocklist: point to reserved empty blocklist
	  blockmaplump[i] = tot;

      free(bmap);    // Free uncompressed blockmap
    }
  }

  // [crispy] copied over from P_LoadBlockMap()
  {
    int count = sizeof(*blocklinks) * bmapwidth * bmapheight;
    blocklinks = Z_Malloc(count, PU_LEVEL, 0);
    memset(blocklinks, 0, count);
    blockmap = blockmaplump+4;
  }
}
*/
