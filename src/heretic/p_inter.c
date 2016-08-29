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

// P_inter.c

#include "doomdef.h"
#include "deh_str.h"
#include "i_system.h"
#include "i_timer.h"
#include "m_random.h"
#include "p_local.h"
#include "s_sound.h"

#define BONUSADD 6

int WeaponValue[] = {
    1,                          // staff
    3,                          // goldwand
    4,                          // crossbow
    5,                          // blaster
    6,                          // skullrod
    7,                          // phoenixrod
    8,                          // mace
    2,                          // gauntlets
    0                           // beak
};

int maxammo[NUMAMMO] = {
    100,                        // gold wand
    50,                         // crossbow
    200,                        // blaster
    200,                        // skull rod
    20,                         // phoenix rod
    150                         // mace
};

int GetWeaponAmmo[NUMWEAPONS] = {
    0,                          // staff
    25,                         // gold wand
    10,                         // crossbow
    30,                         // blaster
    50,                         // skull rod
    2,                          // phoenix rod
    50,                         // mace
    0,                          // gauntlets
    0                           // beak
};

static weapontype_t GetAmmoChange[] = {
    wp_goldwand,
    wp_crossbow,
    wp_blaster,
    wp_skullrod,
    wp_phoenixrod,
    wp_mace
};

/*
static boolean GetAmmoChangePL1[NUMWEAPONS][NUMAMMO] =
{
	// staff
	{wp_goldwand, wp_crossbow, wp_blaster, wp_skullrod, -1, wp_mace},
	// gold wand
	{-1, wp_crossbow, wp_blaster, wp_skullrod, -1, wp_mace},
	// crossbow
	{-1, -1, wp_blaster, wp_skullrod, -1, -1},
	// blaster
	{-1, -1, -1, -1, -1, -1},
	// skull rod
	{-1, -1, -1, -1, -1, -1},
	// phoenix rod
	{-1, -1, -1, -1, -1, -1},
	// mace
	{-1, wp_crossbow, wp_blaster, wp_skullrod, -1, -1},
	// gauntlets
	{-1, wp_crossbow, wp_blaster, wp_skullrod, -1, wp_mace}
};
*/

/*
static boolean GetAmmoChangePL2[NUMWEAPONS][NUMAMMO] =
{
	// staff
	{wp_goldwand, wp_crossbow, wp_blaster, wp_skullrod, wp_phoenixrod,
		wp_mace},
	// gold wand
	{-1, wp_crossbow, wp_blaster, wp_skullrod, wp_phoenixrod, wp_mace},
	// crossbow
	{-1, -1, wp_blaster, wp_skullrod, wp_phoenixrod, -1},
	// blaster
	{-1, -1, -1, wp_skullrod, wp_phoenixrod, -1},
	// skull rod
	{-1, -1, -1, -1, -1, -1},
	// phoenix rod
	{-1, -1, -1, -1, -1, -1},
	// mace
	{-1, wp_crossbow, wp_blaster, wp_skullrod, -1, -1},
	// gauntlets
	{-1, -1, -1, wp_skullrod, wp_phoenixrod, wp_mace}
};
*/

//--------------------------------------------------------------------------
//
// PROC P_SetMessage
//
//--------------------------------------------------------------------------

boolean ultimatemsg;

void P_SetMessage(player_t * player, char *message, boolean ultmsg)
{
    extern boolean messageson;

    if ((ultimatemsg || !messageson) && !ultmsg)
    {
        return;
    }
    player->message = message;
    player->messageTics = MESSAGETICS;
    BorderTopRefresh = true;
    if (ultmsg)
    {
        ultimatemsg = true;
    }
}

//--------------------------------------------------------------------------
//
// FUNC P_GiveAmmo
//
// Returns true if the player accepted the ammo, false if it was
// refused (player has maxammo[ammo]).
//
//--------------------------------------------------------------------------

boolean P_GiveAmmo(player_t * player, ammotype_t ammo, int count)
{
    int prevAmmo;
    //weapontype_t changeWeapon;

    if (ammo == am_noammo)
    {
        return (false);
    }
    if ((unsigned int) ammo > NUMAMMO)
    {
        I_Error("P_GiveAmmo: bad type %i", ammo);
    }
    if (player->ammo[ammo] == player->maxammo[ammo])
    {
        return (false);
    }
    if (gameskill == sk_baby || gameskill == sk_nightmare)
    {                           // extra ammo in baby mode and nightmare mode
        count += count >> 1;
    }
    prevAmmo = player->ammo[ammo];

    player->ammo[ammo] += count;
    if (player->ammo[ammo] > player->maxammo[ammo])
    {
        player->ammo[ammo] = player->maxammo[ammo];
    }
    if (prevAmmo)
    {
        // Don't attempt to change weapons if the player already had
        // ammo of the type just given
        return (true);
    }
    if (player->readyweapon == wp_staff
        || player->readyweapon == wp_gauntlets)
    {
        if (player->weaponowned[GetAmmoChange[ammo]])
        {
            player->pendingweapon = GetAmmoChange[ammo];
        }
    }
/*
	if(player->powers[pw_weaponlevel2])
	{
		changeWeapon = GetAmmoChangePL2[player->readyweapon][ammo];
	}
	else
	{
		changeWeapon = GetAmmoChangePL1[player->readyweapon][ammo];
	}
	if(changeWeapon != -1)
	{
		if(player->weaponowned[changeWeapon])
		{
			player->pendingweapon = changeWeapon;
		}
	}
*/
    return (true);
}

