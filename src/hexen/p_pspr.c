
//**************************************************************************
//**
//** p_pspr.c : Heretic 2 : Raven Software, Corp.
//**
//** $RCSfile: p_pspr.c,v $
//** $Revision: 1.105 $
//** $Date: 96/01/06 03:23:35 $
//** $Author: bgokey $
//**
//**************************************************************************

// HEADER FILES ------------------------------------------------------------

#include "h2def.h"
#include "p_local.h"
#include "soundst.h"

// MACROS ------------------------------------------------------------------

#define LOWERSPEED FRACUNIT*6
#define RAISESPEED FRACUNIT*6
#define WEAPONBOTTOM 128*FRACUNIT
#define WEAPONTOP 32*FRACUNIT

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

extern void P_ExplodeMissile(mobj_t *mo);
extern void A_UnHideThing(mobj_t *actor);

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

extern fixed_t FloatBobOffsets[64];

// PUBLIC DATA DEFINITIONS -------------------------------------------------

fixed_t bulletslope;

weaponinfo_t WeaponInfo[NUMWEAPONS][NUMCLASSES] =
{
	{ // First Weapons
		{ // Fighter First Weapon - Punch
			MANA_NONE,			// mana
			S_PUNCHUP,			// upstate
			S_PUNCHDOWN,		// downstate
			S_PUNCHREADY,		// readystate
			S_PUNCHATK1_1,		// atkstate
			S_PUNCHATK1_1,		// holdatkstate
			S_NULL				// flashstate
		},
		{ // Cleric First Weapon - Mace
			MANA_NONE,			// mana
			S_CMACEUP,			// upstate
			S_CMACEDOWN,		// downstate
			S_CMACEREADY,		// readystate
			S_CMACEATK_1,		// atkstate
			S_CMACEATK_1,		// holdatkstate
			S_NULL				// flashstate
		},
		{ // Mage First Weapon - Wand
			MANA_NONE,
			S_MWANDUP,
			S_MWANDDOWN,
			S_MWANDREADY,
			S_MWANDATK_1,
			S_MWANDATK_1,
			S_NULL
		},
		{ // Pig - Snout
			MANA_NONE,			// mana
			S_SNOUTUP,			// upstate
			S_SNOUTDOWN,		// downstate
			S_SNOUTREADY,		// readystate
			S_SNOUTATK1,		// atkstate
			S_SNOUTATK1,		// holdatkstate
			S_NULL				// flashstate
		}
	},
	{ // Second Weapons
		{ // Fighter - Axe
			MANA_NONE,			// mana
			S_FAXEUP,			// upstate
			S_FAXEDOWN,			// downstate
			S_FAXEREADY,		// readystate
			S_FAXEATK_1,		// atkstate
			S_FAXEATK_1,		// holdatkstate
			S_NULL				// flashstate
		},
		{ // Cleric - Serpent Staff
			MANA_1,			// mana
			S_CSTAFFUP,		// upstate
			S_CSTAFFDOWN,	// downstate
			S_CSTAFFREADY,	// readystate
			S_CSTAFFATK_1,	// atkstate
			S_CSTAFFATK_1,	// holdatkstate
			S_NULL			// flashstate
		},
		{ // Mage - Cone of shards
			MANA_1,			// mana
			S_CONEUP,		// upstate
			S_CONEDOWN,		// downstate
			S_CONEREADY,	// readystate
			S_CONEATK1_1,	// atkstate
			S_CONEATK1_3,	// holdatkstate
			S_NULL			// flashstate
		},
		{ // Pig - Snout
			MANA_NONE,			// mana
			S_SNOUTUP,			// upstate
			S_SNOUTDOWN,		// downstate
			S_SNOUTREADY,		// readystate
			S_SNOUTATK1,		// atkstate
			S_SNOUTATK1,		// holdatkstate
			S_NULL				// flashstate
		}
	},
	{ // Third Weapons
		{ // Fighter - Hammer
			MANA_NONE,			// mana
			S_FHAMMERUP,		// upstate
			S_FHAMMERDOWN,		// downstate
			S_FHAMMERREADY,		// readystate
			S_FHAMMERATK_1,		// atkstate
			S_FHAMMERATK_1,		// holdatkstate
			S_NULL				// flashstate
		},
		{ // Cleric - Flame Strike
			MANA_2,				// mana
			S_CFLAMEUP,			// upstate
			S_CFLAMEDOWN,		// downstate
			S_CFLAMEREADY1,		// readystate
			S_CFLAMEATK_1,		// atkstate
			S_CFLAMEATK_1,		// holdatkstate
			S_NULL				// flashstate
		},
		{ // Mage - Lightning
			MANA_2,		// mana
			S_MLIGHTNINGUP,		// upstate
			S_MLIGHTNINGDOWN,	// downstate
			S_MLIGHTNINGREADY,	// readystate
			S_MLIGHTNINGATK_1,	// atkstate
			S_MLIGHTNINGATK_1,	// holdatkstate
			S_NULL				// flashstate
		},
		{ // Pig - Snout
			MANA_NONE,			// mana
			S_SNOUTUP,			// upstate
			S_SNOUTDOWN,		// downstate
			S_SNOUTREADY,		// readystate
			S_SNOUTATK1,		// atkstate
			S_SNOUTATK1,		// holdatkstate
			S_NULL				// flashstate
		}
	},
	{ // Fourth Weapons
		{ // Fighter - Rune Sword
			MANA_BOTH,			// mana
			S_FSWORDUP,			// upstate
			S_FSWORDDOWN,		// downstate
			S_FSWORDREADY,		// readystate
			S_FSWORDATK_1,		// atkstate
			S_FSWORDATK_1,		// holdatkstate
			S_NULL				// flashstate
		},
		{ // Cleric - Holy Symbol
			MANA_BOTH,			// mana
			S_CHOLYUP,		// upstate
			S_CHOLYDOWN,		// downstate
			S_CHOLYREADY,		// readystate
			S_CHOLYATK_1,		// atkstate
			S_CHOLYATK_1,		// holdatkstate
			S_NULL				// flashstate
		},
		{ // Mage - Staff
			MANA_BOTH,		// mana
			S_MSTAFFUP,		// upstate
			S_MSTAFFDOWN,		// downstate
			S_MSTAFFREADY,		// readystate
			S_MSTAFFATK_1,	// atkstate
			S_MSTAFFATK_1,	// holdatkstate
			S_NULL				// flashstate
		},
		{ // Pig - Snout
			MANA_NONE,			// mana
			S_SNOUTUP,			// upstate
			S_SNOUTDOWN,		// downstate
			S_SNOUTREADY,		// readystate
			S_SNOUTATK1,		// atkstate
			S_SNOUTATK1,		// holdatkstate
			S_NULL				// flashstate
		}
	}
};

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static int WeaponManaUse[NUMCLASSES][NUMWEAPONS] = 
{
	{ 0, 2, 3, 14 },
	{ 0, 1, 4, 18 },
	{ 0, 3, 5, 15 },
	{ 0, 0, 0, 0 }
};

// CODE --------------------------------------------------------------------

//---------------------------------------------------------------------------
//
// PROC P_SetPsprite
//
//---------------------------------------------------------------------------

void P_SetPsprite(player_t *player, int position, statenum_t stnum)
{
	pspdef_t *psp;
	state_t *state;

	psp = &player->psprites[position];
	do
	{
		if(!stnum)
		{ // Object removed itself.
			psp->state = NULL;
			break;
		}
		state = &states[stnum];
		psp->state = state;
		psp->tics = state->tics; // could be 0
		if(state->misc1)
		{ // Set coordinates.
			psp->sx = state->misc1<<FRACBITS;
		}
		if(state->misc2)
		{
			psp->sy = state->misc2<<FRACBITS;
		}
		if(state->action)
		{ // Call action routine.
			state->action(player, psp);
			if(!psp->state)
			{
				break;
			}
		}
		stnum = psp->state->nextstate;
	} while(!psp->tics); // An initial state of 0 could cycle through.
}

//---------------------------------------------------------------------------
//
// PROC P_SetPspriteNF
//
// Identical to P_SetPsprite, without calling the action function
//---------------------------------------------------------------------------

void P_SetPspriteNF(player_t *player, int position, statenum_t stnum)
{
	pspdef_t *psp;
	state_t *state;

	psp = &player->psprites[position];
	do
	{
		if(!stnum)
		{ // Object removed itself.
			psp->state = NULL;
			break;
		}
		state = &states[stnum];
		psp->state = state;
		psp->tics = state->tics; // could be 0
		if(state->misc1)
		{ // Set coordinates.
			psp->sx = state->misc1<<FRACBITS;
		}
		if(state->misc2)
		{
			psp->sy = state->misc2<<FRACBITS;
		}
		stnum = psp->state->nextstate;
	} while(!psp->tics); // An initial state of 0 could cycle through.
}

/*
=================
=
= P_CalcSwing
=
=================
*/

