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

fixed_t         botforwardmove[2] = {0x19, 0x32}; 
fixed_t         botsidemove[2] = {0x18, 0x28}; 
fixed_t         botangleturn[3] = {640, 1280, 320};    // + slow turn 

/*
{
	if ((mind->me->readyweapon == wp_fist) || (mind->me->readyweapon == wp_chainsaw))
	{
		if (((pri) >= targpin) && (((mobj_t*)currentthinker)->health > 0) && B_Distance(mind->me->mo, ((mobj_t*)currentthinker)) < distance)
		{
			distance = B_Distance(mind->me->mo, ((mobj_t*)currentthinker));
			targpin = (pri);
			newtarget = ((mobj_t*)currentthinker);
			mind->node = (nodex);
		}
	}
	else
	{
		if(((pri) >= targpin) && (((mobj_t*)currentthinker)->health > 0))
		{
			targpin = (pri);
			newtarget = ((mobj_t*)currentthinker);
			mind->node = (nodex);
		}
	}
}
*/

#define THISIS(x) (((mobj_t*)currentthinker)->type == (x))
#define SETTARGET(pri,nodex) \
{\
if ((mind->me->readyweapon == wp_fist) || (mind->me->readyweapon == wp_chainsaw))\
{\
if (((pri) >= targpin) && (((mobj_t*)currentthinker)->health > 0) && B_Distance(mind->me->mo, ((mobj_t*)currentthinker)) < distance)\
{\
distance = B_Distance(mind->me->mo, ((mobj_t*)currentthinker));\
targpin = (pri);\
newtarget = ((mobj_t*)currentthinker);\
mind->node = (nodex);\
}\
}\
else\
{\
if(((pri) >= targpin) && (((mobj_t*)currentthinker)->health > 0) && (B_Distance(((mobj_t*)currentthinker), mind->me->mo) < 2048))\
{\
targpin = (pri);\
newtarget = ((mobj_t*)currentthinker);\
mind->node = (nodex);\
}\
}\
}

/*	x pri thisgun
if (THISIS((x)) && !(mind->me->weaponowned[(thisgun)]) && (targpin < 10))
{
	newtarget = ((mobj_t*)currentthinker);
	mind->node = BA_GATHERING;
}
*/

#define CURTHINK ((mobj_t*)currentthinker)

#define DOWHENWEAPON(x,pri,thisgun) if (THISIS((x)) && (mind->me->weaponowned[(thisgun)] == 0) && ((pri) > targpin))\
{\
if ((mind->me->mo->z > (((mobj_t*)currentthinker)->z - 23)) &&\
(mind->me->mo->z < (((mobj_t*)currentthinker)->z + 23)))\
	{\
		newtarget = ((mobj_t*)currentthinker);\
		mind->node = BA_GATHERING;\
		targpin = (pri);\
	}\
}

#define DOWHENHEALTH(x,pri,heals) if (THISIS((x)) && (mind->me->health + (heals) < 100) && ((pri) > 	q targpin))\
{\
if ((mind->me->mo->z > (((mobj_t*)currentthinker)->z - 23)) &&\
(mind->me->mo->z < (((mobj_t*)currentthinker)->z + 23)))\
	{\
		newtarget = ((mobj_t*)currentthinker);\
		mind->node = BA_GATHERING;\
		targpin = (pri);\
	}\
}

//{if(((pri) >= targpin) && (((mobj_t*)currentthinker)->health > 0) && ()) { targpin = (pri); newtarget = ((mobj_t*)currentthinker); mind->node = (nodex);} }
#define DOWHENENEMY(x,pri,nodex) if (THISIS((x))) SETTARGET((pri),(nodex))
#define WHATDISTANCE (B_Distance(((mobj_t*)currentthinker), mind->me->mo))
#define TARMOBJ ((mobj_t*)currentthinker)

// returns distance
int B_Distance(mobj_t *a, mobj_t *b)
{
	double x1, x2;
	double y1, y2;
	double z1, z2;
	
	if (a && b)
	{
		x1 = a->x >> FRACBITS;
		x2 = b->x >> FRACBITS;
		y1 = a->y >> FRACBITS;
		y2 = b->y >> FRACBITS;
		z1 = a->z >> FRACBITS;
		z2 = b->z >> FRACBITS;
		
		return (int)sqrt(
			pow(x2 - x1, 2) +
			pow(y2 - y1, 2) +
			pow(z2 - z1, 2)
			);
	}
}

