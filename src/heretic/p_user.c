//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 1993-2008 Raven Software
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

// P_user.c

#include <stdlib.h>

#include "doomdef.h"
#include "deh_str.h"
#include "m_random.h"
#include "p_local.h"
#include "s_sound.h"

void P_PlayerNextArtifact(player_t * player);

// Macros

#define MAXBOB 0x100000         // 16 pixels of bob

// Data

boolean onground;
int newtorch;                   // used in the torch flicker effect.
int newtorchdelta;

boolean WeaponInShareware[] = {
    true,                       // Staff
    true,                       // Gold wand
    true,                       // Crossbow
    true,                       // Blaster
    false,                      // Skull rod
    false,                      // Phoenix rod
    false,                      // Mace
    true,                       // Gauntlets
    true                        // Beak
};

/*
==================
=
= P_Thrust
=
= moves the given origin along a given angle
=
==================
*/

void P_Thrust(player_t * player, angle_t angle, fixed_t move)
{
    angle >>= ANGLETOFINESHIFT;
    if (player->powers[pw_flight] && !(player->mo->z <= player->mo->floorz))
    {
        player->mo->momx += FixedMul(move, finecosine[angle]);
        player->mo->momy += FixedMul(move, finesine[angle]);
    }
    else if (player->mo->subsector->sector->special == 15)      // Friction_Low
    {
        player->mo->momx += FixedMul(move >> 2, finecosine[angle]);
        player->mo->momy += FixedMul(move >> 2, finesine[angle]);
    }
    else
    {
        player->mo->momx += FixedMul(move, finecosine[angle]);
        player->mo->momy += FixedMul(move, finesine[angle]);
    }
}


/*
==================
=
= P_CalcHeight
=
=Calculate the walking / running height adjustment
=
==================
*/

void P_CalcHeight(player_t * player)
{
    int angle;
    fixed_t bob;

//
// regular movement bobbing (needs to be calculated for gun swing even
// if not on ground)
// OPTIMIZE: tablify angle

    player->bob = FixedMul(player->mo->momx, player->mo->momx) +
        FixedMul(player->mo->momy, player->mo->momy);
    player->bob >>= 2;
    if (player->bob > MAXBOB)
        player->bob = MAXBOB;
    if (player->mo->flags2 & MF2_FLY && !onground)
    {
        player->bob = FRACUNIT / 2;
    }

    if ((player->cheats & CF_NOMOMENTUM))
    {
        player->viewz = player->mo->z + VIEWHEIGHT;
        if (player->viewz > player->mo->ceilingz - 4 * FRACUNIT)
            player->viewz = player->mo->ceilingz - 4 * FRACUNIT;
        player->viewz = player->mo->z + player->viewheight;
        return;
    }

    angle = (FINEANGLES / 20 * leveltime) & FINEMASK;
    bob = FixedMul(player->bob / 2, finesine[angle]);

//
// move viewheight
//
    if (player->playerstate == PST_LIVE)
    {
        player->viewheight += player->deltaviewheight;
        if (player->viewheight > VIEWHEIGHT)
        {
            player->viewheight = VIEWHEIGHT;
            player->deltaviewheight = 0;
        }
        if (player->viewheight < VIEWHEIGHT / 2)
        {
            player->viewheight = VIEWHEIGHT / 2;
            if (player->deltaviewheight <= 0)
                player->deltaviewheight = 1;
        }

        if (player->deltaviewheight)
        {
            player->deltaviewheight += FRACUNIT / 4;
            if (!player->deltaviewheight)
                player->deltaviewheight = 1;
        }
    }

    if (player->chickenTics)
    {
        player->viewz = player->mo->z + player->viewheight - (20 * FRACUNIT);
    }
    else
    {
        player->viewz = player->mo->z + player->viewheight + bob;
    }
    if (player->mo->flags2 & MF2_FEETARECLIPPED
        && player->playerstate != PST_DEAD
        && player->mo->z <= player->mo->floorz)
    {
        player->viewz -= FOOTCLIPSIZE;
    }
    if (player->viewz > player->mo->ceilingz - 4 * FRACUNIT)
    {
        player->viewz = player->mo->ceilingz - 4 * FRACUNIT;
    }
    if (player->viewz < player->mo->floorz + 4 * FRACUNIT)
    {
        player->viewz = player->mo->floorz + 4 * FRACUNIT;
    }
}