/*
fixed_t	swingx, swingy;
void P_CalcSwing (player_t *player)
{
	fixed_t	swing;
	int		angle;

// OPTIMIZE: tablify this

	swing = player->bob;

	angle = (FINEANGLES/70*leveltime)&FINEMASK;
	swingx = FixedMul ( swing, finesine[angle]);

	angle = (FINEANGLES/70*leveltime+FINEANGLES/2)&FINEMASK;
	swingy = -FixedMul ( swingx, finesine[angle]);
}
*/

//---------------------------------------------------------------------------
//
// PROC P_ActivateMorphWeapon
//
//---------------------------------------------------------------------------

void P_ActivateMorphWeapon(player_t *player)
{
	player->pendingweapon = WP_NOCHANGE;
	player->psprites[ps_weapon].sy = WEAPONTOP;
	player->readyweapon = WP_FIRST;	// Snout is the first weapon
	P_SetPsprite(player, ps_weapon, S_SNOUTREADY);
}


//---------------------------------------------------------------------------
//
// PROC P_PostMorphWeapon
//
//---------------------------------------------------------------------------

void P_PostMorphWeapon(player_t *player, weapontype_t weapon)
{
	player->pendingweapon = WP_NOCHANGE;
	player->readyweapon = weapon;
	player->psprites[ps_weapon].sy = WEAPONBOTTOM;
	P_SetPsprite(player, ps_weapon, WeaponInfo[weapon][player->class].upstate);
}

//---------------------------------------------------------------------------
//
// PROC P_BringUpWeapon
//
// Starts bringing the pending weapon up from the bottom of the screen.
//
//---------------------------------------------------------------------------

void P_BringUpWeapon(player_t *player)
{
	statenum_t new;

	if(player->pendingweapon == WP_NOCHANGE)
	{
		player->pendingweapon = player->readyweapon;
	}
	if(player->class == PCLASS_FIGHTER && player->pendingweapon == WP_SECOND
	&& player->mana[MANA_1])
	{
		new = S_FAXEUP_G;
	}
	else
	{
		new = WeaponInfo[player->pendingweapon][player->class].upstate;
	}
	player->pendingweapon = WP_NOCHANGE;
	player->psprites[ps_weapon].sy = WEAPONBOTTOM;
	P_SetPsprite(player, ps_weapon, new);
}

//---------------------------------------------------------------------------
//
// FUNC P_CheckMana
//
// Returns true if there is enough mana to shoot.  If not, selects the
// next weapon to use.
//
//---------------------------------------------------------------------------

boolean P_CheckMana(player_t *player)
{
	manatype_t mana;
	int count;

	mana = WeaponInfo[player->readyweapon][player->class].mana;
	count = WeaponManaUse[player->class][player->readyweapon];
	if(mana == MANA_BOTH)
	{
		if(player->mana[MANA_1] >= count && player->mana[MANA_2] >= count)
		{
			return true;
		}
	}
	else if(mana == MANA_NONE || player->mana[mana] >= count)
	{
		return(true);
	}
	// out of mana, pick a weapon to change to
	do
	{
		if(player->weaponowned[WP_THIRD]
			&& player->mana[MANA_2] >= WeaponManaUse[player->class][WP_THIRD])
		{
			player->pendingweapon = WP_THIRD;
		}
		else if(player->weaponowned[WP_SECOND]
			&& player->mana[MANA_1] >= WeaponManaUse[player->class][WP_SECOND])
		{
			player->pendingweapon = WP_SECOND;
		}
		else if(player->weaponowned[WP_FOURTH]
			&& player->mana[MANA_1] >= WeaponManaUse[player->class][WP_FOURTH]
			&& player->mana[MANA_2] >= WeaponManaUse[player->class][WP_FOURTH])
		{
			player->pendingweapon = WP_FOURTH;
		}
		else
		{
			player->pendingweapon = WP_FIRST;
		}
	} while(player->pendingweapon == WP_NOCHANGE);
	P_SetPsprite(player, ps_weapon,
		WeaponInfo[player->readyweapon][player->class].downstate);
	return(false);
}

//---------------------------------------------------------------------------
//
// PROC P_FireWeapon
//
//---------------------------------------------------------------------------

void P_FireWeapon(player_t *player)
{
	statenum_t attackState;

	if(!P_CheckMana(player))
	{
		return;
	}
	P_SetMobjState(player->mo, PStateAttack[player->class]); // S_PLAY_ATK1);
	if(player->class == PCLASS_FIGHTER && player->readyweapon == WP_SECOND
	&& player->mana[MANA_1] > 0)
	{ // Glowing axe
		attackState = S_FAXEATK_G1;
	}
	else
	{
		attackState = player->refire ? 
			WeaponInfo[player->readyweapon][player->class].holdatkstate
			: WeaponInfo[player->readyweapon][player->class].atkstate;
	}
	P_SetPsprite(player, ps_weapon, attackState);
	P_NoiseAlert(player->mo, player->mo);
}

//---------------------------------------------------------------------------
//
// PROC P_DropWeapon
//
// The player died, so put the weapon away.
//
//---------------------------------------------------------------------------

void P_DropWeapon(player_t *player)
{
	P_SetPsprite(player, ps_weapon,
		WeaponInfo[player->readyweapon][player->class].downstate);
}

//---------------------------------------------------------------------------
//
// PROC A_WeaponReady
//
// The player can fire the weapon or change to another weapon at this time.
//
//---------------------------------------------------------------------------

void A_WeaponReady(player_t *player, pspdef_t *psp)
{
	int angle;

	// Change player from attack state
	if(player->mo->state >= &states[PStateAttack[player->class]]
		&& player->mo->state <= &states[PStateAttackEnd[player->class]])
	{
		P_SetMobjState(player->mo, PStateNormal[player->class]);
	}
	// Put the weapon away if the player has a pending weapon or has
	// died.
	if(player->pendingweapon != WP_NOCHANGE || !player->health)
	{
		P_SetPsprite(player, ps_weapon,
			WeaponInfo[player->readyweapon][player->class].downstate);
		return;
	}

	// Check for fire. 
	if(player->cmd.buttons&BT_ATTACK)
	{
		player->attackdown = true;
		P_FireWeapon(player);
		return;
	}
	else
	{
		player->attackdown = false;
	}

	if(!player->morphTics)
	{
		// Bob the weapon based on movement speed.
		angle = (128*leveltime)&FINEMASK;
		psp->sx = FRACUNIT+FixedMul(player->bob, finecosine[angle]);
		angle &= FINEANGLES/2-1;
		psp->sy = WEAPONTOP+FixedMul(player->bob, finesine[angle]);
	}
}

//---------------------------------------------------------------------------
//
// PROC A_ReFire
//
// The player can re fire the weapon without lowering it entirely.
//
//---------------------------------------------------------------------------

void A_ReFire(player_t *player, pspdef_t *psp)
{
	if((player->cmd.buttons&BT_ATTACK)
		&& player->pendingweapon == WP_NOCHANGE && player->health)
	{
		player->refire++;
		P_FireWeapon(player);
	}
	else
	{
		player->refire = 0;
		P_CheckMana(player);
	}
}

//---------------------------------------------------------------------------
//
// PROC A_Lower
//
//---------------------------------------------------------------------------

void A_Lower(player_t *player, pspdef_t *psp)
{
	if(player->morphTics)
	{
		psp->sy = WEAPONBOTTOM;
	}
	else
	{
		psp->sy += LOWERSPEED;
	}
	if(psp->sy < WEAPONBOTTOM)
	{ // Not lowered all the way yet
		return;
	}
	if(player->playerstate == PST_DEAD)
	{ // Player is dead, so don't bring up a pending weapon
		psp->sy = WEAPONBOTTOM;
		return;
	}
	if(!player->health)
	{ // Player is dead, so keep the weapon off screen
		P_SetPsprite(player,  ps_weapon, S_NULL);
		return;
	}
	player->readyweapon = player->pendingweapon;
	P_BringUpWeapon(player);
}

//---------------------------------------------------------------------------
//
// PROC A_Raise
//
//---------------------------------------------------------------------------

void A_Raise(player_t *player, pspdef_t *psp)
{
	psp->sy -= RAISESPEED;
	if(psp->sy > WEAPONTOP)
	{ // Not raised all the way yet
		return;
	}
	psp->sy = WEAPONTOP;
	if(player->class == PCLASS_FIGHTER && player->readyweapon == WP_SECOND
	&& player->mana[MANA_1])
	{
		P_SetPsprite(player, ps_weapon, S_FAXEREADY_G);
	}
	else
	{	
		P_SetPsprite(player, ps_weapon,
			WeaponInfo[player->readyweapon][player->class].readystate);
	}
}

/*
===============
=
= P_BulletSlope
=
= Sets a slope so a near miss is at aproximately the height of the
= intended target
=
===============
*/

