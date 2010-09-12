// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2005 Simon Howard
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
//	Handling interactions (i.e., collisions).
//
//-----------------------------------------------------------------------------




// Data.
#include "doomdef.h"
#include "dstrings.h"
#include "sounds.h"
#include "deh_main.h"
#include "deh_misc.h"
#include "doomstat.h"
#include "m_random.h"
#include "i_system.h"
#include "am_map.h"
#include "p_local.h"
#include "s_sound.h"
#include "p_inter.h"


#define BONUSADD	6




// a weapon is found with two clip loads,
// a big item has five clip loads
// villsa [STRIFE] updated arrays
int maxammo[NUMAMMO]    = { 250, 50, 25, 400, 100, 30, 16 };
int clipammo[NUMAMMO]   = { 10, 4, 2, 20, 4, 6, 4 };


//
// GET STUFF
//

//
// P_GiveAmmo
// Num is the number of clip loads,
// not the individual count (0= 1/2 clip).
// Returns false if the ammo can't be picked up at all
//

boolean P_GiveAmmo(player_t* player, ammotype_t ammo, int num)
{
    int		oldammo;
	
    if(ammo == am_noammo)
	return false;
		
    if(ammo > NUMAMMO)
	I_Error ("P_GiveAmmo: bad type %i", ammo);
		
    if(player->ammo[ammo] == player->maxammo[ammo])
	return false;
		
    if(num)
	num *= clipammo[ammo];
    else
	num = clipammo[ammo]/2;
    
    if(gameskill == sk_baby
	|| gameskill == sk_nightmare)
    {
	// give double ammo in trainer mode,
	// you'll need in nightmare
	num <<= 1;
    }
    
		
    oldammo = player->ammo[ammo];
    player->ammo[ammo] += num;

    if(player->ammo[ammo] > player->maxammo[ammo])
	player->ammo[ammo] = player->maxammo[ammo];

    // If non zero ammo, 
    // don't change up weapons,
    // player was lower on purpose.
    if(oldammo)
	return true;	

    // We were down to zero,
    // so select a new weapon.
    // Preferences are not user selectable.

    // villsa [STRIFE] ammo update
    // [STRIFE] TODO - where's the check for grenades?
    switch(ammo && !player->readyweapon)
    {
    case am_bullets:
        if(player->weaponowned[wp_rifle])
            player->pendingweapon = wp_rifle;
        break;

    case am_elecbolts:
    case am_poisonbolts:
        if(player->weaponowned[wp_elecbow])
            player->pendingweapon = wp_elecbow;
        break;

    case am_cell:
        if(player->weaponowned[wp_mauler])
            player->pendingweapon = wp_mauler;
        break;

    case am_missiles:
        if(player->weaponowned[wp_missile])
            player->pendingweapon = wp_missile;
        break;

    default:
        break;
    }
	
    return true;
}


//
// P_GiveWeapon
// The weapon name may have a MF_DROPPED flag ored in.
//
// villsa [STRIFE] some stuff has been changed/moved around
//
boolean P_GiveWeapon(player_t* player, weapontype_t weapon, boolean dropped)
{
    boolean gaveammo;
    boolean gaveweapon;

    // villsa [STRIFE] new code for giving alternate version
    // of the weapon to player
    if(player->weaponowned[weapon])
        gaveweapon = false;
    else
    {
        gaveweapon = true;
        player->weaponowned[weapon] = true;

        switch(weapon)
        {
        case wp_elecbow:
            player->weaponowned[wp_poisonbow] = true;
            break;

        case wp_hegrenade:
            player->weaponowned[wp_wpgrenade] = true;
            break;

        case wp_mauler:
            player->weaponowned[wp_torpedo] = true;
            break;
        }

        // check for the standard weapons only
        if(weapon > player->readyweapon && weapon <= wp_sigil)
            player->pendingweapon = weapon;

    }

    if(netgame && (deathmatch != 2) && !dropped)
    {
        // leave placed weapons forever on net games
        if(!gaveweapon)
            return false;

        player->bonuscount += BONUSADD;
        player->weaponowned[weapon] = true;

        if(deathmatch)
            P_GiveAmmo(player, weaponinfo[weapon].ammo, 5);
        else
            P_GiveAmmo(player, weaponinfo[weapon].ammo, 2);

        if(player == &players[consoleplayer])
            S_StartSound (NULL, sfx_wpnup);
        return false;
    }

    if(weaponinfo[weapon].ammo != am_noammo)
    {
        // give one clip with a dropped weapon,
        // two clips with a found weapon
        if(dropped)
            gaveammo = P_GiveAmmo (player, weaponinfo[weapon].ammo, 1);
        else
            gaveammo = P_GiveAmmo (player, weaponinfo[weapon].ammo, 2);
    }
    else
        gaveammo = false;

    return(gaveweapon || gaveammo);
}

 

