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
// R_planes.c

#include <stdlib.h>
#include "doomdef.h"
#include "deh_str.h"
#include "i_system.h"
#include "r_bmaps.h" // [crispy] R_BrightmapForTexName()
#include "r_local.h"
#include "r_swirl.h" // [crispy] R_DistortedFlat()

planefunction_t floorfunc, ceilingfunc;

//
// sky mapping
//
int skyflatnum;
int skytexture;
int skytexturemid;
fixed_t skyiscale;

//
// opening
//

visplane_t *visplanes = NULL, *lastvisplane;
visplane_t *floorplane, *ceilingplane;
static int numvisplanes;

int openings[MAXOPENINGS], *lastopening; // [crispy] 32-bit integer math

//
// clip values are the solid pixel bounding the range
// floorclip starts out SCREENHEIGHT
// ceilingclip starts out -1
//
int floorclip[MAXWIDTH];   // [crispy] 32-bit integer math
int ceilingclip[MAXWIDTH]; // [crispy] 32-bit integer math

//
// spanstart holds the start of a plane span
// initialized to 0 at start
//
int spanstart[MAXHEIGHT];
int spanstop[MAXHEIGHT];

//
// texture mapping
//
lighttable_t **planezlight;
fixed_t planeheight;

fixed_t *yslope;
fixed_t yslopes[LOOKDIRS][MAXHEIGHT]; // [crispy]
fixed_t distscale[MAXWIDTH];
fixed_t basexscale, baseyscale;

fixed_t cachedheight[MAXHEIGHT];
fixed_t cacheddistance[MAXHEIGHT];
fixed_t cachedxstep[MAXHEIGHT];
fixed_t cachedystep[MAXHEIGHT];

static fixed_t xsmoothscrolloffset; // [crispy]
static fixed_t ysmoothscrolloffset; // [crispy]

/*
================
=
= R_InitSkyMap
=
= Called whenever the view size changes
=
================
*/

void R_InitSkyMap(void)
{
    skyflatnum = R_FlatNumForName(DEH_String("F_SKY1"));
    skytexturemid = 200 * FRACUNIT;
    skyiscale = FRACUNIT >> crispy->hires;
}


/*
====================
=
= R_InitPlanes
=
= Only at game startup
====================
*/

void R_InitPlanes(void)
{
}


/*
================
=
= R_MapPlane
=
global vars:

planeheight
ds_source
basexscale
baseyscale
viewx
viewy

BASIC PRIMITIVE
================
*/

void R_MapPlane(int y, int x1, int x2)
{
    fixed_t distance;
    unsigned index;
    int dx, dy;

#ifdef RANGECHECK
    if (x2 < x1 || x1 < 0 || x2 >= viewwidth || (unsigned) y > viewheight)
        I_Error("R_MapPlane: %i, %i at %i", x1, x2, y);
#endif

// [crispy] visplanes with the same flats now match up far better than before
// adapted from prboom-plus/src/r_plane.c:191-239, translated to fixed-point math
//
// SoM: because centery is an actual row of pixels (and it isn't really the
// center row because there are an even number of rows) some corrections need
// to be made depending on where the row lies relative to the centery row.

    if (centery == y)
    {
	return;
    }

    dy = (abs(centery - y) << FRACBITS) + (y < centery ? -FRACUNIT : FRACUNIT) / 2;

    if (planeheight != cachedheight[y])
    {
        cachedheight[y] = planeheight;
        distance = cacheddistance[y] = FixedMul(planeheight, yslope[y]);

        ds_xstep = cachedxstep[y] = FixedDiv(FixedMul(viewsin, planeheight), dy) << detailshift;
        ds_ystep = cachedystep[y] = FixedDiv(FixedMul(viewcos, planeheight), dy) << detailshift;
    }
    else
    {
        distance = cacheddistance[y];
        ds_xstep = cachedxstep[y];
        ds_ystep = cachedystep[y];
    }

    dx = x1 - centerx;
    ds_xfrac = viewx + FixedMul(viewcos, distance) + dx * ds_xstep;
    ds_yfrac = -viewy - FixedMul(viewsin, distance) + dx * ds_ystep;

     // [crispy] add smooth scroll offsets
    ds_xfrac += xsmoothscrolloffset;
    ds_yfrac += ysmoothscrolloffset;

    if (fixedcolormap)
        ds_colormap[0] = ds_colormap[1] = fixedcolormap;
    else
    {
        index = distance >> LIGHTZSHIFT;
        if (index >= MAXLIGHTZ)
            index = MAXLIGHTZ - 1;
        ds_colormap[0] = planezlight[index];
        ds_colormap[1] = colormaps;
    }

    ds_y = y;
    ds_x1 = x1;
    ds_x2 = x2;

    spanfunc();                 // high or low detail
}

