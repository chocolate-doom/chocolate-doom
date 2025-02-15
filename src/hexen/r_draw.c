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


#include "h2def.h"
#include "i_system.h"
#include "i_video.h"
#include "r_local.h"
#include "v_video.h"
#include "v_trans.h" // [crispy] blending functions

/*

All drawing to the view buffer is accomplished in this file.  The other refresh
files only know about ccordinates, not the architecture of the frame buffer.

*/

byte *viewimage;
int viewwidth, scaledviewwidth, viewheight, viewwindowx, viewwindowy;
pixel_t *ylookup[MAXHEIGHT];
int columnofs[MAXWIDTH];
//byte translations[3][256]; // color tables for different players

/*
==================
=
= R_DrawColumn
=
= Source is the top of the column to scale
=
==================
*/

lighttable_t *dc_colormap[2]; // [crispy] brightmaps
int dc_x;
int dc_yl;
int dc_yh;
fixed_t dc_iscale;
fixed_t dc_texturemid;
int dc_texheight; // [crispy]
byte *dc_source;                // first pixel in a column (possibly virtual)

int dccount;                    // just for profiling

// [crispy] Add Lee Killough tutti-frutti fix for all relevant DrawColumn
// functions.

void R_DrawColumn(void)
{
    int count;
    pixel_t *dest;
    fixed_t frac, fracstep;
    int heightmask = dc_texheight - 1; // [crispy]

    count = dc_yh - dc_yl;
    if (count < 0)
        return;

#ifdef RANGECHECK
    if ((unsigned) dc_x >= SCREENWIDTH || dc_yl < 0 || dc_yh >= SCREENHEIGHT)
        I_Error("R_DrawColumn: %i to %i at %i", dc_yl, dc_yh, dc_x);
#endif

    dest = ylookup[dc_yl] + columnofs[dc_x];

    fracstep = dc_iscale;
    frac = dc_texturemid + (dc_yl - centery) * fracstep;

    if (dc_texheight & heightmask) // not a power of 2 -- killough
    {
        heightmask++;
        heightmask <<= FRACBITS;

        if (frac < 0)
            while ((frac += heightmask) < 0);
        else
            while (frac >= heightmask)
                frac -= heightmask;

        do
        {
            // [crispy] brightmaps
            const byte source = dc_source[frac >> FRACBITS];
            *dest = dc_colormap[dc_brightmap[source]][source];
            dest += SCREENWIDTH;
            if ((frac += fracstep) >= heightmask)
                frac -= heightmask;
        } while (count--);
    }
    else // texture height is a power of 2 -- killough
    {
        do
        {
            // [crispy] brightmaps
            const byte source = dc_source[(frac >> FRACBITS) & heightmask];
            *dest = dc_colormap[dc_brightmap[source]][source];
            dest += SCREENWIDTH;
            frac += fracstep;
        } while (count--);
    }
}

void R_DrawColumnLow(void)
{
    int count;
    pixel_t *dest;
    fixed_t frac, fracstep;
    int heightmask = dc_texheight - 1; // [crispy]

    count = dc_yh - dc_yl;
    if (count < 0)
        return;

#ifdef RANGECHECK
    if ((unsigned) dc_x >= SCREENWIDTH || dc_yl < 0 || dc_yh >= SCREENHEIGHT)
        I_Error("R_DrawColumn: %i to %i at %i", dc_yl, dc_yh, dc_x);
//      dccount++;
#endif

    dest = ylookup[dc_yl] + columnofs[dc_x];

    fracstep = dc_iscale;
    frac = dc_texturemid + (dc_yl - centery) * fracstep;

    if (dc_texheight & heightmask) // not a power of 2 -- killough
    {
        heightmask++;
        heightmask <<= FRACBITS;

        if (frac < 0)
            while ((frac += heightmask) < 0);
        else
            while (frac >= heightmask)
                frac -= heightmask;

        do
        {
            // [crispy] brightmaps
            const byte source = dc_source[frac >> FRACBITS];
            *dest = dc_colormap[dc_brightmap[source]][source];
            dest += SCREENWIDTH;
            if ((frac += fracstep) >= heightmask)
                frac -= heightmask;
        } while (count--);
    }
    else // texture height is a power of 2 -- killough
    {
        do
        {
            // [crispy] brightmaps
            const byte source = dc_source[(frac >> FRACBITS) & heightmask];
            *dest = dc_colormap[dc_brightmap[source]][source];
            dest += SCREENWIDTH;
            frac += fracstep;
        } while (count--);
    }
}

