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

int botplayer;
botcontrol_t botactions[MAXPLAYERS];
void B_PerformPress(botcontrol_t *mind);
int interuse = 0;
char *botmessage;

void B_BuildTicCommand(ticcmd_t* cmd)
{	
	botcontrol_t *me = &botactions[botplayer];
	me->cmd = cmd;
	me->cmd->buttons = 0;
	me->me = &players[botplayer];
	
	if (M_CheckParm("-buddy1"))
		me->allied[0] = 1;
	else
		me->allied[0] = 0;
	if (M_CheckParm("-buddy2"))
		me->allied[1] = 1;
	else
		me->allied[1] = 0;
	if (M_CheckParm("-buddy3"))
		me->allied[2] = 1;
	else
		me->allied[2] = 0;
	if (M_CheckParm("-buddy4"))
		me->allied[3] = 1;
	else
		me->allied[3] = 0;
		
	B_PerformPress(me);
	
	if (me->usecooldown > 70)
		me->usecooldown = 70;
	
	if (me->usecooldown > 0)
		me->usecooldown--;
	
	if (M_CheckParm("-botdebug"))
		if (gametic % 35 == 0)
			if (me->target)
				P_SpawnMobj(me->target->x, me->target->y, me->target->z, MT_IFOG);
	
	if (playeringame[botplayer])
	{	
		me->cmd->buttons = 0;
		
		if (me->me->mo)
		{
			if (me->me->health < 1)
				me->cmd->buttons |= BT_USE;
			else
				me->cmd->buttons = 0;
			
			switch(me->node)
			{	
				case BA_EXPLORING:	// Searching around the map
					B_Explore(me);
					break;
			
				case BA_GATHERING:
					B_Gather(me);
					break;
			
				case BA_ATTACKING:
					B_AttackTarget(me);
					break;
			
				default:
					me->node = BA_EXPLORING;
					break;
			}
			
			if (gametic % 2 == 0)
				B_PerformPress(me);
		}
		
		if (gamestate == GS_INTERMISSION)
		{
			if (interuse > 100)
				interuse--;
			else
			{
				me->cmd->buttons = BT_USE;
			}
		}
		else
		{
			interuse = 100;
		}
	}
	
	if (lowres_turn)
	{
		// round angleturn to the nearest 256 boundary
		// for recording demos with single byte values for turn

		me->cmd->angleturn = (me->cmd->angleturn + 128) & 0xff00;
	}
}

botcontrol_t *mind2;
mobj_t* botusething;
int botopenrange;

boolean	B_UseTraverse (intercept_t* in)
{
    int		side;
	
    if (!in->d.line->special)
    {
		P_LineOpening (in->d.line);
		if (botopenrange <= 0) // can't use through a wall
			return false;	
		
		return true; // not a special line, but keep checking
    }
	
    side = 0;
    if (P_PointOnLineSide (botusething->x, botusething->y, in->d.line) == 1)
		side = 1;
	
	if (mind2->usecooldown < 1)
	{
		mind2->cmd->buttons |= BT_USE;
		mind2->usecooldown = 70;
	}
	

    // can't use for than one special line in a row
    return false;
}

void B_PerformPress(botcontrol_t *mind)
{
	int		angle;
    fixed_t	x1;
    fixed_t	y1;
    fixed_t	x2;
    fixed_t	y2;
    
    mind2 = mind;
	
    botusething = mind->me->mo;
		
    angle = mind->me->mo->angle >> ANGLETOFINESHIFT;

    x1 = mind->me->mo->x;
    y1 = mind->me->mo->y;
    x2 = x1 + (USERANGE>>FRACBITS)*finecosine[angle];
    y2 = y1 + (USERANGE>>FRACBITS)*finesine[angle];
	
    P_PathTraverse ( x1, y1, x2, y2, PT_ADDLINES, B_UseTraverse);
}

void B_GoBackExploring(botcontrol_t *mind)
{
	mind->cmd->buttons = 0;
	mind->node = BA_EXPLORING;
	mind->target = NULL; 
}