//=============================================================================

/*
====================
=
= R_ClearPlanes
=
= At begining of frame
====================
*/

void R_ClearPlanes(void)
{
    int i;
    angle_t angle;

//
// opening / clipping determination
//      
    for (i = 0; i < viewwidth; i++)
    {
        floorclip[i] = viewheight;
        ceilingclip[i] = -1;
    }

    lastvisplane = visplanes;
    lastopening = openings;

//
// texture calculation
//
    memset(cachedheight, 0, sizeof(cachedheight));
    angle = (viewangle - ANG90) >> ANGLETOFINESHIFT;    // left to right mapping

    // scale will be unit scale at SCREENWIDTH/2 distance
    basexscale = FixedDiv(finecosine[angle], centerxfrac);
    baseyscale = -FixedDiv(finesine[angle], centerxfrac);
}


// [crispy] remove MAXVISPLANES limit
static void R_RaiseVisplanes (visplane_t** vp)
{
    if (lastvisplane - visplanes == numvisplanes)
    {
	int numvisplanes_old = numvisplanes;
	visplane_t* visplanes_old = visplanes;

	numvisplanes = numvisplanes ? 2 * numvisplanes : MAXVISPLANES;
	visplanes = I_Realloc(visplanes, numvisplanes * sizeof(*visplanes));
	memset(visplanes + numvisplanes_old, 0, (numvisplanes - numvisplanes_old) * sizeof(*visplanes));

	lastvisplane = visplanes + numvisplanes_old;
	floorplane = visplanes + (floorplane - visplanes_old);
	ceilingplane = visplanes + (ceilingplane - visplanes_old);

	if (numvisplanes_old)
	    fprintf(stderr, "R_FindPlane: Hit MAXVISPLANES limit at %d, raised to %d.\n", numvisplanes_old, numvisplanes);

	// keep the pointer passed as argument in relation to the visplanes pointer
	if (vp)
	    *vp = visplanes + (*vp - visplanes_old);
    }
}

/*
===============
=
= R_FindPlane
=
===============
*/

visplane_t *R_FindPlane(fixed_t height, int picnum,
                        int lightlevel, int special)
{
    visplane_t *check;

    // [crispy] add support for MBF sky transfers
    if (picnum == skyflatnum || picnum & PL_SKYFLAT)
    {
        // all skies map together
        height = 0;
        lightlevel = 0;
    }

    for (check = visplanes; check < lastvisplane; check++)
    {
        if (height == check->height
            && picnum == check->picnum
            && lightlevel == check->lightlevel && special == check->special)
            break;
    }

    if (check < lastvisplane)
    {
        return (check);
    }

    R_RaiseVisplanes(&check);

    lastvisplane++;
    check->height = height;
    check->picnum = picnum;
    check->lightlevel = lightlevel;
    check->special = special;
    check->minx = SCREENWIDTH;
    check->maxx = -1;
    memset(check->top, 0xff, sizeof(check->top));
    return (check);
}

/*
===============
=
= R_CheckPlane
=
===============
*/

visplane_t *R_CheckPlane(visplane_t * pl, int start, int stop)
{
    int intrl, intrh;
    int unionl, unionh;
    int x;

    if (start < pl->minx)
    {
        intrl = pl->minx;
        unionl = start;
    }
    else
    {
        unionl = pl->minx;
        intrl = start;
    }

    if (stop > pl->maxx)
    {
        intrh = pl->maxx;
        unionh = stop;
    }
    else
    {
        unionh = pl->maxx;
        intrh = stop;
    }

    for (x = intrl; x <= intrh; x++)
        if (pl->top[x] != 0xffffffffu) // [crispy] hires / 32-bit integer math
            break;

    if (x > intrh)
    {
        pl->minx = unionl;
        pl->maxx = unionh;
        return pl;              // use the same one
    }

// make a new visplane

    R_RaiseVisplanes(&pl);
    lastvisplane->height = pl->height;
    lastvisplane->picnum = pl->picnum;
    lastvisplane->lightlevel = pl->lightlevel;
    lastvisplane->special = pl->special;
    pl = lastvisplane++;
    pl->minx = start;
    pl->maxx = stop;
    memset(pl->top, 0xff, sizeof(pl->top));

    return pl;
}



//=============================================================================

/*
================
=
= R_MakeSpans
=
================
*/

