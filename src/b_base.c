// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// Copyright(C) 2008 GhostlyDeath (ghostlydeath@gmail.com)
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
brejectinfo_t** BotReject = NULL;
UInt8* BotFinal = NULL;

// Thinkers
bmind_t BotMinds[4];

// Nodes inside sectors (Pointer to an array that has pointers to the nodes)
bnode_t*** BotSectorNodes = NULL;

fixed_t botfloor = 0;
fixed_t botceil = 0;
sector_t* botlastsector = NULL;
sector_t* botcursector = NULL;

/* B_IsSectorDynamic() -- Checks to see if a sector will get it's floor or ceiling heights changed */
int B_IsSectorDynamic(sector_t* in)
{
	size_t i;
	line_t* check;

	if (!in)
		return 0;
		
	/** Check Sector Special **/
	if ((in->special == 10) ||	// Door Opens after 30 seconds
		(in->special == 14))	// Door closes after 5 minutes
		return 1;

	/** Check Sector's Lines for Manual doors **/
	for (i = 0; i < in->linecount; i++)
	{
		check = in->lines[i];
		
		/* Doors */
		if (
			(check->special == 1) ||	// DOOR OPEN WAIT CLOSE
			(check->special == 13) ||	// DOOR OPEN STAY
			(check->special == 117)	||	// DOOR OPEN WAIT CLOSE (FAST)
			(check->special == 118)		// DOOR OPEN STAY (FAST)
			)
			return 1;
	}

#if 0
	/** Check Tagged Lines **/
	if (in->tag)
	{
	}
#endif
		
	return 0;
}

sector_t** tDynList = NULL;
sector_t** Brover = NULL;
int RecordAfterSector = 0;

