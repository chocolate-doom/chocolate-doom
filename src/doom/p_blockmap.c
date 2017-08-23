//
// Copyright(C) 1993-1996 Id Software, Inc.
//
// DESCRIPTION:
//	[crispy] Create Blockmap
//
// This code is derived from id's "doombsp" binary space partitioner
// which is accompanied with the following README.TXT:

/*
The source code for the binary space partitioner we used for DOOM is now
available at ftp.uwp.edu: /incoming/id/doombsp.zip.

The catch is that the source has a few objective-c constructs in it, so
it will take some work to port to dos.  The only thing that will be a
hassle is replacing the collection objects, the majority is just
straight C.

This code was written and extended, not evolved, so it probably isn't
the cleanest thing in the world.  Please, PLEASE, PLEASE do not ask
for support on this.  I have far too much occupying my time as it is.

Our map editor does not work on wad files.  It saves an ascii text
representation of the file, then launches doombsp to process that into
a wad file. I have included the input and output for E1M1, so you can
verify any porting work you perform.

Having two programs allowed us to seperate the tasks well under
NEXTSTEP, but people working on dos editors will probably want to
integrate a version of the bsp code directly into the editor.

If you are creating new DOOM maps for other people to use, we would
apreciate it if the wadfiles you create use a PWAD identifier at the
start of the file instead of the normal IWAD.  This causes DOOM to tell
the user that they are playing a modified version, and no technical
support will be given.

If you are creating a map editor for distribution to other people,
contact Jay WIlbur (jayw@idsoftware.com) about getting a license
agreement for the use of our trademark, etc.  Its not a money issue,
just some legal jazz.

BTW, there IS a bug in here that can cause up to a four pixel wide
column to be drawn out of order, causing a more distant floor and
ceiling plane to stream farther forward than it should.  You can
sometimes see this on E1M1 looking towards the imp up on the ledge
at the entrance to the zig zag room.  A few pixel wide column of
slime streams down to the right of the walkway.  It takes a bit of
fidgeting with the mouse to find the spot.  If someone out there
tracks it down, let me know...

Have fun!

John Carmack
Id Software
*/

// At the time of the Crispy Doom 4.1 release the license agreement
// is still pending. For all code modifications the following applies:
//
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

#include <stdlib.h>
#include "i_system.h"
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

		datalist = I_Realloc(datalist, (datalist_size = 2 * datalist_size) * sizeof(*datalist));

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
	datalist = I_Realloc(datalist, datalist_size * sizeof(*datalist));

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