//
// P_GiveBody
// Returns false if the body isn't needed at all
//
// villsa [STRIFE] a lot of changes has been added for stamina
//
boolean P_GiveBody(player_t* player, int num)
{
    int health;

    if(num >= 0)
    {
        health = player->stamina + MAXHEALTH;

        if(health <= player->health)
            return false;

        player->health += num;

        if(health < player->health + num)
            player->health = health;
        player->mo->health = player->health;
    }
    // [STRIFE] handle healing from the front's medic
    else
    {
        health = (-num * (player->stamina + MAXHEALTH)) / MAXHEALTH;
        if(health <= player->health)
            return false;

        player->health = health;
    }
	
    return true;
}



//
// P_GiveArmor
// Returns false if the armor is worse
// than the current armor.
//
boolean P_GiveArmor(player_t* player, int armortype)
{
    int hits;

    // villsa [STRIFE]
    if(armortype < 0)
    {
        if(player->armorpoints)
            return false;

        armortype = -armortype;
    }

    hits = armortype * 100;
    if(player->armorpoints >= hits)
        return false;	// don't pick up

    player->armortype = armortype;
    player->armorpoints = hits;

    return true;
}



//
// P_GiveCard
//
boolean P_GiveCard(player_t* player, card_t card)
{
    if (player->cards[card])
	return false;
    
    // villsa [STRIFE] multiply by 2
    player->bonuscount = BONUSADD * 2;
    player->cards[card] = 1;

    return true;
}


//
// P_GivePower
//
boolean P_GivePower(player_t* player, powertype_t power)
{
    // villsa [STRIFE]
    if(power == pw_targeter)
    {
        player->powers[power] = TARGTICS;
        P_SetPsprite(player, ps_targcenter, S_TRGT_00); // 10
        P_SetPsprite(player, ps_targleft, S_TRGT_01); // 11
        P_SetPsprite(player, ps_targright, S_TRGT_02); // 12

        player->psprites[ps_targcenter].sx  = (160*FRACUNIT);
        player->psprites[ps_targleft].sy    = (100*FRACUNIT);
        player->psprites[ps_targcenter].sy  = (100*FRACUNIT);
        player->psprites[ps_targright].sy   = (100*FRACUNIT);
        return true;
    }

    if(power == pw_invisibility)
    {
        player->powers[power] = INVISTICS;
        player->mo->flags |= MF_SHADOW;
        return true;
    }

    if(power == pw_ironfeet)
    {
        player->powers[power] = IRONTICS;
        return true;
    }

    if(power == pw_strength)
    {
        P_GiveBody(player, 100);
        player->powers[power] = 1;
        return true;
    }

    // villsa [STRIFE]
    if(power == pw_allmap)
    {
        if(gamemap < 40)
            player->mapstate[gamemap] = 1;

        player->powers[power] = 1;
        return true;
    }

    // villsa [STRIFE]
    if(power == pw_communicator)
    {
        player->powers[power] = 1;
        return true;
    }


    if(player->powers[power])
        return false;	// already got it


    player->powers[power] = 1;
    return true;
}


// villsa [STRIFE]
static char pickupmsg[80];