int B_PTR_LinePossible(intercept_t* in)
{
	line_t* li = NULL;
	subsector_t* sub = NULL;
	mobj_t* thing = NULL;
	int newsector = 0;
	
	// We hit a line
	if (in->isaline)
	{
		li = in->d.line;
		
		// If this line isn't two sided or it blocks things, this done
		if (!(li->flags & ML_TWOSIDED) || (li->flags & ML_BLOCKING))
			return false;	// Stop
		
/*		// Can't fit in the sector
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
		}*/
				
		/*** Use different sector mechanics ***/
		// Hit the front of a line from the same sector
		if (li->frontsector == botcursector)
		{
			// Back of the line points to a new sector
			if (li->backsector != botcursector)
			{
				botlastsector = botcursector;
				botcursector = li->backsector;
				newsector = 1;
			}
			// Front of the line points to the same sector
			/* ... */
		}
		
		// Hit the back of a line from the same sector
		else if (li->backsector == botcursector)
		{
			// Front of the line points to a new sector
			if (li->frontsector != botcursector)
			{
				botlastsector = botcursector;
				botcursector = li->frontsector;
				newsector = 1;
			}
			// Back of the line points to the same sector
			/* ... */
		}
		// Other cases are ignored such as line front and backs in the same sector
		// and floating lines
		
		/* Compare current sector to last sector -- if it changed */
		if (newsector)
		{
			// A previous loop set this, we record the sector after the last
			if (RecordAfterSector)
			{
				if (tDynList && Brover - tDynList < 62)
				{
					*Brover = botlastsector;
					Brover++;
					*Brover = botcursector;
					Brover++;
				}
				
				RecordAfterSector = 0;
			}
			
			/** Dynamic Sector **/
			// Sector is tagged or has a door so check it's heights at run time
			
			if (B_IsSectorDynamic(botcursector))
			{
				// Add to the list
				if (!tDynList)
				{
					tDynList = Z_Malloc(sizeof(sector_t*) * 64, PU_STATIC, NULL);
					memset (tDynList, 0, sizeof(sector_t*) * 64);
					Brover = tDynList;
				}
				
				if (Brover - tDynList < 62)
				{
					*Brover = botlastsector;
					Brover++;
					*Brover = botcursector;
					Brover++;
					
					RecordAfterSector = 1;
				}
				else
					return false;	// TODO: Remove this limit

				return true;	// Always continue on dynamic sectors
			}
			
			/** Static Sector **/
			// Sector has no tags and isn't a door
			
			// Check to see if we can fit in the current sector
			if (((botcursector->ceilingheight - botcursector->floorheight) >> FRACBITS) < 56)
				return false;
			
			// Can't step down since a ceiling is in the way
			else if (((botcursector->ceilingheight - botlastsector->floorheight) >> FRACBITS) < 56)
				return false;
				
			// Can't step up since the floor is too high
			else if (((botcursector->floorheight - botlastsector->floorheight) >> FRACBITS) > 24)
				return false;
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
	botcursector = ssub->sector;
	
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
	sector_t** Rover;
	sector_t** Mover;
	Int16* Temp;
	Int16* Temp2;
	Int16* Zover;
	Int16* Wover;
	Int16 firstnum, lastnum;
	
	if (!M_CheckParm("-bot") && !M_CheckParm("-ingamebots"))
		return;
	
	/* Reset Minds */
	memset(BotMinds, 0, sizeof(BotMinds));
	for (i = 0; i < 4; i++)
	{
		BotMinds[i].player = &players[i];
		BotMinds[i].flags = BF_EXPLORING;
		BotMinds[i].failtype = -1;
	}
	
	/* Free Previous Nodes */
	if (BotNodes)
	{
		for(i = 0; i < NumBotNodes; i++)
			if (BotNodes[i].connections)
				Z_Free(BotNodes[i].connections);
		Z_Free(BotNodes);
	}
		
	if (BotReject)
	{
		for (i = 0; i < NumBotNodes; i++)
		{
			for (j = 0; j < NumBotNodes; j++)
				if (BotReject[i][j].DynSectors)
					Z_Free(BotReject[i][j].DynSectors);
			
			Z_Free(BotReject[i]);
		}
		
		Z_Free(BotReject);
	}
	
	if (BotFinal)
		Z_Free(BotFinal);
	
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
	
	BotReject = Z_Malloc(sizeof(brejectinfo_t*) * NumBotNodes, PU_STATIC, 0);
	memset(BotReject, 0, sizeof(brejectinfo_t*) * NumBotNodes);
	SizeUsed += sizeof(brejectinfo_t*) * NumBotNodes;
	
	for (i = 0; i < NumBotNodes; i++)
	{
		BotReject[i] = Z_Malloc(sizeof(brejectinfo_t) * NumBotNodes, PU_STATIC, 0);
		memset(BotReject[i], 0, sizeof(brejectinfo_t) * NumBotNodes);
		SizeUsed += sizeof(brejectinfo_t) * NumBotNodes;
	}
	
#define FIXEDTODOUBLE(x) (((double)(x)) / 65536.0)
#define DOUBLETOFIXED(x) ((double)((x) * 65536.0))
	
	/* Build Basic Node Table */
	if (botparm)
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
	
	if (botparm)
		printf("Done!\n");
	
	/* Prepare Sector Table */
		// This table contains a NULL terminated list of subsectors in a sector
	if (botparm)
	{
		printf("Building sector table...");
		printf("allocating...");
	}
	subsectorcount = Z_Malloc(sizeof(int) * NumBotSectors, PU_STATIC, 0);
	memset(subsectorcount, 0, sizeof(int) * NumBotSectors);
	subsectorit = Z_Malloc(sizeof(int) * NumBotSectors, PU_STATIC, 0);
	memset(subsectorit, 0, sizeof(int) * NumBotSectors);
	
	if (botparm)
		printf("counting...");
	for (i = 0; i < NumBotNodes; i++)
		subsectorcount[BotNodes[i].subsector->sector - sectors]++;
		
	BotSectorNodes = Z_Malloc(sizeof(bnode_t**) * NumBotSectors, PU_STATIC, 0);
	memset(BotSectorNodes, 0, sizeof(bnode_t**) * NumBotSectors);
	SizeUsed += sizeof(bnode_t**) * NumBotSectors;
	
	// Allocate space for pointer arrays
	if (botparm)
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
	if (botparm)
		printf("creating list...");
	for (i = 0; i < NumBotNodes; i++)
	{
		BotSectorNodes[BotNodes[i].subsector->sector - sectors][subsectorit[BotNodes[i].subsector->sector - sectors]] = BotNodes[i].subsector;
		subsectorit[BotNodes[i].subsector->sector - sectors]++;
	}
	
	Z_Free(subsectorit);
	Z_Free(subsectorcount);
	if (botparm)
		printf("Done!\n");
		
	/* For Searching trees */
	BotFinal = Z_Malloc(sizeof(UInt8) * NumBotNodes, PU_STATIC, 0);
	memset(BotFinal, 0, sizeof(UInt8) * NumBotNodes);
	SizeUsed += sizeof(UInt8) * NumBotNodes;
	
	/* Subsector lookup table */
	if (botparm)
		printf("Creating bot reject...\n");
	for (i = 0; i < NumBotNodes; i++)
	{
		if (botparm)
			printf("\r%03i%% Complete", (int)(((float)i / (float)NumBotNodes) * (float)100));
			
		for (j = 0; j < NumBotNodes; j++)
		{	
			Rover = NULL;
			RecordAfterSector = 0;
			
			if (i == j)
				BotReject[i][j].Mode = 1;
			else if (B_LinePossible(BotNodes[i].x, BotNodes[i].y, BotNodes[i].subsector,
								BotNodes[j].x, BotNodes[j].y, BotNodes[j].subsector))
			{
				BotReject[i][j].Mode = 1;
				
				// Dynamic Sector lists
				if (tDynList)
				{
					k = 0;
					
					Rover = tDynList;
					while (*Rover)
					{
						k++;
						Rover++;
					}
					
					BotReject[i][j].DynSectors = Z_Malloc(sizeof(sector_t*) * (k + 1), PU_STATIC, NULL);
					memset(BotReject[i][j].DynSectors, 0, sizeof(sector_t*) * (k + 1));
					memcpy(BotReject[i][j].DynSectors, tDynList, sizeof(sector_t*) * k);
					SizeUsed += sizeof(sector_t*) * (k + 1);
				}
				
				if (tDynList)
				{
					Z_Free(tDynList);
					tDynList = NULL;
				}
			}
		}
	}
	
	if (botparm)
		printf("Done!\n");
	
	/* Setup Connections */
	if (botparm)
		printf("Setting up connections...");
		
	Temp = Z_Malloc(sizeof(Int16) * NumBotNodes, PU_STATIC, 0);
	Temp2 = Z_Malloc(sizeof(Int16) * NumBotNodes, PU_STATIC, 0);
	for (i = 0; i < NumBotNodes; i++)
	{
		// Clear
		memset(Temp, 0, sizeof(Int16) * NumBotNodes);
		memset(Temp2, 0, sizeof(Int16) * NumBotNodes);
		Zover = Temp;
		firstnum = 0;
		lastnum = 0;
		
		for (j = 0; j < NumBotNodes; j++)
		{
			// Ignore self
			if (i == j)
				continue;
				
			// Check reject -- always accept dynamic sectors too
			if (BotReject[i][j].Mode)
			{
				*Zover = j;
				Zover++;
			}
		}
		
		*Zover = -3;
		
		/* Count how many entries we will really create... */
		k = 1;
		Zover = Temp;
		while (*Zover != -3)
		{
			firstnum = *Zover;
			lastnum = *Zover;
		
			while (*Zover == (*(Zover+1) - 1))
			{
				lastnum = *Zover;
				Zover++;
			}
		
			// Alone
			if (firstnum == lastnum)
				k++;
			// Group
			else
				k += 3;
				
			Zover++;
		}
		
		// Create List
		BotNodes[i].connections = Z_Malloc(sizeof(Int16) * k, PU_STATIC, NULL);
		memset(BotNodes[i].connections, 0, sizeof(Int16) * k);
		SizeUsed += sizeof(Int16) * k;
		
		// Copy list
		Zover = Temp;
		Wover = BotNodes[i].connections;
		while (*Zover != -3)
		{
			firstnum = *Zover;
			lastnum = *Zover;
		
			while (*Zover == (*(Zover+1) - 1))
			{
				lastnum = *Zover;
				Zover++;
			}
		
			// Alone
			if (firstnum == lastnum)
			{
				*Wover = firstnum;
				Wover++;
			}
			// Group
			else
			{
				*Wover = -1;
				Wover++;
				*Wover = firstnum;
				Wover++;
				*Wover = lastnum;
				Wover++;
			}
				
			Zover++;
		}
		
		*Wover = -2;
	}
	
	Z_Free(Temp2);
	Z_Free(Temp);
	
	if (botparm)
		printf("Done!\n");
				
	// Spawn a candle
	if (botparm)
		for (i = 0; i < NumBotNodes; i++)
			P_SpawnMobj(
				BotNodes[i].x,
				BotNodes[i].y,
				BotNodes[i].subsector->sector->floorheight,
				MT_MISC49);
				
	if (botparm)		
		printf("Used %lu bytes (%lu KiB, %lu MiB) for bot navigation\n", SizeUsed, SizeUsed >> 10, SizeUsed >> 20);
	
	Z_CheckHeap();
}

int B_CheckLine(bmind_t* mind, subsector_t* src, subsector_t* dest)
{
	brejectinfo_t* rej = &BotReject[src - subsectors][dest - subsectors];
	sector_t* a = NULL;
	sector_t* b = NULL;
	sector_t** Rover = NULL;
	
	if (rej->Mode)
	{
		if (mind)
		{
			// Keys?
			if ((rej->Mode & BRM_YELLOWKEY) || (rej->Mode & BRM_YELLOWSKULL))
			{
				if (!mind->player->cards[it_yellowcard] || !mind->player->cards[it_yellowskull])
					return 0;
			}
			else if ((rej->Mode & BRM_REDKEY) || (rej->Mode & BRM_REDSKULL))
			{
				if (!mind->player->cards[it_redcard] || !mind->player->cards[it_redskull])
					return 0;
			}
			else if ((rej->Mode & BRM_BLUEKEY) || (rej->Mode & BRM_BLUESKULL))
			{
				if (!mind->player->cards[it_bluecard] || !mind->player->cards[it_blueskull])
					return 0;
			}
		}
		
		// Dynamic Sector Check?
		if (rej->DynSectors)
		{
			Rover = rej->DynSectors;
			
			while (*Rover)
			{
				// Get sector pair
				a = *Rover;
				Rover++;
				b = *Rover;
				Rover++;
				
				// Check to see if we can fit in the current sector
				if (((b->ceilingheight - b->floorheight) >> FRACBITS) < 56)
					return 0;
			
				// Can't step down since a ceiling is in the way
				else if (((b->ceilingheight - a->floorheight) >> FRACBITS) < 56)
					return 0;
				
				// Can't step up since the floor is too high
				else if (((b->floorheight - a->floorheight) >> FRACBITS) > 24)
					return 0;
			}
		}
		
		return 1;
	}
	else
		return 0;
}

