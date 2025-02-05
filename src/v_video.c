//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 1993-2008 Raven Software
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
// DESCRIPTION:
//	Gamma correction LUT stuff.
//	Functions to draw patches (by post) directly to screen.
//	Functions to blit a block to the screen.
//

#include "SDL_version.h" // [crispy]

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "i_system.h"

#include "doomtype.h"

#include "deh_str.h"
#include "i_input.h"
#include "i_swap.h"
#include "i_video.h"
#include "m_bbox.h"
#include "m_misc.h"
#ifdef CRISPY_TRUECOLOR
#include "v_trans.h"
#endif
#include "v_video.h"
#include "w_wad.h"
#include "z_zone.h"
#include "crispy.h"

#include "config.h"
#ifdef HAVE_LIBPNG
#include <png.h>
#endif

// TODO: There are separate RANGECHECK defines for different games, but this
// is common code. Fix this.
#define RANGECHECK

// Blending table used for fuzzpatch, etc.
// Only used in Heretic/Hexen

byte *tinttable = NULL;
byte *tranmap = NULL;
byte *dp_translation = NULL;
boolean dp_translucent = false;
#ifdef CRISPY_TRUECOLOR
extern pixel_t *pal_color;
#endif

// villsa [STRIFE] Blending table used for Strife
byte *xlatab = NULL;

// The screen buffer that the v_video.c code draws to.

static pixel_t *dest_screen = NULL;

int dirtybox[4]; 

// haleyjd 08/28/10: clipping callback function for patches.
// This is needed for Chocolate Strife, which clips patches to the screen.
static vpatchclipfunc_t patchclip_callback = NULL;

//
// V_MarkRect 
// 
void V_MarkRect(int x, int y, int width, int height) 
{ 
    // If we are temporarily using an alternate screen, do not 
    // affect the update box.

    if (dest_screen == I_VideoBuffer)
    {
        M_AddToBox (dirtybox, x, y); 
        M_AddToBox (dirtybox, x + width-1, y + height-1); 
    }
} 
 

//
// V_CopyRect 
// 
void V_CopyRect(int srcx, int srcy, pixel_t *source,
                int width, int height,
                int destx, int desty)
{ 
    pixel_t *src;
    pixel_t *dest;
 
    srcx <<= crispy->hires;
    srcy <<= crispy->hires;
    width <<= crispy->hires;
    height <<= crispy->hires;
    destx <<= crispy->hires;
    desty <<= crispy->hires;

#ifdef RANGECHECK 
    if (srcx < 0
     || srcx + width > SCREENWIDTH
     || srcy < 0
     || srcy + height > SCREENHEIGHT 
     || destx < 0
     || destx /* + width */ > SCREENWIDTH
     || desty < 0
     || desty /* + height */ > SCREENHEIGHT)
    {
        I_Error ("Bad V_CopyRect");
    }
#endif 

    // [crispy] prevent framebuffer overflow
    if (destx + width > SCREENWIDTH)
	width = SCREENWIDTH - destx;
    if (desty + height > SCREENHEIGHT)
	height = SCREENHEIGHT - desty;

    V_MarkRect(destx, desty, width, height); 
 
    src = source + SCREENWIDTH * srcy + srcx; 
    dest = dest_screen + SCREENWIDTH * desty + destx; 

    for ( ; height>0 ; height--) 
    { 
        memcpy(dest, src, width * sizeof(*dest));
        src += SCREENWIDTH; 
        dest += SCREENWIDTH; 
    } 
} 
 
//
// V_SetPatchClipCallback
//
// haleyjd 08/28/10: Added for Strife support.
// By calling this function, you can setup runtime error checking for patch 
// clipping. Strife never caused errors by drawing patches partway off-screen.
// Some versions of vanilla DOOM also behaved differently than the default
// implementation, so this could possibly be extended to those as well for
// accurate emulation.
//
void V_SetPatchClipCallback(vpatchclipfunc_t func)
{
    patchclip_callback = func;
}

//
// V_DrawPatch
// Masks a column based masked pic to the screen. 
//

// [crispy] four different rendering functions
// for each possible combination of dp_translation and dp_translucent:
// (1) normal, opaque patch
static const inline pixel_t drawpatchpx00 (const pixel_t dest, const pixel_t source)
#ifndef CRISPY_TRUECOLOR
{return source;}
#else
{return pal_color[source];}
#endif
// (2) color-translated, opaque patch
static const inline pixel_t drawpatchpx01 (const pixel_t dest, const pixel_t source)
#ifndef CRISPY_TRUECOLOR
{return dp_translation[source];}
#else
{return pal_color[dp_translation[source]];}
#endif
// (3) normal, translucent patch
static const inline pixel_t drawpatchpx10 (const pixel_t dest, const pixel_t source)
#ifndef CRISPY_TRUECOLOR
{return tranmap[(dest<<8)+source];}
#else
{return I_BlendOverTranmap(dest, pal_color[source]);}
#endif
// (4) color-translated, translucent patch
static const inline pixel_t drawpatchpx11 (const pixel_t dest, const pixel_t source)
#ifndef CRISPY_TRUECOLOR
{return tranmap[(dest<<8)+dp_translation[source]];}
#else
{return I_BlendOverTranmap(dest, pal_color[dp_translation[source]]);}
#endif

