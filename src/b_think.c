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

fixed_t         botforwardmove[2] = {0x19, 0x32}; 
fixed_t         botsidemove[2] = {0x18, 0x28}; 
fixed_t         botangleturn[3] = {640, 1280, 320};    // + slow turn 

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
	
	if (player->health < 1)
	{
		memset(mind->PathNodes, 0, sizeof(mind->PathNodes));
		mind->PathIterator = 0;
		mind->GatherTarget = NULL;
		
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
						
				for (i = 0; i < NumBotNodes; i++)
					if (BotReject[player->mo->subsector - subsectors][i])
						P_SpawnMobj(
						BotNodes[i].x,
						BotNodes[i].y,
						BotNodes[i].subsector->sector->floorheight,
						MT_PUFF);
			}
		}
	
		// Do we have a target to follow
		if (mind->PathNodes[mind->PathIterator])
		{
			B_FaceNode(mind, cmd, mind->PathNodes[mind->PathIterator]);
			cmd->forwardmove = botforwardmove[1];
			
			if (!BotReject[player->mo->subsector - subsectors][mind->PathNodes[mind->PathIterator]->subsector - subsectors])
			{
				memset(mind->PathNodes, 0, sizeof(mind->PathNodes));
				mind->PathIterator = 0;
			}
			else if (player->mo->subsector == mind->PathNodes[mind->PathIterator]->subsector)
				if (B_PathDistance(player->mo, mind->PathNodes[mind->PathIterator]) <= 16)
					mind->PathIterator++;
		}
		else if (!mind->PathNodes[mind->PathIterator] && mind->PathIterator)
		{
			memset(mind->PathNodes, 0, sizeof(mind->PathNodes));
			mind->PathIterator = 0;
		}
		else if (!mind->PathNodes[mind->PathIterator] && !mind->PathIterator && mind->GatherTarget)
		{
			if (!BotReject[player->mo->subsector - subsectors][mind->GatherTarget->subsector - subsectors])
				mind->GatherTarget = NULL;
			else
			{
				B_FaceMobj(mind, cmd, mind->GatherTarget);
				cmd->forwardmove = botforwardmove[1];
			}
		}
	
		if (mind->GatherTarget && mind->GatherTarget->mobjdontexist)
			mind->GatherTarget = NULL;
	}
}