/*
=================
=
= P_MovePlayer
=
=================
*/

void P_MovePlayer(player_t * player)
{
    int look;
    int fly;
    ticcmd_t *cmd;

    cmd = &player->cmd;
    player->mo->angle += (cmd->angleturn << 16);

    onground = (player->mo->z <= player->mo->floorz
                || (player->mo->flags2 & MF2_ONMOBJ));

    if (player->chickenTics)
    {                           // Chicken speed
        if (cmd->forwardmove && (onground || player->mo->flags2 & MF2_FLY))
            P_Thrust(player, player->mo->angle, cmd->forwardmove * 2500);
        if (cmd->sidemove && (onground || player->mo->flags2 & MF2_FLY))
            P_Thrust(player, player->mo->angle - ANG90, cmd->sidemove * 2500);
    }
    else
    {                           // Normal speed
        if (cmd->forwardmove && (onground || player->mo->flags2 & MF2_FLY))
            P_Thrust(player, player->mo->angle, cmd->forwardmove * 2048);
        if (cmd->sidemove && (onground || player->mo->flags2 & MF2_FLY))
            P_Thrust(player, player->mo->angle - ANG90, cmd->sidemove * 2048);
    }

    if (cmd->forwardmove || cmd->sidemove)
    {
        if (player->chickenTics)
        {
            if (player->mo->state == &states[S_CHICPLAY])
            {
                P_SetMobjState(player->mo, S_CHICPLAY_RUN1);
            }
        }
        else
        {
            if (player->mo->state == &states[S_PLAY])
            {
                P_SetMobjState(player->mo, S_PLAY_RUN1);
            }
        }
    }

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
            player->lookdir += 5 * look;
            if (player->lookdir > 90 || player->lookdir < -110)
            {
                player->lookdir -= 5 * look;
            }
        }
    }
    if (player->centering)
    {
        if (player->lookdir > 0)
        {
            player->lookdir -= 8;
        }
        else if (player->lookdir < 0)
        {
            player->lookdir += 8;
        }
        if (abs(player->lookdir) < 8)
        {
            player->lookdir = 0;
            player->centering = false;
        }
    }
    fly = cmd->lookfly >> 4;
    if (fly > 7)
    {
        fly -= 16;
    }
    if (fly && player->powers[pw_flight])
    {
        if (fly != TOCENTER)
        {
            player->flyheight = fly * 2;
            if (!(player->mo->flags2 & MF2_FLY))
            {
                player->mo->flags2 |= MF2_FLY;
                player->mo->flags |= MF_NOGRAVITY;
            }
        }
        else
        {
            player->mo->flags2 &= ~MF2_FLY;
            player->mo->flags &= ~MF_NOGRAVITY;
        }
    }
    else if (fly > 0)
    {
        P_PlayerUseArtifact(player, arti_fly);
    }
    if (player->mo->flags2 & MF2_FLY)
    {
        player->mo->momz = player->flyheight * FRACUNIT;
        if (player->flyheight)
        {
            player->flyheight /= 2;
        }
    }
}

/*
=================
=
= P_DeathThink
=
=================
*/

#define         ANG5    (ANG90/18)
extern int inv_ptr;
extern int curpos;

