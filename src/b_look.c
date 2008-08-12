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

// Blank Player for offsets and stuff
static player_t botblank;

#define BCP_HIGH	1024	// 3 << 8, Use Dijkstra's Algoritm
#define BCP_MEDIUM	512		// 2 << 8, Use BFS Search
#define BCP_LOW		256		// 1 << 8, Use Sector to Sector Lookup (L-Shape Search)
#define BCP_NONE	0		// 0 << 8, Same as above

/* B_CheckWeapon() -- Checks a weapon to determine if it should be picked up */
static int B_CheckWeapon(bmind_t* mind, mobj_t* target, bmocheck_t* check, void* data)
{
	int MaxAmmo;
	int CurAmmo;
	int ClipSize;
	ammotype_t AmmoType;
	
	/* Check to see if we own the weapon */
	if (mind->player->weaponowned[(int)data])
	{
		AmmoType = weaponinfo[(int)data].ammo;
		
		/* Early out if we can't pick it up since it has no ammo */
		if (AmmoType == am_noammo)
			return 0;
		
		/* If we are in solo, altdm or the gun was dropped we can pick it up */
		if (!netgame || deathmatch == 2 || target->flags & MF_DROPPED)
		{
			MaxAmmo = mind->player->maxammo[AmmoType];
			CurAmmo = mind->player->ammo[AmmoType];
			ClipSize = clipammo[AmmoType];
			
			// Deathmatch 1 has 5 clips in a gun, non dropped items have 2 clips inside
			if (deathmatch == 1)
				ClipSize *= 5;
			else if (!(target->flags & MF_DROPPED))
				ClipSize <<= 1;
			
			// Double ammo on ITYTD and NM
			if (gameskill == sk_baby || gameskill == sk_nightmare)
				ClipSize <<= 1;
				
			// Should we pick it up for ammo?
			if (CurAmmo < ((MaxAmmo - ClipSize) >> 1))
				return 1 | BCP_LOW;
			else if (CurAmmo < (MaxAmmo - ClipSize))
				return 1 | BCP_NONE;
			else
				return 0;
		}
		
		/* Otherwise, ignore it */
		else
			return 0;
	}
	
	/* We don't own it so we can freely pick it up! */
	else
		return 1 | check->pform;
}

/* B_CheckMonster() -- Checks a monster to determine if it's worth attacking */
static int B_CheckMonster(bmind_t* mind, mobj_t* target, bmocheck_t* check, void* data)
{
	if (target->health > 0)
		return 1 | check->pform;
	else
		return 0;
}

/* B_CheckPlayer() -- Checks a player to determine if it's worth attacking */
static int B_CheckPlayer(bmind_t* mind, mobj_t* target, bmocheck_t* check, void* data)
{
	if (deathmatch)
	{
		if (target->health > 0)
			return 1 | check->pform;
		else
			return 0;
	}
	else
		return 0;
}

/* B_CheckArmor() -- Checks armor to determine if it's worth picking up */
static int B_CheckArmor(bmind_t* mind, mobj_t* target, bmocheck_t* check, void* data)
{
	if (mind->player->armorpoints < (((int)data) >> 1))
		return 1 | check->pform;
	else
		return 0;
}

/* B_CheckHealth() -- Checks health to determine if it's worth picking up */
// priority matches input
static int B_CheckHealth(bmind_t* mind, mobj_t* target, bmocheck_t* check, void* data)
{
	if (mind->player->health < (int)data)
		return 1 | check->pform;
	else
		return 0;
}

/* B_CheckAmmo() -- Checks ammo to determine if it's worth picking up */
// None = Current ammo is more than half the pickup range
// Low  = Current ammo is less than half the pickup range
static int B_CheckAmmo(bmind_t* mind, mobj_t* target, bmocheck_t* check, void* data)
{
	ammotype_t AmmoType = (int)data & 0xF;
	int MaxAmmo = mind->player->maxammo[AmmoType];
	int CurAmmo = mind->player->ammo[AmmoType];
	int ClipSize = clipammo[AmmoType] * ((int)data >> 4);
	
	if (gameskill == sk_baby || gameskill == sk_nightmare)
		ClipSize <<= 1;
		
	if (CurAmmo < ((MaxAmmo - ClipSize) >> 1))
		return 1 | BCP_LOW;
	else if (CurAmmo < (MaxAmmo - ClipSize))
		return 1 | BCP_NONE;
	else
		return 0;
}

