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
//	Do all the WAD I/O, get map description,
//	set up initial state and misc. LUTs.
//



#include <math.h>

#include "z_zone.h"

#include "deh_main.h"
#include "i_swap.h"
#include "m_argv.h"
#include "m_bbox.h"
#include "m_misc.h" // [crispy] M_StringJoin()

#include "g_game.h"

#include "i_system.h"
#include "w_wad.h"

#include "doomdef.h"
#include "p_local.h"

#include "s_sound.h"

#include "doomstat.h"

// [crispy] support maps with compressed ZDBSP nodes
#include "config.h"
#ifdef HAVE_LIBZ
#include <zlib.h>
#endif

void	P_SpawnMapThing (mapthing_t*	mthing);


//
// MAP related Lookup tables.
// Store VERTEXES, LINEDEFS, SIDEDEFS, etc.
//
int		numvertexes;
vertex_t*	vertexes;

int		numsegs;
seg_t*		segs;

int		numsectors;
sector_t*	sectors;

int		numsubsectors;
subsector_t*	subsectors;

int		numnodes;
node_t*		nodes;

int		numlines;
line_t*		lines;

int		numsides;
side_t*		sides;

static int      totallines;

// BLOCKMAP
// Created from axis aligned bounding box
// of the map, a rectangular array of
// blocks of size ...
// Used to speed up collision detection
// by spatial subdivision in 2D.
//
// Blockmap size.
int		bmapwidth;
int		bmapheight;	// size in mapblocks
int32_t*	blockmap;	// int for larger maps // [crispy] BLOCKMAP limit
// offsets in blockmap are from here
int32_t*	blockmaplump; // [crispy] BLOCKMAP limit
// origin of block map
fixed_t		bmaporgx;
fixed_t		bmaporgy;
// for thing chains
mobj_t**	blocklinks;		


// REJECT
// For fast sight rejection.
// Speeds up enemy AI by skipping detailed
//  LineOf Sight calculation.
// Without special effect, this could be
//  used as a PVS lookup as well.
//
byte*		rejectmatrix;


// Maintain single and multi player starting spots.
#define MAX_DEATHMATCH_STARTS	10

mapthing_t	deathmatchstarts[MAX_DEATHMATCH_STARTS];
mapthing_t*	deathmatch_p;
mapthing_t	playerstarts[MAXPLAYERS];

typedef enum
{
    MFMT_DOOMBSP = 0x000,
    MFMT_DEEPBSP = 0x001,
    MFMT_ZDBSPX  = 0x002,
    MFMT_ZDBSPZ  = 0x004,
    MFMT_HEXEN   = 0x100,
} mapformat_t;

// [crispy] recalculate seg offsets
// adapted from prboom-plus/src/p_setup.c:474-482
static fixed_t GetOffset(vertex_t *v1, vertex_t *v2)
{
    fixed_t dx, dy;
    fixed_t r;

    dx = (v1->x - v2->x)>>FRACBITS;
    dy = (v1->y - v2->y)>>FRACBITS;
    r = (fixed_t)(sqrt(dx*dx + dy*dy))<<FRACBITS;

    return r;
}




//
// P_LoadVertexes
//
void P_LoadVertexes (int lump)
{
    byte*		data;
    int			i;
    mapvertex_t*	ml;
    vertex_t*		li;

    // Determine number of lumps:
    //  total lump length / vertex record length.
    numvertexes = W_LumpLength (lump) / sizeof(mapvertex_t);

    // Allocate zone memory for buffer.
    vertexes = Z_Malloc (numvertexes*sizeof(vertex_t),PU_LEVEL,0);	

    // Load data into cache.
    data = W_CacheLumpNum (lump, PU_STATIC);
	
    ml = (mapvertex_t *)data;
    li = vertexes;

    // Copy and convert vertex coordinates,
    // internal representation as fixed.
    for (i=0 ; i<numvertexes ; i++, li++, ml++)
    {
	li->x = SHORT(ml->x)<<FRACBITS;
	li->y = SHORT(ml->y)<<FRACBITS;

	if (crispy_fliplevels)
	    li->x = -li->x;

	// [crispy] initialize pseudovertexes with actual vertex coordinates
	li->px = li->x;
	li->py = li->y;
	li->moved = false;
    }

    // Free buffer memory.
    W_ReleaseLumpNum(lump);
}

//
// GetSectorAtNullAddress
//
sector_t* GetSectorAtNullAddress(void)
{
    static boolean null_sector_is_initialized = false;
    static sector_t null_sector;

    if (!null_sector_is_initialized)
    {
        memset(&null_sector, 0, sizeof(null_sector));
        I_GetMemoryValue(0, &null_sector.floorheight, 4);
        I_GetMemoryValue(4, &null_sector.ceilingheight, 4);
        null_sector_is_initialized = true;
    }

    return &null_sector;
}

