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
#define DEH_DEAFULT_RNG {
    0,   8, 109, 220, 222, 241, 149, 107,  75, 248, 254, 140,  16,  66 ,
    74,  21, 211,  47,  80, 242, 154,  27, 205, 128, 161,  89,  77,  36 ,
    95, 110,  85,  48, 212, 140, 211, 249,  22,  79, 200,  50,  28, 188 ,
    52, 140, 202, 120,  68, 145,  62,  70, 184, 190,  91, 197, 152, 224 ,
    149, 104,  25, 178, 252, 182, 202, 182, 141, 197,   4,  81, 181, 242 ,
    145,  42,  39, 227, 156, 198, 225, 193, 219,  93, 122, 175, 249,   0 ,
    175, 143,  70, 239,  46, 246, 163,  53, 163, 109, 168, 135,   2, 235 ,
    25,  92,  20, 145, 138,  77,  69, 166,  78, 176, 173, 212, 166, 113 ,
    94, 161,  41,  50, 239,  49, 111, 164,  70,  60,   2,  37, 171,  75 ,
    136, 156,  11,  56,  42, 146, 138, 229,  73, 146,  77,  61,  98, 196 ,
    135, 106,  63, 197, 195,  86,  96, 203, 113, 101, 170, 247, 181, 113 ,
    80, 250, 108,   7, 255, 237, 129, 226,  79, 107, 112, 166, 103, 241 ,
    24, 223, 239, 120, 198,  58,  60,  82, 128,   3, 184,  66, 143, 224 ,
    145, 224,  81, 206, 163,  45,  63,  90, 168, 114,  59,  33, 159,  95 ,
    28, 139, 123,  98, 125, 196,  15,  70, 194, 253,  54,  14, 109, 226 ,
    71,  17, 161,  93, 186,  87, 244, 138,  20,  52, 123, 251,  26,  36 ,
    17,  46,  52, 231, 232,  76,  31, 221,  84,  37, 216, 165, 212, 106 ,
    197, 242,  98,  43,  39, 175, 254, 145, 190,  84, 118, 222, 187, 136 ,
    120, 163, 236, 249};

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
extern int[] deh_rngtable;

#if 0

// To compile without dehacked, it's possible to use these:

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
#define deh_rngtable DEH_DEAFULT_RNG

#endif

#endif /* #ifndef DEH_MISC_H */

