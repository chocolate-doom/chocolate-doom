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
//     Tunable indexed framebuffer-to-C64 converter used by the Sysop-64
//     renderer, including palette scoring, dithering, and fast lookup tables.
//

#include "config.h"
#include "sysop64_image_internal.h"

#include <limits.h>
#include <math.h>
#include <pthread.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

typedef struct {
    int r, g, b;
    int lum;
    uint8_t targetR, targetG, targetB;
    uint8_t lockedColor;
    uint8_t locked;
} MegaSample;

typedef struct {
    uint8_t r, g, b, lum;
} MegaPairSample;

#define MEGA_PAIR_TABLE_SIZE (256 * 256)
#define MEGA_DISTANCE_CACHE_SIZE (1u << 16)
#define MEGA_DISTANCE_CACHE_PROBES 8

typedef struct {
    uint32_t key;
    uint32_t generation;
    int dist[16];
} MegaDistanceCacheEntry;

typedef struct {
    const char *key;
    const char *label;
    int rgb[16][3];
} MegaNamedPalette;

static const MegaNamedPalette mega_vice_palettes[] = {
    { "c64hq", "C64HQ", { { 10, 10, 10 }, { 255, 248, 255 }, { 133, 31, 2 }, { 101, 205, 168 }, { 167, 59, 159 }, { 77, 171, 25 }, { 26, 12, 146 }, { 235, 227, 83 }, { 169, 75, 2 }, { 68, 30, 0 }, { 210, 128, 116 }, { 70, 70, 70 }, { 139, 139, 139 }, { 142, 246, 142 }, { 77, 145, 209 }, { 186, 186, 186 } } },
    { "c64s", "C64S", { { 0, 0, 0 }, { 252, 252, 252 }, { 168, 0, 0 }, { 84, 252, 252 }, { 168, 0, 168 }, { 0, 168, 0 }, { 0, 0, 168 }, { 252, 252, 0 }, { 168, 84, 0 }, { 128, 44, 0 }, { 252, 84, 84 }, { 84, 84, 84 }, { 128, 128, 128 }, { 84, 252, 84 }, { 84, 84, 252 }, { 168, 168, 168 } } },
    { "ccs64", "CCS64", { { 16, 16, 16 }, { 255, 255, 255 }, { 224, 64, 64 }, { 96, 255, 255 }, { 224, 96, 224 }, { 64, 224, 64 }, { 64, 64, 224 }, { 255, 255, 64 }, { 224, 160, 64 }, { 156, 116, 72 }, { 255, 160, 160 }, { 84, 84, 84 }, { 136, 136, 136 }, { 160, 255, 160 }, { 160, 160, 255 }, { 192, 192, 192 } } },
    { "cjam", "Christopher Jam", { { 0, 0, 0 }, { 255, 255, 255 }, { 125, 32, 44 }, { 79, 179, 165 }, { 132, 37, 140 }, { 51, 152, 64 }, { 42, 27, 157 }, { 191, 208, 74 }, { 127, 65, 13 }, { 76, 46, 0 }, { 180, 79, 92 }, { 60, 60, 60 }, { 100, 100, 100 }, { 124, 229, 135 }, { 99, 81, 219 }, { 147, 147, 147 } } },
    { "colodore", "Colodore (PAL)", { { 0, 0, 0 }, { 255, 255, 255 }, { 150, 40, 46 }, { 91, 214, 206 }, { 159, 45, 173 }, { 65, 185, 54 }, { 39, 36, 196 }, { 239, 243, 71 }, { 159, 72, 21 }, { 94, 53, 0 }, { 218, 95, 102 }, { 71, 71, 71 }, { 120, 120, 120 }, { 145, 255, 132 }, { 104, 100, 255 }, { 174, 174, 174 } } },
    { "community-colors", "Community Colors (Retrofan et al)", { { 0, 0, 0 }, { 255, 255, 255 }, { 175, 42, 41 }, { 98, 216, 204 }, { 176, 63, 182 }, { 74, 198, 74 }, { 55, 57, 196 }, { 228, 237, 78 }, { 182, 89, 28 }, { 104, 56, 8 }, { 234, 116, 108 }, { 77, 77, 77 }, { 132, 132, 132 }, { 166, 250, 158 }, { 112, 124, 230 }, { 182, 182, 181 } } },
    { "deekay", "Deekay/Crest", { { 0, 0, 0 }, { 255, 255, 255 }, { 136, 32, 0 }, { 104, 208, 168 }, { 168, 56, 160 }, { 80, 184, 24 }, { 24, 16, 144 }, { 240, 232, 88 }, { 160, 72, 0 }, { 71, 43, 27 }, { 200, 120, 112 }, { 72, 72, 72 }, { 128, 128, 128 }, { 152, 255, 152 }, { 80, 144, 208 }, { 184, 184, 184 } } },
    { "frodo", "Frodo", { { 0, 0, 0 }, { 255, 255, 255 }, { 204, 0, 0 }, { 0, 255, 204 }, { 255, 0, 255 }, { 0, 204, 0 }, { 0, 0, 204 }, { 255, 255, 0 }, { 255, 136, 0 }, { 136, 68, 0 }, { 255, 136, 136 }, { 68, 68, 68 }, { 136, 136, 136 }, { 136, 255, 136 }, { 136, 136, 255 }, { 204, 204, 204 } } },
    { "godot", "Godot", { { 0, 0, 0 }, { 255, 255, 255 }, { 136, 0, 0 }, { 170, 255, 238 }, { 204, 68, 204 }, { 0, 204, 85 }, { 0, 0, 170 }, { 238, 238, 119 }, { 221, 136, 85 }, { 102, 68, 0 }, { 254, 119, 119 }, { 51, 51, 51 }, { 119, 119, 119 }, { 170, 255, 102 }, { 0, 136, 255 }, { 187, 187, 187 } } },
    { "lemon64", "Lemon64", { { 0, 0, 0 }, { 255, 255, 255 }, { 139, 62, 66 }, { 124, 211, 205 }, { 151, 70, 160 }, { 92, 178, 84 }, { 60, 57, 169 }, { 227, 231, 110 }, { 148, 87, 49 }, { 89, 60, 7 }, { 205, 119, 124 }, { 80, 80, 80 }, { 131, 131, 131 }, { 175, 248, 166 }, { 127, 125, 244 }, { 186, 186, 186 } } },
    { "palette", "PALette (by PAL/Offence)", { { 0, 0, 0 }, { 213, 213, 213 }, { 114, 53, 44 }, { 101, 159, 166 }, { 115, 58, 145 }, { 86, 141, 53 }, { 46, 35, 125 }, { 174, 183, 94 }, { 119, 79, 30 }, { 75, 60, 0 }, { 156, 99, 90 }, { 71, 71, 71 }, { 107, 107, 107 }, { 143, 194, 113 }, { 103, 93, 182 }, { 143, 143, 143 } } },
    { "palette_6569r1_v1r", "PALette 6569R1 (by Tobias)", { { 0, 0, 0 }, { 255, 255, 255 }, { 123, 31, 50 }, { 134, 223, 205 }, { 179, 88, 194 }, { 73, 166, 75 }, { 56, 41, 173 }, { 199, 213, 85 }, { 177, 116, 58 }, { 83, 61, 0 }, { 190, 98, 117 }, { 61, 61, 61 }, { 128, 128, 128 }, { 140, 232, 142 }, { 123, 108, 240 }, { 194, 194, 194 } } },
    { "palette_6569r5_v1r", "PALette 6569R5 (by Tobias)", { { 0, 0, 0 }, { 255, 255, 255 }, { 141, 48, 67 }, { 102, 192, 173 }, { 144, 53, 159 }, { 73, 166, 75 }, { 56, 41, 173 }, { 199, 213, 85 }, { 142, 81, 23 }, { 83, 61, 0 }, { 190, 98, 117 }, { 78, 78, 78 }, { 118, 118, 118 }, { 140, 232, 142 }, { 113, 98, 230 }, { 163, 163, 163 } } },
    { "palette_8565r2_v1r", "PALette 8565R2 (by Tobias)", { { 0, 0, 0 }, { 255, 255, 255 }, { 139, 52, 54 }, { 101, 190, 185 }, { 141, 54, 162 }, { 74, 166, 70 }, { 45, 48, 168 }, { 210, 207, 87 }, { 142, 80, 27 }, { 84, 61, 0 }, { 188, 101, 104 }, { 78, 78, 78 }, { 118, 118, 118 }, { 141, 233, 137 }, { 102, 105, 225 }, { 163, 163, 163 } } },
    { "palette_c64_amber", "PALette Amber P3/602nm (by Tobias)", { { 34, 22, 0 }, { 255, 169, 0 }, { 101, 67, 0 }, { 175, 116, 0 }, { 114, 75, 0 }, { 144, 96, 0 }, { 86, 57, 0 }, { 203, 134, 0 }, { 114, 75, 0 }, { 86, 57, 0 }, { 144, 96, 0 }, { 101, 67, 0 }, { 136, 90, 0 }, { 203, 134, 0 }, { 136, 90, 0 }, { 175, 116, 0 } } },
    { "palette_c64_cyan", "PALette Cyan (by Tobias)", { { 0, 35, 25 }, { 0, 255, 182 }, { 0, 103, 73 }, { 0, 176, 125 }, { 0, 115, 81 }, { 0, 145, 103 }, { 0, 88, 62 }, { 0, 203, 144 }, { 0, 115, 81 }, { 0, 88, 62 }, { 0, 145, 103 }, { 0, 103, 73 }, { 0, 137, 97 }, { 0, 203, 144 }, { 0, 137, 97 }, { 0, 176, 125 } } },
    { "palette_c64_green", "PALette Green P1/525nm (by Tobias)", { { 7, 35, 0 }, { 55, 255, 0 }, { 22, 103, 0 }, { 38, 176, 0 }, { 25, 115, 0 }, { 31, 145, 0 }, { 19, 88, 0 }, { 43, 203, 0 }, { 25, 115, 0 }, { 19, 88, 0 }, { 31, 145, 0 }, { 22, 103, 0 }, { 29, 137, 0 }, { 43, 203, 0 }, { 29, 137, 0 }, { 38, 176, 0 } } },
    { "pc64", "PC64", { { 33, 33, 33 }, { 255, 255, 255 }, { 181, 33, 33 }, { 115, 255, 255 }, { 181, 33, 181 }, { 33, 181, 33 }, { 33, 33, 181 }, { 255, 255, 33 }, { 181, 115, 33 }, { 148, 66, 33 }, { 255, 115, 115 }, { 115, 115, 115 }, { 148, 148, 148 }, { 115, 255, 115 }, { 115, 115, 255 }, { 181, 181, 181 } } },
    { "pepto-ntsc", "Pepto (NTSC)", { { 0, 0, 0 }, { 255, 255, 255 }, { 103, 55, 43 }, { 112, 163, 177 }, { 111, 61, 134 }, { 88, 140, 66 }, { 52, 40, 121 }, { 183, 198, 110 }, { 111, 78, 37 }, { 66, 56, 0 }, { 153, 102, 89 }, { 67, 67, 67 }, { 107, 107, 107 }, { 154, 209, 131 }, { 107, 94, 181 }, { 149, 149, 149 } } },
    { "pepto-ntsc-sony", "Pepto (NTSC, Sony Matrix)", { { 0, 0, 0 }, { 255, 255, 255 }, { 124, 53, 43 }, { 90, 166, 177 }, { 105, 65, 133 }, { 93, 134, 67 }, { 33, 46, 120 }, { 207, 190, 111 }, { 137, 74, 38 }, { 91, 51, 0 }, { 175, 100, 89 }, { 67, 67, 67 }, { 107, 107, 107 }, { 160, 203, 132 }, { 86, 101, 179 }, { 149, 149, 149 } } },
    { "pepto-pal", "Pepto (PAL)", { { 0, 0, 0 }, { 255, 255, 255 }, { 104, 55, 43 }, { 112, 164, 178 }, { 111, 61, 134 }, { 88, 141, 67 }, { 53, 40, 121 }, { 184, 199, 111 }, { 111, 79, 37 }, { 67, 57, 0 }, { 154, 103, 89 }, { 68, 68, 68 }, { 108, 108, 108 }, { 154, 210, 132 }, { 108, 94, 181 }, { 149, 149, 149 } } },
    { "pepto-palold", "Pepto (old PAL)", { { 0, 0, 0 }, { 255, 255, 255 }, { 88, 41, 29 }, { 145, 198, 213 }, { 145, 92, 168 }, { 88, 141, 67 }, { 53, 40, 121 }, { 184, 199, 111 }, { 145, 111, 67 }, { 67, 57, 0 }, { 154, 103, 89 }, { 53, 53, 53 }, { 116, 116, 116 }, { 154, 210, 132 }, { 116, 102, 190 }, { 184, 184, 184 } } },
    { "pixcen", "PixCen", { { 0, 0, 0 }, { 255, 255, 255 }, { 137, 64, 54 }, { 122, 191, 199 }, { 138, 70, 174 }, { 104, 169, 65 }, { 62, 49, 162 }, { 208, 220, 113 }, { 144, 95, 37 }, { 92, 71, 0 }, { 187, 119, 109 }, { 85, 85, 85 }, { 128, 128, 128 }, { 172, 234, 136 }, { 124, 112, 218 }, { 171, 171, 171 } } },
    { "ptoing", "Ptoing", { { 0, 0, 0 }, { 255, 255, 255 }, { 140, 62, 52 }, { 122, 191, 199 }, { 141, 71, 179 }, { 104, 169, 65 }, { 62, 49, 162 }, { 208, 220, 113 }, { 144, 95, 37 }, { 87, 66, 0 }, { 187, 119, 109 }, { 84, 84, 84 }, { 128, 128, 128 }, { 172, 234, 136 }, { 124, 112, 218 }, { 171, 171, 171 } } },
    { "rgb", "RGB (fully saturated)", { { 0, 0, 0 }, { 255, 255, 255 }, { 255, 0, 0 }, { 0, 255, 255 }, { 255, 0, 255 }, { 0, 255, 0 }, { 0, 0, 255 }, { 255, 255, 0 }, { 255, 128, 0 }, { 128, 64, 0 }, { 255, 128, 128 }, { 64, 64, 64 }, { 128, 128, 128 }, { 128, 255, 128 }, { 128, 128, 255 }, { 192, 192, 192 } } },
    { "the64", "THE64", { { 0, 0, 0 }, { 255, 255, 255 }, { 150, 40, 46 }, { 91, 214, 206 }, { 159, 45, 173 }, { 65, 185, 54 }, { 39, 36, 196 }, { 239, 243, 71 }, { 159, 72, 21 }, { 94, 53, 0 }, { 218, 95, 102 }, { 71, 71, 71 }, { 120, 120, 120 }, { 145, 255, 132 }, { 104, 100, 255 }, { 174, 174, 174 } } },
    { "vice", "original VICE palette", { { 0, 0, 0 }, { 253, 254, 252 }, { 190, 26, 36 }, { 48, 230, 198 }, { 180, 26, 226 }, { 31, 210, 30 }, { 33, 27, 174 }, { 223, 246, 10 }, { 184, 65, 4 }, { 106, 51, 4 }, { 254, 74, 87 }, { 66, 69, 64 }, { 112, 116, 111 }, { 89, 254, 89 }, { 95, 83, 254 }, { 164, 167, 162 } } },
};