/*
void P_BulletSlope (mobj_t *mo)
{
	angle_t		an;

//
// see which target is to be aimed at
//
	an = mo->angle;
	bulletslope = P_AimLineAttack (mo, an, 16*64*FRACUNIT);
	if (!linetarget)
	{
		an += 1<<26;
		bulletslope = P_AimLineAttack (mo, an, 16*64*FRACUNIT);
		if (!linetarget)
		{
			an -= 2<<26;
			bulletslope = P_AimLineAttack (mo, an, 16*64*FRACUNIT);
		}
		if (!linetarget)
		{
			an += 1<<26;
			bulletslope = (mo->player->lookdir<<FRACBITS)/173;
		}
	}
}
*/

//****************************************************************************
//
// WEAPON ATTACKS
//
//****************************************************************************

//============================================================================
//
//	AdjustPlayerAngle
//
//============================================================================

#define MAX_ANGLE_ADJUST (5*ANGLE_1)

void AdjustPlayerAngle(mobj_t *pmo)
{
	angle_t angle;
	int difference;

	angle = R_PointToAngle2(pmo->x, pmo->y, linetarget->x, linetarget->y);
	difference = (int)angle-(int)pmo->angle;
	if(abs(difference) > MAX_ANGLE_ADJUST)
	{
		pmo->angle += difference > 0 ? MAX_ANGLE_ADJUST : -MAX_ANGLE_ADJUST;
	}
	else
	{
		pmo->angle = angle;
	}
}

//============================================================================
//
// A_SnoutAttack
//
//============================================================================

void A_SnoutAttack(player_t *player, pspdef_t *psp)
{
	angle_t angle;
	int damage;
	int slope;

	damage = 3+(P_Random()&3);
	angle = player->mo->angle;
	slope = P_AimLineAttack(player->mo, angle, MELEERANGE);
	PuffType = MT_SNOUTPUFF;
	PuffSpawned = NULL;
	P_LineAttack(player->mo, angle, MELEERANGE, slope, damage);
	S_StartSound(player->mo, SFX_PIG_ACTIVE1+(P_Random()&1));
	if(linetarget)
	{
		AdjustPlayerAngle(player->mo);
//		player->mo->angle = R_PointToAngle2(player->mo->x,
//			player->mo->y, linetarget->x, linetarget->y);
		if(PuffSpawned)
		{ // Bit something
			S_StartSound(player->mo, SFX_PIG_ATTACK);
		}
	}
}

//============================================================================
//
// A_FHammerAttack
//
//============================================================================

#define HAMMER_RANGE	(MELEERANGE+MELEERANGE/2)

void A_FHammerAttack(player_t *player, pspdef_t *psp)
{
	angle_t angle;
	mobj_t *pmo=player->mo;
	int damage;
	fixed_t power;
	int slope;
	int i;

	damage = 60+(P_Random()&63);
	power = 10*FRACUNIT;
	PuffType = MT_HAMMERPUFF;
	for(i = 0; i < 16; i++)
	{
		angle = pmo->angle+i*(ANG45/32);
		slope = P_AimLineAttack(pmo, angle, HAMMER_RANGE);
		if(linetarget)
		{
			P_LineAttack(pmo, angle, HAMMER_RANGE, slope, damage);
			AdjustPlayerAngle(pmo);
			if (linetarget->flags&MF_COUNTKILL || linetarget->player)
			{
				P_ThrustMobj(linetarget, angle, power);
			}
			pmo->special1 = false; // Don't throw a hammer
			goto hammerdone;
		}
		angle = pmo->angle-i*(ANG45/32);
		slope = P_AimLineAttack(pmo, angle, HAMMER_RANGE);
		if(linetarget)
		{
			P_LineAttack(pmo, angle, HAMMER_RANGE, slope, damage);
			AdjustPlayerAngle(pmo);
			if (linetarget->flags&MF_COUNTKILL || linetarget->player)
			{
				P_ThrustMobj(linetarget, angle, power);
			}
			pmo->special1 = false; // Don't throw a hammer
			goto hammerdone;
		}
	}
	// didn't find any targets in meleerange, so set to throw out a hammer
	PuffSpawned = NULL;
	angle = pmo->angle;
	slope = P_AimLineAttack(pmo, angle, HAMMER_RANGE);
	P_LineAttack(pmo, angle, HAMMER_RANGE, slope, damage);
	if(PuffSpawned)
	{
		pmo->special1 = false;
	}
	else
	{
		pmo->special1 = true;
	}
hammerdone:
	if(player->mana[MANA_2] < 
		WeaponManaUse[player->class][player->readyweapon])
	{ // Don't spawn a hammer if the player doesn't have enough mana
		pmo->special1 = false;
	}
	return;		
}

//============================================================================
//
// A_FHammerThrow
//
//============================================================================

void A_FHammerThrow(player_t *player, pspdef_t *psp)
{
	mobj_t *mo;

	if(!player->mo->special1)
	{
		return;
	}
	player->mana[MANA_2] -= WeaponManaUse[player->class][player->readyweapon];
	mo = P_SpawnPlayerMissile(player->mo, MT_HAMMER_MISSILE); 
	if(mo)
	{
		mo->special1 = 0;
	}	
}

//============================================================================
//
// A_FSwordAttack
//
//============================================================================

void A_FSwordAttack(player_t *player, pspdef_t *psp)
{
	mobj_t *pmo;

	player->mana[MANA_1] -= WeaponManaUse[player->class][player->readyweapon];
	player->mana[MANA_2] -= WeaponManaUse[player->class][player->readyweapon];
	pmo = player->mo;
	P_SPMAngleXYZ(pmo, pmo->x, pmo->y, pmo->z-10*FRACUNIT, MT_FSWORD_MISSILE, 
		pmo->angle+ANG45/4);
	P_SPMAngleXYZ(pmo, pmo->x, pmo->y, pmo->z-5*FRACUNIT, MT_FSWORD_MISSILE, 
		pmo->angle+ANG45/8);
	P_SPMAngleXYZ(pmo, pmo->x, pmo->y, pmo->z, MT_FSWORD_MISSILE, pmo->angle);
	P_SPMAngleXYZ(pmo, pmo->x, pmo->y, pmo->z+5*FRACUNIT, MT_FSWORD_MISSILE, 
		pmo->angle-ANG45/8);
	P_SPMAngleXYZ(pmo, pmo->x, pmo->y, pmo->z+10*FRACUNIT, MT_FSWORD_MISSILE, 
		pmo->angle-ANG45/4);
	S_StartSound(pmo, SFX_FIGHTER_SWORD_FIRE);
}

//============================================================================
//
// A_FSwordAttack2
//
//============================================================================

void A_FSwordAttack2(mobj_t *actor)
{
	angle_t angle = actor->angle;

	P_SpawnMissileAngle(actor, MT_FSWORD_MISSILE,angle+ANG45/4, 0);
	P_SpawnMissileAngle(actor, MT_FSWORD_MISSILE,angle+ANG45/8, 0);
	P_SpawnMissileAngle(actor, MT_FSWORD_MISSILE,angle,         0);
	P_SpawnMissileAngle(actor, MT_FSWORD_MISSILE,angle-ANG45/8, 0);
	P_SpawnMissileAngle(actor, MT_FSWORD_MISSILE,angle-ANG45/4, 0);
	S_StartSound(actor, SFX_FIGHTER_SWORD_FIRE);
}

//============================================================================
//
// A_FSwordFlames
//
//============================================================================

void A_FSwordFlames(mobj_t *actor)
{
	int i;

	for(i = 1+(P_Random()&3); i; i--)
	{
		P_SpawnMobj(actor->x+((P_Random()-128)<<12), actor->y
			+((P_Random()-128)<<12), actor->z+((P_Random()-128)<<11),
			MT_FSWORD_FLAME);
	}
}

//============================================================================
//
// A_MWandAttack
//
//============================================================================

void A_MWandAttack(player_t *player, pspdef_t *psp)
{
	mobj_t *mo;

	mo = P_SpawnPlayerMissile(player->mo, MT_MWAND_MISSILE);
	if(mo)
	{
		mo->thinker.function = P_BlasterMobjThinker;
	}
	S_StartSound(player->mo, SFX_MAGE_WAND_FIRE);
}

// ===== Mage Lightning Weapon =====

//============================================================================
//
// A_LightningReady
//
//============================================================================

void A_LightningReady(player_t *player, pspdef_t *psp)
{
	A_WeaponReady(player, psp);
	if(P_Random() < 160)
	{
		S_StartSound(player->mo, SFX_MAGE_LIGHTNING_READY);
	}
}

//============================================================================
//
// A_LightningClip
//
//============================================================================

#define ZAGSPEED	FRACUNIT

