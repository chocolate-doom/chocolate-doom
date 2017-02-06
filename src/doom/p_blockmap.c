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

static int P_PointOnSide (vertex_t *p, line_t *l)
{
	int64_t dx, dy, ldx, ldy;
	int64_t left, right;
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

static void GenerateBlockList (int x, int y)
{
	line_t *wl;
	int i;

	*data_p++ = 0;

	xl = bmaporgx + x * MAPBLOCKSIZE;
	xh = xl + MAPBLOCKSIZE;
	yl = bmaporgy + y * MAPBLOCKSIZE;
	yh = yl + MAPBLOCKSIZE;

	wl = lines;

	for (i = 0; i < numlines; i++, wl++)
	{
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

void P_CreateBlockMap (void)
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

		minx -= 8; miny -= 8;
		maxx += 8; maxy += 8;

		bmaporgx = minx << FRACBITS;
		bmaporgy = miny << FRACBITS;
		bmapwidth = ((maxx - minx) + MAPBLOCKUNITS) / MAPBLOCKUNITS;
		bmapheight = ((maxy - miny) + MAPBLOCKUNITS) / MAPBLOCKUNITS;
	}

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
		}
	}

	len = sizeof(datalist) * (data_p - datalist);

	fprintf(stderr, "P_CreateBlockMap: (%i, %i) = %i\n", bmapwidth, bmapheight, len);

	// [crispy] copied over from P_LoadBlockMap()
	{
		const int count = sizeof(*blocklinks) * bmapwidth * bmapheight;
		blocklinks = Z_Malloc(count, PU_LEVEL, 0);
		memset(blocklinks, 0, count);
	}

	blockmaplump = datalist;
	blockmap = blockmaplump + 4;
}
