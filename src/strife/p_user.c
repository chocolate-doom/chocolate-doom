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

#include <stdlib.h>

#include "doomdef.h"
#include "d_event.h"
#include "p_local.h"
#include "sounds.h"     // villsa [STRIFE]
#include "p_dialog.h"   // villsa [STRIFE]
#include "d_main.h"     // villsa [STRIFE]
#include "doomstat.h"
#include "deh_str.h"    // haleyjd [STRIFE]
#include "z_zone.h"
#include "w_wad.h"
#include "p_pspr.h"
#include "m_misc.h"
#include "m_random.h"
#include "s_sound.h"
#include "p_inter.h"



// Index of the special effects (INVUL inverse) map.
#define LOOKPITCHAMOUNT         6                       // villsa [STRIFE]
#define CENTERVIEWAMOUNT        (LOOKPITCHAMOUNT + 2)   // villsa [STRIFE]
#define LOOKUPMAX               90                      // villsa [STRIFE]
#define LOOKDOWNMAX             -110                    // villsa [STRIFE]


void P_DropInventoryItem(player_t* player, int sprite); // villsa [STRIFE]
boolean P_ItemBehavior(player_t* player, int item);     // villsa [STRIFE]
static char useinventorymsg[44];    // villsa [STRIFE]

//
// Movement.
//

// 16 pixels of bob
#define MAXBOB	0x100000	

boolean		onground;


//
// P_Thrust
// Moves the given origin along a given angle.
//
// [STRIFE] Verified unmodified
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
// [STRIFE] Some odd adjustments, and terrain view height adjustment
//
void P_CalcHeight (player_t* player) 
{
    int     angle;
    fixed_t bob;
    
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

    // haleyjd 20110205 [STRIFE]: No CF_NOMOMENTUM check, and Rogue also removed
    // the dead code inside.
    if (!onground)
    {
        /*
        player->viewz = player->mo->z + VIEWHEIGHT;

        if (player->viewz > player->mo->ceilingz-4*FRACUNIT)
            player->viewz = player->mo->ceilingz-4*FRACUNIT;
        */

        player->viewz = player->mo->z + player->viewheight;
        return;
    }

    angle = (FINEANGLES/20*leveltime)&FINEMASK;
    bob = FixedMul ( player->bob/2, finesine[angle]);

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

    // villsa [STRIFE] account for terrain lowering the view
    if(player->mo->flags & MF_FEETCLIPPED)
        player->viewz -= 13*FRACUNIT;

    if (player->viewz > player->mo->ceilingz-4*FRACUNIT)
        player->viewz = player->mo->ceilingz-4*FRACUNIT;

    // haleyjd [STRIFE]: added a floorz clip here
    if (player->viewz < player->mo->floorz)
        player->viewz = player->mo->floorz;
}



//
// P_MovePlayer
//
// [STRIFE] Adjustments to allow air control, jumping, and up/down look.
//
void P_MovePlayer (player_t* player)
{
    ticcmd_t*      cmd;

    cmd = &player->cmd;

    player->mo->angle += (cmd->angleturn<<16);

    // Do not let the player control movement
    //  if not onground.
    onground = (player->mo->z <= player->mo->floorz);

    // villsa [STRIFE] allows player to climb over things by jumping
    // haleyjd 20110205: air control thrust should be 256, not cmd->forwardmove
    if(!onground)
    {
        if(cmd->forwardmove)
            P_Thrust (player, player->mo->angle, 256);
    }
    else
    {
        // villsa [STRIFE] jump button
        if (cmd->buttons2 & BT2_JUMP)
        {
            if(!player->deltaviewheight)
                player->mo->momz += 8*FRACUNIT;
        }

        // haleyjd 20110205 [STRIFE] Either Rogue or Watcom removed the
        // redundant "onground" checks from these if's.
        if (cmd->forwardmove)
            P_Thrust (player, player->mo->angle, cmd->forwardmove*2048);

        if (cmd->sidemove)
            P_Thrust (player, player->mo->angle-ANG90, cmd->sidemove*2048);
    }

    // villsa [STRIFE] player walking state set
    if ( (cmd->forwardmove || cmd->sidemove) 
        && player->mo->state == &states[S_PLAY_00] )
    {
        P_SetMobjState (player->mo, S_PLAY_01);
    }

    // villsa [STRIFE] centerview button
    if (cmd->buttons2 & BT2_CENTERVIEW)
        player->centerview = 1;

    // villsa [STRIFE] adjust player's pitch when centerviewing
    if (player->centerview)
    {
        if (player->pitch <= 0)
        {
            if (player->pitch < 0)
                player->pitch = player->pitch + CENTERVIEWAMOUNT;
        }
        else
        {
            player->pitch = player->pitch - CENTERVIEWAMOUNT;
        }
        if (abs(player->pitch) < CENTERVIEWAMOUNT)
        {
            player->pitch = 0;
            player->centerview = 0;
        }
    }
    
    // villsa [STRIFE] look up action
    if (cmd->buttons2 & BT2_LOOKUP)
    {
        player->pitch += LOOKPITCHAMOUNT;
        if (player->pitch > LOOKUPMAX ||
            player->pitch < LOOKDOWNMAX)
            player->pitch -= LOOKPITCHAMOUNT;
    }
    else
    {
        // villsa [STRIFE] look down action
        if (cmd->buttons2 & BT2_LOOKDOWN)
        {
            player->pitch -= LOOKPITCHAMOUNT;
            if (player->pitch > LOOKUPMAX ||
                player->pitch < LOOKDOWNMAX)
                player->pitch += LOOKPITCHAMOUNT;
        }
    }

}



