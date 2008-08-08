// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// Copyright(C) 2008 GhostlyDeath
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
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
// 02111-1307, USA.
//
// DESCRIPTION:
//      Bot Code
//
//-----------------------------------------------------------------------------

#include "b_bot.h"
#include "z_zone.h"
#include <math.h>

// Number of nodes
size_t NumBotNodes = 0;

// Number of sectors
size_t NumBotSectors = 0;

// Nodes
bnode_t* BotNodes = NULL;

// Reject map for nodes
UInt8** BotReject = NULL;

// Thinkers
bmind_t BotMinds[4];

// Nodes inside sectors (Pointer to an array that has pointers to the nodes)
bnode_t*** BotSectorNodes = NULL;

fixed_t botfloor = 0;
fixed_t botceil = 0;
sector_t* botlastsector = NULL;

int B_PTR_LinePossible(intercept_t* in)
{
	line_t* li = NULL;
	subsector_t* sub = NULL;
	mobj_t* thing = NULL;
	
	// We hit a line
	if (in->isaline)
	{
		li = in->d.line;
		
		// If this line isn't two sided or it blocks things, this done
		if (!(li->flags & ML_TWOSIDED) || (li->flags & ML_BLOCKING))
			return false;	// Stop
		
		// Can't fit in the sector
		if (((li->frontsector->ceilingheight - li->frontsector->floorheight >> FRACBITS) <= 56) ||
			((li->backsector->ceilingheight - li->backsector->floorheight >> FRACBITS) <= 56))
			return false;	// Stop
		
		// Can we step up?
		if (botfloor == li->frontsector->floorheight)
		{
			if (!(((li->backsector->floorheight - botfloor) >> FRACBITS) <= 24))
				return false;
			
			botfloor = li->backsector->floorheight;
		}
		else if (botfloor == li->backsector->floorheight)
		{
			if (!(((li->frontsector->floorheight - botfloor) >> FRACBITS) <= 24))
				return false;
				
			botfloor = li->frontsector->floorheight;
		}
		
		// There may be a ceiling in the way
		if (botceil == li->frontsector->ceilingheight)
		{
			if (li->backsector->ceilingheight <= botfloor)
				return false;
				
			botceil = li->backsector->ceilingheight;
		}
		else if (botceil == li->backsector->ceilingheight)
		{
			if (li->frontsector->ceilingheight <= botfloor)
				return false;
			
			botceil = li->frontsector->ceilingheight;
		}
	}
	// The thing we hit could be something that's solid that we can't destroy
	else if (in->isathing)
	{
		thing = in->d.thing;
		
		if (thing->flags & MF_SOLID && !(thing->flags & MF_SHOOTABLE))
			return false;	// Stop
				
	}
	
	return true;
}

/* B_LinePossible() -- Draws a floor line from one area to another and sees if
                       walking to that location is possible */
int B_LinePossible(
	fixed_t sx, fixed_t sy, subsector_t* ssub,
	fixed_t dx, fixed_t dy, subsector_t* dsub)
{
	botfloor = ssub->sector->floorheight;
	botceil = ssub->sector->ceilingheight;
	botlastsector = ssub->sector;
	
	return P_PathTraverse(sx, sy, dx, dy, PT_ADDLINES | PT_ADDTHINGS, B_PTR_LinePossible);
}

