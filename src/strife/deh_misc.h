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
// Parses "Misc" sections in dehacked files
//

#ifndef DEH_MISC_H
#define DEH_MISC_H

#include "doomfeatures.h"

#define DEH_DEFAULT_INITIAL_HEALTH 100
#define DEH_DEFAULT_INITIAL_BULLETS 50
#define DEH_DEFAULT_MAX_HEALTH 200
#define DEH_DEFAULT_MAX_ARMOR 200
#define DEH_DEFAULT_GREEN_ARMOR_CLASS 1
#define DEH_DEFAULT_BLUE_ARMOR_CLASS 2
#define DEH_DEFAULT_MAX_SOULSPHERE 200
#define DEH_DEFAULT_SOULSPHERE_HEALTH 100
#define DEH_DEFAULT_MEGASPHERE_HEALTH 200
#define DEH_DEFAULT_GOD_MODE_HEALTH 100
#define DEH_DEFAULT_IDFA_ARMOR 200
#define DEH_DEFAULT_IDFA_ARMOR_CLASS 2
#define DEH_DEFAULT_IDKFA_ARMOR 200
#define DEH_DEFAULT_IDKFA_ARMOR_CLASS 2
#define DEH_DEFAULT_BFG_CELLS_PER_SHOT 40
#define DEH_DEFAULT_SPECIES_INFIGHTING 0

#ifdef FEATURE_DEHACKED

extern int deh_initial_health;
extern int deh_initial_bullets;
extern int deh_max_health;
extern int deh_max_armor;
extern int deh_green_armor_class;
extern int deh_blue_armor_class;
extern int deh_max_soulsphere;
extern int deh_soulsphere_health;
extern int deh_megasphere_health;
extern int deh_god_mode_health;
extern int deh_idfa_armor;
extern int deh_idfa_armor_class;
extern int deh_idkfa_armor;
extern int deh_idkfa_armor_class;
extern int deh_bfg_cells_per_shot;
extern int deh_species_infighting;

#else

// If dehacked is disabled, hard coded values

#define deh_initial_health      DEH_DEFAULT_INITIAL_HEALTH
#define deh_initial_bullets     DEH_DEFAULT_INITIAL_BULLETS
#define deh_max_health          DEH_DEFAULT_MAX_HEALTH
#define deh_max_armor           DEH_DEFAULT_MAX_ARMOR
#define deh_green_armor_class   DEH_DEFAULT_GREEN_ARMOR_CLASS
#define deh_blue_armor_class    DEH_DEFAULT_BLUE_ARMOR_CLASS
#define deh_max_soulsphere      DEH_DEFAULT_MAX_SOULSPHERE
#define deh_soulsphere_health   DEH_DEFAULT_SOULSPHERE_HEALTH
#define deh_megasphere_health   DEH_DEFAULT_MEGASPHERE_HEALTH
#define deh_god_mode_health     DEH_DEFAULT_GOD_MODE_HEALTH
#define deh_idfa_armor          DEH_DEFAULT_IDFA_ARMOR
#define deh_idfa_armor_class    DEH_DEFAULT_IDFA_ARMOR_CLASS
#define deh_idkfa_armor         DEH_DEFAULT_IDKFA_ARMOR
#define deh_idkfa_armor_class   DEH_DEFAULT_IDKFA_ARMOR_CLASS
#define deh_bfg_cells_per_shot  DEH_DEFAULT_BFG_CELLS_PER_SHOT
#define deh_species_infighting  DEH_DEFAULT_SPECIES_INFIGHTING

#endif

#endif /* #ifndef DEH_MISC_H */

