//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2005-2014 Simon Howard
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
// DESCRIPTION:
//	Player related stuff.
//	Bobbing POV/weapon, movement.
//	Pending weapon.
//




#include "doomdef.h"
#include "d_compat.h"
#include "d_event.h"

#include "p_local.h"

#include "doomstat.h"

#include "deh_str.h"
#include "dstrings.h"


// Index of the special effects (INVUL inverse) map.
#define INVERSECOLORMAP		32


//
// Movement.
//

// 16 pixels of bob
#define MAXBOB	0x100000	

boolean		onground;

void DropBackpack(player_t* player)
{
	int x, y;
	unsigned an;
	mobj_t* backpack;
	mobjtype_t item = 0;

	if (player->playerstate == PST_DEAD)
		return;

	if (player->ammo[am_clip] > 20
		&& player->ammo[am_shell] > 8
		&& player->ammo[am_misl] > 4
		&& player->ammo[am_cell] > 40)
	{
		item = MT_MISC95;
	}
	else if (player->ammo[am_clip] > 20
		&& player->ammo[am_shell] > 8
		&& player->ammo[am_misl] > 4)
	{
		item = MT_MISC94;
	}
	else if (player->ammo[am_clip] > 20
		&& player->ammo[am_shell] > 8)
	{
		item = MT_MISC93;
	}
	else if (player->ammo[am_clip] > 20)
	{
		item = MT_MISC92;
	}
	else
	{
		player->message = DEH_String(NOAMMO);
		return;
	}

	an = player->mo->angle >> ANGLETOFINESHIFT; 

	x = player->mo->x + FixedMul (3*24*FRACUNIT, finecosine[an]);
	y = player->mo->y + FixedMul (3*24*FRACUNIT, finesine[an]);

	backpack = P_SpawnMobj (x, y, ONFLOORZ, item);
	if (!P_CheckPosition (backpack, backpack->x, backpack->y))
	{
		P_RemoveMobj (backpack);
		return;
	}
	backpack->flags |= MF_DROPPED;

	if (item == MT_MISC95)
		player->ammo[am_cell] -= 40;
	if (item >= MT_MISC94)
		player->ammo[am_misl] -= 4;
	if (item >= MT_MISC93)
		player->ammo[am_shell] -= 8;
	player->ammo[am_clip] -= 20;
}

//
// P_Thrust
// Moves the given origin along a given angle.
//
void
P_Thrust
( player_t*	player,
  angle_t	angle,
  fixed_t	move ) 
{
    angle >>= angletocoarseshift;

    player->mo->momx += FixedMul(move, coarsecosine[angle]); 
    player->mo->momy += FixedMul(move, coarsesine[angle]);
}




//
// P_CalcHeight
// Calculate the walking / running height adjustment
//
void P_CalcHeight (player_t* player) 
{
    int		angle;
    fixed_t	bob;
    
    // Regular movement bobbing
    // (needs to be calculated for gun swing
    // even if not on ground)
    // OPTIMIZE: tablify angle
    // Note: a LUT allows for effects
    //  like a ramp with low health.
    player->bob =
	FixedMul (player->mo->momx, player->mo->momx)
	+ FixedMul (player->mo->momy,player->mo->momy);
    
    player->bob >>= 2;

    if (player->bob>MAXBOB)
	player->bob = MAXBOB;

    if ((player->cheats & CF_NOMOMENTUM) || !onground)
    {
	player->viewz = player->mo->z + VIEWHEIGHT;

	if (player->viewz > player->mo->ceilingz-4*FRACUNIT)
	    player->viewz = player->mo->ceilingz-4*FRACUNIT;

	player->viewz = player->mo->z + player->viewheight;
	return;
    }
	
    angle = (coarseangles/20*leveltime)&coarsemask;
    bob = FixedMul ( player->bob/2, coarsesine[angle]);
    
    // move viewheight
    if (player->playerstate == PST_LIVE)
    {
	player->viewheight += player->deltaviewheight;

	if (player->viewheight > VIEWHEIGHT)
	{
	    player->viewheight = VIEWHEIGHT;
	    player->deltaviewheight = 0;
	}

	if (player->viewheight < VIEWHEIGHT/2)
	{
	    player->viewheight = VIEWHEIGHT/2;
	    if (player->deltaviewheight <= 0)
		player->deltaviewheight = 1;
	}
	
	if (player->deltaviewheight)	
	{
	    player->deltaviewheight += FRACUNIT/4;
	    if (!player->deltaviewheight)
		player->deltaviewheight = 1;
	}
    }
    player->viewz = player->mo->z + player->viewheight + bob;

    if (player->viewz > player->mo->ceilingz-4*FRACUNIT)
	player->viewz = player->mo->ceilingz-4*FRACUNIT;
}



