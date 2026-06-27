//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2005-2014 Simon Howard
// Copyright(C) 2026 Sysop-64 contributors
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
//     Internal shared declarations for Sysop-64 image conversion modules.
//

#ifndef SYSOP64_IMAGE_INTERNAL_H
#define SYSOP64_IMAGE_INTERNAL_H

#include "i_video.h"
#include "sysop64_image.h"

#include <stdint.h>

extern const int sysop_default_palette[16][3];
extern int g_sysop_image_bg_color;
extern int g_sysop_image_menu_dither_mode;

// Add a unique non-background color to a small per-cell palette candidate list.
void Sysop_ImageAddPaletteCandidate(uint8_t *candidates, int *count, int color, int bg);
// Score how well one C64 bitmap cell palette covers the 32 multicolor samples
// in a cell.
int Sysop_ImageScorePaletteSet(const int sampleDist[32][16], int bg, int c0, int c1, int c2);
// Assign three local colors to screen RAM high/low nibbles and color RAM while
// preferring stable roles from the previous frame.
void Sysop_ImageChoosePaletteRoles(uint8_t p0,
                                   uint8_t p1,
                                   uint8_t p2,
                                   uint8_t prevScreen,
                                   uint8_t prevColor,
                                   uint8_t *screenRAM,
                                   uint8_t *colorRAM);
// Resolve two per-pixel color locks into the single C64 multicolor pair color.
uint8_t Sysop_ImageSelectLockedPairColor(uint8_t left, uint8_t right);

// Mark the mega converter's internal fast color tables dirty.
void Sysop_ImageMegaRecomputeColorTables(void);

#endif