void A_LightningClip(mobj_t *actor)
{
	mobj_t *cMo;
	mobj_t *target;
	int zigZag;

	if(actor->type == MT_LIGHTNING_FLOOR)
	{
		actor->z = actor->floorz;
		target = (mobj_t *)((mobj_t *)actor->special2)->special1;
	}
	else if(actor->type == MT_LIGHTNING_CEILING)
	{
		actor->z = actor->ceilingz-actor->height;
		target = (mobj_t *)actor->special1;
	}
	if(actor->type == MT_LIGHTNING_FLOOR)
	{ // floor lightning zig-zags, and forces the ceiling lightning to mimic
		cMo = (mobj_t *)actor->special2;
		zigZag = P_Random();
		if((zigZag > 128 && actor->special1 < 2) || actor->special1 < -2)
		{
			P_ThrustMobj(actor, actor->angle+ANG90, ZAGSPEED);
			if(cMo)
			{
				P_ThrustMobj(cMo, actor->angle+ANG90, ZAGSPEED);
			}
			actor->special1++;
		}
		else
		{
			P_ThrustMobj(actor, actor->angle-ANG90, ZAGSPEED);
			if(cMo)
			{
				P_ThrustMobj(cMo, cMo->angle-ANG90, ZAGSPEED);
			}
			actor->special1--;
		}
	}
	if(target)
	{
		if(target->health <= 0)
		{
			P_ExplodeMissile(actor);
		}
		else
		{
			actor->angle = R_PointToAngle2(actor->x, actor->y, target->x,
				target->y);
			actor->momx = 0;
			actor->momy = 0;
			P_ThrustMobj(actor, actor->angle, actor->info->speed>>1);
		}
	}
}

//============================================================================
//
// A_LightningZap
//
//============================================================================

void A_LightningZap(mobj_t *actor)
{
	mobj_t *mo;
	fixed_t deltaZ;

	A_LightningClip(actor);

	actor->health -= 8;
	if(actor->health <= 0)
	{
		P_SetMobjState(actor, actor->info->deathstate);
		return;
	}
	if(actor->type == MT_LIGHTNING_FLOOR)
	{
		deltaZ = 10*FRACUNIT;
	}
	else
	{
		deltaZ = -10*FRACUNIT;
	}
	mo = P_SpawnMobj(actor->x+((P_Random()-128)*actor->radius/256), 
		actor->y+((P_Random()-128)*actor->radius/256), 
		actor->z+deltaZ, MT_LIGHTNING_ZAP);
	if(mo)
	{
		mo->special2 = (int)actor;
		mo->momx = actor->momx;
		mo->momy = actor->momy;
		mo->target = actor->target;
		if(actor->type == MT_LIGHTNING_FLOOR)
		{
			mo->momz = 20*FRACUNIT;
		}
		else 
		{
			mo->momz = -20*FRACUNIT;
		}
	}
/*
	mo = P_SpawnMobj(actor->x+((P_Random()-128)*actor->radius/256), 
		actor->y+((P_Random()-128)*actor->radius/256), 
		actor->z+deltaZ, MT_LIGHTNING_ZAP);
	if(mo)
	{
		mo->special2 = (int)actor;
		mo->momx = actor->momx;
		mo->momy = actor->momy;
		mo->target = actor->target;
		if(actor->type == MT_LIGHTNING_FLOOR)
		{
			mo->momz = 16*FRACUNIT;
		}
		else 
		{
			mo->momz = -16*FRACUNIT;
		}
	}
*/
	if(actor->type == MT_LIGHTNING_FLOOR && P_Random() < 160)
	{
		S_StartSound(actor, SFX_MAGE_LIGHTNING_CONTINUOUS);
	}
}

//============================================================================
//
// A_MLightningAttack2
//
//============================================================================

void A_MLightningAttack2(mobj_t *actor)
{
	mobj_t *fmo, *cmo;

	fmo = P_SpawnPlayerMissile(actor, MT_LIGHTNING_FLOOR);
	cmo = P_SpawnPlayerMissile(actor, MT_LIGHTNING_CEILING);
	if(fmo)
	{
		fmo->special1 = 0;
		fmo->special2 = (int)cmo;
		A_LightningZap(fmo);	
	}
	if(cmo)
	{
		cmo->special1 = 0;	// mobj that it will track
		cmo->special2 = (int)fmo;
		A_LightningZap(cmo);	
	}
	S_StartSound(actor, SFX_MAGE_LIGHTNING_FIRE);
}

//============================================================================
//
// A_MLightningAttack
//
//============================================================================

void A_MLightningAttack(player_t *player, pspdef_t *psp)
{
	A_MLightningAttack2(player->mo);
	player->mana[MANA_2] -= WeaponManaUse[player->class][player->readyweapon];
}

//============================================================================
//
// A_ZapMimic
//
//============================================================================

void A_ZapMimic(mobj_t *actor)
{
	mobj_t *mo;

	mo = (mobj_t *)actor->special2;
	if(mo)
	{
		if(mo->state >= &states[mo->info->deathstate]
			|| mo->state == &states[S_FREETARGMOBJ])
		{
			P_ExplodeMissile(actor);
		}
		else
		{
			actor->momx = mo->momx;
			actor->momy = mo->momy;
		}
	}
}

//============================================================================
//
// A_LastZap
//
//============================================================================

void A_LastZap(mobj_t *actor)
{
	mobj_t *mo;

	mo = P_SpawnMobj(actor->x, actor->y, actor->z, MT_LIGHTNING_ZAP);
	if(mo)
	{
		P_SetMobjState(mo, S_LIGHTNING_ZAP_X1);
		mo->momz = 40*FRACUNIT;
	}
}

//============================================================================
//
// A_LightningRemove
//
//============================================================================

void A_LightningRemove(mobj_t *actor)
{
	mobj_t *mo;

	mo = (mobj_t *)actor->special2;
	if(mo)
	{
		mo->special2 = 0;
		P_ExplodeMissile(mo);
	}
}


//============================================================================
//
// MStaffSpawn
//
//============================================================================
void MStaffSpawn(mobj_t *pmo, angle_t angle)
{
	mobj_t *mo;

	mo = P_SPMAngle(pmo, MT_MSTAFF_FX2, angle);
	if (mo)
	{
		mo->target = pmo;
		mo->special1 = (int)P_RoughMonsterSearch(mo, 10);
	}
}

//============================================================================
//
// A_MStaffAttack
//
//============================================================================

void A_MStaffAttack(player_t *player, pspdef_t *psp)
{
	angle_t angle;
	mobj_t *pmo;

	player->mana[MANA_1] -= WeaponManaUse[player->class][player->readyweapon];
	player->mana[MANA_2] -= WeaponManaUse[player->class][player->readyweapon];
	pmo = player->mo;
	angle = pmo->angle;
	
	MStaffSpawn(pmo, angle);
	MStaffSpawn(pmo, angle-ANGLE_1*5);
	MStaffSpawn(pmo, angle+ANGLE_1*5);
	S_StartSound(player->mo, SFX_MAGE_STAFF_FIRE);
	if(player == &players[consoleplayer])
	{
		player->damagecount = 0;
		player->bonuscount = 0;
		I_SetPalette((byte *)W_CacheLumpNum(W_GetNumForName("playpal"),
			PU_CACHE)+STARTSCOURGEPAL*768);
	}
}

//============================================================================
//
// A_MStaffPalette
//
//============================================================================

void A_MStaffPalette(player_t *player, pspdef_t *psp)
{
	int pal;

	if(player == &players[consoleplayer])
	{
		pal = STARTSCOURGEPAL+psp->state-(&states[S_MSTAFFATK_2]);
		if(pal == STARTSCOURGEPAL+3)
		{ // reset back to original playpal
			pal = 0;
		}
		I_SetPalette((byte *)W_CacheLumpNum(W_GetNumForName("playpal"),
			PU_CACHE)+pal*768);
	}
}

//============================================================================
//
// A_MStaffWeave
//
//============================================================================

void A_MStaffWeave(mobj_t *actor)
{
	fixed_t newX, newY;
	int weaveXY, weaveZ;
	int angle;

	weaveXY = actor->special2>>16;
	weaveZ = actor->special2&0xFFFF;
	angle = (actor->angle+ANG90)>>ANGLETOFINESHIFT;
	newX = actor->x-FixedMul(finecosine[angle], 
		FloatBobOffsets[weaveXY]<<2);
	newY = actor->y-FixedMul(finesine[angle],
		FloatBobOffsets[weaveXY]<<2);
	weaveXY = (weaveXY+6)&63;
	newX += FixedMul(finecosine[angle], 
		FloatBobOffsets[weaveXY]<<2);
	newY += FixedMul(finesine[angle], 
		FloatBobOffsets[weaveXY]<<2);
	P_TryMove(actor, newX, newY);
	actor->z -= FloatBobOffsets[weaveZ]<<1;
	weaveZ = (weaveZ+3)&63;
	actor->z += FloatBobOffsets[weaveZ]<<1;
	if(actor->z <= actor->floorz)
	{
		actor->z = actor->floorz+FRACUNIT;
	}
	actor->special2 = weaveZ+(weaveXY<<16);
}


//============================================================================
//
// A_MStaffTrack
//
//============================================================================

void A_MStaffTrack(mobj_t *actor)
{
	if ((actor->special1 == 0) && (P_Random()<50))
	{
		actor->special1 = (int)P_RoughMonsterSearch(actor, 10);
	}
	P_SeekerMissile(actor, ANGLE_1*2, ANGLE_1*10);
}


