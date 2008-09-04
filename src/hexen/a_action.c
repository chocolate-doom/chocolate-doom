
//**************************************************************************
//**
//** a_action.c : Heretic 2 : Raven Software, Corp.
//**
//** $RCSfile: a_action.c,v $
//** $Revision: 1.86 $
//** $Date: 96/03/22 13:15:33 $
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

// EXTERNAL DATA DECLARATIONS ----------------------------------------------
extern fixed_t FloatBobOffsets[64];

// PUBLIC DATA DEFINITIONS -------------------------------------------------
int orbitTableX[256]=
{
983025, 982725, 981825, 980340, 978255, 975600, 972330, 968490,
964065, 959070, 953475, 947325, 940590, 933300, 925440, 917025,
908055, 898545, 888495, 877905, 866775, 855135, 842985, 830310,
817155, 803490, 789360, 774735, 759660, 744120, 728130, 711690,
694845, 677565, 659880, 641805, 623340, 604500, 585285, 565725,
545820, 525600, 505050, 484200, 463065, 441645, 419955, 398010,
375840, 353430, 330810, 307995, 285000, 261825, 238485, 215010,
191400, 167685, 143865, 119955, 95970, 71940, 47850, 23745,
-375, -24495, -48600, -72690, -96720, -120705, -144600, -168420,
-192150, -215745, -239220, -262545, -285720, -308715, -331530, -354135,
-376530, -398700, -420630, -442320, -463725, -484860, -505695, -526230,
-546450, -566340, -585885, -605085, -623925, -642375, -660435, -678105,
-695370, -712215, -728625, -744600, -760125, -775200, -789795, -803925,
-817575, -830715, -843375, -855510, -867135, -878235, -888810, -898845,
-908340, -917295, -925695, -933540, -940815, -947520, -953670, -959235,
-964215, -968625, -972450, -975690, -978330, -980400, -981870, -982740,
-983025, -982725, -981825, -980340, -978255, -975600, -972330, -968490,
-964065, -959070, -953475, -947325, -940590, -933300, -925440, -917025,
-908055, -898545, -888495, -877905, -866775, -855135, -842985, -830310,
-817155, -803490, -789360, -774735, -759660, -744120, -728130, -711690,
-694845, -677565, -659880, -641805, -623340, -604485, -585285, -565725,
-545820, -525600, -505050, -484200, -463065, -441645, -419955, -398010,
-375840, -353430, -330810, -307995, -285000, -261825, -238485, -215010,
-191400, -167685, -143865, -119955, -95970, -71940, -47850, -23745,
375, 24495, 48600, 72690, 96720, 120705, 144600, 168420,
192150, 215745, 239220, 262545, 285720, 308715, 331530, 354135,
376530, 398700, 420630, 442320, 463725, 484860, 505695, 526230,
546450, 566340, 585885, 605085, 623925, 642375, 660435, 678105,
695370, 712215, 728625, 744600, 760125, 775200, 789795, 803925,
817575, 830715, 843375, 855510, 867135, 878235, 888810, 898845,
908340, 917295, 925695, 933540, 940815, 947520, 953670, 959235,
964215, 968625, 972450, 975690, 978330, 980400, 981870, 982740
};

int orbitTableY[256]=
{
375, 24495, 48600, 72690, 96720, 120705, 144600, 168420,
192150, 215745, 239220, 262545, 285720, 308715, 331530, 354135,
376530, 398700, 420630, 442320, 463725, 484860, 505695, 526230,
546450, 566340, 585885, 605085, 623925, 642375, 660435, 678105,
695370, 712215, 728625, 744600, 760125, 775200, 789795, 803925,
817575, 830715, 843375, 855510, 867135, 878235, 888810, 898845,
908340, 917295, 925695, 933540, 940815, 947520, 953670, 959235,
964215, 968625, 972450, 975690, 978330, 980400, 981870, 982740,
983025, 982725, 981825, 980340, 978255, 975600, 972330, 968490,
964065, 959070, 953475, 947325, 940590, 933300, 925440, 917025,
908055, 898545, 888495, 877905, 866775, 855135, 842985, 830310,
817155, 803490, 789360, 774735, 759660, 744120, 728130, 711690,
694845, 677565, 659880, 641805, 623340, 604500, 585285, 565725,
545820, 525600, 505050, 484200, 463065, 441645, 419955, 398010,
375840, 353430, 330810, 307995, 285000, 261825, 238485, 215010,
191400, 167685, 143865, 119955, 95970, 71940, 47850, 23745,
-375, -24495, -48600, -72690, -96720, -120705, -144600, -168420,
-192150, -215745, -239220, -262545, -285720, -308715, -331530, -354135,
-376530, -398700, -420630, -442320, -463725, -484860, -505695, -526230,
-546450, -566340, -585885, -605085, -623925, -642375, -660435, -678105,
-695370, -712215, -728625, -744600, -760125, -775200, -789795, -803925,
-817575, -830715, -843375, -855510, -867135, -878235, -888810, -898845,
-908340, -917295, -925695, -933540, -940815, -947520, -953670, -959235,
-964215, -968625, -972450, -975690, -978330, -980400, -981870, -982740,
-983025, -982725, -981825, -980340, -978255, -975600, -972330, -968490,
-964065, -959070, -953475, -947325, -940590, -933300, -925440, -917025,
-908055, -898545, -888495, -877905, -866775, -855135, -842985, -830310,
-817155, -803490, -789360, -774735, -759660, -744120, -728130, -711690,
-694845, -677565, -659880, -641805, -623340, -604485, -585285, -565725,
-545820, -525600, -505050, -484200, -463065, -441645, -419955, -398010,
-375840, -353430, -330810, -307995, -285000, -261825, -238485, -215010,
-191400, -167685, -143865, -119955, -95970, -71940, -47850, -23745
};

