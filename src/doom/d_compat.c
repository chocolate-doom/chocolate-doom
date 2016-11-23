//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2016 Alexey Khokholov
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
// DESCRIPTION:
//  Older Doom EXEs emulation.
//

#include "doomdef.h"
#include "doomstat.h"

#include "i_system.h"

#include "info.h"
#include "sounds.h"

void  A_Light0();
void A_WeaponReady();
void A_Lower();
void A_Raise();
void A_Punch();
void A_ReFire();
void A_FirePistol();
void A_Light1();
void A_FireShotgun();
void A_Light2();
void A_FireShotgun2();
void A_CheckReload();
void A_OpenShotgun2();
void A_LoadShotgun2();
void A_CloseShotgun2();
void A_FireCGun();
void A_GunFlash();
void A_FireMissile();
void A_Saw();
void A_FirePlasma();
void A_BFGsound();
void A_FireBFG();
void A_BFGSpray();
void A_Explode();
void A_Pain();
void A_PlayerScream();
void A_Fall();
void A_XScream();
void A_Look();
void A_Chase();
void A_FaceTarget();
void A_PosAttack();
void A_Scream();
void A_SPosAttack();
void A_VileChase();
void A_VileStart();
void A_VileTarget();
void A_VileAttack();
void A_StartFire();
void A_Fire();
void A_FireCrackle();
void A_Tracer();
void A_SkelWhoosh();
void A_SkelFist();
void A_SkelMissile();
void A_FatRaise();
void A_FatAttack1();
void A_FatAttack2();
void A_FatAttack3();
void A_BossDeath();
void A_CPosAttack();
void A_CPosRefire();
void A_TroopAttack();
void A_SargAttack();
void A_HeadAttack();
void A_BruisAttack();
void A_SkullAttack();
void A_Metal();
void A_SpidRefire();
void A_BabyMetal();
void A_BspiAttack();
void A_Hoof();
void A_CyberAttack();
void A_PainAttack();
void A_PainDie();
void A_KeenDie();
void A_BrainPain();
void A_BrainScream();
void A_BrainDie();
void A_BrainAwake();
void A_BrainSpit();
void A_SpawnSound();
void A_SpawnFly();
void A_BrainExplode();

static spritenum_t spritemap_1_6[] = {
    SPR_TROO, SPR_SHTG, SPR_PUNG, SPR_PISG, SPR_PISF, SPR_SHTF, SPR_SHT2,
    SPR_CHGG, SPR_CHGF, SPR_MISG, SPR_MISF, SPR_SAWG, SPR_PLSG, SPR_PLSF,
    SPR_BFGG, SPR_BFGF, SPR_BLUD, SPR_PUFF, SPR_BAL1, SPR_BAL2, SPR_PLSS,
    SPR_PLSE, SPR_MISL, SPR_BFS1, SPR_BFE1, SPR_BFE2, SPR_TFOG, SPR_IFOG,
    SPR_PLAY, SPR_POSS, SPR_SPOS, SPR_VILE, SPR_SKEL, SPR_BAL7, SPR_FATT,
    SPR_CPOS, SPR_SARG, SPR_HEAD, SPR_BOSS, SPR_BOS2, SPR_SKUL, SPR_SPID,
    SPR_BSPI, SPR_CYBR, SPR_PAIN, SPR_ARM1, SPR_ARM2, SPR_BAR1, SPR_BEXP,
    SPR_FCAN, SPR_BON1, SPR_BON2, SPR_BKEY, SPR_RKEY, SPR_YKEY, SPR_BSKU,
    SPR_RSKU, SPR_YSKU, SPR_STIM, SPR_MEDI, SPR_SOUL, SPR_PINV, SPR_PSTR,
    SPR_PINS, SPR_SUIT, SPR_PMAP, SPR_PVIS, SPR_CLIP, SPR_AMMO, SPR_ROCK,
    SPR_BROK, SPR_CELL, SPR_CELP, SPR_SHEL, SPR_SBOX, SPR_BPAK, SPR_BFUG,
    SPR_MGUN, SPR_CSAW, SPR_LAUN, SPR_PLAS, SPR_SHOT, SPR_SGN2, SPR_COLU,
    SPR_SMT2, SPR_GOR1, SPR_POL2, SPR_POL5, SPR_POL4, SPR_POL3, SPR_POL1,
    SPR_POL6, SPR_GOR2, SPR_GOR3, SPR_GOR4, SPR_GOR5, SPR_SMIT, SPR_COL1,
    SPR_COL2, SPR_COL3, SPR_COL4, SPR_CAND, SPR_CBRA, SPR_COL6, SPR_TRE1,
    SPR_TRE2, SPR_ELEC, SPR_CEYE, SPR_FSKU, SPR_COL5, SPR_TBLU, SPR_TGRN,
    SPR_TRED, SPR_SMBT, SPR_SMGT, SPR_SMRT, SPR_KEEN, SPR_HDB1, SPR_HDB2,
    SPR_HDB3, SPR_HDB4, SPR_HDB5, SPR_HDB6, SPR_POB1, SPR_POB2, SPR_BRS1
};

static spritenum_t spritemap[NUMSPRITES];
static int spritemap_size;
static spritenum_t spritemap_rev[NUMSPRITES];
static boolean spriteexists[NUMSPRITES];

