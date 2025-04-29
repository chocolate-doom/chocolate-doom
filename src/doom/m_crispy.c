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

#include "crispy.h"
#include "doomstat.h"
#include "i_input.h" // [crispy] start/stop text input
#include "m_menu.h" // [crispy] M_SetDefaultDifficulty()
#include "p_local.h" // [crispy] thinkercap
#include "s_sound.h"
#include "r_defs.h" // [crispy] laserpatch
#include "r_sky.h" // [crispy] R_InitSkyMap()

#include "m_crispy.h"

multiitem_t multiitem_bobfactor[NUM_BOBFACTORS] =
{
    {BOBFACTOR_FULL, "full"},
    {BOBFACTOR_75, "75%"},
    {BOBFACTOR_OFF, "off"},
};

multiitem_t multiitem_brightmaps[NUM_BRIGHTMAPS] =
{
    {BRIGHTMAPS_OFF, "none"},
    {BRIGHTMAPS_TEXTURES, "walls"},
    {BRIGHTMAPS_SPRITES, "items"},
    {BRIGHTMAPS_BOTH, "both"},
};

multiitem_t multiitem_coloredblood[NUM_COLOREDBLOOD] =
{
    {COLOREDBLOOD_OFF, "off"},
    {COLOREDBLOOD_BLOOD, "blood"},
    {COLOREDBLOOD_ALL, "all"}
};