// PRIVATE DATA DEFINITIONS ------------------------------------------------

// CODE --------------------------------------------------------------------

//--------------------------------------------------------------------------
//
// Environmental Action routines
//
//--------------------------------------------------------------------------

//==========================================================================
//
// A_DripBlood
//
//==========================================================================

/*
void A_DripBlood(mobj_t *actor)
{
	mobj_t *mo;

	mo = P_SpawnMobj(actor->x+((P_Random()-P_Random())<<11),
		actor->y+((P_Random()-P_Random())<<11), actor->z, MT_BLOOD);
	mo->momx = (P_Random()-P_Random())<<10;
	mo->momy = (P_Random()-P_Random())<<10;
	mo->flags2 |= MF2_LOGRAV;
}
*/

//============================================================================
//
// A_PotteryExplode
//
//============================================================================

void A_PotteryExplode(mobj_t *actor)
{
	mobj_t *mo=NULL;
	int i;

	for(i = (P_Random()&3)+3; i; i--)
	{
		mo = P_SpawnMobj(actor->x, actor->y, actor->z, MT_POTTERYBIT1);
		P_SetMobjState(mo, mo->info->spawnstate+(P_Random()%5));
		if(mo)
		{
			mo->momz = ((P_Random()&7)+5)*(3*FRACUNIT/4);
			mo->momx = (P_Random()-P_Random())<<(FRACBITS-6);
			mo->momy = (P_Random()-P_Random())<<(FRACBITS-6);
		}
	}
	S_StartSound(mo, SFX_POTTERY_EXPLODE);
	if(actor->args[0])
	{ // Spawn an item
		if(!nomonsters 
		|| !(mobjinfo[TranslateThingType[actor->args[0]]].flags&MF_COUNTKILL))
		{ // Only spawn monsters if not -nomonsters
			P_SpawnMobj(actor->x, actor->y, actor->z,
				TranslateThingType[actor->args[0]]);
		}
	}
	P_RemoveMobj(actor);
}

//============================================================================
//
// A_PotteryChooseBit
//
//============================================================================

void A_PotteryChooseBit(mobj_t *actor)
{
	P_SetMobjState(actor, actor->info->deathstate+(P_Random()%5)+1);
	actor->tics = 256+(P_Random()<<1);
}

//============================================================================
//
// A_PotteryCheck
//
//============================================================================

void A_PotteryCheck(mobj_t *actor)
{
	int i;
	mobj_t *pmo;

	if(!netgame)
	{
		pmo = players[consoleplayer].mo;
		if(P_CheckSight(actor, pmo) && (abs(R_PointToAngle2(pmo->x,
			pmo->y, actor->x, actor->y)-pmo->angle) <= ANGLE_45))
		{ // Previous state (pottery bit waiting state)
			P_SetMobjState(actor, actor->state-&states[0]-1);
		}
		else
		{
			return;
		}
	}
	else
	{
		for(i = 0; i < MAXPLAYERS; i++)
		{
			if(!playeringame[i])
			{
				continue;
			}
			pmo = players[i].mo;
			if(P_CheckSight(actor, pmo) && (abs(R_PointToAngle2(pmo->x,
				pmo->y, actor->x, actor->y)-pmo->angle) <= ANGLE_45))
			{ // Previous state (pottery bit waiting state)
				P_SetMobjState(actor, actor->state-&states[0]-1);
				return;
			}
		}
	}		
}

//============================================================================
//
// A_CorpseBloodDrip
//
//============================================================================

void A_CorpseBloodDrip(mobj_t *actor)
{
	if(P_Random() > 128)
	{
		return;
	}
	P_SpawnMobj(actor->x, actor->y, actor->z+actor->height/2, 
		MT_CORPSEBLOODDRIP);
}