//--------------------------------------------------------------------------
//
// FUNC P_GiveWeapon
//
// Returns true if the weapon or its ammo was accepted.
//
//--------------------------------------------------------------------------

boolean P_GiveWeapon(player_t * player, weapontype_t weapon)
{
    boolean gaveAmmo;
    boolean gaveWeapon;

    if (netgame && !deathmatch)
    {                           // Cooperative net-game
        if (player->weaponowned[weapon])
        {
            return (false);
        }
        player->bonuscount += BONUSADD;
        player->weaponowned[weapon] = true;
        P_GiveAmmo(player, wpnlev1info[weapon].ammo, GetWeaponAmmo[weapon]);
        player->pendingweapon = weapon;
        if (player == &players[consoleplayer])
        {
            S_StartSound(NULL, sfx_wpnup);
        }
        return (false);
    }
    gaveAmmo = P_GiveAmmo(player, wpnlev1info[weapon].ammo,
                          GetWeaponAmmo[weapon]);
    if (player->weaponowned[weapon])
    {
        gaveWeapon = false;
    }
    else
    {
        gaveWeapon = true;
        player->weaponowned[weapon] = true;
        if (WeaponValue[weapon] > WeaponValue[player->readyweapon])
        {                       // Only switch to more powerful weapons
            player->pendingweapon = weapon;
        }
    }
    return (gaveWeapon || gaveAmmo);
}

//---------------------------------------------------------------------------
//
// FUNC P_GiveBody
//
// Returns false if the body isn't needed at all.
//
//---------------------------------------------------------------------------

boolean P_GiveBody(player_t * player, int num)
{
    int max;

    max = MAXHEALTH;
    if (player->chickenTics)
    {
        max = MAXCHICKENHEALTH;
    }
    if (player->health >= max)
    {
        return (false);
    }
    player->health += num;
    if (player->health > max)
    {
        player->health = max;
    }
    player->mo->health = player->health;
    return (true);
}

//---------------------------------------------------------------------------
//
// FUNC P_GiveArmor
//
// Returns false if the armor is worse than the current armor.
//
//---------------------------------------------------------------------------

boolean P_GiveArmor(player_t * player, int armortype)
{
    int hits;

    hits = armortype * 100;
    if (player->armorpoints >= hits)
    {
        return (false);
    }
    player->armortype = armortype;
    player->armorpoints = hits;
    return (true);
}

//---------------------------------------------------------------------------
//
// PROC P_GiveKey
//
//---------------------------------------------------------------------------

void P_GiveKey(player_t * player, keytype_t key)
{
    extern int playerkeys;
    extern vertex_t KeyPoints[];

    if (player->keys[key])
    {
        return;
    }
    if (player == &players[consoleplayer])
    {
        playerkeys |= 1 << key;
        KeyPoints[key].x = 0;
        KeyPoints[key].y = 0;
    }
    player->bonuscount = BONUSADD;
    player->keys[key] = true;
}

//---------------------------------------------------------------------------
//
// FUNC P_GivePower
//
// Returns true if power accepted.
//
//---------------------------------------------------------------------------

boolean P_GivePower(player_t * player, powertype_t power)
{
    if (power == pw_invulnerability)
    {
        if (player->powers[power] > BLINKTHRESHOLD)
        {                       // Already have it
            return (false);
        }
        player->powers[power] = INVULNTICS;
        return (true);
    }
    if (power == pw_weaponlevel2)
    {
        if (player->powers[power] > BLINKTHRESHOLD)
        {                       // Already have it
            return (false);
        }
        player->powers[power] = WPNLEV2TICS;
        return (true);
    }
    if (power == pw_invisibility)
    {
        if (player->powers[power] > BLINKTHRESHOLD)
        {                       // Already have it
            return (false);
        }
        player->powers[power] = INVISTICS;
        player->mo->flags |= MF_SHADOW;
        return (true);
    }
    if (power == pw_flight)
    {
        if (player->powers[power] > BLINKTHRESHOLD)
        {                       // Already have it
            return (false);
        }
        player->powers[power] = FLIGHTTICS;
        player->mo->flags2 |= MF2_FLY;
        player->mo->flags |= MF_NOGRAVITY;
        if (player->mo->z <= player->mo->floorz)
        {
            player->flyheight = 10;     // thrust the player in the air a bit
        }
        return (true);
    }
    if (power == pw_infrared)
    {
        if (player->powers[power] > BLINKTHRESHOLD)
        {                       // Already have it
            return (false);
        }
        player->powers[power] = INFRATICS;
        return (true);
    }
/*
	if(power == pw_ironfeet)
	{
		player->powers[power] = IRONTICS;
		return(true);
	}
	if(power == pw_strength)
	{
		P_GiveBody(player, 100);
		player->powers[power] = 1;
		return(true);
	}
*/
    if (player->powers[power])
    {
        return (false);         // already got it
    }
    player->powers[power] = 1;
    return (true);
}

//---------------------------------------------------------------------------
//
// FUNC P_GiveArtifact
//
// Returns true if artifact accepted.
//
//---------------------------------------------------------------------------