//
// P_TouchSpecialThing
//
void P_TouchSpecialThing(mobj_t* special, mobj_t* toucher)
{
    player_t*   player;
    int         i;
    fixed_t     delta;
    int         sound;

    delta = special->z - toucher->z;

    if(delta > toucher->height || delta < -8*FRACUNIT)
        return; // out of reach


    sound = sfx_itemup;	
    player = toucher->player;

    // Dead thing touching.
    // Can happen with a sliding player corpse.
    if (toucher->health <= 0)
        return;

    // villsa [STRIFE]
    pickupmsg[0] = 0;

    // Identify by sprite.
    // villsa [STRIFE] new items
    switch(special->sprite)
    {
    // bullets
    case SPR_BLIT:
        if(!P_GiveAmmo(player, am_bullets, 1))
            return;
        break;

    // box of bullets
    case SPR_BBOX:
        if(!P_GiveAmmo(player, am_bullets, 5))
            return;
        break;

    // missile
    case SPR_ROKT:
        if(!P_GiveAmmo(player, am_missiles, 1))
            return;
        break;

    // box of missiles
    case SPR_MSSL:
        if(!P_GiveAmmo(player, am_missiles, 5))
            return;
        break;

    // battery
    case SPR_BRY1:
        if(!P_GiveAmmo(player, am_cell, 1))
            return;
        break;

    // cell pack
    case SPR_CPAC:
        if(!P_GiveAmmo(player, am_cell, 5))
            return;
        break;

    // poison bolts
    case SPR_PQRL:
        if(!P_GiveAmmo(player, am_poisonbolts, 5))
            return;
        break;

    // electric bolts
    case SPR_XQRL:
        if(!P_GiveAmmo(player, am_elecbolts, 5))
            return;
        break;

    // he grenades
    case SPR_GRN1:
        if(!P_GiveAmmo(player, am_hegrenades, 1))
            return;
        break;

    // wp grenades
    case SPR_GRN2:
        if(!P_GiveAmmo(player, am_wpgrenades, 1))
            return;
        break;

    // rifle
    case SPR_RIFL:
        if(!P_GiveWeapon(player, wp_rifle, special->flags&MF_DROPPED))
            return;
        break;

    // flame thrower
    case SPR_FLAM:
        if(!P_GiveWeapon(player, wp_flame, false))
            return;
        break;

    // missile launcher
    case SPR_MMSL:
        if(!P_GiveWeapon(player, wp_missile, false))
            return;
        break;

    // missile launcher
    case SPR_GRND:
        if(!P_GiveWeapon(player, wp_hegrenade, special->flags&MF_DROPPED))
            return;
        break;

    // mauler
    case SPR_TRPD:
        if(!P_GiveWeapon(player, wp_mauler, false))
            return;
        break;

    // crossbow
    case SPR_CBOW:
        if(!P_GiveWeapon(player, wp_elecbow, special->flags&MF_DROPPED))
            return;
        break;

    // backpack
    case SPR_BKPK:
        if(!player->backpack)
        {
            for(i = 0; i < NUMAMMO; i++)
                player->maxammo[i] *= 2;

            player->backpack = true;
        }
        for(i = 0; i < NUMAMMO; i++)
	    P_GiveAmmo(player, i, 1);
        break;

    case SPR_COIN:
        P_GiveInventoryItem(player, SPR_COIN, MT_MONY_1);
        break;

    case SPR_CRED:
        for(i = 0; i < 10; i++)
            P_GiveInventoryItem(player, SPR_COIN, MT_MONY_1);
        break;

    case SPR_SACK:
        for(i = 0; i < 25; i++)
            P_GiveInventoryItem(player, SPR_COIN, MT_MONY_1);
        break;

    case SPR_CHST:
        for(i = 0; i < 50; i++)
            P_GiveInventoryItem(player, SPR_COIN, MT_MONY_1);
        break;

    case SPR_ARM1:
        if(!P_GiveArmor(player, -2))
            if(!P_GiveInventoryItem(player, special->sprite, special->type))
                pickupmsg[0] = '!';
        break;

    case SPR_ARM2:
        if(!P_GiveArmor(player, -1))
            if(!P_GiveInventoryItem(player, special->sprite, special->type))
                pickupmsg[0] = '!';
        break;

    case SPR_PMAP:
        if(!P_GivePower(player, pw_allmap))
            return;
        sound = sfx_yeah;	
        break;

    case SPR_COMM:
        if(!P_GivePower(player, pw_communicator))
            return;
        sound = sfx_yeah;
        break;

    // villsa [STRIFE] check default items
    case SPR_TOKN:
    default:
        if(special->type >= MT_KEY_BASE && special->type <= MT_NEWKEY5)
        {
            if(!P_GiveCard(player, special->type - MT_KEY_BASE))
                return;
        }
        else
        {
            if(!P_GiveInventoryItem(player, special->sprite, special->type))
                    pickupmsg[0] = '!';
        }
    }

    // villsa [STRIFE] set message
    if(!pickupmsg[0])
    {
        if(special->info->name)
            sprintf(pickupmsg, "You picked up the %s.", special->info->name);
        else
            sprintf(pickupmsg, "You picked up the item.");
    }
    // use the first character to indicate that the player is full on items
    else if(pickupmsg[0] == '!')
    {
        sprintf(pickupmsg, "You cannot hold any more.");
        player->message = pickupmsg;
        return;
    }

    if(special->flags & MF_GIVEQUEST)
    {
        // [STRIFE] TODO - verify this. Seems that questflag isn't
        // applied if the special's speed is equal to 8 or if
        // the player has recieved the SLIDESHOW token
        if(special->info->speed != 8 || !(player->questflags & 32))
            player->questflags |= 1 << (special->info->speed - 1);
    }


    // haleyjd 08/30/10: [STRIFE] No itemcount
    //if (special->flags & MF_COUNTITEM)
    //    player->itemcount++;

    P_RemoveMobj(special);
    player->message = pickupmsg;
    player->bonuscount += BONUSADD;

    if(player == &players[consoleplayer])
        S_StartSound(NULL, sound);
}