// Structure containing check info
bmocheck_t BotMobjCheck[NUMMOBJTYPES] =
{
	{B_CheckPlayer, BMC_PLAYER, 0, BCP_HIGH},			// MT_PLAYER
	{B_CheckMonster, BMC_MONSTER, 0, BCP_NONE},		// MT_POSSESSED
	{B_CheckMonster, BMC_MONSTER, 0, BCP_LOW},		// MT_SHOTGUY
	{B_CheckMonster, BMC_MONSTER, 0, BCP_HIGH},		// MT_VILE
	{NULL, 0, 0, 0},		// MT_FIRE
	{B_CheckMonster, BMC_MONSTER, 0, BCP_MEDIUM},		// MT_UNDEAD
	{NULL, 0, 0, 0},		// MT_TRACER
	{NULL, 0, 0, 0},		// MT_SMOKE
	{B_CheckMonster, BMC_MONSTER, 0, BCP_MEDIUM},		// MT_FATSO
	{NULL, 0, 0, 0},		// MT_FATSHOT
	{B_CheckMonster, BMC_MONSTER, 0, BCP_LOW},		// MT_CHAINGUY
	{B_CheckMonster, BMC_MONSTER, 0, BCP_LOW},		// MT_TROOP
	{B_CheckMonster, BMC_MONSTER, 0, BCP_LOW},		// MT_SERGEANT
	{B_CheckMonster, BMC_MONSTER, 0, BCP_LOW},		// MT_SHADOWS
	{B_CheckMonster, BMC_MONSTER, 0, BCP_MEDIUM},		// MT_HEAD
	{B_CheckMonster, BMC_MONSTER, 0, BCP_MEDIUM},		// MT_BRUISER
	{NULL, 0, 0, 0},		// MT_BRUISERSHOT
	{B_CheckMonster, BMC_MONSTER, 0, BCP_MEDIUM},		// MT_KNIGHT
	{B_CheckMonster, BMC_MONSTER, 0, BCP_NONE},		// MT_SKULL
	{B_CheckMonster, BMC_MONSTER, 0, BCP_HIGH},		// MT_SPIDER
	{B_CheckMonster, BMC_MONSTER, 0, BCP_MEDIUM},		// MT_BABY
	{B_CheckMonster, BMC_MONSTER, 0, BCP_HIGH},		// MT_CYBORG
	{B_CheckMonster, BMC_MONSTER, 0, BCP_HIGH},		// MT_PAIN
	{B_CheckMonster, BMC_MONSTER, 0, BCP_LOW},		// MT_WOLFSS
	{B_CheckMonster, BMC_MONSTER, 0, BCP_HIGH},		// MT_KEEN
	{B_CheckMonster, BMC_MONSTER, 0, BCP_NONE},		// MT_BOSSBRAIN
	{NULL, 0, 0, 0},		// MT_BOSSSPIT
	{NULL, 0, 0, 0},		// MT_BOSSTARGET
	{NULL, 0, 0, 0},		// MT_SPAWNSHOT
	{NULL, 0, 0, 0},		// MT_SPAWNFIRE
	{B_CheckMonster, BMC_MONSTER, 0, BCP_NONE},		// MT_BARREL
	{NULL, 0, 0, 0},		// MT_TROOPSHOT
	{NULL, 0, 0, 0},		// MT_HEADSHOT
	{NULL, 0, 0, 0},		// MT_ROCKET
	{NULL, 0, 0, 0},		// MT_PLASMA
	{NULL, 0, 0, 0},		// MT_BFG
	{NULL, 0, 0, 0},		// MT_ARACHPLAZ
	{NULL, 0, 0, 0},		// MT_PUFF
	{NULL, 0, 0, 0},		// MT_BLOOD
	{NULL, 0, 0, 0},		// MT_TFOG
	{NULL, 0, 0, 0},		// MT_IFOG
	{NULL, 0, 0, 0},		// MT_TELEPORTMAN
	{NULL, 0, 0, 0},		// MT_EXTRABFG
	{B_CheckArmor, BMC_ARMOR, 100, BCP_LOW},		// MT_MISC0	-- Green Armor
	{B_CheckArmor, BMC_ARMOR, 200, BCP_MEDIUM},		// MT_MISC1	-- Blue Armor
	{B_CheckHealth, BMC_HEALTH, 200, BCP_NONE},		// MT_MISC2 -- Health Potion
	{B_CheckArmor, BMC_ARMOR, 400, BCP_NONE},		// MT_MISC3 -- Armor Helmet
	{NULL, 0, 0, 0},		// MT_MISC4
	{NULL, 0, 0, 0},		// MT_MISC5
	{NULL, 0, 0, 0},		// MT_MISC6
	{NULL, 0, 0, 0},		// MT_MISC7
	{NULL, 0, 0, 0},		// MT_MISC8
	{NULL, 0, 0, 0},		// MT_MISC9
	{B_CheckHealth, BMC_HEALTH, 90, BCP_LOW},		// MT_MISC10 -- Stimpack
	{B_CheckHealth, BMC_HEALTH, 75, BCP_LOW},		// MT_MISC11 -- Medikit
	{B_CheckHealth, BMC_HEALTH, 150, BCP_HIGH},		// MT_MISC12 -- Soulsphere
	{NULL, 0, 0, 0},		// MT_INV
	{B_CheckHealth, BMC_HEALTH, 25, BCP_MEDIUM},		// MT_MISC13 -- Bezerker
	{NULL, 0, 0, 0},		// MT_INS
	{NULL, 0, 0, 0},		// MT_MISC14
	{NULL, 0, 0, 0},		// MT_MISC15
	{NULL, 0, 0, 0},		// MT_MISC16
	{NULL, 0, 0, 0},		// MT_MEGA
	{B_CheckAmmo, BMC_AMMO, am_clip | (1 << 4), 0},		// MT_CLIP
	{B_CheckAmmo, BMC_AMMO, am_clip | (5 << 4), 0},		// MT_MISC17
	{B_CheckAmmo, BMC_AMMO, am_misl | (1 << 4), 0},		// MT_MISC18
	{B_CheckAmmo, BMC_AMMO, am_misl | (5 << 4), 0},		// MT_MISC19
	{B_CheckAmmo, BMC_AMMO, am_cell | (1 << 4), 0},		// MT_MISC20
	{B_CheckAmmo, BMC_AMMO, am_cell | (5 << 4), 0},		// MT_MISC21
	{B_CheckAmmo, BMC_AMMO, am_shell | (1 << 4), 0},		// MT_MISC22
	{B_CheckAmmo, BMC_AMMO, am_shell | (5 << 4), 0},		// MT_MISC23
	{NULL, 0, 0, 0},		// MT_MISC24
	{B_CheckWeapon, BMC_WEAPON, wp_bfg, BCP_MEDIUM},			// MT_MISC25
	{B_CheckWeapon, BMC_WEAPON, wp_chaingun, BCP_MEDIUM},		// MT_CHAINGUN
	{B_CheckWeapon, BMC_WEAPON, wp_chainsaw, BCP_LOW},		// MT_MISC26
	{B_CheckWeapon, BMC_WEAPON, wp_missile, BCP_MEDIUM},		// MT_MISC27
	{B_CheckWeapon, BMC_WEAPON, wp_plasma, BCP_MEDIUM},			// MT_MISC28
	{B_CheckWeapon, BMC_WEAPON, wp_shotgun, BCP_MEDIUM},		// MT_SHOTGUN
	{B_CheckWeapon, BMC_WEAPON, wp_supershotgun, BCP_MEDIUM},	// MT_SUPERSHOTGUN
	{NULL, 0, 0, 0},		// MT_MISC29
	{NULL, 0, 0, 0},		// MT_MISC30
	{NULL, 0, 0, 0},		// MT_MISC31
	{NULL, 0, 0, 0},		// MT_MISC32
	{NULL, 0, 0, 0},		// MT_MISC33
	{NULL, 0, 0, 0},		// MT_MISC34
	{NULL, 0, 0, 0},		// MT_MISC35
	{NULL, 0, 0, 0},		// MT_MISC36
	{NULL, 0, 0, 0},		// MT_MISC37
	{NULL, 0, 0, 0},		// MT_MISC38
	{NULL, 0, 0, 0},		// MT_MISC39
	{NULL, 0, 0, 0},		// MT_MISC40
	{NULL, 0, 0, 0},		// MT_MISC41
	{NULL, 0, 0, 0},		// MT_MISC42
	{NULL, 0, 0, 0},		// MT_MISC43
	{NULL, 0, 0, 0},		// MT_MISC44
	{NULL, 0, 0, 0},		// MT_MISC45
	{NULL, 0, 0, 0},		// MT_MISC46
	{NULL, 0, 0, 0},		// MT_MISC47
	{NULL, 0, 0, 0},		// MT_MISC48
	{NULL, 0, 0, 0},		// MT_MISC49
	{NULL, 0, 0, 0},		// MT_MISC50
	{NULL, 0, 0, 0},		// MT_MISC51
	{NULL, 0, 0, 0},		// MT_MISC52
	{NULL, 0, 0, 0},		// MT_MISC53
	{NULL, 0, 0, 0},		// MT_MISC54
	{NULL, 0, 0, 0},		// MT_MISC55
	{NULL, 0, 0, 0},		// MT_MISC56
	{NULL, 0, 0, 0},		// MT_MISC57
	{NULL, 0, 0, 0},		// MT_MISC58
	{NULL, 0, 0, 0},		// MT_MISC59
	{NULL, 0, 0, 0},		// MT_MISC60
	{NULL, 0, 0, 0},		// MT_MISC61
	{NULL, 0, 0, 0},		// MT_MISC62
	{NULL, 0, 0, 0},		// MT_MISC63
	{NULL, 0, 0, 0},		// MT_MISC64
	{NULL, 0, 0, 0},		// MT_MISC65
	{NULL, 0, 0, 0},		// MT_MISC66
	{NULL, 0, 0, 0},		// MT_MISC67
	{NULL, 0, 0, 0},		// MT_MISC68
	{NULL, 0, 0, 0},		// MT_MISC69
	{NULL, 0, 0, 0},		// MT_MISC70
	{NULL, 0, 0, 0},		// MT_MISC71
	{NULL, 0, 0, 0},		// MT_MISC72
	{NULL, 0, 0, 0},		// MT_MISC73
	{NULL, 0, 0, 0},		// MT_MISC74
	{NULL, 0, 0, 0},		// MT_MISC75
	{NULL, 0, 0, 0},		// MT_MISC76
	{NULL, 0, 0, 0},		// MT_MISC77
	{NULL, 0, 0, 0},		// MT_MISC78
	{NULL, 0, 0, 0},		// MT_MISC79
	{NULL, 0, 0, 0},		// MT_MISC80
	{NULL, 0, 0, 0},		// MT_MISC81
	{NULL, 0, 0, 0},		// MT_MISC82
	{NULL, 0, 0, 0},		// MT_MISC83
	{NULL, 0, 0, 0},		// MT_MISC84
	{NULL, 0, 0, 0},		// MT_MISC85
	{NULL, 0, 0, 0},		// MT_MISC86
};

