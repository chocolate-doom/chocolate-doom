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

// P_main.c

#include <math.h>
#include <stdlib.h>

#include "doomdef.h"
#include "i_swap.h"
#include "i_system.h"
#include "m_argv.h"
#include "m_bbox.h"
#include "p_local.h"
#include "s_sound.h"
#include "p_extnodes.h"

void P_SpawnMapThing(mapthing_t * mthing);

int numvertexes;
vertex_t *vertexes;

int numsegs;
seg_t *segs;

int numsectors;
sector_t *sectors;

int numsubsectors;
subsector_t *subsectors;

int numnodes;
node_t *nodes;

int numlines;
line_t *lines;

int numsides;
side_t *sides;

int32_t *blockmap;	            // offsets in blockmap are from here // [crispy] BLOCKMAP limit
int32_t *blockmaplump;          // [crispy] BLOCKMAP limit
int bmapwidth, bmapheight;      // in mapblocks
fixed_t bmaporgx, bmaporgy;     // origin of block map
mobj_t **blocklinks;            // for thing chains

byte *rejectmatrix;             // for fast sight rejection

mapthing_t deathmatchstarts[10], *deathmatch_p;
mapthing_t playerstarts[MAXPLAYERS];
boolean playerstartsingame[MAXPLAYERS];

// [crispy] recalculate seg offsets
// adapted from prboom-plus/src/p_setup.c:474-482
fixed_t GetOffset(vertex_t *v1, vertex_t *v2)
{
    fixed_t dx, dy;
    fixed_t r;

    dx = (v1->x - v2->x)>>FRACBITS;
    dy = (v1->y - v2->y)>>FRACBITS;
    r = (fixed_t)(sqrt(dx*dx + dy*dy))<<FRACBITS;

    return r;
}

/*
=================
=
= P_LoadVertexes
=
=================
*/

void P_LoadVertexes(int lump)
{
    byte *data;
    int i;
    mapvertex_t *ml;
    vertex_t *li;

    numvertexes = W_LumpLength(lump) / sizeof(mapvertex_t);
    vertexes = Z_Malloc(numvertexes * sizeof(vertex_t), PU_LEVEL, 0);
    data = W_CacheLumpNum(lump, PU_STATIC);

    ml = (mapvertex_t *) data;
    li = vertexes;
    for (i = 0; i < numvertexes; i++, li++, ml++)
    {
        li->x = SHORT(ml->x) << FRACBITS;
        li->y = SHORT(ml->y) << FRACBITS;

        // [crispy] initialize pseudovertexes with actual vertex coordinates
        li->r_x = li->x;
        li->r_y = li->y;
        li->moved = false;
    }

    W_ReleaseLumpNum(lump);
}


/*
=================
=
= P_LoadSegs
=
=================
*/

void P_LoadSegs(int lump)
{
    byte *data;
    int i;
    mapseg_t *ml;
    seg_t *li;
    line_t *ldef;
    int linedef, side;

    numsegs = W_LumpLength(lump) / sizeof(mapseg_t);
    segs = Z_Malloc(numsegs * sizeof(seg_t), PU_LEVEL, 0);
    memset(segs, 0, numsegs * sizeof(seg_t));
    data = W_CacheLumpNum(lump, PU_STATIC);

    ml = (mapseg_t *) data;
    li = segs;
    for (i = 0; i < numsegs; i++, li++, ml++)
    {
        li->v1 = &vertexes[(unsigned short)SHORT(ml->v1)]; // [crispy] extended nodes
        li->v2 = &vertexes[(unsigned short)SHORT(ml->v2)]; // [crispy] extended nodes

        li->angle = (SHORT(ml->angle)) << 16;
        li->offset = (SHORT(ml->offset)) << 16;
        linedef = (unsigned short)SHORT(ml->linedef); // [crispy] extended nodes
        ldef = &lines[linedef];
        li->linedef = ldef;
        side = SHORT(ml->side);
        li->sidedef = &sides[ldef->sidenum[side]];
        li->frontsector = sides[ldef->sidenum[side]].sector;
        // [crispy] recalculate
        li->offset = GetOffset(li->v1, (ml->side ? ldef->v2 : ldef->v1));
        if (ldef->flags & ML_TWOSIDED)
            li->backsector = sides[ldef->sidenum[side ^ 1]].sector;
        else
            li->backsector = 0;
    }

    W_ReleaseLumpNum(lump);
}