void P_DeathThink(player_t * player)
{
    angle_t angle, delta;
    int lookDelta;

    P_MovePsprites(player);

    onground = (player->mo->z <= player->mo->floorz);
    if (player->mo->type == MT_BLOODYSKULL)
    {                           // Flying bloody skull
        player->viewheight = 6 * FRACUNIT;
        player->deltaviewheight = 0;
        //player->damagecount = 20;
        if (onground)
        {
            if (player->lookdir < 60)
            {
                lookDelta = (60 - player->lookdir) / 8;
                if (lookDelta < 1 && (leveltime & 1))
                {
                    lookDelta = 1;
                }
                else if (lookDelta > 6)
                {
                    lookDelta = 6;
                }
                player->lookdir += lookDelta;
            }
        }
    }
    else
    {                           // Fall to ground
        player->deltaviewheight = 0;
        if (player->viewheight > 6 * FRACUNIT)
            player->viewheight -= FRACUNIT;
        if (player->viewheight < 6 * FRACUNIT)
            player->viewheight = 6 * FRACUNIT;
        if (player->lookdir > 0)
        {
            player->lookdir -= 6;
        }
        else if (player->lookdir < 0)
        {
            player->lookdir += 6;
        }
        if (abs(player->lookdir) < 6)
        {
            player->lookdir = 0;
        }
    }
    P_CalcHeight(player);

    if (player->attacker && player->attacker != player->mo)
    {
        angle = R_PointToAngle2(player->mo->x, player->mo->y,
                                player->attacker->x, player->attacker->y);
        delta = angle - player->mo->angle;
        if (delta < ANG5 || delta > (unsigned) -ANG5)
        {                       // Looking at killer, so fade damage flash down
            player->mo->angle = angle;
            if (player->damagecount)
            {
                player->damagecount--;
            }
        }
        else if (delta < ANG180)
            player->mo->angle += ANG5;
        else
            player->mo->angle -= ANG5;
    }
    else if (player->damagecount)
    {
        player->damagecount--;
    }

    if (player->cmd.buttons & BT_USE)
    {
        if (player == &players[consoleplayer])
        {
            I_SetPalette(W_CacheLumpName(DEH_String("PLAYPAL"), PU_CACHE));
            inv_ptr = 0;
            curpos = 0;
            newtorch = 0;
            newtorchdelta = 0;
        }
        player->playerstate = PST_REBORN;
        // Let the mobj know the player has entered the reborn state.  Some
        // mobjs need to know when it's ok to remove themselves.
        player->mo->special2.i = 666;
    }
}

//----------------------------------------------------------------------------
//
// PROC P_ChickenPlayerThink
//
//----------------------------------------------------------------------------

void P_ChickenPlayerThink(player_t * player)
{
    mobj_t *pmo;

    if (player->health > 0)
    {                           // Handle beak movement
        P_UpdateBeak(player, &player->psprites[ps_weapon]);
    }
    if (player->chickenTics & 15)
    {
        return;
    }
    pmo = player->mo;
    if (!(pmo->momx + pmo->momy) && P_Random() < 160)
    {                           // Twitch view angle
        pmo->angle += P_SubRandom() << 19;
    }
    if ((pmo->z <= pmo->floorz) && (P_Random() < 32))
    {                           // Jump and noise
        pmo->momz += FRACUNIT;
        P_SetMobjState(pmo, S_CHICPLAY_PAIN);
        return;
    }
    if (P_Random() < 48)
    {                           // Just noise
        S_StartSound(pmo, sfx_chicact);
    }
}

//----------------------------------------------------------------------------
//
// FUNC P_GetPlayerNum
//
//----------------------------------------------------------------------------

int P_GetPlayerNum(player_t * player)
{
    int i;

    for (i = 0; i < MAXPLAYERS; i++)
    {
        if (player == &players[i])
        {
            return (i);
        }
    }
    return (0);
}

//----------------------------------------------------------------------------
//
// FUNC P_UndoPlayerChicken
//
//----------------------------------------------------------------------------

