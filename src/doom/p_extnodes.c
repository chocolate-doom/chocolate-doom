//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2005-2014 Simon Howard
// Copyright(C) 2015-2018 Fabian Greffrath
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
//	[crispy] support maps with NODES in compressed or uncompressed ZDBSP
// 	format or DeePBSP format and/or LINEDEFS and THINGS lumps in Hexen format
//

#include "m_bbox.h"
#include "p_local.h"
#include "i_swap.h"
#include "i_system.h"
#include "w_wad.h"
#include "z_zone.h"

// [crispy] support maps with compressed ZDBSP nodes
#include "config.h"
#ifdef HAVE_LIBZ
#include <zlib.h>
#endif

#include "p_extnodes.h"

void P_SpawnMapThing (mapthing_t*    mthing);
fixed_t GetOffset(vertex_t *v1, vertex_t *v2);
sector_t* GetSectorAtNullAddress(void);

// [crispy] support maps with NODES in compressed or uncompressed ZDBSP
// format or DeePBSP format and/or LINEDEFS and THINGS lumps in Hexen format
mapformat_t P_CheckMapFormat (int lumpnum)
{
    mapformat_t format = 0;
    byte *nodes = NULL;
    int b;

    if ((b = lumpnum+ML_BLOCKMAP+1) < numlumps &&
        !strncasecmp(lumpinfo[b]->name, "BEHAVIOR", 8))
    {
	fprintf(stderr, "Hexen (");
	format |= MFMT_HEXEN;
    }
    else
	fprintf(stderr, "Doom (");

    if (!((b = lumpnum+ML_NODES) < numlumps &&
        (nodes = W_CacheLumpNum(b, PU_CACHE)) &&
        W_LumpLength(b) > 0))
	fprintf(stderr, "no nodes");
    else
    if (!memcmp(nodes, "xNd4\0\0\0\0", 8))
    {
	fprintf(stderr, "DeePBSP");
	format |= MFMT_DEEPBSP;
    }
    else
    if (!memcmp(nodes, "XNOD", 4))
    {
	fprintf(stderr, "ZDBSP");
	format |= MFMT_ZDBSPX;
    }
    else
    if (!memcmp(nodes, "ZNOD", 4))
    {
	fprintf(stderr, "compressed ZDBSP");
	format |= MFMT_ZDBSPZ;
    }
    else
	fprintf(stderr, "BSP");

    if (nodes)
	W_ReleaseLumpNum(b);

    return format;
}

// [crispy] support maps with DeePBSP nodes
// adapted from prboom-plus/src/p_setup.c:633-752
void P_LoadSegs_DeePBSP (int lump)
{
    int i;
    mapseg_deepbsp_t *data;

    numsegs = W_LumpLength(lump) / sizeof(mapseg_deepbsp_t);
    segs = Z_Malloc(numsegs * sizeof(seg_t), PU_LEVEL, 0);
    data = (mapseg_deepbsp_t *)W_CacheLumpNum(lump, PU_STATIC);

    for (i = 0; i < numsegs; i++)
    {
	seg_t *li = segs + i;
	mapseg_deepbsp_t *ml = data + i;
	int side, linedef;
	line_t *ldef;
	int vn1, vn2;

	// [MB] 2020-04-30: Fix endianess for DeePBSDP V4 nodes
	vn1 = LONG(ml->v1);
	vn2 = LONG(ml->v2);

	li->v1 = &vertexes[vn1];
	li->v2 = &vertexes[vn2];

	li->angle = (SHORT(ml->angle))<<FRACBITS;

//	li->offset = (SHORT(ml->offset))<<FRACBITS; // [crispy] recalculated below
	linedef = (unsigned short)SHORT(ml->linedef);
	ldef = &lines[linedef];
	li->linedef = ldef;
	side = SHORT(ml->side);

	// e6y: check for wrong indexes
	if ((unsigned)linedef >= (unsigned)numlines)
	{
		I_Error("P_LoadSegs: seg %d references a non-existent linedef %d",
			i, (unsigned)linedef);
	}
	if ((unsigned)ldef->sidenum[side] >= (unsigned)numsides)
	{
		I_Error("P_LoadSegs: linedef %d for seg %d references a non-existent sidedef %d",
			linedef, i, (unsigned)ldef->sidenum[side]);
	}

	li->sidedef = &sides[ldef->sidenum[side]];
	li->frontsector = sides[ldef->sidenum[side]].sector;
	// [crispy] recalculate
	li->offset = GetOffset(li->v1, (ml->side ? ldef->v2 : ldef->v1));

	if (ldef->flags & ML_TWOSIDED)
	{
	    int sidenum = ldef->sidenum[side ^ 1];

	    if (sidenum < 0 || sidenum >= numsides)
	    {
		if (li->sidedef->midtexture)
		{
		    li->backsector = 0;
		    fprintf(stderr, "P_LoadSegs: Linedef %d has two-sided flag set, but no second sidedef\n", linedef);
		}
		else
		    li->backsector = GetSectorAtNullAddress();
	    }
	    else
		li->backsector = sides[sidenum].sector;
	}
	else
	    li->backsector = 0;
    }

    W_ReleaseLumpNum(lump);
}