//============================================================================
//
// A_CorpseExplode
//
//============================================================================

void A_CorpseExplode(mobj_t *actor)
{
	mobj_t *mo;
	int i;

	for(i = (P_Random()&3)+3; i; i--)
	{
		mo = P_SpawnMobj(actor->x, actor->y, actor->z, MT_CORPSEBIT);
		P_SetMobjState(mo, mo->info->spawnstate+(P_Random()%3));
		if(mo)
		{
			mo->momz = ((P_Random()&7)+5)*(3*FRACUNIT/4);
			mo->momx = (P_Random()-P_Random())<<(FRACBITS-6);
			mo->momy = (P_Random()-P_Random())<<(FRACBITS-6);
		}
	}
	// Spawn a skull
	mo = P_SpawnMobj(actor->x, actor->y, actor->z, MT_CORPSEBIT);
	P_SetMobjState(mo, S_CORPSEBIT_4);
	if(mo)
	{
		mo->momz = ((P_Random()&7)+5)*(3*FRACUNIT/4);
		mo->momx = (P_Random()-P_Random())<<(FRACBITS-6);
		mo->momy = (P_Random()-P_Random())<<(FRACBITS-6);
		S_StartSound(mo, SFX_FIRED_DEATH);
	}
	P_RemoveMobj(actor);
}

//============================================================================
//
// A_LeafSpawn
//
//============================================================================

void A_LeafSpawn(mobj_t *actor)
{
	mobj_t *mo;
	int i;

	for(i = (P_Random()&3)+1; i; i--)
	{
		mo = P_SpawnMobj(actor->x+((P_Random()-P_Random())<<14), actor->y+
			((P_Random()-P_Random())<<14), actor->z+(P_Random()<<14), 
			MT_LEAF1+(P_Random()&1));
		if(mo)
		{
			P_ThrustMobj(mo, actor->angle, (P_Random()<<9)+3*FRACUNIT);
			mo->target = actor;
			mo->special1 = 0;
		}
	}
}

//============================================================================
//
// A_LeafThrust
//
//============================================================================

void A_LeafThrust(mobj_t *actor)
{
	if(P_Random() > 96)
	{
		return;
	}
	actor->momz += (P_Random()<<9)+FRACUNIT;
}

//============================================================================
//
// A_LeafCheck
//
//============================================================================

void A_LeafCheck(mobj_t *actor)
{
	actor->special1++;
	if(actor->special1 >= 20)
	{
		P_SetMobjState(actor, S_NULL);
		return;
	}
	if(P_Random() > 64)
	{
		if(!actor->momx && !actor->momy)
		{
			P_ThrustMobj(actor, actor->target->angle,
				(P_Random()<<9)+FRACUNIT);
		}
		return;
	}
	P_SetMobjState(actor, S_LEAF1_8);
	actor->momz = (P_Random()<<9)+FRACUNIT;
	P_ThrustMobj(actor, actor->target->angle, (P_Random()<<9)+2*FRACUNIT);
	actor->flags |= MF_MISSILE;
}

/*
#define ORBIT_RADIUS	(15*FRACUNIT)
void GenerateOrbitTable(void)
{
	int angle;

	for (angle=0; angle<256; angle++)
	{
		orbitTableX[angle] = FixedMul(ORBIT_RADIUS, finecosine[angle<<5]);
		orbitTableY[angle] = FixedMul(ORBIT_RADIUS, finesine[angle<<5]);
	}

	printf("int orbitTableX[256]=\n{\n");
	for (angle=0; angle<256; angle+=8)
	{
		printf("%d, %d, %d, %d, %d, %d, %d, %d,\n",
			orbitTableX[angle],
			orbitTableX[angle+1],
			orbitTableX[angle+2],
			orbitTableX[angle+3],
			orbitTableX[angle+4],
			orbitTableX[angle+5],
			orbitTableX[angle+6],
			orbitTableX[angle+7]);
	}
	printf("};\n\n");

	printf("int orbitTableY[256]=\n{\n");
	for (angle=0; angle<256; angle+=8)
	{
		printf("%d, %d, %d, %d, %d, %d, %d, %d,\n",
			orbitTableY[angle],
			orbitTableY[angle+1],
			orbitTableY[angle+2],
			orbitTableY[angle+3],
			orbitTableY[angle+4],
			orbitTableY[angle+5],
			orbitTableY[angle+6],
			orbitTableY[angle+7]);
	}
	printf("};\n");
}
*/

// New bridge stuff
//	Parent
//		special1	true == removing from world
//
//	Child
//		target		pointer to center mobj
//		args[0]		angle of ball

void A_BridgeOrbit(mobj_t *actor)
{
	if (actor->target->special1)
	{
		P_SetMobjState(actor, S_NULL);
	}
	actor->args[0]+=3;
	actor->x = actor->target->x + orbitTableX[actor->args[0]];
	actor->y = actor->target->y + orbitTableY[actor->args[0]];
	actor->z = actor->target->z;
}


