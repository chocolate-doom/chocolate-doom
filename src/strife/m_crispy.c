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
//  [crispy] Crispness menu
//

#include "crispy.h"
#include "i_input.h"
#include "m_menu.h"
#include "p_local.h"
#include "r_defs.h"
#include "m_crispy.h"

multiitem_t multiitem_bobfactor[NUM_BOBFACTORS] =
{
    {BOBFACTOR_FULL, "Full"},
    {BOBFACTOR_75, "75%"},
    {BOBFACTOR_OFF, "Off"},
};

multiitem_t multiitem_centerweapon[NUM_CENTERWEAPON] =
{
    {CENTERWEAPON_OFF, "Off"},
    {CENTERWEAPON_CENTER, "Centered"},
    {CENTERWEAPON_BOB, "Bobbing"},
};

multiitem_t multiitem_difficulties[NUM_SKILLS] =
{
    {SKILL_HNTR, "Rookie"},
    {SKILL_HMP, "Veteran"},
    {SKILL_UV, "Elite"},
    {SKILL_NIGHTMARE, "Bloodbath"},
    {SKILL_ITYTD, "Training"},
};

multiitem_t multiitem_widescreen[NUM_RATIOS] =
{
    {RATIO_ORIG, "Original"},
    {RATIO_MATCH_SCREEN, "Match Screen"},
    {RATIO_16_10, "16:10"},
    {RATIO_16_9, "16:9"},
    {RATIO_21_9, "21:9"},
};

extern void AM_LevelInit (boolean reinit);
extern void EnableLoadingDisk (void);
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

void M_CrispyToggleBobfactor(int choice)
{
    ChangeSettingEnum(&crispy->bobfactor, choice, NUM_BOBFACTORS);
}

void M_CrispyToggleCenterweapon(int choice)
{
    ChangeSettingEnum(&crispy->centerweapon, choice, NUM_CENTERWEAPON);
}

void M_CrispyToggleDefaultSkill(int choice)
{
    ChangeSettingEnum(&crispy->defaultskill, choice, NUM_SKILLS);
    M_SetDefaultDifficulty();
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

        if (crispy->fpslimit < CRISPY_FPSLIMIT_MIN)
        {
            crispy->fpslimit = 0;
        }
    }
    else if (choice == 1)
    {
        crispy->fpslimit++;
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

    if (crispy->fpslimit && crispy->fpslimit < CRISPY_FPSLIMIT_MIN)
    {
        crispy->fpslimit = CRISPY_FPSLIMIT_MIN;
    }
    else if (crispy->fpslimit > CRISPY_FPSLIMIT_MAX)
    {
        crispy->fpslimit = CRISPY_FPSLIMIT_MAX;
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

void M_CrispyToggleSmoothScaling(int choice)
{
    choice = 0;

    crispy->smoothscaling = !crispy->smoothscaling;
}

void M_CrispyToggleUncapped(int choice)
{
    choice = 0;

    crispy->uncapped = !crispy->uncapped;
}

void M_CrispyToggleVsyncHook (void)
{
    crispy->vsync = !crispy->vsync;

    I_ReInitGraphics(REINIT_RENDERER | REINIT_TEXTURES | REINIT_ASPECTRATIO);
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

void M_CrispyToggleWidescreen(int choice)
{
    hookchoice = choice;

    crispy->post_rendering_hook = M_CrispyToggleWidescreenHook;
}