// [crispy] TINTTAB rendering functions:
// V_DrawShadowedPatch (shadow only)
static const inline pixel_t drawshadow (const pixel_t dest, const pixel_t source)
#ifndef CRISPY_TRUECOLOR
{return tinttable[(dest<<8)];}
#else
{return I_BlendDark(dest, 0xa0);} // 160 (62.75%) of 256 full translucency
#endif
// V_DrawTLPatch (translucent patch, no coloring or color-translation are used)
static const inline pixel_t drawtinttab (const pixel_t dest, const pixel_t source)
#ifndef CRISPY_TRUECOLOR
{return tinttable[dest+(source<<8)];}
#else
{return I_BlendOverTinttab(dest, pal_color[source]);}
#endif
// V_DrawTLPatch Translated Option (translucent patch, color-translated)
static const inline pixel_t drawtrtinttab (const pixel_t dest, const pixel_t source)
#ifndef CRISPY_TRUECOLOR
{return tinttable[dest+(dp_translation[source]<<8)];}
#else
{return I_BlendOverTinttab(dest, pal_color[dp_translation[source]]);}
#endif
// V_DrawAltTLPatch (translucent patch, no coloring or color-translation are used)
static const inline pixel_t drawalttinttab (const pixel_t dest, const pixel_t source)
#ifndef CRISPY_TRUECOLOR
{return tinttable[(dest<<8)+source];}
#else
{return I_BlendOverAltTinttab(dest, pal_color[source]);}
#endif
// V_DrawXlaPatch (translucent patch, no coloring or color-translation are used)
static const inline pixel_t drawxlatab (const pixel_t dest, const pixel_t source)
#ifndef CRISPY_TRUECOLOR
{return xlatab[dest+(source<<8)];}
#else
{return I_BlendOverXlatab(dest, pal_color[source]);}
#endif

// [crispy] array of function pointers holding the different rendering functions
typedef const pixel_t drawpatchpx_t (const pixel_t dest, const pixel_t source);
static drawpatchpx_t *const drawpatchpx_a[2][2] = {{drawpatchpx11, drawpatchpx10}, {drawpatchpx01, drawpatchpx00}};
static drawpatchpx_t *const drawtlpatchpx_a[2] = {drawtrtinttab, drawtinttab};

static fixed_t dx, dxi, dy, dyi;

void V_DrawPatch(int x, int y, patch_t *patch)
{ 
    int count;
    int col;
    column_t *column;
    pixel_t *desttop;
    pixel_t *dest;
    byte *source;
    int w;

    // [crispy] four different rendering functions
    drawpatchpx_t *const drawpatchpx = drawpatchpx_a[!dp_translucent][!dp_translation];

    y -= SHORT(patch->topoffset);
    x -= SHORT(patch->leftoffset);
    x += WIDESCREENDELTA; // [crispy] horizontal widescreen offset

/*
    // haleyjd 08/28/10: Strife needs silent error checking here.
    if(patchclip_callback)
    {
        if(!patchclip_callback(patch, x, y))
            return;
    }
*/

#ifdef RANGECHECK_NOTHANKS
    if (x < 0
     || x + SHORT(patch->width) > ORIGWIDTH
     || y < 0
     || y + SHORT(patch->height) > ORIGHEIGHT)
    {
        I_Error("Bad V_DrawPatch");
    }
#endif

    V_MarkRect(x, y, SHORT(patch->width), SHORT(patch->height));

    col = 0;
    if (x < 0)
    {
	col += dxi * ((-x * dx) >> FRACBITS);
	x = 0;
    }

    desttop = dest_screen + ((y * dy) >> FRACBITS) * SCREENWIDTH + ((x * dx) >> FRACBITS);

    w = SHORT(patch->width);

    // convert x to screen position
    x = (x * dx) >> FRACBITS;

    for ( ; col<w << FRACBITS ; x++, col+=dxi, desttop++)
    {
        int topdelta = -1;

        // [crispy] too far right / width
        if (x >= SCREENWIDTH)
        {
            break;
        }

        column = (column_t *)((byte *)patch + LONG(patch->columnofs[col >> FRACBITS]));

        // step through the posts in a column
        while (column->topdelta != 0xff)
        {
            int top, srccol = 0;
            // [crispy] support for DeePsea tall patches
            if (column->topdelta <= topdelta)
            {
                topdelta += column->topdelta;
            }
            else
            {
                topdelta = column->topdelta;
            }
            top = ((y + topdelta) * dy) >> FRACBITS;
            source = (byte *)column + 3;
            dest = desttop + ((topdelta * dy) >> FRACBITS)*SCREENWIDTH;
            count = (column->length * dy) >> FRACBITS;

            // [crispy] too low / height
            if (top + count > SCREENHEIGHT)
            {
                count = SCREENHEIGHT - top;
            }

            // [crispy] nothing left to draw?
            if (count < 1)
            {
                break;
            }

            while (count--)
            {
                // [crispy] too high
                if (top++ >= 0)
                {
                    *dest = drawpatchpx(*dest, source[srccol >> FRACBITS]);
                }
                srccol += dyi;
                dest += SCREENWIDTH;
            }
            column = (column_t *)((byte *)column + column->length + 4);
        }
    }
}