void B_LookForStuff(bmind_t* mind)
{
	thinker_t* currentthinker = thinkercap.next;
	mobj_t* thatmobj = NULL;
	mobj_t* NewGather = NULL;
	mobj_t* NewAttack = NULL;
	int GatherDistance = 8192;
	int AttackDistance = 8192;
	int CurDist;
	int inView = 0;
	int priority = 0;
	int priorityx = 0;
	
	while (currentthinker != &thinkercap)
	{
		if ((currentthinker->function.acp1 == (actionf_p1)P_MobjThinker))
		{
			thatmobj = (mobj_t*)currentthinker;
			
			if (thatmobj == mind->player->mo)
			{
				currentthinker = currentthinker->next;
				continue;
			}
			
			// Check to see if it has a checker function
			if (BotMobjCheck[thatmobj->type].func)
			{
				// Gathering or attacking?
				if ((BotMobjCheck[thatmobj->type].type >= BMC_WEAPON) &&
					(BotMobjCheck[thatmobj->type].type <= BMC_ARMOR))
				{
					if (!mind->GatherTarget && thatmobj->type != mind->failtype)
						if (priority = BotMobjCheck[thatmobj->type].func(
								mind, thatmobj,
								&BotMobjCheck[thatmobj->type],
								BotMobjCheck[thatmobj->type].poff)
							)
						{
							/* just this is needed */
							if ((CurDist = B_BuildPath(mind, mind->player->mo->subsector,
								thatmobj->subsector, BP_CHECKPATH, priority >> 8)) < GatherDistance)
							{
								GatherDistance = CurDist;
								NewGather = thatmobj;
								priorityx = priority;
							}

#if 0
							if ((CurDist = B_BuildPath(mind, mind->player->mo->subsector,
								thatmobj->subsector, BP_CHECKPATH, priority >> 1)) < GatherDistance)
							{
								GatherDistance = CurDist;
								NewGather = thatmobj;
								priorityx = priority;
							}
							else if (P_CheckSight(mind->player->mo, thatmobj) &&
								(CurDist = B_PathDistance(mind->player->mo, thatmobj)) < GatherDistance)
							{
								GatherDistance = CurDist;
								NewGather = thatmobj;
								priorityx = priority;
							}
#endif
						}
				}
				else if ((BotMobjCheck[thatmobj->type].type >= BMC_PLAYER) &&
						(BotMobjCheck[thatmobj->type].type <= BMC_MONSTER))
						if (!mind->AttackTarget)
							if (priority = BotMobjCheck[thatmobj->type].func(
									mind, thatmobj,
									&BotMobjCheck[thatmobj->type],
									BotMobjCheck[thatmobj->type].poff)
								)
							{
								CurDist = B_PathDistance(mind->player->mo, thatmobj);
								
								if (P_CheckSight(mind->player->mo, thatmobj) &&
									CurDist < (MISSILERANGE >> FRACBITS) && CurDist < AttackDistance)
								{
									AttackDistance = CurDist;
									NewAttack = thatmobj;
									inView = 1;
									priorityx = priority;
								}
								else if ((CurDist = B_BuildPath(mind, mind->player->mo->subsector,
									thatmobj->subsector, BP_CHECKPATH, priority >> 8)) < AttackDistance)
								{
									AttackDistance = CurDist;
									NewAttack = thatmobj;
									inView = 0;
									priorityx = priority;
								}
							}
			}
		}
		
		currentthinker = currentthinker->next;
	}
	
	if (NewGather)
	{
		mind->GatherTarget = NewGather;
		mind->flags |= BF_GATHERING;
		
		if (!mind->PathNodes[mind->PathIterator] && !mind->PathIterator)
		{
			B_BuildPath(mind, mind->player->mo->subsector, mind->GatherTarget->subsector, 0, priorityx >> 8);
			mind->priority = priorityx >> 8;
			mind->failcount = 0;
			mind->failtype = -1;
		}
	}
	
	if (NewAttack)
	{
		mind->AttackTarget = NewAttack;
		mind->flags |= BF_ATTACKING;
		
		if (!inView)
			if (!mind->PathNodes[mind->PathIterator] && !mind->PathIterator)
			{
				B_BuildPath(mind, mind->player->mo->subsector, mind->AttackTarget->subsector, 0, priorityx >> 8);		
				mind->priority = priorityx >> 8;
				mind->failcount = 0;
			}
	}
}

