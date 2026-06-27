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
//     Shared C64 image conversion state, palette data, and Koala frame buffer
//     helpers for the Sysop-64 renderer.
//

#include "config.h"
#include "i_video.h"
#include "sysop64_image.h"
#include "sysop64_image_internal.h"

#include <limits.h>
#include <stdint.h>
#include <string.h>

uint8_t sysop_c64_frame[10000 + 1];

static int sysop_conversion_palette[16][3] = {
    {  0,   0,   0},
    {255, 255, 255},
    {190,  26,  36},
    { 48, 230, 198},
    {180,  26, 226},
    { 31, 210,  30},
    { 33,  27, 174},
    {223, 246,  10},
    {184,  65,   4},
    {106,  51,   4},
    {254,  74,  87},
    { 66,  69,  64},
    {112, 116, 111},
    { 89, 254,  89},
    { 85,  83, 254},
    {164, 167, 162}
};

const int sysop_default_palette[16][3] = {
    {  0,   0,   0},
    {255, 255, 255},
    {190,  26,  36},
    { 48, 230, 198},
    {180,  26, 226},
    { 31, 210,  30},
    { 33,  27, 174},
    {223, 246,  10},
    {184,  65,   4},
    {106,  51,   4},
    {254,  74,  87},
    { 66,  69,  64},
    {112, 116, 111},
    { 89, 254,  89},
    { 85,  83, 254},
    {164, 167, 162}
};

static unsigned int sysop_conversion_palette_revision = 1;
int g_sysop_image_bg_color = 0;
int g_sysop_image_menu_dither_mode = SYSOP_MENU_DITHER_SHARP;

// Copy one 16-color RGB palette without exposing callers to the storage size.
void Sysop_ImageCopyPalette(int dst[16][3], const int src[16][3])
{
    memcpy(dst, src, sizeof(int) * 16 * 3);
}

// Return the display palette selected by the mega converter, falling back to
// the Sysop default palette if the model is unavailable.
const int (*Sysop_ImageBaseDisplayPalette(void))[3]
{
    SysopMegaOptions mega_options;
    const int (*mega_palette)[3];

    Sysop_ImageMegaGetOptions(&mega_options);
    mega_palette = Sysop_ImageMegaPaletteForModel(mega_options.palette_model);
    if (mega_palette != NULL) {
        return mega_palette;
    }

    return sysop_default_palette;
}

// Update the palette used for color matching and bump its revision when the
// values actually change.
static void Sysop_ImageSetConversionPaletteInternal(const int palette[16][3])
{
    int palette_changed = memcmp(sysop_conversion_palette, palette, sizeof(int) * 16 * 3) != 0;

    if (palette_changed) {
        for (int i = 0; i < 16; i++) {
            sysop_conversion_palette[i][0] = palette[i][0];
            sysop_conversion_palette[i][1] = palette[i][1];
            sysop_conversion_palette[i][2] = palette[i][2];
        }
    }

    if (palette_changed) {
        ++sysop_conversion_palette_revision;
        if (sysop_conversion_palette_revision == 0) {
            sysop_conversion_palette_revision = 1;
        }
    }
}

// Install a conversion palette for special cases such as menu readability
// tuning.
void Sysop_ImageSetConversionPaletteFrom(const int palette[16][3])
{
    Sysop_ImageSetConversionPaletteInternal(palette);
}

// Install the gameplay conversion palette used by the normal Doom frame path.
void Sysop_ImageSetGameplayConversionPaletteFrom(const int display_palette[16][3])
{
    Sysop_ImageSetConversionPaletteInternal(display_palette);
}

// Expose palette revision changes so callers can rebuild cached lookup tables
// only when needed.
unsigned int Sysop_ImageConversionPaletteRevision(void)
{
    return sysop_conversion_palette_revision;
}

// Score RGB distance with luma-weighted integer math tuned for C64 palette
// matching.
static int Sysop_ImageColorDistance(int r1, int g1, int b1, int r2, int g2, int b2)
{
    int dr = r1 - r2;
    int dg = g1 - g2;
    int db = b1 - b2;
    return dr * dr * 30 + dg * dg * 59 + db * db * 11;
}

// Score a source RGB value against one color in the active conversion palette.
static int Sysop_ImageConversionColorDistance(int r, int g, int b, int c64Color)
{
    return Sysop_ImageColorDistance(sysop_conversion_palette[c64Color][0],
                                    sysop_conversion_palette[c64Color][1],
                                    sysop_conversion_palette[c64Color][2],
                                    r, g, b);
}