//
// P_DeathThink
// Fall on your face when dying.
// Decrease POV height to floor height.
//
// [STRIFE] Modifications for up/down look.
//
#define ANG5    (ANG90/18)

void P_DeathThink(player_t* player)
{
    angle_t angle;
    angle_t delta;

    P_MovePsprites(player);

    // fall to the ground
    if (player->viewheight > 6*FRACUNIT)
        player->viewheight -= FRACUNIT;

    if (player->viewheight < 6*FRACUNIT)
        player->viewheight = 6*FRACUNIT;

    player->deltaviewheight = 0;
    onground = (player->mo->z <= player->mo->floorz);
    P_CalcHeight(player);

    if(player->attacker && player->attacker != player->mo)
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

    // villsa [STRIFE]
    if(player->pitch <= 90)
        player->pitch = player->pitch + 3;

    if(player->cmd.buttons & BT_USE)
        player->playerstate = PST_REBORN;
}



//
// P_PlayerThink
//
// [STRIFE] Massive changes/additions:
// * NOCLIP hack removed
// * P_DeathThink moved up
// * Inventory use/dropping
// * Strife weapons logic
// * Dialog
// * Strife powerups and nukage behavior
// * Fire Death/Sigil Shock
//
void P_PlayerThink (player_t* player)
{
    ticcmd_t*       cmd;
    weapontype_t    newweapon;

    // villsa [STRIFE] unused code (see ST_Responder)
    /*
    // fixme: do this in the cheat code
    if (player->cheats & CF_NOCLIP)
        player->mo->flags |= MF_NOCLIP;
    else
        player->mo->flags &= ~MF_NOCLIP;
    */

    // haleyjd 20110205 [STRIFE]: P_DeathThink moved up
    if (player->playerstate == PST_DEAD)
    {
        P_DeathThink (player);
        return;
    }

    // chain saw run forward
    cmd = &player->cmd;
    if (player->mo->flags & MF_JUSTATTACKED)
    {
        cmd->angleturn = 0;
        cmd->forwardmove = 0xc800/512;
        cmd->sidemove = 0;
        player->mo->flags &= ~MF_JUSTATTACKED;
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

    // villsa [STRIFE] handle inventory input
    if(cmd->buttons2 & (BT2_HEALTH|BT2_INVUSE|BT2_INVDROP))
    {
        if(!player->inventorydown)
        {
            if(cmd->buttons2 & BT2_HEALTH)
                P_UseInventoryItem(player, SPR_FULL);
            else if(cmd->buttons2 & BT2_INVUSE)
                P_UseInventoryItem(player, cmd->inventory);
            else if(cmd->buttons2 & BT2_INVDROP)
            {
                P_DropInventoryItem(player, cmd->inventory);

                // haleyjd 20110205: removed incorrect "else" here
                // villsa [STRIFE]
                if(workparm)
                {
                    int cheat = player->cheats ^ 1;
                    player->cheats ^= CF_NOCLIP;

                    if(cheat & CF_NOCLIP)
                    {
                        player->message = DEH_String("No Clipping Mode ON");
                        player->mo->flags |= MF_NOCLIP;
                    }
                    else
                    {
                        player->mo->flags &= ~MF_NOCLIP;
                        player->message = DEH_String("No Clipping Mode OFF");
                    }
                }
            }
        }

        player->inventorydown = true;
    }
    else
        player->inventorydown = false;

    // Check for weapon change.

    // A special event has no other buttons.
    if(cmd->buttons & BT_SPECIAL)
        cmd->buttons = 0;

    if(cmd->buttons & BT_CHANGE)
    {
        // The actual changing of the weapon is done
        //  when the weapon psprite can do it
        //  (read: not in the middle of an attack).
        newweapon = (cmd->buttons & BT_WEAPONMASK) >> BT_WEAPONSHIFT;

        // villsa [STRIFE] select poison bow
        if(newweapon == wp_elecbow)
        {
            if(player->weaponowned[wp_poisonbow] && player->readyweapon == wp_elecbow)
            {
                if(player->ammo[weaponinfo[wp_poisonbow].ammo])
                    newweapon = wp_poisonbow;
            }
        }

        // villsa [STRIFE] select wp grenade launcher
        if(newweapon == wp_hegrenade)
        {
            if(player->weaponowned[wp_wpgrenade] && player->readyweapon == wp_hegrenade)
            {
                if(player->ammo[weaponinfo[wp_wpgrenade].ammo])
                    newweapon = wp_wpgrenade;
            }
        }

        // villsa [STRIFE] select torpedo
        if(newweapon == wp_mauler)
        {
            if(player->weaponowned[wp_torpedo] && player->readyweapon == wp_mauler)
            {
                // haleyjd 20140924: bug fix - using wrong enum value am_cell
                // caused this to check the missile launcher for rocket ammo
                if(player->ammo[weaponinfo[wp_torpedo].ammo] >= 30)
                    newweapon = wp_torpedo;
            }
        }

        if(player->weaponowned[newweapon] && newweapon != player->readyweapon)
        {
            // villsa [STRIFE] check weapon if in demo mode or not
            if(weaponinfo[newweapon].availabledemo || !isdemoversion)
            {
                if(player->ammo[weaponinfo[newweapon].ammo])
                    player->pendingweapon = newweapon;
                else
                {
                    // decide between electric bow or poison arrow
                    if(newweapon == wp_elecbow &&
                        player->ammo[am_poisonbolts] &&
                        player->readyweapon != wp_poisonbow)
                    {
                        player->pendingweapon = wp_poisonbow;
                    }
                    // decide between hp grenade launcher or wp grenade launcher
                    else if(newweapon == wp_hegrenade &&
                        player->ammo[am_wpgrenades] &&
                        player->readyweapon != wp_wpgrenade)
                    {
                        player->pendingweapon = wp_wpgrenade;
                    }

                    // villsa [STRIFE] - no check for mauler/torpedo??
                }
            }
        }
    }

    // check for use
    if(cmd->buttons & BT_USE)
    {
        if(!player->usedown)
        {
            P_DialogStart(player);  // villsa [STRIFE]
            P_UseLines (player);
            player->usedown = true;
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

    // villsa [STRIFE] targeter powerup
    if(player->powers[pw_targeter])
    {
        player->powers[pw_targeter]--;
        if(player->powers[pw_targeter] == 1)
        {
            P_SetPsprite(player, ps_targcenter, S_NULL);
            P_SetPsprite(player, ps_targleft,   S_NULL);
            P_SetPsprite(player, ps_targright,  S_NULL);
        }
        else if(player->powers[pw_targeter] - 1 < 5*TICRATE)
        {
            if(player->powers[pw_targeter] & 32)
            {
                P_SetPsprite(player, ps_targright, S_NULL);
                P_SetPsprite(player, ps_targleft,  S_TRGT_01);   // 11
            }
            else if(player->powers[pw_targeter] & 16) // haleyjd 20110205: missing else
            {
                P_SetPsprite(player, ps_targright, S_TRGT_02);  // 12
                P_SetPsprite(player, ps_targleft,  S_NULL);
            }
        }
    }

    if(player->powers[pw_invisibility])
    {
        // villsa [STRIFE] remove mvis flag as well
        if(!--player->powers[pw_invisibility])
            player->mo->flags &= ~(MF_SHADOW|MF_MVIS);
    }

    if(player->powers[pw_ironfeet])
    {
        player->powers[pw_ironfeet]--;

        // villsa [STRIFE] gasmask sound
        if(!(leveltime & 0x3f))
            S_StartSound(player->mo, sfx_mask);
    }

    if(player->powers[pw_allmap] > 1)
        player->powers[pw_allmap]--;

    // haleyjd 08/30/10: [STRIFE]
    // Nukage count keeps track of exposure to hazardous conditions over time.
    // After accumulating 16 total seconds or more of exposure, you will take
    // 5 damage roughly once per second until the count drops back under 560
    // tics.
    if(player->nukagecount)
    {
        player->nukagecount--;
        if(!(leveltime & 0x1f) && player->nukagecount > 16*TICRATE)
            P_DamageMobj(player->mo, NULL, NULL, 5);
    }

    if(player->damagecount)
        player->damagecount--;

    if(player->bonuscount)
        player->bonuscount--;

    // villsa [STRIFE] checks for extralight
    if(player->extralight >= 0)
    {
        if(player->cheats & CF_ONFIRE)
            player->fixedcolormap = 1;
        else
            player->fixedcolormap = 0;
    }
    else // Sigil shock:
        player->fixedcolormap = INVERSECOLORMAP;
}


//
// P_RemoveInventoryItem
// villsa [STRIFE] new function
//
char* P_RemoveInventoryItem(player_t *player, int slot, int amount)
{
    mobjtype_t type;

    player->inventory[slot].amount -= amount;
    player->st_update = true;

    type = player->inventory[slot].type;

    if(!player->inventory[slot].amount)
    {
        // shift everything above it down
        // see P_TakeDialogItem for notes on possible bugs
        int j;

        for(j = slot + 1; j <= player->numinventory; j++)
        {
            inventory_t *item1 = &(player->inventory[j - 1]);
            inventory_t *item2 = &(player->inventory[j]);

            *item1 = *item2;
        }

        player->inventory[player->numinventory].type = NUMMOBJTYPES;
        player->inventory[player->numinventory].sprite = -1;
        player->numinventory--;

        // update cursor position
        if(player->inventorycursor >= player->numinventory)
        {
            if(player->inventorycursor)
                player->inventorycursor--;
        }
    }

    return mobjinfo[type].name;
}

//
// P_DropInventoryItem
// villsa [STRIFE] new function
//
void P_DropInventoryItem(player_t* player, int sprite)
{
    int invslot;
    inventory_t *item;
    mobjtype_t type;
    int amount;

    invslot = 0;
    amount = 1;

    while(invslot < player->numinventory && sprite != player->inventory[invslot].sprite)
        invslot++;

    item = &(player->inventory[invslot]);
    type = item->type;

    if(item->amount)
    {
        angle_t angle;
        fixed_t dist;
        mobj_t* mo;
        mobj_t* mobjitem;
        fixed_t x;
        fixed_t y;
        fixed_t z;
        int r;

        if(item->type == MT_MONY_1)
        {
            if(item->amount >= 50)
            {
                type = MT_MONY_50;
                amount = 50;
            }
            else if(item->amount >= 25)
            {
                type = MT_MONY_25;
                amount = 25;
            }
            else if(item->amount >= 10)
            {
                type = MT_MONY_10;
                amount = 10;
            }
        }

        if(type >= NUMMOBJTYPES)
            return;

        angle = player->mo->angle;
        r = P_Random();
        angle = (angle + ((r - P_Random()) << 18)) >> ANGLETOFINESHIFT;

        if(angle < 7618 && angle >= 6718)
            angle = 7618;

        else if(angle < 5570 && angle >= 4670)
            angle = 5570;

        else if(angle < 3522 && angle >= 2622)
            angle = 3522;

        else if(angle < 1474 && angle >= 574)
            angle = 1474;

        mo = player->mo;
        dist = mobjinfo[type].radius + mo->info->radius + (4*FRACUNIT);

        x = mo->x + FixedMul(finecosine[angle], dist);
        y = mo->y + FixedMul(finesine[angle], dist);
        z = mo->z + (10*FRACUNIT);
        mobjitem = P_SpawnMobj(x, y, z, type);
        mobjitem->flags |= (MF_SPECIAL|MF_DROPPED);

        if(P_CheckPosition(mobjitem, x, y))
        {
            mobjitem->angle = (angle << ANGLETOFINESHIFT);
            mobjitem->momx = FixedMul(finecosine[angle], (5*FRACUNIT)) + mo->momx;
            mobjitem->momy = FixedMul(finesine[angle], (5*FRACUNIT)) + mo->momy;
            mobjitem->momz = FRACUNIT;

            P_RemoveInventoryItem(player, invslot, amount);
        }
        else
            P_RemoveMobj(mobjitem);
    }
}

//
// P_TossDegninOre
// villsa [STRIFE] new function
//
boolean P_TossDegninOre(player_t* player)
{
    angle_t angle;
    mobj_t* mo;
    mobj_t* ore;
    fixed_t x;
    fixed_t y;
    fixed_t z;
    fixed_t dist;

    angle = player->mo->angle >> ANGLETOFINESHIFT;

    if(angle < 7618 && angle >= 6718)
        angle = 7618;

    else if(angle < 5570 && angle >= 4670)
        angle = 5570;

    else if(angle < 3522 && angle >= 2622)
        angle = 3522;

    else if(angle < 1474 && angle >= 574)
        angle = 1474;

    mo = player->mo;
    dist = mobjinfo[MT_DEGNINORE].radius + mo->info->radius + (4*FRACUNIT);

    x = mo->x + FixedMul(finecosine[angle], dist);
    y = mo->y + FixedMul(finesine[angle], dist);
    z = mo->z + (10*FRACUNIT);
    ore = P_SpawnMobj(x, y, z, MT_DEGNINORE);

    if(P_CheckPosition(ore, x, y))
    {
        ore->target = mo;
        ore->angle = (angle << ANGLETOFINESHIFT);
        ore->momx = FixedMul(finecosine[angle], (5*FRACUNIT));
        ore->momy = FixedMul(finesine[angle], (5*FRACUNIT));
        ore->momz = FRACUNIT;
        return true;
    }
    else
        P_RemoveMobj(ore);

    return false;
}

//
// P_SpawnTeleportBeacon
//
// villsa [STRIFE] new function
// haleyjd 20140918: bug fixed to propagate allegiance properly.
//
boolean P_SpawnTeleportBeacon(player_t* player)
{
    angle_t angle;
    int r;
    mobj_t* mo;
    mobj_t* beacon;
    fixed_t x;
    fixed_t y;
    fixed_t z;
    fixed_t dist;

    angle = player->mo->angle;
    r = P_Random();
    angle = (angle + ((r - P_Random()) << 18)) >> ANGLETOFINESHIFT;

    if(angle < 7618 && angle >= 6718)
        angle = 7618;

    else if(angle < 5570 && angle >= 4670)
        angle = 5570;

    else if(angle < 3522 && angle >= 2622)
        angle = 3522;

    else if(angle < 1474 && angle >= 574)
        angle = 1474;

    mo = player->mo;
    dist = mobjinfo[MT_BEACON].radius + mo->info->radius + (4*FRACUNIT);

    x = mo->x + FixedMul(finecosine[angle], dist);
    y = mo->y + FixedMul(finesine[angle], dist);
    z = mo->z + (10*FRACUNIT);
    beacon = P_SpawnMobj(x, y, z, MT_BEACON);

    if(P_CheckPosition(beacon, x, y))
    {
        beacon->target = mo;
        beacon->miscdata = (byte)(player->allegiance);
        beacon->angle = (angle << ANGLETOFINESHIFT);
        beacon->momx = FixedMul(finecosine[angle], (5*FRACUNIT));
        beacon->momy = FixedMul(finesine[angle], (5*FRACUNIT));
        beacon->momz = FRACUNIT;
        P_SetMobjState(beacon, beacon->info->seestate);
        return true;
    }
    else
        P_RemoveMobj(beacon);

    return false;
}

//
// P_UseInventoryItem
// villsa [STRIFE] new function
//
boolean P_UseInventoryItem(player_t* player, int item)
{
    int i;
    char* name;

    if(player->cheats & CF_ONFIRE)
        return false;

    for(i = 0; i < player->numinventory; i++)
    {
        if(item != player->inventory[i].sprite)
            continue;

        if(!P_ItemBehavior(player, item))
            return false;

        name = P_RemoveInventoryItem(player, i, 1);
        if(name == NULL)
            name = "Item";

        M_snprintf(useinventorymsg, sizeof(useinventorymsg),
                   "You used the %s.", name);
        player->message = useinventorymsg;

        if(player == &players[consoleplayer])
            S_StartSound(NULL, sfx_itemup);

        return true;
    }
    
    return false;
}

//
// P_ItemBehavior
// villsa [STRIFE] new function
//
boolean P_ItemBehavior(player_t* player, int item)
{
    switch(item)
    {
    case SPR_ARM1:  // 136
        return P_GiveArmor(player, 2);

    case SPR_ARM2:  // 137
        return P_GiveArmor(player, 1);

    case SPR_SHD1:  // 186
        return P_GivePower(player, pw_invisibility);

    case SPR_MASK:  // 187
        return P_GivePower(player, pw_ironfeet);

    case SPR_PMUP:  // 191
        if(!player->powers[pw_allmap])
        {
            player->message = "The scanner won't work without a map!";
            return false;
        }
        player->powers[pw_allmap] = PMUPTICS;
        return true; // haleyjd 20110228: repaired

    case SPR_STMP:  // 180
        return P_GiveBody(player, 10);

    case SPR_MDKT:  // 181
        return P_GiveBody(player, 25);

    case SPR_FULL:  // 130
        return P_GiveBody(player, 200);

    case SPR_BEAC:  // 135
        return P_SpawnTeleportBeacon(player);

    case SPR_TARG:  // 108
        return P_GivePower(player, pw_targeter);
    }

    return false;
}