void B_AttackTarget(botcontrol_t *mind)
{
	mind->cmd->buttons &= ~((byte)BT_ATTACK);
	BOTTEXT("ATTACKING");

	if (mind->target == NULL) // welp, it seems our target decided to die
		B_GoBackExploring(mind);
	else if (mind->target->player && (mind->target->player->team == mind->me->team) && (mind->me->team != 0))
		B_GoBackExploring(mind);
	else
	{
		if (P_CheckSight(mind->me->mo, mind->target))
		{
			if (mind->target->health > 0)
			{
				// First Face the target
				B_FaceTarget(mind);
				
				mind->attackcooldown++;

				// Forward Moving
				switch (mind->me->readyweapon)
				{
					case wp_fist:		// chharrrge!!
					case wp_chainsaw:
						// see if we can actually hit it with the fist/chainsaw...
						if ((mind->target->type == MT_BABY) ||
							(mind->target->type == MT_SPIDER))
						{
							int j;
							for (j = 0; j < NUMWEAPONS; j++)
							{
								if (mind->me->weaponowned[j] &&
									(j != wp_fist) &&
									(j != wp_chainsaw))
								{
									if (mind->me->ammo[weaponinfo[j].ammo] > 0)
									{
										mind->cmd->buttons |= BT_CHANGE; 
										mind->cmd->buttons |= j<<BT_WEAPONSHIFT;
										return;
									}
								}
							}
						}
						
						if (mind->forwardtics > 0)
						{
							if ((B_Random() % 10) == 0)
								mind->cmd->forwardmove += botforwardmove[0];
							else
								mind->cmd->forwardmove += botforwardmove[1];
			
							mind->forwardtics--;
						}
						else
							mind->forwardtics = B_Random() * 2;
							
						if (!(mind->cmd->buttons & BT_ATTACK))
							mind->cmd->buttons |= BT_ATTACK;
						break;
						
					case wp_bfg:
					case wp_missile:
					case wp_plasma:
						if (mind->forwardtics > 0)
						{
							if ((B_Random() % 10) == 0)
								mind->cmd->forwardmove += botforwardmove[0];
							else
								mind->cmd->forwardmove += botforwardmove[1];
			
							mind->forwardtics--;
						}
						else if (mind->forwardtics < 0)
						{
							if ((B_Random() % 10) == 0)
								mind->cmd->forwardmove -= botforwardmove[0];
							else
								mind->cmd->forwardmove -= botforwardmove[1];
			
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
							mind->cmd->sidemove += botsidemove[1];
							mind->sidetics--;
						}
						else if (mind->sidetics < 0)
						{
							mind->cmd->sidemove -= botsidemove[1];
							mind->sidetics++;
						}
						else
						{
							if (B_Random() % 2 == 0)
								mind->sidetics = B_Random();
							else
								mind->sidetics = -(B_Random());
						}
						
						if (B_Random() % 2 == 0)
							mind->cmd->angleturn -= B_Random() / 4;
						else
							mind->cmd->angleturn += B_Random() / 4;
						
						// Now shoot at it!
						if (!(mind->cmd->buttons & BT_ATTACK))
							mind->cmd->buttons |= BT_ATTACK;
						break;
						
					case wp_pistol:
					case wp_chaingun:
						if (mind->forwardtics > 0)
						{
							if ((B_Random() % 10) == 0)
								mind->cmd->forwardmove += botforwardmove[0];
							else
								mind->cmd->forwardmove += botforwardmove[1];
			
							mind->forwardtics--;
						}
						else if (mind->forwardtics < 0)
						{
							if ((B_Random() % 10) == 0)
								mind->cmd->forwardmove -= botforwardmove[0];
							else
								mind->cmd->forwardmove -= botforwardmove[1];
			
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
							mind->cmd->sidemove += botsidemove[1];
							mind->sidetics--;
						}
						else if (mind->sidetics < 0)
						{
							mind->cmd->sidemove -= botsidemove[1];
							mind->sidetics++;
						}
						else
						{
							if (B_Random() % 4 == 0)
								mind->sidetics = -(B_Random());
							else
								mind->sidetics = B_Random();
						}
						
						// Now shoot at it!
						if ((mind->me->readyweapon == wp_pistol))
						{
							if (mind->pistoltimeout == 0)
							{
								mind->pistoltimeout = 20; // 4 + 6 + 4 + 5 = 19 + 1 for good luck
								mind->cmd->buttons |= BT_ATTACK;
							}
							else
								mind->pistoltimeout--;
						}
						else if ((mind->me->readyweapon == wp_chaingun))
						{
							if (mind->chainguntimeout == 0)
							{
								mind->chainguntimeout = 9; // 4 + 4 + 0 = 8 + 1 for good luck
								mind->cmd->buttons |= BT_ATTACK;
							}
							else
								mind->chainguntimeout--;
						}
						break;
						
					case wp_shotgun:
					case wp_supershotgun:
					default:
						if ((B_Distance(mind->target, mind->me->mo) > 2048) && (mind->me->readyweapon == wp_supershotgun))
						{
							int j;
							for (j = 0; j < NUMWEAPONS; j++)
							{
								if (mind->me->weaponowned[j] &&
									(j != wp_supershotgun) &&
									(j != wp_fist) &&
									(j != wp_chainsaw))
								{
									if (mind->me->ammo[weaponinfo[j].ammo] > 0)
									{
										mind->cmd->buttons = BT_CHANGE; 
										mind->cmd->buttons |= j<<BT_WEAPONSHIFT;
										return;
									}
								}
							}
						}
				
						if (mind->forwardtics > 0)
						{
							if ((B_Random() % 10) == 0)
								mind->cmd->forwardmove += botforwardmove[0];
							else
								mind->cmd->forwardmove += botforwardmove[1];
			
							mind->forwardtics--;
						}
						else if (mind->forwardtics < 0)
						{
							if ((B_Random() % 10) == 0)
								mind->cmd->forwardmove -= botforwardmove[0];
							else
								mind->cmd->forwardmove -= botforwardmove[1];
			
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
							mind->cmd->sidemove += botsidemove[1];
							mind->sidetics--;
						}
						else if (mind->sidetics < 0)
						{
							mind->cmd->sidemove -= botsidemove[1];
							mind->sidetics++;
						}
						else
						{
							if (B_Random() % 2 == 0)
								mind->sidetics = B_Random();
							else
								mind->sidetics = -(B_Random());
						}
						
						// Now shoot at it!
						mind->cmd->buttons |= BT_ATTACK;
						break;
				}
			}
			else
				B_GoBackExploring(mind);
		}
		else
			B_GoBackExploring(mind);
	}
}