//
// P_LoadSegs
//
void P_LoadSegs (int lump)
{
    byte*		data;
    int			i;
    mapseg_t*		ml;
    seg_t*		li;
    line_t*		ldef;
    int			linedef;
    int			side;
    int                 sidenum;
	
    numsegs = W_LumpLength (lump) / sizeof(mapseg_t);
    segs = Z_Malloc (numsegs*sizeof(seg_t),PU_LEVEL,0);	
    memset (segs, 0, numsegs*sizeof(seg_t));
    data = W_CacheLumpNum (lump,PU_STATIC);
	
    ml = (mapseg_t *)data;
    li = segs;
    for (i=0 ; i<numsegs ; i++, li++, ml++)
    {
	li->v1 = &vertexes[(unsigned short)SHORT(ml->v1)]; // [crispy] extended nodes
	li->v2 = &vertexes[(unsigned short)SHORT(ml->v2)]; // [crispy] extended nodes

	if (crispy_fliplevels)
	{
            vertex_t* tmp = li->v1;
            li->v1 = li->v2;
            li->v2 = tmp;
	}

	li->angle = (SHORT(ml->angle))<<FRACBITS;

	if (crispy_fliplevels)
            li->angle = -li->angle;

//	li->offset = (SHORT(ml->offset))<<FRACBITS; // [crispy] recalculated below
	linedef = (unsigned short)SHORT(ml->linedef); // [crispy] extended nodes
	ldef = &lines[linedef];
	li->linedef = ldef;
	side = SHORT(ml->side);

        // e6y: check for wrong indexes
        if ((unsigned)ldef->sidenum[side] >= (unsigned)numsides)
        {
            I_Error("P_LoadSegs: linedef %d for seg %d references a non-existent sidedef %d",
                    linedef, i, (unsigned)ldef->sidenum[side]);
        }

	li->sidedef = &sides[ldef->sidenum[side]];
	li->frontsector = sides[ldef->sidenum[side]].sector;
	// [crispy] recalculate
	li->offset = GetOffset(li->v1, ((ml->side ^ crispy_fliplevels) ? ldef->v2 : ldef->v1));

        if (ldef-> flags & ML_TWOSIDED)
        {
            sidenum = ldef->sidenum[side ^ 1];

            // If the sidenum is out of range, this may be a "glass hack"
            // impassible window.  Point at side #0 (this may not be
            // the correct Vanilla behavior; however, it seems to work for
            // OTTAWAU.WAD, which is the one place I've seen this trick
            // used).

            if (sidenum < 0 || sidenum >= numsides)
            {
                // [crispy] linedef has two-sided flag set, but no valid second sidedef;
                // but since it has a midtexture, it is supposed to be rendered just
                // like a regular one-sided linedef
                if (li->sidedef->midtexture)
                {
                    li->backsector = 0;
                    fprintf(stderr, "P_LoadSegs: Linedef %d has two-sided flag set, but no second sidedef\n", i);
                }
                else
                li->backsector = GetSectorAtNullAddress();
            }
            else
            {
                li->backsector = sides[sidenum].sector;
            }
        }
        else
        {
	    li->backsector = 0;
        }
    }
	
    W_ReleaseLumpNum(lump);
}

// [crispy] support maps with DeePBSP nodes
// adapted from prboom-plus/src/p_setup.c:633-752
static void P_LoadSegs_DeePBSP (int lump)
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

	li->v1 = &vertexes[ml->v1];
	li->v2 = &vertexes[ml->v2];

	if (crispy_fliplevels)
	{
	    vertex_t* tmp = li->v1;
	    li->v1 = li->v2;
	    li->v2 = tmp;
	}

	li->angle = (SHORT(ml->angle))<<FRACBITS;

	if (crispy_fliplevels)
	    li->angle = -li->angle;