// [crispy] fix long wall wobble

static angle_t anglediff(angle_t a, angle_t b)
{
    if (b > a)
        return anglediff(b, a);

    if (a - b < ANG180)
        return a - b;
    else // [crispy] wrap around
        return b - a;
}

void P_SegLengths(boolean contrast_only)
{
    int i;
    const int rightangle = abs(finesine[(ANG60/2) >> ANGLETOFINESHIFT]);

    for (i = 0; i < numsegs; i++)
    {
        seg_t *const li = &segs[i];
        int64_t dx, dy;

        dx = li->v2->r_x - li->v1->r_x;
        dy = li->v2->r_y - li->v1->r_y;

        if (!contrast_only)
        {
        li->length = (uint32_t)(sqrt((double)dx * dx + (double)dy * dy) / 2);

        // [crispy] re-calculate angle used for rendering
        viewx = li->v1->r_x;
        viewy = li->v1->r_y;
        li->r_angle = R_PointToAngleCrispy(li->v2->r_x, li->v2->r_y);
        // [crispy] more than just a little adjustment?
        // back to the original angle then
        if (anglediff(li->r_angle, li->angle) > ANG60/2)
        {
            li->r_angle = li->angle;
        }
        }

        // [crispy] smoother fake contrast
        if (!dy)
            li->fakecontrast = -LIGHTBRIGHT;
        else
        if (abs(finesine[li->r_angle >> ANGLETOFINESHIFT]) < rightangle)
            li->fakecontrast = -LIGHTBRIGHT / 2;
        else
        if (!dx)
            li->fakecontrast = LIGHTBRIGHT;
        else
        if (abs(finecosine[li->r_angle >> ANGLETOFINESHIFT]) < rightangle)
            li->fakecontrast = LIGHTBRIGHT / 2;
        else
            li->fakecontrast = 0;
    }
}

/*
=================
=
= P_LoadSubsectors
=
=================
*/

void P_LoadSubsectors(int lump)
{
    byte *data;
    int i;
    mapsubsector_t *ms;
    subsector_t *ss;

    numsubsectors = W_LumpLength(lump) / sizeof(mapsubsector_t);
    subsectors = Z_Malloc(numsubsectors * sizeof(subsector_t), PU_LEVEL, 0);
    data = W_CacheLumpNum(lump, PU_STATIC);

    ms = (mapsubsector_t *) data;
    memset(subsectors, 0, numsubsectors * sizeof(subsector_t));
    ss = subsectors;
    for (i = 0; i < numsubsectors; i++, ss++, ms++)
    {
        ss->numlines = (unsigned short)SHORT(ms->numsegs); // [crispy] extended nodes
        ss->firstline = (unsigned short)SHORT(ms->firstseg); // [crispy] extended nodes
    }

    W_ReleaseLumpNum(lump);
}


/*
=================
=
= P_LoadSectors
=
=================
*/

void P_LoadSectors(int lump)
{
    byte *data;
    int i;
    mapsector_t *ms;
    sector_t *ss;

    numsectors = W_LumpLength(lump) / sizeof(mapsector_t);
    sectors = Z_Malloc(numsectors * sizeof(sector_t), PU_LEVEL, 0);
    memset(sectors, 0, numsectors * sizeof(sector_t));
    data = W_CacheLumpNum(lump, PU_STATIC);

    ms = (mapsector_t *) data;
    ss = sectors;
    for (i = 0; i < numsectors; i++, ss++, ms++)
    {
        ss->floorheight = SHORT(ms->floorheight) << FRACBITS;
        ss->ceilingheight = SHORT(ms->ceilingheight) << FRACBITS;
        ss->floorpic = R_FlatNumForName(ms->floorpic);
        ss->ceilingpic = R_FlatNumForName(ms->ceilingpic);
        ss->lightlevel = SHORT(ms->lightlevel);
        // [crispy] A11Y light level used for rendering
        ss->rlightlevel = ss->lightlevel;
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
        // [crispy] inhibit sector interpolation during the 0th gametic
        ss->oldgametic = -1;
    }

    W_ReleaseLumpNum(lump);
}