/*
if (intype == (x))
{
	if ((mind->target->flags & MF_DROPPED) && )
	{
		if (!(mind->me->ammo[weaponinfo[(thisgun)].ammo] < mind->me->maxammo[weaponinfo[(thisgun)].ammo]))
		{
			B_GoBackExploring(mind);
			return;
		}
	}
	else
	{
		if (mind->me->weaponowned[(thisgun)])
		{
			B_GoBackExploring(mind);
			return;
		}
	}
}
*/

#define DIDWEGETTHEWEAPON(x,thisgun) if (intype == (x))\
{\
	if ((mind->target->flags & MF_DROPPED) || (deathmatch == 2))\
	{\
		if (!(mind->me->ammo[weaponinfo[(thisgun)].ammo] < mind->me->maxammo[weaponinfo[(thisgun)].ammo]))\
		{\
			B_GoBackExploring(mind);\
			return;\
		}\
	}\
	else\
	{\
		if (mind->me->weaponowned[(thisgun)])\
		{\
			B_GoBackExploring(mind);\
			return;\
		}\
	}\
}

void B_Gather(botcontrol_t *mind)
{
	int intype = mind->target->type;
	
	BOTTEXT("GATHERING");
	
	if (mind->target == NULL)
		B_GoBackExploring(mind);
	else if (mind->me->attacker &&			// WHO THE FUCK PISSED ME OFF!?
		(mind->me->attacker->health > 0) &&
		(P_CheckSight(mind->me->mo, mind->me->attacker)))
	{
		mind->target = mind->me->attacker;
		mind->node = BA_ATTACKING;
	}
	else
	{	
		if (P_CheckSight(mind->me->mo, mind->target))
		{
			B_FaceTarget(mind);
			mind->cmd->forwardmove = botforwardmove[1];
			
			DIDWEGETTHEWEAPON(MT_CHAINGUN, wp_chaingun)
			DIDWEGETTHEWEAPON(MT_SHOTGUN, wp_shotgun)
			DIDWEGETTHEWEAPON(MT_SUPERSHOTGUN, wp_supershotgun)
			DIDWEGETTHEWEAPON(MT_MISC25, wp_bfg)
			DIDWEGETTHEWEAPON(MT_MISC26, wp_chainsaw)
			DIDWEGETTHEWEAPON(MT_MISC27, wp_missile)
			DIDWEGETTHEWEAPON(MT_MISC28, wp_plasma)
			
			if ((mind->target->state == S_NULL) || (mind->target->mobjdontexist))
				B_GoBackExploring(mind);
		}
		else
			B_GoBackExploring(mind);
	}
}

