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
#include "m_misc.h"
#include "doomdef.h"

extern int longtics;

fixed_t         botforwardmove[2] = {0x19, 0x32}; 
fixed_t         botsidemove[2] = {0x18, 0x28}; 
fixed_t         botangleturn[3] = {640, 1280, 320};    // + slow turn 

// Huge ass if statement to determine if we should pick up this discarded weapon for ammo
/*#define ISWEAPONAMMOWORTHIT(wp) \
(\
	player->weaponowned[wp] &&\
	(\
		((netgame && deathmatch == 2 && !mind->GatherTarget->flags & MF_DROPPED) &&\
			(player->ammo[weaponinfo[wp].ammo] < (maxammo[weaponinfo[wp].ammo] - \
			((deathmatch ? clipammo[weaponinfo[wp].ammo] * 5 : clipammo[weaponinfo[wp].ammo] << 1) <<\
				(gameskill == sk_baby || gameskill == sk_nightmare ? 1 : 0))))) ||\
		((!netgame || deathmatch != 2) && \
			(player->ammo[weaponinfo[wp].ammo] < (maxammo[weaponinfo[wp].ammo] - \
			((mind->GatherTarget->flags & MF_DROPPED ? clipammo[weaponinfo[wp].ammo] : clipammo[weaponinfo[wp].ammo] << 1) <<\
				(gameskill == sk_baby || gameskill == sk_nightmare ? 1 : 0)))))\
	)\
)*/

int B_IsWeaponAmmoWorthIt(bmind_t* mind, weapontype_t wp)
{
	int max = mind->player->maxammo[weaponinfo[wp].ammo];
	int cur = mind->player->ammo[weaponinfo[wp].ammo];
	int clip = clipammo[weaponinfo[wp].ammo];
	
	if (!mind->GatherTarget)
		return 0;
	
	// Weapons that have no ammo, cannot be picked up for ammo
	if (weaponinfo[wp].ammo == am_noammo)
		return 0;
	
	/* Singleplayer, Deathmatch 2 and dropped items */
	if (!netgame || deathmatch == 2 || mind->GatherTarget->flags & MF_DROPPED)
	{
		if (!(mind->GatherTarget->flags & MF_DROPPED))
			clip <<= 1;
	}
	
	/* Cooperative and Deathmatch */
	else
	{
		// Guns give * 5 ammo in DM
		if (deathmatch)
			clip *= 5;
		else
			clip <<= 1;
	}
	
	/* ITYTD or NM */
	if (gameskill == sk_baby || gameskill == sk_nightmare)
		clip <<= 1;
	
	return (cur < max - clip);
}

void B_FaceNode(bmind_t* mind, ticcmd_t* cmd, bnode_t* target)
{
	angle_t myangle = 0;
	angle_t actualangle = 0;
	angle_t virtualangle = 0;
	int someactualangle = 0;
	int somevirtualangle = 0;
	int somemyangle = 0;
	int someoffset = 0;
	
	// First Face the target
	actualangle = R_PointToAngle2 (mind->player->mo->x, mind->player->mo->y, target->x ,target->y);
	virtualangle = mind->player->mo->angle;
	myangle = mind->player->mo->angle;
	
	someactualangle = actualangle >> 16;
	somevirtualangle = virtualangle >> 16;
	somemyangle = myangle >> 16;
	
	while (somevirtualangle != (someactualangle))
	{
		if (somevirtualangle + someoffset < (someactualangle))
			someoffset++;
		else if (somevirtualangle + someoffset > (someactualangle))
			someoffset--;
		else
		{
			cmd->angleturn += someoffset;
			break;
		}
	}
}

void B_FaceMobj(bmind_t* mind, ticcmd_t* cmd, mobj_t *target)
{
	angle_t myangle = 0;
	angle_t actualangle = 0;
	angle_t virtualangle = 0;
	int someactualangle = 0;
	int somevirtualangle = 0;
	int somemyangle = 0;
	int someoffset = 0;
	
	// First Face the target
	actualangle = R_PointToAngle2 (mind->player->mo->x, mind->player->mo->y, target->x ,target->y);
	virtualangle = mind->player->mo->angle;
	myangle = mind->player->mo->angle;
	
	someactualangle = actualangle >> 16;
	somevirtualangle = virtualangle >> 16;
	somemyangle = myangle >> 16;
	
	while (somevirtualangle != (someactualangle))
	{
		if (somevirtualangle + someoffset < (someactualangle))
			someoffset++;
		else if (somevirtualangle + someoffset > (someactualangle))
			someoffset--;
		else
		{
			cmd->angleturn += someoffset;
			break;
		}
	}
}