/*
=================
=
= P_LoadNodes
=
=================
*/

void P_LoadNodes(int lump)
{
    byte *data;
    int i, j, k;
    mapnode_t *mn;
    node_t *no;

    numnodes = W_LumpLength(lump) / sizeof(mapnode_t);
    nodes = Z_Malloc(numnodes * sizeof(node_t), PU_LEVEL, 0);
    data = W_CacheLumpNum(lump, PU_STATIC);

    mn = (mapnode_t *) data;
    no = nodes;
    for (i = 0; i < numnodes; i++, no++, mn++)
    {
        no->x = SHORT(mn->x) << FRACBITS;
        no->y = SHORT(mn->y) << FRACBITS;
        no->dx = SHORT(mn->dx) << FRACBITS;
        no->dy = SHORT(mn->dy) << FRACBITS;
        for (j = 0; j < 2; j++)
        {
            no->children[j] = (unsigned short)SHORT(mn->children[j]); // [crispy] extended nodes

            // [crispy] add support for extended nodes
            // from prboom-plus/src/p_setup.c:937-957
            if (no->children[j] == NO_INDEX)
                no->children[j] = -1;
            else
            if (no->children[j] & NF_SUBSECTOR_VANILLA)
            {
                no->children[j] &= ~NF_SUBSECTOR_VANILLA;

                if (no->children[j] >= numsubsectors)
                    no->children[j] = 0;

                no->children[j] |= NF_SUBSECTOR;
            }

            for (k = 0; k < 4; k++)
                no->bbox[j][k] = SHORT(mn->bbox[j][k]) << FRACBITS;
        }
    }

    W_ReleaseLumpNum(lump);
}



/*
=================
=
= P_LoadThings
=
=================
*/

void P_LoadThings(int lump)
{
    byte *data;
    int i;
    mapthing_t spawnthing;
    mapthing_t *mt;
    int numthings;

    data = W_CacheLumpNum(lump, PU_STATIC);
    numthings = W_LumpLength(lump) / sizeof(mapthing_t);

    mt = (mapthing_t *) data;
    for (i = 0; i < numthings; i++, mt++)
    {
        spawnthing.x = SHORT(mt->x);
        spawnthing.y = SHORT(mt->y);
        spawnthing.angle = SHORT(mt->angle);
        spawnthing.type = SHORT(mt->type);
        spawnthing.options = SHORT(mt->options);
        P_SpawnMapThing(&spawnthing);
    }

    if (!deathmatch)
    {
        for (i = 0; i < MAXPLAYERS; i++)
        {
            if (playeringame[i] && !playerstartsingame[i])
            {
                I_Error("P_LoadThings: Player %d start missing (vanilla crashes here)", i + 1);
            }
            playerstartsingame[i] = false;
        }
    }

    W_ReleaseLumpNum(lump);
}



/*
=================
=
= P_LoadLineDefs
=
= Also counts secret lines for intermissions
=================
*/

void P_LoadLineDefs(int lump)
{
    byte *data;
    int i;
    maplinedef_t *mld;
    line_t *ld;
    vertex_t *v1, *v2;

    numlines = W_LumpLength(lump) / sizeof(maplinedef_t);
    lines = Z_Malloc(numlines * sizeof(line_t), PU_LEVEL, 0);
    memset(lines, 0, numlines * sizeof(line_t));
    data = W_CacheLumpNum(lump, PU_STATIC);

    mld = (maplinedef_t *) data;
    ld = lines;
    for (i = 0; i < numlines; i++, mld++, ld++)
    {
        ld->flags = (unsigned short)SHORT(mld->flags); // [crispy] extended nodes
        ld->special = SHORT(mld->special);
        ld->tag = SHORT(mld->tag);
        v1 = ld->v1 = &vertexes[(unsigned short)SHORT(mld->v1)]; // [crispy] extended nodes
        v2 = ld->v2 = &vertexes[(unsigned short)SHORT(mld->v2)]; // [crispy] extended nodes
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
        if (ld->sidenum[0] != NO_INDEX) // [crispy] extended nodes
            ld->frontsector = sides[ld->sidenum[0]].sector;
        else
            ld->frontsector = 0;
        if (ld->sidenum[1] != NO_INDEX) // [crispy] extended nodes
            ld->backsector = sides[ld->sidenum[1]].sector;
        else
            ld->backsector = 0;
    }

    W_ReleaseLumpNum(lump);
}