boolean P_GiveArtifact(player_t * player, artitype_t arti, mobj_t * mo)
{
    int i;

    i = 0;
    while (player->inventory[i].type != arti && i < player->inventorySlotNum)
    {
        i++;
    }
    if (i == player->inventorySlotNum)
    {
        player->inventory[i].count = 1;
        player->inventory[i].type = arti;
        player->inventorySlotNum++;
    }
    else
    {
        if (player->inventory[i].count >= 16)
        {                       // Player already has 16 of this item
            return (false);
        }
        player->inventory[i].count++;
    }
    if (player->artifactCount == 0)
    {
        player->readyArtifact = arti;
    }
    player->artifactCount++;
    if (mo && (mo->flags & MF_COUNTITEM))
    {
        player->itemcount++;
    }
    return (true);
}

//---------------------------------------------------------------------------
//
// PROC P_SetDormantArtifact
//
// Removes the MF_SPECIAL flag, and initiates the artifact pickup
// animation.
//
//---------------------------------------------------------------------------

void P_SetDormantArtifact(mobj_t * arti)
{
    arti->flags &= ~MF_SPECIAL;
    if (deathmatch && (arti->type != MT_ARTIINVULNERABILITY)
        && (arti->type != MT_ARTIINVISIBILITY))
    {
        P_SetMobjState(arti, S_DORMANTARTI1);
    }
    else
    {                           // Don't respawn
        P_SetMobjState(arti, S_DEADARTI1);
    }
    S_StartSound(arti, sfx_artiup);
}

//---------------------------------------------------------------------------
//
// PROC A_RestoreArtifact
//
//---------------------------------------------------------------------------

void A_RestoreArtifact(mobj_t * arti)
{
    arti->flags |= MF_SPECIAL;
    P_SetMobjState(arti, arti->info->spawnstate);
    S_StartSound(arti, sfx_respawn);
}

//----------------------------------------------------------------------------
//
// PROC P_HideSpecialThing
//
//----------------------------------------------------------------------------

void P_HideSpecialThing(mobj_t * thing)
{
    thing->flags &= ~MF_SPECIAL;
    thing->flags2 |= MF2_DONTDRAW;
    P_SetMobjState(thing, S_HIDESPECIAL1);
}

//---------------------------------------------------------------------------
//
// PROC A_RestoreSpecialThing1
//
// Make a special thing visible again.
//
//---------------------------------------------------------------------------

void A_RestoreSpecialThing1(mobj_t * thing)
{
    if (thing->type == MT_WMACE)
    {                           // Do random mace placement
        P_RepositionMace(thing);
    }
    thing->flags2 &= ~MF2_DONTDRAW;
    S_StartSound(thing, sfx_respawn);
}

//---------------------------------------------------------------------------
//
// PROC A_RestoreSpecialThing2
//
//---------------------------------------------------------------------------

void A_RestoreSpecialThing2(mobj_t * thing)
{
    thing->flags |= MF_SPECIAL;
    P_SetMobjState(thing, thing->info->spawnstate);
}

//---------------------------------------------------------------------------
//
// PROC P_TouchSpecialThing
//
//---------------------------------------------------------------------------

