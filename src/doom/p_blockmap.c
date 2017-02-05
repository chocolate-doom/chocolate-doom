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

#include <stdlib.h>
#include "p_local.h"
#include "z_zone.h"

static int datalist[0x10000], *data_p;

static fixed_t xl, xh, yl, yh;

static int j, k;

static int P_PointOnSide (vertex_t *p, line_t *l)
{
	fixed_t dx, dy;
	fixed_t left, right;
	fixed_t a, b, c, d;
if (j == 7&& k == 200) 
{
	puts("!pos!");
}

	if (!l->dx)
	{
if (j == 7&& k == 200) 
{
	puts("!dx!");
}
		if (p->x > l->v1->x - 2*FRACUNIT && p->x < l->v1->x + 2*FRACUNIT)
		{
			return -1;
		}

		if (p->x < l->v1->x)
		{
			return l->dy > 0;
		}

		return l->dy < 0;
	}

	if (!l->dy)
	{
if (j == 7&& k == 200) 
{
	puts("!dy!");
}
		if (p->y > l->v1->y - 2*FRACUNIT && p->y < l->v1->y + 2*FRACUNIT)
		{
			return -1;
		}

		if (p->y < l->v1->y)
		{
			return l->dx < 0;
		}

		return l->dx > 0;
	}

	dx = l->v1->x - p->x;
	dy = l->v1->y - p->y;
	a = FixedMul(l->dx, l->dx) + FixedMul(l->dy, l->dy);
	b = 2 * (FixedMul(l->dx, dx) + FixedMul(l->dy, dy));
	c = FixedMul(dx, dx) + FixedMul(dy, dy) - 2 * 2*FRACUNIT; // 2 unit radius
	d = FixedMul(b, b) - 4 * FixedMul(a, c);
if (j == 7&& k == 200) 
{
	printf("*** %d %d %d %d\n", a>>FRACBITS, b>>FRACBITS, c>>FRACBITS, d>>FRACBITS);
}

	if (d > 0)
	{
		// within four pixels of line
		return -1;
	}

	dx = p->x - l->v1->x;
	dy = p->y - l->v1->y;

	left = FixedMul(l->dy>>FRACBITS, dx);
	right = FixedMul(dy, l->dx>>FRACBITS);
if (j == 7&& k == 200) 
{
	printf("*** %d %d\n", left>>FRACBITS, right>>FRACBITS);
}

	if (abs(left-right) < FRACUNIT>>8) // allow slop
	{
		// on line
		return -1;
	}

	if (right < left)
	{
		// front side
		return 0;
	}

	// back side
	return 1;
}

static boolean LineContact (line_t *wl)
{
	vertex_t *p1, *p2, pt1, pt2, pd;
	fixed_t lxl, lxh, lyl, lyh;
	line_t ld;
	int s1, s2;

	p1 = wl->v1;
	p2 = wl->v2;

	ld.v1 = &pd;
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

if (j == 7&& wl-lines == 200) 
{
	printf("%d %d\n%d %d\n%d %d\n%d %d\n", lxl >>FRACBITS, xh>>FRACBITS , lxh>>FRACBITS ,xl >>FRACBITS, lyl>>FRACBITS , yh >>FRACBITS, lyh >>FRACBITS, yl>>FRACBITS);
}

	// no bbox intersections
	if (lxl >= xh || lxh < xl || lyl >= yh || lyh < yl)
	{
		return false;
	}
if (j == 7&& wl-lines == 200) 
{
	puts("hier!");
}

	if (FixedDiv(ld.dy, ld.dx) > 0)
	{
		// positive slope
		pt1.x = xl;
		pt1.y = yh;
		pt2.x = xh;
		pt2.y = yl;
if (j == 7&& wl-lines == 200) 
{
	puts("pos!");
}
	}
	else
	{
		// negative slope
		pt1.x = xh;
		pt1.y = yh;
		pt2.x = xl;
		pt2.y = yl;
if (j == 7&& wl-lines == 200) 
{
	puts("neg!");
}
	}

	s1 = P_PointOnSide(&pt1, &ld);
	s2 = P_PointOnSide(&pt2, &ld);

	return s1 != s2;
}

static void GenerateBlockList (int x, int y)
{
	line_t *wl;
	int i;

	// leave space for thing links
	*data_p++ = 0;

	xl = bmaporgx + x * MAPBLOCKSIZE;
	xh = xl + MAPBLOCKSIZE;
	yl = bmaporgy + y * MAPBLOCKSIZE;
	yh = yl + MAPBLOCKSIZE;

	wl = lines;

	for (i = 0; i < numlines; i++, wl++)
	{
k=wl-lines;
		// vertical
		if (wl->v1->x == wl->v2->x)
		{
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

		// horizontal
		if (wl->v1->y == wl->v2->y)
		{
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
		if (LineContact(wl))
		{
			*data_p++ = i;
		}
	}

	// end of list marker
	*data_p++ = -1;
}

void SaveBlocks (void)
{
	int x ,y, len;
	int *pointer_p;

	{
		int i;
		fixed_t minx = INT_MAX, miny = INT_MAX, maxx = INT_MIN, maxy = INT_MIN;

		for (i = 0; i < numvertexes; i++)
		{
			const vertex_t *const v = &vertexes[i];
			const fixed_t vx = v->x >> FRACBITS, vy = v->y >> FRACBITS;

			if (vx < minx)
				minx = vx;
			else
			if (vx > maxx)
				maxx = vx;

			if (vy < miny)
				miny = vy;
			else
			if (vy > maxy)
				maxy = vy;
		}

		minx -= 8;
		miny -= 8;

		bmaporgx = minx << FRACBITS;
		bmaporgy = miny << FRACBITS;
		bmapwidth = ((maxx - minx) + MAPBLOCKUNITS) / MAPBLOCKUNITS;
		bmapheight = ((maxy - miny) + MAPBLOCKUNITS) / MAPBLOCKUNITS;
	}

//printf("%d %d %d %d\n", bmaporgx, bmaporgy, bmapwidth, bmapheight); exit(1);

	pointer_p = datalist;
	*pointer_p++ = bmaporgx;
	*pointer_p++ = bmaporgy;
	*pointer_p++ = bmapwidth;
	*pointer_p++ = bmapheight;

	data_p = pointer_p + bmapwidth * bmapheight;

	for (y = 0; y < bmapheight; y++)
	{
		for (x = 0; x < bmapwidth; x++)
		{
			len = data_p - datalist;
			*pointer_p++ = len;
			GenerateBlockList(x, y);
			j++;
		}
	}

	len = 2 * (data_p - datalist);

	printf ("blockmap: (%i, %i) = %i\n", bmapwidth, bmapheight, len);

	// [crispy] copied over from P_LoadBlockMap()
	{
		const int count = sizeof(*blocklinks) * bmapwidth * bmapheight;
		blocklinks = Z_Malloc(count, PU_LEVEL, 0);
		memset(blocklinks, 0, count);
	}

	blockmaplump = datalist;
	blockmap = blockmaplump + 4;
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
