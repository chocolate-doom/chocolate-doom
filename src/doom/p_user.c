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




#include <stdlib.h> // [crispy] abs()
#include "doomdef.h"
#include "d_event.h"

#include "p_local.h"

#include "doomstat.h"
#include "a11y.h" // [crispy] A11Y

#include "s_sound.h" // [NS] Jump sound etc.


// Index of the special effects (INVUL inverse) map.
#define INVERSECOLORMAP		32


//
// Movement.
//

// 16 pixels of bob
#define MAXBOB	0x100000	

// [crispy] variable player view bob
static const fixed_t crispy_bobfactor[3] = {4, 3, 0};

boolean		onground;


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
    angle >>= ANGLETOFINESHIFT;
    
    player->mo->momx += FixedMul(move,finecosine[angle]); 
    player->mo->momy += FixedMul(move,finesine[angle]);
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

    // [crispy] variable player view bob
    player->bob2 = crispy_bobfactor[crispy->bobfactor] * player->bob / 4;

    if ((player->cheats & CF_NOMOMENTUM) || !onground)
    {
	player->viewz = player->mo->z + VIEWHEIGHT;

	if (player->viewz > player->mo->ceilingz-4*FRACUNIT)
	    player->viewz = player->mo->ceilingz-4*FRACUNIT;

	// [crispy] fix player viewheight in NOMOMENTUM mode
	//player->viewz = player->mo->z + player->viewheight;
	return;
    }
		
    angle = (FINEANGLES/20*leveltime)&FINEMASK;
    bob = FixedMul ( player->bob2/2, finesine[angle]); // [crispy] variable player view bob

    
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
void P_MovePlayer (player_t* player)
{
    ticcmd_t*		cmd;
    int		look;
	
    cmd = &player->cmd;
	
    player->mo->angle += (cmd->angleturn<<FRACBITS);

    // Do not let the player control movement
    //  if not onground.
    onground = (player->mo->z <= player->mo->floorz);
    // [crispy] give full control in no-clipping mode
    onground |= (player->mo->flags & MF_NOCLIP);
	
    // [crispy] fast polling
    if (player == &players[consoleplayer])
    {
        localview.ticangle += localview.ticangleturn << 16;
        localview.ticangleturn = 0;
    }

    if (cmd->forwardmove && onground)
	P_Thrust (player, player->mo->angle, cmd->forwardmove*2048);
    else
    // [crispy] in-air movement is only possible with jumping enabled
    if (cmd->forwardmove && critical->jump)
        P_Thrust (player, player->mo->angle, FRACUNIT >> 8);
    
    if (cmd->sidemove && onground)
	P_Thrust (player, player->mo->angle-ANG90, cmd->sidemove*2048);
    else
    // [crispy] in-air movement is only possible with jumping enabled
    if (cmd->sidemove && critical->jump)
            P_Thrust(player, player->mo->angle-ANG90, FRACUNIT >> 8);

    if ( (cmd->forwardmove || cmd->sidemove) 
	 && player->mo->state == &states[S_PLAY] )
    {
	P_SetMobjState (player->mo, S_PLAY_RUN1);
    }

    // [crispy] apply lookdir delta
    look = cmd->lookfly & 15;
    if (look > 7)
    {
        look -= 16;
    }
    if (look)
    {
        if (look == TOCENTER)
        {
            player->centering = true;
        }
        else
        {
            cmd->lookdir = MLOOKUNIT * 5 * look;
        }
    }
    if (!menuactive && !demoplayback)
    {
	player->lookdir = BETWEEN(-LOOKDIRMIN * MLOOKUNIT,
	                          LOOKDIRMAX * MLOOKUNIT,
	                          player->lookdir + cmd->lookdir);
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
	
    // [AM] Assume we can interpolate at the beginning
    //      of the tic.
    player->mo->interp = true;

    // [AM] Store starting position for player interpolation.
    player->mo->oldx = player->mo->x;
    player->mo->oldy = player->mo->y;
    player->mo->oldz = player->mo->z;
    player->mo->oldangle = player->mo->angle;
    player->oldviewz = player->viewz;
    player->oldlookdir = player->lookdir;
    player->oldrecoilpitch = player->recoilpitch;

    // [crispy] fast polling
    if (player == &players[consoleplayer])
    {
        localview.oldticangle = localview.ticangle;
    }

    // [crispy] update weapon sound source coordinates
    if (player->so != player->mo)
    {
	memcpy(player->so, player->mo, sizeof(degenmobj_t));
    }

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
	player->mo->flags &= ~MF_JUSTATTACKED;
    }
			
	
    // [crispy] center view
    // e.g. after teleporting, dying, jumping and on demand
    if (player->centering)
    {
        if (player->lookdir > 0)
        {
            player->lookdir -= 8 * MLOOKUNIT;
        }
        else if (player->lookdir < 0)
        {
            player->lookdir += 8 * MLOOKUNIT;
        }
        if (abs(player->lookdir) < 8 * MLOOKUNIT)
        {
            player->lookdir = 0;
            player->centering = false;
        }
    }

    // [crispy] weapon recoil pitch
    if (player->recoilpitch)
    {
        if (player->recoilpitch > 0)
        {
            player->recoilpitch -= 1;
        }
        else if (player->recoilpitch < 0)
        {
            player->recoilpitch += 1;
        }
    }

    if (player->playerstate == PST_DEAD)
    {
	P_DeathThink (player);
	return;
    }

    // [crispy] negative player health
    player->neghealth = player->health;

    // [crispy] delay next possible jump
    if (player->jumpTics)
    {
        player->jumpTics--;
    }
    
    // Move around.
    // Reactiontime is used to prevent movement
    //  for a bit after a teleport.
    if (player->mo->reactiontime)
	player->mo->reactiontime--;
    else
	P_MovePlayer (player);
    
    P_CalcHeight (player);

    if (player->mo->subsector->sector->special)
	P_PlayerInSpecialSector (player);
    
    // [crispy] jumping: apply vertical momentum
    if (cmd->arti)
    {
        if ((cmd->arti & AFLAG_JUMP) && onground &&
            player->viewz < player->mo->ceilingz-16*FRACUNIT &&
            !player->jumpTics)
        {
            // [crispy] Hexen sets 9; Strife adds 8
            player->mo->momz = (7 + critical->jump) * FRACUNIT;
            player->jumpTics = 18;
            // [NS] Jump sound.
            S_StartSoundOptional(player->mo, sfx_pljump, -1);
        }
    }

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
	
	if (newweapon == wp_fist
	    && player->weaponowned[wp_chainsaw]
	    && !(player->readyweapon == wp_chainsaw
		 && player->powers[pw_strength]))
	{
	    newweapon = wp_chainsaw;
	}
	
	if ( (crispy->havessg)
	    && newweapon == wp_shotgun 
	    && player->weaponowned[wp_supershotgun]
	    && player->readyweapon != wp_supershotgun)
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
	    // [crispy] "use" button timer
	    if (crispy->btusetimer)
	    {
		player->btuse = leveltime;
		player->btuse_tics = 5*TICRATE/2; // [crispy] 2.5 seconds
	    }
	}
    }
    else
	player->usedown = false;
    
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
	    player->fixedcolormap = a11y_invul_colormap ? INVERSECOLORMAP : 1; // [crispy] A11Y
	else
	    // [crispy] Visor effect when Invulnerability is fading out
	    player->fixedcolormap = (player->powers[pw_infrared] && a11y_invul_colormap) ? 1 : 0; // [crispy] A11Y
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


