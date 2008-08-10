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
#include <math.h>

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

/* B_FindBetterWeapon() -- Searches for a better weapon */
void B_FindBetterWeapon(bmind_t* mind, ticcmd_t* cmd)
{
	int changeto = -1;
	
	/* Target is a player */
	if (mind->AttackTarget->type == MT_PLAYER)
	{	
	}
	
	/* Target is a monster */
	else
	{
	}
	
	if (changeto != -1)
	{
		cmd->buttons |= BT_CHANGE;
		cmd->buttons |= changeto << BT_WEAPONSHIFT;
	}
}

/* B_CheckFriendlyFire() -- Checks to see if we are going to hurt a buddy */
int B_PTR_FFCheck(intercept_t* in)
{
	if (!deathmatch && in->d.thing->type == MT_PLAYER)
		return true;
	else
		return false;
}

int B_CheckFriendlyFire(bmind_t* mind)
{
	if (deathmatch)
		return 1;
	else
		return P_PathTraverse(
			mind->player->mo->x, mind->player->mo->y,
			mind->AttackTarget->x, mind->AttackTarget->y,
			PT_ADDTHINGS, B_PTR_FFCheck);
}

int dopress = 0;
bmind_t* mindx = NULL;

int B_PTR_PressCheck(intercept_t* in)
{
    int		side;
	
    if (!in->d.line->special)
    {
		P_LineOpening(in->d.line);
		if (openrange <= 0) // can't use through a wall
			return false;	
		
		return true; // not a special line, but keep checking
    }
	
    /*side = 0;
    
    if (P_PointOnLineSide (mindx->player->mo->x, mindx->player->mo->y, in->d.line) == 1)
		side = 1;*/
	
	// togglable door!
	switch (in->d.line->special)
	{
		case 1:
		case 117:
			if (in->d.line->backsector->ceilingheight == in->d.line->backsector->floorheight)
				dopress = 1;
			return false;
			
		default:
			return false;
	}
}

int B_CheckPress(bmind_t* mind)
{
	dopress = 0;
	mindx = mind;
	
	if (!mind)
		return 0;
	if (!mind->player)
		return 0;
	if (!mind->player->mo)
		return 0;
	
	P_PathTraverse(mind->player->mo->x, mind->player->mo->y,
		mind->player->mo->x + ((USERANGE>>FRACBITS)*finecosine[mind->player->mo->angle >> ANGLETOFINESHIFT]),
		mind->player->mo->y + ((USERANGE>>FRACBITS)*finesine[mind->player->mo->angle >> ANGLETOFINESHIFT]),
		PT_ADDLINES, B_PTR_PressCheck);
	
	return dopress;
}

int EnterTic = 0;