void V_DrawPatchFullScreen(patch_t *patch, boolean flipped)
{
    int x = ((SCREENWIDTH >> crispy->hires) - SHORT(patch->width)) / 2 - WIDESCREENDELTA;
    static int black = -1;

    patch->leftoffset = 0;
    patch->topoffset = 0;

    if (black == -1)
    {
#ifndef CRISPY_TRUECOLOR
        black = I_GetPaletteIndex(0x00, 0x00, 0x00);
#else
        black = I_MapRGB(0x00, 0x00, 0x00);
#endif
    }

    // [crispy] fill pillarboxes in widescreen mode
    if (SCREENWIDTH != NONWIDEWIDTH)
    {
        V_DrawFilledBox(0, 0, SCREENWIDTH, SCREENHEIGHT, black);
    }

    if (flipped)
    {
        V_DrawPatchFlipped(x, 0, patch);
    }
    else
    {
        V_DrawPatch(x, 0, patch);
    }
}

//
// V_DrawPatchFlipped
// Masks a column based masked pic to the screen.
// Flips horizontally, e.g. to mirror face.
//

void V_DrawPatchFlipped(int x, int y, patch_t *patch)
{
    int count;
    int col; 
    column_t *column; 
    pixel_t *desttop;
    pixel_t *dest;
    byte *source; 
    int w; 
 
    y -= SHORT(patch->topoffset); 
    x -= SHORT(patch->leftoffset); 
    x += WIDESCREENDELTA; // [crispy] horizontal widescreen offset

/*
    // haleyjd 08/28/10: Strife needs silent error checking here.
    if(patchclip_callback)
    {
        if(!patchclip_callback(patch, x, y))
            return;
    }
*/

#ifdef RANGECHECK_NOTHANKS
    if (x < 0
     || x + SHORT(patch->width) > ORIGWIDTH
     || y < 0
     || y + SHORT(patch->height) > ORIGHEIGHT)
    {
        I_Error("Bad V_DrawPatchFlipped");
    }
#endif

    V_MarkRect (x, y, SHORT(patch->width), SHORT(patch->height));

    col = 0;
    if (x < 0)
    {
	col += dxi * ((-x * dx) >> FRACBITS);
	x = 0;
    }

    desttop = dest_screen + ((y * dy) >> FRACBITS) * SCREENWIDTH + ((x * dx) >> FRACBITS);

    w = SHORT(patch->width);

    // convert x to screen position
    x = (x * dx) >> FRACBITS;

    for ( ; col<w << FRACBITS ; x++, col+=dxi, desttop++)
    {
        int topdelta = -1;

        // [crispy] too far left
        if (x < 0)
        {
            continue;
        }

        // [crispy] too far right / width
        if (x >= SCREENWIDTH)
        {
            break;
        }

        column = (column_t *)((byte *)patch + LONG(patch->columnofs[w-1-(col >> FRACBITS)]));

        // step through the posts in a column
        while (column->topdelta != 0xff )
        {
            int top, srccol = 0;
            // [crispy] support for DeePsea tall patches
            if (column->topdelta <= topdelta)
            {
                topdelta += column->topdelta;
            }
            else
            {
                topdelta = column->topdelta;
            }
            top = ((y + topdelta) * dy) >> FRACBITS;
            source = (byte *)column + 3;
            dest = desttop + ((topdelta * dy) >> FRACBITS)*SCREENWIDTH;
            count = (column->length * dy) >> FRACBITS;

            // [crispy] too low / height
            if (top + count > SCREENHEIGHT)
            {
                count = SCREENHEIGHT - top;
            }

            // [crispy] nothing left to draw?
            if (count < 1)
            {
                break;
            }

            while (count--)
            {
                // [crispy] too high
                if (top++ >= 0)
                {
#ifndef CRISPY_TRUECOLOR
                    *dest = source[srccol >> FRACBITS];
#else
                    *dest = pal_color[source[srccol >> FRACBITS]];
#endif
                }
                srccol += dyi;
                dest += SCREENWIDTH;
            }
            column = (column_t *)((byte *)column + column->length + 4);
        }
    }
}



//
// V_DrawPatchDirect
// Draws directly to the screen on the pc. 
//

void V_DrawPatchDirect(int x, int y, patch_t *patch)
{
    V_DrawPatch(x, y, patch); 
} 

//
// V_DrawTLPatch
//
// Masks a column based translucent masked pic to the screen.
//

void V_DrawTLPatch(int x, int y, patch_t * patch)
{
    int count, col;
    column_t *column;
    pixel_t *desttop, *dest;
    byte *source;
    int w;

    // [crispy] translucent patch with optional color-translation used
    drawpatchpx_t *const drawpatchpx = drawtlpatchpx_a[!dp_translation];

    y -= SHORT(patch->topoffset);
    x -= SHORT(patch->leftoffset);
    x += WIDESCREENDELTA; // [crispy] horizontal widescreen offset

    if (x < 0
     || x + SHORT(patch->width) > (SCREENWIDTH >> crispy->hires)
     || y < 0
     || y + SHORT(patch->height) > (SCREENHEIGHT >> crispy->hires))
    {
        I_Error("Bad V_DrawTLPatch");
    }

    col = 0;
    desttop = dest_screen + ((y * dy) >> FRACBITS) * SCREENWIDTH + ((x * dx) >> FRACBITS);

    w = SHORT(patch->width);
    for (; col < w << FRACBITS; x++, col+=dxi, desttop++)
    {
        column = (column_t *) ((byte *) patch + LONG(patch->columnofs[col >> FRACBITS]));

        // step through the posts in a column

        while (column->topdelta != 0xff)
        {
            int srccol = 0;
            source = (byte *) column + 3;
            dest = desttop + ((column->topdelta * dy) >> FRACBITS) * SCREENWIDTH;
            count = (column->length * dy) >> FRACBITS;

            while (count--)
            {
                *dest = drawpatchpx(*dest, source[srccol >> FRACBITS]);
                srccol += dyi;
                dest += SCREENWIDTH;
            }
            column = (column_t *) ((byte *) column + column->length + 4);
        }
    }
}

