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
	int count;
	subsector_t* subsector;
} bnode_t;

typedef struct bmind_s
{
	player_t* player;
	UInt16 flags;
	
	/* Path */
	int PathIterator;
	bnode_t* PathNodes[MAXPATHSEGMENTS];
	
	/* Gathering */
	mobj_t* GatherTarget;
	
	/* Attacking */
	mobj_t* AttackTarget;
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
} bmocheck_t;

extern bmocheck_t BotMobjCheck[NUMMOBJTYPES];
extern size_t NumBotNodes;
extern size_t NumBotSectors;
extern bnode_t* BotNodes;
extern UInt8** BotReject;
extern bmind_t BotMinds[4];
extern bnode_t*** BotSectorNodes;

int B_BuildPath(bmind_t* mind, subsector_t* src, subsector_t* dest, int flags);
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