//
// P_MovePlayer
//
void P_MovePlayer (player_t* player, boolean justattacked)
{
    ticcmd_t*		cmd;
    fixed_t forwardmove, sidemove;
	
    cmd = &player->cmd;
	
    player->mo->angle += (cmd->angleturn<<FRACBITS);

    forwardmove = cmd->forwardmove * cmd_move_scale;
    sidemove = cmd->sidemove * cmd_move_scale;

    // Need to set this manually, as 0xc800 doesn't divide cleanly by 900
    if (justattacked && gameversion < exe_doom_1_2)
    {
        forwardmove = 0xc800;
    }

    // Do not let the player control movement
    //  if not onground.
    onground = (player->mo->z <= player->mo->floorz);
	
    if (forwardmove && onground)
	P_Thrust (player, player->mo->angle, forwardmove);
    
    if (sidemove && onground)
	P_Thrust (player, player->mo->angle-ANG90, sidemove);

    if ( (forwardmove || sidemove) 
	 && player->mo->state == &states[S_PLAY] )
    {
	P_SetMobjState (player->mo, S_PLAY_RUN1);
    }
}	



//
// P_DeathThink
// Fall on your face when dying.
// Decrease POV height to floor height.
//
#define ANG5   	(ANG90/18)

void P_DeathThink (player_t* player)
{
    angle_t		angle;
    angle_t		delta;

    P_MovePsprites (player);
	
    // fall to the ground
    if (player->viewheight > 6*FRACUNIT)
	player->viewheight -= FRACUNIT;

    if (player->viewheight < 6*FRACUNIT)
	player->viewheight = 6*FRACUNIT;

    player->deltaviewheight = 0;
    onground = (player->mo->z <= player->mo->floorz);
    P_CalcHeight (player);
	
    if (player->attacker && player->attacker != player->mo)
    {
	angle = R_PointToAngle2 (player->mo->x,
				 player->mo->y,
				 player->attacker->x,
				 player->attacker->y);
	
	delta = angle - player->mo->angle;
	
	if (delta < ANG5 || delta > (unsigned)-ANG5)
	{
	    // Looking at killer,
	    //  so fade damage flash down.
	    player->mo->angle = angle;

	    if (player->damagecount)
		player->damagecount--;
	}
	else if (delta < ANG180)
	    player->mo->angle += ANG5;
	else
	    player->mo->angle -= ANG5;
    }
    else if (player->damagecount)
	player->damagecount--;
	

    if (player->cmd.buttons & BT_USE)
	player->playerstate = PST_REBORN;
}



