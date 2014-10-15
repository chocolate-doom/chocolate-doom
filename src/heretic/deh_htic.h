//
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
//
// Common header for Heretic dehacked (HHE) support.
//

#ifndef DEH_HTIC_H
#define DEH_HTIC_H

#include "info.h"

// HHE executable version.  Loading HHE patches is (unfortunately)
// dependent on the version of the Heretic executable used to make them.

typedef enum
{
    deh_hhe_1_0,
    deh_hhe_1_2,
    deh_hhe_1_3,
    deh_hhe_num_versions
} deh_hhe_version_t;

// HHE doesn't know about the last two states in the state table, so
// these are considered invalid.

#define DEH_HERETIC_NUMSTATES (NUMSTATES - 2)

// It also doesn't know about the last two things in the mobjinfo table
// (which correspond to the states above)

#define DEH_HERETIC_NUMMOBJTYPES (NUMMOBJTYPES - 2)

void DEH_HereticInit(void);
int DEH_MapHereticThingType(int type);
int DEH_MapHereticFrameNumber(int frame);
void DEH_SuggestHereticVersion(deh_hhe_version_t version);

extern deh_hhe_version_t deh_hhe_version;

#endif /* #ifndef DEH_HTIC_H */