//
// KillMobj
//
void
P_KillMobj
( mobj_t*	source,
  mobj_t*	target )
{
    mobjtype_t	item;
    mobj_t*	mo;
	
    // villsa [STRIFE] MF_SPECIAL is added in the check
    target->flags &= ~(MF_SHOOTABLE|MF_FLOAT|MF_BOUNCE|MF_SPECIAL);

    // villsa [STRIFE] unused
    /*if (target->type != MT_SKULL)
	target->flags &= ~MF_NOGRAVITY;*/

    target->flags |= MF_CORPSE|MF_DROPOFF;
    target->height >>= 2;

    if (source && source->player)
    {
	// count for intermission
	if (target->flags & MF_COUNTKILL)
	    source->player->killcount++;	

	if (target->player)
	    source->player->frags[target->player-players]++;
    }
    else if (!netgame && (target->flags & MF_COUNTKILL) )
    {
	// count all monster deaths,
	// even those caused by other monsters
	players[0].killcount++;
    }
    
    if (target->player)
    {
	// count environment kills against you
	if (!source)	
	    target->player->frags[target->player-players]++;
			
	target->flags &= ~MF_SOLID;
	target->player->playerstate = PST_DEAD;
	P_DropWeapon (target->player);

	if (target->player == &players[consoleplayer]
	    && automapactive)
	{
	    // don't die in auto map,
	    // switch view prior to dying
	    AM_Stop ();
	}
	
    }

    if (target->health < -target->info->spawnhealth 
	&& target->info->xdeathstate)
    {
	P_SetMobjState (target, target->info->xdeathstate);
    }
    else
	P_SetMobjState (target, target->info->deathstate);
    target->tics -= P_Random()&3;

    if (target->tics < 1)
	target->tics = 1;
		
    //	I_StartSound (&actor->r, actor->info->deathsound);

    // In Chex Quest, monsters don't drop items.

    if (gameversion == exe_chex)
    {
        return;
    }

    // Drop stuff.
    // This determines the kind of object spawned
    // during the death frame of a thing.
    switch (target->type)
    {
        // villsa [STRIFE] TODO - fix me
      /*case MT_WOLFSS:
      case MT_POSSESSED:
	item = MT_CLIP;
	break;
	
      case MT_SHOTGUY:
	item = MT_SHOTGUN;
	break;
	
      case MT_CHAINGUY:
	item = MT_CHAINGUN;
	break;*/
	
      default:
	return;
    }

    mo = P_SpawnMobj (target->x,target->y,ONFLOORZ, item);
    mo->flags |= MF_DROPPED;	// special versions of items
}