//============================================================================
//
// MStaffSpawn2 - for use by mage class boss
//
//============================================================================

void MStaffSpawn2(mobj_t *actor, angle_t angle)
{
	mobj_t *mo;

	mo = P_SpawnMissileAngle(actor, MT_MSTAFF_FX2, angle, 0);
	if (mo)
	{
		mo->target = actor;
		mo->special1 = (int)P_RoughMonsterSearch(mo, 10);
	}
}

//============================================================================
//
// A_MStaffAttack2 - for use by mage class boss
//
//============================================================================

void A_MStaffAttack2(mobj_t *actor)
{
	angle_t angle;
	angle = actor->angle;
	MStaffSpawn2(actor, angle);
	MStaffSpawn2(actor, angle-ANGLE_1*5);
	MStaffSpawn2(actor, angle+ANGLE_1*5);
	S_StartSound(actor, SFX_MAGE_STAFF_FIRE);
}

//============================================================================
//
// A_FPunchAttack
//
//============================================================================

void A_FPunchAttack(player_t *player, pspdef_t *psp)
{
	angle_t angle;
	int damage;
	int slope;
	mobj_t *pmo = player->mo;
	fixed_t power;
	int i;

	damage = 40+(P_Random()&15);
	power = 2*FRACUNIT;
	PuffType = MT_PUNCHPUFF;
	for(i = 0; i < 16; i++)
	{
		angle = pmo->angle+i*(ANG45/16);
		slope = P_AimLineAttack(pmo, angle, 2*MELEERANGE);
		if(linetarget)
		{
			player->mo->special1++;
			if(pmo->special1 == 3)
			{
				damage <<= 1;
				power = 6*FRACUNIT;
				PuffType = MT_HAMMERPUFF;
			}
			P_LineAttack(pmo, angle, 2*MELEERANGE, slope, damage);
			if (linetarget->flags&MF_COUNTKILL || linetarget->player)
			{
				P_ThrustMobj(linetarget, angle, power);
			}
			AdjustPlayerAngle(pmo);
			goto punchdone;
		}
		angle = pmo->angle-i*(ANG45/16);
		slope = P_AimLineAttack(pmo, angle, 2*MELEERANGE);
		if(linetarget)
		{
			pmo->special1++;
			if(pmo->special1 == 3)
			{
				damage <<= 1;
				power = 6*FRACUNIT;
				PuffType = MT_HAMMERPUFF;
			}
			P_LineAttack(pmo, angle, 2*MELEERANGE, slope, damage);
			if (linetarget->flags&MF_COUNTKILL || linetarget->player)
			{
				P_ThrustMobj(linetarget, angle, power);
			}
			AdjustPlayerAngle(pmo);
			goto punchdone;
		}
	}
	// didn't find any creatures, so try to strike any walls
	pmo->special1 = 0;

	angle = pmo->angle;
	slope = P_AimLineAttack(pmo, angle, MELEERANGE);
	P_LineAttack(pmo, angle, MELEERANGE, slope, damage);

punchdone:
	if(pmo->special1 == 3)
	{
		pmo->special1 = 0;
		P_SetPsprite(player, ps_weapon, S_PUNCHATK2_1);
		S_StartSound(pmo, SFX_FIGHTER_GRUNT);
	}
	return;		
}

//============================================================================
//
// A_FAxeAttack
//
//============================================================================

#define AXERANGE	2.25*MELEERANGE

void A_FAxeAttack(player_t *player, pspdef_t *psp)
{
	angle_t angle;
	mobj_t *pmo=player->mo;
	fixed_t power;
	int damage;
	int slope;
	int i;
	int useMana;

	damage = 40+(P_Random()&15)+(P_Random()&7);
	power = 0;
	if(player->mana[MANA_1] > 0)
	{
		damage <<= 1;
		power = 6*FRACUNIT;
		PuffType = MT_AXEPUFF_GLOW;
		useMana = 1;
	}
	else
	{
		PuffType = MT_AXEPUFF;
		useMana = 0;
	}
	for(i = 0; i < 16; i++)
	{
		angle = pmo->angle+i*(ANG45/16);
		slope = P_AimLineAttack(pmo, angle, AXERANGE);
		if(linetarget)
		{
			P_LineAttack(pmo, angle, AXERANGE, slope, damage);
			if (linetarget->flags&MF_COUNTKILL || linetarget->player)
			{
				P_ThrustMobj(linetarget, angle, power);
			}
			AdjustPlayerAngle(pmo);
			useMana++; 
			goto axedone;
		}
		angle = pmo->angle-i*(ANG45/16);
		slope = P_AimLineAttack(pmo, angle, AXERANGE);
		if(linetarget)
		{
			P_LineAttack(pmo, angle, AXERANGE, slope, damage);
			if (linetarget->flags&MF_COUNTKILL)
			{
				P_ThrustMobj(linetarget, angle, power);
			}
			AdjustPlayerAngle(pmo);
			useMana++; 
			goto axedone;
		}
	}
	// didn't find any creatures, so try to strike any walls
	pmo->special1 = 0;

	angle = pmo->angle;
	slope = P_AimLineAttack(pmo, angle, MELEERANGE);
	P_LineAttack(pmo, angle, MELEERANGE, slope, damage);

axedone:
	if(useMana == 2)
	{
		player->mana[MANA_1] -= 
			WeaponManaUse[player->class][player->readyweapon];
		if(player->mana[MANA_1] <= 0)
		{
			P_SetPsprite(player, ps_weapon, S_FAXEATK_5);
		}
	}
	return;		
}

//===========================================================================
//
// A_CMaceAttack
//
//===========================================================================

void A_CMaceAttack(player_t *player, pspdef_t *psp)
{
	angle_t angle;
	int damage;
	int slope;
	int i;

	damage = 25+(P_Random()&15);
	PuffType = MT_HAMMERPUFF;
	for(i = 0; i < 16; i++)
	{
		angle = player->mo->angle+i*(ANG45/16);
		slope = P_AimLineAttack(player->mo, angle, 2*MELEERANGE);
		if(linetarget)
		{
			P_LineAttack(player->mo, angle, 2*MELEERANGE, slope, 
				damage);
			AdjustPlayerAngle(player->mo);
//			player->mo->angle = R_PointToAngle2(player->mo->x,
//				player->mo->y, linetarget->x, linetarget->y);
			goto macedone;
		}
		angle = player->mo->angle-i*(ANG45/16);
		slope = P_AimLineAttack(player->mo, angle, 2*MELEERANGE);
		if(linetarget)
		{
			P_LineAttack(player->mo, angle, 2*MELEERANGE, slope, 
				damage);
			AdjustPlayerAngle(player->mo);
//			player->mo->angle = R_PointToAngle2(player->mo->x,
//				player->mo->y, linetarget->x, linetarget->y);
			goto macedone;
		}
	}
	// didn't find any creatures, so try to strike any walls
	player->mo->special1 = 0;

	angle = player->mo->angle;
	slope = P_AimLineAttack(player->mo, angle, MELEERANGE);
	P_LineAttack(player->mo, angle, MELEERANGE, slope, 
		damage);
macedone:
	return;		
}

//============================================================================
//
// A_CStaffCheck
//
//============================================================================

void A_CStaffCheck(player_t *player, pspdef_t *psp)
{
	mobj_t *pmo;
	int damage;
	int newLife;
	angle_t angle;
	int slope;
	int i;

	pmo = player->mo;
	damage = 20+(P_Random()&15);
	PuffType = MT_CSTAFFPUFF;
	for(i = 0; i < 3; i++)
	{
		angle = pmo->angle+i*(ANG45/16);
		slope = P_AimLineAttack(pmo, angle, 1.5*MELEERANGE);
		if(linetarget)
		{
			P_LineAttack(pmo, angle, 1.5*MELEERANGE, slope, damage);
			pmo->angle = R_PointToAngle2(pmo->x, pmo->y, 
				linetarget->x, linetarget->y);
			if((linetarget->player || linetarget->flags&MF_COUNTKILL)
				&& (!(linetarget->flags2&(MF2_DORMANT+MF2_INVULNERABLE))))
			{
				newLife = player->health+(damage>>3);
				newLife = newLife > 100 ? 100 : newLife;
				pmo->health = player->health = newLife;
				P_SetPsprite(player, ps_weapon, S_CSTAFFATK2_1);
			}
			player->mana[MANA_1] -= 
				WeaponManaUse[player->class][player->readyweapon];
			break;
		}
		angle = pmo->angle-i*(ANG45/16);
		slope = P_AimLineAttack(player->mo, angle, 1.5*MELEERANGE);
		if(linetarget)
		{
			P_LineAttack(pmo, angle, 1.5*MELEERANGE, slope, damage);
			pmo->angle = R_PointToAngle2(pmo->x, pmo->y, 
				linetarget->x, linetarget->y);
			if(linetarget->player || linetarget->flags&MF_COUNTKILL)
			{
				newLife = player->health+(damage>>4);
				newLife = newLife > 100 ? 100 : newLife;
				pmo->health = player->health = newLife;
				P_SetPsprite(player, ps_weapon, S_CSTAFFATK2_1);
			}
			player->mana[MANA_1] -= 
				WeaponManaUse[player->class][player->readyweapon];
			break;
		}
	}
}

