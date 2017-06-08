//
// Copyright(C) 1993-1996 Id Software, Inc.
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
//	Here is a core component: drawing the floors and ceilings,
//	 while maintaining a per column clipping list only.
//	Moreover, the sky areas have to be determined.
//


#include <stdio.h>
#include <stdlib.h>

#include "i_system.h"
#include "z_zone.h"
#include "w_wad.h"

#include "doomdef.h"
#include "doomstat.h"

#include "r_local.h"
#include "r_sky.h"



planefunction_t		floorfunc;
planefunction_t		ceilingfunc;

//
// opening
//

// Here comes the obnoxious "visplane".
#define MAXVISPLANES	128
visplane_t*		visplanes = NULL;
visplane_t*		lastvisplane;
visplane_t*		floorplane;
visplane_t*		ceilingplane;
static int		numvisplanes;

// ?
#define MAXOPENINGS	SCREENWIDTH*64*4
int			openings[MAXOPENINGS]; // [crispy] 32-bit integer math
int*			lastopening; // [crispy] 32-bit integer math


//
// Clip values are the solid pixel bounding the range.
//  floorclip starts out SCREENHEIGHT
//  ceilingclip starts out -1
//
int			floorclip[SCREENWIDTH]; // [crispy] 32-bit integer math
int			ceilingclip[SCREENWIDTH]; // [crispy] 32-bit integer math

//
// spanstart holds the start of a plane span
// initialized to 0 at start
//
int			spanstart[SCREENHEIGHT];
int			spanstop[SCREENHEIGHT];

//
// texture mapping
//
lighttable_t**		planezlight;
fixed_t			planeheight;

fixed_t*			yslope;
fixed_t			yslopes[LOOKDIRS][SCREENHEIGHT];
fixed_t			distscale[SCREENWIDTH];
fixed_t			basexscale;
fixed_t			baseyscale;

fixed_t			cachedheight[SCREENHEIGHT];
fixed_t			cacheddistance[SCREENHEIGHT];
fixed_t			cachedxstep[SCREENHEIGHT];
fixed_t			cachedystep[SCREENHEIGHT];



//
// R_InitPlanes
// Only at game startup.
//
void R_InitPlanes (void)
{
  // Doh!
}