//
// V_DrawXlaPatch
//
// villsa [STRIFE] Masks a column based translucent masked pic to the screen.
//

void V_DrawXlaPatch(int x, int y, patch_t * patch)
{
    int count, col;
    column_t *column;
    pixel_t *desttop, *dest;
    byte *source;
    int w;

    // [crispy] translucent patch, no coloring or color-translation are used
    drawpatchpx_t *const drawpatchpx = drawxlatab;

    y -= SHORT(patch->topoffset);
    x -= SHORT(patch->leftoffset);
    x += WIDESCREENDELTA; // [crispy] horizontal widescreen offset

/*
    if(patchclip_callback)
    {
        if(!patchclip_callback(patch, x, y))
            return;
    }
*/

    col = 0;
    desttop = dest_screen + ((y * dy) >> FRACBITS) * SCREENWIDTH + ((x * dx) >> FRACBITS);

    w = SHORT(patch->width);
    for(; col < w << FRACBITS; x++, col+=dxi, desttop++)
    {
        column = (column_t *) ((byte *) patch + LONG(patch->columnofs[col >> FRACBITS]));

        // step through the posts in a column

        while(column->topdelta != 0xff)
        {
            int srccol = 0;
            source = (byte *) column + 3;
            dest = desttop + ((column->topdelta * dy) >> FRACBITS) * SCREENWIDTH;
            count = (column->length * dy) >> FRACBITS;

            while(count--)
            {
                *dest = drawpatchpx(*dest, source[srccol >> FRACBITS]);
                srccol += dyi;
                dest += SCREENWIDTH;
            }
            column = (column_t *) ((byte *) column + column->length + 4);
        }
    }
}

//
// V_DrawAltTLPatch
//
// Masks a column based translucent masked pic to the screen.
//

void V_DrawAltTLPatch(int x, int y, patch_t * patch)
{
    int count, col;
    column_t *column;
    pixel_t *desttop, *dest;
    byte *source;
    int w;

    // [crispy] translucent patch, no coloring or color-translation are used
    drawpatchpx_t *const drawpatchpx = drawalttinttab;

    y -= SHORT(patch->topoffset);
    x -= SHORT(patch->leftoffset);
    x += WIDESCREENDELTA; // [crispy] horizontal widescreen offset

    if (x < 0
     || x + SHORT(patch->width) > (SCREENWIDTH >> crispy->hires)
     || y < 0
     || y + SHORT(patch->height) > (SCREENHEIGHT >> crispy->hires))
    {
        I_Error("Bad V_DrawAltTLPatch");
    }

    col = 0;
    desttop = dest_screen + ((y * dy) >> FRACBITS) * SCREENWIDTH + ((x * dx) >> FRACBITS);

    w = SHORT(patch->width);
    for (; col < w << FRACBITS; x++, col+=dxi, desttop++)
    {
        column = (column_t *) ((byte *) patch + LONG(patch->columnofs[col >> FRACBITS]));

        // step through the posts in a column

        while (column->topdelta != 0xff)
        {
            int srccol = 0;
            source = (byte *) column + 3;
            dest = desttop + ((column->topdelta * dy) >> FRACBITS) * SCREENWIDTH;
            count = (column->length * dy) >> FRACBITS;

            while (count--)
            {
                *dest = drawpatchpx(*dest, source[srccol >> FRACBITS]);
                srccol += dyi;
                dest += SCREENWIDTH;
            }
            column = (column_t *) ((byte *) column + column->length + 4);
        }
    }
}

//
// V_DrawShadowedPatch
//
// Masks a column based masked pic to the screen.
//

void V_DrawShadowedPatch(int x, int y, patch_t *patch)
{
    int count, col;
    column_t *column;
    pixel_t *desttop, *dest;
    byte *source;
    pixel_t *desttop2, *dest2;
    int w;

    // [crispy] four different rendering functions
    drawpatchpx_t *const drawpatchpx = drawpatchpx_a[!dp_translucent][!dp_translation];
    // [crispy] shadow, no coloring or color-translation are used
    drawpatchpx_t *const drawpatchpx2 = drawshadow;

    y -= SHORT(patch->topoffset);
    x -= SHORT(patch->leftoffset);
    x += WIDESCREENDELTA; // [crispy] horizontal widescreen offset

    if (x < 0
     || x + SHORT(patch->width) > (SCREENWIDTH >> crispy->hires)
     || y < 0
     || y + SHORT(patch->height) > (SCREENHEIGHT >> crispy->hires))
    {
        I_Error("Bad V_DrawShadowedPatch");
    }

    col = 0;
    desttop = dest_screen + ((y * dy) >> FRACBITS) * SCREENWIDTH + ((x * dx) >> FRACBITS);
    desttop2 = dest_screen + (((y + 2) * dy) >> FRACBITS) * SCREENWIDTH + (((x + 2) * dx) >> FRACBITS);

    w = SHORT(patch->width);
    for (; col < w << FRACBITS; x++, col+=dxi, desttop++, desttop2++)
    {
        column = (column_t *) ((byte *) patch + LONG(patch->columnofs[col >> FRACBITS]));

        // step through the posts in a column

        while (column->topdelta != 0xff)
        {
            int srccol = 0;
            source = (byte *) column + 3;
            dest = desttop + ((column->topdelta * dy) >> FRACBITS) * SCREENWIDTH;
            dest2 = desttop2 + ((column->topdelta * dy) >> FRACBITS) * SCREENWIDTH;
            count = (column->length * dy) >> FRACBITS;

            while (count--)
            {
                *dest2 = drawpatchpx2(*dest2, source[srccol >> FRACBITS]);
                dest2 += SCREENWIDTH;
                *dest = drawpatchpx(*dest, source[srccol >> FRACBITS]);
                srccol += dyi;
                dest += SCREENWIDTH;

            }
            column = (column_t *) ((byte *) column + column->length + 4);
        }
    }
}