boolean P_UndoPlayerChicken(player_t * player)
{
    mobj_t *fog;
    mobj_t *mo;
    mobj_t *pmo;
    fixed_t x;
    fixed_t y;
    fixed_t z;
    angle_t angle;
    int playerNum;
    weapontype_t weapon;
    int oldFlags;
    int oldFlags2;

    pmo = player->mo;
    x = pmo->x;
    y = pmo->y;
    z = pmo->z;
    angle = pmo->angle;
    weapon = pmo->special1.i;
    oldFlags = pmo->flags;
    oldFlags2 = pmo->flags2;
    P_SetMobjState(pmo, S_FREETARGMOBJ);
    mo = P_SpawnMobj(x, y, z, MT_PLAYER);
    if (P_TestMobjLocation(mo) == false)
    {                           // Didn't fit
        P_RemoveMobj(mo);
        mo = P_SpawnMobj(x, y, z, MT_CHICPLAYER);
        mo->angle = angle;
        mo->health = player->health;
        mo->special1.i = weapon;
        mo->player = player;
        mo->flags = oldFlags;
        mo->flags2 = oldFlags2;
        player->mo = mo;
        player->chickenTics = 2 * 35;
        return (false);
    }
    playerNum = P_GetPlayerNum(player);
    if (playerNum != 0)
    {                           // Set color translation
        mo->flags |= playerNum << MF_TRANSSHIFT;
    }
    mo->angle = angle;
    mo->player = player;
    mo->reactiontime = 18;
    if (oldFlags2 & MF2_FLY)
    {
        mo->flags2 |= MF2_FLY;
        mo->flags |= MF_NOGRAVITY;
    }
    player->chickenTics = 0;
    player->powers[pw_weaponlevel2] = 0;
    player->health = mo->health = MAXHEALTH;
    player->mo = mo;
    angle >>= ANGLETOFINESHIFT;
    fog = P_SpawnMobj(x + 20 * finecosine[angle],
                      y + 20 * finesine[angle], z + TELEFOGHEIGHT, MT_TFOG);
    S_StartSound(fog, sfx_telept);
    P_PostChickenWeapon(player, weapon);
    return (true);
}

//----------------------------------------------------------------------------
//
// PROC P_PlayerThink
//
//----------------------------------------------------------------------------

