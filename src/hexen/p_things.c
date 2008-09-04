
//**************************************************************************
//**
//** p_things.c : Heretic 2 : Raven Software, Corp.
//**
//** $RCSfile: p_things.c,v $
//** $Revision: 1.36 $
//** $Date: 96/02/08 15:16:13 $
//** $Author: bgokey $
//**
//**************************************************************************

// HEADER FILES ------------------------------------------------------------

#include "h2def.h"
#include "p_local.h"
#include "soundst.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

static boolean ActivateThing(mobj_t *mobj);
static boolean DeactivateThing(mobj_t *mobj);

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

mobjtype_t TranslateThingType[] =
{
	MT_MAPSPOT,				// T_NONE
	MT_CENTAUR,				// T_CENTAUR
	MT_CENTAURLEADER,		// T_CENTAURLEADER
	MT_DEMON,				// T_DEMON
	MT_ETTIN,				// T_ETTIN
	MT_FIREDEMON,			// T_FIREGARGOYLE
	MT_SERPENT,				// T_WATERLURKER
	MT_SERPENTLEADER,		// T_WATERLURKERLEADER
	MT_WRAITH,				// T_WRAITH
	MT_WRAITHB,				// T_WRAITHBURIED
	MT_FIREBALL1,			// T_FIREBALL1
	MT_MANA1,				// T_MANA1
	MT_MANA2,				// T_MANA2
	MT_SPEEDBOOTS,			// T_ITEMBOOTS
	MT_ARTIEGG,				// T_ITEMEGG
	MT_ARTIFLY,				// T_ITEMFLIGHT
	MT_SUMMONMAULATOR,		// T_ITEMSUMMON
	MT_TELEPORTOTHER,		// T_ITEMTPORTOTHER
	MT_ARTITELEPORT,		// T_ITEMTELEPORT
	MT_BISHOP,				// T_BISHOP
	MT_ICEGUY,				// T_ICEGOLEM
	MT_BRIDGE,				// T_BRIDGE
	MT_BOOSTARMOR,			// T_DRAGONSKINBRACERS
	MT_HEALINGBOTTLE,		// T_ITEMHEALTHPOTION
	MT_HEALTHFLASK,			// T_ITEMHEALTHFLASK
	MT_ARTISUPERHEAL,		// T_ITEMHEALTHFULL
	MT_BOOSTMANA,			// T_ITEMBOOSTMANA
	MT_FW_AXE,				// T_FIGHTERAXE
	MT_FW_HAMMER,			// T_FIGHTERHAMMER
	MT_FW_SWORD1,			// T_FIGHTERSWORD1
	MT_FW_SWORD2,			// T_FIGHTERSWORD2
	MT_FW_SWORD3,			// T_FIGHTERSWORD3
	MT_CW_SERPSTAFF,		// T_CLERICSTAFF
	MT_CW_HOLY1,			// T_CLERICHOLY1
	MT_CW_HOLY2,			// T_CLERICHOLY2
	MT_CW_HOLY3,			// T_CLERICHOLY3
	MT_MW_CONE,				// T_MAGESHARDS
	MT_MW_STAFF1,			// T_MAGESTAFF1
	MT_MW_STAFF2,			// T_MAGESTAFF2
	MT_MW_STAFF3,			// T_MAGESTAFF3
	MT_EGGFX,				// T_MORPHBLAST
	MT_ROCK1,				// T_ROCK1
	MT_ROCK2,				// T_ROCK2
	MT_ROCK3,				// T_ROCK3
	MT_DIRT1,				// T_DIRT1
	MT_DIRT2,				// T_DIRT2
	MT_DIRT3,				// T_DIRT3
	MT_DIRT4,				// T_DIRT4
	MT_DIRT5,				// T_DIRT5
	MT_DIRT6,				// T_DIRT6
	MT_ARROW,				// T_ARROW
	MT_DART,				// T_DART
	MT_POISONDART,			// T_POISONDART
	MT_RIPPERBALL,			// T_RIPPERBALL
	MT_SGSHARD1,			// T_STAINEDGLASS1
	MT_SGSHARD2,			// T_STAINEDGLASS2
	MT_SGSHARD3,			// T_STAINEDGLASS3
	MT_SGSHARD4,			// T_STAINEDGLASS4
	MT_SGSHARD5,			// T_STAINEDGLASS5
	MT_SGSHARD6,			// T_STAINEDGLASS6
	MT_SGSHARD7,			// T_STAINEDGLASS7
	MT_SGSHARD8,			// T_STAINEDGLASS8
	MT_SGSHARD9,			// T_STAINEDGLASS9
	MT_SGSHARD0,			// T_STAINEDGLASS0
	MT_PROJECTILE_BLADE,	// T_BLADE
	MT_ICESHARD,			// T_ICESHARD
	MT_FLAME_SMALL,			// T_FLAME_SMALL
	MT_FLAME_LARGE,			// T_FLAME_LARGE
	MT_ARMOR_1,				// T_MESHARMOR
	MT_ARMOR_2,				// T_FALCONSHIELD
	MT_ARMOR_3,				// T_PLATINUMHELM
	MT_ARMOR_4,				// T_AMULETOFWARDING
	MT_ARTIPOISONBAG,		// T_ITEMFLECHETTE
	MT_ARTITORCH,			// T_ITEMTORCH
	MT_BLASTRADIUS,			// T_ITEMREPULSION
	MT_MANA3,				// T_MANA3
	MT_ARTIPUZZSKULL,		// T_PUZZSKULL
	MT_ARTIPUZZGEMBIG,		// T_PUZZGEMBIG
	MT_ARTIPUZZGEMRED,		// T_PUZZGEMRED
	MT_ARTIPUZZGEMGREEN1,	// T_PUZZGEMGREEN1
	MT_ARTIPUZZGEMGREEN2,	// T_PUZZGEMGREEN2
	MT_ARTIPUZZGEMBLUE1,	// T_PUZZGEMBLUE1
	MT_ARTIPUZZGEMBLUE2,	// T_PUZZGEMBLUE2
	MT_ARTIPUZZBOOK1,		// T_PUZZBOOK1
	MT_ARTIPUZZBOOK2,		// T_PUZZBOOK2
	MT_KEY1,				// T_METALKEY
	MT_KEY2,				// T_SMALLMETALKEY
	MT_KEY3,				// T_AXEKEY
	MT_KEY4,				// T_FIREKEY
	MT_KEY5,				// T_GREENKEY
	MT_KEY6,				// T_MACEKEY
	MT_KEY7,				// T_SILVERKEY
	MT_KEY8,				// T_RUSTYKEY
	MT_KEY9,				// T_HORNKEY
	MT_KEYA,				// T_SERPENTKEY
	MT_WATER_DRIP,			// T_WATERDRIP
	MT_FLAME_SMALL_TEMP,	// T_TEMPSMALLFLAME
	MT_FLAME_SMALL,			// T_PERMSMALLFLAME
	MT_FLAME_LARGE_TEMP,	// T_TEMPLARGEFLAME
	MT_FLAME_LARGE,			// T_PERMLARGEFLAME
	MT_DEMON_MASH,			// T_DEMON_MASH
	MT_DEMON2_MASH,			// T_DEMON2_MASH
	MT_ETTIN_MASH,			// T_ETTIN_MASH
	MT_CENTAUR_MASH,		// T_CENTAUR_MASH
	MT_THRUSTFLOOR_UP,		// T_THRUSTSPIKEUP
	MT_THRUSTFLOOR_DOWN,	// T_THRUSTSPIKEDOWN
	MT_WRAITHFX4,			// T_FLESH_DRIP1
	MT_WRAITHFX5,			// T_FLESH_DRIP2
	MT_WRAITHFX2			// T_SPARK_DRIP
};

