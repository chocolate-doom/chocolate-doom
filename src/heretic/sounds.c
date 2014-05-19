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


// sounds.c

#include "doomdef.h"
#include "i_sound.h"
#include "sounds.h"

// Music info

#define MUSIC(name) \
    { name, 0, NULL, NULL }

musicinfo_t S_music[] = {
    MUSIC("MUS_E1M1"),            // 1-1
    MUSIC("MUS_E1M2"),
    MUSIC("MUS_E1M3"),
    MUSIC("MUS_E1M4"),
    MUSIC("MUS_E1M5"),
    MUSIC("MUS_E1M6"),
    MUSIC("MUS_E1M7"),
    MUSIC("MUS_E1M8"),
    MUSIC("MUS_E1M9"),

    MUSIC("MUS_E2M1"),            // 2-1
    MUSIC("MUS_E2M2"),
    MUSIC("MUS_E2M3"),
    MUSIC("MUS_E2M4"),
    MUSIC("MUS_E1M4"),
    MUSIC("MUS_E2M6"),
    MUSIC("MUS_E2M7"),
    MUSIC("MUS_E2M8"),
    MUSIC("MUS_E2M9"),

    MUSIC("MUS_E1M1"),            // 3-1
    MUSIC("MUS_E3M2"),
    MUSIC("MUS_E3M3"),
    MUSIC("MUS_E1M6"),
    MUSIC("MUS_E1M3"),
    MUSIC("MUS_E1M2"),
    MUSIC("MUS_E1M5"),
    MUSIC("MUS_E1M9"),
    MUSIC("MUS_E2M6"),

    MUSIC("MUS_E1M6"),            // 4-1
    MUSIC("MUS_E1M2"),
    MUSIC("MUS_E1M3"),
    MUSIC("MUS_E1M4"),
    MUSIC("MUS_E1M5"),
    MUSIC("MUS_E1M1"),
    MUSIC("MUS_E1M7"),
    MUSIC("MUS_E1M8"),
    MUSIC("MUS_E1M9"),

    MUSIC("MUS_E2M1"),            // 5-1
    MUSIC("MUS_E2M2"),
    MUSIC("MUS_E2M3"),
    MUSIC("MUS_E2M4"),
    MUSIC("MUS_E1M4"),
    MUSIC("MUS_E2M6"),
    MUSIC("MUS_E2M7"),
    MUSIC("MUS_E2M8"),
    MUSIC("MUS_E2M9"),

    MUSIC("MUS_E3M2"),            // 6-1
    MUSIC("MUS_E3M3"),            // 6-2
    MUSIC("MUS_E1M6"),            // 6-3

    MUSIC("MUS_TITL"),
    MUSIC("MUS_INTR"),
    MUSIC("MUS_CPTD")
};

// Sound info

    /* Macro for original heretic sfxinfo_t 
#define SOUND(name, priority, numchannels) \
    { name, NULL, priority, -1, NULL, 0, numchannels }
#define SOUND_LINK(name, link_id, priority, numchannels) \
    { name, &S_sfx[link_id], priority, -1, NULL, 0, numchannels }
    */

#define SOUND(name, priority, numchannels) \
    { NULL, name, priority, NULL, -1, -1, -1, 0, numchannels, NULL }
#define SOUND_LINK(name, link_id, priority, numchannels) \
    { NULL, name, priority, &S_sfx[link_id], 0, 0, -1, 0, numchannels, NULL }