void A_BridgeInit(mobj_t *actor)
{
	byte startangle;
	mobj_t *ball1, *ball2, *ball3;
	fixed_t cx,cy,cz;

//	GenerateOrbitTable();

	cx = actor->x;
	cy = actor->y;
	cz = actor->z;
	startangle = P_Random();
	actor->special1 = 0;

	// Spawn triad into world
	ball1 = P_SpawnMobj(cx, cy, cz, MT_BRIDGEBALL);
	ball1->args[0] = startangle;
	ball1->target = actor;

	ball2 = P_SpawnMobj(cx, cy, cz, MT_BRIDGEBALL);
	ball2->args[0] = (startangle+85)&255;
	ball2->target = actor;

	ball3 = P_SpawnMobj(cx, cy, cz, MT_BRIDGEBALL);
	ball3->args[0] = (startangle+170)&255;
	ball3->target = actor;

	A_BridgeOrbit(ball1);
	A_BridgeOrbit(ball2);
	A_BridgeOrbit(ball3);
}

void A_BridgeRemove(mobj_t *actor)
{
	actor->special1 = true;		// Removing the bridge
	actor->flags &= ~MF_SOLID;
	P_SetMobjState(actor, S_FREE_BRIDGE1);
}


//==========================================================================
//
// A_GhostOn
//
//==========================================================================

/*
void A_GhostOn(mobj_t *actor)
{
	actor->flags |= MF_SHADOW;
}
*/

//==========================================================================
//
// A_GhostOff
//
//==========================================================================

/*
void A_GhostOff(mobj_t *actor)
{
	actor->flags &= ~MF_SHADOW;
}
*/

//==========================================================================
//
// A_HideThing
//
//==========================================================================

void A_HideThing(mobj_t *actor)
{
	actor->flags2 |= MF2_DONTDRAW;
}

//==========================================================================
//
// A_UnHideThing
//
//==========================================================================

void A_UnHideThing(mobj_t *actor)
{
	actor->flags2 &= ~MF2_DONTDRAW;
}

//==========================================================================
//
// A_SetShootable
//
//==========================================================================

void A_SetShootable(mobj_t *actor)
{
	actor->flags2 &= ~MF2_NONSHOOTABLE;
	actor->flags |= MF_SHOOTABLE;
}

//==========================================================================
//
// A_UnSetShootable
//
//==========================================================================

void A_UnSetShootable(mobj_t *actor)
{
	actor->flags2 |= MF2_NONSHOOTABLE;
	actor->flags &= ~MF_SHOOTABLE;
}

//==========================================================================
//
// A_SetAltShadow
//
//==========================================================================

void A_SetAltShadow(mobj_t *actor)
{
	actor->flags &= ~MF_SHADOW;
	actor->flags |= MF_ALTSHADOW;
}

//==========================================================================
//
// A_UnSetAltShadow
//
//==========================================================================

/*
void A_UnSetAltShadow(mobj_t *actor)
{
	actor->flags &= ~MF_ALTSHADOW;
}
*/

//--------------------------------------------------------------------------
//
// Sound Action Routines
//
//--------------------------------------------------------------------------

//==========================================================================
//
// A_ContMobjSound
//
//==========================================================================

void A_ContMobjSound(mobj_t *actor)
{
	switch(actor->type)
	{
		case MT_SERPENTFX:
			S_StartSound(actor, SFX_SERPENTFX_CONTINUOUS);
			break;
		case MT_HAMMER_MISSILE:
			S_StartSound(actor, SFX_FIGHTER_HAMMER_CONTINUOUS);
			break;
		case MT_QUAKE_FOCUS:
			S_StartSound(actor, SFX_EARTHQUAKE);
			break;
		default:
			break;
	}
}

//==========================================================================
//
// PROC A_ESound
//
//==========================================================================

void A_ESound(mobj_t *mo)
{
	int sound;

	switch(mo->type)
	{
		case MT_SOUNDWIND:
			sound = SFX_WIND;
			break;
		default:
			sound = SFX_NONE;
			break;
	}
	S_StartSound(mo, sound);	
}



//==========================================================================
// Summon Minotaur -- see p_enemy for variable descriptions
//==========================================================================


