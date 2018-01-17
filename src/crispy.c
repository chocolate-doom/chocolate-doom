//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2005-2014 Simon Howard
// Copyright(C) 2014-2017 Fabian Greffrath
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
//	Crispy Doom specific variables.
//


#include "crispy.h"
#include <string.h>

// [crispy] "regular" config variables
static crispy_t crispy_s = {
	0,
	.extautomap = 1,
	.extsaveg = 1,
	.smoothscaling = 1,
	.soundfix = 1,
};
crispy_t *const crispy = &crispy_s;

// [crispy] "critical" config variables
static crispy_t critical_s = {0};
crispy_t *const critical = &critical_s;

// [crispy] update the "singleplayer" variable and the "critical" struct
void CheckCrispySingleplayer (boolean singleplayer)
{
	if ((crispy->singleplayer = singleplayer))
	{
		memcpy(critical, crispy, sizeof(critical_s));
	}
	else
	{
		memset(critical, 0, sizeof(critical_s));
	}
}
