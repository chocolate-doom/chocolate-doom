// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 1993-2008 Raven Software
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
//-----------------------------------------------------------------------------


#include "h2def.h"
#include "sounds.h"

// Music info

/*
musicinfo_t S_music[] =
{
	{ "MUS_E1M1", 0 }, // 1-1
	{ "MUS_E1M2", 0 },
	{ "MUS_E1M3", 0 },
	{ "MUS_E1M4", 0 },
	{ "MUS_E1M5", 0 },
	{ "MUS_E1M6", 0 },
	{ "MUS_E1M7", 0 },
	{ "MUS_E1M8", 0 },
	{ "MUS_E1M9", 0 },
	{ "MUS_E2M1", 0 }, // 2-1
	{ "MUS_E2M2", 0 },
	{ "MUS_E2M3", 0 },
	{ "MUS_E2M4", 0 },
	{ "MUS_E1M4", 0 },
	{ "MUS_E2M6", 0 },
	{ "MUS_E2M7", 0 },
	{ "MUS_E2M8", 0 },
	{ "MUS_E2M9", 0 },
	{ "MUS_E1M1", 0 }, // 3-1
	{ "MUS_E3M2", 0 },
	{ "MUS_E3M3", 0 },
	{ "MUS_E1M6", 0 },
	{ "MUS_E1M3", 0 },
	{ "MUS_E1M2", 0 },
	{ "MUS_E1M5", 0 },
	{ "MUS_E1M9", 0 },
	{ "MUS_E2M6", 0 },
	{ "MUS_E1M6", 0 }, // 4-1
	{ "MUS_TITL", 0 },
	{ "MUS_INTR", 0 },
	{ "MUS_CPTD", 0 }
};
*/

// Sound info