void B_BuildTicCommand(ticcmd_t* cmd, int playernum)
{
	size_t i;
	player_t* player = &players[playernum];
	bmind_t* mind = &BotMinds[playernum];
	angle_t	angle;
	int mx, my;
	int nx, ny;
	int* timer;
	int px;
	int terminate;
	
	if (gamestate == GS_INTERMISSION)
	{
		if (!EnterTic)
			EnterTic = gametic;
		else
			if (gametic - EnterTic > (TICRATE * 5))
				if (M_CheckParm("-bot"))
				{
					cmd->buttons |= BT_USE;
					EnterTic = 0;
				}
	}
	else if (player->health < 1)
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
			}
		}
		
		
		/**** DETERMINE IF WE SHOULD KEEP ATTACKING/GATHERING ****/
		// Note: Yes, I know this can be made simpler by using logical ORs
		// but I am doing this for code generation, instead of checking all
		// possibilities, check some of them and if it happens just fall out
		// if and if statement evaluates the contents are executed then control
		// is set after all of the statements, improves speed a bit.
		
		/* Do we stop attacking monsters? */
		if (mind->AttackTarget)
		{
			if (mind->AttackTarget->mobjdontexist)
				mind->AttackTarget = NULL;
			else if (mind->AttackTarget->health < 1)
				mind->AttackTarget = NULL;
			else if (!P_CheckSight(mind->player->mo, mind->AttackTarget))
				mind->AttackTarget = NULL;
			else if (!(BotMobjCheck[mind->AttackTarget->type].func
				(
					mind, mind->AttackTarget,
					&BotMobjCheck[mind->AttackTarget->type],
					BotMobjCheck[mind->AttackTarget->type].poff)
				))
				mind->AttackTarget = NULL;
				
			if (!mind->AttackTarget)
				mind->flags &= ~BF_ATTACKING;
		}
		
		/* Do we stop gathering items? */
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
				
			if (!mind->GatherTarget)
				mind->flags &= ~BF_GATHERING;
		}
		
		/* Follow a path */
		if (mind->PathNodes[mind->PathIterator])
		{
			if (B_CheckPress(mind))
				cmd->buttons |= BT_USE;
			else
				cmd->buttons &= ~BT_USE;
			
			/*if (mind->AttackTarget)
			{
				B_FaceMobj(mind, cmd, mind->AttackTarget);
				cmd->buttons |= BT_ATTACK;
			}
			else
			{*/
				B_FaceNode(mind, cmd, mind->PathNodes[mind->PathIterator]);
				cmd->forwardmove = botforwardmove[1];
			/*}*/
			
			// If we happen to have landed in a sector where we can't possibly get to our goal
			if (!B_CheckLine(mind, player->mo->subsector, mind->PathNodes[mind->PathIterator]->subsector))
			{
				// Increment the failcount
				mind->failcount++;
				
				px = mind->priority + (mind->failcount >> 1);
				
				if (px > 3)
					px = 3;
					
				terminate = 0;
				
				// Attempt to rebuild the path
				if (mind->failcount < (1 << 4))
				{
					if (B_BuildPath(mind, mind->player->mo->subsector, mind->GatherTarget->subsector, 0, px) == BOTBADPATH)
						terminate = 1;
				}
				else
					terminate = 1;
				
				if (terminate)
				{
					// Failed too many times, drop the path and ignore the item
					if (mind->GatherTarget)
					{
						mind->failtype = mind->GatherTarget->type;
						mind->GatherTarget = NULL;
					}
					
					mind->failcount = 0;
					mind->priority = 0;
					
					// There was obviously an error in the node system
					if (mind->PathIterator)
						BotReject[mind->PathNodes[mind->PathIterator - 1]->subsector - subsectors]
							[mind->PathNodes[mind->PathIterator]->subsector - subsectors].Mode = 0;
					
					// Clear Path
					memset(mind->PathNodes, 0, sizeof(mind->PathNodes));
					mind->PathIterator = 0;
				}
			}
			
			// If we reached the destination, iterate
			else if (player->mo->subsector == mind->PathNodes[mind->PathIterator]->subsector)
				if (B_PathDistance(player->mo, mind->PathNodes[mind->PathIterator]) <= 32)
				{
					mind->PathIterator++;
					if (!mind->PathNodes[mind->PathIterator])
					{
						memset(mind->PathNodes, 0, sizeof(mind->PathNodes));
						mind->PathIterator = 0;
						mind->failcount = 0;
					}
				}
		}
		
		/* Run tword an item*/
		else if (mind->GatherTarget)
		{
			// If we can't see the item but our reject says we can get to it, plot a new path
			if (!P_CheckSight(player->mo, mind->GatherTarget) &&
				B_CheckLine(mind, player->mo->subsector, mind->GatherTarget->subsector))
			{
				if (B_BuildPath(mind, mind->player->mo->subsector, mind->GatherTarget->subsector, 0, mind->priority) == BOTBADPATH)
					mind->GatherTarget = NULL;
			}
			// We are trying to go for a straight path to the item but we can't get to it
			else if (!B_CheckLine(mind, player->mo->subsector, mind->GatherTarget->subsector))
				mind->GatherTarget = NULL;
			// None of the above
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
				
			switch (mind->player->readyweapon)
			{
				/* Melee Weapons */
				case wp_fist:
				case wp_chainsaw:
					if (mind->AttackTarget->info->radius > 60 * FRACUNIT)
						B_FindBetterWeapon(mind, cmd);
					else
					{	// Charge twords the opponent
						cmd->forwardmove = botforwardmove[1];
						cmd->buttons |= BT_ATTACK;
					}
					break;
					
				/* Hitscan weapons */
				// Accurate (Tap)
				case wp_pistol:
					if (gametic >= mind->pistoltimeout)
					{
						mind->pistoltimeout = gametic + 20; // 4 + 6 + 4 + 5 = 19 + 1 for good luck

						if (B_CheckFriendlyFire(mind))
							cmd->buttons |= BT_ATTACK;
					}
					break;
				
				case wp_chaingun:
					if (gametic >= mind->chainguntimeout)
					{
						mind->chainguntimeout = gametic + 9; // 4 + 4 + 0 = 8 + 1 for good luck

						if (B_CheckFriendlyFire(mind))
							cmd->buttons |= BT_ATTACK;
					}
					break;
				
				/* Other Weapons */
				case wp_shotgun:
				case wp_supershotgun:
				case wp_plasma:
				case wp_missile:
				case wp_bfg:
				default:
					cmd->buttons |= BT_ATTACK;
					break;
			}
			
			// Strafing
			if (((mind->player->readyweapon >= wp_pistol) && (mind->player->readyweapon <= wp_bfg)) ||
				(mind->player->readyweapon == wp_supershotgun))
			{
				if (mind->forwardtics > 0)
				{
					cmd->forwardmove += botforwardmove[1];
					mind->forwardtics--;
				}
				else if (mind->forwardtics < 0)
				{
					cmd->forwardmove -= botforwardmove[1];
					mind->forwardtics++;
				}
				else
				{
					if ((B_Random() % 2) == 0)
						mind->forwardtics = -(B_Random() / 2);
					else
						mind->forwardtics = B_Random() / 2;
				}
		
				// Strafe and dodging
				if (mind->sidetics > 0)
				{
					cmd->sidemove += botsidemove[1];
					mind->sidetics--;
				}
				else if (mind->sidetics < 0)
				{
					cmd->sidemove -= botsidemove[1];
					mind->sidetics++;
				}
				else
				{
					if (B_Random() % 2 == 0)
						mind->sidetics = B_Random();
					else
						mind->sidetics = -(B_Random());
				}
			}
		}
	}
	
	// Lower resolution for demo recording goodness
	if (!longtics)
		cmd->angleturn = (cmd->angleturn + 128) & 0xff00;
}