//
// Load tint table from TINTTAB lump.
//

void V_LoadTintTable(void)
{
    tinttable = W_CacheLumpName("TINTTAB", PU_STATIC);
}

//
// V_LoadXlaTable
//
// villsa [STRIFE] Load xla table from XLATAB lump.
//

void V_LoadXlaTable(void)
{
    xlatab = W_CacheLumpName("XLATAB", PU_STATIC);
}

//
// V_DrawBlock
// Draw a linear block of pixels into the view buffer.
//

void V_DrawBlock(int x, int y, int width, int height, pixel_t *src)
{ 
    pixel_t *dest;
 
#ifdef RANGECHECK 
    if (x < 0
     || x + width >SCREENWIDTH
     || y < 0
     || y + height > SCREENHEIGHT)
    {
	I_Error ("Bad V_DrawBlock");
    }
#endif 
 
    V_MarkRect (x, y, width, height); 
 
    dest = dest_screen + (y << crispy->hires) * SCREENWIDTH + x;

    while (height--) 
    { 
	memcpy (dest, src, width * sizeof(*dest));
	src += width; 
	dest += SCREENWIDTH; 
    } 
} 

// [crispy] scaled version of V_DrawBlock()
void V_DrawScaledBlock(int x, int y, int width, int height, byte *src)
{
    pixel_t *dest;
    int i, j;

    x += WIDESCREENDELTA; // [crispy] horizontal widescreen offset

#ifdef RANGECHECK
    if (x < 0
     || x + width > SCREENWIDTH
     || y < 0
     || y + height > SCREENWIDTH)
    {
	I_Error ("Bad V_DrawScaledBlock");
    }
#endif

    V_MarkRect (x, y, width, height);

    dest = dest_screen + (y << crispy->hires) * SCREENWIDTH + (x << crispy->hires);

    for (i = 0; i < (height << crispy->hires); i++)
    {
        for (j = 0; j < (width << crispy->hires); j++)
        {
#ifndef CRISPY_TRUECOLOR
            *(dest + i * SCREENWIDTH + j) = *(src + (i >> crispy->hires) * width + (j >> crispy->hires));
#else
            *(dest + i * SCREENWIDTH + j) = pal_color[*(src + (i >> crispy->hires) * width + (j >> crispy->hires))];
#endif
        }
    }
}

void V_DrawFilledBox(int x, int y, int w, int h, int c)
{
    pixel_t *buf, *buf1;
    int x1, y1;

    buf = I_VideoBuffer + SCREENWIDTH * y + x;

    for (y1 = 0; y1 < h; ++y1)
    {
        buf1 = buf;

        for (x1 = 0; x1 < w; ++x1)
        {
            *buf1++ = c;
        }

        buf += SCREENWIDTH;
    }
}

void V_DrawHorizLine(int x, int y, int w, int c)
{
    pixel_t *buf;
    int x1;

    // [crispy] prevent framebuffer overflows
    if (x + w > (unsigned)SCREENWIDTH)
	w = SCREENWIDTH - x;

    buf = I_VideoBuffer + SCREENWIDTH * y + x;

    for (x1 = 0; x1 < w; ++x1)
    {
        *buf++ = c;
    }
}

void V_DrawVertLine(int x, int y, int h, int c)
{
    pixel_t *buf;
    int y1;

    buf = I_VideoBuffer + SCREENWIDTH * y + x;

    for (y1 = 0; y1 < h; ++y1)
    {
        *buf = c;
        buf += SCREENWIDTH;
    }
}

void V_DrawBox(int x, int y, int w, int h, int c)
{
    V_DrawHorizLine(x, y, w, c);
    V_DrawHorizLine(x, y+h-1, w, c);
    V_DrawVertLine(x, y, h, c);
    V_DrawVertLine(x+w-1, y, h, c);
}

//
// Draw a "raw" screen (lump containing raw data to blit directly
// to the screen)
//

void V_CopyScaledBuffer(pixel_t *dest, byte *src, size_t size)
{
    int i, j, index;

#ifdef RANGECHECK
    if (size > ORIGWIDTH * ORIGHEIGHT)
    {
        I_Error("Bad V_CopyScaledBuffer");
    }
#endif

    // [crispy] Fill pillarboxes in widescreen mode. Needs to be two separate
    // pillars to allow for Heretic finale vertical scrolling.
    if (SCREENWIDTH != NONWIDEWIDTH)
    {
        V_DrawFilledBox(0, 0, WIDESCREENDELTA << crispy->hires, SCREENHEIGHT, 0);
        V_DrawFilledBox(SCREENWIDTH - (WIDESCREENDELTA << crispy->hires), 0,
                        WIDESCREENDELTA << crispy->hires, SCREENHEIGHT, 0);
    }

    index = ((size / ORIGWIDTH) << crispy->hires) * SCREENWIDTH - 1;

    if (size % ORIGWIDTH)
    {
        // [crispy] Handles starting in the middle of a row.
        index += ((size % ORIGWIDTH) + WIDESCREENDELTA) << crispy->hires;
    }
    else
    {
        index -= WIDESCREENDELTA << crispy->hires;
    }

    while (size--)
    {
        for (i = 0; i <= crispy->hires; i++)
        {
            for (j = 0; j <= crispy->hires; j++)
            {
#ifndef CRISPY_TRUECOLOR
                *(dest + index - (j * SCREENWIDTH) - i) = *(src + size);
#else
                *(dest + index - (j * SCREENWIDTH) - i) = pal_color[src[size]];
#endif
            }
        }
        if (size % ORIGWIDTH == 0)
        {
            index -= 2 * (WIDESCREENDELTA << crispy->hires)
                     + crispy->hires * SCREENWIDTH;
        }
        index -= 1 + crispy->hires;
    }
}
 
