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
//	Created by a sound utility.
//	Kept as a sample, DOOM2 sounds.
//


#include <stdlib.h>


#include "doomtype.h"
#include "sounds.h"

//
// Information about all the music
//

#define MUSIC(name) \
    { name, 0, NULL, NULL }

// villsa [STRIFE]
musicinfo_t S_music[] =
{
    MUSIC(NULL),
    MUSIC("logo"),
    MUSIC("action"),
    MUSIC("tavern"),
    MUSIC("danger"),
    MUSIC("fast"),
    MUSIC("intro"),
    MUSIC("darker"),
    MUSIC("strike"),
    MUSIC("slide"),
    MUSIC("tribal"),
    MUSIC("march"),
    MUSIC("danger"),
    MUSIC("mood"),
    MUSIC("castle"),
    MUSIC("darker"),
    MUSIC("action"),
    MUSIC("fight"),
    MUSIC("spense"),
    MUSIC("slide"),
    MUSIC("strike"),
    MUSIC("dark"),
    MUSIC("tech"),
    MUSIC("slide"),
    MUSIC("drone"),
    MUSIC("panthr"),
    MUSIC("sad"),
    MUSIC("instry"),
    MUSIC("tech"),
    MUSIC("action"),
    MUSIC("instry"),
    MUSIC("drone"),
    MUSIC("fight"),
    MUSIC("happy"),
    MUSIC("end")


};


//
// Information about all the sfx
//

#define SOUND(name, priority) \
  { NULL, name, priority, NULL, -1, -1, 0, 0, -1, NULL }
#define SOUND_LINK(name, priority, link_id, pitch, volume) \
  { NULL, name, priority, &S_sfx[link_id], pitch, volume, 0, 0, -1, NULL }

// villsa [STRIFE]
sfxinfo_t S_sfx[] =
{
  // S_sfx[0] needs to be a dummy for odd reasons.
  SOUND("none",     0),
  SOUND("swish",     64),
  SOUND("meatht",    64),
  SOUND("mtalht",    64),
  SOUND("wpnup",     78),
  SOUND("rifle",     64),
  SOUND("mislht",    64),
  SOUND("barexp",    32),
  SOUND("flburn",    64),
  SOUND("flidl",     118),
  SOUND("agrsee",    98),
  SOUND("plpain",    96),
  SOUND("pcrush",    96),
  SOUND("pespna",    98),
  SOUND("pespnb",    98),
  SOUND("pespnc",    98),
  SOUND("pespnd",    98),
  SOUND("agrdpn",    98),
  SOUND("pldeth",    32),
  SOUND("plxdth",    32),
  SOUND("slop",      78),
  SOUND("rebdth",    98),
  SOUND("agrdth",    98),
  SOUND("lgfire",    211),
  SOUND("smfire",    211),
  SOUND("alarm",     210),
  SOUND("drlmto",    98),
  SOUND("drlmtc",    98),
  SOUND("drsmto",    98),
  SOUND("drsmtc",    98),
  SOUND("drlwud",    98),
  SOUND("drswud",    98),
  SOUND("drston",    98),
  SOUND("bdopn",     98),
  SOUND("bdcls",     98),
  SOUND("swtchn",    78),
  SOUND("swbolt",    98),
  SOUND("swscan",    98),
  SOUND("yeah",      10),
  SOUND("mask",      210),
  SOUND("pstart",    100),
  SOUND("pstop",     100),
  SOUND("itemup",    78),
  SOUND("bglass",    200),
  SOUND("wriver",    201),
  SOUND("wfall",     201),
  SOUND("wdrip",     201),
  SOUND("wsplsh",    95),
  SOUND("rebact",    200),
  SOUND("agrac1",    98),
  SOUND("agrac2",    98),
  SOUND("agrac3",    98),
  SOUND("agrac4",    98),
  SOUND("ambppl",    218),
  SOUND("ambbar",    218),
  SOUND("telept",    32),
  SOUND("ratact",    99),
  SOUND("itmbk",     100),
  SOUND("xbow",      99),
  SOUND("burnme",    95),
  SOUND("oof",       96),
  SOUND("wbrldt",    98),
  SOUND("psdtha",    109),
  SOUND("psdthb",    109),
  SOUND("psdthc",    109),
  SOUND("rb2pn",     96),
  SOUND("rb2dth",    32),
  SOUND("rb2see",    98),
  SOUND("rb2act",    98),
  SOUND("firxpl",    70),
  SOUND("stnmov",    100),
  SOUND("noway",     78),
  SOUND("rlaunc",    64),
  SOUND("rflite",    65),
  SOUND("radio",     60),
  SOUND("pulchn",    98),
  SOUND("swknob",    98),
  SOUND("keycrd",    98),
  SOUND("swston",    98),
  SOUND("sntsee",    98),
  SOUND("sntdth",    98),
  SOUND("sntact",    98),
  SOUND("pgrdat",    64),
  SOUND("pgrsee",    90),
  SOUND("pgrdpn",    96),
  SOUND("pgrdth",    32),
  SOUND("pgract",    120),
  SOUND("proton",    64),
  SOUND("protfl",    64),
  SOUND("plasma",    64),
  SOUND("dsrptr",    30),
  SOUND("reavat",    64),
  SOUND("revbld",    64),
  SOUND("revsee",    90),
  SOUND("reavpn",    96),
  SOUND("revdth",    32),
  SOUND("revact",    120),
  SOUND("spisit",    90),
  SOUND("spdwlk",    65),
  SOUND("spidth",    32),
  SOUND("spdatk",    32),
  SOUND("chant",     218),
  SOUND("static",    32),
  SOUND("chain",     70),
  SOUND("tend",      100),
  SOUND("phoot",     32),
  SOUND("explod",    32),
  SOUND("sigil",     32),
  SOUND("sglhit",    32),
  SOUND("siglup",    32),
  SOUND("prgpn",     96),
  SOUND("progac",    120),
  SOUND("lorpn",     96),
  SOUND("lorsee",    90),
  SOUND("difool",    32),
  SOUND("inqdth",    32),
  SOUND("inqact",    98),
  SOUND("inqsee",    90),
  SOUND("inqjmp",    65),
  SOUND("amaln1",    99),
  SOUND("amaln2",    99),
  SOUND("amaln3",    99),
  SOUND("amaln4",    99),
  SOUND("amaln5",    99),
  SOUND("amaln6",    99),
  SOUND("mnalse",    64),
  SOUND("alnsee",    64),
  SOUND("alnpn",     96),
  SOUND("alnact",    120),
  SOUND("alndth",    32),
  SOUND("mnaldt",    32),
  SOUND("reactr",    31),
  SOUND("airlck",    98),
  SOUND("drchno",    98),
  SOUND("drchnc",    98),
  SOUND("valve",     98)
};

