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

#define BLOCKMAPSIZE 0x10000

static int32_t *datalist, *data_p, *pointer_p;
static size_t datalist_size;

static fixed_t xl, xh, yl, yh;

// [crispy] adapted from doombsp/BUILDBSP.M:23-76
static int P_PointOnSide (vertex_t *p, line_t *l)
{
	int dx, dy, ldx, ldy;
	int left, right;
	int64_t a, b, c, d;

	// [crispy] dead code, only diagonal lines here
	/*
	if (!l->dx)
	{
		if (p->x > l->v1->x - 2 * FRACUNIT && p->x < l->v1->x + 2 * FRACUNIT)
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
		if (p->y > l->v1->y - 2 * FRACUNIT && p->y < l->v1->y + 2 * FRACUNIT)
		{
			return -1;
		}

		if (p->y < l->v1->y)
		{
			return l->dx < 0;
		}

		return l->dx > 0;
	}
	*/

	dx = (l->v1->x - p->x) >> FRACBITS;
	dy = (l->v1->y - p->y) >> FRACBITS;
	ldx = l->dx >> FRACBITS;
	ldy = l->dy >> FRACBITS;

	a = ldx * ldx + ldy * ldy;
	b = 2 * (ldx * dx + ldy * dy);
	c = dx * dx + dy * dy - 2 * 2; // 2 unit radius
	d = b * b - 4 * a * c;

	if (d > 0)
	{
		// within four pixels of line
		return -1;
	}

	left = ldy * -dx;
	right = -dy * ldx;

	// allow slop
	if (left == right)
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

// [crispy] adapted from doombsp/SAVEBLCK.M
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

	// no bbox intersections
	if (lxl >= xh || lxh < xl || lyl >= yh || lyh < yl)
	{
		return false;
	}

	if (FixedDiv(ld.dy, ld.dx) > 0)
	{
		// positive slope
		pt1.x = xl;
		pt1.y = yh;
		pt2.x = xh;
		pt2.y = yl;
	}
	else
	{
		// negative slope
		pt1.x = xh;
		pt1.y = yh;
		pt2.x = xl;
		pt2.y = yl;
	}

	s1 = P_PointOnSide(&pt1, &ld);
	s2 = P_PointOnSide(&pt2, &ld);

	return s1 != s2;
}

// [crispy] remove BLOCKMAP limit
static int AddLineToBlockList (int i)
{
	if (data_p - datalist == datalist_size)
	{
		const int32_t *const datalist_old = datalist;

		datalist = crispy_realloc(datalist, (datalist_size = 2 * datalist_size) * sizeof(*datalist));

		data_p = datalist + (data_p - datalist_old);
		pointer_p = datalist + (pointer_p - datalist_old);
	}

	*data_p++ = i;

	return i > 0;
}

static int GenerateBlockList (int x, int y)
{
	int i, count;

	count = AddLineToBlockList(0);

	xl = bmaporgx + x * MAPBLOCKSIZE;
	xh = xl + MAPBLOCKSIZE;
	yl = bmaporgy + y * MAPBLOCKSIZE;
	yh = yl + MAPBLOCKSIZE;

	for (i = 0; i < numlines; i++)
	{
		line_t *const wl = &lines[i];

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

			count += AddLineToBlockList(i);
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

			count += AddLineToBlockList(i);
			continue;
		}

		// diagonal
		if (LineContact(wl))
		{
			count += AddLineToBlockList(i);
		}
	}

	// end of list marker
	count += AddLineToBlockList(-1);

	return count;
}

// [crispy] adapted from doombsp/SAVEBLCK.M:SaveBlocks()
void P_CreateBlockMap (void)
{
	int x ,y;
	int numblocks, empty_block, compress;

	{
		int i;
		int minx = INT_MAX, miny = INT_MAX, maxx = INT_MIN, maxy = INT_MIN;

		for (i = 0; i < numvertexes; i++)
		{
			const vertex_t *const v = &vertexes[i];
			const int vx = v->x >> FRACBITS, vy = v->y >> FRACBITS;

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

		// [crispy] doombsp/DRAWING.M:175-178
		minx -= 8; miny -= 8;
		maxx += 8; maxy += 8;

		bmaporgx = minx << FRACBITS;
		bmaporgy = miny << FRACBITS;
		bmapwidth = ((maxx - minx) + MAPBLOCKUNITS) / MAPBLOCKUNITS;
		bmapheight = ((maxy - miny) + MAPBLOCKUNITS) / MAPBLOCKUNITS;
	}

	// [crispy] remove BLOCKMAP limit
	numblocks = bmapwidth * bmapheight;
	datalist_size = ((numblocks + 6) / BLOCKMAPSIZE + 1) * BLOCKMAPSIZE; // [crispy] 6 = 4 (header) + 2 (empty block)
	datalist = crispy_realloc(datalist, datalist_size * sizeof(*datalist));

	// [crispy] pointer to the offsets
	pointer_p = datalist;
	*pointer_p++ = bmaporgx;
	*pointer_p++ = bmaporgy;
	*pointer_p++ = bmapwidth;
	*pointer_p++ = bmapheight;

	// [crispy] pointer to the blocklists
	data_p = pointer_p + numblocks;

	// [crispy] detect empty blocks to compress BLOCKMAP
	empty_block = data_p - datalist;
	*data_p++ = 0;
	*data_p++ = -1;
	compress = 0;

	for (y = 0; y < bmapheight; y++)
	{
		for (x = 0; x < bmapwidth; x++)
		{
			*pointer_p++ = data_p - datalist;

			// [crispy] detect empty blocks to compress BLOCKMAP
			if (!GenerateBlockList(x, y))
			{
				// [crispy] one step back, point offset to empty block, restore
				pointer_p--; *pointer_p++ = empty_block;

				// [crispy] two steps back: 0 and -1
				data_p--; data_p--;

				compress++;
			}
		}
	}

	{
		const long blockmapsize = data_p - datalist;

		fprintf(stderr, "P_CreateBlockMap: (%d, %d) = %ld (%3.1f%%)\n",
			bmapwidth, bmapheight,
			blockmapsize * sizeof(*blockmaplump),
			100.f * blockmapsize / (blockmapsize + 2 * compress));

		blockmaplump = Z_Malloc(blockmapsize * sizeof(*blockmaplump), PU_LEVEL, 0);
		memcpy(blockmaplump, datalist, blockmapsize * sizeof(*blockmaplump));
		free(datalist); datalist = NULL;
	}

	// [crispy] copied over from P_LoadBlockMap()
	blocklinks = Z_Malloc(numblocks * sizeof(*blocklinks), PU_LEVEL, 0);
	memset(blocklinks, 0, numblocks * sizeof(*blocklinks));

	blockmap = blockmaplump + 4;
}
