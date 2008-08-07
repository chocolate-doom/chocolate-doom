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

// Huge ass if statement to determine if we should pick up this discarded weapon for ammo
/*#define ISWEAPONAMMOWORTHIT(wp) \
(\
	player->weaponowned[wp] &&\
	(\
		((netgame && deathmatch == 2 && !thatmobj->flags & MF_DROPPED) &&\
			(player->ammo[weaponinfo[wp].ammo] < (maxammo[weaponinfo[wp].ammo] - \
			((deathmatch ? clipammo[weaponinfo[wp].ammo] * 5 : clipammo[weaponinfo[wp].ammo] << 1) <<\
				(gameskill == sk_baby || gameskill == sk_nightmare ? 1 : 0))))) ||\
		((!netgame || deathmatch != 2) && \
			(player->ammo[weaponinfo[wp].ammo] < (maxammo[weaponinfo[wp].ammo] - \
			((thatmobj->flags & MF_DROPPED ? clipammo[weaponinfo[wp].ammo] : clipammo[weaponinfo[wp].ammo] << 1) <<\
				(gameskill == sk_baby || gameskill == sk_nightmare ? 1 : 0)))))\
	)\
)*/

void B_LookForStuff(bmind_t* mind)
{
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