//	li->offset = (SHORT(ml->offset))<<FRACBITS; // [crispy] recalculated below
	linedef = (unsigned short)SHORT(ml->linedef);
	ldef = &lines[linedef];
	li->linedef = ldef;
	side = SHORT(ml->side);

        // e6y: check for wrong indexes
        if ((unsigned)ldef->sidenum[side] >= (unsigned)numsides)
        {
            I_Error("P_LoadSegs: linedef %d for seg %d references a non-existent sidedef %d",
                    linedef, i, (unsigned)ldef->sidenum[side]);
        }

	li->sidedef = &sides[ldef->sidenum[side]];
	li->frontsector = sides[ldef->sidenum[side]].sector;
	// [crispy] recalculate
	li->offset = GetOffset(li->v1, ((ml->side ^ crispy_fliplevels) ? ldef->v2 : ldef->v1));

	if (ldef->flags & ML_TWOSIDED)
	{
	    int sidenum = ldef->sidenum[side ^ 1];

	    if (sidenum < 0 || sidenum >= numsides)
	    {
		if (li->sidedef->midtexture)
		{
		    li->backsector = 0;
		    fprintf(stderr, "P_LoadSegs: Linedef %d has two-sided flag set, but no second sidedef\n", i);
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

// [crispy] fix long wall wobble
void P_SegLengths (void)
{
    int i;
//  const int rightangle = abs(finesine[(ANG60/2) >> ANGLETOFINESHIFT]);

    for (i = 0; i < numsegs; i++)
    {
	seg_t *const li = &segs[i];
	int64_t dx, dy;

	dx = li->v2->px - li->v1->px;
	dy = li->v2->py - li->v1->py;
	li->length = (uint32_t)(sqrt((double)dx*dx + (double)dy*dy)/2);

	// [crispy] re-calculate angle used for rendering
	viewx = li->v1->px;
	viewy = li->v1->py;
	li->pangle = R_PointToAngleCrispy(li->v2->px, li->v2->py);

	// [crispy] smoother fake contrast -- disabled :(
//	if (abs(finesine[li->pangle >> ANGLETOFINESHIFT]) < rightangle)
	if (!dy)
	    li->fakecontrast = -1;
	else
//	if (abs(finecosine[li->pangle >> ANGLETOFINESHIFT]) < rightangle)
	if (!dx)
	    li->fakecontrast = 1;
	else
	    li->fakecontrast = 0;
    }
}

//
// P_LoadSubsectors
//
void P_LoadSubsectors (int lump)
{
    byte*		data;
    int			i;
    mapsubsector_t*	ms;
    subsector_t*	ss;
	
    numsubsectors = W_LumpLength (lump) / sizeof(mapsubsector_t);
    subsectors = Z_Malloc (numsubsectors*sizeof(subsector_t),PU_LEVEL,0);	
    data = W_CacheLumpNum (lump,PU_STATIC);
	
    // [crispy] fail on missing subsectors
    if (!data || !numsubsectors)
	I_Error("P_LoadSubsectors: No subsectors in map!");

    ms = (mapsubsector_t *)data;
    memset (subsectors,0, numsubsectors*sizeof(subsector_t));
    ss = subsectors;
    
    for (i=0 ; i<numsubsectors ; i++, ss++, ms++)
    {
	ss->numlines = (unsigned short)SHORT(ms->numsegs); // [crispy] extended nodes
	ss->firstline = (unsigned short)SHORT(ms->firstseg); // [crispy] extended nodes
    }
	
    W_ReleaseLumpNum(lump);
}

// [crispy] support maps with DeePBSP nodes
// adapted from prboom-plus/src/p_setup.c:843-863
static void P_LoadSubsectors_DeePBSP (int lump)
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
	subsectors[i].numlines = (int)data[i].numsegs;
	subsectors[i].firstline = (int)data[i].firstseg;
    }

    W_ReleaseLumpNum(lump);
}


//
// P_LoadSectors
//
void P_LoadSectors (int lump)
{
    byte*		data;
    int			i;
    mapsector_t*	ms;
    sector_t*		ss;
	
    // [crispy] fail on missing sectors
    if (lump >= numlumps)
	I_Error("P_LoadSectors: No sectors in map!");

    numsectors = W_LumpLength (lump) / sizeof(mapsector_t);
    sectors = Z_Malloc (numsectors*sizeof(sector_t),PU_LEVEL,0);	
    memset (sectors, 0, numsectors*sizeof(sector_t));
    data = W_CacheLumpNum (lump,PU_STATIC);
	
    // [crispy] fail on missing sectors
    if (!data || !numsectors)
	I_Error("P_LoadSectors: No sectors in map!");

    ms = (mapsector_t *)data;
    ss = sectors;
    for (i=0 ; i<numsectors ; i++, ss++, ms++)
    {
	ss->floorheight = SHORT(ms->floorheight)<<FRACBITS;
	ss->ceilingheight = SHORT(ms->ceilingheight)<<FRACBITS;
	ss->floorpic = R_FlatNumForName(ms->floorpic);
	ss->ceilingpic = R_FlatNumForName(ms->ceilingpic);
	ss->lightlevel = SHORT(ms->lightlevel);
	ss->special = SHORT(ms->special);
	ss->tag = SHORT(ms->tag);
	ss->thinglist = NULL;
	// [crispy] WiggleFix: [kb] for R_FixWiggle()
	ss->cachedheight = 0;
        // [AM] Sector interpolation.  Even if we're
        //      not running uncapped, the renderer still
        //      uses this data.
        ss->oldfloorheight = ss->floorheight;
        ss->interpfloorheight = ss->floorheight;
        ss->oldceilingheight = ss->ceilingheight;
        ss->interpceilingheight = ss->ceilingheight;
        ss->oldgametic = 0;
    }
	
    W_ReleaseLumpNum(lump);
}


//
// P_LoadNodes
//
void P_LoadNodes (int lump)
{
    byte*	data;
    int		i;
    int		j;
    int		k;
    mapnode_t*	mn;
    node_t*	no;
	
    numnodes = W_LumpLength (lump) / sizeof(mapnode_t);
    nodes = Z_Malloc (numnodes*sizeof(node_t),PU_LEVEL,0);	
    data = W_CacheLumpNum (lump,PU_STATIC);
	
    // [crispy] warn about missing nodes
    if (!data || !numnodes)
    {
	if (numsubsectors == 1)
	    fprintf(stderr, "P_LoadNodes: No nodes in map, but only one subsector.\n");
	else
	    I_Error("P_LoadNodes: No nodes in map!");
    }

    mn = (mapnode_t *)data;
    no = nodes;
    
    for (i=0 ; i<numnodes ; i++, no++, mn++)
    {
	no->x = SHORT(mn->x)<<FRACBITS;
	no->y = SHORT(mn->y)<<FRACBITS;
	no->dx = SHORT(mn->dx)<<FRACBITS;
	no->dy = SHORT(mn->dy)<<FRACBITS;

	if (crispy_fliplevels)
	{
	    no->x += no->dx;
	    no->y += no->dy;
	    no->x = -no->x;
	    no->dy = -no->dy;
	}

	for (j=0 ; j<2 ; j++)
	{
	    no->children[j] = (unsigned short)SHORT(mn->children[j]); // [crispy] extended nodes

	    // [crispy] add support for extended nodes
	    // from prboom-plus/src/p_setup.c:937-957
	    if (no->children[j] == 0xFFFF)
		no->children[j] = -1;
	    else
	    if (no->children[j] & 0x8000)
	    {
		no->children[j] &= ~0x8000;

		if (no->children[j] >= numsubsectors)
		    no->children[j] = 0;

		no->children[j] |= NF_SUBSECTOR;
	    }

	    for (k=0 ; k<4 ; k++)
		no->bbox[j][k] = SHORT(mn->bbox[j][k])<<FRACBITS;

	    if (crispy_fliplevels)
	    {
		fixed_t tmp = no->bbox[j][2];
		no->bbox[j][2] = -no->bbox[j][3];
		no->bbox[j][3] = -tmp;
	    }
	}
    }
	
    W_ReleaseLumpNum(lump);
}

// [crispy] support maps with DeePBSP nodes
// adapted from prboom-plus/src/p_setup.c:995-1038
static void P_LoadNodes_DeePBSP (int lump)
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
	mapnode_deepbsp_t *mn = (mapnode_deepbsp_t *) data + i;
	int j;

	no->x = SHORT(mn->x)<<FRACBITS;
	no->y = SHORT(mn->y)<<FRACBITS;
	no->dx = SHORT(mn->dx)<<FRACBITS;
	no->dy = SHORT(mn->dy)<<FRACBITS;

	if (crispy_fliplevels)
	{
	    no->x += no->dx;
	    no->y += no->dy;
	    no->x = -no->x;
	    no->dy = -no->dy;
	}

	for (j = 0; j < 2; j++)
	{
	    int k;
	    no->children[j] = (unsigned int)(mn->children[j]);

	    for (k = 0; k < 4; k++)
		no->bbox[j][k] = SHORT(mn->bbox[j][k])<<FRACBITS;

	    if (crispy_fliplevels)
	    {
		fixed_t tmp = no->bbox[j][2];
		no->bbox[j][2] = -no->bbox[j][3];
		no->bbox[j][3] = -tmp;
	    }
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
static void P_LoadNodes_ZDBSP (int lump, boolean compressed)
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

    orgVerts = *((unsigned int*)data);
    data += sizeof(orgVerts);

    newVerts = *((unsigned int*)data);
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
	newvertarray[i + orgVerts].px =
	newvertarray[i + orgVerts].x = *((unsigned int*)data);
	data += sizeof(newvertarray[0].x);

	if (crispy_fliplevels)
	{
	    newvertarray[i + orgVerts].px =
	    newvertarray[i + orgVerts].x = -newvertarray[i + orgVerts].x;
	}

	newvertarray[i + orgVerts].py =
	newvertarray[i + orgVerts].y = *((unsigned int*)data);
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

    numSubs = *((unsigned int*)data);
    data += sizeof(numSubs);

    if (numSubs < 1)
	I_Error("P_LoadNodes: No subsectors in map!");

    numsubsectors = numSubs;
    subsectors = Z_Malloc(numsubsectors * sizeof(subsector_t), PU_LEVEL, 0);

    for (i = currSeg = 0; i < numsubsectors; i++)
    {
	mapsubsector_zdbsp_t *mseg = (mapsubsector_zdbsp_t*) data + i;

	subsectors[i].firstline = currSeg;
	subsectors[i].numlines = mseg->numsegs;
	currSeg += mseg->numsegs;
    }

    data += numsubsectors * sizeof(mapsubsector_zdbsp_t);

    // 3. Load segs

    numSegs = *((unsigned int*)data);
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

	li->v1 = &vertexes[ml->v1];
	li->v2 = &vertexes[ml->v2];

	if (crispy_fliplevels)
	{
	    vertex_t* tmp = li->v1;
	    li->v1 = li->v2;
	    li->v2 = tmp;
	}

	linedef = (unsigned short)SHORT(ml->linedef);
	ldef = &lines[linedef];
	li->linedef = ldef;
	side = ml->side;

        // e6y: check for wrong indexes
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
		    fprintf(stderr, "P_LoadSegs: Linedef %u has two-sided flag set, but no second sidedef\n", i);
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

    numNodes = *((unsigned int*)data);
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

	if (crispy_fliplevels)
	{
	    no->x += no->dx;
	    no->y += no->dy;
	    no->x = -no->x;
	    no->dy = -no->dy;
	}

	for (j = 0; j < 2; j++)
	{
	    no->children[j] = (unsigned int)(mn->children[j]);

	    for (k = 0; k < 4; k++)
		no->bbox[j][k] = SHORT(mn->bbox[j][k])<<FRACBITS;

	    if (crispy_fliplevels)
	    {
		fixed_t tmp = no->bbox[j][2];
		no->bbox[j][2] = -no->bbox[j][3];
		no->bbox[j][3] = -tmp;
	    }
	}
    }

#ifdef HAVE_LIBZ
    if (compressed)
	Z_Free(output);
    else
#endif
    W_ReleaseLumpNum(lump);
}