void P_PlayerThink(player_t * player)
{
    ticcmd_t *cmd;
    weapontype_t newweapon;

    // No-clip cheat
    if (player->cheats & CF_NOCLIP)
    {
        player->mo->flags |= MF_NOCLIP;
    }
    else
    {
        player->mo->flags &= ~MF_NOCLIP;
    }
    cmd = &player->cmd;
    if (player->mo->flags & MF_JUSTATTACKED)
    {                           // Gauntlets attack auto forward motion
        cmd->angleturn = 0;
        cmd->forwardmove = 0xc800 / 512;
        cmd->sidemove = 0;
        player->mo->flags &= ~MF_JUSTATTACKED;
    }
// messageTics is above the rest of the counters so that messages will
//              go away, even in death.
    player->messageTics--;      // Can go negative
    if (!player->messageTics)
    {                           // Refresh the screen when a message goes away
        ultimatemsg = false;    // clear out any chat messages.
        BorderTopRefresh = true;
    }
    if (player->playerstate == PST_DEAD)
    {
        P_DeathThink(player);
        return;
    }
    if (player->chickenTics)
    {
        P_ChickenPlayerThink(player);
    }
    // Handle movement
    if (player->mo->reactiontime)
    {                           // Player is frozen
        player->mo->reactiontime--;
    }
    else
    {
        P_MovePlayer(player);
    }
    P_CalcHeight(player);
    if (player->mo->subsector->sector->special)
    {
        P_PlayerInSpecialSector(player);
    }
    if (cmd->arti)
    {                           // Use an artifact
        if (cmd->arti == 0xff)
        {
            P_PlayerNextArtifact(player);
        }
        else
        {
            P_PlayerUseArtifact(player, cmd->arti);
        }
    }
    // Check for weapon change
    if (cmd->buttons & BT_SPECIAL)
    {                           // A special event has no other buttons
        cmd->buttons = 0;
    }
    if (cmd->buttons & BT_CHANGE)
    {
        // The actual changing of the weapon is done when the weapon
        // psprite can do it (A_WeaponReady), so it doesn't happen in
        // the middle of an attack.
        newweapon = (cmd->buttons & BT_WEAPONMASK) >> BT_WEAPONSHIFT;
        if (newweapon == wp_staff && player->weaponowned[wp_gauntlets]
            && !(player->readyweapon == wp_gauntlets))
        {
            newweapon = wp_gauntlets;
        }
        if (player->weaponowned[newweapon]
            && newweapon != player->readyweapon)
        {
            if (WeaponInShareware[newweapon] || gamemode != shareware)
            {
                player->pendingweapon = newweapon;
            }
        }
    }
    // Check for use
    if (cmd->buttons & BT_USE)
    {
        if (!player->usedown)
        {
            P_UseLines(player);
            player->usedown = true;
        }
    }
    else
    {
        player->usedown = false;
    }
    // Chicken counter
    if (player->chickenTics)
    {
        if (player->chickenPeck)
        {                       // Chicken attack counter
            player->chickenPeck -= 3;
        }
        if (!--player->chickenTics)
        {                       // Attempt to undo the chicken
            P_UndoPlayerChicken(player);
        }
    }
    // Cycle psprites
    P_MovePsprites(player);
    // Other Counters
    if (player->powers[pw_invulnerability])
    {
        player->powers[pw_invulnerability]--;
    }
    if (player->powers[pw_invisibility])
    {
        if (!--player->powers[pw_invisibility])
        {
            player->mo->flags &= ~MF_SHADOW;
        }
    }
    if (player->powers[pw_infrared])
    {
        player->powers[pw_infrared]--;
    }
    if (player->powers[pw_flight])
    {
        if (!--player->powers[pw_flight])
        {
            // haleyjd: removed externdriver crap
            if (player->mo->z != player->mo->floorz)
            {
                player->centering = true;
            }

            player->mo->flags2 &= ~MF2_FLY;
            player->mo->flags &= ~MF_NOGRAVITY;
            BorderTopRefresh = true;    //make sure the sprite's cleared out
        }
    }
    if (player->powers[pw_weaponlevel2])
    {
        if (!--player->powers[pw_weaponlevel2])
        {
            if ((player->readyweapon == wp_phoenixrod)
                && (player->psprites[ps_weapon].state
                    != &states[S_PHOENIXREADY])
                && (player->psprites[ps_weapon].state
                    != &states[S_PHOENIXUP]))
            {
                P_SetPsprite(player, ps_weapon, S_PHOENIXREADY);
                player->ammo[am_phoenixrod] -= USE_PHRD_AMMO_2;
                player->refire = 0;
            }
            else if ((player->readyweapon == wp_gauntlets)
                     || (player->readyweapon == wp_staff))
            {
                player->pendingweapon = player->readyweapon;
            }
            BorderTopRefresh = true;
        }
    }
    if (player->damagecount)
    {
        player->damagecount--;
    }
    if (player->bonuscount)
    {
        player->bonuscount--;
    }
    // Colormaps
    if (player->powers[pw_invulnerability])
    {
        if (player->powers[pw_invulnerability] > BLINKTHRESHOLD
            || (player->powers[pw_invulnerability] & 8))
        {
            player->fixedcolormap = INVERSECOLORMAP;
        }
        else
        {
            player->fixedcolormap = 0;
        }
    }
    else if (player->powers[pw_infrared])
    {
        if (player->powers[pw_infrared] <= BLINKTHRESHOLD)
        {
            if (player->powers[pw_infrared] & 8)
            {
                player->fixedcolormap = 0;
            }
            else
            {
                player->fixedcolormap = 1;
            }
        }
        else if (!(leveltime & 16) && player == &players[consoleplayer])
        {
            if (newtorch)
            {
                if (player->fixedcolormap + newtorchdelta > 7
                    || player->fixedcolormap + newtorchdelta < 1
                    || newtorch == player->fixedcolormap)
                {
                    newtorch = 0;
                }
                else
                {
                    player->fixedcolormap += newtorchdelta;
                }
            }
            else
            {
                newtorch = (M_Random() & 7) + 1;
                newtorchdelta = (newtorch == player->fixedcolormap) ?
                    0 : ((newtorch > player->fixedcolormap) ? 1 : -1);
            }
        }
    }
    else
    {
        player->fixedcolormap = 0;
    }
}