sfxinfo_t S_sfx[] = {
    SOUND("",        0,   0),
    SOUND("gldhit",  32,  2),
    SOUND("gntful",  32,  -1),
    SOUND("gnthit",  32,  -1),
    SOUND("gntpow",  32,  -1),
    SOUND("gntact",  32,  -1),
    SOUND("gntuse",  32,  -1),
    SOUND("phosht",  32,  2),
    SOUND("phohit",  32,  -1),
    SOUND_LINK("-phopow", sfx_hedat1, 32, 1),
    SOUND("lobsht",  20,  2),
    SOUND("lobhit",  20,  2),
    SOUND("lobpow",  20,  2),
    SOUND("hrnsht",  32,  2),
    SOUND("hrnhit",  32,  2),
    SOUND("hrnpow",  32,  2),
    SOUND("ramphit", 32,  2),
    SOUND("ramrain", 10,  2),
    SOUND("bowsht",  32,  2),
    SOUND("stfhit",  32,  2),
    SOUND("stfpow",  32,  2),
    SOUND("stfcrk",  32,  2),
    SOUND("impsit",  32,  2),
    SOUND("impat1",  32,  2),
    SOUND("impat2",  32,  2),
    SOUND("impdth",  80,  2),
    SOUND_LINK("-impact", sfx_impsit, 20, 2),
    SOUND("imppai",  32,  2),
    SOUND("mumsit",  32,  2),
    SOUND("mumat1",  32,  2),
    SOUND("mumat2",  32,  2),
    SOUND("mumdth",  80,  2),
    SOUND_LINK("-mumact", sfx_mumsit, 20, 2),
    SOUND("mumpai",  32,  2),
    SOUND("mumhed",  32,  2),
    SOUND("bstsit",  32,  2),
    SOUND("bstatk",  32,  2),
    SOUND("bstdth",  80,  2),
    SOUND("bstact",  20,  2),
    SOUND("bstpai",  32,  2),
    SOUND("clksit",  32,  2),
    SOUND("clkatk",  32,  2),
    SOUND("clkdth",  80,  2),
    SOUND("clkact",  20,  2),
    SOUND("clkpai",  32,  2),
    SOUND("snksit",  32,  2),
    SOUND("snkatk",  32,  2),
    SOUND("snkdth",  80,  2),
    SOUND("snkact",  20,  2),
    SOUND("snkpai",  32,  2),
    SOUND("kgtsit",  32,  2),
    SOUND("kgtatk",  32,  2),
    SOUND("kgtat2",  32,  2),
    SOUND("kgtdth",  80,  2),
    SOUND_LINK("-kgtact", sfx_kgtsit, 20, 2),
    SOUND("kgtpai",  32,  2),
    SOUND("wizsit",  32,  2),
    SOUND("wizatk",  32,  2),
    SOUND("wizdth",  80,  2),
    SOUND("wizact",  20,  2),
    SOUND("wizpai",  32,  2),
    SOUND("minsit",  32,  2),
    SOUND("minat1",  32,  2),
    SOUND("minat2",  32,  2),
    SOUND("minat3",  32,  2),
    SOUND("mindth",  80,  2),
    SOUND("minact",  20,  2),
    SOUND("minpai",  32,  2),
    SOUND("hedsit",  32,  2),
    SOUND("hedat1",  32,  2),
    SOUND("hedat2",  32,  2),
    SOUND("hedat3",  32,  2),
    SOUND("heddth",  80,  2),
    SOUND("hedact",  20,  2),
    SOUND("hedpai",  32,  2),
    SOUND("sorzap",  32,  2),
    SOUND("sorrise", 32,  2),
    SOUND("sorsit",  200, 2),
    SOUND("soratk",  32,  2),
    SOUND("soract",  200, 2),
    SOUND("sorpai",  200, 2),
    SOUND("sordsph", 200, 2),
    SOUND("sordexp", 200, 2),
    SOUND("sordbon", 200, 2),
    SOUND_LINK("-sbtsit", sfx_bstsit, 32, 2),
    SOUND_LINK("-sbtatk", sfx_bstatk, 32, 2),
    SOUND("sbtdth",  80,  2),
    SOUND("sbtact",  20,  2),
    SOUND("sbtpai",  32,  2),
    SOUND("plroof",  32,  2),
    SOUND("plrpai",  32,  2),
    SOUND("plrdth",  80,  2),
    SOUND("gibdth",  100, 2),
    SOUND("plrwdth", 80,  2),
    SOUND("plrcdth", 100, 2),
    SOUND("itemup",  32,  2),
    SOUND("wpnup",   32,  2),
    SOUND("telept",  50,  2),
    SOUND("doropn",  40,  2),
    SOUND("dorcls",  40,  2),
    SOUND("dormov",  40,  2),
    SOUND("artiup",  32,  2),
    SOUND("switch",  40,  2),
    SOUND("pstart",  40,  2),
    SOUND("pstop",   40,  2),
    SOUND("stnmov",  40,  2),
    SOUND("chicpai", 32,  2),
    SOUND("chicatk", 32,  2),
    SOUND("chicdth", 40,  2),
    SOUND("chicact", 32,  2),
    SOUND("chicpk1", 32,  2),
    SOUND("chicpk2", 32,  2),
    SOUND("chicpk3", 32,  2),
    SOUND("keyup",   50,  2),
    SOUND("ripslop", 16,  2),
    SOUND("newpod",  16,  -1),
    SOUND("podexp",  40,  -1),
    SOUND("bounce",  16,  2),
    SOUND_LINK("-volsht", sfx_bstatk, 16, 2),
    SOUND_LINK("-volhit", sfx_lobhit, 16, 2),
    SOUND("burn",    10,  2),
    SOUND("splash",  10,  1),
    SOUND("gloop",   10,  2),
    SOUND("respawn", 10,  1),
    SOUND("blssht",  32,  2),
    SOUND("blshit",  32,  2),
    SOUND("chat",    100, 1),
    SOUND("artiuse", 32,  1),
    SOUND("gfrag",   100, 1),
    SOUND("waterfl", 16,  2),

    // Monophonic sounds

    SOUND("wind",    16,  1),
    SOUND("amb1",    1,   1),
    SOUND("amb2",    1,   1),
    SOUND("amb3",    1,   1),
    SOUND("amb4",    1,   1),
    SOUND("amb5",    1,   1),
    SOUND("amb6",    1,   1),
    SOUND("amb7",    1,   1),
    SOUND("amb8",    1,   1),
    SOUND("amb9",    1,   1),
    SOUND("amb10",   1,   1),
    SOUND("amb11",   1,   0)
};