static statenum_t statemap_1_6[] = {
    S_NULL, S_LIGHTDONE, S_PUNCH, S_PUNCHDOWN, S_PUNCHUP, S_PUNCH1, S_PUNCH2,
    S_PUNCH3, S_PUNCH4, S_PUNCH5, S_PISTOL, S_PISTOLDOWN, S_PISTOLUP, S_PISTOL1,
    S_PISTOL2, S_PISTOL3, S_PISTOL4, S_PISTOLFLASH, S_SGUN, S_SGUNDOWN,
    S_SGUNUP, S_SGUN1, S_SGUN2, S_SGUN3, S_SGUN4, S_SGUN5, S_SGUN6, S_SGUN7,
    S_SGUN8, S_SGUN9, S_SGUNFLASH1, S_SGUNFLASH2, S_DSGUN, S_DSGUNDOWN,
    S_DSGUNUP, S_DSGUN1, S_DSGUN2, S_DSGUN3, S_DSGUN4, S_DSGUN5, S_DSGUN6,
    S_DSGUN7, S_DSGUN8, S_DSGUN9, S_DSGUNFLASH1, S_DSGUNFLASH2, S_CHAIN,
    S_CHAINDOWN, S_CHAINUP, S_CHAIN1, S_CHAIN2, S_CHAIN3, S_CHAINFLASH1,
    S_CHAINFLASH2, S_MISSILE, S_MISSILEDOWN, S_MISSILEUP, S_MISSILE1,
    S_MISSILE2, S_MISSILE3, S_MISSILEFLASH1, S_MISSILEFLASH2, S_MISSILEFLASH3,
    S_MISSILEFLASH4, S_SAW, S_SAWB, S_SAWDOWN, S_SAWUP, S_SAW1, S_SAW2, S_SAW3,
    S_PLASMA, S_PLASMADOWN, S_PLASMAUP, S_PLASMA1, S_PLASMA2, S_PLASMAFLASH1,
    S_PLASMAFLASH2, S_BFG, S_BFGDOWN, S_BFGUP, S_BFG1, S_BFG2, S_BFG3, S_BFG4,
    S_BFGFLASH1, S_BFGFLASH2, S_BLOOD1, S_BLOOD2, S_BLOOD3, S_PUFF1, S_PUFF2,
    S_PUFF3, S_PUFF4, S_TBALL1, S_TBALL2, S_TBALLX1, S_TBALLX2, S_TBALLX3,
    S_RBALL1, S_RBALL2, S_RBALLX1, S_RBALLX2, S_RBALLX3, S_PLASBALL,
    S_PLASBALL2, S_PLASEXP, S_PLASEXP2, S_PLASEXP3, S_PLASEXP4, S_PLASEXP5,
    S_ROCKET, S_BFGSHOT, S_BFGSHOT2, S_BFGLAND, S_BFGLAND2, S_BFGLAND3,
    S_BFGLAND4, S_BFGLAND5, S_BFGLAND6, S_BFGEXP, S_BFGEXP2, S_BFGEXP3,
    S_BFGEXP4, S_EXPLODE1, S_EXPLODE2, S_EXPLODE3, S_TFOG, S_TFOG01, S_TFOG02,
    S_TFOG2, S_TFOG3, S_TFOG4, S_TFOG5, S_TFOG6, S_TFOG7, S_TFOG8, S_TFOG9,
    S_TFOG10, S_IFOG, S_IFOG01, S_IFOG02, S_IFOG2, S_IFOG3, S_IFOG4, S_IFOG5,
    S_PLAY, S_PLAY_RUN1, S_PLAY_RUN2, S_PLAY_RUN3, S_PLAY_RUN4, S_PLAY_ATK1,
    S_PLAY_ATK2, S_PLAY_PAIN, S_PLAY_PAIN2, S_PLAY_DIE1, S_PLAY_DIE2,
    S_PLAY_DIE3, S_PLAY_DIE4, S_PLAY_DIE5, S_PLAY_DIE6, S_PLAY_DIE7,
    S_PLAY_XDIE1, S_PLAY_XDIE2, S_PLAY_XDIE3, S_PLAY_XDIE4, S_PLAY_XDIE5,
    S_PLAY_XDIE6, S_PLAY_XDIE7, S_PLAY_XDIE8, S_PLAY_XDIE9, S_POSS_STND,
    S_POSS_STND2, S_POSS_RUN1, S_POSS_RUN2, S_POSS_RUN3, S_POSS_RUN4,
    S_POSS_RUN5, S_POSS_RUN6, S_POSS_RUN7, S_POSS_RUN8, S_POSS_ATK1,
    S_POSS_ATK2, S_POSS_ATK3, S_POSS_PAIN, S_POSS_PAIN2, S_POSS_DIE1,
    S_POSS_DIE2, S_POSS_DIE3, S_POSS_DIE4, S_POSS_DIE5, S_POSS_XDIE1,
    S_POSS_XDIE2, S_POSS_XDIE3, S_POSS_XDIE4, S_POSS_XDIE5, S_POSS_XDIE6,
    S_POSS_XDIE7, S_POSS_XDIE8, S_POSS_XDIE9, S_POSS_RAISE1, S_POSS_RAISE2,
    S_POSS_RAISE3, S_POSS_RAISE4, S_SPOS_STND, S_SPOS_STND2, S_SPOS_RUN1,
    S_SPOS_RUN2, S_SPOS_RUN3, S_SPOS_RUN4, S_SPOS_RUN5, S_SPOS_RUN6,
    S_SPOS_RUN7, S_SPOS_RUN8, S_SPOS_ATK1, S_SPOS_ATK2, S_SPOS_ATK3,
    S_SPOS_PAIN, S_SPOS_PAIN2, S_SPOS_DIE1, S_SPOS_DIE2, S_SPOS_DIE3,
    S_SPOS_DIE4, S_SPOS_DIE5, S_SPOS_XDIE1, S_SPOS_XDIE2, S_SPOS_XDIE3,
    S_SPOS_XDIE4, S_SPOS_XDIE5, S_SPOS_XDIE6, S_SPOS_XDIE7, S_SPOS_XDIE8,
    S_SPOS_XDIE9, S_SPOS_RAISE1, S_SPOS_RAISE2, S_SPOS_RAISE3, S_SPOS_RAISE4,
    S_SPOS_RAISE5, S_VILE_STND, S_VILE_STND2, S_VILE_RUN1, S_VILE_RUN2,
    S_VILE_RUN3, S_VILE_RUN4, S_VILE_RUN5, S_VILE_RUN6, S_VILE_RUN7,
    S_VILE_RUN8, S_VILE_RUN9, S_VILE_RUN10, S_VILE_RUN11, S_VILE_RUN12,
    S_VILE_ATK1, S_VILE_ATK2, S_VILE_ATK3, S_VILE_ATK4, S_VILE_ATK5,
    S_VILE_ATK6, S_VILE_ATK7, S_VILE_ATK8, S_VILE_ATK9, S_VILE_ATK10,
    S_VILE_ATK11, S_VILE_HEAL1, S_VILE_HEAL2, S_VILE_HEAL3, S_VILE_PAIN,
    S_VILE_PAIN2, S_VILE_DIE1, S_VILE_DIE2, S_VILE_DIE3, S_VILE_DIE4,
    S_VILE_DIE5, S_VILE_DIE6, S_VILE_DIE7, S_VILE_DIE8, S_VILE_DIE9,
    S_VILE_DIE10, S_FIRE1, S_FIRE2, S_FIRE3, S_FIRE4, S_FIRE5, S_FIRE6, S_FIRE7,
    S_FIRE8, S_FIRE9, S_FIRE10, S_FIRE11, S_FIRE12, S_FIRE13, S_FIRE14,
    S_FIRE15, S_FIRE16, S_FIRE17, S_FIRE18, S_FIRE19, S_FIRE20, S_FIRE21,
    S_FIRE22, S_FIRE23, S_FIRE24, S_FIRE25, S_FIRE26, S_FIRE27, S_FIRE28,
    S_FIRE29, S_FIRE30, S_TRACER, S_TRACEEXP1, S_TRACEEXP2, S_TRACEEXP3,
    S_SMOKE1, S_SMOKE2, S_SMOKE3, S_SMOKE4, S_SMOKE5, S_SKEL_STND, S_SKEL_STND2,
    S_SKEL_RUN1, S_SKEL_RUN2, S_SKEL_RUN3, S_SKEL_RUN4, S_SKEL_RUN5,
    S_SKEL_RUN6, S_SKEL_RUN7, S_SKEL_RUN8, S_SKEL_RUN9, S_SKEL_RUN10,
    S_SKEL_RUN11, S_SKEL_RUN12, S_SKEL_FIST1, S_SKEL_FIST2, S_SKEL_FIST3,
    S_SKEL_FIST4, S_SKEL_MISS1, S_SKEL_MISS2, S_SKEL_MISS3, S_SKEL_MISS4,
    S_SKEL_PAIN, S_SKEL_PAIN2, S_SKEL_DIE1, S_SKEL_DIE2, S_SKEL_DIE3,
    S_SKEL_DIE4, S_SKEL_DIE5, S_SKEL_DIE6, S_SKEL_RAISE1, S_SKEL_RAISE2,
    S_SKEL_RAISE3, S_SKEL_RAISE4, S_SKEL_RAISE5, S_SKEL_RAISE6, S_FATSHOT1,
    S_FATSHOT2, S_FATSHOTX1, S_FATSHOTX2, S_FATSHOTX3, S_FATT_STND,
    S_FATT_STND2, S_FATT_RUN1, S_FATT_RUN2, S_FATT_RUN3, S_FATT_RUN4,
    S_FATT_RUN5, S_FATT_RUN6, S_FATT_RUN7, S_FATT_RUN8, S_FATT_RUN9,
    S_FATT_RUN10, S_FATT_RUN11, S_FATT_RUN12, S_FATT_ATK1, S_FATT_ATK2,
    S_FATT_ATK3, S_FATT_ATK4, S_FATT_ATK5, S_FATT_ATK6, S_FATT_ATK7,
    S_FATT_ATK8, S_FATT_ATK9, S_FATT_ATK10, S_FATT_PAIN, S_FATT_PAIN2,
    S_FATT_DIE1, S_FATT_DIE2, S_FATT_DIE3, S_FATT_DIE4, S_FATT_DIE5,
    S_FATT_DIE6, S_FATT_DIE7, S_FATT_DIE8, S_FATT_RAISE1, S_FATT_RAISE2,
    S_FATT_RAISE3, S_FATT_RAISE4, S_FATT_RAISE5, S_FATT_RAISE6, S_FATT_RAISE7,
    S_FATT_RAISE8, S_CPOS_STND, S_CPOS_STND2, S_CPOS_RUN1, S_CPOS_RUN2,
    S_CPOS_RUN3, S_CPOS_RUN4, S_CPOS_RUN5, S_CPOS_RUN6, S_CPOS_RUN7,
    S_CPOS_RUN8, S_CPOS_ATK1, S_CPOS_ATK2, S_CPOS_ATK3, S_CPOS_ATK4,
    S_CPOS_PAIN, S_CPOS_PAIN2, S_CPOS_DIE1, S_CPOS_DIE2, S_CPOS_DIE3,
    S_CPOS_DIE4, S_CPOS_DIE5, S_CPOS_DIE6, S_CPOS_DIE7, S_CPOS_XDIE1,
    S_CPOS_XDIE2, S_CPOS_XDIE3, S_CPOS_XDIE4, S_CPOS_XDIE5, S_CPOS_XDIE6,
    S_CPOS_RAISE1, S_CPOS_RAISE2, S_CPOS_RAISE3, S_CPOS_RAISE4, S_CPOS_RAISE5,
    S_CPOS_RAISE6, S_CPOS_RAISE7, S_TROO_STND, S_TROO_STND2, S_TROO_RUN1,
    S_TROO_RUN2, S_TROO_RUN3, S_TROO_RUN4, S_TROO_RUN5, S_TROO_RUN6,
    S_TROO_RUN7, S_TROO_RUN8, S_TROO_ATK1, S_TROO_ATK2, S_TROO_ATK3,
    S_TROO_PAIN, S_TROO_PAIN2, S_TROO_DIE1, S_TROO_DIE2, S_TROO_DIE3,
    S_TROO_DIE4, S_TROO_DIE5, S_TROO_XDIE1, S_TROO_XDIE2, S_TROO_XDIE3,
    S_TROO_XDIE4, S_TROO_XDIE5, S_TROO_XDIE6, S_TROO_XDIE7, S_TROO_XDIE8,
    S_TROO_RAISE1, S_TROO_RAISE2, S_TROO_RAISE3, S_TROO_RAISE4, S_TROO_RAISE5,
    S_SARG_STND, S_SARG_STND2, S_SARG_RUN1, S_SARG_RUN2, S_SARG_RUN3,
    S_SARG_RUN4, S_SARG_RUN5, S_SARG_RUN6, S_SARG_RUN7, S_SARG_RUN8,
    S_SARG_ATK1, S_SARG_ATK2, S_SARG_ATK3, S_SARG_PAIN, S_SARG_PAIN2,
    S_SARG_DIE1, S_SARG_DIE2, S_SARG_DIE3, S_SARG_DIE4, S_SARG_DIE5,
    S_SARG_DIE6, S_SARG_RAISE1, S_SARG_RAISE2, S_SARG_RAISE3, S_SARG_RAISE4,
    S_SARG_RAISE5, S_SARG_RAISE6, S_HEAD_STND, S_HEAD_RUN1, S_HEAD_ATK1,
    S_HEAD_ATK2, S_HEAD_ATK3, S_HEAD_PAIN, S_HEAD_PAIN2, S_HEAD_PAIN3,
    S_HEAD_DIE1, S_HEAD_DIE2, S_HEAD_DIE3, S_HEAD_DIE4, S_HEAD_DIE5,
    S_HEAD_DIE6, S_HEAD_RAISE1, S_HEAD_RAISE2, S_HEAD_RAISE3, S_HEAD_RAISE4,
    S_HEAD_RAISE5, S_HEAD_RAISE6, S_BRBALL1, S_BRBALL2, S_BRBALLX1, S_BRBALLX2,
    S_BRBALLX3, S_BOSS_STND, S_BOSS_STND2, S_BOSS_RUN1, S_BOSS_RUN2,
    S_BOSS_RUN3, S_BOSS_RUN4, S_BOSS_RUN5, S_BOSS_RUN6, S_BOSS_RUN7,
    S_BOSS_RUN8, S_BOSS_ATK1, S_BOSS_ATK2, S_BOSS_ATK3, S_BOSS_PAIN,
    S_BOSS_PAIN2, S_BOSS_DIE1, S_BOSS_DIE2, S_BOSS_DIE3, S_BOSS_DIE4,
    S_BOSS_DIE5, S_BOSS_DIE6, S_BOSS_DIE7, S_BOSS_RAISE1, S_BOSS_RAISE2,
    S_BOSS_RAISE3, S_BOSS_RAISE4, S_BOSS_RAISE5, S_BOSS_RAISE6, S_BOSS_RAISE7,
    S_BOS2_STND, S_BOS2_STND2, S_BOS2_RUN1, S_BOS2_RUN2, S_BOS2_RUN3,
    S_BOS2_RUN4, S_BOS2_RUN5, S_BOS2_RUN6, S_BOS2_RUN7, S_BOS2_RUN8,
    S_BOS2_ATK1, S_BOS2_ATK2, S_BOS2_ATK3, S_BOS2_PAIN, S_BOS2_PAIN2,
    S_BOS2_DIE1, S_BOS2_DIE2, S_BOS2_DIE3, S_BOS2_DIE4, S_BOS2_DIE5,
    S_BOS2_DIE6, S_BOS2_DIE7, S_BOS2_RAISE1, S_BOS2_RAISE2, S_BOS2_RAISE3,
    S_BOS2_RAISE4, S_BOS2_RAISE5, S_BOS2_RAISE6, S_BOS2_RAISE7, S_SKULL_STND,
    S_SKULL_STND2, S_SKULL_RUN1, S_SKULL_RUN2, S_SKULL_ATK1, S_SKULL_ATK2,
    S_SKULL_ATK3, S_SKULL_ATK4, S_SKULL_PAIN, S_SKULL_PAIN2, S_SKULL_DIE1,
    S_SKULL_DIE2, S_SKULL_DIE3, S_SKULL_DIE4, S_SKULL_DIE5, S_SKULL_DIE6,
    S_SPID_STND, S_SPID_STND2, S_SPID_RUN1, S_SPID_RUN2, S_SPID_RUN3,
    S_SPID_RUN4, S_SPID_RUN5, S_SPID_RUN6, S_SPID_RUN7, S_SPID_RUN8,
    S_SPID_RUN9, S_SPID_RUN10, S_SPID_RUN11, S_SPID_RUN12, S_SPID_ATK1,
    S_SPID_ATK2, S_SPID_ATK3, S_SPID_ATK4, S_SPID_PAIN, S_SPID_PAIN2,
    S_SPID_DIE1, S_SPID_DIE2, S_SPID_DIE3, S_SPID_DIE4, S_SPID_DIE5,
    S_SPID_DIE6, S_SPID_DIE7, S_SPID_DIE8, S_SPID_DIE9, S_SPID_DIE10,
    S_SPID_DIE11, S_BSPI_STND, S_BSPI_STND2, S_BSPI_RUN1, S_BSPI_RUN2,
    S_BSPI_RUN3, S_BSPI_RUN4, S_BSPI_RUN5, S_BSPI_RUN6, S_BSPI_RUN7,
    S_BSPI_RUN8, S_BSPI_RUN9, S_BSPI_RUN10, S_BSPI_RUN11, S_BSPI_RUN12,
    S_BSPI_ATK1, S_BSPI_ATK2, S_BSPI_ATK3, S_BSPI_ATK4, S_BSPI_PAIN,
    S_BSPI_PAIN2, S_BSPI_DIE1, S_BSPI_DIE2, S_BSPI_DIE3, S_BSPI_DIE4,
    S_BSPI_DIE5, S_BSPI_DIE6, S_BSPI_DIE7, S_ARACH_PLAZ, S_ARACH_PLAZ2,
    S_ARACH_PLEX, S_BSPI_RAISE1, S_BSPI_RAISE2, S_BSPI_RAISE3, S_BSPI_RAISE4,
    S_BSPI_RAISE5, S_BSPI_RAISE6, S_BSPI_RAISE7, S_ARACH_PLEX2, S_ARACH_PLEX3,
    S_ARACH_PLEX4, S_CYBER_STND, S_CYBER_STND2, S_CYBER_RUN1, S_CYBER_RUN2,
    S_CYBER_RUN3, S_CYBER_RUN4, S_CYBER_RUN5, S_CYBER_RUN6, S_CYBER_RUN7,
    S_CYBER_RUN8, S_CYBER_ATK1, S_CYBER_ATK2, S_CYBER_ATK3, S_CYBER_ATK4,
    S_CYBER_ATK5, S_CYBER_ATK6, S_CYBER_PAIN, S_CYBER_DIE1, S_CYBER_DIE2,
    S_CYBER_DIE3, S_CYBER_DIE4, S_CYBER_DIE5, S_CYBER_DIE6, S_CYBER_DIE7,
    S_CYBER_DIE8, S_CYBER_DIE9, S_CYBER_DIE10, S_PAIN_STND, S_PAIN_RUN1,
    S_PAIN_RUN2, S_PAIN_RUN3, S_PAIN_RUN4, S_PAIN_RUN5, S_PAIN_RUN6,
    S_PAIN_ATK1, S_PAIN_ATK2, S_PAIN_ATK3, S_PAIN_ATK4, S_PAIN_PAIN,
    S_PAIN_PAIN2, S_PAIN_DIE1, S_PAIN_DIE2, S_PAIN_DIE3, S_PAIN_DIE4,
    S_PAIN_DIE5, S_PAIN_DIE6, S_PAIN_RAISE1, S_PAIN_RAISE2, S_PAIN_RAISE3,
    S_PAIN_RAISE4, S_PAIN_RAISE5, S_PAIN_RAISE6, S_ARM1, S_ARM1A, S_ARM2,
    S_ARM2A, S_BAR1, S_BAR2, S_BEXP, S_BEXP2, S_BEXP3, S_BEXP4, S_BEXP5,
    S_BBAR1, S_BBAR2, S_BBAR3, S_BON1, S_BON1A, S_BON1B, S_BON1C, S_BON1D,
    S_BON1E, S_BON2, S_BON2A, S_BON2B, S_BON2C, S_BON2D, S_BON2E, S_BKEY,
    S_BKEY2, S_RKEY, S_RKEY2, S_YKEY, S_YKEY2, S_BSKULL, S_BSKULL2, S_RSKULL,
    S_RSKULL2, S_YSKULL, S_YSKULL2, S_STIM, S_MEDI, S_SOUL, S_SOUL2, S_SOUL3,
    S_SOUL4, S_SOUL5, S_SOUL6, S_PINV, S_PINV2, S_PINV3, S_PINV4, S_PSTR,
    S_PINS, S_PINS2, S_PINS3, S_PINS4, S_SUIT, S_PMAP, S_PMAP2, S_PMAP3,
    S_PMAP4, S_PMAP5, S_PMAP6, S_PVIS, S_PVIS2, S_CLIP, S_AMMO, S_ROCK, S_BROK,
    S_CELL, S_CELP, S_SHEL, S_SBOX, S_BPAK, S_BFUG, S_MGUN, S_CSAW, S_LAUN,
    S_PLAS, S_SHOT, S_SHOT2, S_COLU, S_STALAG, S_BLOODYTWITCH, S_BLOODYTWITCH2,
    S_BLOODYTWITCH3, S_BLOODYTWITCH4, S_DEADTORSO, S_DEADBOTTOM, S_HEADSONSTICK,
    S_GIBS, S_HEADONASTICK, S_HEADCANDLES, S_HEADCANDLES2, S_DEADSTICK,
    S_LIVESTICK, S_LIVESTICK2, S_MEAT2, S_MEAT3, S_MEAT4, S_MEAT5, S_STALAGTITE,
    S_TALLGRNCOL, S_SHRTGRNCOL, S_TALLREDCOL, S_SHRTREDCOL, S_CANDLESTIK,
    S_CANDELABRA, S_SKULLCOL, S_TORCHTREE, S_BIGTREE, S_TECHPILLAR, S_EVILEYE,
    S_EVILEYE2, S_EVILEYE3, S_EVILEYE4, S_FLOATSKULL, S_FLOATSKULL2,
    S_FLOATSKULL3, S_HEARTCOL, S_HEARTCOL2, S_BLUETORCH, S_BLUETORCH2,
    S_BLUETORCH3, S_BLUETORCH4, S_GREENTORCH, S_GREENTORCH2, S_GREENTORCH3,
    S_GREENTORCH4, S_REDTORCH, S_REDTORCH2, S_REDTORCH3, S_REDTORCH4,
    S_BTORCHSHRT, S_BTORCHSHRT2, S_BTORCHSHRT3, S_BTORCHSHRT4, S_GTORCHSHRT,
    S_GTORCHSHRT2, S_GTORCHSHRT3, S_GTORCHSHRT4, S_RTORCHSHRT, S_RTORCHSHRT2,
    S_RTORCHSHRT3, S_RTORCHSHRT4, S_COMMKEEN, S_COMMKEEN2, S_COMMKEEN3,
    S_COMMKEEN4, S_COMMKEEN5, S_COMMKEEN6, S_COMMKEEN7, S_COMMKEEN8,
    S_COMMKEEN9, S_COMMKEEN10, S_COMMKEEN11, S_COMMKEEN12, S_HANGNOGUTS,
    S_HANGBNOBRAIN, S_HANGTLOOKDN, S_HANGTSKULL, S_HANGTLOOKUP, S_HANGTNOBRAIN,
    S_COLONGIBS, S_SMALLPOOL, S_BRAINSTEM
};