// Search through map objects and find something to do
void B_Look(botcontrol_t *mind)
{
	thinker_t*	currentthinker;
	mobj_t* newtarget;
	int targpin = 0;
	currentthinker = thinkercap.next;
	int distance = 10000;
	
	mind->target = NULL;
	
	while (currentthinker != &thinkercap)
	{
		if ((currentthinker->function.acp1 == (actionf_p1)P_MobjThinker))
		{
			if ((P_CheckSight(mind->me->mo, (mobj_t*)currentthinker)) &&
							(mind->me->mo != (mobj_t*)currentthinker))
			{
				/* ENEMY PLAYER */
				DOWHENENEMY(MT_PLAYER, 100, BA_ATTACKING)
				/*if ((((mobj_t*)currentthinker)->type == MT_PLAYER) && deathmatch)
				{
					int isanally;
					int i;
					
					for (i = 0; i < MAXPLAYERS; i++)
					{
						if (playeringame[i])
							if (((mobj_t*)currentthinker) == players[i].mo)
							{
								if (mind->allied[i] == 1)
									isanally = 1;
								else
									isanally = 0;
									
								break;
							}
					}
					
					if (isanally == 0)
					{
						if ((mind->me->readyweapon == wp_fist) || (mind->me->readyweapon == wp_chainsaw))
						{
							if ((100 >= targpin) && (((mobj_t*)currentthinker)->health > 0) && B_Distance(mind->me->mo, ((mobj_t*)currentthinker)) < distance)
							{
								distance = B_Distance(mind->me->mo, ((mobj_t*)currentthinker));
								targpin = 100;
								newtarget = ((mobj_t*)currentthinker);
								mind->node = BA_ATTACKING;
							}
						}
						else
						{
							if((100 >= targpin) && (((mobj_t*)currentthinker)->health > 0))
							{
								targpin = 100;
								newtarget = ((mobj_t*)currentthinker);
								mind->node = BA_ATTACKING;
							}
						}
					}
				}*/
		
				/* MONSTERS */
				// Boss Monsters that may win the game (ingore barons of hell here)
				DOWHENENEMY(MT_BOSSBRAIN, 99, BA_ATTACKING)
				DOWHENENEMY(MT_CYBORG, 98, BA_ATTACKING)
				DOWHENENEMY(MT_SPIDER, 97, BA_ATTACKING)
				DOWHENENEMY(MT_KEEN, 96, BA_ATTACKING)
				
				// Monsters that Give Ammo (Zombies)
				DOWHENENEMY(MT_CHAINGUY, 22, BA_ATTACKING)
				DOWHENENEMY(MT_SHOTGUY, 20, BA_ATTACKING)
				DOWHENENEMY(MT_WOLFSS, 19, BA_ATTACKING)
				DOWHENENEMY(MT_POSSESSED, 10, BA_ATTACKING)
				
				// Monsters that are annoying
				DOWHENENEMY(MT_PAIN, 40, BA_ATTACKING)
				
				// Monsters that are annoying when close
				if (WHATDISTANCE < 128)
				{
					DOWHENENEMY(MT_SKULL, 80, BA_ATTACKING)
					DOWHENENEMY(MT_SERGEANT, 82, BA_ATTACKING)
					DOWHENENEMY(MT_SHADOWS, 82, BA_ATTACKING)
					DOWHENENEMY(MT_UNDEAD, 90, BA_ATTACKING)
				}
				else
				{
					DOWHENENEMY(MT_SKULL, 10, BA_ATTACKING)
					DOWHENENEMY(MT_SERGEANT, 12, BA_ATTACKING)
					DOWHENENEMY(MT_SHADOWS, 12, BA_ATTACKING)
					DOWHENENEMY(MT_UNDEAD, 58, BA_ATTACKING)
				}
				
				// Monsters
				DOWHENENEMY(MT_VILE, 70, BA_ATTACKING)
				DOWHENENEMY(MT_BABY, 65, BA_ATTACKING)
				DOWHENENEMY(MT_FATSO, 65, BA_ATTACKING)
				DOWHENENEMY(MT_BRUISER, 60, BA_ATTACKING)
				DOWHENENEMY(MT_KNIGHT, 59, BA_ATTACKING)
				DOWHENENEMY(MT_TROOP, 19, BA_ATTACKING)
				DOWHENENEMY(MT_HEAD, 10, BA_ATTACKING)
		
				/* WEAPONS AND AMMO */
				DOWHENWEAPON(MT_CHAINGUN, 30, wp_chaingun)
				DOWHENWEAPON(MT_SHOTGUN, 30, wp_shotgun)
				DOWHENWEAPON(MT_SUPERSHOTGUN, 30, wp_supershotgun)
				DOWHENWEAPON(MT_MISC25, 30, wp_bfg)
				DOWHENWEAPON(MT_MISC26, 30, wp_chainsaw)
				DOWHENWEAPON(MT_MISC27, 30, wp_missile)
				DOWHENWEAPON(MT_MISC28, 30, wp_plasma)
				
				/* HEALTH */
				/*DOWHENHEALTH(MT_MISC10, 40, 10)	// Stimpack
				DOWHENHEALTH(MT_MISC11, 40, 25) // Medkit
				DOWHENHEALTH(MT_MISC2, 40, -1) // Health bonus*/
			}
		}
		
		// next
		currentthinker = currentthinker->next;
	}
	
	mind->target = newtarget;
}

void B_Explore(botcontrol_t *mind)
{
	BOTTEXT("EXPLORING");
	
	B_Look(mind);
	
	if (mind->node == BA_EXPLORING)
	{
		// Forward Moving
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
			if ((B_Random() % 4) == 0)
				mind->forwardtics = -(B_Random() / 5);
			else
				mind->forwardtics = B_Random() * 2;
		}
		
		// Turning
		if (mind->turntics > 0)
		{
			if ((B_Random() % 10) == 0)
				mind->cmd->angleturn -= botangleturn[1];
			else
				mind->cmd->angleturn -= botangleturn[2];
			
			mind->turntics--;
		}
		else if (mind->forwardtics > 0)
		{
			if ((B_Random() % 10) == 0)
				mind->cmd->angleturn += botangleturn[1];
			else
				mind->cmd->angleturn += botangleturn[2];
			
			mind->turntics++;
		}
		else
		{
			if ((B_Random() % 3) == 0)
				mind->turntics = -(B_Random() * 4);
			else if ((B_Random() % 3) == 1)
				mind->turntics = 0;
			else
				mind->turntics = B_Random() * 4;
		}
		
		// Strafing
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
	}
}

void B_FaceTarget(botcontrol_t *mind)
{
	angle_t victimangle = 0;
	angle_t myangle = 0;
	angle_t actualangle = 0;
	angle_t virtualangle = 0;
	int someactualangle = 0;
	int somevirtualangle = 0;
	int somemyangle = 0;
	int someoffset = 0;
	
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
}

