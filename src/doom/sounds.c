// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2005 Simon Howard
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
// DESCRIPTION:
//	Created by a sound utility.
//	Kept as a sample, DOOM2 sounds.
//
//-----------------------------------------------------------------------------


#include <stdlib.h>


#include "doomtype.h"
#include "sounds.h"

//
// Information about all the music
//

musicinfo_t S_music[] =
{
    { NULL, 0, 0, 0 },
    { "e1m1", 0, 0, 0 },
    { "e1m2", 0, 0, 0 },
    { "e1m3", 0, 0, 0 },
    { "e1m4", 0, 0, 0 },
    { "e1m5", 0, 0, 0 },
    { "e1m6", 0, 0, 0 },
    { "e1m7", 0, 0, 0 },
    { "e1m8", 0, 0, 0 },
    { "e1m9", 0, 0, 0 },
    { "e2m1", 0, 0, 0 },
    { "e2m2", 0, 0, 0 },
    { "e2m3", 0, 0, 0 },
    { "e2m4", 0, 0, 0 },
    { "e2m5", 0, 0, 0 },
    { "e2m6", 0, 0, 0 },
    { "e2m7", 0, 0, 0 },
    { "e2m8", 0, 0, 0 },
    { "e2m9", 0, 0, 0 },
    { "e3m1", 0, 0, 0 },
    { "e3m2", 0, 0, 0 },
    { "e3m3", 0, 0, 0 },
    { "e3m4", 0, 0, 0 },
    { "e3m5", 0, 0, 0 },
    { "e3m6", 0, 0, 0 },
    { "e3m7", 0, 0, 0 },
    { "e3m8", 0, 0, 0 },
    { "e3m9", 0, 0, 0 },
    { "inter", 0, 0, 0 },
    { "intro", 0, 0, 0 },
    { "bunny", 0, 0, 0 },
    { "victor", 0, 0, 0 },
    { "introa", 0, 0, 0 },
    { "runnin", 0, 0, 0 },
    { "stalks", 0, 0, 0 },
    { "countd", 0, 0, 0 },
    { "betwee", 0, 0, 0 },
    { "doom", 0, 0, 0 },
    { "the_da", 0, 0, 0 },
    { "shawn", 0, 0, 0 },
    { "ddtblu", 0, 0, 0 },
    { "in_cit", 0, 0, 0 },
    { "dead", 0, 0, 0 },
    { "stlks2", 0, 0, 0 },
    { "theda2", 0, 0, 0 },
    { "doom2", 0, 0, 0 },
    { "ddtbl2", 0, 0, 0 },
    { "runni2", 0, 0, 0 },
    { "dead2", 0, 0, 0 },
    { "stlks3", 0, 0, 0 },
    { "romero", 0, 0, 0 },
    { "shawn2", 0, 0, 0 },
    { "messag", 0, 0, 0 },
    { "count2", 0, 0, 0 },
    { "ddtbl3", 0, 0, 0 },
    { "ampie", 0, 0, 0 },
    { "theda3", 0, 0, 0 },
    { "adrian", 0, 0, 0 },
    { "messg2", 0, 0, 0 },
    { "romer2", 0, 0, 0 },
    { "tense", 0, 0, 0 },
    { "shawn3", 0, 0, 0 },
    { "openin", 0, 0, 0 },
    { "evil", 0, 0, 0 },
    { "ultima", 0, 0, 0 },
    { "read_m", 0, 0, 0 },
    { "dm2ttl", 0, 0, 0 },
    { "dm2int", 0, 0, 0 } 
};


//
// Information about all the sfx
//