void P_TouchSpecialThing(mobj_t * special, mobj_t * toucher)
{
    int i;
    player_t *player;
    fixed_t delta;
    int sound;
    boolean respawn;

    delta = special->z - toucher->z;
    if (delta > toucher->height || delta < -32 * FRACUNIT)
    {                           // Out of reach
        return;
    }
    if (toucher->health <= 0)
    {                           // Toucher is dead
        return;
    }
    sound = sfx_itemup;
    player = toucher->player;
    respawn = true;
    switch (special->sprite)
    {
            // Items
        case SPR_PTN1:         // Item_HealingPotion
            if (!P_GiveBody(player, 10))
            {
                return;
            }
            P_SetMessage(player, DEH_String(TXT_ITEMHEALTH), false);
            break;
        case SPR_SHLD:         // Item_Shield1
            if (!P_GiveArmor(player, 1))
            {
                return;
            }
            P_SetMessage(player, DEH_String(TXT_ITEMSHIELD1), false);
            break;
        case SPR_SHD2:         // Item_Shield2
            if (!P_GiveArmor(player, 2))
            {
                return;
            }
            P_SetMessage(player, DEH_String(TXT_ITEMSHIELD2), false);
            break;
        case SPR_BAGH:         // Item_BagOfHolding
            if (!player->backpack)
            {
                for (i = 0; i < NUMAMMO; i++)
                {
                    player->maxammo[i] *= 2;
                }
                player->backpack = true;
            }
            P_GiveAmmo(player, am_goldwand, AMMO_GWND_WIMPY);
            P_GiveAmmo(player, am_blaster, AMMO_BLSR_WIMPY);
            P_GiveAmmo(player, am_crossbow, AMMO_CBOW_WIMPY);
            P_GiveAmmo(player, am_skullrod, AMMO_SKRD_WIMPY);
            P_GiveAmmo(player, am_phoenixrod, AMMO_PHRD_WIMPY);
            P_SetMessage(player, DEH_String(TXT_ITEMBAGOFHOLDING), false);
            break;
        case SPR_SPMP:         // Item_SuperMap
            if (!P_GivePower(player, pw_allmap))
            {
                return;
            }
            P_SetMessage(player, DEH_String(TXT_ITEMSUPERMAP), false);
            break;

            // Keys
        case SPR_BKYY:         // Key_Blue
            if (!player->keys[key_blue])
            {
                P_SetMessage(player, DEH_String(TXT_GOTBLUEKEY), false);
            }
            P_GiveKey(player, key_blue);
            sound = sfx_keyup;
            if (!netgame)
            {
                break;
            }
            return;
        case SPR_CKYY:         // Key_Yellow
            if (!player->keys[key_yellow])
            {
                P_SetMessage(player, DEH_String(TXT_GOTYELLOWKEY), false);
            }
            sound = sfx_keyup;
            P_GiveKey(player, key_yellow);
            if (!netgame)
            {
                break;
            }
            return;
        case SPR_AKYY:         // Key_Green
            if (!player->keys[key_green])
            {
                P_SetMessage(player, DEH_String(TXT_GOTGREENKEY), false);
            }
            sound = sfx_keyup;
            P_GiveKey(player, key_green);
            if (!netgame)
            {
                break;
            }
            return;

            // Artifacts
        case SPR_PTN2:         // Arti_HealingPotion
            if (P_GiveArtifact(player, arti_health, special))
            {
                P_SetMessage(player, DEH_String(TXT_ARTIHEALTH), false);
                P_SetDormantArtifact(special);
            }
            return;
        case SPR_SOAR:         // Arti_Fly
            if (P_GiveArtifact(player, arti_fly, special))
            {
                P_SetMessage(player, DEH_String(TXT_ARTIFLY), false);
                P_SetDormantArtifact(special);
            }
            return;
        case SPR_INVU:         // Arti_Invulnerability
            if (P_GiveArtifact(player, arti_invulnerability, special))
            {
                P_SetMessage(player, DEH_String(TXT_ARTIINVULNERABILITY), false);
                P_SetDormantArtifact(special);
            }
            return;
        case SPR_PWBK:         // Arti_TomeOfPower
            if (P_GiveArtifact(player, arti_tomeofpower, special))
            {
                P_SetMessage(player, DEH_String(TXT_ARTITOMEOFPOWER), false);
                P_SetDormantArtifact(special);
            }
            return;
        case SPR_INVS:         // Arti_Invisibility
            if (P_GiveArtifact(player, arti_invisibility, special))
            {
                P_SetMessage(player, DEH_String(TXT_ARTIINVISIBILITY), false);
                P_SetDormantArtifact(special);
            }
            return;
        case SPR_EGGC:         // Arti_Egg
            if (P_GiveArtifact(player, arti_egg, special))
            {
                P_SetMessage(player, DEH_String(TXT_ARTIEGG), false);
                P_SetDormantArtifact(special);
            }
            return;
        case SPR_SPHL:         // Arti_SuperHealth
            if (P_GiveArtifact(player, arti_superhealth, special))
            {
                P_SetMessage(player, DEH_String(TXT_ARTISUPERHEALTH), false);
                P_SetDormantArtifact(special);
            }
            return;
        case SPR_TRCH:         // Arti_Torch
            if (P_GiveArtifact(player, arti_torch, special))
            {
                P_SetMessage(player, DEH_String(TXT_ARTITORCH), false);
                P_SetDormantArtifact(special);
            }
            return;
        case SPR_FBMB:         // Arti_FireBomb
            if (P_GiveArtifact(player, arti_firebomb, special))
            {
                P_SetMessage(player, DEH_String(TXT_ARTIFIREBOMB), false);
                P_SetDormantArtifact(special);
            }
            return;
        case SPR_ATLP:         // Arti_Teleport
            if (P_GiveArtifact(player, arti_teleport, special))
            {
                P_SetMessage(player, DEH_String(TXT_ARTITELEPORT), false);
                P_SetDormantArtifact(special);
            }
            return;

            // Ammo
        case SPR_AMG1:         // Ammo_GoldWandWimpy
            if (!P_GiveAmmo(player, am_goldwand, special->health))
            {
                return;
            }
            P_SetMessage(player, DEH_String(TXT_AMMOGOLDWAND1), false);
            break;
        case SPR_AMG2:         // Ammo_GoldWandHefty
            if (!P_GiveAmmo(player, am_goldwand, special->health))
            {
                return;
            }
            P_SetMessage(player, DEH_String(TXT_AMMOGOLDWAND2), false);
            break;
        case SPR_AMM1:         // Ammo_MaceWimpy
            if (!P_GiveAmmo(player, am_mace, special->health))
            {
                return;
            }
            P_SetMessage(player, DEH_String(TXT_AMMOMACE1), false);
            break;
        case SPR_AMM2:         // Ammo_MaceHefty
            if (!P_GiveAmmo(player, am_mace, special->health))
            {
                return;
            }
            P_SetMessage(player, DEH_String(TXT_AMMOMACE2), false);
            break;
        case SPR_AMC1:         // Ammo_CrossbowWimpy
            if (!P_GiveAmmo(player, am_crossbow, special->health))
            {
                return;
            }
            P_SetMessage(player, DEH_String(TXT_AMMOCROSSBOW1), false);
            break;
        case SPR_AMC2:         // Ammo_CrossbowHefty
            if (!P_GiveAmmo(player, am_crossbow, special->health))
            {
                return;
            }
            P_SetMessage(player, DEH_String(TXT_AMMOCROSSBOW2), false);
            break;
        case SPR_AMB1:         // Ammo_BlasterWimpy
            if (!P_GiveAmmo(player, am_blaster, special->health))
            {
                return;
            }
            P_SetMessage(player, DEH_String(TXT_AMMOBLASTER1), false);
            break;
        case SPR_AMB2:         // Ammo_BlasterHefty
            if (!P_GiveAmmo(player, am_blaster, special->health))
            {
                return;
            }
            P_SetMessage(player, DEH_String(TXT_AMMOBLASTER2), false);
            break;
        case SPR_AMS1:         // Ammo_SkullRodWimpy
            if (!P_GiveAmmo(player, am_skullrod, special->health))
            {
                return;
            }
            P_SetMessage(player, DEH_String(TXT_AMMOSKULLROD1), false);
            break;
        case SPR_AMS2:         // Ammo_SkullRodHefty
            if (!P_GiveAmmo(player, am_skullrod, special->health))
            {
                return;
            }
            P_SetMessage(player, DEH_String(TXT_AMMOSKULLROD2), false);
            break;
        case SPR_AMP1:         // Ammo_PhoenixRodWimpy
            if (!P_GiveAmmo(player, am_phoenixrod, special->health))
            {
                return;
            }
            P_SetMessage(player, DEH_String(TXT_AMMOPHOENIXROD1), false);
            break;
        case SPR_AMP2:         // Ammo_PhoenixRodHefty
            if (!P_GiveAmmo(player, am_phoenixrod, special->health))
            {
                return;
            }
            P_SetMessage(player, DEH_String(TXT_AMMOPHOENIXROD2), false);
            break;

            // Weapons
        case SPR_WMCE:         // Weapon_Mace
            if (!P_GiveWeapon(player, wp_mace))
            {
                return;
            }
            P_SetMessage(player, DEH_String(TXT_WPNMACE), false);
            sound = sfx_wpnup;
            break;
        case SPR_WBOW:         // Weapon_Crossbow
            if (!P_GiveWeapon(player, wp_crossbow))
            {
                return;
            }
            P_SetMessage(player, DEH_String(TXT_WPNCROSSBOW), false);
            sound = sfx_wpnup;
            break;
        case SPR_WBLS:         // Weapon_Blaster
            if (!P_GiveWeapon(player, wp_blaster))
            {
                return;
            }
            P_SetMessage(player, DEH_String(TXT_WPNBLASTER), false);
            sound = sfx_wpnup;
            break;
        case SPR_WSKL:         // Weapon_SkullRod
            if (!P_GiveWeapon(player, wp_skullrod))
            {
                return;
            }
            P_SetMessage(player, DEH_String(TXT_WPNSKULLROD), false);
            sound = sfx_wpnup;
            break;
        case SPR_WPHX:         // Weapon_PhoenixRod
            if (!P_GiveWeapon(player, wp_phoenixrod))
            {
                return;
            }
            P_SetMessage(player, DEH_String(TXT_WPNPHOENIXROD), false);
            sound = sfx_wpnup;
            break;
        case SPR_WGNT:         // Weapon_Gauntlets
            if (!P_GiveWeapon(player, wp_gauntlets))
            {
                return;
            }
            P_SetMessage(player, DEH_String(TXT_WPNGAUNTLETS), false);
            sound = sfx_wpnup;
            break;
        default:
            I_Error("P_SpecialThing: Unknown gettable thing");
    }
    if (special->flags & MF_COUNTITEM)
    {
        player->itemcount++;
    }
    if (deathmatch && respawn && !(special->flags & MF_DROPPED))
    {
        P_HideSpecialThing(special);
    }
    else
    {
        P_RemoveMobj(special);
    }
    player->bonuscount += BONUSADD;
    if (player == &players[consoleplayer])
    {
        S_StartSound(NULL, sound);
        SB_PaletteFlash();
    }
}