/*
=================
=
= P_LoadSideDefs
=
=================
*/

void P_LoadSideDefs(int lump)
{
    byte *data;
    int i;
    mapsidedef_t *msd;
    side_t *sd;

    numsides = W_LumpLength(lump) / sizeof(mapsidedef_t);
    sides = Z_Malloc(numsides * sizeof(side_t), PU_LEVEL, 0);
    memset(sides, 0, numsides * sizeof(side_t));
    data = W_CacheLumpNum(lump, PU_STATIC);

    msd = (mapsidedef_t *) data;
    sd = sides;
    for (i = 0; i < numsides; i++, msd++, sd++)
    {
        sd->textureoffset = SHORT(msd->textureoffset) << FRACBITS;
        sd->rowoffset = SHORT(msd->rowoffset) << FRACBITS;
        sd->toptexture = R_TextureNumForName(msd->toptexture);
        sd->bottomtexture = R_TextureNumForName(msd->bottomtexture);
        sd->midtexture = R_TextureNumForName(msd->midtexture);
        sd->sector = &sectors[SHORT(msd->sector)];
        // [crispy] smooth texture scrolling
        sd->basetextureoffset = sd->textureoffset;
    }

    W_ReleaseLumpNum(lump);
}



/*
=================
=
= P_LoadBlockMap
=
=================
*/

boolean P_LoadBlockMap(int lump)
{
    int i, count;
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
    wadblockmaplump = Z_Malloc(lumplen, PU_LEVEL, NULL);
    W_ReadLump(lump, wadblockmaplump);
    blockmaplump = Z_Malloc(sizeof(*blockmaplump) * count, PU_LEVEL, NULL);
    blockmap = blockmaplump + 4;

    blockmaplump[0] = SHORT(wadblockmaplump[0]);
    blockmaplump[1] = SHORT(wadblockmaplump[1]);
    blockmaplump[2] = (int32_t)(SHORT(wadblockmaplump[2])) & 0xffff;
    blockmaplump[3] = (int32_t)(SHORT(wadblockmaplump[3])) & 0xffff;

    // Swap all short integers to native byte ordering:

    // count = lumplen / 2; // [crispy] moved up
    for (i=4; i<count; i++)
    {
        short t = SHORT(wadblockmaplump[i]);
        blockmaplump[i] = (t == -1) ? -1l : (int32_t) t & 0xffff;
    }

    Z_Free(wadblockmaplump);

    bmaporgx = blockmaplump[0] << FRACBITS;
    bmaporgy = blockmaplump[1] << FRACBITS;
    bmapwidth = blockmaplump[2];
    bmapheight = blockmaplump[3];

// clear out mobj chains
    count = sizeof(*blocklinks) * bmapwidth * bmapheight;
    blocklinks = Z_Malloc(count, PU_LEVEL, 0);
    memset(blocklinks, 0, count);

    // [crispy] (re-)create BLOCKMAP if necessary
    return true;
}




/*
=================
=
= P_GroupLines
=
= Builds sector line lists and subsector sector numbers
= Finds block bounding boxes for sectors
= [crispy] updated old Doom 1.2 code with actual implementation of P_GroupLines
= from Chocolate Doom for better handling and faster loading of complex levels
=================
*/

