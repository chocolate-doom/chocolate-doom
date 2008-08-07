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
#include <math.h>

// Blank Player for offsets and stuff
static player_t botblank;

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
			if (CurAmmo < MaxAmmo - ClipSize)
				return 1;
			else
				return 0;
		}
		
		/* Otherwise, ignore it */
		else
			return 0;
	}
	
	/* We don't own it so we can freely pick it up! */
	else
		return 1;
}

// Structure containing check info
bmocheck_t BotMobjCheck[NUMMOBJTYPES] =
{
	{NULL, 0, 0},		// MT_PLAYER
	{NULL, 0, 0},		// MT_POSSESSED
	{NULL, 0, 0},		// MT_SHOTGUY
	{NULL, 0, 0},		// MT_VILE
	{NULL, 0, 0},		// MT_FIRE
	{NULL, 0, 0},		// MT_UNDEAD
	{NULL, 0, 0},		// MT_TRACER
	{NULL, 0, 0},		// MT_SMOKE
	{NULL, 0, 0},		// MT_FATSO
	{NULL, 0, 0},		// MT_FATSHOT
	{NULL, 0, 0},		// MT_CHAINGUY
	{NULL, 0, 0},		// MT_TROOP
	{NULL, 0, 0},		// MT_SERGEANT
	{NULL, 0, 0},		// MT_SHADOWS
	{NULL, 0, 0},		// MT_HEAD
	{NULL, 0, 0},		// MT_BRUISER
	{NULL, 0, 0},		// MT_BRUISERSHOT
	{NULL, 0, 0},		// MT_KNIGHT
	{NULL, 0, 0},		// MT_SKULL
	{NULL, 0, 0},		// MT_SPIDER
	{NULL, 0, 0},		// MT_BABY
	{NULL, 0, 0},		// MT_CYBORG
	{NULL, 0, 0},		// MT_PAIN
	{NULL, 0, 0},		// MT_WOLFSS
	{NULL, 0, 0},		// MT_KEEN
	{NULL, 0, 0},		// MT_BOSSBRAIN
	{NULL, 0, 0},		// MT_BOSSSPIT
	{NULL, 0, 0},		// MT_BOSSTARGET
	{NULL, 0, 0},		// MT_SPAWNSHOT
	{NULL, 0, 0},		// MT_SPAWNFIRE
	{NULL, 0, 0},		// MT_BARREL
	{NULL, 0, 0},		// MT_TROOPSHOT
	{NULL, 0, 0},		// MT_HEADSHOT
	{NULL, 0, 0},		// MT_ROCKET
	{NULL, 0, 0},		// MT_PLASMA
	{NULL, 0, 0},		// MT_BFG
	{NULL, 0, 0},		// MT_ARACHPLAZ
	{NULL, 0, 0},		// MT_PUFF
	{NULL, 0, 0},		// MT_BLOOD
	{NULL, 0, 0},		// MT_TFOG
	{NULL, 0, 0},		// MT_IFOG
	{NULL, 0, 0},		// MT_TELEPORTMAN
	{NULL, 0, 0},		// MT_EXTRABFG
	{NULL, 0, 0},		// MT_MISC0
	{NULL, 0, 0},		// MT_MISC1
	{NULL, 0, 0},		// MT_MISC2
	{NULL, 0, 0},		// MT_MISC3
	{NULL, 0, 0},		// MT_MISC4
	{NULL, 0, 0},		// MT_MISC5
	{NULL, 0, 0},		// MT_MISC6
	{NULL, 0, 0},		// MT_MISC7
	{NULL, 0, 0},		// MT_MISC8
	{NULL, 0, 0},		// MT_MISC9
	{NULL, 0, 0},		// MT_MISC10
	{NULL, 0, 0},		// MT_MISC11
	{NULL, 0, 0},		// MT_MISC12
	{NULL, 0, 0},		// MT_INV
	{NULL, 0, 0},		// MT_MISC13
	{NULL, 0, 0},		// MT_INS
	{NULL, 0, 0},		// MT_MISC14
	{NULL, 0, 0},		// MT_MISC15
	{NULL, 0, 0},		// MT_MISC16
	{NULL, 0, 0},		// MT_MEGA
	{NULL, 0, 0},		// MT_CLIP
	{NULL, 0, 0},		// MT_MISC17
	{NULL, 0, 0},		// MT_MISC18
	{NULL, 0, 0},		// MT_MISC19
	{NULL, 0, 0},		// MT_MISC20
	{NULL, 0, 0},		// MT_MISC21
	{NULL, 0, 0},		// MT_MISC22
	{NULL, 0, 0},		// MT_MISC23
	{NULL, 0, 0},		// MT_MISC24
	{B_CheckWeapon, BMC_WEAPON, wp_bfg},			// MT_MISC25
	{B_CheckWeapon, BMC_WEAPON, wp_chaingun},		// MT_CHAINGUN
	{B_CheckWeapon, BMC_WEAPON, wp_chainsaw},		// MT_MISC26
	{B_CheckWeapon, BMC_WEAPON, wp_missile},		// MT_MISC27
	{B_CheckWeapon, BMC_WEAPON, wp_plasma},			// MT_MISC28
	{B_CheckWeapon, BMC_WEAPON, wp_shotgun},		// MT_SHOTGUN
	{B_CheckWeapon, BMC_WEAPON, wp_supershotgun},	// MT_SUPERSHOTGUN
	{NULL, 0, 0},		// MT_MISC29
	{NULL, 0, 0},		// MT_MISC30
	{NULL, 0, 0},		// MT_MISC31
	{NULL, 0, 0},		// MT_MISC32
	{NULL, 0, 0},		// MT_MISC33
	{NULL, 0, 0},		// MT_MISC34
	{NULL, 0, 0},		// MT_MISC35
	{NULL, 0, 0},		// MT_MISC36
	{NULL, 0, 0},		// MT_MISC37
	{NULL, 0, 0},		// MT_MISC38
	{NULL, 0, 0},		// MT_MISC39
	{NULL, 0, 0},		// MT_MISC40
	{NULL, 0, 0},		// MT_MISC41
	{NULL, 0, 0},		// MT_MISC42
	{NULL, 0, 0},		// MT_MISC43
	{NULL, 0, 0},		// MT_MISC44
	{NULL, 0, 0},		// MT_MISC45
	{NULL, 0, 0},		// MT_MISC46
	{NULL, 0, 0},		// MT_MISC47
	{NULL, 0, 0},		// MT_MISC48
	{NULL, 0, 0},		// MT_MISC49
	{NULL, 0, 0},		// MT_MISC50
	{NULL, 0, 0},		// MT_MISC51
	{NULL, 0, 0},		// MT_MISC52
	{NULL, 0, 0},		// MT_MISC53
	{NULL, 0, 0},		// MT_MISC54
	{NULL, 0, 0},		// MT_MISC55
	{NULL, 0, 0},		// MT_MISC56
	{NULL, 0, 0},		// MT_MISC57
	{NULL, 0, 0},		// MT_MISC58
	{NULL, 0, 0},		// MT_MISC59
	{NULL, 0, 0},		// MT_MISC60
	{NULL, 0, 0},		// MT_MISC61
	{NULL, 0, 0},		// MT_MISC62
	{NULL, 0, 0},		// MT_MISC63
	{NULL, 0, 0},		// MT_MISC64
	{NULL, 0, 0},		// MT_MISC65
	{NULL, 0, 0},		// MT_MISC66
	{NULL, 0, 0},		// MT_MISC67
	{NULL, 0, 0},		// MT_MISC68
	{NULL, 0, 0},		// MT_MISC69
	{NULL, 0, 0},		// MT_MISC70
	{NULL, 0, 0},		// MT_MISC71
	{NULL, 0, 0},		// MT_MISC72
	{NULL, 0, 0},		// MT_MISC73
	{NULL, 0, 0},		// MT_MISC74
	{NULL, 0, 0},		// MT_MISC75
	{NULL, 0, 0},		// MT_MISC76
	{NULL, 0, 0},		// MT_MISC77
	{NULL, 0, 0},		// MT_MISC78
	{NULL, 0, 0},		// MT_MISC79
	{NULL, 0, 0},		// MT_MISC80
	{NULL, 0, 0},		// MT_MISC81
	{NULL, 0, 0},		// MT_MISC82
	{NULL, 0, 0},		// MT_MISC83
	{NULL, 0, 0},		// MT_MISC84
	{NULL, 0, 0},		// MT_MISC85
	{NULL, 0, 0},		// MT_MISC86
};