// Rebuild converter-side lookup tables after palette or tuning changes.
void Sysop_ImageRecomputeColorQuantizationTables(void)
{
    Sysop_ImageMegaRecomputeColorTables();
}

// Prepare the shared C64 frame buffer and quantization tables before the first
// rendered frame.
void Sysop_ImagePrecomputeColorQuantization(void)
{
    memset(sysop_c64_frame, 0, 10000 + 1);
    Sysop_ImageRecomputeColorQuantizationTables();
}

// Add a unique non-background C64 color to a small candidate list.
void Sysop_ImageAddPaletteCandidate(uint8_t *candidates, int *count, int color, int bg)
{
    if (color < 0 || color >= 16 || color == bg) {
        return;
    }

    for (int i = 0; i < *count; i++) {
        if (candidates[i] == color) {
            return;
        }
    }

    if (*count < 10) {
        candidates[(*count)++] = (uint8_t)color;
    }
}

// Score how well one Koala cell palette covers the 32 multicolor samples in a
// block.
int Sysop_ImageScorePaletteSet(const int sampleDist[32][16], int bg, int c0, int c1, int c2)
{
    int score = 0;

    for (int i = 0; i < 32; i++) {
        int best = sampleDist[i][bg];
        if (sampleDist[i][c0] < best) best = sampleDist[i][c0];
        if (sampleDist[i][c1] < best) best = sampleDist[i][c1];
        if (sampleDist[i][c2] < best) best = sampleDist[i][c2];
        score += best;
    }

    return score;
}

// Assign three local colors to screen high nibble, screen low nibble, and color
// RAM while preferring roles that changed least from the previous frame.
void Sysop_ImageChoosePaletteRoles(uint8_t p0,
                                   uint8_t p1,
                                   uint8_t p2,
                                   uint8_t prevScreen,
                                   uint8_t prevColor,
                                   uint8_t *screenRAM,
                                   uint8_t *colorRAM)
{
    uint8_t p[3] = { p0, p1, p2 };
    int bestPenalty = INT_MAX;
    uint8_t bestScreen = 0, bestColor = 0;

    for (int hi = 0; hi < 3; hi++) {
        for (int lo = 0; lo < 3; lo++) {
            if (lo == hi) continue;

            for (int cr = 0; cr < 3; cr++) {
                if (cr == hi || cr == lo) continue;

                uint8_t s = (p[hi] << 4) | p[lo];
                uint8_t c = p[cr];
                int penalty = 0;

                if (s != prevScreen) penalty += 8;
                if (c != (prevColor & 0x0f)) penalty += 8;
                if (p[hi] != (prevScreen >> 4)) penalty += 1;
                if (p[lo] != (prevScreen & 0x0f)) penalty += 1;

                if (penalty < bestPenalty) {
                    bestPenalty = penalty;
                    bestScreen = s;
                    bestColor = c;
                }
            }
        }
    }

    *screenRAM = bestScreen;
    *colorRAM = bestColor;
}

// Rank locked menu colors when two source pixels in a multicolor pair request
// different fixed C64 colors.
static int lockedC64ColorPriority(uint8_t color)
{
    if (g_sysop_image_menu_dither_mode == SYSOP_MENU_DITHER_POSTER) {
        switch (color) {
            case 1: return 100;
            case 7: return 90;
            case 10: return 80;
            case 11: return 78;
            case 2: return 70;
            case 8: return 60;
            case 15: return 55;
            case 9: return 50;
            case 12: return 45;
            default: return 20;
        }
    }

    switch (color) {
        case 1: return 100;
        case 7: return 90;
        case 10: return 80;
        case 2: return 70;
        case 8: return 60;
        case 9: return 50;
        case 15: return 45;
        case 12: return 40;
        case 11: return 35;
        default: return 20;
    }
}

// Resolve two per-pixel color locks into the one color available to a C64
// multicolor pixel pair.
uint8_t Sysop_ImageSelectLockedPairColor(uint8_t left, uint8_t right)
{
    left = (left > 0 && left < 16) ? left : 0;
    right = (right > 0 && right < 16) ? right : 0;

    if (!left) return right;
    if (!right) return left;
    if (left == right) return left;

    return lockedC64ColorPriority(right) > lockedC64ColorPriority(left) ? right : left;
}

// Find the nearest C64 palette color in the active conversion palette.
int Sysop_ImageNearestC64PaletteColorRgb(int r, int g, int b)
{
    int best = 0;
    int bestDist = INT_MAX;

    for (int color = 0; color < 16; color++) {
        int d = Sysop_ImageConversionColorDistance(r, g, b, color);
        if (d < bestDist) {
            bestDist = d;
            best = color;
        }
    }

    return best;
}
