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

#ifndef HERETIC_P_ACTION_H
#define HERETIC_P_ACTION_H

void A_FreeTargMobj(struct mobj_s *actor, struct player_s *player, struct pspdef_s *psp);
void A_RestoreSpecialThing1(struct mobj_s *actor, struct player_s *player, struct pspdef_s *psp);
void A_RestoreSpecialThing2(struct mobj_s *actor, struct player_s *player, struct pspdef_s *psp);
void A_HideThing(struct mobj_s *actor, struct player_s *player, struct pspdef_s *psp);
void A_UnHideThing(struct mobj_s *actor, struct player_s *player, struct pspdef_s *psp);
void A_RestoreArtifact(struct mobj_s *actor, struct player_s *player, struct pspdef_s *psp);
void A_Scream(struct mobj_s *actor, struct player_s *player, struct pspdef_s *psp);
void A_Explode(struct mobj_s *actor, struct player_s *player, struct pspdef_s *psp);
void A_PodPain(struct mobj_s *actor, struct player_s *player, struct pspdef_s *psp);
void A_RemovePod(struct mobj_s *actor, struct player_s *player, struct pspdef_s *psp);
void A_MakePod(struct mobj_s *actor, struct player_s *player, struct pspdef_s *psp);
void A_InitKeyGizmo(struct mobj_s *actor, struct player_s *player, struct pspdef_s *psp);
void A_VolcanoSet(struct mobj_s *actor, struct player_s *player, struct pspdef_s *psp);
void A_VolcanoBlast(struct mobj_s *actor, struct player_s *player, struct pspdef_s *psp);
void A_BeastPuff(struct mobj_s *actor, struct player_s *player, struct pspdef_s *psp);
void A_VolcBallImpact(struct mobj_s *actor, struct player_s *player, struct pspdef_s *psp);
void A_SpawnTeleGlitter(struct mobj_s *actor, struct player_s *player, struct pspdef_s *psp);
void A_SpawnTeleGlitter2(struct mobj_s *actor, struct player_s *player, struct pspdef_s *psp);
void A_AccTeleGlitter(struct mobj_s *actor, struct player_s *player, struct pspdef_s *psp);
void A_Light0(struct mobj_s *actor, struct player_s *player, struct pspdef_s *psp);
void A_WeaponReady(struct mobj_s *actor, struct player_s *player, struct pspdef_s *psp);
void A_Lower(struct mobj_s *actor, struct player_s *player, struct pspdef_s *psp);
void A_Raise(struct mobj_s *actor, struct player_s *player, struct pspdef_s *psp);
void A_StaffAttackPL1(struct mobj_s *actor, struct player_s *player, struct pspdef_s *psp);
void A_ReFire(struct mobj_s *actor, struct player_s *player, struct pspdef_s *psp);
void A_StaffAttackPL2(struct mobj_s *actor, struct player_s *player, struct pspdef_s *psp);
void A_BeakReady(struct mobj_s *actor, struct player_s *player, struct pspdef_s *psp);
void A_BeakRaise(struct mobj_s *actor, struct player_s *player, struct pspdef_s *psp);
void A_BeakAttackPL1(struct mobj_s *actor, struct player_s *player, struct pspdef_s *psp);
void A_BeakAttackPL2(struct mobj_s *actor, struct player_s *player, struct pspdef_s *psp);
void A_GauntletAttack(struct mobj_s *actor, struct player_s *player, struct pspdef_s *psp);
void A_FireBlasterPL1(struct mobj_s *actor, struct player_s *player, struct pspdef_s *psp);
void A_FireBlasterPL2(struct mobj_s *actor, struct player_s *player, struct pspdef_s *psp);
void A_SpawnRippers(struct mobj_s *actor, struct player_s *player, struct pspdef_s *psp);
void A_FireMacePL1(struct mobj_s *actor, struct player_s *player, struct pspdef_s *psp);
void A_FireMacePL2(struct mobj_s *actor, struct player_s *player, struct pspdef_s *psp);
void A_MacePL1Check(struct mobj_s *actor, struct player_s *player, struct pspdef_s *psp);
void A_MaceBallImpact(struct mobj_s *actor, struct player_s *player, struct pspdef_s *psp);
void A_MaceBallImpact2(struct mobj_s *actor, struct player_s *player, struct pspdef_s *psp);
void A_DeathBallImpact(struct mobj_s *actor, struct player_s *player, struct pspdef_s *psp);
void A_FireSkullRodPL1(struct mobj_s *actor, struct player_s *player, struct pspdef_s *psp);
void A_FireSkullRodPL2(struct mobj_s *actor, struct player_s *player, struct pspdef_s *psp);
void A_SkullRodPL2Seek(struct mobj_s *actor, struct player_s *player, struct pspdef_s *psp);
void A_AddPlayerRain(struct mobj_s *actor, struct player_s *player, struct pspdef_s *psp);
void A_HideInCeiling(struct mobj_s *actor, struct player_s *player, struct pspdef_s *psp);
void A_SkullRodStorm(struct mobj_s *actor, struct player_s *player, struct pspdef_s *psp);
void A_RainImpact(struct mobj_s *actor, struct player_s *player, struct pspdef_s *psp);
void A_FireGoldWandPL1(struct mobj_s *actor, struct player_s *player, struct pspdef_s *psp);
void A_FireGoldWandPL2(struct mobj_s *actor, struct player_s *player, struct pspdef_s *psp);
void A_FirePhoenixPL1(struct mobj_s *actor, struct player_s *player, struct pspdef_s *psp);
void A_InitPhoenixPL2(struct mobj_s *actor, struct player_s *player, struct pspdef_s *psp);
void A_FirePhoenixPL2(struct mobj_s *actor, struct player_s *player, struct pspdef_s *psp);
void A_ShutdownPhoenixPL2(struct mobj_s *actor, struct player_s *player, struct pspdef_s *psp);
void A_PhoenixPuff(struct mobj_s *actor, struct player_s *player, struct pspdef_s *psp);
void A_RemovedPhoenixFunc(struct mobj_s *actor, struct player_s *player, struct pspdef_s *psp);
void A_FlameEnd(struct mobj_s *actor, struct player_s *player, struct pspdef_s *psp);
void A_FloatPuff(struct mobj_s *actor, struct player_s *player, struct pspdef_s *psp);
void A_FireCrossbowPL1(struct mobj_s *actor, struct player_s *player, struct pspdef_s *psp);
void A_FireCrossbowPL2(struct mobj_s *actor, struct player_s *player, struct pspdef_s *psp);
void A_BoltSpark(struct mobj_s *actor, struct player_s *player, struct pspdef_s *psp);
void A_Pain(struct mobj_s *actor, struct player_s *player, struct pspdef_s *psp);
void A_NoBlocking(struct mobj_s *actor, struct player_s *player, struct pspdef_s *psp);
void A_AddPlayerCorpse(struct mobj_s *actor, struct player_s *player, struct pspdef_s *psp);
void A_SkullPop(struct mobj_s *actor, struct player_s *player, struct pspdef_s *psp);
void A_FlameSnd(struct mobj_s *actor, struct player_s *player, struct pspdef_s *psp);
void A_CheckBurnGone(struct mobj_s *actor, struct player_s *player, struct pspdef_s *psp);
void A_CheckSkullFloor(struct mobj_s *actor, struct player_s *player, struct pspdef_s *psp);
void A_CheckSkullDone(struct mobj_s *actor, struct player_s *player, struct pspdef_s *psp);
void A_Feathers(struct mobj_s *actor, struct player_s *player, struct pspdef_s *psp);
void A_ChicLook(struct mobj_s *actor, struct player_s *player, struct pspdef_s *psp);
void A_ChicChase(struct mobj_s *actor, struct player_s *player, struct pspdef_s *psp);
void A_ChicPain(struct mobj_s *actor, struct player_s *player, struct pspdef_s *psp);
void A_FaceTarget(struct mobj_s *actor, struct player_s *player, struct pspdef_s *psp);
void A_ChicAttack(struct mobj_s *actor, struct player_s *player, struct pspdef_s *psp);
void A_Look(struct mobj_s *actor, struct player_s *player, struct pspdef_s *psp);
void A_Chase(struct mobj_s *actor, struct player_s *player, struct pspdef_s *psp);
void A_MummyAttack(struct mobj_s *actor, struct player_s *player, struct pspdef_s *psp);
void A_MummyAttack2(struct mobj_s *actor, struct player_s *player, struct pspdef_s *psp);
void A_MummySoul(struct mobj_s *actor, struct player_s *player, struct pspdef_s *psp);
void A_ContMobjSound(struct mobj_s *actor, struct player_s *player, struct pspdef_s *psp);
void A_MummyFX1Seek(struct mobj_s *actor, struct player_s *player, struct pspdef_s *psp);
void A_BeastAttack(struct mobj_s *actor, struct player_s *player, struct pspdef_s *psp);
void A_SnakeAttack(struct mobj_s *actor, struct player_s *player, struct pspdef_s *psp);
void A_SnakeAttack2(struct mobj_s *actor, struct player_s *player, struct pspdef_s *psp);
void A_HeadAttack(struct mobj_s *actor, struct player_s *player, struct pspdef_s *psp);
void A_BossDeath(struct mobj_s *actor, struct player_s *player, struct pspdef_s *psp);
void A_HeadIceImpact(struct mobj_s *actor, struct player_s *player, struct pspdef_s *psp);
void A_HeadFireGrow(struct mobj_s *actor, struct player_s *player, struct pspdef_s *psp);
void A_WhirlwindSeek(struct mobj_s *actor, struct player_s *player, struct pspdef_s *psp);
void A_ClinkAttack(struct mobj_s *actor, struct player_s *player, struct pspdef_s *psp);
void A_WizAtk1(struct mobj_s *actor, struct player_s *player, struct pspdef_s *psp);
void A_WizAtk2(struct mobj_s *actor, struct player_s *player, struct pspdef_s *psp);
void A_WizAtk3(struct mobj_s *actor, struct player_s *player, struct pspdef_s *psp);
void A_GhostOff(struct mobj_s *actor, struct player_s *player, struct pspdef_s *psp);
void A_ImpMeAttack(struct mobj_s *actor, struct player_s *player, struct pspdef_s *psp);
void A_ImpMsAttack(struct mobj_s *actor, struct player_s *player, struct pspdef_s *psp);
void A_ImpMsAttack2(struct mobj_s *actor, struct player_s *player, struct pspdef_s *psp);
void A_ImpDeath(struct mobj_s *actor, struct player_s *player, struct pspdef_s *psp);
void A_ImpXDeath1(struct mobj_s *actor, struct player_s *player, struct pspdef_s *psp);
void A_ImpXDeath2(struct mobj_s *actor, struct player_s *player, struct pspdef_s *psp);
void A_ImpExplode(struct mobj_s *actor, struct player_s *player, struct pspdef_s *psp);
void A_KnightAttack(struct mobj_s *actor, struct player_s *player, struct pspdef_s *psp);
void A_DripBlood(struct mobj_s *actor, struct player_s *player, struct pspdef_s *psp);
void A_Sor1Chase(struct mobj_s *actor, struct player_s *player, struct pspdef_s *psp);
void A_Sor1Pain(struct mobj_s *actor, struct player_s *player, struct pspdef_s *psp);
void A_Srcr1Attack(struct mobj_s *actor, struct player_s *player, struct pspdef_s *psp);
void A_SorZap(struct mobj_s *actor, struct player_s *player, struct pspdef_s *psp);
void A_SorcererRise(struct mobj_s *actor, struct player_s *player, struct pspdef_s *psp);
void A_SorRise(struct mobj_s *actor, struct player_s *player, struct pspdef_s *psp);
void A_SorSightSnd(struct mobj_s *actor, struct player_s *player, struct pspdef_s *psp);
void A_Srcr2Decide(struct mobj_s *actor, struct player_s *player, struct pspdef_s *psp);
void A_Srcr2Attack(struct mobj_s *actor, struct player_s *player, struct pspdef_s *psp);
void A_Sor2DthInit(struct mobj_s *actor, struct player_s *player, struct pspdef_s *psp);
void A_SorDSph(struct mobj_s *actor, struct player_s *player, struct pspdef_s *psp);
void A_Sor2DthLoop(struct mobj_s *actor, struct player_s *player, struct pspdef_s *psp);
void A_SorDExp(struct mobj_s *actor, struct player_s *player, struct pspdef_s *psp);
void A_SorDBon(struct mobj_s *actor, struct player_s *player, struct pspdef_s *psp);
void A_BlueSpark(struct mobj_s *actor, struct player_s *player, struct pspdef_s *psp);
void A_GenWizard(struct mobj_s *actor, struct player_s *player, struct pspdef_s *psp);
void A_MinotaurAtk1(struct mobj_s *actor, struct player_s *player, struct pspdef_s *psp);
void A_MinotaurDecide(struct mobj_s *actor, struct player_s *player, struct pspdef_s *psp);
void A_MinotaurAtk2(struct mobj_s *actor, struct player_s *player, struct pspdef_s *psp);
void A_MinotaurAtk3(struct mobj_s *actor, struct player_s *player, struct pspdef_s *psp);
void A_MinotaurCharge(struct mobj_s *actor, struct player_s *player, struct pspdef_s *psp);
void A_MntrFloorFire(struct mobj_s *actor, struct player_s *player, struct pspdef_s *psp);
void A_ESound(struct mobj_s *actor, struct player_s *player, struct pspdef_s *psp);

#endif /* #ifndef HERETIC_P_ACTION_H */

