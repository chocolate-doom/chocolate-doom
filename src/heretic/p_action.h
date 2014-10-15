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

void A_FreeTargMobj();
void A_RestoreSpecialThing1();
void A_RestoreSpecialThing2();
void A_HideThing();
void A_UnHideThing();
void A_RestoreArtifact();
void A_Scream();
void A_Explode();
void A_PodPain();
void A_RemovePod();
void A_MakePod();
void A_InitKeyGizmo();
void A_VolcanoSet();
void A_VolcanoBlast();
void A_BeastPuff();
void A_VolcBallImpact();
void A_SpawnTeleGlitter();
void A_SpawnTeleGlitter2();
void A_AccTeleGlitter();
void A_Light0();
void A_WeaponReady();
void A_Lower();
void A_Raise();
void A_StaffAttackPL1();
void A_ReFire();
void A_StaffAttackPL2();
void A_BeakReady();
void A_BeakRaise();
void A_BeakAttackPL1();
void A_BeakAttackPL2();
void A_GauntletAttack();
void A_FireBlasterPL1();
void A_FireBlasterPL2();
void A_SpawnRippers();
void A_FireMacePL1();
void A_FireMacePL2();
void A_MacePL1Check();
void A_MaceBallImpact();
void A_MaceBallImpact2();
void A_DeathBallImpact();
void A_FireSkullRodPL1();
void A_FireSkullRodPL2();
void A_SkullRodPL2Seek();
void A_AddPlayerRain();
void A_HideInCeiling();
void A_SkullRodStorm();
void A_RainImpact();
void A_FireGoldWandPL1();
void A_FireGoldWandPL2();
void A_FirePhoenixPL1();
void A_InitPhoenixPL2();
void A_FirePhoenixPL2();
void A_ShutdownPhoenixPL2();
void A_PhoenixPuff();
void A_RemovedPhoenixFunc();
void A_FlameEnd();
void A_FloatPuff();
void A_FireCrossbowPL1();
void A_FireCrossbowPL2();
void A_BoltSpark();
void A_Pain();
void A_NoBlocking();
void A_AddPlayerCorpse();
void A_SkullPop();
void A_FlameSnd();
void A_CheckBurnGone();
void A_CheckSkullFloor();
void A_CheckSkullDone();
void A_Feathers();
void A_ChicLook();
void A_ChicChase();
void A_ChicPain();
void A_FaceTarget();
void A_ChicAttack();
void A_Look();
void A_Chase();
void A_MummyAttack();
void A_MummyAttack2();
void A_MummySoul();
void A_ContMobjSound();
void A_MummyFX1Seek();
void A_BeastAttack();
void A_SnakeAttack();
void A_SnakeAttack2();
void A_HeadAttack();
void A_BossDeath();
void A_HeadIceImpact();
void A_HeadFireGrow();
void A_WhirlwindSeek();
void A_ClinkAttack();
void A_WizAtk1();
void A_WizAtk2();
void A_WizAtk3();
void A_GhostOff();
void A_ImpMeAttack();
void A_ImpMsAttack();
void A_ImpMsAttack2();
void A_ImpDeath();
void A_ImpXDeath1();
void A_ImpXDeath2();
void A_ImpExplode();
void A_KnightAttack();
void A_DripBlood();
void A_Sor1Chase();
void A_Sor1Pain();
void A_Srcr1Attack();
void A_SorZap();
void A_SorcererRise();
void A_SorRise();
void A_SorSightSnd();
void A_Srcr2Decide();
void A_Srcr2Attack();
void A_Sor2DthInit();
void A_SorDSph();
void A_Sor2DthLoop();
void A_SorDExp();
void A_SorDBon();
void A_BlueSpark();
void A_GenWizard();
void A_MinotaurAtk1();
void A_MinotaurDecide();
void A_MinotaurAtk2();
void A_MinotaurAtk3();
void A_MinotaurCharge();
void A_MntrFloorFire();
void A_ESound();

#endif /* #ifndef HERETIC_P_ACTION_H */