void B_LookForStuff(bmind_t* mind)
{
	thinker_t* currentthinker = thinkercap.next;
	mobj_t* thatmobj = NULL;
	mobj_t* NewGather = NULL;
	int GatherDistance = 8192;
	
	while (currentthinker != &thinkercap)
	{
		if ((currentthinker->function.acp1 == (actionf_p1)P_MobjThinker))
		{
			thatmobj = (mobj_t*)currentthinker;
			
			// Check to see if it has a checker function
			if (BotMobjCheck[thatmobj->type].func)
			{
				// Gathering or attacking?
				if ((BotMobjCheck[thatmobj->type].type >= BMC_WEAPON) &&
					(BotMobjCheck[thatmobj->type].type <= BMC_ARMOR))
				{
					if (!mind->GatherTarget)
						if (BotMobjCheck[thatmobj->type].func(
								mind, thatmobj,
								&BotMobjCheck[thatmobj->type],
								BotMobjCheck[thatmobj->type].poff)
							)
						{
							if (B_BuildPath(mind, mind->player->mo->subsector,
								thatmobj->subsector, BP_CHECKPATH) < GatherDistance)
							{
								GatherDistance = B_BuildPath(mind, mind->player->mo->subsector, thatmobj->subsector, BP_CHECKPATH);
								NewGather = thatmobj;
							}
							else if (P_CheckSight(mind->player->mo, thatmobj) &&
								B_PathDistance(mind->player->mo, thatmobj) < GatherDistance)
							{
								GatherDistance = B_PathDistance(mind->player->mo, thatmobj);
								NewGather = thatmobj;
							}
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
		
		// Check to see if we can easily walk to the item that we will pickup
		if (!(BotReject[mind->player->mo->subsector - subsectors][mind->GatherTarget->subsector - subsectors]))
			if (!mind->PathNodes[mind->PathIterator] && !mind->PathIterator)
				B_BuildPath(mind, mind->player->mo->subsector, mind->GatherTarget->subsector, 0);
	}
	
#if 0
	player_t* player = mind->player;
	size_t i, j;
	thinker_t* currentthinker = thinkercap.next;
	mobj_t* ourmobj = player->mo;
	mobj_t* thatmobj = NULL;
	int DoGather = 0;
	int DoAttack = 0;
	int GatherDistance = 10000;
	int AttackDistance = 10000;
	int DoPickup = 0;
	int GoAttack = 0;
	int AttackBySight = 0;
	mobj_t* tGather = NULL;
	mobj_t* tAttack = NULL;
	
	if (!mind->GatherTarget)
		DoGather = 1;
	if (!mind->AttackTarget)
		DoAttack = 1;
	
	if (DoAttack || DoGather)
		while (currentthinker != &thinkercap)
		{
			if ((currentthinker->function.acp1 == (actionf_p1)P_MobjThinker))
			{
				thatmobj = (mobj_t*)currentthinker;
			
				if (DoAttack)
				{
					GoAttack = 0;
					
					/* Thing has life > 0 */
					if (thatmobj->health > 0)
					{
						/* Players */
						if (thatmobj->type == MT_PLAYER)
						{
							if (thatmobj == player->mo);
							{
								currentthinker = currentthinker->next;
								continue;
							}
							
							// In Deathmatch, so we want it dead
							if (deathmatch)
							{
								GoAttack = 1;
							}
					
							// In Coop, so we want to protect it
							else
							{
							}
						}
			
						/* Monsters */
						else if (((thatmobj->type >= MT_POSSESSED) && (thatmobj->type <= MT_VILE)) ||
							(thatmobj->type == MT_UNDEAD) ||
							(thatmobj->type == MT_FATSO) ||
							((thatmobj->type >= MT_CHAINGUY) && (thatmobj->type <= MT_BRUISER)) ||
							((thatmobj->type >= MT_KNIGHT) && (thatmobj->type <= MT_BOSSBRAIN)))
						{
							GoAttack = 1;
						}
					}
					
					if (GoAttack)
					{
						if (tAttack && thatmobj->health > tAttack->health)
						{
							if (B_BuildPath(mind, player->mo->subsector, thatmobj->subsector, BP_CHECKPATH) < AttackDistance)
							{
								AttackDistance = B_BuildPath(mind, player->mo->subsector, thatmobj->subsector, BP_CHECKPATH);
								tAttack = thatmobj;
							}
							else if (P_CheckSight(player->mo, thatmobj))
							{
								AttackDistance = 64;
								tAttack = thatmobj;
							}
						}
						else if (!tAttack)
						{
							if (B_BuildPath(mind, player->mo->subsector, thatmobj->subsector, BP_CHECKPATH) < AttackDistance)
							{
								AttackDistance = B_BuildPath(mind, player->mo->subsector, thatmobj->subsector, BP_CHECKPATH);
								tAttack = thatmobj;
							}
							else if (P_CheckSight(player->mo, thatmobj))
							{
								AttackDistance = 64;
								tAttack = thatmobj;
							}
						}
					}	
				}
			
				if (DoGather)
				{
					DoPickup = 0;
				
					/* Health Items */
					switch (thatmobj->type)
					{
						case MT_MISC2:	// Health Bonus
							if (player->health < 200)
								DoPickup = 1;
							break;
						case MT_MISC10:	// Stim pack
							if (player->health < 90)
								DoPickup = 1;
							break;
						case MT_MISC11:	// Medikit
							if (player->health < 75)
								DoPickup = 1;
							break;
						case MT_MISC12:	// Soul Sphere
							if (player->health < 150)
								DoPickup = 1;
							break;
						case MT_MISC13:	// Berzerker
							if (player->health < 25)
								DoPickup = 1;
							break;
						default:
							break;
					}
			
					/* Armor Items */
			
					/* Weapons */
					if ((thatmobj->type >= MT_MISC25) && (thatmobj->type <= MT_SUPERSHOTGUN))
					{
						switch (thatmobj->type)
						{
							case MT_MISC25:			// BFG9000
								if (!player->weaponowned[wp_bfg] || ISWEAPONAMMOWORTHIT(wp_bfg))
									DoPickup = 1;
								break;
							case MT_CHAINGUN:		// Chaingun
								if (!player->weaponowned[wp_chaingun] || ISWEAPONAMMOWORTHIT(wp_chaingun))
									DoPickup = 1;
								break;
							case MT_MISC26:			// Chainsaw
								if (!player->weaponowned[wp_chainsaw] || ISWEAPONAMMOWORTHIT(wp_chainsaw))
									DoPickup = 1;
								break;
							case MT_MISC27:			// Rocket Launcher
								if (!player->weaponowned[wp_missile] || ISWEAPONAMMOWORTHIT(wp_missile))
									DoPickup = 1;
								break;
							case MT_MISC28:			// Plasma Gun
								if (!player->weaponowned[wp_plasma] || ISWEAPONAMMOWORTHIT(wp_plasma))
									DoPickup = 1;
								break;
							case MT_SHOTGUN:		// Shotgun
								if (!player->weaponowned[wp_shotgun] || ISWEAPONAMMOWORTHIT(wp_shotgun))
									DoPickup = 1;
								break;
							case MT_SUPERSHOTGUN:	// Super Shotgun
								if (!player->weaponowned[wp_supershotgun] || ISWEAPONAMMOWORTHIT(wp_supershotgun))
									DoPickup = 1;
								break;
							default:
								break;
						}
					}
			
					/* Ammo */
					if ((thatmobj->type >= MT_CLIP) && (thatmobj->type <= MT_MISC24))
					{
					}
			
					/* Keys */
					if ((thatmobj->type >= MT_MISC4) && (thatmobj->type <= MT_MISC9))
					{
					}
				
					if (DoPickup)
						if (B_BuildPath(mind, player->mo->subsector, thatmobj->subsector, BP_CHECKPATH) < GatherDistance)
						{
							GatherDistance = B_BuildPath(mind, player->mo->subsector, thatmobj->subsector, BP_CHECKPATH);
							tGather = thatmobj;
						}
				}
			}
		
			// Next Thinker
			currentthinker = currentthinker->next;
		}
	
	// Build a path
	if (DoGather && tGather)
	{
		mind->GatherTarget = tGather;
		mind->flags |= BF_GATHERING;
		if (!mind->PathNodes[mind->PathIterator] && !mind->PathIterator)
			B_BuildPath(mind, player->mo->subsector, mind->GatherTarget->subsector, 0);
	}
	
	// Attack enemy
	if (DoAttack && tAttack)
	{
		mind->AttackTarget = tAttack;
		mind->flags |= BF_ATTACKING;
		if (!mind->PathNodes[mind->PathIterator] && !mind->PathIterator)
			B_BuildPath(mind, player->mo->subsector, mind->AttackTarget->subsector, 0);
	}
#endif
}

/* B_PathDistance() -- Distance between nodes in normal ints */
#if 0
int B_PathDistance(bnode_t* a, bnode_t* b)
{
	return (int)sqrt(
		pow(a->x >> FRACBITS, 2) +
		pow(b->y >> FRACBITS, 2));
}
#endif

/* B_BuildPath() -- Build path to the target and return the distance */
int B_BuildPath(bmind_t* mind, subsector_t* src, subsector_t* dest, int flags)
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
	if (BotReject[src - subsectors][dest - subsectors])
	{
		list[0] = &BotNodes[dest - subsectors];
		actualdistance = B_PathDistance(&BotNodes[src - subsectors], list[0]);
		
		realdistance = actualdistance;
	}
	
	/* No straight path exists */
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
			
			if (BotReject[curss - subsectors][dest - subsectors])
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
	}
	
	// Create path in the bot's mind
	if (!(flags & BP_CHECKPATH))
	{
		mind->PathIterator = 0;
		memcpy(mind->PathNodes, list, sizeof(list));
	}
	
	return realdistance;
}