/* B_BFSLink() -- Searches through the node tree for a path */
int B_BFSLink(bmind_t* mind, subsector_t* src, subsector_t* dest, bnode_t** listptr)
{
	int dist = BOTBADPATH;
	Int16* Chunk = NULL;
	Int16* Crumbs = NULL;
	Int16* In = NULL;
	Int16* Out = NULL;
	Int16* Con = NULL;
	Int16 First, Last;
	Int16 x;
	Int16 Goal = dest - subsectors;
	size_t i = 0, j = 0, k = 0;
	bnode_t* t;
	
	/* Allocate the Chunk */
	Chunk = Z_Malloc(sizeof(Int16) * NumBotNodes * NumBotNodes, PU_STATIC, NULL);
	Crumbs = Z_Malloc(sizeof(Int16) * NumBotNodes, PU_STATIC, NULL);
	memset(Chunk, -1, sizeof(Int16) * NumBotNodes * NumBotNodes);
	memset(Crumbs, 0xFF, sizeof(Int16) * NumBotNodes);
	In = Chunk;
	Out = Chunk;
	
	/* Add the first links to the chunk */
	// Never bother checking the first connections as these lines were already
	// checked before the graph is even being checked
	Con = BotNodes[src - subsectors].connections;
	
	// Add the first node
	*Out = src - subsectors;
	Out++;
	
	// There may be no connections, be sure there are
	if (Con)
	{
		// Scan the input
		while (*In != -1)
		{
			// If the input wasn't visited, visit it
			if (!BotFinal[*In])
			{
				// Visit self
				BotFinal[*In] = 1;
				
				// Set the connection list to the node we shall visit
				Con = BotNodes[*In].connections;
				
				if (Con)
				{
					// There are so let's add them
					while (*Con != -2)
					{
						// Continual Array
						if (*Con == -1)
						{
							Con++;
							First = *Con;
							Con++;
							Last = *Con;
							Con++;
				
							for (x = First; x < Last; x++)
							{
								if (!BotFinal[First + x])
								{
									*Out = First + x;
									BotFinal[First + x] = 1;
									Crumbs[First + x] = *In;
							
									if (*Out == Goal)
										goto wefoundamatch;
								
									Out++;
								}
							}
						}
			
						// Normal List
						else
						{
							if (!BotFinal[*Con])
							{
								*Out = *Con;
								BotFinal[*Con] = 1;
								Crumbs[*Con] = *In;
						
								if (*Out == Goal)
									goto wefoundamatch;
						
								Out++;
							}
						
							Con++;
						}
					}
				}
			}
			
			In++;
		}
	}

normaltermination:
	/* Deallocate the Chunk */
	Z_Free(Chunk);
	Z_Free(Crumbs);
	
	return dist;

	/* They say gotos are bad... */	
wefoundamatch:
	x = *Out;
	i = 0;
	dist = 0;
	
	while (x != -1)
	{
		printf("%i: %i\n", x, i);
		listptr[i] = &BotNodes[x];
		
		if (x == Crumbs[x])
		{
			printf("Self referenced node!");
			break;
		}
			
		x = Crumbs[x];
		i++;
	}
	
	k = i;
	
	for (j = 0; j < i; j++, i--)
	{
		t = listptr[j];
		listptr[j] = listptr[i];
		listptr[i] = t;
	}
	
	dist = 0;
	
	for (i = 0; i < k - 2; k++)
		dist += B_PathDistance(listptr[i], listptr[i+1]);
	
	goto normaltermination;
	
#if 0
	int dist = BOTBADPATH;
	size_t it = 0;
	
	// Mark all points unvisited
	memset(BotFinal, 0, sizeof(UInt8) * NumBotNodes);
	
	// Mark root as visisted
	BotFinal[src - subsectors] = 1;
	
	// Add the initial kids to the queue
	
	
	// Add children to the queue
	while 
	
	return dist;
#endif
}