void A_Summon(mobj_t *actor)
{
	mobj_t *mo;
	mobj_t *master;

	mo = P_SpawnMobj(actor->x, actor->y, actor->z, MT_MINOTAUR);
	if (mo)
	{
		if(P_TestMobjLocation(mo) == false || !actor->special1)
		{ // Didn't fit - change back to artifact
			P_SetMobjState(mo, S_NULL);
			mo = P_SpawnMobj(actor->x, actor->y, actor->z, MT_SUMMONMAULATOR);
			if (mo) mo->flags2 |= MF2_DROPPED;
			return;
		}

		memcpy((void *)mo->args, &leveltime, sizeof(leveltime));
		master = (mobj_t *)actor->special1;
		if (master->flags&MF_CORPSE)
		{	// Master dead
			mo->special1 = 0;		// No master
		}
		else
		{
			mo->special1 = actor->special1;		// Pointer to master (mobj_t *)
			P_GivePower(master->player, pw_minotaur);
		}

		// Make smoke puff
		P_SpawnMobj(actor->x, actor->y, actor->z, MT_MNTRSMOKE);
		S_StartSound(actor, SFX_MAULATOR_ACTIVE);
	}
}



//==========================================================================
// Fog Variables:
//
//		args[0]		Speed (0..10) of fog
//		args[1]		Angle of spread (0..128)
// 		args[2]		Frequency of spawn (1..10)
//		args[3]		Lifetime countdown
//		args[4]		Boolean: fog moving?
//		special1		Internal:  Counter for spawn frequency
//		special2		Internal:  Index into floatbob table
//
//==========================================================================

void A_FogSpawn(mobj_t *actor)
{
	mobj_t *mo=NULL;
	angle_t delta;

	if (actor->special1-- > 0) return;

	actor->special1 = actor->args[2];		// Reset frequency count

	switch(P_Random()%3)
	{
		case 0:
			mo = P_SpawnMobj(actor->x, actor->y, actor->z, MT_FOGPATCHS);
			break;
		case 1:
			mo = P_SpawnMobj(actor->x, actor->y, actor->z, MT_FOGPATCHM);
			break;
		case 2:
			mo = P_SpawnMobj(actor->x, actor->y, actor->z, MT_FOGPATCHL);
			break;
	}

	if (mo)
	{
		delta = actor->args[1];
		if (delta==0) delta=1;
		mo->angle = actor->angle + (((P_Random()%delta)-(delta>>1))<<24);
		mo->target = actor;
		if (actor->args[0] < 1) actor->args[0] = 1;
		mo->args[0] = (P_Random() % (actor->args[0]))+1;	// Random speed
		mo->args[3] = actor->args[3];						// Set lifetime
		mo->args[4] = 1;									// Set to moving
		mo->special2 = P_Random()&63;
	}
}


void A_FogMove(mobj_t *actor)
{
	int speed = actor->args[0]<<FRACBITS;
	angle_t angle;
	int weaveindex;

	if (!(actor->args[4])) return;

	if (actor->args[3]-- <= 0)
	{
		P_SetMobjStateNF(actor, actor->info->deathstate);
		return;
	}

	if ((actor->args[3] % 4) == 0)
	{
		weaveindex = actor->special2;
		actor->z += FloatBobOffsets[weaveindex]>>1;
		actor->special2 = (weaveindex+1)&63;
	}

	angle = actor->angle>>ANGLETOFINESHIFT;
	actor->momx = FixedMul(speed, finecosine[angle]);
	actor->momy = FixedMul(speed, finesine[angle]);
}

//===========================================================================
//
// A_PoisonBagInit
//
//===========================================================================

void A_PoisonBagInit(mobj_t *actor)
{
	mobj_t *mo;

	mo = P_SpawnMobj(actor->x, actor->y, actor->z+28*FRACUNIT,
		MT_POISONCLOUD);
	if(!mo)
	{
		return;
	}
	mo->momx = 1; // missile objects must move to impact other objects
	mo->special1 = 24+(P_Random()&7);
	mo->special2 = 0;
	mo->target = actor->target;
	mo->radius = 20*FRACUNIT;
	mo->height = 30*FRACUNIT;
	mo->flags &= ~MF_NOCLIP;
}

//===========================================================================
//
// A_PoisonBagCheck
//
//===========================================================================

void A_PoisonBagCheck(mobj_t *actor)
{
	if(!--actor->special1)
	{
		P_SetMobjState(actor, S_POISONCLOUD_X1);
	}
	else
	{
		return;
	}
}

//===========================================================================
//
// A_PoisonBagDamage
//
//===========================================================================

void A_PoisonBagDamage(mobj_t *actor)
{
	int bobIndex;
	
	extern void A_Explode(mobj_t *actor);

	A_Explode(actor);	

	bobIndex = actor->special2;
	actor->z += FloatBobOffsets[bobIndex]>>4;
	actor->special2 = (bobIndex+1)&63;
}

//===========================================================================
//
// A_PoisonShroom
//
//===========================================================================

void A_PoisonShroom(mobj_t *actor)
{
	actor->tics = 128+(P_Random()<<1);
}

//===========================================================================
//
// A_CheckThrowBomb
//
//===========================================================================