// [crispy] support maps with DeePBSP nodes
// adapted from prboom-plus/src/p_setup.c:843-863
void P_LoadSubsectors_DeePBSP (int lump)
{
    mapsubsector_deepbsp_t *data;
    int i;

    numsubsectors = W_LumpLength(lump) / sizeof(mapsubsector_deepbsp_t);
    subsectors = Z_Malloc(numsubsectors * sizeof(subsector_t), PU_LEVEL, 0);
    data = (mapsubsector_deepbsp_t *)W_CacheLumpNum(lump, PU_STATIC);

    // [crispy] fail on missing subsectors
    if (!data || !numsubsectors)
	I_Error("P_LoadSubsectors: No subsectors in map!");

    for (i = 0; i < numsubsectors; i++)
    {
	// [MB] 2020-04-30: Fix endianess for DeePBSDP V4 nodes
	subsectors[i].numlines = (unsigned short)SHORT(data[i].numsegs);
	subsectors[i].firstline = LONG(data[i].firstseg);
    }

    W_ReleaseLumpNum(lump);
}
// [crispy] support maps with DeePBSP nodes
// adapted from prboom-plus/src/p_setup.c:995-1038
void P_LoadNodes_DeePBSP (int lump)
{
    const byte *data;
    int i;

    numnodes = (W_LumpLength (lump) - 8) / sizeof(mapnode_deepbsp_t);
    nodes = Z_Malloc(numnodes * sizeof(node_t), PU_LEVEL, 0);
    data = W_CacheLumpNum (lump, PU_STATIC);

    // [crispy] warn about missing nodes
    if (!data || !numnodes)
    {
	if (numsubsectors == 1)
	    fprintf(stderr, "P_LoadNodes: No nodes in map, but only one subsector.\n");
	else
	    I_Error("P_LoadNodes: No nodes in map!");
    }

    // skip header
    data += 8;

    for (i = 0; i < numnodes; i++)
    {
	node_t *no = nodes + i;
	const mapnode_deepbsp_t *mn = (const mapnode_deepbsp_t *) data + i;
	int j;

	no->x = SHORT(mn->x)<<FRACBITS;
	no->y = SHORT(mn->y)<<FRACBITS;
	no->dx = SHORT(mn->dx)<<FRACBITS;
	no->dy = SHORT(mn->dy)<<FRACBITS;

	for (j = 0; j < 2; j++)
	{
	    int k;
	    // [MB] 2020-04-30: Fix endianess for DeePBSDP V4 nodes
	    no->children[j] = LONG(mn->children[j]);

	    for (k = 0; k < 4; k++)
		no->bbox[j][k] = SHORT(mn->bbox[j][k])<<FRACBITS;
	}
    }

  W_ReleaseLumpNum(lump);
}