void R_DrawTLColumn(void)
{
    int count;
    pixel_t *dest;
    fixed_t frac, fracstep;
    int heightmask = dc_texheight - 1; // [crispy]

    // [crispy] Show transparent lines at top and bottom of screen.
    /*
    if (!dc_yl)
        dc_yl = 1;
    if (dc_yh == viewheight - 1)
        dc_yh = viewheight - 2;
    */

    count = dc_yh - dc_yl;
    if (count < 0)
        return;

#ifdef RANGECHECK
    if ((unsigned) dc_x >= SCREENWIDTH || dc_yl < 0 || dc_yh >= SCREENHEIGHT)
        I_Error("R_DrawTLColumn: %i to %i at %i", dc_yl, dc_yh, dc_x);
#endif

    dest = ylookup[dc_yl] + columnofs[dc_x];

    fracstep = dc_iscale;
    frac = dc_texturemid + (dc_yl - centery) * fracstep;

    if (dc_texheight & heightmask) // not a power of 2 -- killough
    {
        heightmask++;
        heightmask <<= FRACBITS;

        if (frac < 0)
            while ((frac += heightmask) < 0);
        else
            while (frac >= heightmask)
                frac -= heightmask;

        do
        {
            // [crispy] brightmaps
            const byte source = dc_source[frac >> FRACBITS];
#ifndef CRISPY_TRUECOLOR
            *dest = tinttable[*dest +
                              (dc_colormap[dc_brightmap[source]][source] <<
                               8)];
#else
            const pixel_t destrgb = dc_colormap[dc_brightmap[source]][source];
            *dest = blendfunc(*dest, destrgb);
#endif
            dest += SCREENWIDTH;
            if ((frac += fracstep) >= heightmask)
                frac -= heightmask;
        } while (count--);
    }
    else // texture height is a power of 2 -- killough
    {
        do
        {
            // [crispy] brightmaps
            const byte source = dc_source[(frac >> FRACBITS) & heightmask];
#ifndef CRISPY_TRUECOLOR
            *dest = tinttable[*dest +
                              (dc_colormap[dc_brightmap[source]][source] <<
                               8)];
#else
            const pixel_t destrgb = dc_colormap[dc_brightmap[source]][source];
            *dest = blendfunc(*dest, destrgb);
#endif
            dest += SCREENWIDTH;
            frac += fracstep;
        } while (count--);
    }
}

//============================================================================
//
// R_DrawAltTLColumn
//
//============================================================================

void R_DrawAltTLColumn(void)
{
    int count;
    pixel_t *dest;
    fixed_t frac, fracstep;
    int heightmask = dc_texheight - 1; // [crispy]

    // [crispy] Show transparent lines at top and bottom of screen.
    /*
    if (!dc_yl)
        dc_yl = 1;
    if (dc_yh == viewheight - 1)
        dc_yh = viewheight - 2;
    */

    count = dc_yh - dc_yl;
    if (count < 0)
        return;

#ifdef RANGECHECK
    if ((unsigned) dc_x >= SCREENWIDTH || dc_yl < 0 || dc_yh >= SCREENHEIGHT)
        I_Error("R_DrawAltTLColumn: %i to %i at %i", dc_yl, dc_yh, dc_x);
#endif

    dest = ylookup[dc_yl] + columnofs[dc_x];

    fracstep = dc_iscale;
    frac = dc_texturemid + (dc_yl - centery) * fracstep;

    if (dc_texheight & heightmask) // not a power of 2 -- killough
    {
        heightmask++;
        heightmask <<= FRACBITS;

        if (frac < 0)
            while ((frac += heightmask) < 0);
        else
            while (frac >= heightmask)
                frac -= heightmask;

        do
        {
            // [crispy] brightmaps
            const byte source = dc_source[frac >> FRACBITS];
#ifndef CRISPY_TRUECOLOR
            *dest = tinttable[((*dest) << 8)
                              + dc_colormap[dc_brightmap[source]][source]];
#else
            const pixel_t destrgb = dc_colormap[dc_brightmap[source]][source];
            *dest = blendfunc(*dest, destrgb);
#endif
            dest += SCREENWIDTH;
            if ((frac += fracstep) >= heightmask)
                frac -= heightmask;
        } while (count--);
    }
    else // texture height is a power of 2 -- killough
    {
        do
        {
            // [crispy] brightmaps
            const byte source = dc_source[(frac >> FRACBITS) & heightmask];
#ifndef CRISPY_TRUECOLOR
            *dest = tinttable[((*dest) << 8)
                              + dc_colormap[dc_brightmap[source]][source]];
#else
            const pixel_t destrgb = dc_colormap[dc_brightmap[source]][source];
            *dest = blendfunc(*dest, destrgb);
#endif
            dest += SCREENWIDTH;
            frac += fracstep;
        } while (count--);
    }
}