//---------------------------------------------------------------------------
//
// PROC P_KillMobj
//
//---------------------------------------------------------------------------

void P_KillMobj(mobj_t * source, mobj_t * target)
{
    target->flags &= ~(MF_SHOOTABLE | MF_FLOAT | MF_SKULLFLY | MF_NOGRAVITY);
    target->flags |= MF_CORPSE | MF_DROPOFF;
    target->flags2 &= ~MF2_PASSMOBJ;
    target->height >>= 2;
    if (source && source->player)
    {
        if (target->flags & MF_COUNTKILL)
        {                       // Count for intermission
            source->player->killcount++;
        }
        if (target->player)
        {                       // Frag stuff
            if (target == source)
            {                   // Self-frag
                target->player->frags[target->player - players]--;
            }
            else
            {
                source->player->frags[target->player - players]++;
                if (source->player == &players[consoleplayer])
                {
                    S_StartSound(NULL, sfx_gfrag);
                }
                if (source->player->chickenTics)
                {               // Make a super chicken
                    P_GivePower(source->player, pw_weaponlevel2);
                }
            }
        }
    }
    else if (!netgame && (target->flags & MF_COUNTKILL))
    {                           // Count all monster deaths
        players[0].killcount++;
    }
    if (target->player)
    {
        if (!source)
        {                       // Self-frag
            target->player->frags[target->player - players]--;
        }
        target->flags &= ~MF_SOLID;
        target->flags2 &= ~MF2_FLY;
        target->player->powers[pw_flight] = 0;
        target->player->powers[pw_weaponlevel2] = 0;
        target->player->playerstate = PST_DEAD;
        P_DropWeapon(target->player);
        if (target->flags2 & MF2_FIREDAMAGE)
        {                       // Player flame death
            P_SetMobjState(target, S_PLAY_FDTH1);
            //S_StartSound(target, sfx_hedat1); // Burn sound
            return;
        }
    }
    if (target->health < -(target->info->spawnhealth >> 1)
        && target->info->xdeathstate)
    {                           // Extreme death
        P_SetMobjState(target, target->info->xdeathstate);
    }
    else
    {                           // Normal death
        P_SetMobjState(target, target->info->deathstate);
    }
    target->tics -= P_Random() & 3;
//      I_StartSound(&actor->r, actor->info->deathsound);
}