// [crispy] support maps with compressed or uncompressed ZDBSP nodes
// adapted from prboom-plus/src/p_setup.c:1040-1331
// heavily modified, condensed and simplyfied
// - removed most paranoid checks, brought in line with Vanilla P_LoadNodes()
// - removed const type punning pointers
// - inlined P_LoadZSegs()
// - added support for compressed ZDBSP nodes
// - added support for flipped levels
// [MB] 2020-04-30: Fix endianess for ZDoom extended nodes
void P_LoadNodes_ZDBSP (int lump, boolean compressed)
{
    byte *data;
    unsigned int i;
#ifdef HAVE_LIBZ
    byte *output;
#endif

    unsigned int orgVerts, newVerts;
    unsigned int numSubs, currSeg;
    unsigned int numSegs;
    unsigned int numNodes;
    vertex_t *newvertarray = NULL;

    data = W_CacheLumpNum(lump, PU_LEVEL);

    // 0. Uncompress nodes lump (or simply skip header)

    if (compressed)
    {
#ifdef HAVE_LIBZ
	const int len =  W_LumpLength(lump);
	int outlen, err;
	z_stream *zstream;

	// first estimate for compression rate:
	// output buffer size == 2.5 * input size
	outlen = 2.5 * len;
	output = Z_Malloc(outlen, PU_STATIC, 0);

	// initialize stream state for decompression
	zstream = malloc(sizeof(*zstream));
	memset(zstream, 0, sizeof(*zstream));
	zstream->next_in = data + 4;
	zstream->avail_in = len - 4;
	zstream->next_out = output;
	zstream->avail_out = outlen;

	if (inflateInit(zstream) != Z_OK)
	    I_Error("P_LoadNodes: Error during ZDBSP nodes decompression initialization!");

	// resize if output buffer runs full
	while ((err = inflate(zstream, Z_SYNC_FLUSH)) == Z_OK)
	{
	    int outlen_old = outlen;
	    outlen = 2 * outlen_old;
	    output = I_Realloc(output, outlen);
	    zstream->next_out = output + outlen_old;
	    zstream->avail_out = outlen - outlen_old;
	}

	if (err != Z_STREAM_END)
	    I_Error("P_LoadNodes: Error during ZDBSP nodes decompression!");

	fprintf(stderr, "P_LoadNodes: ZDBSP nodes compression ratio %.3f\n",
	        (float)zstream->total_out/zstream->total_in);

	data = output;

	if (inflateEnd(zstream) != Z_OK)
	    I_Error("P_LoadNodes: Error during ZDBSP nodes decompression shut-down!");

	// release the original data lump
	W_ReleaseLumpNum(lump);
	free(zstream);
#else
	I_Error("P_LoadNodes: Compressed ZDBSP nodes are not supported!");
#endif
    }
    else
    {
	// skip header
	data += 4;
    }

    // 1. Load new vertices added during node building

    orgVerts = LONG(*((unsigned int*)data));
    data += sizeof(orgVerts);

    newVerts = LONG(*((unsigned int*)data));
    data += sizeof(newVerts);

    if (orgVerts + newVerts == (unsigned int)numvertexes)
    {
	newvertarray = vertexes;
    }
    else
    {
	newvertarray = Z_Malloc((orgVerts + newVerts) * sizeof(vertex_t), PU_LEVEL, 0);
	memcpy(newvertarray, vertexes, orgVerts * sizeof(vertex_t));
	memset(newvertarray + orgVerts, 0, newVerts * sizeof(vertex_t));
    }

    for (i = 0; i < newVerts; i++)
    {
	newvertarray[i + orgVerts].r_x =
	newvertarray[i + orgVerts].x = LONG(*((unsigned int*)data));
	data += sizeof(newvertarray[0].x);

	newvertarray[i + orgVerts].r_y =
	newvertarray[i + orgVerts].y = LONG(*((unsigned int*)data));
	data += sizeof(newvertarray[0].y);
    }

    if (vertexes != newvertarray)
    {
	for (i = 0; i < (unsigned int)numlines; i++)
	{
	    lines[i].v1 = lines[i].v1 - vertexes + newvertarray;
	    lines[i].v2 = lines[i].v2 - vertexes + newvertarray;
	}

	Z_Free(vertexes);
	vertexes = newvertarray;
	numvertexes = orgVerts + newVerts;
    }

    // 2. Load subsectors

    numSubs = LONG(*((unsigned int*)data));
    data += sizeof(numSubs);

    if (numSubs < 1)
	I_Error("P_LoadNodes: No subsectors in map!");

    numsubsectors = numSubs;
    subsectors = Z_Malloc(numsubsectors * sizeof(subsector_t), PU_LEVEL, 0);

    for (i = currSeg = 0; i < numsubsectors; i++)
    {
	mapsubsector_zdbsp_t *mseg = (mapsubsector_zdbsp_t*) data + i;

	subsectors[i].firstline = currSeg;
	subsectors[i].numlines = LONG(mseg->numsegs);
	currSeg += LONG(mseg->numsegs);
    }

    data += numsubsectors * sizeof(mapsubsector_zdbsp_t);

    // 3. Load segs

    numSegs = LONG(*((unsigned int*)data));
    data += sizeof(numSegs);

    // The number of stored segs should match the number of segs used by subsectors
    if (numSegs != currSeg)
    {
	I_Error("P_LoadNodes: Incorrect number of segs in ZDBSP nodes!");
    }

    numsegs = numSegs;
    segs = Z_Malloc(numsegs * sizeof(seg_t), PU_LEVEL, 0);

    for (i = 0; i < numsegs; i++)
    {
	line_t *ldef;
	unsigned int linedef;
	unsigned char side;
	seg_t *li = segs + i;
	mapseg_zdbsp_t *ml = (mapseg_zdbsp_t *) data + i;
	unsigned int v1, v2;

	v1 = LONG(ml->v1);
	v2 = LONG(ml->v2);
	li->v1 = &vertexes[v1];
	li->v2 = &vertexes[v2];

	linedef = (unsigned short)SHORT(ml->linedef);
	ldef = &lines[linedef];
	li->linedef = ldef;
	side = ml->side;

	// e6y: check for wrong indexes
	if ((unsigned)linedef >= (unsigned)numlines)
	{
		I_Error("P_LoadSegs: seg %d references a non-existent linedef %d",
			i, (unsigned)linedef);
	}
	if ((unsigned)ldef->sidenum[side] >= (unsigned)numsides)
	{
		I_Error("P_LoadSegs: linedef %d for seg %d references a non-existent sidedef %d",
			linedef, i, (unsigned)ldef->sidenum[side]);
	}

	li->sidedef = &sides[ldef->sidenum[side]];
	li->frontsector = sides[ldef->sidenum[side]].sector;

	// seg angle and offset are not included
	li->angle = R_PointToAngle2(segs[i].v1->x, segs[i].v1->y, segs[i].v2->x, segs[i].v2->y);
	li->offset = GetOffset(li->v1, (ml->side ? ldef->v2 : ldef->v1));

	if (ldef->flags & ML_TWOSIDED)
	{
	    int sidenum = ldef->sidenum[side ^ 1];

	    if (sidenum < 0 || sidenum >= numsides)
	    {
		if (li->sidedef->midtexture)
		{
		    li->backsector = 0;
		    fprintf(stderr, "P_LoadSegs: Linedef %u has two-sided flag set, but no second sidedef\n", linedef);
		}
		else
		    li->backsector = GetSectorAtNullAddress();
	    }
	    else
		li->backsector = sides[sidenum].sector;
	}
	else
	    li->backsector = 0;
    }

    data += numsegs * sizeof(mapseg_zdbsp_t);

    // 4. Load nodes

    numNodes = LONG(*((unsigned int*)data));
    data += sizeof(numNodes);

    numnodes = numNodes;
    nodes = Z_Malloc(numnodes * sizeof(node_t), PU_LEVEL, 0);

    for (i = 0; i < numnodes; i++)
    {
	int j, k;
	node_t *no = nodes + i;
	mapnode_zdbsp_t *mn = (mapnode_zdbsp_t *) data + i;

	no->x = SHORT(mn->x)<<FRACBITS;
	no->y = SHORT(mn->y)<<FRACBITS;
	no->dx = SHORT(mn->dx)<<FRACBITS;
	no->dy = SHORT(mn->dy)<<FRACBITS;

	for (j = 0; j < 2; j++)
	{
	    no->children[j] = LONG(mn->children[j]);
	
	    for (k = 0; k < 4; k++)
		no->bbox[j][k] = SHORT(mn->bbox[j][k])<<FRACBITS;
	}
    }

#ifdef HAVE_LIBZ
    if (compressed)
	Z_Free(output);
    else
#endif
    W_ReleaseLumpNum(lump);
}