// PRIVATE DATA DEFINITIONS ------------------------------------------------

// CODE --------------------------------------------------------------------

//==========================================================================
//
// EV_ThingProjectile
//
//==========================================================================

boolean EV_ThingProjectile(byte *args, boolean gravity)
{
	int tid;
	angle_t angle;
	int fineAngle;
	fixed_t speed;
	fixed_t vspeed;
	mobjtype_t moType;
	mobj_t *mobj;
	mobj_t *newMobj;
	int searcher;
	boolean success;

	success = false;
	searcher = -1;
	tid = args[0];
	moType = TranslateThingType[args[1]];
	if(nomonsters && (mobjinfo[moType].flags&MF_COUNTKILL))
	{ // Don't spawn monsters if -nomonsters
		return false;
	}
	angle = (int)args[2]<<24;
	fineAngle = angle>>ANGLETOFINESHIFT;
	speed = (int)args[3]<<13;
	vspeed = (int)args[4]<<13;
	while((mobj = P_FindMobjFromTID(tid, &searcher)) != NULL)
	{
		newMobj = P_SpawnMobj(mobj->x, mobj->y, mobj->z, moType);
		if(newMobj->info->seesound)
		{
			S_StartSound(newMobj, newMobj->info->seesound);
		}
		newMobj->target = mobj; // Originator
		newMobj->angle = angle;
		newMobj->momx = FixedMul(speed, finecosine[fineAngle]);
		newMobj->momy = FixedMul(speed, finesine[fineAngle]);
		newMobj->momz = vspeed;
		newMobj->flags2 |= MF2_DROPPED; // Don't respawn
		if(gravity == true)
		{
			newMobj->flags &= ~MF_NOGRAVITY;
			newMobj->flags2 |= MF2_LOGRAV;
		}
		if(P_CheckMissileSpawn(newMobj) == true)
		{
			success = true;
		}
	}
	return success;
}