/*
========================
=
= R_DrawTranslatedColumn
=
========================
*/

byte *dc_translation;
byte *translationtables;

void R_DrawTranslatedColumn(void)
{
    int count;
    pixel_t *dest;
    fixed_t frac, fracstep;

    count = dc_yh - dc_yl;
    if (count < 0)
        return;

#ifdef RANGECHECK
    if ((unsigned) dc_x >= SCREENWIDTH || dc_yl < 0 || dc_yh >= SCREENHEIGHT)
        I_Error("R_DrawColumn: %i to %i at %i", dc_yl, dc_yh, dc_x);
#endif

    dest = ylookup[dc_yl] + columnofs[dc_x];

    fracstep = dc_iscale;
    frac = dc_texturemid + (dc_yl - centery) * fracstep;

    do
    {
        // [crispy] brightmaps
        const byte source = dc_source[frac >> FRACBITS];
        *dest = dc_colormap[dc_brightmap[source]][dc_translation[source]];
        dest += SCREENWIDTH;
        frac += fracstep;
    }
    while (count--);
}

//============================================================================
//
// R_DrawTranslatedTLColumn
//
//============================================================================

void R_DrawTranslatedTLColumn(void)
{
    int count;
    pixel_t *dest;
    fixed_t frac, fracstep;

    count = dc_yh - dc_yl;
    if (count < 0)
        return;

#ifdef RANGECHECK
    if ((unsigned) dc_x >= SCREENWIDTH || dc_yl < 0 || dc_yh >= SCREENHEIGHT)
        I_Error("R_DrawColumn: %i to %i at %i", dc_yl, dc_yh, dc_x);
#endif

    dest = ylookup[dc_yl] + columnofs[dc_x];

    fracstep = dc_iscale;
    frac = dc_texturemid + (dc_yl - centery) * fracstep;

    do
    {
        // [crispy] brightmaps
        byte src = dc_translation[dc_source[frac >> FRACBITS]];
#ifndef CRISPY_TRUECOLOR
        *dest = tinttable[((*dest) << 8)
                          +
                          dc_colormap[dc_brightmap[src]][src]];
#else
        const pixel_t destrgb = dc_colormap[dc_brightmap[src]][src];
        *dest = blendfunc(*dest, destrgb);
#endif
        dest += SCREENWIDTH;
        frac += fracstep;
    }
    while (count--);
}

//============================================================================
//
// R_DrawTranslatedAltTLColumn
//
//============================================================================

/*
void R_DrawTranslatedAltTLColumn (void)
{
	int			count;
	byte		*dest;
	fixed_t		frac, fracstep;	

	count = dc_yh - dc_yl;
	if (count < 0)
		return;
				
#ifdef RANGECHECK
	if ((unsigned)dc_x >= SCREENWIDTH || dc_yl < 0 || dc_yh >= SCREENHEIGHT)
		I_Error ("R_DrawColumn: %i to %i at %i", dc_yl, dc_yh, dc_x);
#endif

	dest = ylookup[dc_yl] + columnofs[dc_x];
	
	fracstep = dc_iscale;
	frac = dc_texturemid + (dc_yl-centery)*fracstep;

	do
	{
		*dest = tinttable[*dest
			+(dc_colormap[dc_translation[dc_source[frac>>FRACBITS]]]<<8)];
		dest += SCREENWIDTH;
		frac += fracstep;
	} while (count--);
}
*/

//--------------------------------------------------------------------------
//
// PROC R_InitTranslationTables
//
//--------------------------------------------------------------------------