//
// R_MapPlane
//
// Uses global vars:
//  planeheight
//  ds_source
//  basexscale
//  baseyscale
//  viewx
//  viewy
//
// BASIC PRIMITIVE
//
void
R_MapPlane
( int		y,
  int		x1,
  int		x2 )
{
// [crispy] see below
//  angle_t	angle;
    fixed_t	distance;
//  fixed_t	length;
    unsigned	index;
    int dx, dy;
	
#ifdef RANGECHECK
    if (x2 < x1
     || x1 < 0
     || x2 >= viewwidth
     || y > viewheight)
    {
	I_Error ("R_MapPlane: %i, %i at %i",x1,x2,y);
    }
#endif

// [crispy] visplanes with the same flats now match up far better than before
// adapted from prboom-plus/src/r_plane.c:191-239, translated to fixed-point math

    if (!(dy = abs(centery - y)))
    {
	return;
    }

    if (planeheight != cachedheight[y])
    {
	cachedheight[y] = planeheight;
	distance = cacheddistance[y] = FixedMul (planeheight, yslope[y]);
	ds_xstep = cachedxstep[y] = FixedMul (viewsin, planeheight) / dy;
	ds_ystep = cachedystep[y] = FixedMul (viewcos, planeheight) / dy;
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

    if (fixedcolormap)
	ds_colormap = fixedcolormap;
    else
    {
	index = distance >> LIGHTZSHIFT;
	
	if (index >= MAXLIGHTZ )
	    index = MAXLIGHTZ-1;

	ds_colormap = planezlight[index];
    }
	
    ds_y = y;
    ds_x1 = x1;
    ds_x2 = x2;

    // high or low detail
    spanfunc ();	
}


//
// R_ClearPlanes
// At begining of frame.
//
void R_ClearPlanes (void)
{
    int		i;
    angle_t	angle;
    
    // opening / clipping determination
    for (i=0 ; i<viewwidth ; i++)
    {
	floorclip[i] = viewheight;
	ceilingclip[i] = -1;
    }

    lastvisplane = visplanes;
    lastopening = openings;
    
    // texture calculation
    memset (cachedheight, 0, sizeof(cachedheight));

    // left to right mapping
    angle = (viewangle-ANG90)>>ANGLETOFINESHIFT;
	
    // scale will be unit scale at SCREENWIDTH/2 distance
    basexscale = FixedDiv (finecosine[angle],centerxfrac);
    baseyscale = -FixedDiv (finesine[angle],centerxfrac);
}



// [crispy] remove MAXVISPLANES Vanilla limit
static void R_RaiseVisplanes (visplane_t** vp)
{
    if (lastvisplane - visplanes == numvisplanes)
    {
	int numvisplanes_old = numvisplanes;
	visplane_t* visplanes_old = visplanes;

	numvisplanes = numvisplanes ? 2 * numvisplanes : MAXVISPLANES;
	visplanes = crispy_realloc(visplanes, numvisplanes * sizeof(*visplanes));
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

//
// R_FindPlane
//
visplane_t*
R_FindPlane
( fixed_t	height,
  int		picnum,
  int		lightlevel )
{
    visplane_t*	check;
	
    // [crispy] add support for MBF sky tranfers
    if (picnum == skyflatnum || picnum & PL_SKYFLAT)
    {
	height = 0;			// all skys map together
	lightlevel = 0;
    }
	
    for (check=visplanes; check<lastvisplane; check++)
    {
	if (height == check->height
	    && picnum == check->picnum
	    && lightlevel == check->lightlevel)
	{
	    break;
	}
    }
    
			
    if (check < lastvisplane)
	return check;
		
    R_RaiseVisplanes(&check);
		
    lastvisplane++;

    check->height = height;
    check->picnum = picnum;
    check->lightlevel = lightlevel;
    check->minx = SCREENWIDTH;
    check->maxx = -1;
    
    memset (check->top,0xff,sizeof(check->top));
		
    return check;
}


//
// R_CheckPlane
//
visplane_t*
R_CheckPlane
( visplane_t*	pl,
  int		start,
  int		stop )
{
    int		intrl;
    int		intrh;
    int		unionl;
    int		unionh;
    int		x;
	
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

    for (x=intrl ; x<= intrh ; x++)
	if (pl->top[x] != 0xffffffffu) // [crispy] hires / 32-bit integer math
	    break;

  // [crispy] fix HOM if ceilingplane and floorplane are the same
  // visplane (e.g. both are skies)
  if (!(pl == floorplane && markceiling && floorplane == ceilingplane))
  {
    if (x > intrh)
    {
	pl->minx = unionl;
	pl->maxx = unionh;

	// use the same one
	return pl;		
    }
  }
	
    // make a new visplane
    R_RaiseVisplanes(&pl);
    lastvisplane->height = pl->height;
    lastvisplane->picnum = pl->picnum;
    lastvisplane->lightlevel = pl->lightlevel;
    
    pl = lastvisplane++;
    pl->minx = start;
    pl->maxx = stop;

    memset (pl->top,0xff,sizeof(pl->top));
		
    return pl;
}


//
// R_MakeSpans
//
void
R_MakeSpans
( int		x,
  unsigned int		t1, // [crispy] 32-bit integer math
  unsigned int		b1, // [crispy] 32-bit integer math
  unsigned int		t2, // [crispy] 32-bit integer math
  unsigned int		b2 ) // [crispy] 32-bit integer math
{
    while (t1 < t2 && t1<=b1)
    {
	R_MapPlane (t1,spanstart[t1],x-1);
	t1++;
    }
    while (b1 > b2 && b1>=t1)
    {
	R_MapPlane (b1,spanstart[b1],x-1);
	b1--;
    }
	
    while (t2 < t1 && t2<=b2)
    {
	spanstart[t2] = x;
	t2++;
    }
    while (b2 > b1 && b2>=t2)
    {
	spanstart[b2] = x;
	b2--;
    }
}

// [crispy] add support for SMMU swirling flats
// adapted from smmu/r_ripple.c, by Simon Howard
static char *R_DistortedFlat (int flatnum)
{
    const int swirlfactor = 8192 / 64;
    const int swirlfactor2 = 8192 / 32;
    const int amp = 2;
    const int amp2 = 2;
    const int speed = 40;

    static int swirltic;
    static int offset[4096];

    static char distortedflat[4096];
    char *normalflat;
    int i;

    if (swirltic != gametic)
    {
	int x, y;

	for (x = 0; x < 64; x++)
	{
	    for (y = 0; y < 64; y++)
	    {
		int x1, y1;
		int sinvalue, sinvalue2;

		sinvalue = (y * swirlfactor + leveltime * speed * 5 + 900) & 8191;
		sinvalue2 = (x * swirlfactor2 + leveltime * speed * 4 + 300) & 8191;
		x1 = x + 128
		   + ((finesine[sinvalue] * amp) >> FRACBITS)
		   + ((finesine[sinvalue2] * amp2) >> FRACBITS);

		sinvalue = (x * swirlfactor + leveltime * speed * 3 + 700) & 8191;
		sinvalue2 = (y * swirlfactor2 + leveltime * speed * 4 + 1200) & 8191;
		y1 = y + 128
		   + ((finesine[sinvalue] * amp) >> FRACBITS)
		   + ((finesine[sinvalue2] * amp2) >> FRACBITS);

		x1 &= 63;
		y1 &= 63;

		offset[(y << 6) + x] = (y1 << 6) + x1;
	    }
	}

	swirltic = gametic;
    }

    normalflat = W_CacheLumpNum(flatnum, PU_STATIC);

    for (i = 0; i < 4096; i++)
    {
	distortedflat[i] = normalflat[offset[i]];
    }

    Z_ChangeTag(normalflat, PU_CACHE);

    return distortedflat;
}

// [crispy] overflow guard for the flattranslation[] array
static inline int R_FlatTranslation (unsigned int picnum)
{
	extern int numflats;

	return (picnum < numflats) ? flattranslation[picnum] :
	       (firstflat + picnum < numlumps) ? picnum :
	       // [crispy] the most reasonable safe default,
	       // it's already too late to return "skyflatnum" at this point
	       0;
}

//
// R_DrawPlanes
// At the end of each frame.
//
void R_DrawPlanes (void)
{
    visplane_t*		pl;
    int			light;
    int			x;
    int			stop;
    int			angle;
    int                 lumpnum;
				
#ifdef RANGECHECK
    if (ds_p - drawsegs > numdrawsegs)
	I_Error ("R_DrawPlanes: drawsegs overflow (%i)",
		 ds_p - drawsegs);
    
    if (lastvisplane - visplanes > numvisplanes)
	I_Error ("R_DrawPlanes: visplane overflow (%i)",
		 lastvisplane - visplanes);
    
    if (lastopening - openings > MAXOPENINGS)
	I_Error ("R_DrawPlanes: opening overflow (%i)",
		 lastopening - openings);
#endif

    for (pl = visplanes ; pl < lastvisplane ; pl++)
    {
	boolean swirling;

	if (pl->minx > pl->maxx)
	    continue;

	
	// sky flat
	// [crispy] add support for MBF sky tranfers
	if (pl->picnum == skyflatnum || pl->picnum & PL_SKYFLAT)
	{
	    int texture;
	    angle_t an = viewangle, flip;
	    if (pl->picnum & PL_SKYFLAT)
	    {
		const line_t *l = &lines[pl->picnum & ~PL_SKYFLAT];
		const side_t *s = *l->sidenum + sides;
		texture = texturetranslation[s->toptexture];
		dc_texturemid = s->rowoffset - 28*FRACUNIT;
		// [crispy] stretch sky
		if (crispy_stretchsky)
		{
		    dc_texturemid = dc_texturemid * (textureheight[texture]>>FRACBITS) / SKYSTRETCH_HEIGHT;
		}
		flip = (l->special == 272) ? 0u : ~0u;
		an += s->textureoffset;
	    }
	    else
	    {
		texture = skytexture;
		dc_texturemid = skytexturemid;
		flip = 0;
	    }
	    dc_iscale = pspriteiscale>>(detailshift && !hires);
	    
	    // Sky is allways drawn full bright,
	    //  i.e. colormaps[0] is used.
	    // Because of this hack, sky is not affected
	    //  by INVUL inverse mapping.
	    dc_colormap = colormaps;
//	    dc_texturemid = skytexturemid;
	    dc_texheight = textureheight[texture]>>FRACBITS; // [crispy] Tutti-Frutti fix
	    // [crispy] stretch sky
	    if (crispy_stretchsky)
	        dc_iscale = dc_iscale * dc_texheight / SKYSTRETCH_HEIGHT;
	    for (x=pl->minx ; x <= pl->maxx ; x++)
	    {
		dc_yl = pl->top[x];
		dc_yh = pl->bottom[x];

		if ((unsigned) dc_yl <= dc_yh) // [crispy] 32-bit integer math
		{
		    angle = ((an + xtoviewangle[x])^flip)>>ANGLETOSKYSHIFT;
		    dc_x = x;
		    dc_source = R_GetColumn(texture, angle, false);
		    colfunc ();
		}
	    }
	    continue;
	}
	
	swirling = (R_FlatTranslation(pl->picnum) == -1);
	// regular flat
        lumpnum = firstflat + (swirling ? pl->picnum : R_FlatTranslation(pl->picnum));
	// [crispy] add support for SMMU swirling flats
	ds_source = swirling ? R_DistortedFlat(lumpnum) : W_CacheLumpNum(lumpnum, PU_STATIC);
	
	planeheight = abs(pl->height-viewz);
	light = (pl->lightlevel >> LIGHTSEGSHIFT)+extralight;

	if (light >= LIGHTLEVELS)
	    light = LIGHTLEVELS-1;

	if (light < 0)
	    light = 0;

	planezlight = zlight[light];

	pl->top[pl->maxx+1] = 0xffffffffu; // [crispy] hires / 32-bit integer math
	pl->top[pl->minx-1] = 0xffffffffu; // [crispy] hires / 32-bit integer math
		
	stop = pl->maxx + 1;

	for (x=pl->minx ; x<= stop ; x++)
	{
	    R_MakeSpans(x,pl->top[x-1],
			pl->bottom[x-1],
			pl->top[x],
			pl->bottom[x]);
	}
	
        W_ReleaseLumpNum(lumpnum);
    }
}