void V_DrawRawScreen(byte *raw)
{
    V_CopyScaledBuffer(dest_screen, raw, ORIGWIDTH * ORIGHEIGHT);
}

// [crispy] For Heretic and Hexen widescreen support of replacement TITLE,
// HELP1, etc. These lumps are normally 320 x 200 raw graphics. If the lump
// size is larger than expected, proceed as if it were a patch.
void V_DrawFullscreenRawOrPatch(lumpindex_t index)
{
    patch_t *patch;

    patch = W_CacheLumpNum(index, PU_CACHE);

    if (W_LumpLength(index) == ORIGWIDTH * ORIGHEIGHT)
    {
        V_DrawRawScreen((byte*)patch);
    }
    else if ((SHORT(patch->height) == 200) && (SHORT(patch->width) >= 320))
    {
        V_DrawPatchFullScreen(patch, false);
    }
    else
    {
        I_Error("Invalid fullscreen graphic.");
    }
}

// [crispy] Unified function of flat filling. Used for intermission
// and finale screens, view border and status bar's wide screen mode.
void V_FillFlat(int y_start, int y_stop, int x_start, int x_stop,
                const byte *src, pixel_t *dest)
{
    int x, y;

    for (y = y_start; y < y_stop; y++)
    {
        for (x = x_start; x < x_stop; x++)
        {
#ifndef CRISPY_TRUECOLOR
            *dest++ = src[((y & 63) * 64) + (x & 63)];
#else
            *dest++ = pal_color[src[((y & 63) * 64) + (x & 63)]];
#endif
        }
    }
}

//
// V_Init
// 
void V_Init (void) 
{ 
    // [crispy] initialize resolution-agnostic patch drawing
    if (NONWIDEWIDTH && SCREENHEIGHT)
    {
        dx = (NONWIDEWIDTH << FRACBITS) / ORIGWIDTH;
        dxi = (ORIGWIDTH << FRACBITS) / NONWIDEWIDTH;
        dy = (SCREENHEIGHT << FRACBITS) / ORIGHEIGHT;
        dyi = (ORIGHEIGHT << FRACBITS) / SCREENHEIGHT;
    }
    // no-op!
    // There used to be separate screens that could be drawn to; these are
    // now handled in the upper layers.
}

// Set the buffer that the code draws to.

void V_UseBuffer(pixel_t *buffer)
{
    dest_screen = buffer;
}

// Restore screen buffer to the i_video screen buffer.

void V_RestoreBuffer(void)
{
    dest_screen = I_VideoBuffer;
}

//
// SCREEN SHOTS
//

typedef PACKED_STRUCT (
{
    char		manufacturer;
    char		version;
    char		encoding;
    char		bits_per_pixel;

    unsigned short	xmin;
    unsigned short	ymin;
    unsigned short	xmax;
    unsigned short	ymax;
    
    unsigned short	hres;
    unsigned short	vres;

    unsigned char	palette[48];
    
    char		reserved;
    char		color_planes;
    unsigned short	bytes_per_line;
    unsigned short	palette_type;
    
    char		filler[58];
    unsigned char	data;		// unbounded
}) pcx_t;


//
// WritePCXfile
//

void WritePCXfile(char *filename, pixel_t *data,
                  int width, int height,
                  byte *palette)
{
    int		i;
    int		length;
    pcx_t*	pcx;
    byte*	pack;
	
    pcx = Z_Malloc (width*height*2+1000, PU_STATIC, NULL);

    pcx->manufacturer = 0x0a;		// PCX id
    pcx->version = 5;			// 256 color
    pcx->encoding = 1;			// uncompressed
    pcx->bits_per_pixel = 8;		// 256 color
    pcx->xmin = 0;
    pcx->ymin = 0;
    pcx->xmax = SHORT(width-1);
    pcx->ymax = SHORT(height-1);
    pcx->hres = SHORT(1);
    pcx->vres = SHORT(1);
    memset (pcx->palette,0,sizeof(pcx->palette));
    pcx->reserved = 0;                  // PCX spec: reserved byte must be zero
    pcx->color_planes = 1;		// chunky image
    pcx->bytes_per_line = SHORT(width);
    pcx->palette_type = SHORT(2);	// not a grey scale
    memset (pcx->filler,0,sizeof(pcx->filler));

    // pack the image
    pack = &pcx->data;
	
    for (i=0 ; i<width*height ; i++)
    {
	if ( (*data & 0xc0) != 0xc0)
	    *pack++ = *data++;
	else
	{
	    *pack++ = 0xc1;
	    *pack++ = *data++;
	}
    }
    
    // write the palette
    *pack++ = 0x0c;	// palette ID byte
    for (i=0 ; i<768 ; i++)
	*pack++ = *palette++;
    
    // write output file
    length = pack - (byte *)pcx;
    M_WriteFile (filename, pcx, length);

    Z_Free (pcx);
}