//==========================================================================
//
// EV_ThingSpawn
//
//==========================================================================

boolean EV_ThingSpawn(byte *args, boolean fog)
{
	int tid;
	angle_t angle;
	mobj_t *mobj;
	mobj_t *newMobj;
	mobj_t *fogMobj;
	mobjtype_t moType;
	int searcher;
	boolean success;
	fixed_t z;

	success = false;
	searcher = -1;
	tid = args[0];
	moType = TranslateThingType[args[1]];
	if(nomonsters && (mobjinfo[moType].flags&MF_COUNTKILL))
	{ // Don't spawn monsters if -nomonsters
		return false;
	}
	angle = (int)args[2]<<24;
	while((mobj = P_FindMobjFromTID(tid, &searcher)) != NULL)
	{
		if (mobjinfo[moType].flags2&MF2_FLOATBOB)
		{
			z = mobj->z - mobj->floorz;
		}
		else
		{
			z = mobj->z;
		}
		newMobj = P_SpawnMobj(mobj->x, mobj->y, z, moType);
		if(P_TestMobjLocation(newMobj) == false)
		{ // Didn't fit
			P_RemoveMobj(newMobj);
		}
		else
		{
			newMobj->angle = angle;
			if(fog == true)
			{
				fogMobj = P_SpawnMobj(mobj->x, mobj->y,
					mobj->z+TELEFOGHEIGHT, MT_TFOG);
				S_StartSound(fogMobj, SFX_TELEPORT);
			}
			newMobj->flags2 |= MF2_DROPPED; // Don't respawn
			if (newMobj->flags2&MF2_FLOATBOB)
			{
				newMobj->special1 = newMobj->z - newMobj->floorz;
			}
			success = true;
		}
	}
	return success;
}

//==========================================================================
//
// EV_ThingActivate
//
//==========================================================================

boolean EV_ThingActivate(int tid)
{
	mobj_t *mobj;
	int searcher;
	boolean success;

	success = false;
	searcher = -1;
	while((mobj = P_FindMobjFromTID(tid, &searcher)) != NULL)
	{
		if(ActivateThing(mobj) == true)
		{
			success = true;
		}
	}
	return success;
}

//==========================================================================
//
// EV_ThingDeactivate
//
//==========================================================================

boolean EV_ThingDeactivate(int tid)
{
	mobj_t *mobj;
	int searcher;
	boolean success;

	success = false;
	searcher = -1;
	while((mobj = P_FindMobjFromTID(tid, &searcher)) != NULL)
	{
		if(DeactivateThing(mobj) == true)
		{
			success = true;
		}
	}
	return success;
}

//==========================================================================
//
// EV_ThingRemove
//
//==========================================================================

boolean EV_ThingRemove(int tid)
{
	mobj_t *mobj;
	int searcher;
	boolean success;

	success = false;
	searcher = -1;
	while((mobj = P_FindMobjFromTID(tid, &searcher)) != NULL)
	{
		if (mobj->type == MT_BRIDGE)
		{
			A_BridgeRemove(mobj);
			return true;
		}
		P_RemoveMobj(mobj);
		success = true;
	}
	return success;
}

//==========================================================================
//
// EV_ThingDestroy
//
//==========================================================================

boolean EV_ThingDestroy(int tid)
{
	mobj_t *mobj;
	int searcher;
	boolean success;

	success = false;
	searcher = -1;
	while((mobj = P_FindMobjFromTID(tid, &searcher)) != NULL)
	{
		if(mobj->flags&MF_SHOOTABLE)
		{
			P_DamageMobj(mobj, NULL, NULL, 10000);
			success = true;
		}
	}
	return success;
}

//==========================================================================
//
// EV_ThingMove
//
// arg[0] = tid
// arg[1] = speed
// arg[2] = angle (255 = use mobj angle)
// arg[3] = distance (pixels>>2)
//
//==========================================================================

/*
boolean EV_ThingMove(byte *args)
{
	return false;
}
*/

//==========================================================================
//
// ActivateThing
//
//==========================================================================