void R_InitTranslationTables(void)
{
    int i;
    byte *transLump;
    int lumpnum;

#ifndef CRISPY_TRUECOLOR
    V_LoadTintTable();
#else
    I_InitTinttab((GameMission_t) hexen);
#endif

    // Allocate translation tables
    translationtables = Z_Malloc(256 * 3 * (maxplayers - 1), PU_STATIC, 0);

    for (i = 0; i < 3 * (maxplayers - 1); i++)
    {
        lumpnum = W_GetNumForName("trantbl0") + i;
        transLump = W_CacheLumpNum(lumpnum, PU_STATIC);
        memcpy(translationtables + i * 256, transLump, 256);
        W_ReleaseLumpNum(lumpnum);
    }
}

/*
================
=
= R_DrawSpan
=
================
*/

int ds_y;
int ds_x1;
int ds_x2;
lighttable_t *ds_colormap;
fixed_t ds_xfrac;
fixed_t ds_yfrac;
fixed_t ds_xstep;
fixed_t ds_ystep;
byte *ds_source;                // start of a 64*64 tile image

int dscount;                    // just for profiling

void R_DrawSpan(void)
{
    fixed_t xfrac, yfrac;
    pixel_t *dest;
    int count, spot;

#ifdef RANGECHECK
    if (ds_x2 < ds_x1 || ds_x1 < 0 || ds_x2 >= SCREENWIDTH
        || (unsigned) ds_y > SCREENHEIGHT)
        I_Error("R_DrawSpan: %i to %i at %i", ds_x1, ds_x2, ds_y);
//      dscount++;
#endif

    xfrac = ds_xfrac;
    yfrac = ds_yfrac;

    dest = ylookup[ds_y] + columnofs[ds_x1];
    count = ds_x2 - ds_x1;
    do
    {
        spot = ((yfrac >> (16 - 6)) & (63 * 64)) + ((xfrac >> 16) & 63);
        *dest++ = ds_colormap[ds_source[spot]];
        xfrac += ds_xstep;
        yfrac += ds_ystep;
    }
    while (count--);
}

void R_DrawSpanLow(void)
{
    fixed_t xfrac, yfrac;
    pixel_t *dest;
    int count, spot;

#ifdef RANGECHECK
    if (ds_x2 < ds_x1 || ds_x1 < 0 || ds_x2 >= SCREENWIDTH
        || (unsigned) ds_y > SCREENHEIGHT)
        I_Error("R_DrawSpan: %i to %i at %i", ds_x1, ds_x2, ds_y);
//      dscount++;
#endif

    xfrac = ds_xfrac;
    yfrac = ds_yfrac;

    dest = ylookup[ds_y] + columnofs[ds_x1];
    count = ds_x2 - ds_x1;
    do
    {
        spot = ((yfrac >> (16 - 6)) & (63 * 64)) + ((xfrac >> 16) & 63);
        *dest++ = ds_colormap[ds_source[spot]];
        xfrac += ds_xstep;
        yfrac += ds_ystep;
    }
    while (count--);
}



/*
================
=
= R_InitBuffer
=
=================
*/

void R_InitBuffer(int width, int height)
{
    int i;

    viewwindowx = (SCREENWIDTH - width) >> 1;
    for (i = 0; i < width; i++)
        columnofs[i] = viewwindowx + i;
    if (width == SCREENWIDTH)
        viewwindowy = 0;
    else
        viewwindowy = (SCREENHEIGHT - SBARHEIGHT - height) >> 1;
    // [crispy] make sure viewwindowy is always an even number
    viewwindowy &= ~1;
    for (i = 0; i < height; i++)
        ylookup[i] = I_VideoBuffer + (i + viewwindowy) * SCREENWIDTH;
}


/*
==================
=
= R_DrawViewBorder
=
= Draws the border around the view for different size windows
==================
*/

boolean BorderNeedRefresh;

