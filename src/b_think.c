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

int botplayer = 0;
botcontrol_t botactions[MAXPLAYERS];
void B_PerformPress(botcontrol_t *mind);
int interuse = 0;

void B_BuildTicCommand(ticcmd_t* cmd)
{
	botplayer = consoleplayer;
	botcontrol_t *me = &botactions[botplayer];
	me->cmd = cmd;
	me->cmd->buttons = 0;
	me->me = &players[botplayer];
	
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
			
			if (gametic % 35 == 0)
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
		
	mind2->cmd->buttons |= BT_USE;

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

static void B_GoBackExploring(botcontrol_t *mind)
{
	mind->cmd->buttons = 0;
	mind->node = BA_EXPLORING;
	mind->target = NULL;    
}

void B_AttackTarget(botcontrol_t *mind)
{
	angle_t victimangle = 0;
	angle_t myangle = 0;
	angle_t actualangle = 0;
	angle_t virtualangle = 0;
	int someactualangle = 0;
	int somevirtualangle = 0;
	int somemyangle = 0;
	int someoffset = 0;
	
	mind->cmd->buttons &= ~((byte)BT_ATTACK);

	if (mind->target == NULL) // right...
	{
		mind->node = BA_EXPLORING;
		mind->target = NULL;
		B_Explore(mind);
	}
	else
	{
		if (P_CheckSight(mind->me->mo, mind->target))
		{
			if (mind->target->health > 0)
			{
				// First Face the target
				actualangle = R_PointToAngle2 (mind->me->mo->x, mind->me->mo->y, mind->target->x ,mind->target->y);
				virtualangle = mind->me->mo->angle;
				myangle = mind->me->mo->angle;
				
				/* Thinking some more
					player->mo->angle += (cmd->angleturn<<16);
				*/
				
				someactualangle = actualangle >> 16;
				somevirtualangle = virtualangle >> 16;
				somemyangle = myangle >> 16;
				
				while (somevirtualangle != someactualangle)
				{
					if (somevirtualangle + someoffset < someactualangle)
						someoffset++;
					else if (somevirtualangle + someoffset > someactualangle)
						someoffset--;
					else
					{
						mind->cmd->angleturn += someoffset;
						break;
					}
				}

				// Forward Moving
				switch (mind->me->readyweapon)
				{
					case wp_fist:		// chharrrge!!
					case wp_chainsaw:
						if (mind->forwardtics > 0)
						{
							if ((B_Random() % 10) == 0)
								mind->cmd->forwardmove = botforwardmove[0];
							else
								mind->cmd->forwardmove = botforwardmove[1];
			
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
								mind->cmd->forwardmove = botforwardmove[0];
							else
								mind->cmd->forwardmove = botforwardmove[1];
			
							mind->forwardtics--;
						}
						else if (mind->forwardtics < 0)
						{
							if ((B_Random() % 10) == 0)
								mind->cmd->forwardmove = -botforwardmove[0];
							else
								mind->cmd->forwardmove = -botforwardmove[1];
			
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
							mind->cmd->sidemove = botsidemove[1];
							mind->sidetics--;
						}
						else if (mind->sidetics < 0)
						{
							mind->cmd->sidemove = -botsidemove[1];
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
								mind->cmd->forwardmove = botforwardmove[0];
							else
								mind->cmd->forwardmove = botforwardmove[1];
			
							mind->forwardtics--;
						}
						else if (mind->forwardtics < 0)
						{
							if ((B_Random() % 10) == 0)
								mind->cmd->forwardmove = -botforwardmove[0];
							else
								mind->cmd->forwardmove = -botforwardmove[1];
			
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
							mind->cmd->sidemove = botsidemove[1];
							mind->sidetics--;
						}
						else if (mind->sidetics < 0)
						{
							mind->cmd->sidemove = -botsidemove[1];
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
						if (mind->forwardtics > 0)
						{
							if ((B_Random() % 10) == 0)
								mind->cmd->forwardmove = botforwardmove[0];
							else
								mind->cmd->forwardmove = botforwardmove[1];
			
							mind->forwardtics--;
						}
						else if (mind->forwardtics < 0)
						{
							if ((B_Random() % 10) == 0)
								mind->cmd->forwardmove = -botforwardmove[0];
							else
								mind->cmd->forwardmove = -botforwardmove[1];
			
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
							mind->cmd->sidemove = botsidemove[1];
							mind->sidetics--;
						}
						else if (mind->sidetics < 0)
						{
							mind->cmd->sidemove = -botsidemove[1];
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

void B_Gather(botcontrol_t *mind)
{
}