//---------------------------------------------------------------------------
//
// FUNC P_MinotaurSlam
//
//---------------------------------------------------------------------------

void P_MinotaurSlam(mobj_t * source, mobj_t * target)
{
    angle_t angle;
    fixed_t thrust;

    angle = R_PointToAngle2(source->x, source->y, target->x, target->y);
    angle >>= ANGLETOFINESHIFT;
    thrust = 16 * FRACUNIT + (P_Random() << 10);
    target->momx += FixedMul(thrust, finecosine[angle]);
    target->momy += FixedMul(thrust, finesine[angle]);
    P_DamageMobj(target, NULL, NULL, HITDICE(6));
    if (target->player)
    {
        target->reactiontime = 14 + (P_Random() & 7);
    }
}

//---------------------------------------------------------------------------
//
// FUNC P_TouchWhirlwind
//
//---------------------------------------------------------------------------

void P_TouchWhirlwind(mobj_t * target)
{
    int randVal;

    target->angle += P_SubRandom() << 20;
    target->momx += P_SubRandom() << 10;
    target->momy += P_SubRandom() << 10;
    if (leveltime & 16 && !(target->flags2 & MF2_BOSS))
    {
        randVal = P_Random();
        if (randVal > 160)
        {
            randVal = 160;
        }
        target->momz += randVal << 10;
        if (target->momz > 12 * FRACUNIT)
        {
            target->momz = 12 * FRACUNIT;
        }
    }
    if (!(leveltime & 7))
    {
        P_DamageMobj(target, NULL, NULL, 3);
    }
}

//---------------------------------------------------------------------------
//
// FUNC P_ChickenMorphPlayer
//
// Returns true if the player gets turned into a chicken.
//
//---------------------------------------------------------------------------

boolean P_ChickenMorphPlayer(player_t * player)
{
    mobj_t *pmo;
    mobj_t *fog;
    mobj_t *chicken;
    fixed_t x;
    fixed_t y;
    fixed_t z;
    angle_t angle;
    int oldFlags2;

    if (player->chickenTics)
    {
        if ((player->chickenTics < CHICKENTICS - TICRATE)
            && !player->powers[pw_weaponlevel2])
        {                       // Make a super chicken
            P_GivePower(player, pw_weaponlevel2);
        }
        return (false);
    }
    if (player->powers[pw_invulnerability])
    {                           // Immune when invulnerable
        return (false);
    }
    pmo = player->mo;
    x = pmo->x;
    y = pmo->y;
    z = pmo->z;
    angle = pmo->angle;
    oldFlags2 = pmo->flags2;
    P_SetMobjState(pmo, S_FREETARGMOBJ);
    fog = P_SpawnMobj(x, y, z + TELEFOGHEIGHT, MT_TFOG);
    S_StartSound(fog, sfx_telept);
    chicken = P_SpawnMobj(x, y, z, MT_CHICPLAYER);
    chicken->special1.i = player->readyweapon;
    chicken->angle = angle;
    chicken->player = player;
    player->health = chicken->health = MAXCHICKENHEALTH;
    player->mo = chicken;
    player->armorpoints = player->armortype = 0;
    player->powers[pw_invisibility] = 0;
    player->powers[pw_weaponlevel2] = 0;
    if (oldFlags2 & MF2_FLY)
    {
        chicken->flags2 |= MF2_FLY;
    }
    player->chickenTics = CHICKENTICS;
    P_ActivateBeak(player);
    return (true);
}

//---------------------------------------------------------------------------
//
// FUNC P_ChickenMorph
//
//---------------------------------------------------------------------------

boolean P_ChickenMorph(mobj_t * actor)
{
    mobj_t *fog;
    mobj_t *chicken;
    mobj_t *target;
    mobjtype_t moType;
    fixed_t x;
    fixed_t y;
    fixed_t z;
    angle_t angle;
    int ghost;

    if (actor->player)
    {
        return (false);
    }
    moType = actor->type;
    switch (moType)
    {
        case MT_POD:
        case MT_CHICKEN:
        case MT_HEAD:
        case MT_MINOTAUR:
        case MT_SORCERER1:
        case MT_SORCERER2:
            return (false);
        default:
            break;
    }
    x = actor->x;
    y = actor->y;
    z = actor->z;
    angle = actor->angle;
    ghost = actor->flags & MF_SHADOW;
    target = actor->target;
    P_SetMobjState(actor, S_FREETARGMOBJ);
    fog = P_SpawnMobj(x, y, z + TELEFOGHEIGHT, MT_TFOG);
    S_StartSound(fog, sfx_telept);
    chicken = P_SpawnMobj(x, y, z, MT_CHICKEN);
    chicken->special2.i = moType;
    chicken->special1.i = CHICKENTICS + P_Random();
    chicken->flags |= ghost;
    chicken->target = target;
    chicken->angle = angle;
    return (true);
}

