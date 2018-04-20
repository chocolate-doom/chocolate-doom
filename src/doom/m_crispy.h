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

#ifndef __M_CRISPY__
#define __M_CRISPY__

typedef struct
{
    int value;
    char *name;
} multiitem_t;

multiitem_t multiitem_brightmaps[NUM_BRIGHTMAPS] =
{
    {BRIGHTMAPS_OFF, "none"},
    {BRIGHTMAPS_TEXTURES, "walls"},
    {BRIGHTMAPS_SPRITES, "items"},
    {BRIGHTMAPS_BOTH, "both"},
};

multiitem_t multiitem_centerweapon[NUM_CENTERWEAPON] =
{
    {CENTERWEAPON_OFF, "off"},
    {CENTERWEAPON_HOR, "horizontal"},
    {CENTERWEAPON_HORVER, "centered"},
    {CENTERWEAPON_BOB, "bobbing"},
    {CENTERWEAPON_BOB2, "bobbing/2"},
};

multiitem_t multiitem_coloredblood[NUM_COLOREDBLOOD] =
{
    {COLOREDBLOOD_OFF, "off"},
    {COLOREDBLOOD_BLOOD, "blood"},
    {COLOREDBLOOD_CORPSE, "corpses"},
    {COLOREDBLOOD_BOTH, "both"},
};

multiitem_t multiitem_coloredhud[NUM_COLOREDHUD] =
{
    {COLOREDHUD_OFF, "off"},
    {COLOREDHUD_BAR, "status bar"},
    {COLOREDHUD_TEXT, "hud texts"},
    {COLOREDHUD_BOTH, "both"},
};

multiitem_t multiitem_crosshair[NUM_CROSSHAIRS] =
{
    {CROSSHAIR_OFF, "off"},
    {CROSSHAIR_STATIC, "static"},
    {CROSSHAIR_PROJECTED, "projected"},
};

multiitem_t multiitem_crosshairtype[] =
{
    {-1, ""},
    {0, "cross"},
    {1, "chevron"},
    {2, "dot"},
};

multiitem_t multiitem_freeaim[NUM_FREEAIMS] =
{
    {FREEAIM_AUTO, "autoaim"},
    {FREEAIM_DIRECT, "direct"},
    {FREEAIM_BOTH, "both"},
};

multiitem_t multiitem_demotimer[NUM_DEMOTIMERS] =
{
    {DEMOTIMER_OFF, "off"},
    {DEMOTIMER_RECORD, "recording"},
    {DEMOTIMER_PLAYBACK, "playback"},
    {DEMOTIMER_BOTH, "both"},
};

multiitem_t multiitem_demotimerdir[] =
{
    {0, ""},
    {1, "forward"},
    {2, "backward"},
};

multiitem_t multiitem_freelook[NUM_FREELOOKS] =
{
    {FREELOOK_OFF, "off"},
    {FREELOOK_SPRING, "spring"},
    {FREELOOK_LOCK, "lock"},
};

multiitem_t multiitem_jump[NUM_JUMPS] =
{
    {JUMP_OFF, "off"},
    {JUMP_LOW, "low"},
    {JUMP_HIGH, "high"},
};

multiitem_t multiitem_neghealth[NUM_NEGHEALTHS] =
{
    {NEGHEALTH_OFF, "off"},
    {NEGHEALTH_DM, "deathmatch"},
    {NEGHEALTH_ON, "always"},
};

multiitem_t multiitem_translucency[NUM_TRANSLUCENCY] =
{
    {TRANSLUCENCY_OFF, "off"},
    {TRANSLUCENCY_MISSILE, "projectiles"},
    {TRANSLUCENCY_ITEM, "items"},
    {TRANSLUCENCY_BOTH, "both"},
};

multiitem_t multiitem_uncapped[NUM_UNCAPPED] =
{
    {UNCAPPED_OFF, "35 fps"},
    {UNCAPPED_ON, "uncapped"},
    {UNCAPPED_VSYNC, "vsync"},
};

extern void M_CrispyToggleAutomapstats(int choice);
extern void M_CrispyToggleBrightmaps(int choice);
extern void M_CrispyToggleCenterweapon(int choice);
extern void M_CrispyToggleColoredblood(int choice);
extern void M_CrispyToggleColoredblood2(int choice);
extern void M_CrispyToggleColoredhud(int choice);
extern void M_CrispyToggleCrosshair(int choice);
extern void M_CrispyToggleCrosshairtype(int choice);
extern void M_CrispyToggleDemoBar(int choice);
extern void M_CrispyToggleDemoTimer(int choice);
extern void M_CrispyToggleDemoTimerDir(int choice);
extern void M_CrispyToggleExtAutomap(int choice);
extern void M_CrispyToggleExtsaveg(int choice);
extern void M_CrispyToggleFlipcorpses(int choice);
extern void M_CrispyToggleFreeaim(int choice);
extern void M_CrispyToggleFreelook(int choice);
extern void M_CrispyToggleFullsounds(int choice);
extern void M_CrispyToggleJumping(int choice);
extern void M_CrispyToggleMouseLook(int choice);
extern void M_CrispyToggleNeghealth(int choice);
extern void M_CrispyToggleOverunder(int choice);
extern void M_CrispyTogglePitch(int choice);
extern void M_CrispyToggleRecoil(int choice);
extern void M_CrispyToggleSecretmessage(int choice);
extern void M_CrispyToggleSmoothScaling(int choice);
extern void M_CrispyToggleSoundfixes(int choice);
extern void M_CrispyToggleTranslucency(int choice);
extern void M_CrispyToggleUncapped(int choice);
extern void M_CrispyToggleWeaponSquat(int choice);

#endif
