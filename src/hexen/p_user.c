
//**************************************************************************
//**
//** p_user.c : Heretic 2 : Raven Software, Corp.
//**
//** $RCSfile: p_user.c,v $
//** $Revision: 1.123 $
//** $Date: 96/01/05 14:21:01 $
//** $Author: bgokey $
//**
//**************************************************************************

#include "h2def.h"
#include "p_local.h"
#include "soundst.h"

void P_PlayerNextArtifact(player_t *player);

// Macros

#define MAXBOB 0x100000 // 16 pixels of bob

// Data

boolean onground;
int newtorch; // used in the torch flicker effect.
int newtorchdelta;

int PStateNormal[NUMCLASSES] = 
{
	S_FPLAY,
	S_CPLAY,
	S_MPLAY,
	S_PIGPLAY
};

int PStateRun[NUMCLASSES] = 
{
	S_FPLAY_RUN1,
	S_CPLAY_RUN1,
	S_MPLAY_RUN1,
	S_PIGPLAY_RUN1
};

int PStateAttack[NUMCLASSES] = 
{
	S_FPLAY_ATK1,
	S_CPLAY_ATK1,
	S_MPLAY_ATK1,
	S_PIGPLAY_ATK1
};

int PStateAttackEnd[NUMCLASSES] = 
{
	S_FPLAY_ATK2,
	S_CPLAY_ATK3,
	S_MPLAY_ATK2,
	S_PIGPLAY_ATK1	
};

int ArmorMax[NUMCLASSES] = { 20, 18, 16, 1 };
/*
==================
=
= P_Thrust
=
= moves the given origin along a given angle
=
==================
*/

