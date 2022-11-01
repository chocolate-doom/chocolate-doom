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
//
// External definitions for action pointer functions.
//

#ifndef HEXEN_P_ACTION_H
#define HEXEN_P_ACTION_H


void A_FreeTargMobj(mobj_t *actor);
void A_FlameCheck(mobj_t *actor);
void A_HideThing(mobj_t *actor);
void A_RestoreSpecialThing1(mobj_t *thing);
void A_RestoreSpecialThing2(mobj_t *thing);
void A_RestoreArtifact(mobj_t *arti);
void A_Summon(mobj_t *actor);
void A_ThrustInitUp(mobj_t *actor);
void A_ThrustInitDn(mobj_t *actor);
void A_ThrustRaise(mobj_t *actor);
void A_ThrustBlock(mobj_t *actor);
void A_ThrustImpale(mobj_t *actor);
void A_ThrustLower(mobj_t *actor);
void A_TeloSpawnC(mobj_t *actor);
void A_TeloSpawnB(mobj_t *actor);
void A_TeloSpawnA(mobj_t *actor);
void A_TeloSpawnD(mobj_t *actor);
void A_CheckTeleRing(mobj_t *actor);
void A_FogSpawn(mobj_t *actor);
void A_FogMove(mobj_t *actor);
void A_Quake(mobj_t *actor);
void A_ContMobjSound(mobj_t *actor);
void A_Scream(mobj_t *actor);
void A_PoisonBagInit(mobj_t *actor);
void A_PoisonBagDamage(mobj_t *actor);
void A_PoisonBagCheck(mobj_t *actor);
void A_CheckThrowBomb(mobj_t *actor);
void A_NoGravity(mobj_t *actor);
void A_PotteryExplode(mobj_t *actor);
void A_PotteryChooseBit(mobj_t *actor);
void A_PotteryCheck(mobj_t *actor);
void A_CorpseBloodDrip(mobj_t *actor);
void A_CorpseExplode(mobj_t *actor);
void A_LeafSpawn(mobj_t *actor);
void A_LeafThrust(mobj_t *actor);
void A_LeafCheck(mobj_t *actor);
void A_BridgeInit(mobj_t *actor);
void A_BridgeOrbit(mobj_t *actor);
void A_TreeDeath(mobj_t *actor);
void A_PoisonShroom(mobj_t *actor);
void A_Pain(mobj_t *actor);
void A_SoAExplode(mobj_t *actor);
void A_BellReset1(mobj_t *actor);
void A_BellReset2(mobj_t *actor);
void A_Light0(player_t *player, pspdef_t *psp);
void A_WeaponReady(player_t *player, pspdef_t *psp);
void A_Lower(player_t *player, pspdef_t *psp);
void A_Raise(player_t *player, pspdef_t *psp);
void A_FPunchAttack(player_t *player, pspdef_t *psp);
void A_ReFire(player_t *player, pspdef_t *psp);
void A_FAxeAttack(player_t *player, pspdef_t *psp);
void A_FHammerAttack(player_t *player, pspdef_t *psp);
void A_FHammerThrow(player_t *player, pspdef_t *psp);
void A_FSwordAttack(player_t *player, pspdef_t *psp);
void A_FSwordFlames(mobj_t *actor);
void A_CMaceAttack(player_t *player, pspdef_t *psp);
void A_CStaffInitBlink(player_t *player, pspdef_t *psp);
void A_CStaffCheckBlink(player_t *player, pspdef_t *psp);
void A_CStaffCheck(player_t * player, pspdef_t *psp);
void A_CStaffAttack(player_t *player, pspdef_t *psp);
void A_CStaffMissileSlither(mobj_t *actor);
void A_CFlameAttack(player_t *player, pspdef_t *psp);
void A_CFlameRotate(mobj_t *actor);
void A_CFlamePuff(mobj_t *actor);
void A_CFlameMissile(mobj_t *actor);
void A_CHolyAttack(player_t *player, pspdef_t *psp);
void A_CHolyPalette(player_t *player, pspdef_t *psp);
void A_CHolySeek(mobj_t *actor);
void A_CHolyCheckScream(mobj_t *actor);
void A_CHolyTail(mobj_t *actor);
void A_CHolySpawnPuff(mobj_t *actor);
void A_CHolyAttack2(mobj_t *actor);
void A_MWandAttack(player_t *player, pspdef_t *psp);
void A_LightningReady(player_t *player, pspdef_t *psp);
void A_MLightningAttack(player_t *player, pspdef_t *psp);
void A_LightningZap(mobj_t *actor);
void A_LightningClip(mobj_t *actor);
void A_LightningRemove(mobj_t *actor);
void A_LastZap(mobj_t *actor);
void A_ZapMimic(mobj_t *actor);
void A_MStaffAttack(player_t *player, pspdef_t *psp);
void A_MStaffPalette(player_t *player, pspdef_t *psp);
void A_MStaffWeave(mobj_t *actor);
void A_MStaffTrack(mobj_t *actor);
void A_SnoutAttack(player_t *player, pspdef_t *psp);
void A_FireConePL1(player_t * player, pspdef_t *psp);
void A_ShedShard(mobj_t *actor);
void A_AddPlayerCorpse(mobj_t *actor);
void A_SkullPop(mobj_t *actor);
void A_FreezeDeath(mobj_t *actor);
void A_CheckBurnGone(mobj_t *actor);
void A_CheckSkullFloor(mobj_t *actor);
void A_CheckSkullDone(mobj_t *actor);
void A_SpeedFade(mobj_t *actor);
void A_IceSetTics(mobj_t *actor);
void A_IceCheckHeadDone(mobj_t *actor);
void A_PigPain(mobj_t *actor);
void A_PigLook(mobj_t *actor);
void A_PigChase(mobj_t *actor);
void A_FaceTarget(mobj_t *actor);
void A_PigAttack(mobj_t *actor);
void A_QueueCorpse(mobj_t *actor);
void A_Look(mobj_t *actor);
void A_Chase(mobj_t *actor);
void A_CentaurAttack(mobj_t *actor);
void A_CentaurAttack2(mobj_t *actor);
void A_SetReflective(mobj_t *actor);
void A_CentaurDefend(mobj_t *actor);
void A_UnSetReflective(mobj_t *actor);
void A_CentaurDropStuff(mobj_t *actor);
void A_CheckFloor(mobj_t *actor);
void A_DemonAttack1(mobj_t *actor);
void A_DemonAttack2(mobj_t *actor);
void A_DemonDeath(mobj_t *actor);
void A_Demon2Death(mobj_t *actor);
void A_WraithRaiseInit(mobj_t *actor);
void A_WraithRaise(mobj_t *actor);
void A_WraithInit(mobj_t *actor);
void A_WraithLook(mobj_t *actor);
void A_WraithChase(mobj_t *actor);
void A_WraithFX3(mobj_t *actor);
void A_WraithMelee(mobj_t *actor);
void A_WraithMissile(mobj_t *actor);
void A_WraithFX2(mobj_t *actor);
void A_MinotaurFade1(mobj_t *actor);
void A_MinotaurFade2(mobj_t *actor);
void A_MinotaurChase(mobj_t *actor);
void A_MinotaurRoam(mobj_t *actor);
void A_MinotaurAtk1(mobj_t *actor);
void A_MinotaurDecide(mobj_t *actor);
void A_MinotaurAtk2(mobj_t *actor);
void A_MinotaurAtk3(mobj_t *actor);
void A_MinotaurCharge(mobj_t *actor);
void A_SmokePuffExit(mobj_t *actor);
void A_MinotaurFade0(mobj_t *actor);
void A_MntrFloorFire(mobj_t *actor);
void A_SerpentChase(mobj_t *actor);
void A_SerpentHumpDecide(mobj_t *actor);
void A_SerpentUnHide(mobj_t *actor);
void A_SerpentRaiseHump(mobj_t *actor);
void A_SerpentLowerHump(mobj_t *actor);
void A_SerpentHide(mobj_t *actor);
void A_SerpentBirthScream(mobj_t *actor);
void A_SetShootable(mobj_t *actor);
void A_SerpentCheckForAttack(mobj_t *actor);
void A_UnSetShootable(mobj_t *actor);
void A_SerpentDiveSound(mobj_t *actor);
void A_SerpentWalk(mobj_t *actor);
void A_SerpentChooseAttack(mobj_t *actor);
void A_SerpentMeleeAttack(mobj_t *actor);
void A_SerpentMissileAttack(mobj_t *actor);
void A_SerpentHeadPop(mobj_t *actor);
void A_SerpentSpawnGibs(mobj_t *actor);
void A_SerpentHeadCheck(mobj_t *actor);
void A_FloatGib(mobj_t *actor);
void A_DelayGib(mobj_t *actor);
void A_SinkGib(mobj_t *actor);
void A_BishopDecide(mobj_t *actor);
void A_BishopDoBlur(mobj_t *actor);
void A_BishopSpawnBlur(mobj_t *actor);
void A_BishopChase(mobj_t *actor);
void A_BishopAttack(mobj_t *actor);
void A_BishopAttack2(mobj_t *actor);
void A_BishopPainBlur(mobj_t *actor);
void A_BishopPuff(mobj_t *actor);
void A_SetAltShadow(mobj_t *actor);
void A_BishopMissileWeave(mobj_t *actor);
void A_BishopMissileSeek(mobj_t *actor);
void A_DragonInitFlight(mobj_t *actor);
void A_DragonFlap(mobj_t *actor);
void A_DragonFlight(mobj_t *actor);
void A_DragonAttack(mobj_t *actor);
void A_DragonPain(mobj_t *actor);
void A_DragonCheckCrash(mobj_t *actor);
void A_DragonFX2(mobj_t *actor);
void A_ESound(mobj_t *mo);
void A_EttinAttack(mobj_t *actor);
void A_DropMace(mobj_t *actor);
void A_FiredRocks(mobj_t *actor);
void A_UnSetInvulnerable(mobj_t *actor);
void A_FiredChase(mobj_t *actor);
void A_FiredAttack(mobj_t *actor);
void A_FiredSplotch(mobj_t *actor);
void A_SmBounce(mobj_t *actor);
void A_IceGuyLook(mobj_t *actor);
void A_IceGuyChase(mobj_t *actor);
void A_IceGuyAttack(mobj_t *actor);
void A_IceGuyDie(mobj_t *actor);
void A_IceGuyMissilePuff(mobj_t *actor);
void A_IceGuyMissileExplode(mobj_t *actor);
void A_ClassBossHealth(mobj_t *actor);
void A_FastChase(mobj_t *actor);
void A_FighterAttack(mobj_t *actor);
void A_ClericAttack(mobj_t *actor);
void A_MageAttack(mobj_t *actor);
void A_SorcBallPop(mobj_t *actor);
void A_SorcFX2Split(mobj_t *actor);
void A_SorcFX2Orbit(mobj_t *actor);
void A_SorcererBishopEntry(mobj_t *actor);
void A_SpawnBishop(mobj_t *actor);
void A_SorcFX4Check(mobj_t *actor);
void A_KoraxStep2(mobj_t *actor);
void A_KoraxChase(mobj_t *actor);
void A_KoraxStep(mobj_t *actor);
void A_KoraxDecide(mobj_t *actor);
void A_KoraxMissile(mobj_t *actor);
void A_KoraxCommand(mobj_t *actor);
void A_KoraxBonePop(mobj_t *actor);
void A_KSpiritRoam(mobj_t *actor);
void A_KBoltRaise(mobj_t *actor);
void A_KBolt(mobj_t *actor);
void A_BatSpawnInit(mobj_t *actor);
void A_BatSpawn(mobj_t *actor);
void A_BatMove(mobj_t *actor);


#endif // HEXEN_P_ACTION_H