#ifdef HAVE_LIBPNG
//
// WritePNGfile
//

static void error_fn(png_structp p, png_const_charp s)
{
    printf("libpng error: %s\n", s);
}

static void warning_fn(png_structp p, png_const_charp s)
{
    printf("libpng warning: %s\n", s);
}

void WritePNGfile(char *filename, pixel_t *data,
                  int width, int height,
                  byte *palette)
{
    png_structp ppng;
    png_infop pinfo;
//  png_colorp pcolor;
    FILE *handle;
    int i, j;
//  int w_factor, h_factor;
    byte *rowbuf;

    extern void I_RenderReadPixels(byte **data, int *w, int *h, int *p);

/*
    if (aspect_ratio_correct == 1)
    {
        // scale up to accommodate aspect ratio correction
        w_factor = 5;
        h_factor = 6;

        width *= w_factor;
        height *= h_factor;
    }
    else
    {
        w_factor = 1;
        h_factor = 1;
    }
*/

    handle = M_fopen(filename, "wb");
    if (!handle)
    {
        return;
    }

    ppng = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL,
                                   error_fn, warning_fn);
    if (!ppng)
    {
        fclose(handle);
        return;
    }

    pinfo = png_create_info_struct(ppng);
    if (!pinfo)
    {
        fclose(handle);
        png_destroy_write_struct(&ppng, NULL);
        return;
    }

    png_init_io(ppng, handle);

    I_RenderReadPixels(&palette, &width, &height, &j);
    rowbuf = palette; // [crispy] pointer abuse!

    png_set_IHDR(ppng, pinfo, width, height,
#if SDL_VERSION_ATLEAST(2, 0, 5)
                 8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE,
#else
                 8, PNG_COLOR_TYPE_RGB_ALPHA, PNG_INTERLACE_NONE,
#endif
                 PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
/*
    png_set_IHDR(ppng, pinfo, width, height,
                 8, PNG_COLOR_TYPE_PALETTE, PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

    pcolor = malloc(sizeof(*pcolor) * 256);
    if (!pcolor)
    {
        fclose(handle);
        png_destroy_write_struct(&ppng, &pinfo);
        return;
    }

    for (i = 0; i < 256; i++)
    {
        pcolor[i].red   = *(palette + 3 * i);
        pcolor[i].green = *(palette + 3 * i + 1);
        pcolor[i].blue  = *(palette + 3 * i + 2);
    }

    png_set_PLTE(ppng, pinfo, pcolor, 256);
    free(pcolor);
*/

    png_write_info(ppng, pinfo);

/*
    rowbuf = malloc(width);

    if (rowbuf)
    {
        for (i = 0; i < SCREENHEIGHT; i++)
        {
            // expand the row 5x
            for (j = 0; j < SCREENWIDTH; j++)
            {
                memset(rowbuf + j * w_factor, *(data + i*SCREENWIDTH + j), w_factor);
            }

            // write the row 6 times
            for (j = 0; j < h_factor; j++)
            {
                png_write_row(ppng, rowbuf);
            }
        }

        free(rowbuf);
    }
*/

    for (i = 0; i < height; i++)
    {
        png_write_row(ppng, rowbuf);
        rowbuf += j;
    }
    free(palette);

    png_write_end(ppng, pinfo);
    png_destroy_write_struct(&ppng, &pinfo);
    fclose(handle);
}
#endif

//
// V_ScreenShot
//

void V_ScreenShot(const char *format)
{
    int i;
    char lbmname[16]; // haleyjd 20110213: BUG FIX - 12 is too small!
    const char *ext;
    
    // find a file name to save it to

#ifdef HAVE_LIBPNG
    if (png_screenshots)
    {
        ext = "png";
    }
    else
#endif
    {
        ext = "pcx";
    }

    for (i=0; i<=9999; i++) // [crispy] increase screenshot filename limit
    {
        M_snprintf(lbmname, sizeof(lbmname), format, i, ext);

        if (!M_FileExists(lbmname))
        {
            break;      // file doesn't exist
        }
    }

    if (i == 10000) // [crispy] increase screenshot filename limit
    {
#ifdef HAVE_LIBPNG
        if (png_screenshots)
        {
            I_Error ("V_ScreenShot: Couldn't create a PNG");
        }
        else
#endif
        {
            I_Error ("V_ScreenShot: Couldn't create a PCX");
        }
    }

#ifdef HAVE_LIBPNG
    if (png_screenshots)
    {
    WritePNGfile(lbmname, I_VideoBuffer,
                 SCREENWIDTH, SCREENHEIGHT,
                 W_CacheLumpName (DEH_String("PLAYPAL"), PU_CACHE));
    }
    else
#endif
    {
    // save the pcx file
    WritePCXfile(lbmname, I_VideoBuffer,
                 SCREENWIDTH, SCREENHEIGHT,
                 W_CacheLumpName (DEH_String("PLAYPAL"), PU_CACHE));
    }
}

#define MOUSE_SPEED_BOX_WIDTH  120
#define MOUSE_SPEED_BOX_HEIGHT 9
#define MOUSE_SPEED_BOX_X (SCREENWIDTH - MOUSE_SPEED_BOX_WIDTH - 10)
#define MOUSE_SPEED_BOX_Y 15

