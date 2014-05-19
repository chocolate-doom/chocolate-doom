//
// Copyright(C) 1993-1996 Id Software, Inc.
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
// DESCRIPTION:
//	Created by the sound utility written by Dave Taylor.
//	Kept as a sample, DOOM2  sounds. Frozen.
//

#ifndef __SOUNDS__
#define __SOUNDS__

#include "i_sound.h"

// the complete set of sound effects
extern sfxinfo_t	S_sfx[];

// the complete set of music
extern musicinfo_t	S_music[];

//
// Identifiers for all music in game.
//

// villsa [STRIFE]
typedef enum
{
    mus_None,
    mus_logo,
    mus_action,
    mus_tavern,
    mus_danger,
    mus_fast,
    mus_intro,
    mus_darker,
    mus_strike,
    mus_slide,
    mus_tribal,
    mus_march,
    mus_danger2,
    mus_mood,
    mus_castle,
    mus_darker2,
    mus_action2,
    mus_fight,
    mus_spense,
    mus_slide2,
    mus_strike2,
    mus_dark,
    mus_tech,
    mus_slide3,
    mus_drone,
    mus_panthr,
    mus_sad,
    mus_instry,
    mus_tech2,
    mus_action3,
    mus_instry2,
    mus_drone2,
    mus_fight2,
    mus_happy,
    mus_end,
    NUMMUSIC
} musicenum_t;


//
// Identifiers for all sfx in game.
//

typedef enum
{
    sfx_None,
    sfx_swish,
    sfx_meatht,
    sfx_mtalht,
    sfx_wpnup,
    sfx_rifle,
    sfx_mislht,
    sfx_barexp,
    sfx_flburn,
    sfx_flidl,
    sfx_agrsee,
    sfx_plpain,
    sfx_pcrush,
    sfx_pespna,
    sfx_pespnb,
    sfx_pespnc,
    sfx_pespnd,
    sfx_agrdpn,
    sfx_pldeth,
    sfx_plxdth,
    sfx_slop,
    sfx_rebdth,
    sfx_agrdth,
    sfx_lgfire,
    sfx_smfire,
    sfx_alarm,
    sfx_drlmto,
    sfx_drlmtc,
    sfx_drsmto,
    sfx_drsmtc,
    sfx_drlwud,
    sfx_drswud,
    sfx_drston,
    sfx_bdopn,
    sfx_bdcls,
    sfx_swtchn,
    sfx_swbolt,
    sfx_swscan,
    sfx_yeah,
    sfx_mask,
    sfx_pstart,
    sfx_pstop,
    sfx_itemup,
    sfx_bglass,
    sfx_wriver,
    sfx_wfall,
    sfx_wdrip,
    sfx_wsplsh,
    sfx_rebact,
    sfx_agrac1,
    sfx_agrac2,
    sfx_agrac3,
    sfx_agrac4,
    sfx_ambppl,
    sfx_ambbar,
    sfx_telept,
    sfx_ratact,
    sfx_itmbk,
    sfx_xbow,
    sfx_burnme,
    sfx_oof,
    sfx_wbrldt,
    sfx_psdtha,
    sfx_psdthb,
    sfx_psdthc,
    sfx_rb2pn,
    sfx_rb2dth,
    sfx_rb2see,
    sfx_rb2act,
    sfx_firxpl,
    sfx_stnmov,
    sfx_noway,
    sfx_rlaunc,
    sfx_rflite,
    sfx_radio,
    sfx_pulchn,
    sfx_swknob,
    sfx_keycrd,
    sfx_swston,
    sfx_sntsee,
    sfx_sntdth,
    sfx_sntact,
    sfx_pgrdat,
    sfx_pgrsee,
    sfx_pgrdpn,
    sfx_pgrdth,
    sfx_pgract,
    sfx_proton,
    sfx_protfl,
    sfx_plasma,
    sfx_dsrptr,
    sfx_reavat,
    sfx_revbld,
    sfx_revsee,
    sfx_reavpn,
    sfx_revdth,
    sfx_revact,
    sfx_spisit,
    sfx_spdwlk,
    sfx_spidth,
    sfx_spdatk,
    sfx_chant,
    sfx_static,
    sfx_chain,
    sfx_tend,
    sfx_phoot,
    sfx_explod,
    sfx_sigil,
    sfx_sglhit,
    sfx_siglup,
    sfx_prgpn,
    sfx_progac,
    sfx_lorpn,
    sfx_lorsee,
    sfx_difool,
    sfx_inqdth,
    sfx_inqact,
    sfx_inqsee,
    sfx_inqjmp,
    sfx_amaln1,
    sfx_amaln2,
    sfx_amaln3,
    sfx_amaln4,
    sfx_amaln5,
    sfx_amaln6,
    sfx_mnalse,
    sfx_alnsee,
    sfx_alnpn,
    sfx_alnact,
    sfx_alndth,
    sfx_mnaldt,
    sfx_reactr,
    sfx_airlck,
    sfx_drchno,
    sfx_drchnc,
    sfx_valve,
    NUMSFX
} sfxenum_t;

#endif