//
// P_PlayerThink
//
void P_PlayerThink (player_t* player)
{
    ticcmd_t*		cmd;
    weapontype_t	newweapon;
    boolean         justattacked = false;
	
    // fixme: do this in the cheat code
    if (player->cheats & CF_NOCLIP)
	player->mo->flags |= MF_NOCLIP;
    else
	player->mo->flags &= ~MF_NOCLIP;
    
    // chain saw run forward
    cmd = &player->cmd;
    if (player->mo->flags & MF_JUSTATTACKED)
    {
	cmd->angleturn = 0;
	cmd->forwardmove = 0xc800/512;
	cmd->sidemove = 0;
        justattacked = true;
	player->mo->flags &= ~MF_JUSTATTACKED;
    }
	
    if (player->playerstate == PST_DEAD)
    {
	P_DeathThink (player);
	return;
    }
    
    // Move around.
    // Reactiontime is used to prevent movement
    //  for a bit after a teleport.
    if (player->mo->reactiontime)
	player->mo->reactiontime--;
    else
	P_MovePlayer (player, justattacked);
    
    P_CalcHeight (player);

    if (player->mo->subsector->sector->special)
	P_PlayerInSpecialSector (player);
    
    // Check for weapon change.

    // A special event has no other buttons.
    if (cmd->buttons & BT_SPECIAL)
	cmd->buttons = 0;			
		
    if (cmd->buttons & BT_CHANGE)
    {
	// The actual changing of the weapon is done
	//  when the weapon psprite can do it
	//  (read: not in the middle of an attack).
	newweapon = (cmd->buttons&BT_WEAPONMASK)>>BT_WEAPONSHIFT;
	
	// [crispy] allow to toggle Fist/Chainsaw
	if (newweapon == wp_fist
	    && player->weaponowned[wp_chainsaw]
	    && !(player->readyweapon == wp_chainsaw
		 && (player->powers[pw_strength] || gameskill == sk_extreme)))
	{
	    newweapon = wp_chainsaw;
	}

	if ( (gamemode == commercial)
	    && newweapon == wp_shotgun 
	    && player->weaponowned[wp_supershotgun]
	    && player->readyweapon != wp_supershotgun
	    && !nod2monsters )
	{
	    newweapon = wp_supershotgun;
	}

	if ( (gamemode == commercial)
	    && newweapon == wp_shotgun 
	    && player->weaponowned[wp_supershotgun]
	    && player->readyweapon == wp_shotgun
	    && nod2monsters )
	{
	    newweapon = wp_supershotgun;
	}

	if (player->weaponowned[newweapon]
	    && newweapon != player->readyweapon)
	{
	    // Do not go to plasma or BFG in shareware,
	    //  even if cheated.
	    if ((newweapon != wp_plasma
		 && newweapon != wp_bfg)
		|| (gamemode != shareware) )
	    {
		player->pendingweapon = newweapon;
	    }
	}
    }
    
    // check for use
    if (cmd->buttons & BT_USE)
    {
	if (!player->usedown)
	{
	    P_UseLines (player);
	    player->usedown = true;
	}
    }
    else
	player->usedown = false;

    // check for drop
    if (cmd->buttons & BT_DROP)
    {
		DropBackpack(player);
	}

    // cycle psprites
    P_MovePsprites (player);
    
    // Counters, time dependend power ups.

    // Strength counts up to diminish fade.
    if (player->powers[pw_strength])
	player->powers[pw_strength]++;	
		
    if (player->powers[pw_invulnerability])
	player->powers[pw_invulnerability]--;

    if (player->powers[pw_invisibility])
	if (! --player->powers[pw_invisibility] )
	    player->mo->flags &= ~MF_SHADOW;
			
    if (player->powers[pw_infrared])
	player->powers[pw_infrared]--;
		
    if (player->powers[pw_ironfeet])
	player->powers[pw_ironfeet]--;
		
    if (player->damagecount)
	player->damagecount--;
		
    if (player->bonuscount)
	player->bonuscount--;

    
    // Handling colormaps.
    if (player->powers[pw_invulnerability])
    {
	if (player->powers[pw_invulnerability] > 4*32
	    || (player->powers[pw_invulnerability]&8) )
	    player->fixedcolormap = INVERSECOLORMAP;
	else
	    player->fixedcolormap = 0;
    }
    else if (player->powers[pw_infrared])	
    {
	if (player->powers[pw_infrared] > 4*32
	    || (player->powers[pw_infrared]&8) )
	{
	    // almost full bright
	    player->fixedcolormap = 1;
	}
	else
	    player->fixedcolormap = 0;
    }
    else
	player->fixedcolormap = 0;
}