//---------------------------------------------------------------------------
//
// FUNC P_AutoUseChaosDevice
//
//---------------------------------------------------------------------------

boolean P_AutoUseChaosDevice(player_t * player)
{
    int i;

    for (i = 0; i < player->inventorySlotNum; i++)
    {
        if (player->inventory[i].type == arti_teleport)
        {
            P_PlayerUseArtifact(player, arti_teleport);
            player->health = player->mo->health = (player->health + 1) / 2;
            return (true);
        }
    }
    return (false);
}

//---------------------------------------------------------------------------
//
// PROC P_AutoUseHealth
//
//---------------------------------------------------------------------------

void P_AutoUseHealth(player_t * player, int saveHealth)
{
    int i;
    int count;
    int normalCount;
    int normalSlot;
    int superCount;
    int superSlot;

    normalCount = 0;
    superCount = 0;
    normalSlot = 0;
    superSlot = 0;

    for (i = 0; i < player->inventorySlotNum; i++)
    {
        if (player->inventory[i].type == arti_health)
        {
            normalSlot = i;
            normalCount = player->inventory[i].count;
        }
        else if (player->inventory[i].type == arti_superhealth)
        {
            superSlot = i;
            superCount = player->inventory[i].count;
        }
    }
    if ((gameskill == sk_baby) && (normalCount * 25 >= saveHealth))
    {                           // Use quartz flasks
        count = (saveHealth + 24) / 25;
        for (i = 0; i < count; i++)
        {
            player->health += 25;
            P_PlayerRemoveArtifact(player, normalSlot);
        }
    }
    else if (superCount * 100 >= saveHealth)
    {                           // Use mystic urns
        count = (saveHealth + 99) / 100;
        for (i = 0; i < count; i++)
        {
            player->health += 100;
            P_PlayerRemoveArtifact(player, superSlot);
        }
    }
    else if ((gameskill == sk_baby)
             && (superCount * 100 + normalCount * 25 >= saveHealth))
    {                           // Use mystic urns and quartz flasks
        count = (saveHealth + 24) / 25;
        saveHealth -= count * 25;
        for (i = 0; i < count; i++)
        {
            player->health += 25;
            P_PlayerRemoveArtifact(player, normalSlot);
        }
        count = (saveHealth + 99) / 100;
        for (i = 0; i < count; i++)
        {
            player->health += 100;
            P_PlayerRemoveArtifact(player, normalSlot);
        }
    }
    player->mo->health = player->health;
}

/*
=================
=
= P_DamageMobj
=
= Damages both enemies and players
= inflictor is the thing that caused the damage
= 		creature or missile, can be NULL (slime, etc)
= source is the thing to target after taking damage
=		creature or NULL
= Source and inflictor are the same for melee attacks
= source can be null for barrel explosions and other environmental stuff
==================
*/