static statenum_t statemap[NUMSTATES];
static int statemap_size;
static statenum_t statemap_rev[NUMSTATES];
static boolean stateexists[NUMSTATES];

static mobjtype_t mobjmap_1_6[] = {
    MT_PLAYER, MT_POSSESSED, MT_SHOTGUY, MT_VILE, MT_FIRE, MT_UNDEAD,
    MT_TRACER, MT_SMOKE, MT_FATSO, MT_FATSHOT, MT_CHAINGUY, MT_TROOP,
    MT_SERGEANT, MT_SHADOWS, MT_HEAD, MT_BRUISER, MT_BRUISERSHOT, MT_KNIGHT,
    MT_SKULL, MT_SPIDER, MT_BABY, MT_CYBORG, MT_PAIN, MT_BARREL, MT_TROOPSHOT,
    MT_HEADSHOT, MT_ROCKET, MT_PLASMA, MT_BFG, MT_PUFF, MT_BLOOD, MT_TFOG,
    MT_IFOG, MT_TELEPORTMAN, MT_EXTRABFG, MT_MISC0, MT_MISC1, MT_MISC2,
    MT_MISC3, MT_MISC4, MT_MISC5, MT_MISC6, MT_MISC7, MT_MISC8, MT_MISC9,
    MT_MISC10, MT_MISC11, MT_MISC12, MT_INV, MT_MISC13, MT_INS, MT_MISC14,
    MT_MISC15, MT_MISC16, MT_CLIP, MT_MISC17, MT_MISC18, MT_MISC19, MT_MISC20,
    MT_MISC21, MT_MISC22, MT_MISC23, MT_MISC24, MT_MISC25, MT_CHAINGUN,
    MT_MISC26, MT_MISC27, MT_MISC28, MT_SHOTGUN, MT_SUPERSHOTGUN, MT_MISC31,
    MT_MISC32, MT_MISC33, MT_MISC34, MT_MISC35, MT_MISC36, MT_MISC37,
    MT_MISC38, MT_MISC39, MT_MISC40, MT_MISC41, MT_MISC42, MT_MISC43,
    MT_MISC44, MT_MISC45, MT_MISC46, MT_MISC47, MT_MISC48, MT_MISC49,
    MT_MISC50, MT_MISC51, MT_MISC52, MT_MISC53, MT_MISC54, MT_MISC55,
    MT_MISC56, MT_MISC57, MT_MISC58, MT_MISC59, MT_MISC60, MT_MISC61,
    MT_MISC62, MT_MISC63, MT_MISC64, MT_MISC65, MT_MISC66, MT_MISC67,
    MT_MISC68, MT_MISC69, MT_MISC70, MT_MISC71, MT_MISC72, MT_MISC73,
    MT_MISC74, MT_MISC75, MT_MISC76, MT_MISC77, MT_KEEN, MT_MISC78, MT_MISC79,
    MT_MISC80, MT_MISC81, MT_MISC82, MT_MISC83, MT_MISC84, MT_MISC85,
    MT_MISC86
};