/* B_InitializeForLevel() -- Create nodes and everything */
void B_InitializeForLevel(void)
{
	size_t i, j, k;
	int* subsectorcount = NULL;
	int* subsectorit = NULL;
#if 0
	double PolyArea;
	double CenterX, CenterY;
	double Sum;
	fixed_t* vx = NULL;
	fixed_t* vy = NULL;
#endif
	double dx, dy;
	fixed_t tx, ty;
	size_t SizeUsed = 0;
	
	if (!M_CheckParm("-bot") && !M_CheckParm("-ingamebots"))
		return;
	
	/* Reset Minds */
	memset(BotMinds, 0, sizeof(BotMinds));
	for (i = 0; i < 4; i++)
	{
		BotMinds[i].player = &players[i];
		BotMinds[i].flags = BF_EXPLORING;
	}
	
	/* Free Previous Nodes */
	if (BotNodes)
		Z_Free(BotNodes);
		
	if (BotReject)
	{
		for (i = 0; i < NumBotNodes; i++)
			Z_Free(BotReject[i]);
		Z_Free(BotReject);
	}
	
	if (BotSectorNodes)
	{
		for (i = 0; i < NumBotSectors; i++)
			if (BotSectorNodes[i])
				Z_Free(BotSectorNodes[i]);
		Z_Free(BotSectorNodes);
	}
	
	/* Prepare node table */
	NumBotNodes = numsubsectors;
	NumBotSectors = numsectors;
	
	BotNodes = Z_Malloc(sizeof(bnode_t) * NumBotNodes, PU_STATIC, 0);
	memset(BotNodes, 0, sizeof(bnode_t) * NumBotNodes);
	SizeUsed += sizeof(bnode_t) * NumBotNodes;
	
	BotReject = Z_Malloc(sizeof(UInt8*) * NumBotNodes, PU_STATIC, 0);
	memset(BotReject, 0, sizeof(UInt8*) * NumBotNodes);
	SizeUsed += sizeof(UInt8*) * NumBotNodes;
	
	for (i = 0; i < NumBotNodes; i++)
	{
		BotReject[i] = Z_Malloc(sizeof(UInt8) * NumBotNodes, PU_STATIC, 0);
		memset(BotReject[i], 0, sizeof(UInt8) * NumBotNodes);
		SizeUsed += sizeof(UInt8) * NumBotNodes;
	}
	
#define FIXEDTODOUBLE(x) (((double)(x)) / 65536.0)
#define DOUBLETOFIXED(x) ((double)((x) * 65536.0))
	
	/* Build Basic Node Table */
	printf("Building node table...");
	for (i = 0; i < NumBotNodes; i++)
	{
		tx = 0;
		ty = 0;
		dx = 0;
		dy = 0;
		
		for (j = 0; j < subsectors[i].numlines; j++)
		{
			dx += FIXEDTODOUBLE(segs[subsectors[i].firstline + j].v1->x);
			dy += FIXEDTODOUBLE(segs[subsectors[i].firstline + j].v1->y);
		}
		
		BotNodes[i].x = DOUBLETOFIXED(dx / (double)subsectors[i].numlines);
		BotNodes[i].y = DOUBLETOFIXED(dy / (double)subsectors[i].numlines);
		BotNodes[i].subsector = &subsectors[i];
	}
	printf("Done!\n");
	
	/* Prepare Sector Table */
		// This table contains a NULL terminated list of subsectors in a sector
	printf("Building sector table...");
	printf("allocating...");
	subsectorcount = Z_Malloc(sizeof(int) * NumBotSectors, PU_STATIC, 0);
	memset(subsectorcount, 0, sizeof(int) * NumBotSectors);
	subsectorit = Z_Malloc(sizeof(int) * NumBotSectors, PU_STATIC, 0);
	memset(subsectorit, 0, sizeof(int) * NumBotSectors);
	
	printf("counting...");
	for (i = 0; i < NumBotNodes; i++)
		subsectorcount[BotNodes[i].subsector->sector - sectors]++;
		
	BotSectorNodes = Z_Malloc(sizeof(bnode_t**) * NumBotSectors, PU_STATIC, 0);
	memset(BotSectorNodes, 0, sizeof(bnode_t**) * NumBotSectors);
	SizeUsed += sizeof(bnode_t**) * NumBotSectors;
	
	// Allocate space for pointer arrays
	printf("allocating sub lists...");
	for (i = 0; i < NumBotSectors; i++)
	{
		if (subsectorcount[i])
		{
			BotSectorNodes[i] = Z_Malloc((sizeof(bnode_t*) * subsectorcount[i]) + sizeof(bnode_t*), PU_STATIC, 0);
			memset(BotSectorNodes[i], 0, (sizeof(bnode_t*) * subsectorcount[i]) + sizeof(bnode_t*));
			SizeUsed += (sizeof(bnode_t*) * subsectorcount[i]) + sizeof(bnode_t*);
		}
		else
			BotSectorNodes[i] = NULL;
	}
	
	// Create NULL Terminated list
	printf("creating list...");
	for (i = 0; i < NumBotNodes; i++)
	{
		BotSectorNodes[BotNodes[i].subsector->sector - sectors][subsectorit[BotNodes[i].subsector->sector - sectors]] = BotNodes[i].subsector;
		subsectorit[BotNodes[i].subsector->sector - sectors]++;
	}
	
	Z_Free(subsectorit);
	Z_Free(subsectorcount);
	printf("Done!\n");
	
	/* Subsector lookup table */
	printf("Creating bot reject...\n");
	for (i = 0; i < NumBotNodes; i++)
	{
		printf("\r%03i%% Complete", (int)(((float)i / (float)NumBotNodes) * (float)100));
			
		for (j = 0; j < NumBotNodes; j++)
		{	
			if (i == j)
				BotReject[i][j] = 1;
			else if (B_LinePossible(BotNodes[i].x, BotNodes[i].y, BotNodes[i].subsector,
								BotNodes[j].x, BotNodes[j].y, BotNodes[j].subsector))
				BotReject[i][j] = 1;
		}
	}
	printf("Done!\n");
				
	// Spawn a candle
	if (botparm)
		for (i = 0; i < NumBotNodes; i++)
			P_SpawnMobj(
				BotNodes[i].x,
				BotNodes[i].y,
				BotNodes[i].subsector->sector->floorheight,
				MT_MISC49);
			
	printf("Used %lu bytes (%lu KiB, %lu MiB) for bot navigation\n", SizeUsed, SizeUsed >> 10, SizeUsed >> 20);
}