void R_DrawViewBorder(void)
{
    byte *src;
    pixel_t *dest;
    int x, y;

    if (scaledviewwidth == SCREENWIDTH)
        return;

    src = W_CacheLumpName("F_022", PU_CACHE);
    dest = I_VideoBuffer;

    // [crispy] use unified flat filling function
    V_FillFlat(0, SCREENHEIGHT - SBARHEIGHT, 0, SCREENWIDTH, src, dest);

    for (x = (viewwindowx >> crispy->hires); x < (viewwindowx + viewwidth) >> crispy->hires; x += 16)
    {
        V_DrawPatch(x - WIDESCREENDELTA, (viewwindowy >> crispy->hires) - 4,
                    W_CacheLumpName("bordt", PU_CACHE));
        V_DrawPatch(x - WIDESCREENDELTA, (viewwindowy + viewheight) >> crispy->hires,
                    W_CacheLumpName("bordb", PU_CACHE));
    }
    for (y = (viewwindowy >> crispy->hires); y < (viewwindowy + viewheight) >> crispy->hires; y += 16)
    {
        V_DrawPatch((viewwindowx >> crispy->hires) - 4 - WIDESCREENDELTA, y,
                     W_CacheLumpName("bordl", PU_CACHE));
        V_DrawPatch(((viewwindowx + viewwidth) >> crispy->hires) - WIDESCREENDELTA, y,
                      W_CacheLumpName("bordr", PU_CACHE));
    }
    V_DrawPatch((viewwindowx >> crispy->hires) - 4 - WIDESCREENDELTA,
                (viewwindowy >> crispy->hires) - 4,
                 W_CacheLumpName("bordtl", PU_CACHE));
    V_DrawPatch(((viewwindowx + viewwidth) >> crispy->hires) - WIDESCREENDELTA,
                (viewwindowy >> crispy->hires) - 4,
                W_CacheLumpName("bordtr", PU_CACHE));
    V_DrawPatch(((viewwindowx + viewwidth) >> crispy->hires) - WIDESCREENDELTA,
                (viewwindowy + viewheight) >> crispy->hires,
                W_CacheLumpName("bordbr", PU_CACHE));
    V_DrawPatch((viewwindowx >> crispy->hires) - 4 - WIDESCREENDELTA,
                (viewwindowy + viewheight) >> crispy->hires,
                W_CacheLumpName("bordbl", PU_CACHE));
}

/*
==================
=
= R_DrawTopBorder
=
= Draws the top border around the view for different size windows
==================
*/

boolean BorderTopRefresh;

void R_DrawTopBorder(void)
{
    byte *src;
    pixel_t *dest;

    if (scaledviewwidth == SCREENWIDTH)
        return;

/*	if(gamemode == shareware)
	{
		src = W_CacheLumpName ("FLOOR04", PU_CACHE);
	}
	else
	{
		src = W_CacheLumpName ("FLAT513", PU_CACHE);
	}
*/
    src = W_CacheLumpName("F_022", PU_CACHE);
    dest = I_VideoBuffer;

    // [crispy] use unified flat filling function
    V_FillFlat(0, 34 << crispy->hires, 0, SCREENWIDTH, src, dest);

    if ((viewwindowy >> crispy->hires) < 35)
    {
        int x;

        for (x = (viewwindowx >> crispy->hires); x < (viewwindowx + viewwidth) >> crispy->hires; x += 16)
        {
            V_DrawPatch(x - WIDESCREENDELTA, (viewwindowy >> crispy->hires) - 4,
                        W_CacheLumpName("bordt", PU_CACHE));
        }
        V_DrawPatch((viewwindowx >> crispy->hires) - 4 - WIDESCREENDELTA,
                    viewwindowy >> crispy->hires,
                    W_CacheLumpName("bordl", PU_CACHE));
        V_DrawPatch(((viewwindowx + viewwidth) >> crispy->hires) - WIDESCREENDELTA,
                    viewwindowy >> crispy->hires,
                    W_CacheLumpName("bordr", PU_CACHE));
        V_DrawPatch((viewwindowx >> crispy->hires) - 4 - WIDESCREENDELTA,
                    (viewwindowy >> crispy->hires) + 16,
                    W_CacheLumpName("bordl", PU_CACHE));
        V_DrawPatch(((viewwindowx + viewwidth) >> crispy->hires) - WIDESCREENDELTA,
                    (viewwindowy >> crispy->hires) + 16,
                    W_CacheLumpName("bordr", PU_CACHE));

        V_DrawPatch((viewwindowx >> crispy->hires) - 4 - WIDESCREENDELTA,
                    (viewwindowy >> crispy->hires) - 4,
                    W_CacheLumpName("bordtl", PU_CACHE));
        V_DrawPatch(((viewwindowx + viewwidth) >> crispy->hires) - WIDESCREENDELTA,
                    (viewwindowy >> crispy->hires) - 4,
                    W_CacheLumpName("bordtr", PU_CACHE));
    }
}