static mobjtype_t mobjmap[NUMMOBJTYPES];
static int mobjmap_size;
static mobjtype_t mobjmap_rev[NUMMOBJTYPES];
static boolean mobjexists[NUMMOBJTYPES];

void D_Compat_Init(void)
{
    int i, j;
    spritenum_t *sprite;
    statenum_t *state;
    mobjtype_t *mobj;

    // Build tables.

    switch (gameversion)
    {
    case exe_doom_1_6:
        // Doom 1.6 Beta
        spritemap_size = arrlen(spritemap_1_6);
        sprite = spritemap_1_6;

        statemap_size = arrlen(statemap_1_6);
        state = statemap_1_6;

        mobjmap_size = arrlen(mobjmap_1_6);
        mobj = mobjmap_1_6;
        break;
    default:
        // Doom 1.666 onward
        spritemap_size = NUMSPRITES;
        for (i = 0; i < NUMSPRITES; i++)
        {
            spritemap[i] = i;
            spritemap_rev[i] = i;
            spriteexists[i] = true;
        }

        statemap_size = NUMSTATES;
        for (i = 0; i < NUMSTATES; i++)
        {
            statemap[i] = i;
            statemap_rev[i] = i;
            stateexists[i] = true;
        }

        mobjmap_size = NUMMOBJTYPES;
        for (i = 0; i < NUMMOBJTYPES; i++)
        {
            mobjmap[i] = i;
            mobjmap_rev[i] = i;
            mobjexists[i] = true;
        }
        return;
    }

    for (i = 0; i < NUMSPRITES; i++)
    {
        spriteexists[i] = false;
    }

    for (i = 0; i < spritemap_size; i++)
    {
        spritemap[i] = sprite[i];
        spritemap_rev[sprite[i]] = i;
        spriteexists[sprite[i]] = true;
    }

    for (i = 0, j = spritemap_size; i < NUMSPRITES; i++)
    {
        if (!spriteexists[i])
        {
            spritemap_rev[i] = j;
            spritemap[j] = i;
            j++;
        }
    }

    for (i = 0; i < NUMSTATES; i++)
    {
        stateexists[i] = false;
    }

    for (i = 0; i < statemap_size; i++)
    {
        statemap[i] = state[i];
        statemap_rev[state[i]] = i;
        stateexists[state[i]] = true;
    }

    for (i = 0, j = statemap_size; i < NUMSTATES; i++)
    {
        if (!stateexists[i])
        {
            statemap_rev[i] = j;
            statemap[j] = i;
            j++;
        }
    }

    for (i = 0; i < NUMMOBJTYPES; i++)
    {
        mobjexists[i] = false;
    }

    for (i = 0; i < mobjmap_size; i++)
    {
        mobjmap[i] = mobj[i];
        mobjmap_rev[mobj[i]] = i;
        mobjexists[mobj[i]] = true;
    }

    for (i = 0, j = mobjmap_size; i < NUMMOBJTYPES; i++)
    {
        if (!mobjexists[i])
        {
            mobjmap_rev[i] = j;
            mobjmap[j] = i;
            j++;
        }
    }

    // Apply hacks.

    switch (gameversion)
    {
    case exe_doom_1_6:
        // Super shotgun
        states[S_DSGUN1].tics = 5;
        states[S_DSGUN2].tics = 9;
        states[S_DSGUN4].action.acv = NULL;
        states[S_DSGUN5].tics = 6;
        states[S_DSGUN5].action.acv = NULL;
        states[S_DSGUN6].frame = 2;
        states[S_DSGUN7].frame = 1;
        states[S_DSGUN7].action.acv = NULL;
        states[S_DSGUN8].frame = 0;
        states[S_DSGUN8].tics = 7;
        states[S_DSGUN9].frame = 0;
        states[S_DSGUN9].tics = 7;
        states[S_DSGUN9].action.acv = A_ReFire;
        states[S_DSGUN9].nextstate = S_DSGUN;
        states[S_DSGUNFLASH1].frame = 32772;
        states[S_DSGUNFLASH2].frame = 32773;
        // Player
        states[S_PLAY_DIE2].action.acv = A_Scream;
        // Arch vile
        states[S_VILE_DIE1].tics = 5;
        states[S_VILE_DIE2].tics = 5;
        states[S_VILE_DIE3].tics = 5;
        states[S_VILE_DIE4].tics = 5;
        states[S_VILE_DIE5].tics = 5;
        states[S_VILE_DIE6].tics = 5;
        states[S_VILE_DIE7].tics = 5;
        mobjinfo[MT_VILE].seesound = sfx_posit2;
        mobjinfo[MT_VILE].painsound = sfx_popain;
        mobjinfo[MT_VILE].deathsound = sfx_podth2;
        mobjinfo[MT_VILE].speed = 10;
        mobjinfo[MT_VILE].activesound = sfx_posact;
        // Fire
        states[S_FIRE1].sprite = SPR_TFOG;
        states[S_FIRE1].action.acv = A_Fire;
        states[S_FIRE2].sprite = SPR_TFOG;
        states[S_FIRE2].frame = 32768;
        states[S_FIRE3].sprite = SPR_TFOG;
        states[S_FIRE4].sprite = SPR_TFOG;
        states[S_FIRE5].sprite = SPR_TFOG;
        states[S_FIRE5].frame = 32769;
        states[S_FIRE5].action.acv = A_Fire;
        states[S_FIRE6].sprite = SPR_TFOG;
        states[S_FIRE7].sprite = SPR_TFOG;
        states[S_FIRE8].sprite = SPR_TFOG;
        states[S_FIRE8].frame = 32770;
        states[S_FIRE9].sprite = SPR_TFOG;
        states[S_FIRE10].sprite = SPR_TFOG;
        states[S_FIRE11].sprite = SPR_TFOG;
        states[S_FIRE11].frame = 32771;
        states[S_FIRE12].sprite = SPR_TFOG;
        states[S_FIRE13].sprite = SPR_TFOG;
        states[S_FIRE13].frame = 32772;
        states[S_FIRE14].sprite = SPR_TFOG;
        states[S_FIRE14].frame = 32772;
        states[S_FIRE15].sprite = SPR_TFOG;
        states[S_FIRE16].sprite = SPR_TFOG;
        states[S_FIRE16].frame = 32773;
        states[S_FIRE17].sprite = SPR_TFOG;
        states[S_FIRE17].frame = 32773;
        states[S_FIRE18].sprite = SPR_TFOG;
        states[S_FIRE18].frame = 32773;
        states[S_FIRE19].sprite = SPR_TFOG;
        states[S_FIRE19].frame = 32774;
        states[S_FIRE19].action.acv = A_Fire;
        states[S_FIRE20].sprite = SPR_TFOG;
        states[S_FIRE20].frame = 32774;
        states[S_FIRE21].sprite = SPR_TFOG;
        states[S_FIRE21].frame = 32774;
        states[S_FIRE22].sprite = SPR_TFOG;
        states[S_FIRE22].frame = 32775;
        states[S_FIRE23].sprite = SPR_TFOG;
        states[S_FIRE23].frame = 32775;
        states[S_FIRE24].sprite = SPR_TFOG;
        states[S_FIRE24].frame = 32775;
        states[S_FIRE25].sprite = SPR_TFOG;
        states[S_FIRE25].frame = 32776;
        states[S_FIRE26].sprite = SPR_TFOG;
        states[S_FIRE26].frame = 32776;
        states[S_FIRE27].sprite = SPR_TFOG;
        states[S_FIRE27].frame = 32776;
        states[S_FIRE28].sprite = SPR_TFOG;
        states[S_FIRE28].frame = 32777;
        states[S_FIRE29].sprite = SPR_TFOG;
        states[S_FIRE29].frame = 32777;
        states[S_FIRE30].sprite = SPR_TFOG;
        states[S_FIRE30].frame = 32777;
        // Tracer
        states[S_TRACER].sprite = SPR_MISL;
        states[S_TRACER].tics = 1;
        states[S_TRACER].nextstate = S_TRACER;
        states[S_TRACEEXP1].sprite = SPR_MISL;
        states[S_TRACEEXP1].frame = 32769;
        states[S_TRACEEXP2].sprite = SPR_MISL;
        states[S_TRACEEXP2].frame = 32770;
        states[S_TRACEEXP3].sprite = SPR_MISL;
        states[S_TRACEEXP3].frame = 32771;
        // Revenant
        states[S_SKEL_FIST2].action.acv = A_FaceTarget;
        states[S_SKEL_MISS1].frame = 9;
        states[S_SKEL_MISS2].frame = 9;
        states[S_SKEL_DIE1].tics = 10;
        states[S_SKEL_DIE2].tics = 10;
        states[S_SKEL_DIE3].tics = 10;
        states[S_SKEL_DIE4].tics = 10;
        states[S_SKEL_DIE5].tics = 10;
        mobjinfo[MT_UNDEAD].seesound = sfx_posit2;
        mobjinfo[MT_UNDEAD].painsound = sfx_popain;
        mobjinfo[MT_UNDEAD].deathsound = sfx_podth2;
        mobjinfo[MT_UNDEAD].activesound = sfx_posact;
        // Mancubus fireball
        states[S_FATSHOT1].sprite = SPR_BAL7;
        states[S_FATSHOT2].sprite = SPR_BAL7;
        states[S_FATSHOTX1].sprite = SPR_BAL7;
        states[S_FATSHOTX1].frame = 32770;
        states[S_FATSHOTX1].tics = 6;
        states[S_FATSHOTX2].sprite = SPR_BAL7;
        states[S_FATSHOTX2].frame = 32771;
        states[S_FATSHOTX3].sprite = SPR_BAL7;
        states[S_FATSHOTX3].frame = 32772;
        states[S_FATSHOTX3].tics = 6;
        // Mancubus
        states[S_FATT_DIE1].tics = 10;
        states[S_FATT_DIE2].tics = 10;
        states[S_FATT_DIE3].tics = 10;
        states[S_FATT_DIE4].tics = 10;
        states[S_FATT_DIE5].tics = 10;
        states[S_FATT_DIE6].tics = 10;
        states[S_FATT_DIE7].tics = 10;
        states[S_FATT_DIE8].tics = -1;
        states[S_FATT_DIE8].nextstate = S_NULL;
        states[S_FATT_RAISE8].tics = -1;
        mobjinfo[MT_FATSO].seesound = sfx_posit2;
        mobjinfo[MT_FATSO].painsound = sfx_popain;
        mobjinfo[MT_FATSO].deathsound = sfx_podth2;
        // Hell knight
        mobjinfo[MT_KNIGHT].seesound = sfx_brssit;
        mobjinfo[MT_KNIGHT].deathsound = sfx_brsdth;
        // Lost Soul
        mobjinfo[MT_SKULL].flags = MF_SOLID | MF_SHOOTABLE | MF_FLOAT
                                 | MF_NOGRAVITY | MF_COUNTKILL;
        // Arachnotron
        states[S_BSPI_RUN1].action.acv = A_Metal;
        states[S_BSPI_RUN5].action.acv = A_Metal;
        states[S_BSPI_RUN7].action.acv = A_Chase;
        states[S_BSPI_RUN9].action.acv = A_Metal;
        states[S_BSPI_ATK2].action.acv = A_PosAttack;
        states[S_BSPI_ATK3].action.acv = A_PosAttack;
        states[S_BSPI_DIE2].tics = 10;
        states[S_BSPI_DIE3].tics = 10;
        states[S_BSPI_DIE4].tics = 10;
        states[S_BSPI_DIE5].tics = 10;
        states[S_BSPI_DIE6].tics = 10;
        states[S_BSPI_DIE7].tics = 10;
        states[S_BSPI_DIE7].action.acv = NULL;
        states[S_BSPI_DIE7].nextstate = S_ARACH_PLAZ;
        states[S_ARACH_PLAZ].sprite = SPR_BSPI;
        states[S_ARACH_PLAZ].frame = 16;
        states[S_ARACH_PLAZ].tics = 10;
        states[S_ARACH_PLAZ2].sprite = SPR_BSPI;
        states[S_ARACH_PLAZ2].frame = 17;
        states[S_ARACH_PLAZ2].tics = 10;
        states[S_ARACH_PLAZ2].nextstate = S_ARACH_PLEX;
        states[S_ARACH_PLEX].sprite = SPR_BSPI;
        states[S_ARACH_PLEX].frame = 18;
        states[S_ARACH_PLEX].tics = -1;
        states[S_ARACH_PLEX].nextstate = S_NULL;
        states[S_BSPI_RAISE1].frame = 18;
        states[S_BSPI_RAISE2].frame = 17;
        states[S_BSPI_RAISE3].frame = 16;
        states[S_BSPI_RAISE4].frame = 15;
        states[S_BSPI_RAISE5].frame = 14;
        states[S_BSPI_RAISE6].frame = 13;
        states[S_BSPI_RAISE7].frame = 12;
        states[S_BSPI_RAISE7].nextstate = S_ARACH_PLEX2;
        states[S_ARACH_PLEX2].sprite = SPR_BSPI;
        states[S_ARACH_PLEX2].frame = 11;
        states[S_ARACH_PLEX3].sprite = SPR_BSPI;
        states[S_ARACH_PLEX3].frame = 10;
        states[S_ARACH_PLEX4].sprite = SPR_BSPI;
        states[S_ARACH_PLEX4].frame = 9;
        states[S_ARACH_PLEX4].nextstate = S_BSPI_RUN1;
        mobjinfo[MT_BABY].seesound = sfx_spisit;
        mobjinfo[MT_BABY].attacksound = sfx_shotgn;
        mobjinfo[MT_BABY].deathsound = sfx_spidth;
        mobjinfo[MT_BABY].activesound = sfx_dmact;
        // Pain elemental
        states[S_PAIN_DIE5].action.acv = A_Fall;
        mobjinfo[MT_PAIN].seesound = sfx_cacsit;
        mobjinfo[MT_PAIN].painsound = sfx_dmpain;
        mobjinfo[MT_PAIN].deathsound = sfx_cacdth;
        // Keen
        states[S_COMMKEEN].tics = 4;
        states[S_COMMKEEN2].tics = 4;
        states[S_COMMKEEN3].tics = 4;
        states[S_COMMKEEN3].action.acv = NULL;
        states[S_COMMKEEN4].tics = 4;
        states[S_COMMKEEN5].tics = 4;
        states[S_COMMKEEN6].tics = 4;
        states[S_COMMKEEN7].tics = 4;
        states[S_COMMKEEN8].tics = 4;
        states[S_COMMKEEN9].tics = 4;
        states[S_COMMKEEN10].tics = 4;
        states[S_COMMKEEN11].tics = 4;
        states[S_COMMKEEN11].action.acv = NULL;
        states[S_COMMKEEN12].tics = 4;
        states[S_COMMKEEN12].nextstate = S_COMMKEEN;
        break;
    default:
        break;
    }
}