void B_BuildTicCommand(ticcmd_t* cmd, int playernum)
{
	size_t i;
	player_t* player = &players[playernum];
	bmind_t* mind = &BotMinds[playernum];
	angle_t	angle;
	int mx, my;
	int nx, ny;
	
	if (player->health < 1)
	{
		memset(mind->PathNodes, 0, sizeof(mind->PathNodes));
		mind->PathIterator = 0;
		mind->GatherTarget = NULL;
		mind->AttackTarget = NULL;
		
		cmd->buttons |= BT_USE;
	}
	else
	{
		B_LookForStuff(mind);

		if (botparm)
		{
			if (gametic % 10 == 0)
			{
				i = 0;
		
				while (mind->PathNodes[i])
				{
					P_SpawnMobj(
						mind->PathNodes[i]->x,
						mind->PathNodes[i]->y,
						mind->PathNodes[i]->subsector->sector->floorheight,
						MT_IFOG);
					i++;
				}
				
				if (mind->GatherTarget)
					P_SpawnMobj(
						mind->GatherTarget->x,
						mind->GatherTarget->y,
						mind->GatherTarget->z,
						MT_TFOG);
						
				if (mind->AttackTarget)
					P_SpawnMobj(
						mind->AttackTarget->x,
						mind->AttackTarget->y,
						mind->AttackTarget->z,
						MT_TFOG);
						
				for (i = 0; i < NumBotNodes; i++)
					if (BotReject[player->mo->subsector - subsectors][i])
						P_SpawnMobj(
						BotNodes[i].x,
						BotNodes[i].y,
						BotNodes[i].subsector->sector->floorheight,
						MT_PUFF);
			}
		}
		
		if (mind->GatherTarget)
		{
			if (mind->GatherTarget->mobjdontexist)
				mind->GatherTarget = NULL;
			else if (!(BotMobjCheck[mind->GatherTarget->type].func
				(
					mind, mind->GatherTarget,
					&BotMobjCheck[mind->GatherTarget->type],
					BotMobjCheck[mind->GatherTarget->type].poff)
				))
				mind->GatherTarget = NULL;
		}
		
		/* Follow a path */
		if (mind->PathNodes[mind->PathIterator])
		{
			if (mind->AttackTarget)
			{
				B_FaceMobj(mind, cmd, mind->AttackTarget);
				cmd->buttons |= BT_ATTACK;
			}
			else
			{
				B_FaceNode(mind, cmd, mind->PathNodes[mind->PathIterator]);
				cmd->forwardmove = botforwardmove[1];
			}
			
			if (!BotReject[player->mo->subsector - subsectors][mind->PathNodes[mind->PathIterator]->subsector - subsectors])
			{
				memset(mind->PathNodes, 0, sizeof(mind->PathNodes));
				mind->PathIterator = 0;
			}
			else if (player->mo->subsector == mind->PathNodes[mind->PathIterator]->subsector)
				if (B_PathDistance(player->mo, mind->PathNodes[mind->PathIterator]) <= 32)
				{
					mind->PathIterator++;
					if (!mind->PathNodes[mind->PathIterator])
					{
						memset(mind->PathNodes, 0, sizeof(mind->PathNodes));
						mind->PathIterator = 0;
					}
				}
		}
		
		/* Run tword an item*/
		else if (mind->GatherTarget)
		{
			// If we can't see the item but out reject says we can get to it, plot a new path
			if (!P_CheckSight(player->mo, mind->GatherTarget) &&
				BotReject[player->mo->subsector - subsectors][mind->GatherTarget->subsector - subsectors])
			{
				if (B_BuildPath(mind, mind->player->mo->subsector, mind->GatherTarget->subsector, 0) == BOTBADPATH)
					mind->GatherTarget = NULL;
			}
			else
			{
				B_FaceMobj(mind, cmd, mind->GatherTarget);
				cmd->forwardmove = botforwardmove[1];
			}
		}
		
		/* Shoot an enemy */
		else if (mind->AttackTarget)
		{
			B_FaceMobj(mind, cmd, mind->AttackTarget);
			cmd->buttons |= BT_ATTACK;
		}
	}
	
	// Lower resolution for demo recording goodness
	if (!longtics)
		cmd->angleturn = (cmd->angleturn + 128) & 0xff00;
}