void P_GroupLines(void)
{
    line_t **linebuffer;
    int i, j, total;
    line_t *li;
    sector_t *sector;
    subsector_t *ss;
    seg_t *seg;
    fixed_t bbox[4];
    int block;

// look up sector number for each subsector
    ss = subsectors;
    for (i = 0; i < numsubsectors; i++, ss++)
    {
        seg = &segs[ss->firstline];
        ss->sector = seg->sidedef->sector;
    }

// count number of lines in each sector
    li = lines;
    total = 0;
    for (i = 0; i < numlines; i++, li++)
    {
        total++;
        li->frontsector->linecount++;
        if (li->backsector && li->backsector != li->frontsector)
        {
            li->backsector->linecount++;
            total++;
        }
    }

// build line tables for each sector    
    linebuffer = Z_Malloc(total * sizeof(line_t *), PU_LEVEL, 0);
    for (i = 0; i < numsectors; ++i)
    {
        // Assign the line buffer for this sector
        sectors[i].lines = linebuffer;
        linebuffer += sectors[i].linecount;

        // Reset linecount to zero so in the next stage we can count
        // lines into the list.
        sectors[i].linecount = 0;
    }

// [crispy] assign lines to sectors
    for (i = 0; i < numlines; ++i)
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

// [crispy] generate bounding boxes for sectors
    sector = sectors;
    for (i = 0; i < numsectors; i++, sector++)
    {
        M_ClearBox(bbox);
        for (j = 0; j < sector->linecount; j++)
        {
                li = sector->lines[j];
        
                M_AddToBox(bbox, li->v1->x, li->v1->y);
                M_AddToBox(bbox, li->v2->x, li->v2->y);
        }

        // set the degenmobj_t to the middle of the bounding box
        sector->soundorg.x = (bbox[BOXRIGHT] + bbox[BOXLEFT]) / 2;
        sector->soundorg.y = (bbox[BOXTOP] + bbox[BOXBOTTOM]) / 2;

        // adjust bounding box to map blocks
        block = (bbox[BOXTOP] - bmaporgy + MAXRADIUS) >> MAPBLOCKSHIFT;
        block = block >= bmapheight ? bmapheight - 1 : block;
        sector->blockbox[BOXTOP] = block;

        block = (bbox[BOXBOTTOM] - bmaporgy - MAXRADIUS) >> MAPBLOCKSHIFT;
        block = block < 0 ? 0 : block;
        sector->blockbox[BOXBOTTOM] = block;

        block = (bbox[BOXRIGHT] - bmaporgx + MAXRADIUS) >> MAPBLOCKSHIFT;
        block = block >= bmapwidth ? bmapwidth - 1 : block;
        sector->blockbox[BOXRIGHT] = block;

        block = (bbox[BOXLEFT] - bmaporgx - MAXRADIUS) >> MAPBLOCKSHIFT;
        block = block < 0 ? 0 : block;
        sector->blockbox[BOXLEFT] = block;
    }

}

//=============================================================================

// [crispy] remove slime trails
// mostly taken from Lee Killough's implementation in mbfsrc/P_SETUP.C:849-924,
// with the exception that not the actual vertex coordinates are modified,
// but separate coordinates that are *only* used in rendering,
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
			v->r_x = (fixed_t)((dx2 * v->x + dy2 * l->v1->x + dxy * (v->y - l->v1->y)) / s);
			v->r_y = (fixed_t)((dy2 * v->y + dx2 * l->v1->y + dxy * (v->x - l->v1->x)) / s);

			// [crispy] wait a minute... moved more than 8 map units?
			// maybe that's a linguortal then, back to the original coordinates
			if (abs(v->r_x - v->x) > 8*FRACUNIT || abs(v->r_y - v->y) > 8*FRACUNIT)
			{
			    v->r_x = v->x;
			    v->r_y = v->y;
			}
		    }
		}
	    // [crispy] if v doesn't point to the second vertex of the seg already, point it there
	    } while ((v != segs[i].v2) && (v = segs[i].v2));
	}
    }
}

lumpinfo_t *maplumpinfo;

/*
=================
=
= P_SetupLevel
=
=================
*/