//----------------------------------------------------------------------------
//
// PROC P_ArtiTele
//
//----------------------------------------------------------------------------

void P_ArtiTele(player_t * player)
{
    int i;
    int selections;
    fixed_t destX;
    fixed_t destY;
    angle_t destAngle;

    if (deathmatch)
    {
        selections = deathmatch_p - deathmatchstarts;
        i = P_Random() % selections;
        destX = deathmatchstarts[i].x << FRACBITS;
        destY = deathmatchstarts[i].y << FRACBITS;
        destAngle = ANG45 * (deathmatchstarts[i].angle / 45);
    }
    else
    {
        destX = playerstarts[0].x << FRACBITS;
        destY = playerstarts[0].y << FRACBITS;
        destAngle = ANG45 * (playerstarts[0].angle / 45);
    }
    P_Teleport(player->mo, destX, destY, destAngle);
    S_StartSound(NULL, sfx_wpnup);      // Full volume laugh
}

//----------------------------------------------------------------------------
//
// PROC P_PlayerNextArtifact
//
//----------------------------------------------------------------------------

void P_PlayerNextArtifact(player_t * player)
{
    if (player == &players[consoleplayer])
    {
        inv_ptr--;
        if (inv_ptr < 6)
        {
            curpos--;
            if (curpos < 0)
            {
                curpos = 0;
            }
        }
        if (inv_ptr < 0)
        {
            inv_ptr = player->inventorySlotNum - 1;
            if (inv_ptr < 6)
            {
                curpos = inv_ptr;
            }
            else
            {
                curpos = 6;
            }
        }
        player->readyArtifact = player->inventory[inv_ptr].type;
    }
}

//----------------------------------------------------------------------------
//
// PROC P_PlayerRemoveArtifact
//
//----------------------------------------------------------------------------

void P_PlayerRemoveArtifact(player_t * player, int slot)
{
    int i;
    player->artifactCount--;
    if (!(--player->inventory[slot].count))
    {                           // Used last of a type - compact the artifact list
        player->readyArtifact = arti_none;
        player->inventory[slot].type = arti_none;
        for (i = slot + 1; i < player->inventorySlotNum; i++)
        {
            player->inventory[i - 1] = player->inventory[i];
        }
        player->inventorySlotNum--;
        if (player == &players[consoleplayer])
        {                       // Set position markers and get next readyArtifact
            inv_ptr--;
            if (inv_ptr < 6)
            {
                curpos--;
                if (curpos < 0)
                {
                    curpos = 0;
                }
            }
            if (inv_ptr >= player->inventorySlotNum)
            {
                inv_ptr = player->inventorySlotNum - 1;
            }
            if (inv_ptr < 0)
            {
                inv_ptr = 0;
            }
            player->readyArtifact = player->inventory[inv_ptr].type;
        }
    }
}

//----------------------------------------------------------------------------
//
// PROC P_PlayerUseArtifact
//
//----------------------------------------------------------------------------