sfxinfo_t S_sfx[] = {
    // tagname, lumpname, priority, usefulness, snd_ptr, lumpnum, numchannels,
    //              pitchshift
    {"", "", 0, -1, NULL, 0, 0, 0}
    ,
    {"PlayerFighterNormalDeath", "", 256, -1, NULL, 0, 2, 1}
    ,
    {"PlayerFighterCrazyDeath", "", 256, -1, NULL, 0, 2, 1}
    ,
    {"PlayerFighterExtreme1Death", "", 256, -1, NULL, 0, 2, 1}
    ,
    {"PlayerFighterExtreme2Death", "", 256, -1, NULL, 0, 2, 1}
    ,
    {"PlayerFighterExtreme3Death", "", 256, -1, NULL, 0, 2, 1}
    ,
    {"PlayerFighterBurnDeath", "", 256, -1, NULL, 0, 2, 1}
    ,
    {"PlayerClericNormalDeath", "", 256, -1, NULL, 0, 2, 1}
    ,
    {"PlayerClericCrazyDeath", "", 256, -1, NULL, 0, 2, 1}
    ,
    {"PlayerClericExtreme1Death", "", 256, -1, NULL, 0, 2, 1}
    ,
    {"PlayerClericExtreme2Death", "", 256, -1, NULL, 0, 2, 1}
    ,
    {"PlayerClericExtreme3Death", "", 256, -1, NULL, 0, 2, 1}
    ,
    {"PlayerClericBurnDeath", "", 256, -1, NULL, 0, 2, 1}
    ,
    {"PlayerMageNormalDeath", "", 256, -1, NULL, 0, 2, 0}
    ,
    {"PlayerMageCrazyDeath", "", 256, -1, NULL, 0, 2, 0}
    ,
    {"PlayerMageExtreme1Death", "", 256, -1, NULL, 0, 2, 0}
    ,
    {"PlayerMageExtreme2Death", "", 256, -1, NULL, 0, 2, 0}
    ,
    {"PlayerMageExtreme3Death", "", 256, -1, NULL, 0, 2, 0}
    ,
    {"PlayerMageBurnDeath", "", 256, -1, NULL, 0, 2, 0}
    ,
    {"PlayerFighterPain", "", 256, -1, NULL, 0, 2, 1}
    ,
    {"PlayerClericPain", "", 256, -1, NULL, 0, 2, 1}
    ,
    {"PlayerMagePain", "", 256, -1, NULL, 0, 2, 0}
    ,
    {"PlayerFighterGrunt", "", 256, -1, NULL, 0, 2, 1}
    ,
    {"PlayerClericGrunt", "", 256, -1, NULL, 0, 2, 1}
    ,
    {"PlayerMageGrunt", "", 256, -1, NULL, 0, 2, 0}
    ,
    {"PlayerLand", "", 32, -1, NULL, 0, 2, 1}
    ,
    {"PlayerPoisonCough", "", 256, -1, NULL, 0, 2, 1}
    ,
    {"PlayerFighterFallingScream", "", 256, -1, NULL, 0, 2, 1}
    ,
    {"PlayerClericFallingScream", "", 256, -1, NULL, 0, 2, 1}
    ,
    {"PlayerMageFallingScream", "", 256, -1, NULL, 0, 2, 0}
    ,
    {"PlayerFallingSplat", "", 256, -1, NULL, 0, 2, 1}
    ,
    {"PlayerFighterFailedUse", "", 256, -1, NULL, 0, 1, 1}
    ,
    {"PlayerClericFailedUse", "", 256, -1, NULL, 0, 1, 1}
    ,
    {"PlayerMageFailedUse", "", 256, -1, NULL, 0, 1, 0}
    ,
    {"PlatformStart", "", 36, -1, NULL, 0, 2, 1}
    ,
    {"PlatformStartMetal", "", 36, -1, NULL, 0, 2, 1}
    ,
    {"PlatformStop", "", 40, -1, NULL, 0, 2, 1}
    ,
    {"StoneMove", "", 32, -1, NULL, 0, 2, 1}
    ,
    {"MetalMove", "", 32, -1, NULL, 0, 2, 1}
    ,
    {"DoorOpen", "", 36, -1, NULL, 0, 2, 1}
    ,
    {"DoorLocked", "", 36, -1, NULL, 0, 2, 1}
    ,
    {"DoorOpenMetal", "", 36, -1, NULL, 0, 2, 1}
    ,
    {"DoorCloseMetal", "", 36, -1, NULL, 0, 2, 1}
    ,
    {"DoorCloseLight", "", 36, -1, NULL, 0, 2, 1}
    ,
    {"DoorCloseHeavy", "", 36, -1, NULL, 0, 2, 1}
    ,
    {"DoorCreak", "", 36, -1, NULL, 0, 2, 1}
    ,
    {"PickupWeapon", "", 36, -1, NULL, 0, 2, 0}
    ,
    {"PickupArtifact", "", 36, -1, NULL, 0, 2, 1}
    ,
    {"PickupKey", "", 36, -1, NULL, 0, 2, 1}
    ,
    {"PickupItem", "", 36, -1, NULL, 0, 2, 1}
    ,
    {"PickupPiece", "", 36, -1, NULL, 0, 2, 0}
    ,
    {"WeaponBuild", "", 36, -1, NULL, 0, 2, 0}
    ,
    {"UseArtifact", "", 36, -1, NULL, 0, 2, 1}
    ,
    {"BlastRadius", "", 36, -1, NULL, 0, 2, 1}
    ,
    {"Teleport", "", 256, -1, NULL, 0, 2, 1}
    ,
    {"ThunderCrash", "", 30, -1, NULL, 0, 2, 1}
    ,
    {"FighterPunchMiss", "", 80, -1, NULL, 0, 2, 1}
    ,
    {"FighterPunchHitThing", "", 80, -1, NULL, 0, 2, 1}
    ,
    {"FighterPunchHitWall", "", 80, -1, NULL, 0, 2, 1}
    ,
    {"FighterGrunt", "", 80, -1, NULL, 0, 2, 1}
    ,
    {"FighterAxeHitThing", "", 80, -1, NULL, 0, 2, 1}
    ,
    {"FighterHammerMiss", "", 80, -1, NULL, 0, 2, 1}
    ,
    {"FighterHammerHitThing", "", 80, -1, NULL, 0, 2, 1}
    ,
    {"FighterHammerHitWall", "", 80, -1, NULL, 0, 2, 1}
    ,
    {"FighterHammerContinuous", "", 32, -1, NULL, 0, 2, 1}
    ,
    {"FighterHammerExplode", "", 80, -1, NULL, 0, 2, 1}
    ,
    {"FighterSwordFire", "", 80, -1, NULL, 0, 2, 1}
    ,
    {"FighterSwordExplode", "", 80, -1, NULL, 0, 2, 1}
    ,
    {"ClericCStaffFire", "", 80, -1, NULL, 0, 2, 1}
    ,
    {"ClericCStaffExplode", "", 40, -1, NULL, 0, 2, 1}
    ,
    {"ClericCStaffHitThing", "", 80, -1, NULL, 0, 2, 1}
    ,
    {"ClericFlameFire", "", 80, -1, NULL, 0, 2, 1}
    ,
    {"ClericFlameExplode", "", 80, -1, NULL, 0, 2, 1}
    ,
    {"ClericFlameCircle", "", 80, -1, NULL, 0, 2, 1}
    ,
    {"MageWandFire", "", 80, -1, NULL, 0, 2, 1}
    ,
    {"MageLightningFire", "", 80, -1, NULL, 0, 2, 1}
    ,
    {"MageLightningZap", "", 32, -1, NULL, 0, 2, 1}
    ,
    {"MageLightningContinuous", "", 32, -1, NULL, 0, 2, 1}
    ,
    {"MageLightningReady", "", 30, -1, NULL, 0, 2, 1}
    ,
    {"MageShardsFire", "", 80, -1, NULL, 0, 2, 1}
    ,
    {"MageShardsExplode", "", 36, -1, NULL, 0, 2, 1}
    ,
    {"MageStaffFire", "", 80, -1, NULL, 0, 2, 1}
    ,
    {"MageStaffExplode", "", 40, -1, NULL, 0, 2, 1}
    ,
    {"Switch1", "", 32, -1, NULL, 0, 2, 1}
    ,
    {"Switch2", "", 32, -1, NULL, 0, 2, 1}
    ,
    {"SerpentSight", "", 32, -1, NULL, 0, 2, 1}
    ,
    {"SerpentActive", "", 32, -1, NULL, 0, 2, 1}
    ,
    {"SerpentPain", "", 32, -1, NULL, 0, 2, 1}
    ,
    {"SerpentAttack", "", 32, -1, NULL, 0, 2, 1}
    ,
    {"SerpentMeleeHit", "", 32, -1, NULL, 0, 2, 1}
    ,
    {"SerpentDeath", "", 40, -1, NULL, 0, 2, 1}
    ,
    {"SerpentBirth", "", 32, -1, NULL, 0, 2, 1}
    ,
    {"SerpentFXContinuous", "", 32, -1, NULL, 0, 2, 1}
    ,
    {"SerpentFXHit", "", 32, -1, NULL, 0, 2, 1}
    ,
    {"PotteryExplode", "", 32, -1, NULL, 0, 2, 1}
    ,
    {"Drip", "", 32, -1, NULL, 0, 2, 1}
    ,
    {"CentaurSight", "", 32, -1, NULL, 0, 2, 1}
    ,
    {"CentaurActive", "", 32, -1, NULL, 0, 2, 1}
    ,
    {"CentaurPain", "", 32, -1, NULL, 0, 2, 1}
    ,
    {"CentaurAttack", "", 32, -1, NULL, 0, 2, 1}
    ,
    {"CentaurDeath", "", 40, -1, NULL, 0, 2, 1}
    ,
    {"CentaurLeaderAttack", "", 32, -1, NULL, 0, 2, 1}
    ,
    {"CentaurMissileExplode", "", 32, -1, NULL, 0, 2, 1}
    ,
    {"Wind", "", 1, -1, NULL, 0, 2, 1}
    ,
    {"BishopSight", "", 32, -1, NULL, 0, 2, 1}
    ,
    {"BishopActive", "", 32, -1, NULL, 0, 2, 1}
    ,
    {"BishopPain", "", 32, -1, NULL, 0, 2, 1}
    ,
    {"BishopAttack", "", 32, -1, NULL, 0, 2, 1}
    ,
    {"BishopDeath", "", 40, -1, NULL, 0, 2, 1}
    ,
    {"BishopMissileExplode", "", 32, -1, NULL, 0, 2, 1}
    ,
    {"BishopBlur", "", 32, -1, NULL, 0, 2, 1}
    ,
    {"DemonSight", "", 32, -1, NULL, 0, 2, 1}
    ,
    {"DemonActive", "", 32, -1, NULL, 0, 2, 1}
    ,
    {"DemonPain", "", 32, -1, NULL, 0, 2, 1}
    ,
    {"DemonAttack", "", 32, -1, NULL, 0, 2, 1}
    ,
    {"DemonMissileFire", "", 32, -1, NULL, 0, 2, 1}
    ,
    {"DemonMissileExplode", "", 32, -1, NULL, 0, 2, 1}
    ,
    {"DemonDeath", "", 40, -1, NULL, 0, 2, 1}
    ,
    {"WraithSight", "", 32, -1, NULL, 0, 2, 1}
    ,
    {"WraithActive", "", 32, -1, NULL, 0, 2, 1}
    ,
    {"WraithPain", "", 32, -1, NULL, 0, 2, 1}
    ,
    {"WraithAttack", "", 32, -1, NULL, 0, 2, 1}
    ,
    {"WraithMissileFire", "", 32, -1, NULL, 0, 2, 1}
    ,
    {"WraithMissileExplode", "", 32, -1, NULL, 0, 2, 1}
    ,
    {"WraithDeath", "", 40, -1, NULL, 0, 2, 1}
    ,
    {"PigActive1", "", 32, -1, NULL, 0, 2, 1}
    ,
    {"PigActive2", "", 32, -1, NULL, 0, 2, 1}
    ,
    {"PigPain", "", 32, -1, NULL, 0, 2, 1}
    ,
    {"PigAttack", "", 32, -1, NULL, 0, 2, 1}
    ,
    {"PigDeath", "", 40, -1, NULL, 0, 2, 1}
    ,
    {"MaulatorSight", "", 32, -1, NULL, 0, 2, 1}
    ,
    {"MaulatorActive", "", 32, -1, NULL, 0, 2, 1}
    ,
    {"MaulatorPain", "", 32, -1, NULL, 0, 2, 1}
    ,
    {"MaulatorHamSwing", "", 32, -1, NULL, 0, 2, 1}
    ,
    {"MaulatorHamHit", "", 32, -1, NULL, 0, 2, 1}
    ,
    {"MaulatorMissileHit", "", 32, -1, NULL, 0, 2, 1}
    ,
    {"MaulatorDeath", "", 40, -1, NULL, 0, 2, 1}
    ,
    {"FreezeDeath", "", 40, -1, NULL, 0, 2, 1}
    ,
    {"FreezeShatter", "", 40, -1, NULL, 0, 2, 1}
    ,
    {"EttinSight", "", 32, -1, NULL, 0, 2, 1}
    ,
    {"EttinActive", "", 32, -1, NULL, 0, 2, 1}
    ,
    {"EttinPain", "", 32, -1, NULL, 0, 2, 1}
    ,
    {"EttinAttack", "", 32, -1, NULL, 0, 2, 1}
    ,
    {"EttinDeath", "", 40, -1, NULL, 0, 2, 1}
    ,
    {"FireDemonSpawn", "", 32, -1, NULL, 0, 2, 1}
    ,
    {"FireDemonActive", "", 32, -1, NULL, 0, 2, 1}
    ,
    {"FireDemonPain", "", 32, -1, NULL, 0, 2, 1}
    ,
    {"FireDemonAttack", "", 32, -1, NULL, 0, 2, 1}
    ,
    {"FireDemonMissileHit", "", 32, -1, NULL, 0, 2, 1}
    ,
    {"FireDemonDeath", "", 40, -1, NULL, 0, 2, 1}
    ,
    {"IceGuySight", "", 32, -1, NULL, 0, 2, 1}
    ,
    {"IceGuyActive", "", 32, -1, NULL, 0, 2, 1}
    ,
    {"IceGuyAttack", "", 32, -1, NULL, 0, 2, 1}
    ,
    {"IceGuyMissileExplode", "", 32, -1, NULL, 0, 2, 1}
    ,
    {"SorcererSight", "", 256, -1, NULL, 0, 2, 1}
    ,
    {"SorcererActive", "", 256, -1, NULL, 0, 2, 1}
    ,
    {"SorcererPain", "", 256, -1, NULL, 0, 2, 1}
    ,
    {"SorcererSpellCast", "", 256, -1, NULL, 0, 2, 1}
    ,
    {"SorcererBallWoosh", "", 256, -1, NULL, 0, 4, 1}
    ,
    {"SorcererDeathScream", "", 256, -1, NULL, 0, 2, 1}
    ,
    {"SorcererBishopSpawn", "", 80, -1, NULL, 0, 2, 1}
    ,
    {"SorcererBallPop", "", 80, -1, NULL, 0, 2, 1}
    ,
    {"SorcererBallBounce", "", 80, -1, NULL, 0, 3, 1}
    ,
    {"SorcererBallExplode", "", 80, -1, NULL, 0, 3, 1}
    ,
    {"SorcererBigBallExplode", "", 80, -1, NULL, 0, 3, 1}
    ,
    {"SorcererHeadScream", "", 256, -1, NULL, 0, 2, 1}
    ,
    {"DragonSight", "", 64, -1, NULL, 0, 2, 1}
    ,
    {"DragonActive", "", 64, -1, NULL, 0, 2, 1}
    ,
    {"DragonWingflap", "", 64, -1, NULL, 0, 2, 1}
    ,
    {"DragonAttack", "", 64, -1, NULL, 0, 2, 1}
    ,
    {"DragonPain", "", 64, -1, NULL, 0, 2, 1}
    ,
    {"DragonDeath", "", 64, -1, NULL, 0, 2, 1}
    ,
    {"DragonFireballExplode", "", 32, -1, NULL, 0, 2, 1}
    ,
    {"KoraxSight", "", 256, -1, NULL, 0, 2, 1}
    ,
    {"KoraxActive", "", 256, -1, NULL, 0, 2, 1}
    ,
    {"KoraxPain", "", 256, -1, NULL, 0, 2, 1}
    ,
    {"KoraxAttack", "", 256, -1, NULL, 0, 2, 1}
    ,
    {"KoraxCommand", "", 256, -1, NULL, 0, 2, 1}
    ,
    {"KoraxDeath", "", 256, -1, NULL, 0, 2, 1}
    ,
    {"KoraxStep", "", 128, -1, NULL, 0, 2, 1}
    ,
    {"ThrustSpikeRaise", "", 32, -1, NULL, 0, 2, 1}
    ,
    {"ThrustSpikeLower", "", 32, -1, NULL, 0, 2, 1}
    ,
    {"GlassShatter", "", 32, -1, NULL, 0, 2, 1}
    ,
    {"FlechetteBounce", "", 32, -1, NULL, 0, 2, 1}
    ,
    {"FlechetteExplode", "", 32, -1, NULL, 0, 2, 1}
    ,
    {"LavaMove", "", 36, -1, NULL, 0, 2, 1}
    ,
    {"WaterMove", "", 36, -1, NULL, 0, 2, 1}
    ,
    {"IceStartMove", "", 36, -1, NULL, 0, 2, 1}
    ,
    {"EarthStartMove", "", 36, -1, NULL, 0, 2, 1}
    ,
    {"WaterSplash", "", 32, -1, NULL, 0, 2, 1}
    ,
    {"LavaSizzle", "", 32, -1, NULL, 0, 2, 1}
    ,
    {"SludgeGloop", "", 32, -1, NULL, 0, 2, 1}
    ,
    {"HolySymbolFire", "", 64, -1, NULL, 0, 2, 1}
    ,
    {"SpiritActive", "", 32, -1, NULL, 0, 2, 1}
    ,
    {"SpiritAttack", "", 32, -1, NULL, 0, 2, 1}
    ,
    {"SpiritDie", "", 32, -1, NULL, 0, 2, 1}
    ,
    {"ValveTurn", "", 36, -1, NULL, 0, 2, 1}
    ,
    {"RopePull", "", 36, -1, NULL, 0, 2, 1}
    ,
    {"FlyBuzz", "", 20, -1, NULL, 0, 2, 1}
    ,
    {"Ignite", "", 32, -1, NULL, 0, 2, 1}
    ,
    {"PuzzleSuccess", "", 256, -1, NULL, 0, 2, 1}
    ,
    {"PuzzleFailFighter", "", 256, -1, NULL, 0, 2, 1}
    ,
    {"PuzzleFailCleric", "", 256, -1, NULL, 0, 2, 1}
    ,
    {"PuzzleFailMage", "", 256, -1, NULL, 0, 2, 1}
    ,
    {"Earthquake", "", 32, -1, NULL, 0, 2, 1}
    ,
    {"BellRing", "", 32, -1, NULL, 0, 2, 0}
    ,
    {"TreeBreak", "", 32, -1, NULL, 0, 2, 1}
    ,
    {"TreeExplode", "", 32, -1, NULL, 0, 2, 1}
    ,
    {"SuitofArmorBreak", "", 32, -1, NULL, 0, 2, 1}
    ,
    {"PoisonShroomPain", "", 20, -1, NULL, 0, 2, 1}
    ,
    {"PoisonShroomDeath", "", 32, -1, NULL, 0, 2, 1}
    ,
    {"Ambient1", "", 1, -1, NULL, 0, 1, 1}
    ,
    {"Ambient2", "", 1, -1, NULL, 0, 1, 1}
    ,
    {"Ambient3", "", 1, -1, NULL, 0, 1, 1}
    ,
    {"Ambient4", "", 1, -1, NULL, 0, 1, 1}
    ,
    {"Ambient5", "", 1, -1, NULL, 0, 1, 1}
    ,
    {"Ambient6", "", 1, -1, NULL, 0, 1, 1}
    ,
    {"Ambient7", "", 1, -1, NULL, 0, 1, 1}
    ,
    {"Ambient8", "", 1, -1, NULL, 0, 1, 1}
    ,
    {"Ambient9", "", 1, -1, NULL, 0, 1, 1}
    ,
    {"Ambient10", "", 1, -1, NULL, 0, 1, 1}
    ,
    {"Ambient11", "", 1, -1, NULL, 0, 1, 1}
    ,
    {"Ambient12", "", 1, -1, NULL, 0, 1, 1}
    ,
    {"Ambient13", "", 1, -1, NULL, 0, 1, 1}
    ,
    {"Ambient14", "", 1, -1, NULL, 0, 1, 1}
    ,
    {"Ambient15", "", 1, -1, NULL, 0, 1, 1}
    ,
    {"StartupTick", "", 32, -1, NULL, 0, 2, 1}
    ,
    {"SwitchOtherLevel", "", 32, -1, NULL, 0, 2, 1}
    ,
    {"Respawn", "", 32, -1, NULL, 0, 2, 1}
    ,
    {"KoraxVoiceGreetings", "", 512, -1, NULL, 0, 2, 1}
    ,
    {"KoraxVoiceReady", "", 512, -1, NULL, 0, 2, 1}
    ,
    {"KoraxVoiceBlood", "", 512, -1, NULL, 0, 2, 1}
    ,
    {"KoraxVoiceGame", "", 512, -1, NULL, 0, 2, 1}
    ,
    {"KoraxVoiceBoard", "", 512, -1, NULL, 0, 2, 1}
    ,
    {"KoraxVoiceWorship", "", 512, -1, NULL, 0, 2, 1}
    ,
    {"KoraxVoiceMaybe", "", 512, -1, NULL, 0, 2, 1}
    ,
    {"KoraxVoiceStrong", "", 512, -1, NULL, 0, 2, 1}
    ,
    {"KoraxVoiceFace", "", 512, -1, NULL, 0, 2, 1}
    ,
    {"BatScream", "", 32, -1, NULL, 0, 2, 1}
    ,
    {"Chat", "", 512, -1, NULL, 0, 2, 1}
    ,
    {"MenuMove", "", 32, -1, NULL, 0, 2, 1}
    ,
    {"ClockTick", "", 32, -1, NULL, 0, 2, 1}
    ,
    {"Fireball", "", 32, -1, NULL, 0, 2, 1}
    ,
    {"PuppyBeat", "", 30, -1, NULL, 0, 2, 1}
    ,
    {"MysticIncant", "", 32, -1, NULL, 0, 4, 1}
};
