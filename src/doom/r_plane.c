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
#include "r_bmaps.h" // [crispy] R_BrightmapForTexName()
#include "r_swirl.h" // [crispy] R_DistortedFlat()



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
#define MAXOPENINGS	MAXWIDTH*64*4
int			openings[MAXOPENINGS]; // [crispy] 32-bit integer math
int*			lastopening; // [crispy] 32-bit integer math


//
// Clip values are the solid pixel bounding the range.
//  floorclip starts out SCREENHEIGHT
//  ceilingclip starts out -1
//
int			floorclip[MAXWIDTH]; // [crispy] 32-bit integer math
int			ceilingclip[MAXWIDTH]; // [crispy] 32-bit integer math

//
// spanstart holds the start of a plane span
// initialized to 0 at start
//
int			spanstart[MAXHEIGHT];
int			spanstop[MAXHEIGHT];

//
// texture mapping
//
lighttable_t**		planezlight;
fixed_t			planeheight;

fixed_t*			yslope;
fixed_t			yslopes[LOOKDIRS][MAXHEIGHT];
fixed_t			distscale[MAXWIDTH];
fixed_t			basexscale;
fixed_t			baseyscale;

fixed_t			cachedheight[MAXHEIGHT];
fixed_t			cacheddistance[MAXHEIGHT];
fixed_t			cachedxstep[MAXHEIGHT];
fixed_t			cachedystep[MAXHEIGHT];



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
	distance = cacheddistance[y] = FixedMul (planeheight, yslope[y]);
	ds_xstep = cachedxstep[y] = FixedDiv(FixedMul (viewsin, planeheight), dy) << detailshift;
	ds_ystep = cachedystep[y] = FixedDiv(FixedMul (viewcos, planeheight), dy) << detailshift;
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
	ds_colormap[0] = ds_colormap[1] = fixedcolormap;
    else
    {
	index = distance >> LIGHTZSHIFT;
	
	if (index >= MAXLIGHTZ )
	    index = MAXLIGHTZ-1;

	ds_colormap[0] = planezlight[index];
	ds_colormap[1] = colormaps;
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
	lightlevel = 0;   // killough 7/19/98: most skies map together

	// haleyjd 05/06/08: but not all. If height > viewpoint.z, set height to 1
	// instead of 0, to keep ceilings mapping with ceilings, and floors mapping
	// with floors.
	if (height > viewz)
	    height = 1;
	else
	    height = 0;
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
		
    R_RaiseVisplanes(&check); // [crispy] remove VISPLANES limit
    if (lastvisplane - visplanes == MAXVISPLANES && false)
	I_Error ("R_FindPlane: no more visplanes");
		
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
    R_RaiseVisplanes(&pl); // [crispy] remove VISPLANES limit
    lastvisplane->height = pl->height;
    lastvisplane->picnum = pl->picnum;
    lastvisplane->lightlevel = pl->lightlevel;
    
    if (lastvisplane - visplanes == MAXVISPLANES && false) // [crispy] remove VISPLANES limit
	I_Error ("R_CheckPlane: no more visplanes");

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
	I_Error ("R_DrawPlanes: drawsegs overflow (%td)",
		 ds_p - drawsegs);
    
    if (lastvisplane - visplanes > numvisplanes)
	I_Error ("R_DrawPlanes: visplane overflow (%td)",
		 lastvisplane - visplanes);
    
    if (lastopening - openings > MAXOPENINGS)
	I_Error ("R_DrawPlanes: opening overflow (%td)",
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
		flip = (l->special == 272) ? 0u : ~0u;
		an += s->textureoffset;
	    }
	    else
	    {
		texture = skytexture;
		dc_texturemid = skytexturemid;
		flip = 0;
	    }
	    dc_iscale = pspriteiscale>>detailshift;
	    
	    // Sky is allways drawn full bright,
	    //  i.e. colormaps[0] is used.
	    // Because of this hack, sky is not affected
	    //  by INVUL inverse mapping.
	    // [crispy] no brightmaps for sky
	    dc_colormap[0] = dc_colormap[1] = colormaps;
	    dc_texheight = textureheight[texture]>>FRACBITS; // [crispy] Tutti-Frutti fix

	    // [crispy] stretch short skies
	    if (crispy->stretchsky && dc_texheight < 200)
	    {
	        dc_iscale = dc_iscale * dc_texheight / SKYSTRETCH_HEIGHT;
	        dc_texturemid = dc_texturemid * dc_texheight / SKYSTRETCH_HEIGHT;
	    }

	    for (x=pl->minx ; x <= pl->maxx ; x++)
	    {
		dc_yl = pl->top[x];
		dc_yh = pl->bottom[x];

		if ((unsigned) dc_yl <= dc_yh) // [crispy] 32-bit integer math
		{
		    angle = ((an + xtoviewangle[x])^flip)>>ANGLETOSKYSHIFT;
		    dc_x = x;
		    dc_source = R_GetColumn(texture, angle);
		    colfunc ();
		}
	    }
	    continue;
	}
	
	swirling = (flattranslation[pl->picnum] == -1);
	// regular flat
        lumpnum = firstflat + (swirling ? pl->picnum : flattranslation[pl->picnum]);
	// [crispy] add support for SMMU swirling flats
	ds_source = swirling ? R_DistortedFlat(lumpnum) : W_CacheLumpNum(lumpnum, PU_STATIC);
	ds_brightmap = R_BrightmapForFlatNum(lumpnum-firstflat);
	
	planeheight = abs(pl->height-viewz);
	light = (pl->lightlevel >> LIGHTSEGSHIFT)+(extralight * LIGHTBRIGHT);

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