multiitem_t multiitem_centerweapon[NUM_CENTERWEAPON] =
{
    {CENTERWEAPON_OFF, "off"},
    {CENTERWEAPON_CENTER, "centered"},
    {CENTERWEAPON_BOB, "bobbing"},
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

multiitem_t multiitem_secretmessage[NUM_SECRETMESSAGE] =
{
    {SECRETMESSAGE_OFF, "off"},
    {SECRETMESSAGE_ON, "on"},
    {SECRETMESSAGE_COUNT, "count"},
};

multiitem_t multiitem_difficulties[NUM_SKILLS] =
{
    {SKILL_HMP, "HMP"},
    {SKILL_UV, "UV"},
    {SKILL_NIGHTMARE, "NIGHTMARE"},
    {SKILL_ITYTD, "ITYTD"},
    {SKILL_HNTR, "HNTR"},
};

multiitem_t multiitem_statsformat[NUM_STATSFORMATS] =
{
    {STATSFORMAT_RATIO, "ratio"},
    {STATSFORMAT_REMAINING, "remaining"},
    {STATSFORMAT_PERCENT, "percent"},
    {STATSFORMAT_BOOLEAN, "boolean"},
};

multiitem_t multiitem_translucency[NUM_TRANSLUCENCY] =
{
    {TRANSLUCENCY_OFF, "off"},
    {TRANSLUCENCY_MISSILE, "projectiles"},
    {TRANSLUCENCY_ITEM, "items"},
    {TRANSLUCENCY_BOTH, "both"},
};

multiitem_t multiitem_sndchannels[4] =
{
    {8, "8"},
    {16, "16"},
    {32, "32"},
};

multiitem_t multiitem_widgets[NUM_WIDGETS] =
{
    {WIDGETS_OFF, "never"},
    {WIDGETS_AUTOMAP, "in Automap"},
    {WIDGETS_ALWAYS, "always"},
    {WIDGETS_STBAR, "status bar"},
};

multiitem_t multiitem_widescreen[NUM_RATIOS] =
{
    {RATIO_ORIG, "Original"},
    {RATIO_MATCH_SCREEN, "Match screen"},
    {RATIO_16_10, "16:10"},
    {RATIO_16_9, "16:9"},
    {RATIO_21_9, "21:9"},
};

extern void AM_LevelInit (boolean reinit);
extern void EnableLoadingDisk (void);
extern void P_SegLengths (boolean contrast_only);
extern void R_InitLightTables (void);
extern void I_ReInitGraphics (int reinit);

static void ChangeSettingEnum(int *setting, int choice, int num_values)
{
    if (choice == 1)
    {
        *setting += 1;
    }
    else
    {
        *setting += num_values - 1;
    }

    *setting %= num_values;
}

void M_CrispyToggleAutomapstats(int choice)
{
    ChangeSettingEnum(&crispy->automapstats, choice, NUM_WIDGETS);
}

void M_CrispyToggleBobfactor(int choice)
{
    ChangeSettingEnum(&crispy->bobfactor, choice, NUM_BOBFACTORS);
}

void M_CrispyToggleBrightmaps(int choice)
{
    ChangeSettingEnum(&crispy->brightmaps, choice, NUM_BRIGHTMAPS);
}

void M_CrispyToggleCenterweapon(int choice)
{
    ChangeSettingEnum(&crispy->centerweapon, choice, NUM_CENTERWEAPON);
}

void M_CrispyToggleColoredblood(int choice)
{
    thinker_t *th;

    if (gameversion == exe_chex)
    {
	return;
    }

    ChangeSettingEnum(&crispy->coloredblood, choice, NUM_COLOREDBLOOD);

    // [crispy] switch NOBLOOD flag for Lost Souls
    for (th = thinkercap.next; th && th != &thinkercap; th = th->next)
    {
	if (th->function.acp1 == (actionf_p1)P_MobjThinker)
	{
		mobj_t *mobj = (mobj_t *)th;

		if (mobj->type == MT_SKULL)
		{
			if (crispy->coloredblood == COLOREDBLOOD_ALL)
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
    ChangeSettingEnum(&crispy->coloredhud, choice, NUM_COLOREDHUD);
}

void M_CrispyToggleCrosshair(int choice)
{
    ChangeSettingEnum(&crispy->crosshair, choice, NUM_CROSSHAIRS);
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
	return;
    }

    ChangeSettingEnum(&crispy->crosshairtype, choice, NUM_CROSSHAIRTYPES);
}

void M_CrispyToggleDemoBar(int choice)
{
    choice = 0;
    crispy->demobar = !crispy->demobar;
}

void M_CrispyToggleDemoTimer(int choice)
{
    ChangeSettingEnum(&crispy->demotimer, choice, NUM_DEMOTIMERS);
}

void M_CrispyToggleDemoTimerDir(int choice)
{
    if (!(crispy->demotimer & DEMOTIMER_PLAYBACK))
    {
	return;
    }

    choice = 0;
    crispy->demotimerdir = !crispy->demotimerdir;
}

void M_CrispyToggleDemoUseTimer(int choice)
{
    choice = 0;
    crispy->btusetimer = !crispy->btusetimer;
}

void M_CrispyToggleExtAutomap(int choice)
{
    choice = 0;
    crispy->extautomap = !crispy->extautomap;
}

void M_CrispyToggleFlipcorpses(int choice)
{
    if (gameversion == exe_chex)
    {
	return;
    }

    choice = 0;
    crispy->flipcorpses = !crispy->flipcorpses;
}

void M_CrispyToggleFreeaim(int choice)
{
    if (!crispy->singleplayer)
    {
	return;
    }

    ChangeSettingEnum(&crispy->freeaim, choice, NUM_FREEAIMS);

    // [crispy] update the "critical" struct
    CheckCrispySingleplayer(!demorecording && !demoplayback && !netgame);
}

static void M_CrispyToggleSkyHook (void)
{
    players[consoleplayer].lookdir = 0;
    R_InitSkyMap();
}

void M_CrispyToggleFpsLimit(int choice)
{
    if (!crispy->uncapped)
    {
        return;
    }

    if (choice == 0)
    {
        crispy->fpslimit--;
    }
    else if (choice == 1)
    {
        if (crispy->fpslimit < TICRATE)
        {
            crispy->fpslimit = TICRATE;
        }
        else
        {
            crispy->fpslimit++;
        }
    }
    else if (choice == 2)
    {
        if (numeric_enter)
        {
            crispy->fpslimit = numeric_entry;
            numeric_enter = false;
            I_StopTextInput();
        }
        else
        {
            numeric_enter = true;
            I_StartTextInput(0, 0, 0, 0);
            return;
        }
    }

    if (crispy->fpslimit < TICRATE)
    {
        crispy->fpslimit = 0;
    }
    else if (crispy->fpslimit > CRISPY_FPSLIMIT_MAX)
    {
        crispy->fpslimit = CRISPY_FPSLIMIT_MAX;
    }
}

void M_CrispyToggleFreelook(int choice)
{
    ChangeSettingEnum(&crispy->freelook, choice, NUM_FREELOOKS);
    crispy->post_rendering_hook = M_CrispyToggleSkyHook;
}

void M_CrispyToggleFullsounds(int choice)
{
    int i;

    choice = 0;
    crispy->soundfull = !crispy->soundfull;

    // [crispy] weapon sound sources
    for (i = 0; i < MAXPLAYERS; i++)
    {
	if (playeringame[i])
	{
	    players[i].so = Crispy_PlayerSO(i);
	}
    }
}

static void M_CrispyToggleHiresHook (void)
{
    crispy->hires = !crispy->hires;

    // [crispy] re-initialize framebuffers, textures and renderer
    I_ReInitGraphics(REINIT_FRAMEBUFFERS | REINIT_TEXTURES | REINIT_ASPECTRATIO);
    // [crispy] re-calculate framebuffer coordinates
    R_ExecuteSetViewSize();
    // [crispy] re-draw bezel
    R_FillBackScreen();
    // [crispy] re-calculate disk icon coordinates
    EnableLoadingDisk();
    // [crispy] re-calculate automap coordinates
    AM_LevelInit(true);
}

void M_CrispyToggleHires(int choice)
{
    choice = 0;

    crispy->post_rendering_hook = M_CrispyToggleHiresHook;
}

void M_CrispyToggleJumping(int choice)
{
    if (!crispy->singleplayer)
    {
	return;
    }

    ChangeSettingEnum(&crispy->jump, choice, NUM_JUMPS);

    // [crispy] update the "critical" struct
    CheckCrispySingleplayer(!demorecording && !demoplayback && !netgame);
}

void M_CrispyToggleLeveltime(int choice)
{
    ChangeSettingEnum(&crispy->leveltime, choice, NUM_WIDGETS - 1);
}

void M_CrispyToggleMouseLook(int choice)
{
    choice = 0;
    crispy->mouselook = !crispy->mouselook;

    crispy->post_rendering_hook = M_CrispyToggleSkyHook;
}

void M_CrispyToggleNeghealth(int choice)
{
    choice = 0;
    crispy->neghealth = !crispy->neghealth;
}

void M_CrispyToggleDefaultSkill(int choice)
{
    ChangeSettingEnum(&crispy->defaultskill, choice, NUM_SKILLS);
    M_SetDefaultDifficulty();
}

void M_CrispyToggleOverunder(int choice)
{
    if (!crispy->singleplayer)
    {
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

    crispy->post_rendering_hook = M_CrispyToggleSkyHook;
}

void M_CrispyTogglePlayerCoords(int choice)
{
    // [crispy] disable "always" setting
    ChangeSettingEnum(&crispy->playercoords, choice, NUM_WIDGETS - 2);
}

void M_CrispyToggleSecretmessage(int choice)
{
    ChangeSettingEnum(&crispy->secretmessage, choice, NUM_SECRETMESSAGE);
}

void M_CrispyToggleSmoothScaling(int choice)
{
    choice = 0;
    smooth_pixel_scaling = !smooth_pixel_scaling;
}

static void M_CrispyToggleSmoothLightingHook (void)
{
    crispy->smoothlight = !crispy->smoothlight;

#ifdef CRISPY_TRUECOLOR
    // [crispy] re-calculate amount of colormaps and light tables
    R_InitColormaps();
#endif
    // [crispy] re-calculate the zlight[][] array
    R_InitLightTables();
    // [crispy] re-calculate the scalelight[][] array
    R_ExecuteSetViewSize();
    // [crispy] re-calculate fake contrast
    P_SegLengths(true);
}

void M_CrispyToggleSmoothLighting(int choice)
{
    choice = 0;

    crispy->post_rendering_hook = M_CrispyToggleSmoothLightingHook;
}

void M_CrispyToggleSmoothMap(int choice)
{
    choice = 0;
    crispy->smoothmap = !crispy->smoothmap;
    // Update function pointer used to draw lines
    AM_LevelInit(true);
}


void M_CrispyToggleSndChannels(int choice)
{
    S_UpdateSndChannels(choice);
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

    S_UpdateStereoSeparation();
}

void M_CrispyToggleStatsFormat(int choice)
{
    ChangeSettingEnum(&crispy->statsformat, choice, NUM_STATSFORMATS);
}

void M_CrispyToggleTranslucency(int choice)
{
    ChangeSettingEnum(&crispy->translucency, choice, NUM_TRANSLUCENCY);
}

void M_CrispyToggleUncapped(int choice)
{
    choice = 0;

    crispy->uncapped = !crispy->uncapped;
}

void M_CrispyToggleVsyncHook (void)
{
    crispy->vsync = !crispy->vsync;

    I_ToggleVsync();
}

void M_CrispyToggleVsync(int choice)
{
    choice = 0;

    if (force_software_renderer)
    {
	return;
    }

    crispy->post_rendering_hook = M_CrispyToggleVsyncHook;
}

static int hookchoice;
static void M_CrispyToggleWidescreenHook (void)
{
    ChangeSettingEnum(&crispy->widescreen, hookchoice, NUM_RATIOS);

    // [crispy] no need to re-init when switching from wide to compact
    {
	// [crispy] re-initialize framebuffers, textures and renderer
	I_ReInitGraphics(REINIT_FRAMEBUFFERS | REINIT_TEXTURES | REINIT_ASPECTRATIO);
	// [crispy] re-calculate framebuffer coordinates
	R_ExecuteSetViewSize();
	// [crispy] re-draw bezel
	R_FillBackScreen();
	// [crispy] re-calculate disk icon coordinates
	EnableLoadingDisk();
	// [crispy] re-calculate automap coordinates
	AM_LevelInit(true);
    }
}

void M_CrispyToggleWidescreen(int choice)
{
    hookchoice = choice;

    crispy->post_rendering_hook = M_CrispyToggleWidescreenHook;
}
