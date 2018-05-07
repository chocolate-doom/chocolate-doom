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
#include "p_local.h" // [crispy] thinkercap
#include "s_sound.h"
#include "r_defs.h" // [crispy] laserpatch
#include "r_sky.h" // [crispy] R_InitSkyMap()

#include "m_crispy.h"

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
    {-1, "none"},
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
    {0, "none"},
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

multiitem_t multiitem_translucency[NUM_TRANSLUCENCY] =
{
    {TRANSLUCENCY_OFF, "off"},
    {TRANSLUCENCY_MISSILE, "projectiles"},
    {TRANSLUCENCY_ITEM, "items"},
    {TRANSLUCENCY_BOTH, "both"},
};

multiitem_t multiitem_sndchannels[NUM_SNDCHANNELS] =
{
    {SNDCHANNELS_8, "8"},
    {SNDCHANNELS_16, "16"},
    {SNDCHANNELS_32, "32"},
};

multiitem_t multiitem_uncapped[NUM_UNCAPPED] =
{
    {UNCAPPED_OFF, "35 fps"},
    {UNCAPPED_ON, "uncapped"},
    {UNCAPPED_VSYNC, "vsync"},
};

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
    thinker_t *th;

    if (gameversion == exe_chex)
    {
	S_StartSound(NULL,sfx_oof);
	return;
    }

    choice = 0;
    crispy->coloredblood = !crispy->coloredblood;

    // [crispy] switch NOBLOOD flag for Lost Souls
    for (th = thinkercap.next; th && th != &thinkercap; th = th->next)
    {
	if (th->function.acp1 == (actionf_p1)P_MobjThinker)
	{
		mobj_t *mobj = (mobj_t *)th;

		if (mobj->type == MT_SKULL)
		{
			if (crispy->coloredblood)
			{
				mobj->flags |= MF_NOBLOOD;
			}
			else
			{
				mobj->flags &= ~MF_NOBLOOD;
			}
		}
	}
    }
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

void M_CrispyToggleCrosshairHealth(int choice)
{
    choice = 0;
    crispy->crosshairhealth = !crispy->crosshairhealth;
}

void M_CrispyToggleCrosshairTarget(int choice)
{
    choice = 0;
    crispy->crosshairtarget = !crispy->crosshairtarget;
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

void M_CrispyToggleHires(int choice)
{
    choice = 0;
    crispy->hires = !crispy->hires;
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
    crispy->neghealth = !crispy->neghealth;
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

void M_CrispyToggleSmoothLighting(int choice)
{
    extern void R_InitLightTables (void);
    extern void R_ExecuteSetViewSize (void);
    extern void P_SegLengths (boolean contrast_only);

    choice = 0;
    crispy->smoothlight = !crispy->smoothlight;

    // [crispy] stop rendering for a while ...
    nodrawers = true;
    // [crispy] re-calculate the zlight[][] array
    R_InitLightTables();
    // [crispy] re-calculate the scalelight[][] array
    R_ExecuteSetViewSize();
    // [crispy] re-calculate fake contrast
    P_SegLengths(true);
    // [crispy] ... continue rendering
    nodrawers = false;
}

void M_CrispyToggleSndChannels(int choice)
{
    extern void S_UpdateSndChannels (void);

    choice = 0;
    crispy->sndchannels = (crispy->sndchannels + 1) % NUM_SNDCHANNELS;

    S_UpdateSndChannels();
}

void M_CrispyToggleSoundfixes(int choice)
{
    choice = 0;
    crispy->soundfix = !crispy->soundfix;
}

void M_CrispyToggleSoundMono(int choice)
{
    choice = 0;
    crispy->soundmono = !crispy->soundmono;
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