void A_CheckThrowBomb(mobj_t *actor)
{
	if(abs(actor->momx) < 1.5*FRACUNIT && abs(actor->momy) < 1.5*FRACUNIT
		&& actor->momz < 2*FRACUNIT
		&& actor->state == &states[S_THROWINGBOMB6])
	{
		P_SetMobjState(actor, S_THROWINGBOMB7);
		actor->z = actor->floorz;
		actor->momz = 0;
		actor->flags2 &= ~MF2_FLOORBOUNCE;
		actor->flags &= ~MF_MISSILE;
	}
	if(!--actor->health)
	{
		P_SetMobjState(actor, actor->info->deathstate);
	}
}

//===========================================================================
// Quake variables
//
//		args[0]		Intensity on richter scale (2..9)
//		args[1]		Duration in tics
//		args[2]		Radius for damage
//		args[3]		Radius for tremor
//		args[4]		TID of map thing for focus of quake
//
//===========================================================================

//===========================================================================
//
// A_LocalQuake
//
//===========================================================================

boolean A_LocalQuake(byte *args, mobj_t *actor)
{
	mobj_t *focus, *target;
	int lastfound=0;
	int success=false;

	actor=actor;	// suppress warning

	// Find all quake foci
	do
	{
		target = P_FindMobjFromTID(args[4], &lastfound);
		if (target)
		{
			focus = P_SpawnMobj(target->x,
								target->y,
								target->z, MT_QUAKE_FOCUS);
			if (focus)
			{
				focus->args[0] = args[0];
				focus->args[1] = args[1]>>1;	// decremented every 2 tics
				focus->args[2] = args[2];
				focus->args[3] = args[3];
				focus->args[4] = args[4];
				success = true;
			}
		}
	}while(target != NULL);

	return(success);
}


//===========================================================================
//
// A_Quake
//
//===========================================================================
int	localQuakeHappening[MAXPLAYERS];

void A_Quake(mobj_t *actor)
{
	angle_t an;
	player_t *player;
	mobj_t *victim;
	int richters = actor->args[0];
	int playnum;
	fixed_t dist;

	if (actor->args[1]-- > 0)
	{
		for (playnum=0; playnum < MAXPLAYERS; playnum++)
		{
			player = &players[playnum];
			if (!playeringame[playnum]) continue;

			victim = player->mo;
			dist = P_AproxDistance(actor->x - victim->x,
						actor->y - victim->y) >> (FRACBITS+6);
			// Tested in tile units (64 pixels)
			if (dist < actor->args[3])		// In tremor radius
			{
				localQuakeHappening[playnum] = richters;
			}
			// Check if in damage radius
			if ((dist < actor->args[2]) &&
				(victim->z <= victim->floorz))
			{
				if (P_Random() < 50)
				{
					P_DamageMobj(victim, NULL, NULL, HITDICE(1));
				}
				// Thrust player around
				an = victim->angle + ANGLE_1*P_Random();
				P_ThrustMobj(victim,an,richters<<(FRACBITS-1));
			}
		}
	}
	else
	{
		for (playnum=0; playnum < MAXPLAYERS; playnum++)
		{
			localQuakeHappening[playnum] = false;
		}
		P_SetMobjState(actor, S_NULL);
	}
}




//===========================================================================
//
// Teleport other stuff
//
//===========================================================================

#define TELEPORT_LIFE 1

void A_TeloSpawnA(mobj_t *actor)
{
	mobj_t *mo;

	mo = P_SpawnMobj(actor->x, actor->y, actor->z, MT_TELOTHER_FX2);
	if (mo)
	{
		mo->special1 = TELEPORT_LIFE;			// Lifetime countdown
		mo->angle = actor->angle;
		mo->target = actor->target;
		mo->momx = actor->momx>>1;
		mo->momy = actor->momy>>1;
		mo->momz = actor->momz>>1;
	}
}

void A_TeloSpawnB(mobj_t *actor)
{
	mobj_t *mo;

	mo = P_SpawnMobj(actor->x, actor->y, actor->z, MT_TELOTHER_FX3);
	if (mo)
	{
		mo->special1 = TELEPORT_LIFE;			// Lifetime countdown
		mo->angle = actor->angle;
		mo->target = actor->target;
		mo->momx = actor->momx>>1;
		mo->momy = actor->momy>>1;
		mo->momz = actor->momz>>1;
	}
}

void A_TeloSpawnC(mobj_t *actor)
{
	mobj_t *mo;

	mo = P_SpawnMobj(actor->x, actor->y, actor->z, MT_TELOTHER_FX4);
	if (mo)
	{
		mo->special1 = TELEPORT_LIFE;			// Lifetime countdown
		mo->angle = actor->angle;
		mo->target = actor->target;
		mo->momx = actor->momx>>1;
		mo->momy = actor->momy>>1;
		mo->momz = actor->momz>>1;
	}
}