sfxinfo_t S_sfx[] =
{
  // S_sfx[0] needs to be a dummy for odd reasons.
  { "none", 0, 0, -1, -1, 0, 0, 0, NULL },
  { "pistol", 64, 0, -1, -1, 0, 0, 0, NULL },
  { "shotgn", 64, 0, -1, -1, 0, 0, 0, NULL },
  { "sgcock", 64, 0, -1, -1, 0, 0, 0, NULL },
  { "dshtgn", 64, 0, -1, -1, 0, 0, 0, NULL },
  { "dbopn", 64, 0, -1, -1, 0, 0, 0, NULL },
  { "dbcls", 64, 0, -1, -1, 0, 0, 0, NULL },
  { "dbload", 64, 0, -1, -1, 0, 0, 0, NULL },
  { "plasma", 64, 0, -1, -1, 0, 0, 0, NULL },
  { "bfg", 64, 0, -1, -1, 0, 0, 0, NULL },
  { "sawup", 64, 0, -1, -1, 0, 0, 0, NULL },
  { "sawidl", 118, 0, -1, -1, 0, 0, 0, NULL },
  { "sawful", 64, 0, -1, -1, 0, 0, 0, NULL },
  { "sawhit", 64, 0, -1, -1, 0, 0, 0, NULL },
  { "rlaunc", 64, 0, -1, -1, 0, 0, 0, NULL },
  { "rxplod", 70, 0, -1, -1, 0, 0, 0, NULL },
  { "firsht", 70, 0, -1, -1, 0, 0, 0, NULL },
  { "firxpl", 70, 0, -1, -1, 0, 0, 0, NULL },
  { "pstart", 100, 0, -1, -1, 0, 0, 0, NULL },
  { "pstop", 100, 0, -1, -1, 0, 0, 0, NULL },
  { "doropn", 100, 0, -1, -1, 0, 0, 0, NULL },
  { "dorcls", 100, 0, -1, -1, 0, 0, 0, NULL },
  { "stnmov", 119, 0, -1, -1, 0, 0, 0, NULL },
  { "swtchn", 78, 0, -1, -1, 0, 0, 0, NULL },
  { "swtchx", 78, 0, -1, -1, 0, 0, 0, NULL },
  { "plpain", 96, 0, -1, -1, 0, 0, 0, NULL },
  { "dmpain", 96, 0, -1, -1, 0, 0, 0, NULL },
  { "popain", 96, 0, -1, -1, 0, 0, 0, NULL },
  { "vipain", 96, 0, -1, -1, 0, 0, 0, NULL },
  { "mnpain", 96, 0, -1, -1, 0, 0, 0, NULL },
  { "pepain", 96, 0, -1, -1, 0, 0, 0, NULL },
  { "slop", 78, 0, -1, -1, 0, 0, 0, NULL },
  { "itemup", 78, 0, -1, -1, 0, 0, 0, NULL },
  { "wpnup", 78, 0, -1, -1, 0, 0, 0, NULL },
  { "oof", 96, 0, -1, -1, 0, 0, 0, NULL },
  { "telept", 32, 0, -1, -1, 0, 0, 0, NULL },
  { "posit1", 98, 0, -1, -1, 0, 0, 0, NULL },
  { "posit2", 98, 0, -1, -1, 0, 0, 0, NULL },
  { "posit3", 98, 0, -1, -1, 0, 0, 0, NULL },
  { "bgsit1", 98, 0, -1, -1, 0, 0, 0, NULL },
  { "bgsit2", 98, 0, -1, -1, 0, 0, 0, NULL },
  { "sgtsit", 98, 0, -1, -1, 0, 0, 0, NULL },
  { "cacsit", 98, 0, -1, -1, 0, 0, 0, NULL },
  { "brssit", 94, 0, -1, -1, 0, 0, 0, NULL },
  { "cybsit", 92, 0, -1, -1, 0, 0, 0, NULL },
  { "spisit", 90, 0, -1, -1, 0, 0, 0, NULL },
  { "bspsit", 90, 0, -1, -1, 0, 0, 0, NULL },
  { "kntsit", 90, 0, -1, -1, 0, 0, 0, NULL },
  { "vilsit", 90, 0, -1, -1, 0, 0, 0, NULL },
  { "mansit", 90, 0, -1, -1, 0, 0, 0, NULL },
  { "pesit", 90, 0, -1, -1, 0, 0, 0, NULL },
  { "sklatk", 70, 0, -1, -1, 0, 0, 0, NULL },
  { "sgtatk", 70, 0, -1, -1, 0, 0, 0, NULL },
  { "skepch", 70, 0, -1, -1, 0, 0, 0, NULL },
  { "vilatk", 70, 0, -1, -1, 0, 0, 0, NULL },
  { "claw", 70, 0, -1, -1, 0, 0, 0, NULL },
  { "skeswg", 70, 0, -1, -1, 0, 0, 0, NULL },
  { "pldeth", 32, 0, -1, -1, 0, 0, 0, NULL },
  { "pdiehi", 32, 0, -1, -1, 0, 0, 0, NULL },
  { "podth1", 70, 0, -1, -1, 0, 0, 0, NULL },
  { "podth2", 70, 0, -1, -1, 0, 0, 0, NULL },
  { "podth3", 70, 0, -1, -1, 0, 0, 0, NULL },
  { "bgdth1", 70, 0, -1, -1, 0, 0, 0, NULL },
  { "bgdth2", 70, 0, -1, -1, 0, 0, 0, NULL },
  { "sgtdth", 70, 0, -1, -1, 0, 0, 0, NULL },
  { "cacdth", 70, 0, -1, -1, 0, 0, 0, NULL },
  { "skldth", 70, 0, -1, -1, 0, 0, 0, NULL },
  { "brsdth", 32, 0, -1, -1, 0, 0, 0, NULL },
  { "cybdth", 32, 0, -1, -1, 0, 0, 0, NULL },
  { "spidth", 32, 0, -1, -1, 0, 0, 0, NULL },
  { "bspdth", 32, 0, -1, -1, 0, 0, 0, NULL },
  { "vildth", 32, 0, -1, -1, 0, 0, 0, NULL },
  { "kntdth", 32, 0, -1, -1, 0, 0, 0, NULL },
  { "pedth", 32, 0, -1, -1, 0, 0, 0, NULL },
  { "skedth", 32, 0, -1, -1, 0, 0, 0, NULL },
  { "posact", 120, 0, -1, -1, 0, 0, 0, NULL },
  { "bgact", 120, 0, -1, -1, 0, 0, 0, NULL },
  { "dmact", 120, 0, -1, -1, 0, 0, 0, NULL },
  { "bspact", 100, 0, -1, -1, 0, 0, 0, NULL },
  { "bspwlk", 100, 0, -1, -1, 0, 0, 0, NULL },
  { "vilact", 100, 0, -1, -1, 0, 0, 0, NULL },
  { "noway", 78, 0, -1, -1, 0, 0, 0, NULL },
  { "barexp", 60, 0, -1, -1, 0, 0, 0, NULL },
  { "punch", 64, 0, -1, -1, 0, 0, 0, NULL },
  { "hoof", 70, 0, -1, -1, 0, 0, 0, NULL },
  { "metal", 70, 0, -1, -1, 0, 0, 0, NULL },
  { "chgun", 64, &S_sfx[sfx_pistol], 150, 0, 0, 0, 0, NULL },
  { "tink", 60, 0, -1, -1, 0, 0, 0, NULL },
  { "bdopn", 100, 0, -1, -1, 0, 0, 0, NULL },
  { "bdcls", 100, 0, -1, -1, 0, 0, 0, NULL },
  { "itmbk", 100, 0, -1, -1, 0, 0, 0, NULL },
  { "flame", 32, 0, -1, -1, 0, 0, 0, NULL },
  { "flamst", 32, 0, -1, -1, 0, 0, 0, NULL },
  { "getpow", 60, 0, -1, -1, 0, 0, 0, NULL },
  { "bospit", 70, 0, -1, -1, 0, 0, 0, NULL },
  { "boscub", 70, 0, -1, -1, 0, 0, 0, NULL },
  { "bossit", 70, 0, -1, -1, 0, 0, 0, NULL },
  { "bospn", 70, 0, -1, -1, 0, 0, 0, NULL },
  { "bosdth", 70, 0, -1, -1, 0, 0, 0, NULL },
  { "manatk", 70, 0, -1, -1, 0, 0, 0, NULL },
  { "mandth", 70, 0, -1, -1, 0, 0, 0, NULL },
  { "sssit", 70, 0, -1, -1, 0, 0, 0, NULL },
  { "ssdth", 70, 0, -1, -1, 0, 0, 0, NULL },
  { "keenpn", 70, 0, -1, -1, 0, 0, 0, NULL },
  { "keendt", 70, 0, -1, -1, 0, 0, 0, NULL },
  { "skeact", 70, 0, -1, -1, 0, 0, 0, NULL },
  { "skesit", 70, 0, -1, -1, 0, 0, 0, NULL },
  { "skeatk", 70, 0, -1, -1, 0, 0, 0, NULL },
  { "radio", 60, 0, -1, -1, 0, 0, 0, NULL } 
};