// [crispy] allow loading of Hexen-format maps
// adapted from chocolate-doom/src/hexen/p_setup.c:348-400
void P_LoadThings_Hexen (int lump)
{
    byte *data;
    int i;
    mapthing_t spawnthing;
    mapthing_hexen_t *mt;
    int numthings;

    data = W_CacheLumpNum(lump, PU_STATIC);
    numthings = W_LumpLength(lump) / sizeof(mapthing_hexen_t);

    mt = (mapthing_hexen_t *) data;
    for (i = 0; i < numthings; i++, mt++)
    {
//	spawnthing.tid = SHORT(mt->tid);
	spawnthing.x = SHORT(mt->x);
	spawnthing.y = SHORT(mt->y);
//	spawnthing.height = SHORT(mt->height);
	spawnthing.angle = SHORT(mt->angle);
	spawnthing.type = SHORT(mt->type);
	spawnthing.options = SHORT(mt->options);

//	spawnthing.special = mt->special;
//	spawnthing.arg1 = mt->arg1;
//	spawnthing.arg2 = mt->arg2;
//	spawnthing.arg3 = mt->arg3;
//	spawnthing.arg4 = mt->arg4;
//	spawnthing.arg5 = mt->arg5;

	P_SpawnMapThing(&spawnthing);
    }

    W_ReleaseLumpNum(lump);
}