//============================================================================
//
// A_CStaffAttack
//
//============================================================================

void A_CStaffAttack(player_t *player, pspdef_t *psp)
{
	mobj_t *mo;
	mobj_t *pmo;

	player->mana[MANA_1] -=	WeaponManaUse[player->class][player->readyweapon];
	pmo = player->mo;
	mo = P_SPMAngle(pmo, MT_CSTAFF_MISSILE, pmo->angle-(ANG45/15));
	if(mo)
	{
		mo->special2 = 32;
	}
	mo = P_SPMAngle(pmo, MT_CSTAFF_MISSILE, pmo->angle+(ANG45/15));
	if(mo)
	{
		mo->special2 = 0;
	}
	S_StartSound(player->mo, SFX_CLERIC_CSTAFF_FIRE);
}

//============================================================================
//
// A_CStaffMissileSlither
//
//============================================================================

void A_CStaffMissileSlither(mobj_t *actor)
{
	fixed_t newX, newY;
	int weaveXY;
	int angle;

	weaveXY = actor->special2;
	angle = (actor->angle+ANG90)>>ANGLETOFINESHIFT;
	newX = actor->x-FixedMul(finecosine[angle], 
		FloatBobOffsets[weaveXY]);
	newY = actor->y-FixedMul(finesine[angle],
		FloatBobOffsets[weaveXY]);
	weaveXY = (weaveXY+3)&63;
	newX += FixedMul(finecosine[angle], 
		FloatBobOffsets[weaveXY]);
	newY += FixedMul(finesine[angle], 
		FloatBobOffsets[weaveXY]);
	P_TryMove(actor, newX, newY);
	actor->special2 = weaveXY;	
}

//============================================================================
//
// A_CStaffInitBlink
//
//============================================================================

void A_CStaffInitBlink(player_t *player, pspdef_t *psp)
{
	player->mo->special1 = (P_Random()>>1)+20;
}

//============================================================================
//
// A_CStaffCheckBlink
//
//============================================================================

void A_CStaffCheckBlink(player_t *player, pspdef_t *psp)
{
	if(!--player->mo->special1)
	{
		P_SetPsprite(player, ps_weapon, S_CSTAFFBLINK1);
		player->mo->special1 = (P_Random()+50)>>2;
	}
}

//============================================================================
//
// A_CFlameAttack
//
//============================================================================

#define FLAMESPEED	(0.45*FRACUNIT)
#define CFLAMERANGE	(12*64*FRACUNIT)

void A_CFlameAttack(player_t *player, pspdef_t *psp)
{
	mobj_t *mo;

	mo = P_SpawnPlayerMissile(player->mo, MT_CFLAME_MISSILE);
	if(mo)
	{
		mo->thinker.function = P_BlasterMobjThinker;
		mo->special1 = 2;
	}

	player->mana[MANA_2] -= WeaponManaUse[player->class][player->readyweapon];
	S_StartSound(player->mo, SFX_CLERIC_FLAME_FIRE);
}

//============================================================================
//
// A_CFlamePuff
//
//============================================================================

void A_CFlamePuff(mobj_t *actor)
{
	A_UnHideThing(actor);
	actor->momx = 0;
	actor->momy = 0;
	actor->momz = 0;
	S_StartSound(actor, SFX_CLERIC_FLAME_EXPLODE);
}

//============================================================================
//
// A_CFlameMissile
//
//============================================================================

void A_CFlameMissile(mobj_t *actor)
{
	int i;
	int an, an90;
	fixed_t dist;
	mobj_t *mo;
	
	A_UnHideThing(actor);
	S_StartSound(actor, SFX_CLERIC_FLAME_EXPLODE);
	if(BlockingMobj && BlockingMobj->flags&MF_SHOOTABLE)
	{ // Hit something, so spawn the flame circle around the thing
		dist = BlockingMobj->radius+18*FRACUNIT;
		for(i = 0; i < 4; i++)
		{
			an = (i*ANG45)>>ANGLETOFINESHIFT;
			an90 = (i*ANG45+ANG90)>>ANGLETOFINESHIFT;
			mo = P_SpawnMobj(BlockingMobj->x+FixedMul(dist, finecosine[an]),
				BlockingMobj->y+FixedMul(dist, finesine[an]), 
				BlockingMobj->z+5*FRACUNIT, MT_CIRCLEFLAME);
			if(mo)
			{
				mo->angle = an<<ANGLETOFINESHIFT;
				mo->target = actor->target;
				mo->momx = mo->special1 = FixedMul(FLAMESPEED, finecosine[an]);
				mo->momy = mo->special2 = FixedMul(FLAMESPEED, finesine[an]);
				mo->tics -= P_Random()&3;
			}
			mo = P_SpawnMobj(BlockingMobj->x-FixedMul(dist, finecosine[an]),
				BlockingMobj->y-FixedMul(dist, finesine[an]), 
				BlockingMobj->z+5*FRACUNIT, MT_CIRCLEFLAME);
			if(mo)
			{
				mo->angle = ANG180+(an<<ANGLETOFINESHIFT);
				mo->target = actor->target;
				mo->momx = mo->special1 = FixedMul(-FLAMESPEED, 
					finecosine[an]);
				mo->momy = mo->special2 = FixedMul(-FLAMESPEED, finesine[an]);
				mo->tics -= P_Random()&3;
			}
		}
		P_SetMobjState(actor, S_FLAMEPUFF2_1);
	}
}

/*
void A_CFlameAttack(player_t *player, pspdef_t *psp)
{
	mobj_t *pmo;
	angle_t angle;
	int damage;
	int i;
	int an, an90;
	fixed_t dist;
	mobj_t *mo;

	pmo = player->mo;
	P_BulletSlope(pmo);
	damage = 25+HITDICE(3);
	angle = pmo->angle;
	if(player->refire)
	{
		angle += (P_Random()-P_Random())<<17;
	}
	P_AimLineAttack(pmo, angle, CFLAMERANGE); // Correctly set linetarget
	if(!linetarget)
	{
		angle += ANGLE_1*2;
		P_AimLineAttack(pmo, angle, CFLAMERANGE);
		if(!linetarget)
		{
			angle -= ANGLE_1*4;
			P_AimLineAttack(pmo, angle, CFLAMERANGE);
			if(!linetarget)
			{
				angle += ANGLE_1*2;
			}
		}		
	}
	if(linetarget)
	{
		PuffType = MT_FLAMEPUFF2;
	}
	else
	{
		PuffType = MT_FLAMEPUFF;
	}
	P_LineAttack(pmo, angle, CFLAMERANGE, bulletslope, damage);
	if(linetarget)
	{ // Hit something, so spawn the flame circle around the thing
		dist = linetarget->radius+18*FRACUNIT;
		for(i = 0; i < 4; i++)
		{
			an = (i*ANG45)>>ANGLETOFINESHIFT;
			an90 = (i*ANG45+ANG90)>>ANGLETOFINESHIFT;
			mo = P_SpawnMobj(linetarget->x+FixedMul(dist, finecosine[an]),
				linetarget->y+FixedMul(dist, finesine[an]), 
				linetarget->z+5*FRACUNIT, MT_CIRCLEFLAME);
			if(mo)
			{
				mo->angle = an<<ANGLETOFINESHIFT;
				mo->target = pmo;
				mo->momx = mo->special1 = FixedMul(FLAMESPEED, finecosine[an]);
				mo->momy = mo->special2 = FixedMul(FLAMESPEED, finesine[an]);
				mo->tics -= P_Random()&3;
			}
			mo = P_SpawnMobj(linetarget->x-FixedMul(dist, finecosine[an]),
				linetarget->y-FixedMul(dist, finesine[an]), 
				linetarget->z+5*FRACUNIT, MT_CIRCLEFLAME);
			if(mo)
			{
				mo->angle = ANG180+(an<<ANGLETOFINESHIFT);
				mo->target = pmo;
				mo->momx = mo->special1 = FixedMul(-FLAMESPEED, 
					finecosine[an]);
				mo->momy = mo->special2 = FixedMul(-FLAMESPEED, finesine[an]);
				mo->tics -= P_Random()&3;
			}
		}
	}
// Create a line of flames from the player to the flame puff
	CFlameCreateFlames(player->mo);

	player->mana[MANA_2] -= WeaponManaUse[player->class][player->readyweapon];
	S_StartSound(player->mo, SFX_CLERIC_FLAME_FIRE);
}
*/

//============================================================================
//
// A_CFlameRotate
//
//============================================================================

#define FLAMEROTSPEED	2*FRACUNIT

void A_CFlameRotate(mobj_t *actor)
{
	int an;

	an = (actor->angle+ANG90)>>ANGLETOFINESHIFT;
	actor->momx = actor->special1+FixedMul(FLAMEROTSPEED, finecosine[an]);
	actor->momy = actor->special2+FixedMul(FLAMEROTSPEED, finesine[an]);
	actor->angle += ANG90/15;
}