#define MEGA_VICE_PALETTE_COUNT ((int)(sizeof(mega_vice_palettes) / sizeof(mega_vice_palettes[0])))
#define SYSOP_MEGA_PALETTE_PEPTO_NTSC_SONY (SYSOP_MEGA_PALETTE_VICE_BASE + 19)

SysopMegaOptions g_sysop_mega_options = {
    SYSOP_MEGA_DITHER_BAYER8, SYSOP_MEGA_PALETTE_PEPTO_NTSC_SONY, 75,
    23, 150, 65,
    100, 0,
    0, 0,
    0, 0,
    3, 4, 4,
    0, 0, 0,
    6, -1,
    1
};

static const SysopMegaOptions g_sysop_mega_default_options = {
    SYSOP_MEGA_DITHER_BAYER8, SYSOP_MEGA_PALETTE_PEPTO_NTSC_SONY, 75,
    23, 150, 65,
    100, 0,
    0, 0,
    0, 0,
    3, 4, 4,
    0, 0, 0,
    6, -1,
    1
};

static pthread_mutex_t mega_options_mutex = PTHREAD_MUTEX_INITIALIZER;

static int mega_custom_palette[16][3] = {
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
static unsigned int mega_palette_revision = 1;

static SysopMegaOptions mega_cached_options;
static uint32_t mega_cached_doom_palette_hash = 0;
static uint32_t mega_cached_c64_palette_hash = 0;
static int mega_cached_tables_ready = 0;
static uint8_t mega_cached_tone_lut[256];
static uint8_t mega_cached_nearest_by_color[16 * 16];
static int8_t mega_cached_dither_noise[SCREENHEIGHT][SCREENWIDTH / 2];
static MegaPairSample mega_cached_pair_table[MEGA_PAIR_TABLE_SIZE];
static MegaDistanceCacheEntry mega_distance_cache[MEGA_DISTANCE_CACHE_SIZE];
static uint32_t mega_distance_cache_generation = 1;

static const uint8_t BAYER_2X2[2][2] = {
    { 0, 2 },
    { 3, 1 }
};

static const uint8_t BAYER_4X4[4][4] = {
    {  0,  8,  2, 10 },
    { 12,  4, 14,  6 },
    {  3, 11,  1,  9 },
    { 15,  7, 13,  5 }
};

static const uint8_t BAYER_8X8[8][8] = {
    {  0, 48, 12, 60,  3, 51, 15, 63 },
    { 32, 16, 44, 28, 35, 19, 47, 31 },
    {  8, 56,  4, 52, 11, 59,  7, 55 },
    { 40, 24, 36, 20, 43, 27, 39, 23 },
    {  2, 50, 14, 62,  1, 49, 13, 61 },
    { 34, 18, 46, 30, 33, 17, 45, 29 },
    { 10, 58,  6, 54,  9, 57,  5, 53 },
    { 42, 26, 38, 22, 41, 25, 37, 21 }
};

static const uint8_t BAYER_8X16[16][8] = {
    {   0,  32,   8,  40,   2,  34,  10,  42 },
    {  64,  96,  72, 104,  66,  98,  74, 106 },
    {  16,  48,  24,  56,  18,  50,  26,  58 },
    {  80, 112,  88, 120,  82, 114,  90, 122 },
    {   4,  36,  12,  44,   6,  38,  14,  46 },
    {  68, 100,  76, 108,  70, 102,  78, 110 },
    {  20,  52,  28,  60,  22,  54,  30,  62 },
    {  84, 116,  92, 124,  86, 118,  94, 126 },
    {   1,  33,   9,  41,   3,  35,  11,  43 },
    {  65,  97,  73, 105,  67,  99,  75, 107 },
    {  17,  49,  25,  57,  19,  51,  27,  59 },
    {  81, 113,  89, 121,  83, 115,  91, 123 },
    {   5,  37,  13,  45,   7,  39,  15,  47 },
    {  69, 101,  77, 109,  71, 103,  79, 111 },
    {  21,  53,  29,  61,  23,  55,  31,  63 },
    {  85, 117,  93, 125,  87, 119,  95, 127 }
};

// Clamp integer tuning values to the safe range accepted by the converter.
static inline int clamp_int(int value, int min_value, int max_value)
{
    if (value < min_value) return min_value;
    if (value > max_value) return max_value;
    return value;
}

// Clamp intermediate RGB math back into an 8-bit component.
static inline uint8_t clamp_u8(int value)
{
    if (value < 0) return 0;
    if (value > 255) return 255;
    return (uint8_t)value;
}

// Parse boolean-style command line and HTTP tuner values, accepting friendly
// aliases like on/off and fast/precise.
static int mega_bool_from_name(const char *value, int default_value)
{
    if (value == NULL || value[0] == '\0') {
        return default_value;
    }

    if (!strcasecmp(value, "1") || !strcasecmp(value, "on")
        || !strcasecmp(value, "yes") || !strcasecmp(value, "true")
        || !strcasecmp(value, "fast") || !strcasecmp(value, "lut")
        || !strcasecmp(value, "cache")) {
        return 1;
    }

    if (!strcasecmp(value, "0") || !strcasecmp(value, "off")
        || !strcasecmp(value, "no") || !strcasecmp(value, "false")
        || !strcasecmp(value, "slow") || !strcasecmp(value, "precise")) {
        return 0;
    }

    return atoi(value) != 0;
}

// Strictly parse a signed integer option without accepting trailing text.
static int mega_parse_int_value(const char *value, int *out)
{
    char *end;
    long parsed;

    if (value == NULL || value[0] == '\0') {
        return 0;
    }

    parsed = strtol(value, &end, 10);
    if (end == value || *end != '\0') {
        return 0;
    }

    if (parsed < INT_MIN || parsed > INT_MAX) {
        return 0;
    }

    *out = (int)parsed;
    return 1;
}

// Convert one hexadecimal digit into its numeric value.
static int mega_hex_value(int c)
{
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

// Parse a palette color value from #rrggbb, 0xrrggbb, or r,g,b style text.
static int mega_parse_rgb_value(const char *value, int *r, int *g, int *b)
{
    int rr, gg, bb;
    const char *hex;

    if (value == NULL) {
        return 0;
    }

    hex = value[0] == '#' ? value + 1 : value;
    if ((strlen(hex) == 6)
        && mega_hex_value(hex[0]) >= 0 && mega_hex_value(hex[1]) >= 0
        && mega_hex_value(hex[2]) >= 0 && mega_hex_value(hex[3]) >= 0
        && mega_hex_value(hex[4]) >= 0 && mega_hex_value(hex[5]) >= 0) {
        *r = (mega_hex_value(hex[0]) << 4) | mega_hex_value(hex[1]);
        *g = (mega_hex_value(hex[2]) << 4) | mega_hex_value(hex[3]);
        *b = (mega_hex_value(hex[4]) << 4) | mega_hex_value(hex[5]);
        return 1;
    }

    if (!strncasecmp(value, "0x", 2) && strlen(value + 2) == 6) {
        return mega_parse_rgb_value(value + 2, r, g, b);
    }

    if (sscanf(value, "%d,%d,%d", &rr, &gg, &bb) == 3
        || sscanf(value, "%d:%d:%d", &rr, &gg, &bb) == 3
        || sscanf(value, "%d/%d/%d", &rr, &gg, &bb) == 3) {
        *r = clamp_int(rr, 0, 255);
        *g = clamp_int(gg, 0, 255);
        *b = clamp_int(bb, 0, 255);
        return 1;
    }

    return 0;
}

// Parse target palette color names such as color2 or palette_color_10.
static int mega_color_index_from_name(const char *name)
{
    const char *p = name;

    if (p == NULL) {
        return -1;
    }

    if (!strncasecmp(p, "palette_color_", 14)) {
        p += 14;
    } else if (!strncasecmp(p, "palette-color-", 14)) {
        p += 14;
    } else if (!strncasecmp(p, "palette_color", 13)) {
        p += 13;
    } else if (!strncasecmp(p, "palette", 7)) {
        p += 7;
    } else if (!strncasecmp(p, "color_", 6)) {
        p += 6;
    } else if (!strncasecmp(p, "color", 5)) {
        p += 5;
    } else if ((p[0] == 'c' || p[0] == 'C') && p[1] >= '0' && p[1] <= '9') {
        p += 1;
    } else {
        return -1;
    }

    if (*p < '0' || *p > '9') {
        return -1;
    }

    int color = atoi(p);
    return color >= 0 && color < 16 ? color : -1;
}

// Compare all options that affect conversion output or cached lookup tables.
static int mega_options_equal(const SysopMegaOptions *a, const SysopMegaOptions *b)
{
    return a->dither_pattern == b->dither_pattern
        && a->palette_model == b->palette_model
        && a->dither_strength == b->dither_strength
        && a->brightness == b->brightness
        && a->contrast == b->contrast
        && a->gamma == b->gamma
        && a->saturation == b->saturation
        && a->vibrance == b->vibrance
        && a->detail_pop == b->detail_pop
        && a->surface_detail == b->surface_detail
        && a->luma_weight == b->luma_weight
        && a->chroma_weight == b->chroma_weight
        && a->red_weight == b->red_weight
        && a->green_weight == b->green_weight
        && a->blue_weight == b->blue_weight
        && a->black_penalty == b->black_penalty
        && a->yellow_penalty == b->yellow_penalty
        && a->neutral_guard == b->neutral_guard
        && a->candidate_budget == b->candidate_budget
        && a->background_color == b->background_color
        && a->fast_tables == b->fast_tables;
}

// Hash arbitrary table input so cached conversion tables can detect stale
// source palettes.
static uint32_t mega_hash_bytes(const void *data, size_t len)
{
    const uint8_t *p = (const uint8_t *)data;
    uint32_t hash = 2166136261u;

    for (size_t i = 0; i < len; ++i) {
        hash ^= p[i];
        hash *= 16777619u;
    }

    return hash ? hash : 1;
}

// Hash a 16-color C64 palette for fast-table cache invalidation.
static uint32_t mega_hash_c64_palette(const int palette[16][3])
{
    uint32_t hash = 2166136261u;

    for (int color = 0; color < 16; ++color) {
        for (int component = 0; component < 3; ++component) {
            int value = palette[color][component];
            hash ^= (uint8_t)(value & 0xff);
            hash *= 16777619u;
            hash ^= (uint8_t)((value >> 8) & 0xff);
            hash *= 16777619u;
        }
    }

    return hash ? hash : 1;
}

// Copy one 16-color RGB palette.
static void mega_copy_palette(int dst[16][3], const int src[16][3])
{
    memcpy(dst, src, sizeof(int) * 16 * 3);
}

// Restore the editable custom palette to the Sysop default color values.
static void mega_reset_custom_palette(void)
{
    const int default_palette[16][3] = {
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

    mega_copy_palette(mega_custom_palette, default_palette);
    ++mega_palette_revision;
}

// Advance the distance-cache generation so old RGB-to-C64 scores are ignored.
static void mega_invalidate_distance_cache(void)
{
    ++mega_distance_cache_generation;

    if (mega_distance_cache_generation == 0) {
        memset(mega_distance_cache, 0, sizeof(mega_distance_cache));
        mega_distance_cache_generation = 1;
    }
}

// Compute an integer luma approximation used for tone and perceptual distance.
static inline int mega_luma(int r, int g, int b)
{
    return (r * 77 + g * 150 + b * 29 + 128) >> 8;
}

// Estimate colorfulness as max RGB component minus min component.
static inline int mega_chroma(int r, int g, int b)
{
    int maxc = r;
    if (g > maxc) maxc = g;
    if (b > maxc) maxc = b;

    int minc = r;
    if (g < minc) minc = g;
    if (b < minc) minc = b;

    return maxc - minc;
}

// Generate deterministic per-pixel noise for the hash dither mode.
static uint32_t mega_hash(int x, int y)
{
    uint32_t h = (uint32_t)x * 0x45d9f3bu ^ (uint32_t)y * 0x119de1f3u;
    h ^= h >> 16;
    h *= 0x7feb352du;
    h ^= h >> 15;
    h *= 0x846ca68bu;
    h ^= h >> 16;
    return h;
}

// Resolve a CLI/HTTP dither pattern name into the internal enum value.
static int mega_dither_pattern_from_name(const char *name)
{
    if (name == NULL) return -1;
    if (!strcasecmp(name, "off") || !strcasecmp(name, "none")) return SYSOP_MEGA_DITHER_OFF;
    if (!strcasecmp(name, "bayer2") || !strcasecmp(name, "ordered2") || !strcasecmp(name, "2x2")) return SYSOP_MEGA_DITHER_BAYER2;
    if (!strcasecmp(name, "bayer4") || !strcasecmp(name, "ordered4") || !strcasecmp(name, "4x4")) return SYSOP_MEGA_DITHER_BAYER4;
    if (!strcasecmp(name, "bayer8") || !strcasecmp(name, "ordered8") || !strcasecmp(name, "8x8")) return SYSOP_MEGA_DITHER_BAYER8;
    if (!strcasecmp(name, "bayer8x16") || !strcasecmp(name, "ordered8x16") || !strcasecmp(name, "8x16")) return SYSOP_MEGA_DITHER_BAYER8X16;
    if (!strcasecmp(name, "checker") || !strcasecmp(name, "check")) return SYSOP_MEGA_DITHER_CHECKER;
    if (!strcasecmp(name, "diagonal") || !strcasecmp(name, "diag")) return SYSOP_MEGA_DITHER_DIAGONAL;
    if (!strcasecmp(name, "dot") || !strcasecmp(name, "dots")) return SYSOP_MEGA_DITHER_DOT;
    if (!strcasecmp(name, "hash") || !strcasecmp(name, "noise")) return SYSOP_MEGA_DITHER_HASH;
    return -1;
}

// Return the highest valid palette model id, including VICE-derived palettes.
static int mega_palette_model_max(void)
{
    return SYSOP_MEGA_PALETTE_VICE_BASE + MEGA_VICE_PALETTE_COUNT - 1;
}

// Convert a palette model id into a VICE palette table index.
static int mega_vice_index_from_model(int model)
{
    int index = model - SYSOP_MEGA_PALETTE_VICE_BASE;
    if (index < 0 || index >= MEGA_VICE_PALETTE_COUNT) {
        return -1;
    }
    return index;
}

// Resolve palette names, labels, and .vpl filenames into palette model ids.
static int mega_palette_model_from_name(const char *name)
{
    if (name == NULL) return -1;
    if (!strcasecmp(name, "current") || !strcasecmp(name, "default")
        || !strcasecmp(name, "doom") || !strcasecmp(name, "sysop")
        || !strcasecmp(name, "active")) {
        return SYSOP_MEGA_PALETTE_CURRENT;
    }
    if (!strcasecmp(name, "custom") || !strcasecmp(name, "user")
        || !strcasecmp(name, "editable") || !strcasecmp(name, "edit")) {
        return SYSOP_MEGA_PALETTE_CUSTOM;
    }

    for (int i = 0; i < MEGA_VICE_PALETTE_COUNT; ++i) {
        char vpl_name[96];
        snprintf(vpl_name, sizeof(vpl_name), "%s.vpl", mega_vice_palettes[i].key);
        if (!strcasecmp(name, mega_vice_palettes[i].key)
            || !strcasecmp(name, mega_vice_palettes[i].label)
            || !strcasecmp(name, vpl_name)) {
            return SYSOP_MEGA_PALETTE_VICE_BASE + i;
        }
    }

    return -1;
}

// Return the stable option name for a dither pattern id.
const char *Sysop_ImageMegaDitherName(int pattern)
{
    switch (pattern) {
        case SYSOP_MEGA_DITHER_OFF: return "off";
        case SYSOP_MEGA_DITHER_BAYER2: return "bayer2";
        case SYSOP_MEGA_DITHER_BAYER4: return "bayer4";
        case SYSOP_MEGA_DITHER_BAYER8: return "bayer8";
        case SYSOP_MEGA_DITHER_BAYER8X16: return "bayer8x16";
        case SYSOP_MEGA_DITHER_CHECKER: return "checker";
        case SYSOP_MEGA_DITHER_DIAGONAL: return "diagonal";
        case SYSOP_MEGA_DITHER_DOT: return "dot";
        case SYSOP_MEGA_DITHER_HASH: return "hash";
        default: return "unknown";
    }
}

// Return the stable option name for a palette model id.
const char *Sysop_ImageMegaPaletteName(int model)
{
    int vice_index = mega_vice_index_from_model(model);
    if (vice_index >= 0) {
        return mega_vice_palettes[vice_index].key;
    }

    switch (model) {
        case SYSOP_MEGA_PALETTE_CUSTOM:
            return "custom";
        case SYSOP_MEGA_PALETTE_CURRENT:
        default:
            return "current";
    }
}

// Return the human-readable label for a palette model id.
const char *Sysop_ImageMegaPaletteLabel(int model)
{
    int vice_index = mega_vice_index_from_model(model);
    if (vice_index >= 0) {
        return mega_vice_palettes[vice_index].label;
    }

    switch (model) {
        case SYSOP_MEGA_PALETTE_CUSTOM:
            return "Custom";
        case SYSOP_MEGA_PALETTE_CURRENT:
        default:
            return "Sysop default";
    }
}

// Return how many palette choices the HTTP tuner should expose.
int Sysop_ImageMegaPaletteOptionCount(void)
{
    return SYSOP_MEGA_PALETTE_VICE_BASE + MEGA_VICE_PALETTE_COUNT;
}

// Return the option value string for one palette choice.
const char *Sysop_ImageMegaPaletteOptionName(int index)
{
    if (index < 0 || index >= Sysop_ImageMegaPaletteOptionCount()) {
        return "current";
    }
    return Sysop_ImageMegaPaletteName(index);
}

// Return the display label for one palette choice.
const char *Sysop_ImageMegaPaletteOptionLabel(int index)
{
    if (index < 0 || index >= Sysop_ImageMegaPaletteOptionCount()) {
        return "Sysop default";
    }
    return Sysop_ImageMegaPaletteLabel(index);
}

// Return the 16-color RGB palette backing a palette model id.
const int (*Sysop_ImageMegaPaletteForModel(int model))[3]
{
    int vice_index = mega_vice_index_from_model(model);
    if (vice_index >= 0) {
        return mega_vice_palettes[vice_index].rgb;
    }

    if (model == SYSOP_MEGA_PALETTE_CUSTOM) {
        return (const int (*)[3])mega_custom_palette;
    }

    return sysop_default_palette;
}

// Fetch the active target palette for an option snapshot.
static const int (*mega_palette_for_options(const SysopMegaOptions *opts))[3]
{
    return Sysop_ImageMegaPaletteForModel(opts->palette_model);
}

// Expose the editable custom palette to the Sysop backend and tuner.
const int (*Sysop_ImageMegaCustomPalette(void))[3]
{
    return (const int (*)[3])mega_custom_palette;
}

// Expose palette revision changes so callers can refresh dependent state.
unsigned int Sysop_ImageMegaPaletteRevision(void)
{
    return mega_palette_revision;
}

// Switch to custom-palette mode by copying the currently selected palette; the
// caller must already hold mega_options_mutex.
static void mega_make_custom_from_current_locked(void)
{
    if (g_sysop_mega_options.palette_model != SYSOP_MEGA_PALETTE_CUSTOM) {
        const int (*source)[3] = mega_palette_for_options(&g_sysop_mega_options);
        mega_copy_palette(mega_custom_palette, source);
        g_sysop_mega_options.palette_model = SYSOP_MEGA_PALETTE_CUSTOM;
        ++mega_palette_revision;
    }
}

// Copy the current live tuning options under the options mutex.
void Sysop_ImageMegaGetOptions(SysopMegaOptions *options)
{
    if (options == NULL) {
        return;
    }

    pthread_mutex_lock(&mega_options_mutex);
    *options = g_sysop_mega_options;
    pthread_mutex_unlock(&mega_options_mutex);
}

// Restore default tuning, reset custom palette colors, and invalidate caches.
void Sysop_ImageMegaResetOptions(void)
{
    pthread_mutex_lock(&mega_options_mutex);
    g_sysop_mega_options = g_sysop_mega_default_options;
    mega_reset_custom_palette();
    mega_cached_tables_ready = 0;
    mega_invalidate_distance_cache();
    pthread_mutex_unlock(&mega_options_mutex);
}

// Normalize all tuner values after command line or HTTP updates.
static void clamp_mega_options(SysopMegaOptions *o)
{
    o->dither_pattern = clamp_int(o->dither_pattern, SYSOP_MEGA_DITHER_OFF, SYSOP_MEGA_DITHER_HASH);
    o->palette_model = clamp_int(o->palette_model, SYSOP_MEGA_PALETTE_CURRENT, mega_palette_model_max());
    o->dither_strength = clamp_int(o->dither_strength, 0, 150);
    o->brightness = clamp_int(o->brightness, -96, 96);
    o->contrast = clamp_int(o->contrast, 25, 250);
    o->gamma = clamp_int(o->gamma, 35, 220);
    o->saturation = clamp_int(o->saturation, 0, 240);
    o->vibrance = clamp_int(o->vibrance, 0, 160);
    o->detail_pop = clamp_int(o->detail_pop, 0, 240);
    o->surface_detail = clamp_int(o->surface_detail, 0, 200);
    o->luma_weight = clamp_int(o->luma_weight, 0, 16);
    o->chroma_weight = clamp_int(o->chroma_weight, 0, 16);
    o->red_weight = clamp_int(o->red_weight, 0, 16);
    o->green_weight = clamp_int(o->green_weight, 0, 16);
    o->blue_weight = clamp_int(o->blue_weight, 0, 16);
    o->black_penalty = clamp_int(o->black_penalty, 0, 240);
    o->yellow_penalty = clamp_int(o->yellow_penalty, 0, 240);
    o->neutral_guard = clamp_int(o->neutral_guard, 0, 240);
    o->candidate_budget = clamp_int(o->candidate_budget, 3, 8);
    o->background_color = clamp_int(o->background_color, -1, 15);
    o->fast_tables = clamp_int(o->fast_tables, 0, 1);
}

// Apply one named tuning option from command line or HTTP, updating palette and
// lookup caches when needed.
int Sysop_ImageMegaSetOption(const char *name, const char *value)
{
    int numeric;
    int color_index;
    int handled = 1;

    if (name == NULL || value == NULL) {
        return 0;
    }

    pthread_mutex_lock(&mega_options_mutex);

    color_index = mega_color_index_from_name(name);

    if (color_index >= 0) {
        int r, g, b;
        if (mega_parse_rgb_value(value, &r, &g, &b)) {
            mega_make_custom_from_current_locked();
            mega_custom_palette[color_index][0] = r;
            mega_custom_palette[color_index][1] = g;
            mega_custom_palette[color_index][2] = b;
            ++mega_palette_revision;
            mega_cached_tables_ready = 0;
            mega_invalidate_distance_cache();
        } else {
            handled = 0;
        }
    } else if (!strcasecmp(name, "palette_rgb") || !strcasecmp(name, "custom_palette")
               || !strcasecmp(name, "custom_palette_rgb")) {
        char buffer[512];
        char *cursor;
        int parsed = 0;
        int next_palette[16][3];

        snprintf(buffer, sizeof(buffer), "%s", value);
        cursor = buffer;
        while (parsed < 16 && cursor != NULL && *cursor) {
            char *next = strchr(cursor, ';');
            int r, g, b;

            if (next == NULL) {
                next = strchr(cursor, '|');
            }
            if (next == NULL) {
                next = strchr(cursor, ' ');
            }
            if (next != NULL) {
                *next++ = '\0';
            }

            if (mega_parse_rgb_value(cursor, &r, &g, &b)) {
                next_palette[parsed][0] = r;
                next_palette[parsed][1] = g;
                next_palette[parsed][2] = b;
                ++parsed;
            }

            cursor = next;
        }

        if (parsed == 16) {
            mega_copy_palette(mega_custom_palette, next_palette);
            g_sysop_mega_options.palette_model = SYSOP_MEGA_PALETTE_CUSTOM;
            ++mega_palette_revision;
            mega_cached_tables_ready = 0;
            mega_invalidate_distance_cache();
        } else {
            handled = 0;
        }
    } else if (!strcasecmp(name, "dither") || !strcasecmp(name, "dither_pattern")
        || !strcasecmp(name, "pattern")) {
        int pattern = mega_dither_pattern_from_name(value);
        if (pattern < 0 && !mega_parse_int_value(value, &pattern)) {
            handled = 0;
        } else {
            g_sysop_mega_options.dither_pattern = pattern;
        }
    } else if (!strcasecmp(name, "palette") || !strcasecmp(name, "palette_model")
               || !strcasecmp(name, "conversion_palette")) {
        int model = mega_palette_model_from_name(value);
        if (model < 0 && !mega_parse_int_value(value, &model)) {
            handled = 0;
        } else {
            if (model == SYSOP_MEGA_PALETTE_CUSTOM) {
                mega_make_custom_from_current_locked();
            } else if (model != g_sysop_mega_options.palette_model) {
                ++mega_palette_revision;
            }
            g_sysop_mega_options.palette_model = model;
        }
    } else {
        numeric = atoi(value);

        if (!strcasecmp(name, "strength") || !strcasecmp(name, "dither_strength")) {
            g_sysop_mega_options.dither_strength = numeric;
        } else if (!strcasecmp(name, "brightness") || !strcasecmp(name, "bright")) {
            g_sysop_mega_options.brightness = numeric;
        } else if (!strcasecmp(name, "contrast")) {
            g_sysop_mega_options.contrast = numeric;
        } else if (!strcasecmp(name, "gamma")) {
            g_sysop_mega_options.gamma = numeric;
        } else if (!strcasecmp(name, "saturation") || !strcasecmp(name, "sat")) {
            g_sysop_mega_options.saturation = numeric;
        } else if (!strcasecmp(name, "vibrance") || !strcasecmp(name, "vib")) {
            g_sysop_mega_options.vibrance = numeric;
        } else if (!strcasecmp(name, "detail") || !strcasecmp(name, "detail_pop")
                   || !strcasecmp(name, "pop")) {
            g_sysop_mega_options.detail_pop = numeric;
        } else if (!strcasecmp(name, "surface") || !strcasecmp(name, "surface_detail")) {
            g_sysop_mega_options.surface_detail = numeric;
        } else if (!strcasecmp(name, "luma") || !strcasecmp(name, "luma_weight")) {
            g_sysop_mega_options.luma_weight = numeric;
        } else if (!strcasecmp(name, "chroma") || !strcasecmp(name, "chroma_weight")) {
            g_sysop_mega_options.chroma_weight = numeric;
        } else if (!strcasecmp(name, "red") || !strcasecmp(name, "red_weight")) {
            g_sysop_mega_options.red_weight = numeric;
        } else if (!strcasecmp(name, "green") || !strcasecmp(name, "green_weight")) {
            g_sysop_mega_options.green_weight = numeric;
        } else if (!strcasecmp(name, "blue") || !strcasecmp(name, "blue_weight")) {
            g_sysop_mega_options.blue_weight = numeric;
        } else if (!strcasecmp(name, "black") || !strcasecmp(name, "black_penalty")) {
            g_sysop_mega_options.black_penalty = numeric;
        } else if (!strcasecmp(name, "yellow") || !strcasecmp(name, "yellow_penalty")) {
            g_sysop_mega_options.yellow_penalty = numeric;
        } else if (!strcasecmp(name, "neutral") || !strcasecmp(name, "neutral_guard")) {
            g_sysop_mega_options.neutral_guard = numeric;
        } else if (!strcasecmp(name, "candidates") || !strcasecmp(name, "candidate_budget")) {
            g_sysop_mega_options.candidate_budget = numeric;
        } else if (!strcasecmp(name, "background") || !strcasecmp(name, "background_color")
                   || !strcasecmp(name, "bg")) {
            g_sysop_mega_options.background_color = numeric;
        } else if (!strcasecmp(name, "fast") || !strcasecmp(name, "fast_tables")
                   || !strcasecmp(name, "tables") || !strcasecmp(name, "lut")
                   || !strcasecmp(name, "cache") || !strcasecmp(name, "precompute")) {
            g_sysop_mega_options.fast_tables = mega_bool_from_name(value, 1);
        } else {
            handled = 0;
        }
    }

    clamp_mega_options(&g_sysop_mega_options);
    pthread_mutex_unlock(&mega_options_mutex);
    return handled;
}

// Append formatted text to a bounded JSON buffer while tracking remaining
// space.
static int append_json(char **cursor, int *remaining, const char *fmt, ...)
{
    va_list args;
    int written;

    if (cursor == NULL || *cursor == NULL || remaining == NULL || *remaining <= 0) {
        return 0;
    }

    va_start(args, fmt);
    written = vsnprintf(*cursor, (size_t)*remaining, fmt, args);
    va_end(args);

    if (written < 0) {
        return 0;
    }

    if (written >= *remaining) {
        *cursor += *remaining - 1;
        *remaining = 1;
        return 0;
    }

    *cursor += written;
    *remaining -= written;
    return 1;
}

// Serialize current mega converter state for the HTTP tuner page.
int Sysop_ImageMegaWriteStateJson(char *buffer, int buffer_len)
{
    SysopMegaOptions o;
    const int (*palette)[3];
    char *cursor;
    int remaining;

    if (buffer == NULL || buffer_len <= 0) {
        return 0;
    }

    Sysop_ImageMegaGetOptions(&o);
    clamp_mega_options(&o);
    palette = mega_palette_for_options(&o);
    cursor = buffer;
    remaining = buffer_len;

    append_json(&cursor, &remaining,
        "{"
        "\"converter\":\"mega\","
        "\"dither\":\"%s\","
        "\"palette\":\"%s\","
        "\"palette_label\":\"%s\","
        "\"dither_pattern\":%d,"
        "\"palette_model\":%d,"
        "\"dither_strength\":%d,"
        "\"brightness\":%d,"
        "\"contrast\":%d,"
        "\"gamma\":%d,"
        "\"saturation\":%d,"
        "\"vibrance\":%d,"
        "\"detail_pop\":%d,"
        "\"surface_detail\":%d,"
        "\"luma_weight\":%d,"
        "\"chroma_weight\":%d,"
        "\"red_weight\":%d,"
        "\"green_weight\":%d,"
        "\"blue_weight\":%d,"
        "\"black_penalty\":%d,"
        "\"yellow_penalty\":%d,"
        "\"neutral_guard\":%d,"
        "\"candidate_budget\":%d,"
        "\"background_color\":%d,"
        "\"fast_tables\":\"%s\","
        "\"fast_tables_enabled\":%d,"
        "\"palette_rgb\":[",
        Sysop_ImageMegaDitherName(o.dither_pattern),
        Sysop_ImageMegaPaletteName(o.palette_model),
        Sysop_ImageMegaPaletteLabel(o.palette_model),
        o.dither_pattern,
        o.palette_model,
        o.dither_strength,
        o.brightness,
        o.contrast,
        o.gamma,
        o.saturation,
        o.vibrance,
        o.detail_pop,
        o.surface_detail,
        o.luma_weight,
        o.chroma_weight,
        o.red_weight,
        o.green_weight,
        o.blue_weight,
        o.black_penalty,
        o.yellow_penalty,
        o.neutral_guard,
        o.candidate_budget,
        o.background_color,
        o.fast_tables ? "on" : "off",
        o.fast_tables);

    for (int i = 0; i < 16; ++i) {
        append_json(&cursor, &remaining, "%s[%d,%d,%d]",
                    i == 0 ? "" : ",",
                    palette[i][0],
                    palette[i][1],
                    palette[i][2]);
    }

    append_json(&cursor, &remaining, "],\"custom_palette_rgb\":[");

    for (int i = 0; i < 16; ++i) {
        append_json(&cursor, &remaining, "%s[%d,%d,%d]",
                    i == 0 ? "" : ",",
                    mega_custom_palette[i][0],
                    mega_custom_palette[i][1],
                    mega_custom_palette[i][2]);
    }

    append_json(&cursor, &remaining, "],\"palette_options\":[");

    for (int i = 0; i < Sysop_ImageMegaPaletteOptionCount(); ++i) {
        append_json(&cursor, &remaining,
                    "%s{\"value\":\"%s\",\"label\":\"%s\"}",
                    i == 0 ? "" : ",",
                    Sysop_ImageMegaPaletteOptionName(i),
                    Sysop_ImageMegaPaletteOptionLabel(i));
    }

    append_json(&cursor, &remaining, "]}");
    return (int)(cursor - buffer);
}

// Mark all fast lookup tables dirty after palette or tuning changes.
void Sysop_ImageMegaRecomputeColorTables(void)
{
    mega_cached_tables_ready = 0;
    mega_invalidate_distance_cache();
}

// Precompute brightness, contrast, and gamma adjustment for every 8-bit input
// value.
static void build_tone_lut(const SysopMegaOptions *opts, uint8_t tone_lut[256])
{
    float gamma = (float)clamp_int(opts->gamma, 35, 220) / 100.0f;
    int contrast = clamp_int(opts->contrast, 25, 250);
    int brightness = clamp_int(opts->brightness, -96, 96);

    for (int i = 0; i < 256; ++i) {
        float x = (float)i / 255.0f;
        int v = (int)(powf(x, gamma) * 255.0f + 0.5f);
        v = 128 + ((v - 128) * contrast) / 100 + brightness;
        tone_lut[i] = clamp_u8(v);
    }
}

// Apply tone, saturation, and vibrance controls to one RGB sample.
static void apply_color_controls(const SysopMegaOptions *opts,
                                 const uint8_t tone_lut[256],
                                 int *r, int *g, int *b)
{
    int rr = tone_lut[clamp_int(*r, 0, 255)];
    int gg = tone_lut[clamp_int(*g, 0, 255)];
    int bb = tone_lut[clamp_int(*b, 0, 255)];
    int lum = mega_luma(rr, gg, bb);
    int chroma = mega_chroma(rr, gg, bb);
    int sat = opts->saturation + (opts->vibrance * (255 - chroma)) / 255;

    sat = clamp_int(sat, 0, 260);
    rr = lum + ((rr - lum) * sat) / 100;
    gg = lum + ((gg - lum) * sat) / 100;
    bb = lum + ((bb - lum) * sat) / 100;

    *r = clamp_u8(rr);
    *g = clamp_u8(gg);
    *b = clamp_u8(bb);
}

// Score one adjusted RGB color against one target C64 palette entry using the
// current perceptual weighting options.
static int mega_distance_to_c64(const SysopMegaOptions *opts, int r, int g, int b, int c64_color)
{
    const int (*palette)[3] = mega_palette_for_options(opts);
    int pr = palette[c64_color][0];
    int pg = palette[c64_color][1];
    int pb = palette[c64_color][2];
    int dr = r - pr;
    int dg = g - pg;
    int db = b - pb;
    int lum = mega_luma(r, g, b);
    int pluma = mega_luma(pr, pg, pb);
    int d_luma = lum - pluma;
    int chroma = mega_chroma(r, g, b);
    int pchroma = mega_chroma(pr, pg, pb);
    int d_chroma = chroma - pchroma;
    int dist = dr * dr * opts->red_weight
             + dg * dg * opts->green_weight
             + db * db * opts->blue_weight
             + d_luma * d_luma * opts->luma_weight
             + d_chroma * d_chroma * opts->chroma_weight;

    if (opts->neutral_guard > 0 && chroma < 44) {
        int neutral = 44 - chroma;
        dist += pchroma * pchroma * neutral * opts->neutral_guard / (44 * 8);
    }

    if (c64_color == 0 && opts->black_penalty > 0 && lum > 18) {
        int visible = lum - 18;
        dist += visible * visible * opts->black_penalty / 8;
    }

    if (c64_color == 7 && opts->yellow_penalty > 0) {
        int warmth = ((r + g) >> 1) - b;
        if (lum < 170 || warmth < 24) {
            int mismatch = (170 - lum > 0 ? 170 - lum : 0) + (24 - warmth > 0 ? 24 - warmth : 0);
            dist += mismatch * mismatch * opts->yellow_penalty / 24;
        }
    }

    return dist < 0 ? INT_MAX : dist;
}

// Find the nearest target C64 color without using the distance cache.
static int nearest_mega_color(const SysopMegaOptions *opts, int r, int g, int b)
{
    int best = 0;
    int best_dist = INT_MAX;

    for (int c = 0; c < 16; ++c) {
        int d = mega_distance_to_c64(opts, r, g, b, c);
        if (d < best_dist) {
            best_dist = d;
            best = c;
        }
    }

    return best;
}

// Build ranked fallback color lists for each C64 color to seed local palette
// candidates quickly.
static void build_mega_nearest_table(const SysopMegaOptions *opts, uint8_t nearest_by_color[16 * 16])
{
    const int (*palette)[3] = mega_palette_for_options(opts);

    for (int base = 0; base < 16; ++base) {
        uint8_t chosen[16];
        memset(chosen, 0, sizeof(chosen));
        chosen[base] = 1;

        for (int rank = 0; rank < 15; ++rank) {
            int best = 0xff;
            int best_dist = INT_MAX;

            for (int color = 0; color < 16; ++color) {
                if (chosen[color]) {
                    continue;
                }

                int d = mega_distance_to_c64(opts,
                                             palette[base][0],
                                             palette[base][1],
                                             palette[base][2],
                                             color);
                if (d < best_dist) {
                    best_dist = d;
                    best = color;
                }
            }

            nearest_by_color[base * 16 + rank] = (uint8_t)best;
            if (best >= 0 && best < 16) {
                chosen[best] = 1;
            }
        }

        nearest_by_color[base * 16 + 15] = 0xff;
    }
}

// Scale an ordered-dither rank into roughly centered signed noise.
static int mega_scaled_rank(int rank, int max_rank)
{
    return ((rank * 255 + max_rank / 2) / max_rank) - 128;
}

// Return unattenuated dither noise for one multicolor-pixel coordinate.
static int mega_raw_dither(const SysopMegaOptions *opts, int x, int y)
{
    switch (opts->dither_pattern) {
        case SYSOP_MEGA_DITHER_OFF:
            return 0;
        case SYSOP_MEGA_DITHER_BAYER2:
            return mega_scaled_rank(BAYER_2X2[y & 1][x & 1], 3);
        case SYSOP_MEGA_DITHER_BAYER4:
            return mega_scaled_rank(BAYER_4X4[y & 3][x & 3], 15);
        case SYSOP_MEGA_DITHER_BAYER8:
            return mega_scaled_rank(BAYER_8X8[y & 7][x & 7], 63);
        case SYSOP_MEGA_DITHER_CHECKER:
            return ((x ^ y) & 1) ? 96 : -96;
        case SYSOP_MEGA_DITHER_DIAGONAL:
            return (((x + y) & 7) * 255 / 7) - 128;
        case SYSOP_MEGA_DITHER_DOT: {
            int dx = ((x & 3) * 2) - 3;
            int dy = ((y & 3) * 2) - 3;
            int d = dx * dx + dy * dy;
            return d * 16 - 128;
        }
        case SYSOP_MEGA_DITHER_HASH:
            return (int)(mega_hash(x, y) & 0xff) - 128;
        case SYSOP_MEGA_DITHER_BAYER8X16:
        default:
            return mega_scaled_rank(BAYER_8X16[y & 15][x & 7], 127);
    }
}

// Apply the user dither-strength setting to raw pattern noise.
static int mega_dither_noise(const SysopMegaOptions *opts, int x, int y)
{
    return (mega_raw_dither(opts, x, y) * opts->dither_strength) / 300;
}

// Rebuild the fast conversion tables when options, Doom palette, or target C64
// palette have changed.
static void mega_prepare_cached_tables(const SysopMegaOptions *opts,
                                       const uint8_t palette[256][3],
                                       const int c64_palette[16][3])
{
    uint32_t doom_palette_hash = mega_hash_bytes(palette, 256 * 3);
    uint32_t c64_palette_hash = mega_hash_c64_palette(c64_palette);

    if (mega_cached_tables_ready
        && mega_options_equal(&mega_cached_options, opts)
        && mega_cached_doom_palette_hash == doom_palette_hash
        && mega_cached_c64_palette_hash == c64_palette_hash) {
        return;
    }

    mega_cached_options = *opts;
    mega_cached_doom_palette_hash = doom_palette_hash;
    mega_cached_c64_palette_hash = c64_palette_hash;

    build_tone_lut(opts, mega_cached_tone_lut);
    build_mega_nearest_table(opts, mega_cached_nearest_by_color);

    for (int y = 0; y < SCREENHEIGHT; ++y) {
        for (int x = 0; x < SCREENWIDTH / 2; ++x) {
            mega_cached_dither_noise[y][x] = (int8_t)mega_dither_noise(opts, x, y);
        }
    }

    for (int d0 = 0; d0 < 256; ++d0) {
        for (int d1 = 0; d1 < 256; ++d1) {
            int r = (palette[d0][0] + palette[d1][0]) >> 1;
            int g = (palette[d0][1] + palette[d1][1]) >> 1;
            int b = (palette[d0][2] + palette[d1][2]) >> 1;
            MegaPairSample *sample = &mega_cached_pair_table[(d0 << 8) | d1];

            apply_color_controls(opts, mega_cached_tone_lut, &r, &g, &b);
            sample->r = (uint8_t)r;
            sample->g = (uint8_t)g;
            sample->b = (uint8_t)b;
            sample->lum = (uint8_t)mega_luma(r, g, b);
        }
    }

    mega_invalidate_distance_cache();
    mega_cached_tables_ready = 1;
}

// Fetch precomputed dither noise for one 160-pixel-wide multicolor coordinate.
static inline int mega_cached_noise_at(int x, int y)
{
    return mega_cached_dither_noise[y][x];
}

// Hash a quantized RGB key into the distance-cache table.
static uint32_t mega_distance_cache_hash(uint32_t key)
{
    key ^= key >> 16;
    key *= 0x7feb352du;
    key ^= key >> 15;
    key *= 0x846ca68bu;
    key ^= key >> 16;
    return key;
}

// Return cached distances from one RGB value to all 16 C64 colors.
static const int *mega_distances_for_rgb(const SysopMegaOptions *opts, int r, int g, int b)
{
    r = clamp_u8(r);
    g = clamp_u8(g);
    b = clamp_u8(b);

    uint32_t key = ((uint32_t)r << 16)
                 | ((uint32_t)g << 8)
                 | (uint32_t)b;
    uint32_t slot = mega_distance_cache_hash(key) & (MEGA_DISTANCE_CACHE_SIZE - 1);
    MegaDistanceCacheEntry *entry = NULL;

    for (int probe = 0; probe < MEGA_DISTANCE_CACHE_PROBES; ++probe) {
        MegaDistanceCacheEntry *candidate = &mega_distance_cache[(slot + (uint32_t)probe) & (MEGA_DISTANCE_CACHE_SIZE - 1)];

        if (candidate->generation == mega_distance_cache_generation
            && candidate->key == key) {
            return candidate->dist;
        }

        if (entry == NULL && candidate->generation != mega_distance_cache_generation) {
            entry = candidate;
        }
    }

    if (entry == NULL) {
        entry = &mega_distance_cache[slot];
    }

    entry->key = key;
    entry->generation = mega_distance_cache_generation;
    for (int color = 0; color < 16; ++color) {
        entry->dist[color] = mega_distance_to_c64(opts, r, g, b, color);
    }

    return entry->dist;
}

// Find the nearest C64 color using the RGB distance cache.
static int nearest_mega_color_cached(const SysopMegaOptions *opts, int r, int g, int b)
{
    const int *dist = mega_distances_for_rgb(opts, r, g, b);
    int best = 0;
    int best_dist = dist[0];

    for (int color = 1; color < 16; ++color) {
        if (dist[color] < best_dist) {
            best_dist = dist[color];
            best = color;
        }
    }

    return best;
}

// Build the small set of local palette candidates considered for one 8x8 C64
// bitmap cell.
static void build_mega_palette_candidates(const SysopMegaOptions *opts,
                                          uint8_t *candidates,
                                          int *count,
                                          const uint8_t *color_histogram,
                                          const uint8_t nearest_by_color[16 * 16],
                                          uint8_t prev_screen,
                                          uint8_t prev_color,
                                          int bg)
{
    uint8_t used[16];
    memset(used, 0, sizeof(used));
    *count = 0;

    Sysop_ImageAddPaletteCandidate(candidates, count, prev_color & 0x0f, bg);
    Sysop_ImageAddPaletteCandidate(candidates, count, prev_screen >> 4, bg);
    Sysop_ImageAddPaletteCandidate(candidates, count, prev_screen & 0x0f, bg);

    for (int pick = 0; pick < opts->candidate_budget; ++pick) {
        int best = -1;

        for (int c = 0; c < 16; ++c) {
            if (c != bg && !used[c] && color_histogram[c]
                && (best < 0 || color_histogram[c] > color_histogram[best])) {
                best = c;
            }
        }

        if (best < 0) {
            break;
        }

        used[best] = 1;
        Sysop_ImageAddPaletteCandidate(candidates, count, best, bg);

        const uint8_t *near_colors = &nearest_by_color[best * 16];
        for (int i = 0; i < 3 && near_colors[i] < 16; ++i) {
            Sysop_ImageAddPaletteCandidate(candidates, count, near_colors[i], bg);
        }
    }

    static const uint8_t fallback[] = { 0, 11, 12, 15, 1, 6, 2, 5, 8, 14, 7, 4, 3, 13, 10, 9 };
    for (size_t i = 0; *count < 3 && i < sizeof(fallback); ++i) {
        Sysop_ImageAddPaletteCandidate(candidates, count, fallback[i], bg);
    }
}

// Map one averaged Doom pixel pair to the nearest of the four colors available
// in the selected C64 bitmap cell palette.
static uint8_t mega_nearest_pixel_for_palette(const SysopMegaOptions *opts,
                                              const MegaSample *sample,
                                              int mc_x,
                                              int y,
                                              uint8_t screen_ram,
                                              uint8_t color_ram)
{
    int r = sample->targetR;
    int g = sample->targetG;
    int b = sample->targetB;

    if (!sample->locked) {
        int noise = opts->fast_tables ? mega_cached_noise_at(mc_x, y)
                                      : mega_dither_noise(opts, mc_x, y);
        r = clamp_u8(r + noise);
        g = clamp_u8(g + noise);
        b = clamp_u8(b + noise);
    }

    const int *dist = opts->fast_tables ? mega_distances_for_rgb(opts, r, g, b) : NULL;
    int best_dist = dist != NULL ? dist[g_sysop_image_bg_color]
                                 : mega_distance_to_c64(opts, r, g, b, g_sysop_image_bg_color);
    uint8_t best_pixel = 0;

    int d = dist != NULL ? dist[screen_ram >> 4]
                         : mega_distance_to_c64(opts, r, g, b, screen_ram >> 4);
    if (d < best_dist) {
        best_dist = d;
        best_pixel = 1;
    }

    d = dist != NULL ? dist[screen_ram & 0x0f]
                     : mega_distance_to_c64(opts, r, g, b, screen_ram & 0x0f);
    if (d < best_dist) {
        best_dist = d;
        best_pixel = 2;
    }

    d = dist != NULL ? dist[color_ram & 0x0f]
                     : mega_distance_to_c64(opts, r, g, b, color_ram & 0x0f);
    if (d < best_dist) {
        best_pixel = 3;
    }

    return best_pixel;
}

// Convert Chocolate Doom's indexed 320x200 framebuffer into the shared
// Koala-style C64 bitmap/screen/color frame buffer.
void Sysop_ImageConvertMegaIndexed(const uint8_t *indexed_screen,
                                   const uint8_t palette[256][3],
                                   const uint8_t palette_locks[256],
                                   int menu_dither_mode)
{
    static int prev_color_occurrence[16];
    static int first_bg_guess = 1;
    SysopMegaOptions opts;
    uint8_t tone_lut[256];
    uint8_t nearest_by_color[16 * 16];
    const uint8_t *active_nearest_by_color;
    const int (*mega_palette)[3];

    if (indexed_screen == NULL || palette == NULL) {
        return;
    }

    g_sysop_image_menu_dither_mode = menu_dither_mode;
    Sysop_ImageMegaGetOptions(&opts);
    clamp_mega_options(&opts);
    mega_palette = mega_palette_for_options(&opts);

    if (opts.fast_tables) {
        mega_prepare_cached_tables(&opts, palette, mega_palette);
        active_nearest_by_color = mega_cached_nearest_by_color;
    } else {
        build_tone_lut(&opts, tone_lut);
        build_mega_nearest_table(&opts, nearest_by_color);
        active_nearest_by_color = nearest_by_color;
    }

    if (opts.background_color >= 0) {
        g_sysop_image_bg_color = opts.background_color & 0x0f;
    } else if (first_bg_guess) {
        first_bg_guess = 0;
        g_sysop_image_bg_color = 0;
        memset(prev_color_occurrence, 0, sizeof(prev_color_occurrence));
    } else {
        int best = g_sysop_image_bg_color;
        for (int i = 0; i < 16; ++i) {
            if (prev_color_occurrence[i] > prev_color_occurrence[best]) {
                best = i;
            }
        }

        if (best != g_sysop_image_bg_color
            && prev_color_occurrence[best] > prev_color_occurrence[g_sysop_image_bg_color] + 12) {
            g_sysop_image_bg_color = best;
        }
    }

    int frame_color_occurrence[16];
    memset(frame_color_occurrence, 0, sizeof(frame_color_occurrence));

    for (int y = 0; y < SCREENHEIGHT; y += 8) {
        for (int x = 0; x < SCREENWIDTH; x += 8) {
            uint8_t color_histogram[16];
            MegaSample samples[32];
            int sample_dist[32][16];
            int luma_sum = 0;
            int luma_count = 0;
            int luma_min = 255;
            int luma_max = 0;

            memset(color_histogram, 0, sizeof(color_histogram));

            for (int c = 0; c < 8; ++c) {
                for (int a = 0; a < 8; a += 2) {
                    int idx = (a >> 1) + c * 4;
                    int p = (x + a) + (y + c) * SCREENWIDTH;
                    uint8_t d0 = indexed_screen[p];
                    uint8_t d1 = indexed_screen[p + 1];
                    MegaSample *sample = &samples[idx];
                    memset(sample, 0, sizeof(*sample));

                    if (palette_locks != NULL) {
                        sample->lockedColor = Sysop_ImageSelectLockedPairColor(palette_locks[d0], palette_locks[d1]);
                    }

                    if (sample->lockedColor) {
                        sample->locked = 1;
                        sample->r = mega_palette[sample->lockedColor][0];
                        sample->g = mega_palette[sample->lockedColor][1];
                        sample->b = mega_palette[sample->lockedColor][2];
                    } else if (opts.fast_tables) {
                        const MegaPairSample *pair = &mega_cached_pair_table[(d0 << 8) | d1];
                        sample->r = pair->r;
                        sample->g = pair->g;
                        sample->b = pair->b;
                        sample->lum = pair->lum;
                    } else {
                        int r = (palette[d0][0] + palette[d1][0]) >> 1;
                        int g = (palette[d0][1] + palette[d1][1]) >> 1;
                        int b = (palette[d0][2] + palette[d1][2]) >> 1;
                        apply_color_controls(&opts, tone_lut, &r, &g, &b);
                        sample->r = r;
                        sample->g = g;
                        sample->b = b;
                    }

                    if (!opts.fast_tables || sample->locked) {
                        sample->lum = mega_luma(sample->r, sample->g, sample->b);
                    }
                    if (!sample->locked) {
                        luma_sum += sample->lum;
                        ++luma_count;
                        if (sample->lum < luma_min) luma_min = sample->lum;
                        if (sample->lum > luma_max) luma_max = sample->lum;
                    }
                }
            }

            int luma_mean = luma_count > 0 ? (luma_sum + luma_count / 2) / luma_count : 0;
            int luma_range = luma_max - luma_min;

            for (int c = 0; c < 8; ++c) {
                for (int a = 0; a < 8; a += 2) {
                    int idx = (a >> 1) + c * 4;
                    int mc_x = (x >> 1) + (a >> 1);
                    MegaSample *sample = &samples[idx];

                    if (sample->locked) {
                        sample->targetR = (uint8_t)sample->r;
                        sample->targetG = (uint8_t)sample->g;
                        sample->targetB = (uint8_t)sample->b;
                        color_histogram[sample->lockedColor] += 8;
                    } else {
                        int detail = ((sample->lum - luma_mean) * opts.detail_pop) / 100;
                        int surface = 0;

                        if (luma_range > 0) {
                            surface = ((sample->lum - luma_mean) * opts.surface_detail) / 160;
                        }

                        sample->targetR = clamp_u8(sample->r + detail + surface);
                        sample->targetG = clamp_u8(sample->g + detail + surface);
                        sample->targetB = clamp_u8(sample->b + detail + surface);

                        int histogram_noise = (opts.fast_tables ? mega_cached_noise_at(mc_x, y + c)
                                                                : mega_dither_noise(&opts, mc_x, y + c)) / 2;
                        int nearest = opts.fast_tables
                                      ? nearest_mega_color_cached(&opts,
                                                                  clamp_u8(sample->targetR + histogram_noise),
                                                                  clamp_u8(sample->targetG + histogram_noise),
                                                                  clamp_u8(sample->targetB + histogram_noise))
                                      : nearest_mega_color(&opts,
                                                           clamp_u8(sample->targetR + histogram_noise),
                                                           clamp_u8(sample->targetG + histogram_noise),
                                                           clamp_u8(sample->targetB + histogram_noise));
                        int weight = 1 + (abs(sample->lum - luma_mean) * opts.surface_detail) / 2048;
                        color_histogram[nearest] = clamp_u8(color_histogram[nearest] + weight);
                    }

                    if (opts.fast_tables) {
                        const int *dist = mega_distances_for_rgb(&opts,
                                                                 sample->targetR,
                                                                 sample->targetG,
                                                                 sample->targetB);
                        memcpy(sample_dist[idx], dist, sizeof(sample_dist[idx]));
                    } else {
                        for (int color = 0; color < 16; ++color) {
                            sample_dist[idx][color] = mega_distance_to_c64(&opts,
                                                                           sample->targetR,
                                                                           sample->targetG,
                                                                           sample->targetB,
                                                                           color);
                        }
                    }
                }
            }

            for (int i = 0; i < 16; ++i) {
                if (color_histogram[i]) {
                    frame_color_occurrence[i]++;
                }
            }

            int cell = (y / 8) * 40 + (x / 8);
            uint8_t prev_screen = sysop_c64_frame[8000 + cell];
            uint8_t prev_color = sysop_c64_frame[9000 + cell];
            uint8_t candidates[10];
            int candidate_count = 0;
            build_mega_palette_candidates(&opts, candidates, &candidate_count, color_histogram,
                                          active_nearest_by_color, prev_screen, prev_color,
                                          g_sysop_image_bg_color);

            uint8_t best0 = candidates[0], best1 = candidates[1], best2 = candidates[2];
            int best_score = INT_MAX;

            for (int i = 0; i < candidate_count - 2; ++i) {
                for (int j = i + 1; j < candidate_count - 1; ++j) {
                    for (int k = j + 1; k < candidate_count; ++k) {
                        int score = Sysop_ImageScorePaletteSet(sample_dist,
                                                               g_sysop_image_bg_color,
                                                               candidates[i],
                                                               candidates[j],
                                                               candidates[k]);
                        if (score < best_score) {
                            best_score = score;
                            best0 = candidates[i];
                            best1 = candidates[j];
                            best2 = candidates[k];
                        }
                    }
                }
            }

            uint8_t screen_ram, color_ram;
            Sysop_ImageChoosePaletteRoles(best0, best1, best2,
                                          prev_screen, prev_color,
                                          &screen_ram, &color_ram);

            sysop_c64_frame[8000 + cell] = screen_ram;
            sysop_c64_frame[9000 + cell] = color_ram;

            for (int c = 0; c < 8; ++c) {
                uint8_t bitmap_data = 0;

                for (int a = 0; a < 4; ++a) {
                    int idx = a + c * 4;
                    uint8_t pixel_value = mega_nearest_pixel_for_palette(&opts,
                                                                         &samples[idx],
                                                                         (x >> 1) + a,
                                                                         y + c,
                                                                         screen_ram,
                                                                         color_ram);
                    bitmap_data <<= 2;
                    bitmap_data |= pixel_value;
                }

                sysop_c64_frame[y * 40 + x + c] = bitmap_data;
            }
        }
    }

    memcpy(prev_color_occurrence, frame_color_occurrence, sizeof(prev_color_occurrence));
    sysop_c64_frame[10000] = g_sysop_image_bg_color;
}