void A_TeloSpawnD(mobj_t *actor)
{
	mobj_t *mo;

	mo = P_SpawnMobj(actor->x, actor->y, actor->z, MT_TELOTHER_FX5);
	if (mo)
	{
		mo->special1 = TELEPORT_LIFE;			// Lifetime countdown
		mo->angle = actor->angle;
		mo->target = actor->target;
		mo->momx = actor->momx>>1;
		mo->momy = actor->momy>>1;
		mo->momz = actor->momz>>1;
	}
}

void A_CheckTeleRing(mobj_t *actor)
{
	if (actor->special1-- <= 0)
	{
		P_SetMobjState(actor, actor->info->deathstate);
	}
}




// Dirt stuff

void P_SpawnDirt(mobj_t *actor, fixed_t radius)
{
	fixed_t x,y,z;
	int dtype=0;
	mobj_t *mo;
	angle_t angle;

	angle = P_Random()<<5;		// <<24 >>19
	x = actor->x + FixedMul(radius,finecosine[angle]);
	y = actor->y + FixedMul(radius,finesine[angle]);
//	x = actor->x + ((P_Random()-P_Random())%radius)<<FRACBITS;
//	y = actor->y + ((P_Random()-P_Random()<<FRACBITS)%radius);
	z = actor->z + (P_Random()<<9) + FRACUNIT;
	switch(P_Random()%6)
	{
		case 0:
			dtype = MT_DIRT1;
			break;
		case 1:
			dtype = MT_DIRT2;
			break;
		case 2:
			dtype = MT_DIRT3;
			break;
		case 3:
			dtype = MT_DIRT4;
			break;
		case 4:
			dtype = MT_DIRT5;
			break;
		case 5:
			dtype = MT_DIRT6;
			break;
	}
	mo = P_SpawnMobj(x,y,z,dtype);
	if (mo)
	{
		mo->momz = P_Random()<<10;
	}
}




//===========================================================================
//
// Thrust floor stuff
//
// Thrust Spike Variables
//		special1		pointer to dirt clump mobj
//		special2		speed of raise
//		args[0]		0 = lowered,  1 = raised
//		args[1]		0 = normal,   1 = bloody
//===========================================================================

void A_ThrustInitUp(mobj_t *actor)
{
	actor->special2 = 5;		// Raise speed
	actor->args[0] = 1;		// Mark as up
	actor->floorclip = 0;
	actor->flags = MF_SOLID;
	actor->flags2 = MF2_NOTELEPORT|MF2_FLOORCLIP;
	actor->special1 = 0L;
}

void A_ThrustInitDn(mobj_t *actor)
{
	mobj_t *mo;
	actor->special2 = 5;		// Raise speed
	actor->args[0] = 0;		// Mark as down
	actor->floorclip = actor->info->height;
	actor->flags = 0;
	actor->flags2 = MF2_NOTELEPORT|MF2_FLOORCLIP|MF2_DONTDRAW;
	mo = P_SpawnMobj(actor->x, actor->y, actor->z, MT_DIRTCLUMP);
	actor->special1 = (int)mo;
}


void A_ThrustRaise(mobj_t *actor)
{
	if (A_RaiseMobj(actor))
	{	// Reached it's target height
		actor->args[0] = 1;
		if (actor->args[1])
			P_SetMobjStateNF(actor, S_BTHRUSTINIT2_1);
		else
			P_SetMobjStateNF(actor, S_THRUSTINIT2_1);
	}

	// Lose the dirt clump
	if ((actor->floorclip < actor->height) && actor->special1)
	{
		P_RemoveMobj( (mobj_t *)actor->special1 );
		actor->special1 = 0;
	}

	// Spawn some dirt
	if (P_Random()<40)
		P_SpawnDirt(actor, actor->radius);
	actor->special2++;							// Increase raise speed
}

void A_ThrustLower(mobj_t *actor)
{
	if (A_SinkMobj(actor))
	{
		actor->args[0] = 0;
		if (actor->args[1])
			P_SetMobjStateNF(actor, S_BTHRUSTINIT1_1);
		else
			P_SetMobjStateNF(actor, S_THRUSTINIT1_1);
	}
}

void A_ThrustBlock(mobj_t *actor)
{
	actor->flags |= MF_SOLID;
}

void A_ThrustImpale(mobj_t *actor)
{
	// Impale all shootables in radius
	PIT_ThrustSpike(actor);
}

//===========================================================================
//
// A_SoAExplode - Suit of Armor Explode
//
//===========================================================================