//
// P_LoadThings
//
void P_LoadThings (int lump)
{
    byte               *data;
    int			i;
    mapthing_t         *mt;
    mapthing_t          spawnthing;
    int			numthings;
    boolean		spawn;

    data = W_CacheLumpNum (lump,PU_STATIC);
    numthings = W_LumpLength (lump) / sizeof(mapthing_t);
	
    mt = (mapthing_t *)data;
    for (i=0 ; i<numthings ; i++, mt++)
    {
	spawn = true;

	// Do not spawn cool, new monsters if !commercial
	if (gamemode != commercial)
	{
	    switch (SHORT(mt->type))
	    {
	      case 68:	// Arachnotron
	      case 64:	// Archvile
	      case 88:	// Boss Brain
	      case 89:	// Boss Shooter
	      case 69:	// Hell Knight
	      case 67:	// Mancubus
	      case 71:	// Pain Elemental
	      case 65:	// Former Human Commando
	      case 66:	// Revenant
	      case 84:	// Wolf SS
		spawn = false;
		break;
	    }
	}
	if (spawn == false)
	    break;

	// [crispy] minor fixes to prevent users from getting stuck in levels with mapping errors
	if (singleplayer)
	{
	    // [crispy] spawn Former Human instead of Wolf SS in BFG Edition
	    if (gamevariant == bfgedition && mt->type == 84)
	    {
	        mt->type = 3004;
	    }
	    // [crispy] TNT MAP31 has a yellow key that is erroneously marked as multi-player only
	    if (gamemission == pack_tnt && gamemap == 31 && mt->type == 6)
	    {
	        mt->options &= ~16;
	    }
	}

	// Do spawn all other stuff. 
	spawnthing.x = SHORT(mt->x);
	spawnthing.y = SHORT(mt->y);
	spawnthing.angle = SHORT(mt->angle);
	spawnthing.type = SHORT(mt->type);
	spawnthing.options = SHORT(mt->options);
	
	if (crispy_fliplevels)
	{
	    spawnthing.x = -spawnthing.x;
	    spawnthing.angle = 180 - spawnthing.angle;
	}

	P_SpawnMapThing(&spawnthing);
    }

    W_ReleaseLumpNum(lump);
}

// [crispy] allow loading of Hexen-format maps
// adapted from chocolate-doom/src/hexen/p_setup.c:348-400
static void P_LoadThings_Hexen (int lump)
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

	if (crispy_fliplevels)
	{
	    spawnthing.x = -spawnthing.x;
	    spawnthing.angle = 180 - spawnthing.angle;
	}

	P_SpawnMapThing(&spawnthing);
    }

    W_ReleaseLumpNum(lump);
}