void P_Thrust(player_t *player, angle_t angle, fixed_t move)
{
	angle >>= ANGLETOFINESHIFT;
	if(player->powers[pw_flight] && !(player->mo->z <= player->mo->floorz))
	{
		player->mo->momx += FixedMul(move, finecosine[angle]);
		player->mo->momy += FixedMul(move, finesine[angle]);
	}
	else if(P_GetThingFloorType(player->mo) == FLOOR_ICE) // Friction_Low
	{
		player->mo->momx += FixedMul(move>>1, finecosine[angle]);
		player->mo->momy += FixedMul(move>>1, finesine[angle]);
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
=Calculate the walking / running height adjustment
=
==================
*/

void P_CalcHeight (player_t *player)
{
	int		angle;
	fixed_t	bob;

//
// regular movement bobbing (needs to be calculated for gun swing even
// if not on ground)
// OPTIMIZE: tablify angle

	player->bob = FixedMul (player->mo->momx, player->mo->momx)+
	FixedMul (player->mo->momy,player->mo->momy);
	player->bob >>= 2;
	if (player->bob>MAXBOB)
		player->bob = MAXBOB;
	if(player->mo->flags2&MF2_FLY && !onground)
	{
		player->bob = FRACUNIT/2;
	}	
	
	if ((player->cheats & CF_NOMOMENTUM))
	{
		player->viewz = player->mo->z + VIEWHEIGHT;
		if (player->viewz > player->mo->ceilingz-4*FRACUNIT)
			player->viewz = player->mo->ceilingz-4*FRACUNIT;
		player->viewz = player->mo->z + player->viewheight;
		return;
	}

	angle = (FINEANGLES/20*leveltime)&FINEMASK;
	bob = FixedMul ( player->bob/2, finesine[angle]);

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

	if(player->morphTics)
	{
		player->viewz = player->mo->z+player->viewheight-(20*FRACUNIT);
	}
	else
	{
		player->viewz = player->mo->z+player->viewheight+bob;
	}
	if(player->mo->floorclip && player->playerstate != PST_DEAD
		&& player->mo->z <= player->mo->floorz)
	{
		player->viewz -= player->mo->floorclip;
	}
	if(player->viewz > player->mo->ceilingz-4*FRACUNIT)
	{
		player->viewz = player->mo->ceilingz-4*FRACUNIT;
	}
	if(player->viewz < player->mo->floorz+4*FRACUNIT)
	{
		player->viewz = player->mo->floorz+4*FRACUNIT;
	}
}

/*
=================
=
= P_MovePlayer
=
=================
*/

void P_MovePlayer(player_t *player)
{
	int look;
	int fly;
	ticcmd_t *cmd;

	cmd = &player->cmd;
	player->mo->angle += (cmd->angleturn<<16);

	onground = (player->mo->z <= player->mo->floorz
		|| (player->mo->flags2&MF2_ONMOBJ));

	if(cmd->forwardmove)
	{
		if(onground || player->mo->flags2&MF2_FLY)
		{
			P_Thrust(player, player->mo->angle, cmd->forwardmove*2048);
		}
		else
		{
			P_Thrust(player, player->mo->angle, FRACUNIT>>8);
		}
	}	
	if(cmd->sidemove)
	{
		if(onground || player->mo->flags2&MF2_FLY)
			{
			P_Thrust(player, player->mo->angle-ANG90, cmd->sidemove*2048);
		}
		else
		{
			P_Thrust(player, player->mo->angle, FRACUNIT>>8);
		}
	}
	if(cmd->forwardmove || cmd->sidemove)
	{
		if(player->mo->state == &states[PStateNormal[player->class]])
		{
			P_SetMobjState(player->mo, PStateRun[player->class]);
		}
	}

	look = cmd->lookfly&15;
	if(look > 7)
	{
		look -= 16;
	}
	if(look)
	{
		if(look == TOCENTER)
		{
			player->centering = true;
		}
		else
		{
			player->lookdir += 5*look;
			if(player->lookdir > 90 || player->lookdir < -110)
			{
				player->lookdir -= 5*look;
			}
		}
	}
	if(player->centering)
	{
		if(player->lookdir > 0)
		{
			player->lookdir -= 8;
		}
		else if(player->lookdir < 0)
		{
			player->lookdir += 8;
		}
		if(abs(player->lookdir) < 8)
		{
			player->lookdir = 0;
			player->centering = false;
		}
	}
	fly = cmd->lookfly>>4;
	if(fly > 7)
	{
		fly -= 16;
	}
	if(fly && player->powers[pw_flight])
	{
		if(fly != TOCENTER)
		{
			player->flyheight = fly*2;
			if(!(player->mo->flags2&MF2_FLY))
			{
				player->mo->flags2 |= MF2_FLY;
				player->mo->flags |= MF_NOGRAVITY;
				if(player->mo->momz <= -39*FRACUNIT)
				{ // stop falling scream
					S_StopSound(player->mo);
				}
			}
		}
		else
		{
			player->mo->flags2 &= ~MF2_FLY;
			player->mo->flags &= ~MF_NOGRAVITY;
		}
	}
	else if(fly > 0)
	{
		P_PlayerUseArtifact(player, arti_fly);
	}
	if(player->mo->flags2&MF2_FLY)
	{
		player->mo->momz = player->flyheight*FRACUNIT;
		if(player->flyheight)
		{
			player->flyheight /= 2;
		}
	}
}

//==========================================================================
//
// P_DeathThink
//
//==========================================================================

void P_DeathThink(player_t *player)
{
	int dir;
	angle_t delta;
	int lookDelta;
	extern int inv_ptr;
	extern int curpos;

	P_MovePsprites(player);

	onground = (player->mo->z <= player->mo->floorz);
	if(player->mo->type == MT_BLOODYSKULL || player->mo->type == MT_ICECHUNK)
	{ // Flying bloody skull or flying ice chunk
		player->viewheight = 6*FRACUNIT;
		player->deltaviewheight = 0;
		//player->damagecount = 20;
		if(onground)
		{
			if(player->lookdir < 60)
			{
				lookDelta = (60-player->lookdir)/8;
				if(lookDelta < 1 && (leveltime&1))
				{
					lookDelta = 1;
				}
				else if(lookDelta > 6)
				{
					lookDelta = 6;
				}
				player->lookdir += lookDelta;
			}
		}
	}
	else if(!(player->mo->flags2&MF2_ICEDAMAGE))
	{ // Fall to ground (if not frozen)
		player->deltaviewheight = 0;
		if(player->viewheight > 6*FRACUNIT)
		{
			player->viewheight -= FRACUNIT;
		}
		if(player->viewheight < 6*FRACUNIT)
		{
			player->viewheight = 6*FRACUNIT;
		}
		if(player->lookdir > 0)
		{
			player->lookdir -= 6;
		}
		else if(player->lookdir < 0)
		{
			player->lookdir += 6;
		}
		if(abs(player->lookdir) < 6)
		{
			player->lookdir = 0;
		}
	}
	P_CalcHeight(player);

	if(player->attacker && player->attacker != player->mo)
	{ // Watch killer
		dir = P_FaceMobj(player->mo, player->attacker, &delta);
		if(delta < ANGLE_1*10)
		{ // Looking at killer, so fade damage and poison counters
			if(player->damagecount)
			{
				player->damagecount--;
			}
			if(player->poisoncount)
			{
				player->poisoncount--;
			}
		}
		delta = delta/8;
		if(delta > ANGLE_1*5)
		{
			delta = ANGLE_1*5;
		}
		if(dir)
		{ // Turn clockwise
			player->mo->angle += delta;
		}
		else
		{ // Turn counter clockwise
			player->mo->angle -= delta;
		}
	}
	else if(player->damagecount || player->poisoncount)
	{
		if(player->damagecount)
		{
			player->damagecount--;
		}
		else
		{
			player->poisoncount--;
		}
	}

	if(player->cmd.buttons&BT_USE)
	{
		if(player == &players[consoleplayer])
		{
			I_SetPalette((byte *)W_CacheLumpName("PLAYPAL", PU_CACHE));
			inv_ptr = 0;
			curpos = 0;
			newtorch = 0;
			newtorchdelta = 0;
		}
		player->playerstate = PST_REBORN;
		player->mo->special1 = player->class;
		if(player->mo->special1 > 2)
		{
			player->mo->special1 = 0;
		}
		// Let the mobj know the player has entered the reborn state.  Some
		// mobjs need to know when it's ok to remove themselves.
		player->mo->special2 = 666;
	}
}

//----------------------------------------------------------------------------
//
// PROC P_MorphPlayerThink
//
//----------------------------------------------------------------------------

void P_MorphPlayerThink(player_t *player)
{
	mobj_t *pmo;

	if(player->morphTics&15)
	{
		return;
	}
	pmo = player->mo;
	if(!(pmo->momx+pmo->momy) && P_Random() < 64)
	{ // Snout sniff
		P_SetPspriteNF(player, ps_weapon, S_SNOUTATK2);
		S_StartSound(pmo, SFX_PIG_ACTIVE1); // snort
		return;
	}
	if(P_Random() < 48)
	{
		if(P_Random() < 128)
		{
			S_StartSound(pmo, SFX_PIG_ACTIVE1);
		}
		else
		{
			S_StartSound(pmo, SFX_PIG_ACTIVE2);
		}
	}		
}

//----------------------------------------------------------------------------
//
// FUNC P_GetPlayerNum
//
//----------------------------------------------------------------------------

int P_GetPlayerNum(player_t *player)
{
	int i;

	for(i = 0; i < MAXPLAYERS; i++)
	{
		if(player == &players[i])
		{
			return(i);
		}
	}
	return(0);
}

//----------------------------------------------------------------------------
//
// FUNC P_UndoPlayerMorph
//
//----------------------------------------------------------------------------

boolean P_UndoPlayerMorph(player_t *player)
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
	int oldBeast;

	pmo = player->mo;
	x = pmo->x;
	y = pmo->y;
	z = pmo->z;
	angle = pmo->angle;
	weapon = pmo->special1;
	oldFlags = pmo->flags;
	oldFlags2 = pmo->flags2;
	oldBeast = pmo->type;
	P_SetMobjState(pmo, S_FREETARGMOBJ);
	playerNum = P_GetPlayerNum(player);
	switch(PlayerClass[playerNum])
	{
		case PCLASS_FIGHTER:
			mo = P_SpawnMobj(x, y, z, MT_PLAYER_FIGHTER);
			break;
		case PCLASS_CLERIC:
			mo = P_SpawnMobj(x, y, z, MT_PLAYER_CLERIC);
			break;
		case PCLASS_MAGE:
			mo = P_SpawnMobj(x, y, z, MT_PLAYER_MAGE);
			break;
		default:
			I_Error("P_UndoPlayerMorph:  Unknown player class %d\n", 
				player->class);
	}
	if(P_TestMobjLocation(mo) == false)
	{ // Didn't fit
		P_RemoveMobj(mo);
		mo = P_SpawnMobj(x, y, z, oldBeast);
		mo->angle = angle;
		mo->health = player->health;
		mo->special1 = weapon;
		mo->player = player;
		mo->flags = oldFlags;
		mo->flags2 = oldFlags2;
		player->mo = mo;
		player->morphTics = 2*35;
		return(false);
	}
	if(player->class == PCLASS_FIGHTER)
	{ 
		// The first type should be blue, and the third should be the
		// Fighter's original gold color
		if(playerNum == 0)
		{
			mo->flags |= 2<<MF_TRANSSHIFT;
		}
		else if(playerNum != 2)
		{
			mo->flags |= playerNum<<MF_TRANSSHIFT;
		}
	}
	else if(playerNum)
	{ // Set color translation bits for player sprites
		mo->flags |= playerNum<<MF_TRANSSHIFT;
	}
	mo->angle = angle;
	mo->player = player;
	mo->reactiontime = 18;
	if(oldFlags2&MF2_FLY)
	{
		mo->flags2 |= MF2_FLY;
		mo->flags |= MF_NOGRAVITY;
	}
	player->morphTics = 0;
	player->health = mo->health = MAXHEALTH;
	player->mo = mo;
	player->class = PlayerClass[playerNum];
	angle >>= ANGLETOFINESHIFT;
	fog = P_SpawnMobj(x+20*finecosine[angle],
		y+20*finesine[angle], z+TELEFOGHEIGHT, MT_TFOG);
	S_StartSound(fog, SFX_TELEPORT);
	P_PostMorphWeapon(player, weapon);
	return(true);
}


//----------------------------------------------------------------------------
//
// PROC P_PlayerThink
//
//----------------------------------------------------------------------------

void P_PlayerThink(player_t *player)
{
	ticcmd_t *cmd;
	weapontype_t newweapon;
	int floorType;		
	mobj_t *pmo;		
	
	// No-clip cheat
	if(player->cheats&CF_NOCLIP)
	{
		player->mo->flags |= MF_NOCLIP;
	}
	else
	{
		player->mo->flags &= ~MF_NOCLIP;
	}
	cmd = &player->cmd;
	if(player->mo->flags&MF_JUSTATTACKED)
	{ // Gauntlets attack auto forward motion
		cmd->angleturn = 0;
		cmd->forwardmove = 0xc800/512;
		cmd->sidemove = 0;
		player->mo->flags &= ~MF_JUSTATTACKED;
	}
// messageTics is above the rest of the counters so that messages will 
// 		go away, even in death.
	player->messageTics--; // Can go negative
	if(!player->messageTics || player->messageTics == -1)
	{ // Refresh the screen when a message goes away
		player->ultimateMessage = false; // clear out any chat messages.
		player->yellowMessage = false;
		if(player == &players[consoleplayer])
		{
			BorderTopRefresh = true;
		}
	}
	player->worldTimer++;
	if(player->playerstate == PST_DEAD)
	{
		P_DeathThink(player);
		return;
	}
	if(player->jumpTics)
	{
		player->jumpTics--;
	}
	if(player->morphTics)
	{
		P_MorphPlayerThink(player);
	}
	// Handle movement
	if(player->mo->reactiontime)
	{ // Player is frozen
		player->mo->reactiontime--;
	}
	else
	{
		P_MovePlayer(player);
		pmo = player->mo;
		if(player->powers[pw_speed] && !(leveltime&1)
			&& P_AproxDistance(pmo->momx, pmo->momy) > 12*FRACUNIT)
		{
			mobj_t *speedMo;
			int playerNum;

			speedMo = P_SpawnMobj(pmo->x, pmo->y, pmo->z, MT_PLAYER_SPEED);
			if(speedMo)
			{
				speedMo->angle = pmo->angle;
				playerNum = P_GetPlayerNum(player);
				if(player->class == PCLASS_FIGHTER)
				{ 
					// The first type should be blue, and the 
					// third should be the Fighter's original gold color
					if(playerNum == 0)
					{
						speedMo->flags |= 2<<MF_TRANSSHIFT;
					}
					else if(playerNum != 2)
					{
						speedMo->flags |= playerNum<<MF_TRANSSHIFT;
					}
				}
				else if(playerNum)
				{ // Set color translation bits for player sprites
					speedMo->flags |= playerNum<<MF_TRANSSHIFT;
				}
				speedMo->target = pmo;
				speedMo->special1 = player->class;
				if(speedMo->special1 > 2)
				{
					speedMo->special1 = 0;
				}
				speedMo->sprite = pmo->sprite;
				speedMo->floorclip = pmo->floorclip;
				if(player == &players[consoleplayer])
				{
					speedMo->flags2 |= MF2_DONTDRAW;
				}
			}
		}
	}
	P_CalcHeight(player);
	if(player->mo->subsector->sector->special)
	{
		P_PlayerInSpecialSector(player);
	}
	if((floorType = P_GetThingFloorType(player->mo)) != FLOOR_SOLID)
	{
		P_PlayerOnSpecialFlat(player, floorType);
	}
	switch(player->class)
	{
		case PCLASS_FIGHTER:
			if(player->mo->momz <= -35*FRACUNIT 
				&& player->mo->momz >= -40*FRACUNIT && !player->morphTics
				&& !S_GetSoundPlayingInfo(player->mo,
				SFX_PLAYER_FIGHTER_FALLING_SCREAM))
				{
					S_StartSound(player->mo, 
						SFX_PLAYER_FIGHTER_FALLING_SCREAM);
				}
			break;
		case PCLASS_CLERIC:
			if(player->mo->momz <= -35*FRACUNIT 
				&& player->mo->momz >= -40*FRACUNIT && !player->morphTics
				&& !S_GetSoundPlayingInfo(player->mo,
				SFX_PLAYER_CLERIC_FALLING_SCREAM))
				{
					S_StartSound(player->mo, 
						SFX_PLAYER_CLERIC_FALLING_SCREAM);
				}
			break;
		case PCLASS_MAGE:
			if(player->mo->momz <= -35*FRACUNIT 
				&& player->mo->momz >= -40*FRACUNIT && !player->morphTics
				&& !S_GetSoundPlayingInfo(player->mo,
				SFX_PLAYER_MAGE_FALLING_SCREAM))
				{
					S_StartSound(player->mo, 
						SFX_PLAYER_MAGE_FALLING_SCREAM);
				}
			break;
		default:
			break;
	}
	if(cmd->arti)
	{ // Use an artifact
		if((cmd->arti&AFLAG_JUMP) && onground && !player->jumpTics)
		{
			if(player->morphTics)
			{
				player->mo->momz = 6*FRACUNIT;
			}
			else 
			{
				player->mo->momz = 9*FRACUNIT;
			}
			player->mo->flags2 &= ~MF2_ONMOBJ;
			player->jumpTics = 18; 
		}
		else if(cmd->arti&AFLAG_SUICIDE)
		{
			P_DamageMobj(player->mo, NULL, NULL, 10000);
		}
		if(cmd->arti == NUMARTIFACTS)
		{ // use one of each artifact (except puzzle artifacts)
			int i;

			for(i = 1; i < arti_firstpuzzitem; i++)
			{
				P_PlayerUseArtifact(player, i);
			}
		}
		else
		{
			P_PlayerUseArtifact(player, cmd->arti&AFLAG_MASK);
		}
	}
	// Check for weapon change
	if(cmd->buttons&BT_SPECIAL)
	{ // A special event has no other buttons
		cmd->buttons = 0;
	}
	if(cmd->buttons&BT_CHANGE && !player->morphTics)
	{
		// The actual changing of the weapon is done when the weapon
		// psprite can do it (A_WeaponReady), so it doesn't happen in
		// the middle of an attack.
		newweapon = (cmd->buttons&BT_WEAPONMASK)>>BT_WEAPONSHIFT;
		if(player->weaponowned[newweapon]
			&& newweapon != player->readyweapon)
		{
			player->pendingweapon = newweapon;
		}
	}
	// Check for use
	if(cmd->buttons&BT_USE)
	{
		if(!player->usedown)
		{
			P_UseLines(player);
			player->usedown = true;
		}
	}
	else
	{
		player->usedown = false;
	}
	// Morph counter
	if(player->morphTics)
	{
		if(!--player->morphTics)
		{ // Attempt to undo the pig
			P_UndoPlayerMorph(player);
		}
	}
	// Cycle psprites
	P_MovePsprites(player);
	// Other Counters
	if(player->powers[pw_invulnerability])
	{
		if(player->class == PCLASS_CLERIC)
		{
			if(!(leveltime&7) && player->mo->flags&MF_SHADOW
				&& !(player->mo->flags2&MF2_DONTDRAW))
			{
				player->mo->flags &= ~MF_SHADOW;
				if(!(player->mo->flags&MF_ALTSHADOW))
				{
					player->mo->flags2 |= MF2_DONTDRAW|MF2_NONSHOOTABLE;
				}
			}
			if(!(leveltime&31))
			{
				if(player->mo->flags2&MF2_DONTDRAW)
				{
					if(!(player->mo->flags&MF_SHADOW))
					{
						player->mo->flags |= MF_SHADOW|MF_ALTSHADOW;
					}
					else
					{
						player->mo->flags2 &= ~(MF2_DONTDRAW|MF2_NONSHOOTABLE);
					}
				}
				else
				{
					player->mo->flags |= MF_SHADOW;
					player->mo->flags &= ~MF_ALTSHADOW;
				}
			}		
		}
		if(!(--player->powers[pw_invulnerability]))
		{
			player->mo->flags2 &= ~(MF2_INVULNERABLE|MF2_REFLECTIVE);
			if(player->class == PCLASS_CLERIC)
			{
				player->mo->flags2 &= ~(MF2_DONTDRAW|MF2_NONSHOOTABLE);
				player->mo->flags &= ~(MF_SHADOW|MF_ALTSHADOW);
			}
		}
	}
	if(player->powers[pw_minotaur])
	{
		player->powers[pw_minotaur]--;
	}
	if(player->powers[pw_infrared])
	{
		player->powers[pw_infrared]--;
	}
	if(player->powers[pw_flight] && netgame)
	{
		if(!--player->powers[pw_flight])
		{
			if(player->mo->z != player->mo->floorz)
			{
#ifdef __WATCOMC__
				if(!useexterndriver)
				{
					player->centering = true;
				}
#else
				player->centering = true;
#endif
			}
			player->mo->flags2 &= ~MF2_FLY;
			player->mo->flags &= ~MF_NOGRAVITY;
			BorderTopRefresh = true; //make sure the sprite's cleared out
		}
	}
	if(player->powers[pw_speed])
	{
		player->powers[pw_speed]--;
	}
	if(player->damagecount)
	{
		player->damagecount--;
	}
	if(player->bonuscount)
	{
		player->bonuscount--;
	}
	if(player->poisoncount && !(leveltime&15))
	{
		player->poisoncount -= 5;
		if(player->poisoncount < 0)
		{
			player->poisoncount = 0;
		}
		P_PoisonDamage(player, player->poisoner, 1, true); 
	}
	// Colormaps
//	if(player->powers[pw_invulnerability])
//	{
//		if(player->powers[pw_invulnerability] > BLINKTHRESHOLD
//			|| (player->powers[pw_invulnerability]&8))
//		{
//			player->fixedcolormap = INVERSECOLORMAP;
//		}
//		else
//		{
//			player->fixedcolormap = 0;
//		}
//	}
//	else 
	if(player->powers[pw_infrared])
	{
		if (player->powers[pw_infrared] <= BLINKTHRESHOLD)
		{
			if(player->powers[pw_infrared]&8)
			{
				player->fixedcolormap = 0;
			}
			else
			{
				player->fixedcolormap = 1;
			}
		}
		else if(!(leveltime&16) && player == &players[consoleplayer])
		{
			if(newtorch)
			{
				if(player->fixedcolormap+newtorchdelta > 7 
					|| player->fixedcolormap+newtorchdelta < 1
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
				newtorch = (M_Random()&7)+1;
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

void P_ArtiTele(player_t *player)
{
	int i;
	int selections;
	fixed_t destX;
	fixed_t destY;
	angle_t destAngle;

	if(deathmatch)
	{
		selections = deathmatch_p-deathmatchstarts;
		i = P_Random()%selections;
		destX = deathmatchstarts[i].x<<FRACBITS;
		destY = deathmatchstarts[i].y<<FRACBITS;
		destAngle = ANG45*(deathmatchstarts[i].angle/45);
	}
	else
	{
		destX = playerstarts[0][0].x<<FRACBITS;
		destY = playerstarts[0][0].y<<FRACBITS;
		destAngle = ANG45*(playerstarts[0][0].angle/45);
	}
	P_Teleport(player->mo, destX, destY, destAngle, true);
	if(player->morphTics)
	{ // Teleporting away will undo any morph effects (pig)
		P_UndoPlayerMorph(player);
	}
	//S_StartSound(NULL, sfx_wpnup); // Full volume laugh
}


//----------------------------------------------------------------------------
//
// PROC P_ArtiTeleportOther
//
//----------------------------------------------------------------------------

void P_ArtiTeleportOther(player_t *player)
{
	mobj_t *mo;

	mo=P_SpawnPlayerMissile(player->mo, MT_TELOTHER_FX1);
	if (mo)
	{
		mo->target = player->mo;
	}
}


void P_TeleportToPlayerStarts(mobj_t *victim)
{
	int i,selections=0;
	fixed_t destX,destY;
	angle_t destAngle;

	for (i=0;i<MAXPLAYERS;i++)
	{
	    if (!playeringame[i]) continue;
		selections++;
	}
	i = P_Random()%selections;
	destX = playerstarts[0][i].x<<FRACBITS;
	destY = playerstarts[0][i].y<<FRACBITS;
	destAngle = ANG45*(playerstarts[0][i].angle/45);
	P_Teleport(victim, destX, destY, destAngle, true);
	//S_StartSound(NULL, sfx_wpnup); // Full volume laugh
}

void P_TeleportToDeathmatchStarts(mobj_t *victim)
{
	int i,selections;
	fixed_t destX,destY;
	angle_t destAngle;

	selections = deathmatch_p-deathmatchstarts;
	if (selections)
	{
		i = P_Random()%selections;
		destX = deathmatchstarts[i].x<<FRACBITS;
		destY = deathmatchstarts[i].y<<FRACBITS;
		destAngle = ANG45*(deathmatchstarts[i].angle/45);
		P_Teleport(victim, destX, destY, destAngle, true);
		//S_StartSound(NULL, sfx_wpnup); // Full volume laugh
	}
	else
	{
	 	P_TeleportToPlayerStarts(victim);
	}
}



//----------------------------------------------------------------------------
//
// PROC P_TeleportOther
//
//----------------------------------------------------------------------------
void P_TeleportOther(mobj_t *victim)
{
	if (victim->player)
	{
		if (deathmatch)
			P_TeleportToDeathmatchStarts(victim);
		else
			P_TeleportToPlayerStarts(victim);
	}
	else
	{
		// If death action, run it upon teleport
		if (victim->flags&MF_COUNTKILL && victim->special)
		{
			P_RemoveMobjFromTIDList(victim);
			P_ExecuteLineSpecial(victim->special, victim->args,
					NULL, 0, victim);
			victim->special = 0;
		}

		// Send all monsters to deathmatch spots
		P_TeleportToDeathmatchStarts(victim);
	}
}



#define BLAST_RADIUS_DIST	255*FRACUNIT
#define BLAST_SPEED			20*FRACUNIT
#define BLAST_FULLSTRENGTH	255

void ResetBlasted(mobj_t *mo)
{
	mo->flags2 &= ~MF2_BLASTED;
	if (!(mo->flags&MF_ICECORPSE))
	{
		mo->flags2 &= ~MF2_SLIDE;
	}
}

void P_BlastMobj(mobj_t *source, mobj_t *victim, fixed_t strength)
{
	angle_t angle,ang;
	mobj_t *mo;
	fixed_t x,y,z;

	angle = R_PointToAngle2(source->x, source->y, victim->x, victim->y);
	angle >>= ANGLETOFINESHIFT;
	if (strength < BLAST_FULLSTRENGTH)
	{
		victim->momx = FixedMul(strength, finecosine[angle]);
		victim->momy = FixedMul(strength, finesine[angle]);
		if (victim->player)
		{
			// Players handled automatically
		}
		else
		{
			victim->flags2 |= MF2_SLIDE;
			victim->flags2 |= MF2_BLASTED;
		}
	}
	else		// full strength blast from artifact
	{
		if (victim->flags&MF_MISSILE)
		{
			switch(victim->type)
			{
				case MT_SORCBALL1:	// don't blast sorcerer balls
				case MT_SORCBALL2:
				case MT_SORCBALL3:
					return;
					break;
				case MT_MSTAFF_FX2:	// Reflect to originator
					victim->special1 = (int)victim->target;	
					victim->target = source;
					break;
				default:
					break;
			}
		}
		if (victim->type == MT_HOLY_FX)
		{
			if ((mobj_t *)(victim->special1) == source)
			{
				victim->special1 = (int)victim->target;	
				victim->target = source;
			}
		}
		victim->momx = FixedMul(BLAST_SPEED, finecosine[angle]);
		victim->momy = FixedMul(BLAST_SPEED, finesine[angle]);

		// Spawn blast puff
		ang = R_PointToAngle2(victim->x, victim->y, source->x, source->y);
		ang >>= ANGLETOFINESHIFT;
		x = victim->x + FixedMul(victim->radius+FRACUNIT, finecosine[ang]);
		y = victim->y + FixedMul(victim->radius+FRACUNIT, finesine[ang]);
		z = victim->z - victim->floorclip + (victim->height>>1);
		mo=P_SpawnMobj(x, y, z, MT_BLASTEFFECT);
		if (mo)
		{
			mo->momx = victim->momx;
			mo->momy = victim->momy;
		}

		if (victim->flags&MF_MISSILE)
		{
			victim->momz = 8*FRACUNIT;
			mo->momz = victim->momz;
		}
		else
		{
			victim->momz = (1000/victim->info->mass)<<FRACBITS;
		}
		if (victim->player)
		{
			// Players handled automatically
		}
		else
		{
			victim->flags2 |= MF2_SLIDE;
			victim->flags2 |= MF2_BLASTED;
		}
	}
}


// Blast all mobj things away
void P_BlastRadius(player_t *player)
{
	mobj_t *mo;
	mobj_t *pmo=player->mo;
	thinker_t *think;
	fixed_t dist;

	S_StartSound(pmo, SFX_ARTIFACT_BLAST);
	P_NoiseAlert(player->mo, player->mo);

	for(think = thinkercap.next; think != &thinkercap; think = think->next)
	{
		if(think->function != P_MobjThinker)
		{ // Not a mobj thinker
			continue;
		}
		mo = (mobj_t *)think;
		if((mo == pmo) || (mo->flags2&MF2_BOSS))
		{ // Not a valid monster
			continue;
		}
		if ((mo->type == MT_POISONCLOUD) ||		// poison cloud
			(mo->type == MT_HOLY_FX) ||			// holy fx
			(mo->flags&MF_ICECORPSE))			// frozen corpse
		{
			// Let these special cases go
		}
		else if ((mo->flags&MF_COUNTKILL) &&
			(mo->health <= 0))
		{
			continue;
		}
		else if (!(mo->flags&MF_COUNTKILL) &&
			!(mo->player) &&
			!(mo->flags&MF_MISSILE))
		{	// Must be monster, player, or missile
			continue;
		}
		if (mo->flags2&MF2_DORMANT)
		{
			continue;		// no dormant creatures
		}
		if ((mo->type == MT_WRAITHB) && (mo->flags2&MF2_DONTDRAW))
		{
			continue;		// no underground wraiths
		}
		if ((mo->type == MT_SPLASHBASE) ||
			(mo->type == MT_SPLASH))
		{
			continue;
		}
		if(mo->type == MT_SERPENT || mo->type == MT_SERPENTLEADER)
		{
			continue;
		}
		dist = P_AproxDistance(pmo->x - mo->x, pmo->y - mo->y);
		if(dist > BLAST_RADIUS_DIST)
		{ // Out of range
			continue;
		}
		P_BlastMobj(pmo, mo, BLAST_FULLSTRENGTH);
	}
}


#define HEAL_RADIUS_DIST	255*FRACUNIT

// Do class specific effect for everyone in radius
boolean P_HealRadius(player_t *player)
{
	mobj_t *mo;
	mobj_t *pmo=player->mo;
	thinker_t *think;
	fixed_t dist;
	int effective=false;
	int amount;

	for(think = thinkercap.next; think != &thinkercap; think = think->next)
	{
		if(think->function != P_MobjThinker)
		{ // Not a mobj thinker
			continue;
		}
		mo = (mobj_t *)think;

		if (!mo->player) continue;
		if (mo->health <= 0) continue;
		dist = P_AproxDistance(pmo->x - mo->x, pmo->y - mo->y);
		if(dist > HEAL_RADIUS_DIST)
		{ // Out of range
			continue;
		}

		switch(player->class)
		{
			case PCLASS_FIGHTER:		// Radius armor boost
				if ((P_GiveArmor(mo->player, ARMOR_ARMOR, 1)) ||
					(P_GiveArmor(mo->player, ARMOR_SHIELD, 1)) ||
					(P_GiveArmor(mo->player, ARMOR_HELMET, 1)) ||
					(P_GiveArmor(mo->player, ARMOR_AMULET, 1)))
				{
					effective=true;
					S_StartSound(mo, SFX_MYSTICINCANT);
				}
				break;
			case PCLASS_CLERIC:			// Radius heal
				amount = 50 + (P_Random()%50);
				if (P_GiveBody(mo->player, amount))
				{
					effective=true;
					S_StartSound(mo, SFX_MYSTICINCANT);
				}
				break;
			case PCLASS_MAGE:			// Radius mana boost
				amount = 50 + (P_Random()%50);
				if ((P_GiveMana(mo->player, MANA_1, amount)) ||
					(P_GiveMana(mo->player, MANA_2, amount)))
				{
					effective=true;
					S_StartSound(mo, SFX_MYSTICINCANT);
				}
				break;
			case PCLASS_PIG:
			default:
				break;
		}
	}
	return(effective);
}


//----------------------------------------------------------------------------
//
// PROC P_PlayerNextArtifact
//
//----------------------------------------------------------------------------

void P_PlayerNextArtifact(player_t *player)
{
	extern int inv_ptr;
	extern int curpos;

	if(player == &players[consoleplayer])
	{
		inv_ptr--;
		if(inv_ptr < 6)
		{
			curpos--;
			if(curpos < 0)
			{
				curpos = 0;
			}
		}
		if(inv_ptr < 0)
		{
			inv_ptr = player->inventorySlotNum-1;
			if(inv_ptr < 6)
			{
				curpos = inv_ptr;
			}
			else
			{
				curpos = 6;
			}
		}
		player->readyArtifact =
			player->inventory[inv_ptr].type;
	}
}

//----------------------------------------------------------------------------
//
// PROC P_PlayerRemoveArtifact
//
//----------------------------------------------------------------------------

void P_PlayerRemoveArtifact(player_t *player, int slot)
{
	int i;
	extern int inv_ptr;
	extern int curpos;

	player->artifactCount--;
	if(!(--player->inventory[slot].count))
	{ // Used last of a type - compact the artifact list
		player->readyArtifact = arti_none;
		player->inventory[slot].type = arti_none;
		for(i = slot+1; i < player->inventorySlotNum; i++)
		{
			player->inventory[i-1] = player->inventory[i];
		}
		player->inventorySlotNum--;
		if(player == &players[consoleplayer])
		{ // Set position markers and get next readyArtifact
			inv_ptr--;
			if(inv_ptr < 6)
			{
				curpos--;
				if(curpos < 0)
				{
					curpos = 0;
				}
			}
			if(inv_ptr >= player->inventorySlotNum)
			{
				inv_ptr = player->inventorySlotNum-1;
			}
			if(inv_ptr < 0)
			{
				inv_ptr = 0;
			}
			player->readyArtifact =
				player->inventory[inv_ptr].type;
		}
	}
}

//----------------------------------------------------------------------------
//
// PROC P_PlayerUseArtifact
//
//----------------------------------------------------------------------------

void P_PlayerUseArtifact(player_t *player, artitype_t arti)
{
	int i;

	for(i = 0; i < player->inventorySlotNum; i++)
	{
		if(player->inventory[i].type == arti)
		{ // Found match - try to use
			if(P_UseArtifact(player, arti))
			{ // Artifact was used - remove it from inventory
				P_PlayerRemoveArtifact(player, i);
				if(player == &players[consoleplayer])
				{
					if(arti < arti_firstpuzzitem)
					{
						S_StartSound(NULL, SFX_ARTIFACT_USE);
					}
					else
					{
						S_StartSound(NULL, SFX_PUZZLE_SUCCESS);
					}
					ArtifactFlash = 4;
				}
			}
			else if(arti < arti_firstpuzzitem)
			{ // Unable to use artifact, advance pointer
				P_PlayerNextArtifact(player);
			}
			break;
		}
	}
}

//==========================================================================
//
// P_UseArtifact
//
// Returns true if the artifact was used.
//
//==========================================================================

boolean P_UseArtifact(player_t *player, artitype_t arti)
{
	mobj_t *mo;
	angle_t angle;
	int i;
	int count;

	switch(arti)
	{
		case arti_invulnerability:
			if(!P_GivePower(player, pw_invulnerability))
			{
				return(false);
			}
			break;
		case arti_health:
			if(!P_GiveBody(player, 25))
			{
				return(false);
			}
			break;
		case arti_superhealth:
			if(!P_GiveBody(player, 100))
			{
				return(false);
			}
			break;
		case arti_healingradius:
			if (!P_HealRadius(player))
			{
				return(false);
			}
			break;
		case arti_torch:
			if(!P_GivePower(player, pw_infrared))
			{
				return(false);
			}
			break;
		case arti_egg:
			mo = player->mo;
			P_SpawnPlayerMissile(mo, MT_EGGFX);
			P_SPMAngle(mo, MT_EGGFX, mo->angle-(ANG45/6));
			P_SPMAngle(mo, MT_EGGFX, mo->angle+(ANG45/6));
			P_SPMAngle(mo, MT_EGGFX, mo->angle-(ANG45/3));
			P_SPMAngle(mo, MT_EGGFX, mo->angle+(ANG45/3));
			break;
		case arti_fly:
			if(!P_GivePower(player, pw_flight))
			{
				return(false);
			}
			if(player->mo->momz <= -35*FRACUNIT)
			{ // stop falling scream
				S_StopSound(player->mo);
			}
			break;
		case arti_summon:
			mo = P_SpawnPlayerMissile(player->mo, MT_SUMMON_FX);
			if (mo)
			{
				mo->target = player->mo;
				mo->special1 = (int)(player->mo);
				mo->momz = 5*FRACUNIT;
			}
			break;
		case arti_teleport:
			P_ArtiTele(player);
			break;
		case arti_teleportother:
			P_ArtiTeleportOther(player);
			break;
		case arti_poisonbag:
			angle = player->mo->angle>>ANGLETOFINESHIFT;
			if(player->class == PCLASS_CLERIC)
			{
				mo = P_SpawnMobj(player->mo->x+16*finecosine[angle],
					player->mo->y+24*finesine[angle], player->mo->z-
					player->mo->floorclip+8*FRACUNIT, MT_POISONBAG);
				if(mo)
				{
					mo->target = player->mo;
				}
			}
			else if(player->class == PCLASS_MAGE)
			{
				mo = P_SpawnMobj(player->mo->x+16*finecosine[angle],
					player->mo->y+24*finesine[angle], player->mo->z-
					player->mo->floorclip+8*FRACUNIT, MT_FIREBOMB);
				if(mo)
				{
					mo->target = player->mo;
				}
			}			
			else // PCLASS_FIGHTER, obviously (also pig, not so obviously)
			{
				mo = P_SpawnMobj(player->mo->x, player->mo->y, 
					player->mo->z-player->mo->floorclip+35*FRACUNIT,
					MT_THROWINGBOMB);
				if(mo)
				{
					mo->angle = player->mo->angle+(((P_Random()&7)-4)<<24);
					mo->momz = 4*FRACUNIT+((player->lookdir)<<(FRACBITS-4));
					mo->z += player->lookdir<<(FRACBITS-4);
					P_ThrustMobj(mo, mo->angle, mo->info->speed);
					mo->momx += player->mo->momx>>1;
					mo->momy += player->mo->momy>>1;
					mo->target = player->mo;
					mo->tics -= P_Random()&3;
					P_CheckMissileSpawn(mo);											
				} 
			}
			break;
		case arti_speed:
			if(!P_GivePower(player, pw_speed))
			{
				return(false);
			}
			break;
		case arti_boostmana:
			if(!P_GiveMana(player, MANA_1, MAX_MANA))
			{
				if(!P_GiveMana(player, MANA_2, MAX_MANA))
				{
					return false;
				}
				
			}
			else 
			{
				P_GiveMana(player, MANA_2, MAX_MANA);
			}
			break;
		case arti_boostarmor:
			count = 0;

			for(i = 0; i < NUMARMOR; i++)
			{
				count += P_GiveArmor(player, i, 1); // 1 point per armor type
			}
			if(!count)
			{
				return false;
			}
			break;
		case arti_blastradius:
			P_BlastRadius(player);
			break;

		case arti_puzzskull:
		case arti_puzzgembig:
		case arti_puzzgemred:
		case arti_puzzgemgreen1:
		case arti_puzzgemgreen2:
		case arti_puzzgemblue1:
		case arti_puzzgemblue2:
		case arti_puzzbook1:
		case arti_puzzbook2:
		case arti_puzzskull2:
		case arti_puzzfweapon:
		case arti_puzzcweapon:
		case arti_puzzmweapon:
		case arti_puzzgear1:
		case arti_puzzgear2:
		case arti_puzzgear3:
		case arti_puzzgear4:
			if(P_UsePuzzleItem(player, arti-arti_firstpuzzitem))
			{
				return true;
			}
			else
			{
				P_SetYellowMessage(player, TXT_USEPUZZLEFAILED, false);
				return false;
			}
			break;
		default:
			return false;
	}
	return true;
}

//============================================================================
//
// A_SpeedFade
//
//============================================================================

void A_SpeedFade(mobj_t *actor)
{
	actor->flags |= MF_SHADOW;
	actor->flags &= ~MF_ALTSHADOW;
	actor->sprite = actor->target->sprite;
}