spritenum_t D_Compat_SpriteToOld(spritenum_t sprite)
{
    if (sprite >= NUMSPRITES)
    {
        return sprite;
    }
    return spritemap_rev[sprite];
}

spritenum_t D_Compat_SpriteToNew(spritenum_t sprite)
{
    if (sprite >= NUMSPRITES)
    {
        return sprite;
    }
    return spritemap[sprite];
}

boolean D_Compat_SpriteExists(spritenum_t sprite)
{
    return spriteexists[sprite];
}

statenum_t D_Compat_StateToOld(statenum_t state)
{
    if (state >= NUMSTATES)
    {
        return state;
    }
    return statemap_rev[state];
}

statenum_t D_Compat_StateToNew(statenum_t state)
{
    if (state >= NUMSTATES)
    {
        return state;
    }
    return statemap[state];
}

boolean D_Compat_StateExists(statenum_t state)
{
    return stateexists[state];
}

mobjtype_t D_Compat_MobjToOld(mobjtype_t mobj)
{
    if (mobj >= NUMMOBJTYPES)
    {
        return mobj;
    }
    return mobjmap_rev[mobj];
}

mobjtype_t D_Compat_MobjToNew(mobjtype_t mobj)
{
    if (mobj >= NUMMOBJTYPES)
    {
        return mobj;
    }
    return mobjmap[mobj];
}

boolean D_Compat_MobjExists(mobjtype_t mobj)
{
    return mobjexists[mobj];
}