void A_SoAExplode(mobj_t *actor)
{
	mobj_t *mo;
	int i;

	for(i = 0; i < 10; i++)
	{
		mo = P_SpawnMobj(actor->x+((P_Random()-128)<<12), 
			actor->y+((P_Random()-128)<<12), 
			actor->z+(P_Random()*actor->height/256), MT_ZARMORCHUNK);
		P_SetMobjState(mo, mo->info->spawnstate+i);
		if(mo)
		{
			mo->momz = ((P_Random()&7)+5)*FRACUNIT;
			mo->momx = (P_Random()-P_Random())<<(FRACBITS-6);
			mo->momy = (P_Random()-P_Random())<<(FRACBITS-6);
		}
	}
	if(actor->args[0])
	{ // Spawn an item
		if(!nomonsters 
		|| !(mobjinfo[TranslateThingType[actor->args[0]]].flags&MF_COUNTKILL))
		{ // Only spawn monsters if not -nomonsters
			P_SpawnMobj(actor->x, actor->y, actor->z,
				TranslateThingType[actor->args[0]]);
		}
	}
	S_StartSound(mo, SFX_SUITOFARMOR_BREAK);
	P_RemoveMobj(actor);
}

//===========================================================================
//
// A_BellReset1
//
//===========================================================================

void A_BellReset1(mobj_t *actor)
{
	actor->flags |= MF_NOGRAVITY;
	actor->height <<= 2;	
}

//===========================================================================
//
// A_BellReset2
//
//===========================================================================

void A_BellReset2(mobj_t *actor)
{
	actor->flags |= MF_SHOOTABLE;
	actor->flags &= ~MF_CORPSE;
	actor->health = 5;
}


//===========================================================================
//
// A_FlameCheck
//
//===========================================================================

void A_FlameCheck(mobj_t *actor)
{
	if(!actor->args[0]--)		// Called every 8 tics
	{
		P_SetMobjState(actor, S_NULL);
	}
}


//===========================================================================
// Bat Spawner Variables
//	special1	frequency counter
//	special2	
//	args[0]		frequency of spawn (1=fastest, 10=slowest)
//	args[1]		spread angle (0..255)
//	args[2]		
//	args[3]		duration of bats (in octics)
//	args[4]		turn amount per move (in degrees)
//
// Bat Variables
//	special2	lifetime counter
//	args[4]		turn amount per move (in degrees)
//===========================================================================

void A_BatSpawnInit(mobj_t *actor)
{
	actor->special1 = 0;	// Frequency count
}

void A_BatSpawn(mobj_t *actor)
{
	mobj_t *mo;
	int delta;
	angle_t angle;

	// Countdown until next spawn
	if (actor->special1-- > 0) return;
	actor->special1 = actor->args[0];		// Reset frequency count

	delta = actor->args[1];
	if (delta==0) delta=1;
	angle = actor->angle + (((P_Random()%delta)-(delta>>1))<<24);
	mo = P_SpawnMissileAngle(actor, MT_BAT, angle, 0);
	if (mo)
	{
		mo->args[0] = P_Random()&63;			// floatbob index
		mo->args[4] = actor->args[4];			// turn degrees
		mo->special2 = actor->args[3]<<3;		// Set lifetime
		mo->target = actor;
	}
}


void A_BatMove(mobj_t *actor)
{
	angle_t newangle;
	fixed_t speed;

	if (actor->special2 < 0)
	{
		P_SetMobjState(actor, actor->info->deathstate);
	}
	actor->special2 -= 2;		// Called every 2 tics

	if (P_Random()<128)
	{
		newangle = actor->angle + ANGLE_1*actor->args[4];
	}
	else
	{
		newangle = actor->angle - ANGLE_1*actor->args[4];
	}

	// Adjust momentum vector to new direction
	newangle >>= ANGLETOFINESHIFT;
	speed = FixedMul(actor->info->speed, P_Random()<<10);
	actor->momx = FixedMul(speed, finecosine[newangle]);
	actor->momy = FixedMul(speed, finesine[newangle]);

	if (P_Random()<15)
		S_StartSound(actor, SFX_BAT_SCREAM);

	// Handle Z movement
	actor->z = actor->target->z + 2*FloatBobOffsets[actor->args[0]];
	actor->args[0] = (actor->args[0]+3)&63;	
}

//===========================================================================
//
// A_TreeDeath
//
//===========================================================================

void A_TreeDeath(mobj_t *actor)
{
	if(!(actor->flags2&MF2_FIREDAMAGE))
	{
		actor->height <<= 2;
		actor->flags |= MF_SHOOTABLE;
		actor->flags &= ~(MF_CORPSE+MF_DROPOFF);
		actor->health = 35;
		return;
	}
	else
	{
		P_SetMobjState(actor, actor->info->meleestate);
	}
}

//===========================================================================
//
// A_NoGravity
//
//===========================================================================

void A_NoGravity(mobj_t *actor)
{
	actor->flags |= MF_NOGRAVITY;
}
