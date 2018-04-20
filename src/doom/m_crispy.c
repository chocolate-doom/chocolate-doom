//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2005-2014 Simon Howard
// Copyright(C) 2015-2018 Fabian Greffrath
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
//	[crispy] Crispness menu
//

#include "doomstat.h"
#include "s_sound.h"
#include "r_defs.h" // [crispy] laserpatch
#include "r_sky.h" // [crispy] R_InitSkyMap()

#include "m_crispy.h"

void M_CrispyToggleAutomapstats(int choice)
{
    choice = 0;
    crispy->automapstats = !crispy->automapstats;
}

void M_CrispyToggleBrightmaps(int choice)
{
    choice = 0;
    crispy->brightmaps = (crispy->brightmaps + 1) % NUM_BRIGHTMAPS;
}

void M_CrispyToggleCenterweapon(int choice)
{
    choice = 0;
    crispy->centerweapon = (crispy->centerweapon + 1) % NUM_CENTERWEAPON;
}

void M_CrispyToggleColoredblood(int choice)
{
    // [crispy] preserve coloredblood_fix value when switching colored blood and corpses
    const int coloredblood_fix = crispy->coloredblood & COLOREDBLOOD_FIX;

    if (gameversion == exe_chex)
    {
	S_StartSound(NULL,sfx_oof);
	return;
    }

    choice = 0;
    crispy->coloredblood = (crispy->coloredblood + 1) % NUM_COLOREDBLOOD;
    crispy->coloredblood |= coloredblood_fix;
}

void M_CrispyToggleColoredblood2(int choice)
{
    if (gameversion == exe_chex)
    {
	S_StartSound(NULL,sfx_oof);
	return;
    }

    choice = 0;
    crispy->coloredblood ^= COLOREDBLOOD_FIX;
}

void M_CrispyToggleColoredhud(int choice)
{
    choice = 0;
    crispy->coloredhud = (crispy->coloredhud + 1) % NUM_COLOREDHUD;
}

void M_CrispyToggleCrosshair(int choice)
{
    choice = 0;
    crispy->crosshair = (crispy->crosshair + 1) % NUM_CROSSHAIRS;
}

void M_CrispyToggleCrosshairtype(int choice)
{
    if (!crispy->crosshair)
    {
	S_StartSound(NULL,sfx_oof);
	return;
    }

    choice = 0;
    crispy->crosshairtype = crispy->crosshairtype + 1;

    if (!laserpatch[crispy->crosshairtype].c)
    {
	crispy->crosshairtype = 0;
    }
}

void M_CrispyToggleDemoBar(int choice)
{
    choice = 0;
    crispy->demobar = !crispy->demobar;
}

void M_CrispyToggleDemoTimer(int choice)
{
    choice = 0;
    crispy->demotimer = (crispy->demotimer + 1) % NUM_DEMOTIMERS;
}

void M_CrispyToggleDemoTimerDir(int choice)
{
    if (!(crispy->demotimer & DEMOTIMER_PLAYBACK))
    {
	S_StartSound(NULL,sfx_oof);
	return;
    }

    choice = 0;
    crispy->demotimerdir = !crispy->demotimerdir;
}

void M_CrispyToggleExtAutomap(int choice)
{
    choice = 0;
    crispy->extautomap = !crispy->extautomap;
}

void M_CrispyToggleExtsaveg(int choice)
{
    choice = 0;
    crispy->extsaveg = !crispy->extsaveg;
}

void M_CrispyToggleFlipcorpses(int choice)
{
    if (gameversion == exe_chex)
    {
	S_StartSound(NULL,sfx_oof);
	return;
    }

    choice = 0;
    crispy->flipcorpses = !crispy->flipcorpses;
}

void M_CrispyToggleFreeaim(int choice)
{
    if (!crispy->singleplayer)
    {
	S_StartSound(NULL,sfx_oof);
	return;
    }

    choice = 0;
    crispy->freeaim = (crispy->freeaim + 1) % NUM_FREEAIMS;

    // [crispy] update the "critical" struct
    CheckCrispySingleplayer(!demorecording && !demoplayback && !netgame);
}

void M_CrispyToggleFreelook(int choice)
{
    choice = 0;
    crispy->freelook = (crispy->freelook + 1) % NUM_FREELOOKS;

    players[consoleplayer].lookdir = 0;
    R_InitSkyMap();
}

void M_CrispyToggleFullsounds(int choice)
{
    choice = 0;
    crispy->soundfull = !crispy->soundfull;
}

void M_CrispyToggleJumping(int choice)
{
    if (!crispy->singleplayer)
    {
	S_StartSound(NULL,sfx_oof);
	return;
    }

    choice = 0;
    crispy->jump = (crispy->jump + 1) % NUM_JUMPS;

    // [crispy] update the "critical" struct
    CheckCrispySingleplayer(!demorecording && !demoplayback && !netgame);
}

void M_CrispyToggleMouseLook(int choice)
{
    choice = 0;
    crispy->mouselook = !crispy->mouselook;

    players[consoleplayer].lookdir = 0;
    R_InitSkyMap();
}

void M_CrispyToggleNeghealth(int choice)
{
    choice = 0;
    crispy->neghealth = (crispy->neghealth + 1) % NUM_NEGHEALTHS;
}

void M_CrispyToggleOverunder(int choice)
{
    if (!crispy->singleplayer)
    {
	S_StartSound(NULL,sfx_oof);
	return;
    }

    choice = 0;
    crispy->overunder = !crispy->overunder;

    // [crispy] update the "critical" struct
    CheckCrispySingleplayer(!demorecording && !demoplayback && !netgame);
}

void M_CrispyTogglePitch(int choice)
{
    choice = 0;
    crispy->pitch = !crispy->pitch;
    R_InitSkyMap();
}

void M_CrispyToggleRecoil(int choice)
{
    if (!crispy->singleplayer)
    {
	S_StartSound(NULL,sfx_oof);
	return;
    }

    choice = 0;
    crispy->recoil = !crispy->recoil;

    // [crispy] update the "critical" struct
    CheckCrispySingleplayer(!demorecording && !demoplayback && !netgame);
}

void M_CrispyToggleSecretmessage(int choice)
{
    choice = 0;
    crispy->secretmessage = !crispy->secretmessage;
}

void M_CrispyToggleSmoothScaling(int choice)
{
    choice = 0;
    crispy->smoothscaling = !crispy->smoothscaling;
}

void M_CrispyToggleSoundfixes(int choice)
{
    choice = 0;
    crispy->soundfix = !crispy->soundfix;
}

void M_CrispyToggleTranslucency(int choice)
{
    choice = 0;
    crispy->translucency = (crispy->translucency + 1) % NUM_TRANSLUCENCY;
}

void M_CrispyToggleUncapped(int choice)
{
    choice = 0;

    if (force_software_renderer)
    {
	S_StartSound(NULL,sfx_oof);
	return;
    }

    crispy->uncapped = (crispy->uncapped + 1) % NUM_UNCAPPED;

    // [crispy] restart renderer if vsync is toggled (UNCAPPED_OFF has vsync),
    // i.e. UNCAPPED_OFF -> UNCAPPED_ON and UNCAPPED_ON -> UNCAPPED_VSYNC
    if (crispy->uncapped)
    {
	extern void SetVideoMode (void);
	SetVideoMode();
    }
}

void M_CrispyToggleWeaponSquat(int choice)
{
    choice = 0;
    crispy->weaponsquat = !crispy->weaponsquat;
}