void P_PlayerUseArtifact(player_t * player, artitype_t arti)
{
    int i;

    for (i = 0; i < player->inventorySlotNum; i++)
    {
        if (player->inventory[i].type == arti)
        {                       // Found match - try to use
            if (P_UseArtifact(player, arti))
            {                   // Artifact was used - remove it from inventory
                P_PlayerRemoveArtifact(player, i);
                if (player == &players[consoleplayer])
                {
                    S_StartSound(NULL, sfx_artiuse);
                    ArtifactFlash = 4;
                }
            }
            else
            {                   // Unable to use artifact, advance pointer
                P_PlayerNextArtifact(player);
            }
            break;
        }
    }
}

//----------------------------------------------------------------------------
//
// FUNC P_UseArtifact
//
// Returns true if artifact was used.
//
//----------------------------------------------------------------------------

boolean P_UseArtifact(player_t * player, artitype_t arti)
{
    mobj_t *mo;
    angle_t angle;

    switch (arti)
    {
        case arti_invulnerability:
            if (!P_GivePower(player, pw_invulnerability))
            {
                return (false);
            }
            break;
        case arti_invisibility:
            if (!P_GivePower(player, pw_invisibility))
            {
                return (false);
            }
            break;
        case arti_health:
            if (!P_GiveBody(player, 25))
            {
                return (false);
            }
            break;
        case arti_superhealth:
            if (!P_GiveBody(player, 100))
            {
                return (false);
            }
            break;
        case arti_tomeofpower:
            if (player->chickenTics)
            {                   // Attempt to undo chicken
                if (P_UndoPlayerChicken(player) == false)
                {               // Failed
                    P_DamageMobj(player->mo, NULL, NULL, 10000);
                }
                else
                {               // Succeeded
                    player->chickenTics = 0;
                    S_StartSound(player->mo, sfx_wpnup);
                }
            }
            else
            {
                if (!P_GivePower(player, pw_weaponlevel2))
                {
                    return (false);
                }
                if (player->readyweapon == wp_staff)
                {
                    P_SetPsprite(player, ps_weapon, S_STAFFREADY2_1);
                }
                else if (player->readyweapon == wp_gauntlets)
                {
                    P_SetPsprite(player, ps_weapon, S_GAUNTLETREADY2_1);
                }
            }
            break;
        case arti_torch:
            if (!P_GivePower(player, pw_infrared))
            {
                return (false);
            }
            break;
        case arti_firebomb:
            angle = player->mo->angle >> ANGLETOFINESHIFT;

            // Vanilla bug here:
            // Original code here looks like:
            //   (player->mo->flags2 & MF2_FEETARECLIPPED != 0),
            // Which under C's operator precedence is:
            //   (player->mo->flags2 & (MF2_FEETARECLIPPED != 0)),
            // Which simplifies to:
            //   (player->mo->flags2 & 1),
            mo = P_SpawnMobj(player->mo->x + 24 * finecosine[angle],
                             player->mo->y + 24 * finesine[angle],
                             player->mo->z -
                             15 * FRACUNIT * (player->mo->flags2 & 1),
                             MT_FIREBOMB);
            mo->target = player->mo;
            break;
        case arti_egg:
            mo = player->mo;
            P_SpawnPlayerMissile(mo, MT_EGGFX);
            P_SPMAngle(mo, MT_EGGFX, mo->angle - (ANG45 / 6));
            P_SPMAngle(mo, MT_EGGFX, mo->angle + (ANG45 / 6));
            P_SPMAngle(mo, MT_EGGFX, mo->angle - (ANG45 / 3));
            P_SPMAngle(mo, MT_EGGFX, mo->angle + (ANG45 / 3));
            break;
        case arti_fly:
            if (!P_GivePower(player, pw_flight))
            {
                return (false);
            }
            break;
        case arti_teleport:
            P_ArtiTele(player);
            break;
        default:
            return (false);
    }
    return (true);
}