void R_MakeSpans(int x, unsigned int t1, unsigned int b1, unsigned int t2, unsigned int b2) // [crispy] 32-bit integer math
{
    while (t1 < t2 && t1 <= b1)
    {
        R_MapPlane(t1, spanstart[t1], x - 1);
        t1++;
    }
    while (b1 > b2 && b1 >= t1)
    {
        R_MapPlane(b1, spanstart[b1], x - 1);
        b1--;
    }

    while (t2 < t1 && t2 <= b2)
    {
        spanstart[t2] = x;
        t2++;
    }
    while (b2 > b1 && b2 >= t2)
    {
        spanstart[b2] = x;
        b2--;
    }
}



/*
================
=
= R_DrawPlanes
=
= At the end of each frame
================
*/

#define FLATSCROLL(X) \
    ((interpfactor << (X)) - (((63 - ((leveltime >> 1) & 63)) << (X) & 63) * FRACUNIT))

void R_DrawPlanes(void)
{
    boolean swirling;
    visplane_t *pl;
    int light;
    int x, stop;
    int lumpnum;
    int angle;
    byte *tempSource;

    pixel_t *dest;
    int count;
    fixed_t frac, fracstep;
    int texture, heightmask; // [crispy]
    static int prev_texture, skyheight; // [crispy]
    static int interpfactor; // [crispy]

#ifdef RANGECHECK
    if (ds_p - drawsegs > numdrawsegs)
        I_Error("R_DrawPlanes: drawsegs overflow (%td)",
                ds_p - drawsegs);
    if (lastvisplane - visplanes > numvisplanes)
        I_Error("R_DrawPlanes: visplane overflow (%td)",
                lastvisplane - visplanes);
    if (lastopening - openings > MAXOPENINGS)
        I_Error("R_DrawPlanes: opening overflow (%td)",
                lastopening - openings);
#endif

    for (pl = visplanes; pl < lastvisplane; pl++)
    {
        if (pl->minx > pl->maxx)
            continue;
        //
        // sky flat
        // [crispy] add support for MBF sky transfers
        if (pl->picnum == skyflatnum || pl->picnum & PL_SKYFLAT)
        {
	    angle_t an = viewangle, flip;
	    if (pl->picnum & PL_SKYFLAT)
	    {
		const line_t *l = &lines[pl->picnum & ~PL_SKYFLAT];
		const side_t *s = *l->sidenum + sides;
		texture = texturetranslation[s->toptexture];
		dc_texturemid = s->rowoffset - 28*FRACUNIT;
		flip = (l->special == 272) ? 0u : ~0u;
		an += s->textureoffset;
	    }
	    else
	    {
		texture = skytexture;
		dc_texturemid = skytexturemid;
		flip = 0;
	    }

            if (texture != prev_texture)
            {
                skyheight = R_GetPatchHeight(texture, 0);
                prev_texture = texture;
            }

            dc_iscale = skyiscale;
            // [crispy] no brightmaps for sky
            dc_colormap[0] = dc_colormap[1] = colormaps;    // sky is allways drawn full bright
            dc_texheight = textureheight[texture]>>FRACBITS;

            for (x = pl->minx; x <= pl->maxx; x++)
            {
                dc_yl = pl->top[x];
                dc_yh = pl->bottom[x];
                if ((unsigned) dc_yl <= dc_yh) // [crispy] 32-bit integer math
                {
                    angle = ((an + xtoviewangle[x]) ^ flip) >> ANGLETOSKYSHIFT;
                    dc_x = x;
                    dc_source = R_GetColumn(texture, angle);

                    count = dc_yh - dc_yl;
                    if (count < 0)
                        return;

#ifdef RANGECHECK
                    if ((unsigned) dc_x >= SCREENWIDTH || dc_yl < 0
                        || dc_yh >= SCREENHEIGHT)
                        I_Error("R_DrawColumn: %i to %i at %i", dc_yl, dc_yh,
                                dc_x);
#endif

                    dest = ylookup[dc_yl] + columnofs[dc_x];

                    fracstep = dc_iscale;
                    frac = dc_texturemid + (dc_yl - centery) * fracstep;
                    // not a power of 2 -- killough
                    if (skyheight & (skyheight - 1))
                    {
                        heightmask = skyheight << FRACBITS;

                        if (frac < 0)
                            while ((frac += heightmask) < 0);
                        else
                            while (frac >= heightmask)
                                frac -= heightmask;
                        do
                        {
#ifndef CRISPY_TRUECOLOR
                            *dest = dc_source[frac >> FRACBITS];
#else
                            *dest = pal_color[dc_source[frac >> FRACBITS]];
#endif
                            dest += SCREENWIDTH;

                            if ((frac += fracstep) >= heightmask)
                            {
                                frac -= heightmask;
                            }
                        } while (count--);

//                                      colfunc ();
                    }
                    // texture height is a power of 2 -- killough
                    else
                    {
                        heightmask = skyheight - 1;

                        do
                        {
#ifndef CRISPY_TRUECOLOR
                            *dest = dc_source[(frac >> FRACBITS) & heightmask];
#else
                            *dest = pal_color[dc_source[(frac >> FRACBITS) & heightmask]];
#endif
                            dest += SCREENWIDTH;
                            frac += fracstep;
                        } while (count--);
                    }
                }
            }
            continue;
        }

        //
        // regular flat
        //
        swirling = flattranslation[pl->picnum] == -1;
        if (!swirling) // [crispy] adapt swirl from src/doom to src/heretic
        {
        lumpnum = firstflat + flattranslation[pl->picnum];

        tempSource = W_CacheLumpNum(lumpnum, PU_STATIC);

        // [crispy] Use old value of interpfactor if uncapped and paused. This
        // ensures that scrolling stops smoothly when pausing.
        if (crispy->uncapped && leveltime > oldleveltime)
        {
            // [crispy] Scrolling normally advances every *other* gametic, so
            // interpolation needs to span two tics
            if (leveltime & 1)
            {
                interpfactor = (FRACUNIT + fractionaltic) >> 1;
            }
            else
            {
                interpfactor = fractionaltic >> 1;
            }
        }
        else if (!crispy->uncapped)
        {
            interpfactor = 0;
        }

        //[crispy] use smoothscrolloffsets to unconditonally animate all scrolling floors
        switch (pl->special)
        {
            case 25:
            case 26:
            case 27:
            case 28:
            case 29:           // Scroll_North
                xsmoothscrolloffset = 0;
                ysmoothscrolloffset = FLATSCROLL(pl->special - 25);
                ds_source = tempSource;
                break;
            case 20:
            case 21:
            case 22:
            case 23:
            case 24:           // Scroll_East
                // [crispy] vanilla Heretic animates Eastward scrollers by adding to tempSource.
                // this directly offsets the position the flat is read from, and results in
                // visual artifacts (tutti-frutti on flats that aren't at least 65px tall, jittery
                // animation, unwanted visplane merging of adjacent flats with different scrollers)
                xsmoothscrolloffset = -FLATSCROLL(pl->special - 20);
                ysmoothscrolloffset = 0;
                ds_source = tempSource;
                //ds_source = tempSource+((leveltime>>1)&63);
                break;
            case 30:
            case 31:
            case 32:
            case 33:
            case 34:           // Scroll_South
                xsmoothscrolloffset = 0;
                ysmoothscrolloffset = -FLATSCROLL(pl->special - 30);
                ds_source = tempSource;
                break;
            case 35:
            case 36:
            case 37:
            case 38:
            case 39:           // Scroll_West
                xsmoothscrolloffset = FLATSCROLL(pl->special - 35);
                ysmoothscrolloffset = 0;
                ds_source = tempSource;
                break;
            case 4:            // Scroll_EastLavaDamage
                // [crispy] calculation moved from tempSource, see Scroll_East above
                xsmoothscrolloffset = -FLATSCROLL(3);
                ysmoothscrolloffset = 0;
                ds_source = tempSource;
                break;
            default:
                xsmoothscrolloffset = 0;
                ysmoothscrolloffset = 0;
                ds_source = tempSource;
        }
        }
        else 
        {
            lumpnum = firstflat+pl->picnum;
            ds_source = R_DistortedFlat(lumpnum);
        }
        ds_brightmap = R_BrightmapForFlatNum(lumpnum-firstflat);
        planeheight = abs(pl->height - viewz);
        light = (pl->lightlevel >> LIGHTSEGSHIFT) + (extralight * LIGHTBRIGHT); // [crispy] smooth diminishing lighting
        if (light >= LIGHTLEVELS)
            light = LIGHTLEVELS - 1;
        if (light < 0)
            light = 0;
        planezlight = zlight[light];

        pl->top[pl->maxx + 1] = 0xffffffffu; // [crispy] hires / 32-bit integer math
        pl->top[pl->minx - 1] = 0xffffffffu; // [crispy] hires / 32-bit integer math

        stop = pl->maxx + 1;
        for (x = pl->minx; x <= stop; x++)
            R_MakeSpans(x, pl->top[x - 1], pl->bottom[x - 1], pl->top[x],
                        pl->bottom[x]);

        if (!swirling)
        W_ReleaseLumpNum(lumpnum);
    }
}