//============================================================================
//
// A_CHolyAttack3
//
// 	Spawns the spirits
//============================================================================

void A_CHolyAttack3(mobj_t *actor)
{
	P_SpawnMissile(actor, actor->target, MT_HOLY_MISSILE);
	S_StartSound(actor, SFX_CHOLY_FIRE);
}


//============================================================================
//
// A_CHolyAttack2 
//
// 	Spawns the spirits
//============================================================================

void A_CHolyAttack2(mobj_t *actor)
{
	int j;
	int i;
	mobj_t *mo;
	mobj_t *tail, *next;

	for(j = 0; j < 4; j++)
	{
		mo = P_SpawnMobj(actor->x, actor->y, actor->z, MT_HOLY_FX);
		if(!mo)
		{
			continue;
		}
		switch(j)
		{ // float bob index
			case 0:
				mo->special2 = P_Random()&7; // upper-left
				break;
			case 1:
				mo->special2 = 32+(P_Random()&7); // upper-right
				break;
			case 2:
				mo->special2 = (32+(P_Random()&7))<<16; // lower-left
				break;
			case 3:
				mo->special2 = ((32+(P_Random()&7))<<16)+32+(P_Random()&7);
				break;
		}
		mo->z = actor->z;
		mo->angle = actor->angle+(ANGLE_45+ANGLE_45/2)-ANGLE_45*j;
		P_ThrustMobj(mo, mo->angle, mo->info->speed);
		mo->target = actor->target;
		mo->args[0] = 10; // initial turn value
		mo->args[1] = 0; // initial look angle
		if(deathmatch)
		{ // Ghosts last slightly less longer in DeathMatch
			mo->health = 85;
		}
		if(linetarget)
		{
			mo->special1 = (int)linetarget;
			mo->flags |= MF_NOCLIP|MF_SKULLFLY;
			mo->flags &= ~MF_MISSILE;
		}
		tail = P_SpawnMobj(mo->x, mo->y, mo->z, MT_HOLY_TAIL);
		tail->special2 = (int)mo; // parent
		for(i = 1; i < 3; i++)
		{
			next = P_SpawnMobj(mo->x, mo->y, mo->z, MT_HOLY_TAIL);
			P_SetMobjState(next, next->info->spawnstate+1);
			tail->special1 = (int)next;
			tail = next;
		}
		tail->special1 = 0; // last tail bit
	}
}

//============================================================================
//
// A_CHolyAttack
//
//============================================================================

void A_CHolyAttack(player_t *player, pspdef_t *psp)
{
	mobj_t *mo;

	player->mana[MANA_1] -= WeaponManaUse[player->class][player->readyweapon];
	player->mana[MANA_2] -= WeaponManaUse[player->class][player->readyweapon];
	mo = P_SpawnPlayerMissile(player->mo, MT_HOLY_MISSILE);
	if(player == &players[consoleplayer])
	{
		player->damagecount = 0;
		player->bonuscount = 0;
		I_SetPalette((byte *)W_CacheLumpNum(W_GetNumForName("playpal"),
			PU_CACHE)+STARTHOLYPAL*768);
	}
	S_StartSound(player->mo, SFX_CHOLY_FIRE);
}

//============================================================================
//
// A_CHolyPalette
//
//============================================================================

void A_CHolyPalette(player_t *player, pspdef_t *psp)
{
	int pal;

	if(player == &players[consoleplayer])
	{
		pal = STARTHOLYPAL+psp->state-(&states[S_CHOLYATK_6]);
		if(pal == STARTHOLYPAL+3)
		{ // reset back to original playpal
			pal = 0;
		}
		I_SetPalette((byte *)W_CacheLumpNum(W_GetNumForName("playpal"),
			PU_CACHE)+pal*768);
	}
}

//============================================================================
//
// CHolyFindTarget
//
//============================================================================

static void CHolyFindTarget(mobj_t *actor)
{
	mobj_t *target;

	if(target = P_RoughMonsterSearch(actor, 6))
	{
		actor->special1 = (int)target;
		actor->flags |= MF_NOCLIP|MF_SKULLFLY;
		actor->flags &= ~MF_MISSILE;
	}
}

//============================================================================
//
// CHolySeekerMissile
//
// 	 Similar to P_SeekerMissile, but seeks to a random Z on the target
//============================================================================

static void CHolySeekerMissile(mobj_t *actor, angle_t thresh, angle_t turnMax)
{
	int dir;
	int dist;
	angle_t delta;
	angle_t angle;
	mobj_t *target;
	fixed_t newZ;
	fixed_t deltaZ;

	target = (mobj_t *)actor->special1;
	if(target == NULL)
	{
		return;
	}
	if(!(target->flags&MF_SHOOTABLE) 
	|| (!(target->flags&MF_COUNTKILL) && !target->player))
	{ // Target died/target isn't a player or creature
		actor->special1 = 0;
		actor->flags &= ~(MF_NOCLIP|MF_SKULLFLY);
		actor->flags |= MF_MISSILE;
		CHolyFindTarget(actor);
		return;
	}
	dir = P_FaceMobj(actor, target, &delta);
	if(delta > thresh)
	{
		delta >>= 1;
		if(delta > turnMax)
		{
			delta = turnMax;
		}
	}
	if(dir)
	{ // Turn clockwise
		actor->angle += delta;
	}
	else
	{ // Turn counter clockwise
		actor->angle -= delta;
	}
	angle = actor->angle>>ANGLETOFINESHIFT;
	actor->momx = FixedMul(actor->info->speed, finecosine[angle]);
	actor->momy = FixedMul(actor->info->speed, finesine[angle]);
	if(!(leveltime&15) 
		|| actor->z > target->z+(target->height)
		|| actor->z+actor->height < target->z)
	{
		newZ = target->z+((P_Random()*target->height)>>8);
		deltaZ = newZ-actor->z;
		if(abs(deltaZ) > 15*FRACUNIT)
		{
			if(deltaZ > 0)
			{
				deltaZ = 15*FRACUNIT;
			}
			else
			{
				deltaZ = -15*FRACUNIT;
			}
		}
		dist = P_AproxDistance(target->x-actor->x, target->y-actor->y);
		dist = dist/actor->info->speed;
		if(dist < 1)
		{
			dist = 1;
		}
		actor->momz = deltaZ/dist;
	}
	return;
}

//============================================================================
//
// A_CHolyWeave
//
//============================================================================

static void CHolyWeave(mobj_t *actor)
{
	fixed_t newX, newY;
	int weaveXY, weaveZ;
	int angle;

	weaveXY = actor->special2>>16;
	weaveZ = actor->special2&0xFFFF;
	angle = (actor->angle+ANG90)>>ANGLETOFINESHIFT;
	newX = actor->x-FixedMul(finecosine[angle], 
		FloatBobOffsets[weaveXY]<<2);
	newY = actor->y-FixedMul(finesine[angle],
		FloatBobOffsets[weaveXY]<<2);
	weaveXY = (weaveXY+(P_Random()%5))&63;
	newX += FixedMul(finecosine[angle], 
		FloatBobOffsets[weaveXY]<<2);
	newY += FixedMul(finesine[angle], 
		FloatBobOffsets[weaveXY]<<2);
	P_TryMove(actor, newX, newY);
	actor->z -= FloatBobOffsets[weaveZ]<<1;
	weaveZ = (weaveZ+(P_Random()%5))&63;
	actor->z += FloatBobOffsets[weaveZ]<<1;	
	actor->special2 = weaveZ+(weaveXY<<16);
}

//============================================================================
//
// A_CHolySeek
//
//============================================================================

void A_CHolySeek(mobj_t *actor)
{
	actor->health--;
	if(actor->health <= 0)
	{
		actor->momx >>= 2;
		actor->momy >>= 2;
		actor->momz = 0;
		P_SetMobjState(actor, actor->info->deathstate);
		actor->tics -= P_Random()&3;
		return;
	}
	if(actor->special1)
	{
		CHolySeekerMissile(actor, actor->args[0]*ANGLE_1,
			actor->args[0]*ANGLE_1*2);
		if(!((leveltime+7)&15))
		{
			actor->args[0] = 5+(P_Random()/20);
		}
	}
	CHolyWeave(actor);
}

//============================================================================
//
// CHolyTailFollow
//
//============================================================================