/* B_BuildPath() -- Build path to the target and return the distance */
int B_BuildPath(bmind_t* mind, subsector_t* src, subsector_t* dest, int flags, int priority)
{
	size_t i, j;
	int realdistance = BOTBADPATH;
	int actualdistance = 0;
	sector_t* srcsector = src->sector;
	sector_t* destsector = dest->sector;
	bnode_t* list[MAXPATHSEGMENTS];
	int ssx, dsx;
	int eax;
	subsector_t* curss = NULL;
	
	memset(list, 0, sizeof(list));
	
	/* Check for a straight path */
	if (B_CheckLine(mind, src, dest))
	{
		list[0] = &BotNodes[dest - subsectors];
		actualdistance = B_PathDistance(&BotNodes[src - subsectors], list[0]);
		
		realdistance = actualdistance;
	}
	
	/* No straight path exists -- priority is used here */
	else
	{
		switch (priority)
		{
			/* Dijkstra (Shortest Path) */
			case 3:
				//break;
			
			/* BFS (First Path) */
			case 2:
				realdistance = B_BFSLink(mind, src, dest, list);
				break;
			
			/* SECTOR TO SECTOR (Shortest Path) */	
			case 1:
				ssx = src->sector - sectors;
				dsx = dest->sector - sectors;
				eax = 0;
		
				// BotSectorNodes = Pointer to an array that contains a sector list
				while (BotSectorNodes[ssx][eax] != NULL)
				{
					curss = BotSectorNodes[ssx][eax];
					actualdistance = 0;
			
					if (B_CheckLine(mind, curss, dest))
					{
						actualdistance = B_PathDistance(&BotNodes[src - subsectors], &BotNodes[curss - subsectors]);
						actualdistance += B_PathDistance(&BotNodes[curss - subsectors], &BotNodes[dest - subsectors]);
				
						if (actualdistance < realdistance)
						{
							list[0] = &BotNodes[curss - subsectors];
							list[1] = &BotNodes[dest - subsectors];
				
							realdistance = actualdistance;
						}
					}
			
					eax++;
				}
				break;
			
			/* SECTOR TO SECTOR (First Path) */
			default:
				ssx = src->sector - sectors;
				dsx = dest->sector - sectors;
				eax = 0;
		
				// BotSectorNodes = Pointer to an array that contains a sector list
				while (BotSectorNodes[ssx][eax] != NULL)
				{
					curss = BotSectorNodes[ssx][eax];
					actualdistance = 0;
					
					// If the line checks out, end the loop and set this as the path
					if (B_CheckLine(mind, curss, dest))
					{
						actualdistance = B_PathDistance(&BotNodes[src - subsectors], &BotNodes[curss - subsectors]);
						actualdistance += B_PathDistance(&BotNodes[curss - subsectors], &BotNodes[dest - subsectors]);
				
						list[0] = &BotNodes[curss - subsectors];
						list[1] = &BotNodes[dest - subsectors];
			
						realdistance = actualdistance;
						break;
					}
			
					eax++;
				}
				break;
		}
	}
	
#if 0
	else
	{
		ssx = src->sector - sectors;
		dsx = dest->sector - sectors;
		
		// For now, just do a basic check, L check
		eax = 0;
		
		// BotSectorNodes = Pointer to an array that contains a sector list
		while (BotSectorNodes[ssx][eax] != NULL)
		{
			curss = BotSectorNodes[ssx][eax];
			actualdistance = 0;
			
			if (B_CheckLine(mind, curss, dest))
			{
				actualdistance = B_PathDistance(&BotNodes[src - subsectors], &BotNodes[curss - subsectors]);
				actualdistance += B_PathDistance(&BotNodes[curss - subsectors], &BotNodes[dest - subsectors]);
				
				if (actualdistance < realdistance)
				{
					list[0] = &BotNodes[curss - subsectors];
					list[1] = &BotNodes[dest - subsectors];
				
					realdistance = actualdistance;
					break;
				}
			}
			
			eax++;
		}
	}
#endif
	
	// Create path in the bot's mind
	if (!(flags & BP_CHECKPATH))
	{
		mind->PathIterator = 0;
		memcpy(mind->PathNodes, list, sizeof(list));
	}
	
	return realdistance;
}