void P_DamageMobj
    (mobj_t * target, mobj_t * inflictor, mobj_t * source, int damage)
{
    unsigned ang;
    int saved;
    player_t *player;
    fixed_t thrust;
    int temp;

    if (!(target->flags & MF_SHOOTABLE))
    {
        // Shouldn't happen
        return;
    }
    if (target->health <= 0)
    {
        return;
    }
    if (target->flags & MF_SKULLFLY)
    {
        if (target->type == MT_MINOTAUR)
        {                       // Minotaur is invulnerable during charge attack
            return;
        }
        target->momx = target->momy = target->momz = 0;
    }
    player = target->player;
    if (player && gameskill == sk_baby)
    {
        // Take half damage in trainer mode
        damage >>= 1;
    }
    // Special damage types
    if (inflictor)
    {
        switch (inflictor->type)
        {
            case MT_EGGFX:
                if (player)
                {
                    P_ChickenMorphPlayer(player);
                }
                else
                {
                    P_ChickenMorph(target);
                }
                return;         // Always return
            case MT_WHIRLWIND:
                P_TouchWhirlwind(target);
                return;
            case MT_MINOTAUR:
                if (inflictor->flags & MF_SKULLFLY)
                {               // Slam only when in charge mode
                    P_MinotaurSlam(inflictor, target);
                    return;
                }
                break;
            case MT_MACEFX4:   // Death ball
                if ((target->flags2 & MF2_BOSS) || target->type == MT_HEAD)
                {               // Don't allow cheap boss kills
                    break;
                }
                else if (target->player)
                {               // Player specific checks
                    if (target->player->powers[pw_invulnerability])
                    {           // Can't hurt invulnerable players
                        break;
                    }
                    if (P_AutoUseChaosDevice(target->player))
                    {           // Player was saved using chaos device
                        return;
                    }
                }
                damage = 10000; // Something's gonna die
                break;
            case MT_PHOENIXFX2:        // Flame thrower
                if (target->player && P_Random() < 128)
                {               // Freeze player for a bit
                    target->reactiontime += 4;
                }
                break;
            case MT_RAINPLR1:  // Rain missiles
            case MT_RAINPLR2:
            case MT_RAINPLR3:
            case MT_RAINPLR4:
                if (target->flags2 & MF2_BOSS)
                {               // Decrease damage for bosses
                    damage = (P_Random() & 7) + 1;
                }
                break;
            case MT_HORNRODFX2:
            case MT_PHOENIXFX1:
                if (target->type == MT_SORCERER2 && P_Random() < 96)
                {               // D'Sparil teleports away
                    P_DSparilTeleport(target);
                    return;
                }
                break;
            case MT_BLASTERFX1:
            case MT_RIPPER:
                if (target->type == MT_HEAD)
                {               // Less damage to Ironlich bosses
                    damage = P_Random() & 1;
                    if (!damage)
                    {
                        return;
                    }
                }
                break;
            default:
                break;
        }
    }
    // Push the target unless source is using the gauntlets
    if (inflictor && (!source || !source->player
                      || source->player->readyweapon != wp_gauntlets)
        && !(inflictor->flags2 & MF2_NODMGTHRUST))
    {
        ang = R_PointToAngle2(inflictor->x, inflictor->y,
                              target->x, target->y);
        //thrust = damage*(FRACUNIT>>3)*100/target->info->mass;
        thrust = damage * (FRACUNIT >> 3) * 150 / target->info->mass;
        // make fall forwards sometimes
        if ((damage < 40) && (damage > target->health)
            && (target->z - inflictor->z > 64 * FRACUNIT) && (P_Random() & 1))
        {
            ang += ANG180;
            thrust *= 4;
        }
        ang >>= ANGLETOFINESHIFT;
        if (source && source->player && (source == inflictor)
            && source->player->powers[pw_weaponlevel2]
            && source->player->readyweapon == wp_staff)
        {
            // Staff power level 2
            target->momx += FixedMul(10 * FRACUNIT, finecosine[ang]);
            target->momy += FixedMul(10 * FRACUNIT, finesine[ang]);
            if (!(target->flags & MF_NOGRAVITY))
            {
                target->momz += 5 * FRACUNIT;
            }
        }
        else
        {
            target->momx += FixedMul(thrust, finecosine[ang]);
            target->momy += FixedMul(thrust, finesine[ang]);
        }
    }

    //
    // player specific
    //
    if (player)
    {
        // end of game hell hack
        //if(target->subsector->sector->special == 11
        //      && damage >= target->health)
        //{
        //      damage = target->health - 1;
        //}

        if (damage < 1000 && ((player->cheats & CF_GODMODE)
                              || player->powers[pw_invulnerability]))
        {
            return;
        }
        if (player->armortype)
        {
            if (player->armortype == 1)
            {
                saved = damage >> 1;
            }
            else
            {
                saved = (damage >> 1) + (damage >> 2);
            }
            if (player->armorpoints <= saved)
            {
                // armor is used up
                saved = player->armorpoints;
                player->armortype = 0;
            }
            player->armorpoints -= saved;
            damage -= saved;
        }
        if (damage >= player->health
            && ((gameskill == sk_baby) || deathmatch) && !player->chickenTics)
        {                       // Try to use some inventory health
            P_AutoUseHealth(player, damage - player->health + 1);
        }
        player->health -= damage;       // mirror mobj health here for Dave
        if (player->health < 0)
        {
            player->health = 0;
        }
        player->attacker = source;
        player->damagecount += damage;  // add damage after armor / invuln
        if (player->damagecount > 100)
        {
            player->damagecount = 100;  // teleport stomp does 10k points...
        }
        temp = damage < 100 ? damage : 100;
        if (player == &players[consoleplayer])
        {
            I_Tactile(40, 10, 40 + temp * 2);
            SB_PaletteFlash();
        }
    }

    //
    // do the damage
    //
    target->health -= damage;
    if (target->health <= 0)
    {                           // Death
        target->special1.i = damage;
        if (target->type == MT_POD && source && source->type != MT_POD)
        {                       // Make sure players get frags for chain-reaction kills
            target->target = source;
        }
        if (player && inflictor && !player->chickenTics)
        {                       // Check for flame death
            if ((inflictor->flags2 & MF2_FIREDAMAGE)
                || ((inflictor->type == MT_PHOENIXFX1)
                    && (target->health > -50) && (damage > 25)))
            {
                target->flags2 |= MF2_FIREDAMAGE;
            }
        }
        P_KillMobj(source, target);
        return;
    }
    if ((P_Random() < target->info->painchance)
        && !(target->flags & MF_SKULLFLY))
    {
        target->flags |= MF_JUSTHIT;    // fight back!
        P_SetMobjState(target, target->info->painstate);
    }
    target->reactiontime = 0;   // we're awake now...
    if (!target->threshold && source && !(source->flags2 & MF2_BOSS)
        && !(target->type == MT_SORCERER2 && source->type == MT_WIZARD))
    {
        // Target actor is not intent on another actor,
        // so make him chase after source
        target->target = source;
        target->threshold = BASETHRESHOLD;
        if (target->state == &states[target->info->spawnstate]
            && target->info->seestate != S_NULL)
        {
            P_SetMobjState(target, target->info->seestate);
        }
    }
}