//
// P_LoadLineDefs
// Also counts secret lines for intermissions.
//
void P_LoadLineDefs (int lump)
{
    byte*		data;
    int			i;
    maplinedef_t*	mld;
    line_t*		ld;
    vertex_t*		v1;
    vertex_t*		v2;
    int warn, warn2; // [crispy] warn about invalid linedefs
	
    numlines = W_LumpLength (lump) / sizeof(maplinedef_t);
    lines = Z_Malloc (numlines*sizeof(line_t),PU_LEVEL,0);	
    memset (lines, 0, numlines*sizeof(line_t));
    data = W_CacheLumpNum (lump,PU_STATIC);
	
    mld = (maplinedef_t *)data;
    ld = lines;
    warn = warn2 = 0; // [crispy] warn about invalid linedefs
    for (i=0 ; i<numlines ; i++, mld++, ld++)
    {
	ld->flags = (unsigned short)SHORT(mld->flags); // [crispy] extended nodes
	ld->special = SHORT(mld->special);
	// [crispy] warn about unknown linedef types
	if ((unsigned short) ld->special > 141)
	{
	    fprintf(stderr, "P_LoadLineDefs: Unknown special %d at line %d.\n", ld->special, i);
	    warn++;
	}
	ld->tag = SHORT(mld->tag);
	// [crispy] warn about special linedefs without tag
	if (ld->special && !ld->tag)
	{
	    switch (ld->special)
	    {
		case 1:	// Vertical Door
		case 26:	// Blue Door/Locked
		case 27:	// Yellow Door /Locked
		case 28:	// Red Door /Locked
		case 31:	// Manual door open
		case 32:	// Blue locked door open
		case 33:	// Red locked door open
		case 34:	// Yellow locked door open
		case 117:	// Blazing door raise
		case 118:	// Blazing door open
		case 271:	// MBF sky transfers
		case 272:
		case 48:	// Scroll Wall Left
		case 11:	// s1 Exit level
		case 51:	// s1 Secret exit
		case 52:	// w1 Exit level
		case 124:	// w1 Secret exit
		    break;
		default:
		    fprintf(stderr, "P_LoadLineDefs: Special linedef %d without tag.\n", i);
		    warn2++;
		    break;
	    }
	}
	if (crispy_fliplevels)
	{
	    v1 = ld->v2 = &vertexes[(unsigned short)SHORT(mld->v2)]; // [crispy] extended nodes
	    v2 = ld->v1 = &vertexes[(unsigned short)SHORT(mld->v1)]; // [crispy] extended nodes
	}
	else
	{
	v1 = ld->v1 = &vertexes[(unsigned short)SHORT(mld->v1)]; // [crispy] extended nodes
	v2 = ld->v2 = &vertexes[(unsigned short)SHORT(mld->v2)]; // [crispy] extended nodes
	}
	ld->dx = v2->x - v1->x;
	ld->dy = v2->y - v1->y;
	
	if (!ld->dx)
	    ld->slopetype = ST_VERTICAL;
	else if (!ld->dy)
	    ld->slopetype = ST_HORIZONTAL;
	else
	{
	    if (FixedDiv (ld->dy , ld->dx) > 0)
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

	ld->sidenum[0] = SHORT(mld->sidenum[0]);
	ld->sidenum[1] = SHORT(mld->sidenum[1]);

	// [crispy] substitute dummy sidedef for missing right side
	if (ld->sidenum[0] == NO_INDEX)
	{
	    ld->sidenum[0] = 0;
	    fprintf(stderr, "P_LoadLineDefs: linedef %d without first sidedef!\n", i);
	}

	if (ld->sidenum[0] != NO_INDEX) // [crispy] extended nodes
	    ld->frontsector = sides[ld->sidenum[0]].sector;
	else
	    ld->frontsector = 0;

	if (ld->sidenum[1] != NO_INDEX) // [crispy] extended nodes
	    ld->backsector = sides[ld->sidenum[1]].sector;
	else
	    ld->backsector = 0;
    }

    // [crispy] warn about unknown linedef types
    if (warn)
    {
	fprintf(stderr, "P_LoadLineDefs: Found %d line%s with unknown linedef type.\n", warn, (warn > 1) ? "s" : "");
    }
    // [crispy] warn about special linedefs without tag
    if (warn2)
    {
	fprintf(stderr, "P_LoadLineDefs: Found %d special linedef%s without tag.\n", warn2, (warn2 > 1) ? "s" : "");
    }
    if (warn || warn2)
    {
	fprintf(stderr, "THIS MAP MAY NOT WORK AS EXPECTED!\n");
    }

    W_ReleaseLumpNum(lump);
}

// [crispy] allow loading of Hexen-format maps
// adapted from chocolate-doom/src/hexen/p_setup.c:410-490
static void P_LoadLineDefs_Hexen (int lump)
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

	if (crispy_fliplevels)
	{
	    v1 = ld->v2 = &vertexes[(unsigned short)SHORT(mld->v2)];
	    v2 = ld->v1 = &vertexes[(unsigned short)SHORT(mld->v1)];
	}
	else
	{
	    v1 = ld->v1 = &vertexes[(unsigned short)SHORT(mld->v1)];
	    v2 = ld->v2 = &vertexes[(unsigned short)SHORT(mld->v2)];
	}
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

//
// P_LoadSideDefs
//
void P_LoadSideDefs (int lump)
{
    byte*		data;
    int			i;
    mapsidedef_t*	msd;
    side_t*		sd;
	
    numsides = W_LumpLength (lump) / sizeof(mapsidedef_t);
    sides = Z_Malloc (numsides*sizeof(side_t),PU_LEVEL,0);	
    memset (sides, 0, numsides*sizeof(side_t));
    data = W_CacheLumpNum (lump,PU_STATIC);
	
    msd = (mapsidedef_t *)data;
    sd = sides;
    for (i=0 ; i<numsides ; i++, msd++, sd++)
    {
	sd->textureoffset = SHORT(msd->textureoffset)<<FRACBITS;
	sd->rowoffset = SHORT(msd->rowoffset)<<FRACBITS;
	sd->toptexture = R_TextureNumForName(msd->toptexture);
	sd->bottomtexture = R_TextureNumForName(msd->bottomtexture);
	sd->midtexture = R_TextureNumForName(msd->midtexture);
	sd->sector = &sectors[SHORT(msd->sector)];
    }

    W_ReleaseLumpNum(lump);
}


//
// P_LoadBlockMap
//
boolean P_LoadBlockMap (int lump)
{
    int i;
    int count;
    int lumplen;
    short *wadblockmaplump;

    // [crispy] (re-)create BLOCKMAP if necessary
    if (M_CheckParm("-blockmap") ||
        lump >= numlumps ||
        (lumplen = W_LumpLength(lump)) < 8 ||
        (count = lumplen / 2) >= 0x10000)
    {
	return false;
    }
	
    // [crispy] remove BLOCKMAP limit
    // adapted from boom202s/P_SETUP.C:1025-1076
    wadblockmaplump = Z_Malloc(lumplen, PU_LEVEL, NULL);
    W_ReadLump(lump, wadblockmaplump);
    blockmaplump = Z_Malloc(sizeof(*blockmaplump) * count, PU_LEVEL, NULL);
    blockmap = blockmaplump + 4;

    blockmaplump[0] = SHORT(wadblockmaplump[0]);
    blockmaplump[1] = SHORT(wadblockmaplump[1]);
    blockmaplump[2] = (int32_t)(SHORT(wadblockmaplump[2])) & 0xffff;
    blockmaplump[3] = (int32_t)(SHORT(wadblockmaplump[3])) & 0xffff;

    // Swap all short integers to native byte ordering.
  
    for (i=4; i<count; i++)
    {
	short t = SHORT(wadblockmaplump[i]);
	blockmaplump[i] = (t == -1) ? -1l : (int32_t) t & 0xffff;
    }

    Z_Free(wadblockmaplump);
		
    // Read the header

    bmaporgx = blockmaplump[0]<<FRACBITS;
    bmaporgy = blockmaplump[1]<<FRACBITS;
    bmapwidth = blockmaplump[2];
    bmapheight = blockmaplump[3];
	
    if (crispy_fliplevels)
    {
	int x, y;
	int32_t* rowoffset; // [crispy] BLOCKMAP limit

	bmaporgx += bmapwidth * 128 * FRACUNIT;
	bmaporgx = -bmaporgx;

	for (y = 0; y < bmapheight; y++)
	{
	    rowoffset = blockmap + y * bmapwidth;

	    for (x = 0; x < bmapwidth / 2; x++)
	    {
	        int32_t tmp; // [crispy] BLOCKMAP limit

	        tmp = rowoffset[x];
	        rowoffset[x] = rowoffset[bmapwidth-1-x];
	        rowoffset[bmapwidth-1-x] = tmp;
	    }
	}
    }

    // Clear out mobj chains

    count = sizeof(*blocklinks) * bmapwidth * bmapheight;
    blocklinks = Z_Malloc(count, PU_LEVEL, 0);
    memset(blocklinks, 0, count);

    // [crispy] (re-)create BLOCKMAP if necessary
    return true;
}



//
// P_GroupLines
// Builds sector line lists and subsector sector numbers.
// Finds block bounding boxes for sectors.
//
void P_GroupLines (void)
{
    line_t**		linebuffer;
    int			i;
    int			j;
    line_t*		li;
    sector_t*		sector;
    subsector_t*	ss;
    seg_t*		seg;
    fixed_t		bbox[4];
    int			block;
	
    // look up sector number for each subsector
    ss = subsectors;
    for (i=0 ; i<numsubsectors ; i++, ss++)
    {
	seg = &segs[ss->firstline];
	ss->sector = seg->sidedef->sector;
    }

    // count number of lines in each sector
    li = lines;
    totallines = 0;
    for (i=0 ; i<numlines ; i++, li++)
    {
	totallines++;
	li->frontsector->linecount++;

	if (li->backsector && li->backsector != li->frontsector)
	{
	    li->backsector->linecount++;
	    totallines++;
	}
    }

    // build line tables for each sector	
    linebuffer = Z_Malloc (totallines*sizeof(line_t *), PU_LEVEL, 0);

    for (i=0; i<numsectors; ++i)
    {
        // Assign the line buffer for this sector

        sectors[i].lines = linebuffer;
        linebuffer += sectors[i].linecount;

        // Reset linecount to zero so in the next stage we can count
        // lines into the list.

        sectors[i].linecount = 0;
    }

    // Assign lines to sectors

    for (i=0; i<numlines; ++i)
    { 
        li = &lines[i];

        if (li->frontsector != NULL)
        {
            sector = li->frontsector;

            sector->lines[sector->linecount] = li;
            ++sector->linecount;
        }

        if (li->backsector != NULL && li->frontsector != li->backsector)
        {
            sector = li->backsector;

            sector->lines[sector->linecount] = li;
            ++sector->linecount;
        }
    }
    
    // Generate bounding boxes for sectors
	
    sector = sectors;
    for (i=0 ; i<numsectors ; i++, sector++)
    {
	M_ClearBox (bbox);

	for (j=0 ; j<sector->linecount; j++)
	{
            li = sector->lines[j];

            M_AddToBox (bbox, li->v1->x, li->v1->y);
            M_AddToBox (bbox, li->v2->x, li->v2->y);
	}

	// set the degenmobj_t to the middle of the bounding box
	sector->soundorg.x = (bbox[BOXRIGHT]+bbox[BOXLEFT])/2;
	sector->soundorg.y = (bbox[BOXTOP]+bbox[BOXBOTTOM])/2;
		
	// adjust bounding box to map blocks
	block = (bbox[BOXTOP]-bmaporgy+MAXRADIUS)>>MAPBLOCKSHIFT;
	block = block >= bmapheight ? bmapheight-1 : block;
	sector->blockbox[BOXTOP]=block;

	block = (bbox[BOXBOTTOM]-bmaporgy-MAXRADIUS)>>MAPBLOCKSHIFT;
	block = block < 0 ? 0 : block;
	sector->blockbox[BOXBOTTOM]=block;

	block = (bbox[BOXRIGHT]-bmaporgx+MAXRADIUS)>>MAPBLOCKSHIFT;
	block = block >= bmapwidth ? bmapwidth-1 : block;
	sector->blockbox[BOXRIGHT]=block;

	block = (bbox[BOXLEFT]-bmaporgx-MAXRADIUS)>>MAPBLOCKSHIFT;
	block = block < 0 ? 0 : block;
	sector->blockbox[BOXLEFT]=block;
    }
	
}

// [crispy] remove slime trails
// mostly taken from Lee Killough's implementation in mbfsrc/P_SETUP.C:849-924,
// with the exception that not the actual vertex coordinates are modified,
// but pseudovertexes which are dummies that are *only* used in rendering,
// i.e. r_bsp.c:R_AddLine()

static void P_RemoveSlimeTrails(void)
{
    int i;

    for (i = 0; i < numsegs; i++)
    {
	const line_t *l = segs[i].linedef;
	vertex_t *v = segs[i].v1;

	// [crispy] ignore exactly vertical or horizontal linedefs
	if (l->dx && l->dy)
	{
	    do
	    {
		// [crispy] vertex wasn't already moved
		if (!v->moved)
		{
		    v->moved = true;
		    // [crispy] ignore endpoints of linedefs
		    if (v != l->v1 && v != l->v2)
		    {
			// [crispy] move the vertex towards the linedef
			// by projecting it using the law of cosines
			int64_t dx2 = (l->dx >> FRACBITS) * (l->dx >> FRACBITS);
			int64_t dy2 = (l->dy >> FRACBITS) * (l->dy >> FRACBITS);
			int64_t dxy = (l->dx >> FRACBITS) * (l->dy >> FRACBITS);
			int64_t s = dx2 + dy2;

			// [crispy] MBF actually overrides v->x and v->y here
			v->px = (fixed_t)((dx2 * v->x + dy2 * l->v1->x + dxy * (v->y - l->v1->y)) / s);
			v->py = (fixed_t)((dy2 * v->y + dx2 * l->v1->y + dxy * (v->x - l->v1->x)) / s);

			// [crispy] wait a minute... moved more than 8 map units?
			// maybe that's a linguortal then, back to the original coordinates
			if (abs(v->px - v->x) > 8*FRACUNIT || abs(v->py - v->y) > 8*FRACUNIT)
			{
			    v->px = v->x;
			    v->py = v->y;
			}
		    }
		}
	    // [crispy] if v doesn't point to the second vertex of the seg already, point it there
	    } while ((v != segs[i].v2) && (v = segs[i].v2));
	}
    }
}

// Pad the REJECT lump with extra data when the lump is too small,
// to simulate a REJECT buffer overflow in Vanilla Doom.

static void PadRejectArray(byte *array, unsigned int len)
{
    unsigned int i;
    unsigned int byte_num;
    byte *dest;
    unsigned int padvalue;

    // Values to pad the REJECT array with:

    unsigned int rejectpad[4] =
    {
        0,                                    // Size
        0,                                    // Part of z_zone block header
        50,                                   // PU_LEVEL
        0x1d4a11                              // DOOM_CONST_ZONEID
    };

    rejectpad[0] = ((totallines * 4 + 3) & ~3) + 24;

    // Copy values from rejectpad into the destination array.

    dest = array;

    for (i=0; i<len && i<sizeof(rejectpad); ++i)
    {
        byte_num = i % 4;
        *dest = (rejectpad[i / 4] >> (byte_num * 8)) & 0xff;
        ++dest;
    }

    // We only have a limited pad size.  Print a warning if the
    // REJECT lump is too small.

    if (len > sizeof(rejectpad))
    {
        fprintf(stderr, "PadRejectArray: REJECT lump too short to pad! (%i > %i)\n",
                        len, (int) sizeof(rejectpad));

        // Pad remaining space with 0 (or 0xff, if specified on command line).

        if (M_CheckParm("-reject_pad_with_ff"))
        {
            padvalue = 0xff;
        }
        else
        {
            padvalue = 0xf00;
        }

        memset(array + sizeof(rejectpad), padvalue, len - sizeof(rejectpad));
    }
}

static void P_LoadReject(int lumpnum)
{
    int minlength;
    int lumplen;

    // Calculate the size that the REJECT lump *should* be.

    minlength = (numsectors * numsectors + 7) / 8;

    // If the lump meets the minimum length, it can be loaded directly.
    // Otherwise, we need to allocate a buffer of the correct size
    // and pad it with appropriate data.

    lumplen = W_LumpLength(lumpnum);

    if (lumplen >= minlength)
    {
        rejectmatrix = W_CacheLumpNum(lumpnum, PU_LEVEL);
    }
    else
    {
        rejectmatrix = Z_Malloc(minlength, PU_LEVEL, &rejectmatrix);
        W_ReadLump(lumpnum, rejectmatrix);

        PadRejectArray(rejectmatrix + lumplen, minlength - lumplen);
    }
}

// [crispy] support maps with NODES in compressed or uncompressed ZDBSP
// format or DeePBSP format and/or LINEDEFS and THINGS lumps in Hexen format
static mapformat_t P_CheckMapFormat (int lumpnum)
{
    mapformat_t format = 0;
    byte *nodes = NULL;
    int b;

    if ((b = lumpnum+ML_BLOCKMAP+1) < numlumps &&
        !strncasecmp(lumpinfo[b]->name, "BEHAVIOR", 8))
    {
	fprintf(stderr, "Hexen map format, ");
	format |= MFMT_HEXEN;
    }
    else
	fprintf(stderr, "Doom map format, ");

    if (!((b = lumpnum+ML_NODES) < numlumps &&
        (nodes = W_CacheLumpNum(b, PU_CACHE)) &&
        W_LumpLength(b) > 0))
	fprintf(stderr, "no nodes.\n");
    else
    if (!memcmp(nodes, "xNd4\0\0\0\0", 8))
    {
	fprintf(stderr, "DeePBSP nodes.\n");
	format |= MFMT_DEEPBSP;
    }
    else
    if (!memcmp(nodes, "XNOD", 4))
    {
	fprintf(stderr, "uncompressed ZDBSP nodes.\n");
	format |= MFMT_ZDBSPX;
    }
    else
    if (!memcmp(nodes, "ZNOD", 4))
    {
	fprintf(stderr, "compressed ZDBSP nodes.\n");
	format |= MFMT_ZDBSPZ;
    }
    else
	fprintf(stderr, "normal BSP nodes.\n");

    if (nodes)
	W_ReleaseLumpNum(b);

    return format;
}

// [crispy] log game skill in plain text
const char *skilltable[] =
{
    "No Items",
    "ITYTD",
    "HNTR",
    "HMP",
    "UV",
    "NM"
};

// [crispy] pointer to the current map lump info struct
lumpinfo_t *maplumpinfo;

//
// P_SetupLevel
//
void
P_SetupLevel
( int		episode,
  int		map,
  int		playermask,
  skill_t	skill)
{
    int		i;
    char	lumpname[9];
    int		lumpnum;
    boolean	crispy_validblockmap;
    mapformat_t	crispy_mapformat;
	
    totalkills = totalitems = totalsecret = wminfo.maxfrags = 0;
    // [crispy] count spawned monsters
    extrakills = 0;
    wminfo.partime = 180;
    for (i=0 ; i<MAXPLAYERS ; i++)
    {
	players[i].killcount = players[i].secretcount 
	    = players[i].itemcount = 0;
    }

    // [crispy] No Rest for the Living ...
    if (nervewadfile)
    {
        if (episode == 2)
        {
            gamemission = pack_nerve;
        }
        else
        {
            gamemission = doom2;
        }
    }
    else
    {
        if (gamemission == pack_nerve)
        {
            gameepisode = 2;
        }
    }

    // Initial height of PointOfView
    // will be set by player think.
    players[consoleplayer].viewz = 1; 

    // Make sure all sounds are stopped before Z_FreeTags.
    S_Start ();			

    Z_FreeTags (PU_LEVEL, PU_PURGELEVEL-1);

    // UNUSED W_Profile ();
    P_InitThinkers ();

    // if working with a devlopment map, reload it
    W_Reload ();

    // find map name
    if ( gamemode == commercial)
    {
	if (map<10)
	    DEH_snprintf(lumpname, 9, "map0%i", map);
	else
	    DEH_snprintf(lumpname, 9, "map%i", map);
    }
    else
    {
	lumpname[0] = 'E';
	lumpname[1] = '0' + episode;
	lumpname[2] = 'M';
	lumpname[3] = '0' + map;
	lumpname[4] = 0;
    }

    // [crispy] special-casing for E1M10 "Sewers" support
    if (crispy_havee1m10 && episode == 1 && map == 10)
	DEH_snprintf(lumpname, 9, "E1M10");

    lumpnum = W_GetNumForName (lumpname);
	
    if (nervewadfile && gamemission != pack_nerve && map <= 9)
    {
        lumpnum = W_GetSecondNumForName (lumpname);
    }

    // [crispy] pointer to the current map lump info struct
    maplumpinfo = lumpinfo[lumpnum];

    leveltime = 0;
	
    // [crispy] better logging
    {
	extern int savedleveltime;
	const int ltime = savedleveltime / TICRATE,
	          ttime = (totalleveltimes + savedleveltime) / TICRATE;
	char *rfn_str;

	rfn_str = M_StringJoin(
	    respawnparm ? " -respawn" : "",
	    fastparm ? " -fast" : "",
	    nomonsters ? " -nomonsters" : "",
	    NULL);

	fprintf(stderr, "P_SetupLevel: %s (%s), %s%s, Time %d:%02d:%02d, Total %d:%02d:%02d, ",
	    maplumpinfo->name, maplumpinfo->wad_file->basename,
	    skilltable[BETWEEN(0,5,(int) skill+1)], rfn_str,
	    ltime/3600, (ltime%3600)/60, ltime%60,
	    ttime/3600, (ttime%3600)/60, ttime%60);

	free(rfn_str);
    }
    // [crispy] check and log map and nodes format
    crispy_mapformat = P_CheckMapFormat(lumpnum);

    // note: most of this ordering is important	
    crispy_validblockmap = P_LoadBlockMap (lumpnum+ML_BLOCKMAP); // [crispy] (re-)create BLOCKMAP if necessary
    P_LoadVertexes (lumpnum+ML_VERTEXES);
    P_LoadSectors (lumpnum+ML_SECTORS);
    P_LoadSideDefs (lumpnum+ML_SIDEDEFS);

    if (crispy_mapformat & MFMT_HEXEN)
	P_LoadLineDefs_Hexen (lumpnum+ML_LINEDEFS);
    else
    P_LoadLineDefs (lumpnum+ML_LINEDEFS);
    // [crispy] (re-)create BLOCKMAP if necessary
    if (!crispy_validblockmap)
    {
	extern void P_CreateBlockMap (void);
	P_CreateBlockMap();
    }
    if (crispy_mapformat & (MFMT_ZDBSPX | MFMT_ZDBSPZ))
	P_LoadNodes_ZDBSP (lumpnum+ML_NODES, crispy_mapformat & MFMT_ZDBSPZ);
    else
    if (crispy_mapformat & MFMT_DEEPBSP)
    {
	P_LoadSubsectors_DeePBSP (lumpnum+ML_SSECTORS);
	P_LoadNodes_DeePBSP (lumpnum+ML_NODES);
	P_LoadSegs_DeePBSP (lumpnum+ML_SEGS);
    }
    else
    {
    P_LoadSubsectors (lumpnum+ML_SSECTORS);
    P_LoadNodes (lumpnum+ML_NODES);
    P_LoadSegs (lumpnum+ML_SEGS);
    }

    P_GroupLines ();
    P_LoadReject (lumpnum+ML_REJECT);

    // [crispy] remove slime trails
    P_RemoveSlimeTrails();
    // [crispy] fix long wall wobble
    P_SegLengths();

    bodyqueslot = 0;
    deathmatch_p = deathmatchstarts;
    if (crispy_mapformat & MFMT_HEXEN)
	P_LoadThings_Hexen (lumpnum+ML_THINGS);
    else
    P_LoadThings (lumpnum+ML_THINGS);
    
    // if deathmatch, randomly spawn the active players
    if (deathmatch)
    {
	for (i=0 ; i<MAXPLAYERS ; i++)
	    if (playeringame[i])
	    {
		players[i].mo = NULL;
		G_DeathMatchSpawnPlayer (i);
	    }
			
    }

    // clear special respawning que
    iquehead = iquetail = 0;		
	
    // set up world state
    P_SpawnSpecials ();
	
    // build subsector connect matrix
    //	UNUSED P_ConnectSubsectors ();

    // preload graphics
    if (precache)
	R_PrecacheLevel ();

    //printf ("free memory: 0x%x\n", Z_FreeMemory());

    // [crispy] stop demo warp mode now
    if (crispy_demowarp == map)
    {
	crispy_demowarp = 0;
	nodrawers = false;
	singletics = false;
    }
}



//
// P_Init
//
void P_Init (void)
{
    P_InitSwitchList ();
    P_InitPicAnims ();
    R_InitSprites (sprnames);
}