//
// V_DrawMouseSpeedBox
//

static void DrawAcceleratingBox(int speed)
{
    int red, white, yellow;
    int original_speed;
    int redline_x;
    int linelen;

#ifndef CRISPY_TRUECOLOR
    red = I_GetPaletteIndex(0xff, 0x00, 0x00);
    white = I_GetPaletteIndex(0xff, 0xff, 0xff);
    yellow = I_GetPaletteIndex(0xff, 0xff, 0x00);
#else
    red = I_MapRGB(0xff, 0x00, 0x00);
    white = I_MapRGB(0xff, 0xff, 0xff);
    yellow = I_MapRGB(0xff, 0xff, 0x00);
#endif

    // Calculate the position of the red threshold line when calibrating
    // acceleration.  This is 1/3 of the way along the box.

    redline_x = MOUSE_SPEED_BOX_WIDTH / 3;

    if (speed >= mouse_threshold)
    {
        // Undo acceleration and get back the original mouse speed
        original_speed = speed - mouse_threshold;
        original_speed = (int) (original_speed / mouse_acceleration);
        original_speed += mouse_threshold;

        linelen = (original_speed * redline_x) / mouse_threshold;
    }
    else
    {
        linelen = (speed * redline_x) / mouse_threshold;
    }

    // Horizontal "thermometer"
    if (linelen > MOUSE_SPEED_BOX_WIDTH - 1)
    {
        linelen = MOUSE_SPEED_BOX_WIDTH - 1;
    }

    if (linelen < redline_x)
    {
        V_DrawHorizLine(MOUSE_SPEED_BOX_X + 1,
                        MOUSE_SPEED_BOX_Y + MOUSE_SPEED_BOX_HEIGHT / 2,
                        linelen, white);
    }
    else
    {
        V_DrawHorizLine(MOUSE_SPEED_BOX_X + 1,
                        MOUSE_SPEED_BOX_Y + MOUSE_SPEED_BOX_HEIGHT / 2,
                        redline_x, white);
        V_DrawHorizLine(MOUSE_SPEED_BOX_X + redline_x,
                        MOUSE_SPEED_BOX_Y + MOUSE_SPEED_BOX_HEIGHT / 2,
                        linelen - redline_x, yellow);
    }

    // Draw acceleration threshold line
    V_DrawVertLine(MOUSE_SPEED_BOX_X + redline_x, MOUSE_SPEED_BOX_Y + 1,
                   MOUSE_SPEED_BOX_HEIGHT - 2, red);
}

// Highest seen mouse turn speed. We scale the range of the thermometer
// according to this value, so that it never exceeds the range. Initially
// this is set to a 1:1 setting where 1 pixel = 1 unit of speed.
static int max_seen_speed = MOUSE_SPEED_BOX_WIDTH - 1;

static void DrawNonAcceleratingBox(int speed)
{
    int white;
    int linelen;

#ifndef CRISPY_TRUECOLOR
    white = I_GetPaletteIndex(0xff, 0xff, 0xff);
#else
    white = I_MapRGB(0xff, 0xff, 0xff);
#endif

    if (speed > max_seen_speed)
    {
        max_seen_speed = speed;
    }

    // Draw horizontal "thermometer":
    linelen = speed * (MOUSE_SPEED_BOX_WIDTH - 1) / max_seen_speed;

    V_DrawHorizLine(MOUSE_SPEED_BOX_X + 1,
                    MOUSE_SPEED_BOX_Y + MOUSE_SPEED_BOX_HEIGHT / 2,
                    linelen, white);
}

void V_DrawMouseSpeedBox(int speed)
{
    int bgcolor, bordercolor, black;

    // If the mouse is turned off, don't draw the box at all.
    if (!usemouse)
    {
        return;
    }

    // Get palette indices for colors for widget. These depend on the
    // palette of the game being played.

#ifndef CRISPY_TRUECOLOR
    bgcolor = I_GetPaletteIndex(0x77, 0x77, 0x77);
    bordercolor = I_GetPaletteIndex(0x55, 0x55, 0x55);
    black = I_GetPaletteIndex(0x00, 0x00, 0x00);
#else
    bgcolor = I_MapRGB(0x77, 0x77, 0x77);
    bordercolor = I_MapRGB(0x55, 0x55, 0x55);
    black = I_MapRGB(0x00, 0x00, 0x00);
#endif

    // Calculate box position

    V_DrawFilledBox(MOUSE_SPEED_BOX_X, MOUSE_SPEED_BOX_Y,
                    MOUSE_SPEED_BOX_WIDTH, MOUSE_SPEED_BOX_HEIGHT, bgcolor);
    V_DrawBox(MOUSE_SPEED_BOX_X, MOUSE_SPEED_BOX_Y,
              MOUSE_SPEED_BOX_WIDTH, MOUSE_SPEED_BOX_HEIGHT, bordercolor);
    V_DrawHorizLine(MOUSE_SPEED_BOX_X + 1, MOUSE_SPEED_BOX_Y + 4,
                    MOUSE_SPEED_BOX_WIDTH - 2, black);

    // If acceleration is used, draw a box that helps to calibrate the
    // threshold point.
    if (mouse_threshold > 0 && fabs(mouse_acceleration - 1) > 0.01)
    {
        DrawAcceleratingBox(speed);
    }
    else
    {
        DrawNonAcceleratingBox(speed);
    }
}

