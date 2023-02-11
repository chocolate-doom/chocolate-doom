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

#ifndef __M_CRISPY__
#define __M_CRISPY__

typedef struct
{
    int value;
    const char *name;
} multiitem_t;

extern multiitem_t multiitem_bobfactor[NUM_BOBFACTORS];
extern multiitem_t multiitem_centerweapon[NUM_CENTERWEAPON];
extern multiitem_t multiitem_difficulties[NUM_SKILLS];
extern multiitem_t multiitem_sndchannels[3];
extern multiitem_t multiitem_widescreen[NUM_RATIOS];
extern multiitem_t multiitem_widgets[NUM_WIDGETS];

void M_CrispyToggleBobfactor(int choice);
void M_CrispyToggleCenterweapon(int choice);
void M_CrispyToggleDefaultSkill(int choice);
void M_CrispyToggleFpsLimit(int choice);
void M_CrispyToggleHires(int choice);
void M_CrispyToggleLeveltime(int choice);
void M_CrispyTogglePlayerCoords(int choice);
void M_CrispyToggleRunCentering(int choice);
void M_CrispyToggleSmoothLighting(int choice);
void M_CrispyToggleSmoothMap(int choice);
void M_CrispyToggleSmoothScaling(int choice);
void M_CrispyToggleSndChannels(int choice);
void M_CrispyToggleSoundMono(int choice);
void M_CrispyToggleUncapped(int choice);
void M_CrispyToggleVsync(int choice);
void M_CrispyToggleWidescreen(int choice);

#endif