static boolean ActivateThing(mobj_t *mobj)
{
	if(mobj->flags&MF_COUNTKILL)
	{ // Monster
		if(mobj->flags2&MF2_DORMANT)
		{
			mobj->flags2 &= ~MF2_DORMANT;
			mobj->tics = 1;
			return true;
		}
		return false;
	}
	switch(mobj->type)
	{
		case MT_ZTWINEDTORCH:
		case MT_ZTWINEDTORCH_UNLIT:
			P_SetMobjState(mobj, S_ZTWINEDTORCH_1);
			S_StartSound(mobj, SFX_IGNITE);
			break;
		case MT_ZWALLTORCH:
		case MT_ZWALLTORCH_UNLIT:
			P_SetMobjState(mobj, S_ZWALLTORCH1);
			S_StartSound(mobj, SFX_IGNITE);
			break;
		case MT_ZGEMPEDESTAL:
			P_SetMobjState(mobj, S_ZGEMPEDESTAL2);
			break;
		case MT_ZWINGEDSTATUENOSKULL:
			P_SetMobjState(mobj, S_ZWINGEDSTATUENOSKULL2);
			break;
		case MT_THRUSTFLOOR_UP:
		case MT_THRUSTFLOOR_DOWN:
			if (mobj->args[0]==0)
			{
				S_StartSound(mobj, SFX_THRUSTSPIKE_LOWER);
				mobj->flags2 &= ~MF2_DONTDRAW;
				if (mobj->args[1])
					P_SetMobjState(mobj, S_BTHRUSTRAISE1);
				else
					P_SetMobjState(mobj, S_THRUSTRAISE1);
			}
			break;
		case MT_ZFIREBULL:
		case MT_ZFIREBULL_UNLIT:
			P_SetMobjState(mobj, S_ZFIREBULL_BIRTH);
			S_StartSound(mobj, SFX_IGNITE);
			break;
		case MT_ZBELL:
			if(mobj->health > 0)
			{
				P_DamageMobj(mobj, NULL, NULL, 10); // 'ring' the bell
			}
			break;
		case MT_ZCAULDRON:
		case MT_ZCAULDRON_UNLIT:
			P_SetMobjState(mobj, S_ZCAULDRON1);
			S_StartSound(mobj, SFX_IGNITE);
			break;
		case MT_FLAME_SMALL:
			S_StartSound(mobj, SFX_IGNITE);
			P_SetMobjState(mobj, S_FLAME_SMALL1);
			break;
		case MT_FLAME_LARGE:
			S_StartSound(mobj, SFX_IGNITE);
			P_SetMobjState(mobj, S_FLAME_LARGE1);
			break;
		case MT_BAT_SPAWNER:
			P_SetMobjState(mobj, S_SPAWNBATS1);
			break;
		default:
			return false;
			break;
	}
	return true;
}

//==========================================================================
//
// DeactivateThing
//
//==========================================================================

static boolean DeactivateThing(mobj_t *mobj)
{
	if(mobj->flags&MF_COUNTKILL)
	{ // Monster
		if(!(mobj->flags2&MF2_DORMANT))
		{
			mobj->flags2 |= MF2_DORMANT;
			mobj->tics = -1;
			return true;
		}
		return false;
	}
	switch(mobj->type)
	{
		case MT_ZTWINEDTORCH:
		case MT_ZTWINEDTORCH_UNLIT:
			P_SetMobjState(mobj, S_ZTWINEDTORCH_UNLIT);
			break;
		case MT_ZWALLTORCH:
		case MT_ZWALLTORCH_UNLIT:
			P_SetMobjState(mobj, S_ZWALLTORCH_U);
			break;
		case MT_THRUSTFLOOR_UP:
		case MT_THRUSTFLOOR_DOWN:
			if (mobj->args[0]==1)
			{
				S_StartSound(mobj, SFX_THRUSTSPIKE_RAISE);
				if (mobj->args[1])
					P_SetMobjState(mobj, S_BTHRUSTLOWER);
				else
					P_SetMobjState(mobj, S_THRUSTLOWER);
			}
			break;
		case MT_ZFIREBULL:
		case MT_ZFIREBULL_UNLIT:
			P_SetMobjState(mobj, S_ZFIREBULL_DEATH);
			break;
		case MT_ZCAULDRON:
		case MT_ZCAULDRON_UNLIT:
			P_SetMobjState(mobj, S_ZCAULDRON_U);
			break;
		case MT_FLAME_SMALL:
			P_SetMobjState(mobj, S_FLAME_SDORM1);
			break;
		case MT_FLAME_LARGE:
			P_SetMobjState(mobj, S_FLAME_LDORM1);
			break;
		case MT_BAT_SPAWNER:
			P_SetMobjState(mobj, S_SPAWNBATS_OFF);
			break;
		default:
			return false;
			break;
	}
	return true;
}