void P_SetupLevel(int episode, int map, int playermask, skill_t skill)
{
    int i;
    int parm;
    char lumpname[9];
    int lumpnum;
    mobj_t *mobj;
    boolean crispy_validblockmap;
    mapformat_t crispy_mapformat;

    totalkills = totalitems = totalsecret = 0;
    for (i = 0; i < MAXPLAYERS; i++)
    {
        players[i].killcount = players[i].secretcount
            = players[i].itemcount = 0;
    }
    players[consoleplayer].viewz = 1;   // will be set by player think

    // [crispy] stop demo warp mode now
    if (crispy->demowarp == map)
    {
        crispy->demowarp = 0;
        nodrawers = false;
        singletics = false;
    }

    S_Start();                  // make sure all sounds are stopped before Z_FreeTags

    Z_FreeTags(PU_LEVEL, PU_PURGELEVEL - 1);

    P_InitThinkers();

//
// look for a regular (development) map first
//
    lumpname[0] = 'E';
    lumpname[1] = '0' + episode;
    lumpname[2] = 'M';
    lumpname[3] = '0' + map;
    lumpname[4] = 0;
    leveltime = 0;
    oldleveltime = 0;  // [crispy] Track if game is running

    lumpnum = W_GetNumForName(lumpname);

    // [crispy] check and log map and nodes format
    crispy_mapformat = P_CheckMapFormat(lumpnum);

    maplumpinfo = lumpinfo[lumpnum];

// note: most of this ordering is important     
    crispy_validblockmap = P_LoadBlockMap(lumpnum + ML_BLOCKMAP); // [crispy] (re-)create BLOCKMAP if necessary
    P_LoadVertexes(lumpnum + ML_VERTEXES);
    P_LoadSectors(lumpnum + ML_SECTORS);
    P_LoadSideDefs(lumpnum + ML_SIDEDEFS);

    P_LoadLineDefs(lumpnum + ML_LINEDEFS);

    // [crispy] (re-)create BLOCKMAP if necessary
    if (!crispy_validblockmap)
    {
        P_CreateBlockMap();
    }

    if (crispy_mapformat & (MFMT_ZDBSPX | MFMT_ZDBSPZ))
    {
        P_LoadNodes_ZDBSP(lumpnum + ML_NODES, crispy_mapformat & MFMT_ZDBSPZ);
    }
    else if (crispy_mapformat & MFMT_DEEPBSP)
    {
        P_LoadSubsectors_DeePBSP(lumpnum + ML_SSECTORS);
        P_LoadNodes_DeePBSP(lumpnum + ML_NODES);
        P_LoadSegs_DeePBSP(lumpnum + ML_SEGS);
    }
    else
    {
    P_LoadSubsectors(lumpnum + ML_SSECTORS);
    P_LoadNodes(lumpnum + ML_NODES);
    P_LoadSegs(lumpnum + ML_SEGS);
    }

    rejectmatrix = W_CacheLumpNum(lumpnum + ML_REJECT, PU_LEVEL);
    P_GroupLines();

    // [crispy] remove slime trails
    P_RemoveSlimeTrails();

    // [crispy] fix long wall wobble
    P_SegLengths(false);

    bodyqueslot = 0;
    deathmatch_p = deathmatchstarts;
    P_InitAmbientSound();
    P_InitMonsters();
    P_OpenWeapons();
    P_LoadThings(lumpnum + ML_THINGS);
    P_CloseWeapons();

//
// if deathmatch, randomly spawn the active players
//
    TimerGame = 0;
    if (deathmatch)
    {
        for (i = 0; i < MAXPLAYERS; i++)
        {
            if (playeringame[i])
            {                   // must give a player spot before deathmatchspawn
                mobj = P_SpawnMobj(playerstarts[i].x << 16,
                                   playerstarts[i].y << 16, 0, MT_PLAYER);
                players[i].mo = mobj;
                G_DeathMatchSpawnPlayer(i);
                P_RemoveMobj(mobj);
            }
        }

        //!
        // @arg <n>
        // @category net
        // @vanilla
        //
        // For multiplayer games: exit each level after n minutes.
        //

        parm = M_CheckParmWithArgs("-timer", 1);
        if (parm)
        {
            TimerGame = atoi(myargv[parm + 1]) * 35 * 60;
        }
    }

// set up world state
    P_SpawnSpecials();

// build subsector connect matrix
//      P_ConnectSubsectors ();

// preload graphics
    if (precache)
        R_PrecacheLevel();

//printf ("free memory: 0x%x\n", Z_FreeMemory());

}


/*
=================
=
= P_Init
=
=================
*/

void P_Init(void)
{
    P_InitSwitchList();
    P_InitPicAnims();
    P_InitTerrainTypes();
    P_InitLava();
    R_InitSprites(sprnames);
}
