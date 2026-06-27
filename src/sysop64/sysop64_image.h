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
//     Public interface for Sysop-64 C64 image conversion and mega tuning.
//

#ifndef SYSOP64_IMAGE_H
#define SYSOP64_IMAGE_H

#include <stdint.h>

#define SYSOP_MENU_DITHER_OFF 0
#define SYSOP_MENU_DITHER_SHARP 1
#define SYSOP_MENU_DITHER_POSTER 2

#define SYSOP_MEGA_DITHER_OFF 0
#define SYSOP_MEGA_DITHER_BAYER2 1
#define SYSOP_MEGA_DITHER_BAYER4 2
#define SYSOP_MEGA_DITHER_BAYER8 3
#define SYSOP_MEGA_DITHER_BAYER8X16 4
#define SYSOP_MEGA_DITHER_CHECKER 5
#define SYSOP_MEGA_DITHER_DIAGONAL 6
#define SYSOP_MEGA_DITHER_DOT 7
#define SYSOP_MEGA_DITHER_HASH 8

#define SYSOP_MEGA_PALETTE_CURRENT 0
#define SYSOP_MEGA_PALETTE_CUSTOM 1
#define SYSOP_MEGA_PALETTE_VICE_BASE 2

typedef struct {
    int dither_pattern;
    int palette_model;
    int dither_strength;
    int brightness;
    int contrast;
    int gamma;
    int saturation;
    int vibrance;
    int detail_pop;
    int surface_detail;
    int luma_weight;
    int chroma_weight;
    int red_weight;
    int green_weight;
    int blue_weight;
    int black_penalty;
    int yellow_penalty;
    int neutral_guard;
    int candidate_budget;
    int background_color;
    int fast_tables;
} SysopMegaOptions;

extern SysopMegaOptions g_sysop_mega_options;

extern uint8_t sysop_c64_frame[10000 + 1];

// Copy one 16-color RGB palette between fixed-size palette buffers.
void Sysop_ImageCopyPalette(int dst[16][3], const int src[16][3]);
// Return the base display palette selected by the current mega converter
// settings.
const int (*Sysop_ImageBaseDisplayPalette(void))[3];
// Install the palette used by conversion/scoring for special display modes.
void Sysop_ImageSetConversionPaletteFrom(const int palette[16][3]);
// Install the normal gameplay palette used by the frame converter.
void Sysop_ImageSetGameplayConversionPaletteFrom(const int display_palette[16][3]);
// Return a monotonically changing revision for conversion palette updates.
unsigned int Sysop_ImageConversionPaletteRevision(void);
// Initialize the shared C64 frame buffer and converter lookup state.
void Sysop_ImagePrecomputeColorQuantization(void);
// Mark converter color lookup tables dirty so they rebuild on next use.
void Sysop_ImageRecomputeColorQuantizationTables(void);
// Convert Doom's indexed 320x200 framebuffer into the shared C64 bitmap,
// screen RAM, color RAM, and background-color frame buffer.
void Sysop_ImageConvertMegaIndexed(const uint8_t *indexed_screen,
                                   const uint8_t palette[256][3],
                                   const uint8_t palette_locks[256],
                                   int menu_dither_mode);
// Find the nearest C64 color in the active conversion palette for one RGB
// value.
int Sysop_ImageNearestC64PaletteColorRgb(int r, int g, int b);
// Copy the current live mega converter options for callers and the HTTP tuner.
void Sysop_ImageMegaGetOptions(SysopMegaOptions *options);
// Reset mega converter options and custom palette to defaults.
void Sysop_ImageMegaResetOptions(void);
// Apply one named mega converter option from command line or HTTP input.
int Sysop_ImageMegaSetOption(const char *name, const char *value);
// Serialize the current mega converter state as JSON for the tuner page.
int Sysop_ImageMegaWriteStateJson(char *buffer, int buffer_len);
// Return the stable text name for a dither-pattern enum value.
const char *Sysop_ImageMegaDitherName(int pattern);
// Return the stable text name for a palette-model enum value.
const char *Sysop_ImageMegaPaletteName(int model);
// Return the human-readable label for a palette-model enum value.
const char *Sysop_ImageMegaPaletteLabel(int model);
// Return the number of palette choices exposed to the tuner UI.
int Sysop_ImageMegaPaletteOptionCount(void);
// Return the option value string for a palette choice index.
const char *Sysop_ImageMegaPaletteOptionName(int index);
// Return the display label for a palette choice index.
const char *Sysop_ImageMegaPaletteOptionLabel(int index);
// Return the 16-color RGB palette backing a palette-model enum value.
const int (*Sysop_ImageMegaPaletteForModel(int model))[3];
// Return the editable custom palette buffer used by custom palette mode.
const int (*Sysop_ImageMegaCustomPalette(void))[3];
// Return a revision that changes whenever the selected/custom palette changes.
unsigned int Sysop_ImageMegaPaletteRevision(void);

#endif