// [crispy] allow loading of Hexen-format maps
// adapted from chocolate-doom/src/hexen/p_setup.c:410-490
void P_LoadLineDefs_Hexen (int lump)
{
    byte *data;
    int i;
    maplinedef_hexen_t *mld;
    line_t *ld;
    vertex_t *v1, *v2;
    int warn; // [crispy] warn about unknown linedef types

    numlines = W_LumpLength(lump) / sizeof(maplinedef_hexen_t);
    lines = Z_Malloc(numlines * sizeof(line_t), PU_LEVEL, 0);
    memset(lines, 0, numlines * sizeof(line_t));
    data = W_CacheLumpNum(lump, PU_STATIC);

    mld = (maplinedef_hexen_t *) data;
    ld = lines;
    warn = 0; // [crispy] warn about unknown linedef types
    for (i = 0; i < numlines; i++, mld++, ld++)
    {
	ld->flags = (unsigned short)SHORT(mld->flags);

	ld->special = mld->special;
//	ld->arg1 = mld->arg1;
//	ld->arg2 = mld->arg2;
//	ld->arg3 = mld->arg3;
//	ld->arg4 = mld->arg4;
//	ld->arg5 = mld->arg5;

	// [crispy] warn about unknown linedef types
	if ((unsigned short) ld->special > 141)
	{
	    fprintf(stderr, "P_LoadLineDefs: Unknown special %d at line %d\n", ld->special, i);
	    warn++;
	}

	v1 = ld->v1 = &vertexes[(unsigned short)SHORT(mld->v1)];
	v2 = ld->v2 = &vertexes[(unsigned short)SHORT(mld->v2)];

	ld->dx = v2->x - v1->x;
	ld->dy = v2->y - v1->y;
	if (!ld->dx)
	    ld->slopetype = ST_VERTICAL;
	else if (!ld->dy)
	    ld->slopetype = ST_HORIZONTAL;
	else
	{
	    if (FixedDiv(ld->dy, ld->dx) > 0)
		ld->slopetype = ST_POSITIVE;
	    else
		ld->slopetype = ST_NEGATIVE;
	}

	if (v1->x < v2->x)
	{
	    ld->bbox[BOXLEFT] = v1->x;
	    ld->bbox[BOXRIGHT] = v2->x;
	}
	else
	{
	    ld->bbox[BOXLEFT] = v2->x;
	    ld->bbox[BOXRIGHT] = v1->x;
	}
	if (v1->y < v2->y)
	{
	    ld->bbox[BOXBOTTOM] = v1->y;
	    ld->bbox[BOXTOP] = v2->y;
	}
	else
	{
	    ld->bbox[BOXBOTTOM] = v2->y;
	    ld->bbox[BOXTOP] = v1->y;
	}

	// [crispy] calculate sound origin of line to be its midpoint
	ld->soundorg.x = ld->bbox[BOXLEFT] / 2 + ld->bbox[BOXRIGHT] / 2;
	ld->soundorg.y = ld->bbox[BOXTOP] / 2 + ld->bbox[BOXBOTTOM] / 2;

	ld->sidenum[0] = SHORT(mld->sidenum[0]);
	ld->sidenum[1] = SHORT(mld->sidenum[1]);

	// [crispy] substitute dummy sidedef for missing right side
	if (ld->sidenum[0] == NO_INDEX)
	{
	    ld->sidenum[0] = 0;
	    fprintf(stderr, "P_LoadLineDefs: linedef %d without first sidedef!\n", i);
	}

	if (ld->sidenum[0] != NO_INDEX)
	    ld->frontsector = sides[ld->sidenum[0]].sector;
	else
	    ld->frontsector = 0;
	if (ld->sidenum[1] != NO_INDEX)
	    ld->backsector = sides[ld->sidenum[1]].sector;
	else
	    ld->backsector = 0;
    }

    // [crispy] warn about unknown linedef types
    if (warn)
    {
	fprintf(stderr, "P_LoadLineDefs: Found %d line%s with unknown linedef type.\n"
	                "THIS MAP MAY NOT WORK AS EXPECTED!\n", warn, (warn > 1) ? "s" : "");
    }

    W_ReleaseLumpNum(lump);
}