//
// P_DamageMobj
// Damages both enemies and players
// "inflictor" is the thing that caused the damage
//  creature or missile, can be NULL (slime, etc)
// "source" is the thing to target after taking damage
//  creature or NULL
// Source and inflictor are the same for melee attacks.
// Source can be NULL for slime, barrel explosions
// and other environmental stuff.
//
void
P_DamageMobj
( mobj_t*	target,
  mobj_t*	inflictor,
  mobj_t*	source,
  int 		damage )
{
    unsigned	ang;
    int		saved;
    player_t*	player;
    fixed_t	thrust;
    int		temp;
	
    if ( !(target->flags & MF_SHOOTABLE) )
	return;	// shouldn't happen...
		
    if (target->health <= 0)
	return;

    // villsa [STRIFE] unused
    /*if ( target->flags & MF_SKULLFLY )
    {
	target->momx = target->momy = target->momz = 0;
    }*/
	
    player = target->player;
    if (player && gameskill == sk_baby)
	damage >>= 1; 	// take half damage in trainer mode
		

    // Some close combat weapons should not
    // inflict thrust and push the victim out of reach,
    // thus kick away unless using the chainsaw.
    if (inflictor
	&& !(target->flags & MF_NOCLIP)
	&& (!source
	    || !source->player
	    /*|| source->player->readyweapon != wp_chainsaw*/)) // villsa [STRIFE] unused
    {
	ang = R_PointToAngle2 ( inflictor->x,
				inflictor->y,
				target->x,
				target->y);
		
	thrust = damage*(FRACUNIT>>3)*100/target->info->mass;

	// make fall forwards sometimes
	if ( damage < 40
	     && damage > target->health
	     && target->z - inflictor->z > 64*FRACUNIT
	     && (P_Random ()&1) )
	{
	    ang += ANG180;
	    thrust *= 4;
	}
		
	ang >>= ANGLETOFINESHIFT;
	target->momx += FixedMul (thrust, finecosine[ang]);
	target->momy += FixedMul (thrust, finesine[ang]);
    }
    
    // player specific
    if (player)
    {
	// end of game hell hack
	if (target->subsector->sector->special == 11
	    && damage >= target->health)
	{
	    damage = target->health - 1;
	}
	

	// Below certain threshold,
	// ignore damage in GOD mode.
        // villsa [STRIFE] removed pw_invulnerability check
	if(damage < 1000 && (player->cheats & CF_GODMODE))
            return;
	
	if (player->armortype)
	{
	    if (player->armortype == 1)
		saved = damage/3;
	    else
		saved = damage/2;
	    
	    if (player->armorpoints <= saved)
	    {
		// armor is used up
		saved = player->armorpoints;
		player->armortype = 0;
	    }
	    player->armorpoints -= saved;
	    damage -= saved;
	}
	player->health -= damage; 	// mirror mobj health here for Dave
	if (player->health < 0)
	    player->health = 0;
	
	player->attacker = source;
	player->damagecount += damage;	// add damage after armor / invuln

	if (player->damagecount > 100)
	    player->damagecount = 100;	// teleport stomp does 10k points...
	
	temp = damage < 100 ? damage : 100;

	if (player == &players[consoleplayer])
	    I_Tactile (40,10,40+temp*2);
    }
    
    // do the damage	
    target->health -= damage;	
    if (target->health <= 0)
    {
	P_KillMobj (source, target);
	return;
    }

    if ( (P_Random () < target->info->painchance)
	 /*&& !(target->flags&MF_SKULLFLY)*/ )  // villsa [STRIFE] unused flag
    {
	target->flags |= MF_JUSTHIT;	// fight back!
	
	P_SetMobjState (target, target->info->painstate);
    }
			
    target->reactiontime = 0;		// we're awake now...	

    // villsa [STRIFE] TODO - update to strife version
    if ( (!target->threshold /*|| target->type == MT_VILE*/)
	 && source && source != target
	 /*&& source->type != MT_VILE*/)
    {
	// if not intent on another player,
	// chase after this one
	target->target = source;
	target->threshold = BASETHRESHOLD;
	if (target->state == &states[target->info->spawnstate]
	    && target->info->seestate != S_NULL)
	    P_SetMobjState (target, target->info->seestate);
    }
			
}

