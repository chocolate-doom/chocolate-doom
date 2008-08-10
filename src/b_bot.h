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

#ifndef __B_BOT_H__
#define __B_BOT_H__

#include "doomdef.h"
#include "doomstat.h"
#include "d_ticcmd.h"
#include "p_mobj.h"
#include "d_player.h"
#include "m_random.h"
#include "p_local.h"
#include "d_event.h"

#define MAXPATHSEGMENTS 64

typedef struct bnode_s
{
	fixed_t x;
	fixed_t y;
	subsector_t* subsector;
} bnode_t;

typedef struct bmind_s
{
	player_t* player;
	UInt16 flags;
	
	/* Path */
	int PathIterator;
	bnode_t* PathNodes[MAXPATHSEGMENTS];
	int priority;
	int failcount;
	int failtype;
	
	/* Gathering */
	mobj_t* GatherTarget;
	
	/* Attacking */
	mobj_t* AttackTarget;
	int pistoltimeout;
	int chainguntimeout;
	int sidetics;
	int forwardtics;
	
} bmind_t;

#define BMC_PLAYER		1
#define BMC_MONSTER		2
#define BMC_WEAPON		3
#define BMC_AMMO		4
#define BMC_HEALTH		5
#define BMC_ARMOR		6

typedef struct bmocheck_s
{
	int (*func)(bmind_t*, mobj_t*, struct bmocheck_s*, void*);
	int type;
	void* poff;
	int pform;		// Goes into the priority formula
} bmocheck_t;

/** Instead of having multiple arrays which will waste space in the future? **/
/*
	such as: sector_t**** BotDynSectors
			1d --> Source SubSector (array -> destination)
			2d --> Destination SubSector (array -> pointer list)
			3d --> NULL terminated list of dynamic sectors on the way (array -> contents)
			$$ --> Pointer to a sector
			
		I'm on 64-bit so a pointer is 8 bytes, then let's say there are 100
		nodes... this is a three dimensional array, but let's assume no path
		has dynamic sectors in the way...

		[i]         [j]
		(8 * 100) * (8 * 100) =
			  800 *       800 = 640,000 Bytes (625 KiB)
		
	original was: UInt** BotReject
			1d --> Source Subsector (array -> destination value)
			2d --> Destination Subsector
			$$ --> 1 if a path is possible
	
		[i]         [j]
		(8 * 100) * (1 * 100) =
		      800 *       100 = 80,000 Bytes (78 KiB)
	          
	new is:
			1d --> Source Subsector (array->destination
			2d --> Destination Subsector
			$$ --> Structure
		
		Struct is 1 + 8 = 9
		
		[i]         [j]
		(8 * 100) * (9 * 100) =
		      800 *       900 = 720,000 Bytes (703 KiB)
		
	totals:
			  640,000 Bytes (625 KiB)
			+  80,000 Bytes ( 78 KiB)
		=============================
		      720,000 Bytes (703 KiB)
		      
	I don't know but it could save in the future? since the first bit is always
	8 * 100
*/

#define BRM_BLUEKEY		2	// A dynamic sector requires a blue key
#define BRM_REDKEY		4	// "    "      "        "    a red key
#define BRM_YELLOWKEY	8	// "    "      "        "    a yellow key
#define BRM_BLUESKULL	16	// "    "      "        "    a blue skull
#define BRM_REDSKULL	32	// "    "      "        "    a red skull
#define BRM_YELLOWSKULL	64	// "    "      "        "    a yellow skull

typedef struct brejectinfo_s
{
	UInt8 Mode;				// if first bit is set, path is possible
	
	sector_t** DynSectors;	// Sectors that must be checked to see if there is
							// a clear path. Dynamic sectors 
	// Checked in this order ---->
	// [First Sector] [Next Sector] ...
} brejectinfo_t;

extern brejectinfo_t** BotReject;

extern bmocheck_t BotMobjCheck[NUMMOBJTYPES];
extern size_t NumBotNodes;
extern size_t NumBotSectors;
extern bnode_t* BotNodes;
extern bmind_t BotMinds[4];
extern bnode_t*** BotSectorNodes;

int B_CheckLine(bmind_t* mind, subsector_t* src, subsector_t* dest);
#define B_CheckLineInt(a,s,d) B_CheckLine((a), subsectors + (b), subsectors + (d))
int B_BuildPath(bmind_t* mind, subsector_t* src, subsector_t* dest, int flags, int priority);
void B_LookForStuff(bmind_t* mind);
void B_BuildTicCommand(ticcmd_t* cmd, int playernum);
void B_InitializeForLevel(void);
int B_IsWeaponAmmoWorthIt(bmind_t* mind, weapontype_t wp);

#define BOTBADPATH 0x7FFFFFFF

#define BP_CHECKPATH		1

#define BF_EXPLORING		1
#define BF_GATHERING		2
#define BF_ATTACKING		4
#define BF_HUNTING			8
#define BF_DEFENDING		16

#define B_PathDistance(a,b) ((int)sqrt(pow((((b)->x >> FRACBITS) - ((a)->x >> FRACBITS)), 2) + pow((((b)->y >> FRACBITS) - ((a)->y >> FRACBITS)), 2)))
#define ISWEAPONAMMOWORTHIT(a) B_IsWeaponAmmoWorthIt(mind, (a))

#endif /* __B_BOT_H__ */