static void CHolyTailFollow(mobj_t *actor, fixed_t dist)
{
	mobj_t *child;
	int an;
	fixed_t oldDistance, newDistance;

	child = (mobj_t *)actor->special1;
	if(child)
	{
		an = R_PointToAngle2(actor->x, actor->y, child->x, 
			child->y)>>ANGLETOFINESHIFT;
		oldDistance = P_AproxDistance(child->x-actor->x, child->y-actor->y);
		if(P_TryMove(child, actor->x+FixedMul(dist, finecosine[an]), 
			actor->y+FixedMul(dist, finesine[an])))
		{
			newDistance = P_AproxDistance(child->x-actor->x, 
				child->y-actor->y)-FRACUNIT;
			if(oldDistance < FRACUNIT)
			{
				if(child->z < actor->z)
				{
					child->z = actor->z-dist;
				}
				else
				{
					child->z = actor->z+dist;
				}
			}
			else
			{
				child->z = actor->z+FixedMul(FixedDiv(newDistance, 
					oldDistance), child->z-actor->z);
			}
		}
		CHolyTailFollow(child, dist-FRACUNIT);
	}
}

//============================================================================
//
// CHolyTailRemove
//
//============================================================================

static void CHolyTailRemove(mobj_t *actor)
{
	mobj_t *child;

	child = (mobj_t *)actor->special1;
	if(child)
	{
		CHolyTailRemove(child);
	}
	P_RemoveMobj(actor);
}

//============================================================================
//
// A_CHolyTail
//
//============================================================================

void A_CHolyTail(mobj_t *actor)
{
	mobj_t *parent;

	parent = (mobj_t *)actor->special2;

	if(parent)
	{
		if(parent->state >= &states[parent->info->deathstate])
		{ // Ghost removed, so remove all tail parts
			CHolyTailRemove(actor);
			return;
		}
		else if(P_TryMove(actor, parent->x-FixedMul(14*FRACUNIT,
			finecosine[parent->angle>>ANGLETOFINESHIFT]),
			parent->y-FixedMul(14*FRACUNIT, 
			finesine[parent->angle>>ANGLETOFINESHIFT])))
		{
			actor->z = parent->z-5*FRACUNIT;
		}
		CHolyTailFollow(actor, 10*FRACUNIT);
	}
}
//============================================================================
//
// A_CHolyCheckScream
//
//============================================================================

void A_CHolyCheckScream(mobj_t *actor)
{
	A_CHolySeek(actor);
	if(P_Random() < 20)
	{
		S_StartSound(actor, SFX_SPIRIT_ACTIVE);
	}
	if(!actor->special1)
	{
		CHolyFindTarget(actor);
	}
}

//============================================================================
//
// A_CHolySpawnPuff
//
//============================================================================

void A_CHolySpawnPuff(mobj_t *actor)
{
	P_SpawnMobj(actor->x, actor->y, actor->z, MT_HOLY_MISSILE_PUFF);
}

//----------------------------------------------------------------------------
//
// PROC A_FireConePL1
//
//----------------------------------------------------------------------------

#define SHARDSPAWN_LEFT		1
#define SHARDSPAWN_RIGHT	2
#define SHARDSPAWN_UP		4
#define SHARDSPAWN_DOWN		8

void A_FireConePL1(player_t *player, pspdef_t *psp)
{
	angle_t angle;
	int damage;
	int slope;
	int i;
	mobj_t *pmo,*mo;
	int conedone=false;

	pmo = player->mo;
	player->mana[MANA_1] -= WeaponManaUse[player->class][player->readyweapon];
	S_StartSound(pmo, SFX_MAGE_SHARDS_FIRE);

	damage = 90+(P_Random()&15);
	for(i = 0; i < 16; i++)
	{
		angle = pmo->angle+i*(ANG45/16);
		slope = P_AimLineAttack(pmo, angle, MELEERANGE);
		if(linetarget)
		{
			pmo->flags2 |= MF2_ICEDAMAGE;
			P_DamageMobj(linetarget, pmo, pmo, damage);
			pmo->flags2 &= ~MF2_ICEDAMAGE;
			conedone = true;
			break;
		}
	}

	// didn't find any creatures, so fire projectiles
	if (!conedone)
	{
		mo = P_SpawnPlayerMissile(pmo, MT_SHARDFX1);
		if (mo)
		{
			mo->special1 = SHARDSPAWN_LEFT|SHARDSPAWN_DOWN|SHARDSPAWN_UP
				|SHARDSPAWN_RIGHT;
			mo->special2 = 3; // Set sperm count (levels of reproductivity)
			mo->target = pmo;
			mo->args[0] = 3;		// Mark Initial shard as super damage
		}
	}
}

void A_ShedShard(mobj_t *actor)
{
	mobj_t *mo;
	int spawndir = actor->special1;
	int spermcount = actor->special2;

	if (spermcount <= 0) return;				// No sperm left
	actor->special2 = 0;
	spermcount--;

	// every so many calls, spawn a new missile in it's set directions
	if (spawndir & SHARDSPAWN_LEFT)
	{
		mo=P_SpawnMissileAngleSpeed(actor, MT_SHARDFX1, actor->angle+(ANG45/9),
											 0, (20+2*spermcount)<<FRACBITS);
		if (mo)
		{
			mo->special1 = SHARDSPAWN_LEFT;
			mo->special2 = spermcount;
			mo->momz = actor->momz;
			mo->target = actor->target;
			mo->args[0] = (spermcount==3)?2:0;
		}
	}
	if (spawndir & SHARDSPAWN_RIGHT)
	{
		mo=P_SpawnMissileAngleSpeed(actor, MT_SHARDFX1, actor->angle-(ANG45/9),
											 0, (20+2*spermcount)<<FRACBITS);
		if (mo)
		{
			mo->special1 = SHARDSPAWN_RIGHT;
			mo->special2 = spermcount;
			mo->momz = actor->momz;
			mo->target = actor->target;
			mo->args[0] = (spermcount==3)?2:0;
		}
	}
	if (spawndir & SHARDSPAWN_UP)
	{
		mo=P_SpawnMissileAngleSpeed(actor, MT_SHARDFX1, actor->angle, 
											 0, (15+2*spermcount)<<FRACBITS);
		if (mo)
		{
			mo->momz = actor->momz;
			mo->z += 8*FRACUNIT;
			if (spermcount & 1)			// Every other reproduction
				mo->special1 = SHARDSPAWN_UP | SHARDSPAWN_LEFT | SHARDSPAWN_RIGHT;
			else
				mo->special1 = SHARDSPAWN_UP;
			mo->special2 = spermcount;
			mo->target = actor->target;
			mo->args[0] = (spermcount==3)?2:0;
		}
	}
	if (spawndir & SHARDSPAWN_DOWN)
	{
		mo=P_SpawnMissileAngleSpeed(actor, MT_SHARDFX1, actor->angle, 
											 0, (15+2*spermcount)<<FRACBITS);
		if (mo)
		{
			mo->momz = actor->momz;
			mo->z -= 4*FRACUNIT;
			if (spermcount & 1)			// Every other reproduction
				mo->special1 = SHARDSPAWN_DOWN | SHARDSPAWN_LEFT | SHARDSPAWN_RIGHT;
			else
				mo->special1 = SHARDSPAWN_DOWN;
			mo->special2 = spermcount;
			mo->target = actor->target;
			mo->args[0] = (spermcount==3)?2:0;
		}
	}
}

//----------------------------------------------------------------------------
//
// PROC A_HideInCeiling
//
//----------------------------------------------------------------------------

/*
void A_HideInCeiling(mobj_t *actor)
{
	actor->z = actor->ceilingz+4*FRACUNIT;
}
*/

//----------------------------------------------------------------------------
//
// PROC A_FloatPuff
//
//----------------------------------------------------------------------------

/*
void A_FloatPuff(mobj_t *puff)
{
	puff->momz += 1.8*FRACUNIT;
}
*/

void A_Light0(player_t *player, pspdef_t *psp)
{
	player->extralight = 0;
}

/*
void A_Light1(player_t *player, pspdef_t *psp)
{
	player->extralight = 1;
}
*/

/*
void A_Light2(player_t *player, pspdef_t *psp)
{
	player->extralight = 2;
}
*/

//------------------------------------------------------------------------
//
// PROC P_SetupPsprites
//
// Called at start of level for each player
//
//------------------------------------------------------------------------

void P_SetupPsprites(player_t *player)
{
	int i;

	// Remove all psprites
	for(i = 0; i < NUMPSPRITES; i++)
	{
		player->psprites[i].state = NULL;
	}
	// Spawn the ready weapon
	player->pendingweapon = player->readyweapon;
	P_BringUpWeapon(player);
}

//------------------------------------------------------------------------
//
// PROC P_MovePsprites
//
// Called every tic by player thinking routine
//
//------------------------------------------------------------------------

void P_MovePsprites(player_t *player)
{
	int i;
	pspdef_t *psp;
	state_t *state;

	psp = &player->psprites[0];
	for(i = 0; i < NUMPSPRITES; i++, psp++)
	{
		if((state = psp->state) != 0) // a null state means not active
		{
			// drop tic count and possibly change state
			if(psp->tics != -1)	// a -1 tic count never changes
			{
				psp->tics--;
				if(!psp->tics)
				{
					P_SetPsprite(player, i, psp->state->nextstate);
				}
			}
		}
	}
	player->psprites[ps_flash].sx = player->psprites[ps_weapon].sx;
	player->psprites[ps_flash].sy = player->psprites[ps_weapon].sy;
}
