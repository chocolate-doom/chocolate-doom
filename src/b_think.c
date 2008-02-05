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

int botplayer = 0;
botcontrol_t botactions[MAXPLAYERS];

void B_BuildTicCommand(ticcmd_t* cmd)
{
	botcontrol_t *me = &botactions[botplayer];
	
	switch(botactions[botplayer])
	{	
		case BA_EXPLORING:	// Searching around the map
			
			break;
			
		case BA_ATTACKING:
			B_AttackTarget(me);
			break;
			
		default:
			botactions[botplayer].node = BA_EXPLORING;
			break;
	}
}

