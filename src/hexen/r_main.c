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


#include <math.h>
#include "m_random.h"
#include "h2def.h"
#include "m_bbox.h"
#include "r_local.h"
#include "a11y.h" // [crispy] A11Y

int viewangleoffset;

// haleyjd: removed WATCOMC

int validcount = 1;             // increment every time a check is made

lighttable_t *fixedcolormap;

int centerx, centery;
fixed_t centerxfrac, centeryfrac;
fixed_t projection;

int framecount;                 // just for profiling purposes

int sscount, linecount, loopcount;

fixed_t viewx, viewy, viewz;
angle_t viewangle;
localview_t localview; // [crispy]
fixed_t viewcos, viewsin;
player_t *viewplayer;

int detailshift;                // 0 = high, 1 = low

//
// precalculated math tables
//
angle_t clipangle;

// The viewangletox[viewangle + FINEANGLES/4] lookup maps the visible view
// angles  to screen X coordinates, flattening the arc to a flat projection
// plane.  There will be many angles mapped to the same X.
int viewangletox[FINEANGLES / 2];

// The xtoviewangleangle[] table maps a screen pixel to the lowest viewangle
// that maps back to x ranges from clipangle to -clipangle
angle_t xtoviewangle[MAXWIDTH + 1];

// [crispy] parameterized for smooth diminishing lighting
lighttable_t ***scalelight = NULL;
lighttable_t  **scalelightfixed = NULL;
lighttable_t ***zlight = NULL;

int extralight;                 // bumped light from gun blasts

// [crispy] parameterized for smooth diminishing lighting
int NUMCOLORMAPS;
int LIGHTLEVELS;
int LIGHTSEGSHIFT;
int LIGHTBRIGHT;
int MAXLIGHTSCALE;
int LIGHTSCALESHIFT;
int MAXLIGHTZ;
int LIGHTZSHIFT;

void (*colfunc) (void);
void (*basecolfunc) (void);
void (*tlcolfunc) (void);
void (*transcolfunc) (void);
void (*spanfunc) (void);

void SB_ForceRedraw(void); // [crispy] sb_bar.c

/*
===================
=
= R_AddPointToBox
=
===================
*/

/*
void R_AddPointToBox (int x, int y, fixed_t *box)
{
	if (x< box[BOXLEFT])
		box[BOXLEFT] = x;
	if (x> box[BOXRIGHT])
		box[BOXRIGHT] = x;
	if (y< box[BOXBOTTOM])
		box[BOXBOTTOM] = y;
	if (y> box[BOXTOP])
		box[BOXTOP] = y;
}
*/


/*
===============================================================================
=
= R_PointOnSide
=
= Returns side 0 (front) or 1 (back)
===============================================================================
*/

int R_PointOnSide(fixed_t x, fixed_t y, node_t * node)
{
    fixed_t dx, dy;
    fixed_t left, right;

    if (!node->dx)
    {
        if (x <= node->x)
            return node->dy > 0;
        return node->dy < 0;
    }
    if (!node->dy)
    {
        if (y <= node->y)
            return node->dx < 0;
        return node->dx > 0;
    }

    dx = (x - node->x);
    dy = (y - node->y);

// try to quickly decide by looking at sign bits
    if ((node->dy ^ node->dx ^ dx ^ dy) & 0x80000000)
    {
        if ((node->dy ^ dx) & 0x80000000)
            return 1;           // (left is negative)
        return 0;
    }

    left = FixedMul(node->dy >> FRACBITS, dx);
    right = FixedMul(dy, node->dx >> FRACBITS);

    if (right < left)
        return 0;               // front side
    return 1;                   // back side
}


int R_PointOnSegSide(fixed_t x, fixed_t y, seg_t * line)
{
    fixed_t lx, ly;
    fixed_t ldx, ldy;
    fixed_t dx, dy;
    fixed_t left, right;

    lx = line->v1->x;
    ly = line->v1->y;

    ldx = line->v2->x - lx;
    ldy = line->v2->y - ly;

    if (!ldx)
    {
        if (x <= lx)
            return ldy > 0;
        return ldy < 0;
    }
    if (!ldy)
    {
        if (y <= ly)
            return ldx < 0;
        return ldx > 0;
    }

    dx = (x - lx);
    dy = (y - ly);

// try to quickly decide by looking at sign bits
    if ((ldy ^ ldx ^ dx ^ dy) & 0x80000000)
    {
        if ((ldy ^ dx) & 0x80000000)
            return 1;           // (left is negative)
        return 0;
    }

    left = FixedMul(ldy >> FRACBITS, dx);
    right = FixedMul(dy, ldx >> FRACBITS);

    if (right < left)
        return 0;               // front side
    return 1;                   // back side
}


// [crispy] turned into a general R_PointToAngle() flavor
// called with either slope_div = SlopeDivCrispy() from R_PointToAngleCrispy()
// or slope_div = SlopeDiv() else
/*
===============================================================================
=
= R_PointToAngleSlope
=
===============================================================================
*/

#define	DBITS		(FRACBITS-SLOPEBITS)

angle_t R_PointToAngleSlope(fixed_t x, fixed_t y,
            int (*slope_div) (unsigned int num, unsigned int den))
{
    x -= viewx;
    y -= viewy;
    if ((!x) && (!y))
        return 0;
    if (x >= 0)
    {                           // x >=0
        if (y >= 0)
        {                       // y>= 0
            if (x > y)
                return tantoangle[SlopeDiv(y, x)];      // octant 0
            else
                return ANG90 - 1 - tantoangle[SlopeDiv(x, y)];  // octant 1
        }
        else
        {                       // y<0
            y = -y;
            if (x > y)
                return -tantoangle[SlopeDiv(y, x)];     // octant 8
            else
                return ANG270 + tantoangle[SlopeDiv(x, y)];     // octant 7
        }
    }
    else
    {                           // x<0
        x = -x;
        if (y >= 0)
        {                       // y>= 0
            if (x > y)
                return ANG180 - 1 - tantoangle[SlopeDiv(y, x)]; // octant 3
            else
                return ANG90 + tantoangle[SlopeDiv(x, y)];      // octant 2
        }
        else
        {                       // y<0
            y = -y;
            if (x > y)
                return ANG180 + tantoangle[SlopeDiv(y, x)];     // octant 4
            else
                return ANG270 - 1 - tantoangle[SlopeDiv(x, y)]; // octant 5
        }
    }

    return 0;
}


angle_t R_PointToAngle(fixed_t x, fixed_t y)
{
    return R_PointToAngleSlope(x, y, SlopeDiv);
}

// [crispy] overflow-safe R_PointToAngle() flavor
// called only from R_CheckBBox(), R_AddLine() and P_SegLengths()
angle_t R_PointToAngleCrispy(fixed_t x, fixed_t y)
{
    // [crispy] fix overflows for very long distances
    int64_t y_viewy = (int64_t)y - viewy;
    int64_t x_viewx = (int64_t)x - viewx;

    // [crispy] the worst that could happen is e.g. INT_MIN-INT_MAX = 2*INT_MIN
    if (x_viewx < INT_MIN || x_viewx > INT_MAX ||
        y_viewy < INT_MIN || y_viewy > INT_MAX)
    {
        // [crispy] preserving the angle by halfing the distance in both directions
        x = x_viewx / 2 + viewx;
        y = y_viewy / 2 + viewy;
    }

    return R_PointToAngleSlope(x, y, SlopeDivCrispy);
}


angle_t R_PointToAngle2(fixed_t x1, fixed_t y1, fixed_t x2, fixed_t y2)
{
    viewx = x1;
    viewy = y1;
    // [crispy] R_PointToAngle2() is never called during rendering
    return R_PointToAngleSlope(x2, y2, SlopeDiv);
}


fixed_t R_PointToDist(fixed_t x, fixed_t y)
{
    int angle;
    fixed_t dx, dy, temp;
    fixed_t dist;

    dx = abs(x - viewx);
    dy = abs(y - viewy);

    if (dy > dx)
    {
        temp = dx;
        dx = dy;
        dy = temp;
    }

    angle =
        (tantoangle[FixedDiv(dy, dx) >> DBITS] + ANG90) >> ANGLETOFINESHIFT;

    dist = FixedDiv(dx, finesine[angle]);       // use as cosine

    return dist;
}



/*
=================
=
= R_InitPointToAngle
=
=================
*/

void R_InitPointToAngle(void)
{
// now getting from tables.c
#if 0
    int i;
    long t;
    float f;
//
// slope (tangent) to angle lookup
//
    for (i = 0; i <= SLOPERANGE; i++)
    {
        f = atan((float) i / SLOPERANGE) / (3.141592657 * 2);
        t = 0xffffffff * f;
        tantoangle[i] = t;
    }
#endif
}

//=============================================================================

// [crispy] WiggleFix: move R_ScaleFromGlobalAngle function to r_segs.c, above
// R_StoreWallRange
#if 0
/*
================
=
= R_ScaleFromGlobalAngle
=
= Returns the texture mapping scale for the current line at the given angle
= rw_distance must be calculated first
================
*/

fixed_t R_ScaleFromGlobalAngle(angle_t visangle)
{
    fixed_t scale;
    int anglea, angleb;
    int sinea, sineb;
    fixed_t num, den;

#if 0
    {
        fixed_t dist, z;
        fixed_t sinv, cosv;

        sinv = finesine[(visangle - rw_normalangle) >> ANGLETOFINESHIFT];
        dist = FixedDiv(rw_distance, sinv);
        cosv = finecosine[(viewangle - visangle) >> ANGLETOFINESHIFT];
        z = abs(FixedMul(dist, cosv));
        scale = FixedDiv(projection, z);
        return scale;
    }
#endif

    anglea = ANG90 + (visangle - viewangle);
    angleb = ANG90 + (visangle - rw_normalangle);
// bothe sines are allways positive
    sinea = finesine[anglea >> ANGLETOFINESHIFT];
    sineb = finesine[angleb >> ANGLETOFINESHIFT];
    num = FixedMul(projection, sineb) << detailshift;
    den = FixedMul(rw_distance, sinea);
    if (den > num >> 16)
    {
        scale = FixedDiv(num, den);
        if (scale > 64 * FRACUNIT)
            scale = 64 * FRACUNIT;
        else if (scale < 256)
            scale = 256;
    }
    else
        scale = 64 * FRACUNIT;

    return scale;
}
#endif


// [AM] Interpolate between two angles.
angle_t R_InterpolateAngle(angle_t oangle, angle_t nangle, fixed_t scale)
{
    if (nangle == oangle)
        return nangle;
    else if (nangle > oangle)
    {
        if (nangle - oangle < ANG270)
            return oangle + (angle_t)((nangle - oangle) * FIXED2DOUBLE(scale));
        else // Wrapped around
            return oangle - (angle_t)((oangle - nangle) * FIXED2DOUBLE(scale));
    }
    else // nangle < oangle
    {
        if (oangle - nangle < ANG270)
            return oangle - (angle_t)((oangle - nangle) * FIXED2DOUBLE(scale));
        else // Wrapped around
            return oangle + (angle_t)((nangle - oangle) * FIXED2DOUBLE(scale));
    }
}

/*
=================
=
= R_InitTables
=
=================
*/

void R_InitTables(void)
{
// now getting from tables.c
#if 0
    int i;
    float a, fv;
    int t;

//
// viewangle tangent table
//
    for (i = 0; i < FINEANGLES / 2; i++)
    {
        a = (i - FINEANGLES / 4 + 0.5) * PI * 2 / FINEANGLES;
        fv = FRACUNIT * tan(a);
        t = fv;
        finetangent[i] = t;
    }

//
// finesine table
//
    for (i = 0; i < 5 * FINEANGLES / 4; i++)
    {
// OPTIMIZE: mirror...
        a = (i + 0.5) * PI * 2 / FINEANGLES;
        t = FRACUNIT * sin(a);
        finesine[i] = t;
    }
#endif

}

// [crispy] in widescreen mode, make sure the same number of horizontal
// pixels shows the same part of the game scene as in regular rendering mode
static int scaledviewwidth_nonwide, viewwidth_nonwide;
static fixed_t centerxfrac_nonwide;

/*
=================
=
= R_InitTextureMapping
=
=================
*/

void R_InitTextureMapping(void)
{
    int i;
    int x;
    int t;
    fixed_t focallength;


//
// use tangent table to generate viewangletox
// viewangletox will give the next greatest x after the view angle
//
    // calc focallength so FIELDOFVIEW angles covers SCREENWIDTH
    focallength = FixedDiv(centerxfrac_nonwide,
                           finetangent[FINEANGLES / 4 + FIELDOFVIEW / 2]);

    for (i = 0; i < FINEANGLES / 2; i++)
    {
        if (finetangent[i] > FRACUNIT * 2)
            t = -1;
        else if (finetangent[i] < -FRACUNIT * 2)
            t = viewwidth + 1;
        else
        {
            t = FixedMul(finetangent[i], focallength);
            t = (centerxfrac - t + FRACUNIT - 1) >> FRACBITS;
            if (t < -1)
                t = -1;
            else if (t > viewwidth + 1)
                t = viewwidth + 1;
        }
        viewangletox[i] = t;
    }

//
// scan viewangletox[] to generate xtoviewangleangle[]
//
// xtoviewangle will give the smallest view angle that maps to x
    for (x = 0; x <= viewwidth; x++)
    {
        i = 0;
        while (viewangletox[i] > x)
            i++;
        xtoviewangle[x] = (i << ANGLETOFINESHIFT) - ANG90;
    }

//
// take out the fencepost cases from viewangletox
//
    for (i = 0; i < FINEANGLES / 2; i++)
    {
        t = FixedMul(finetangent[i], focallength);
        t = centerx - t;
        if (viewangletox[i] == -1)
            viewangletox[i] = 0;
        else if (viewangletox[i] == viewwidth + 1)
            viewangletox[i] = viewwidth;
    }

    clipangle = xtoviewangle[0];
}

//=============================================================================

/*
====================
=
= R_InitLightTables
=
= Only inits the zlight table, because the scalelight table changes
= with view size
=
====================
*/

#define		DISTMAP	2

void R_InitLightTables(void)
{
    int i, j, level, startmap;
    int scale;

    if (scalelight)
    {
        for (i = 0; i < LIGHTLEVELS; i++)
        {
            free(scalelight[i]);
        }
        free(scalelight);
    }

    if (scalelightfixed)
    {
        free(scalelightfixed);
    }

    if (zlight)
    {
        for (i = 0; i < LIGHTLEVELS; i++)
        {
            free(zlight[i]);
        }
        free(zlight);
    }

    // [crispy] smooth diminishing lighting
    if (crispy->smoothlight)
    {
#ifdef CRISPY_TRUECOLOR
        if (crispy->truecolor)
        {
            // [crispy] if in TrueColor mode, use smoothest diminished lighting
            LIGHTLEVELS =      16 << 4;
            LIGHTSEGSHIFT =     4 -  4;
            LIGHTBRIGHT =       1 << 4;
            MAXLIGHTSCALE =    48 << 3;
            LIGHTSCALESHIFT =  12 -  3;
            MAXLIGHTZ =       128 << 6;
            LIGHTZSHIFT =      20 -  6;
        }
        else
#endif
        {
            // [crispy] else, use paletted approach
            LIGHTLEVELS =      16 << 1;
            LIGHTSEGSHIFT =     4 -  1;
            LIGHTBRIGHT =       1 << 1;
            MAXLIGHTSCALE =    48 << 0;
            LIGHTSCALESHIFT =  12 -  0;
            MAXLIGHTZ =       128 << 3;
            LIGHTZSHIFT =      20 -  3;
        }
    }
    else
    {
        LIGHTLEVELS =      16;
        LIGHTSEGSHIFT =     4;
        LIGHTBRIGHT =       1;
        MAXLIGHTSCALE =    48;
        LIGHTSCALESHIFT =  12;
        MAXLIGHTZ =       128;
        LIGHTZSHIFT =      20;
    }

    scalelight = malloc(LIGHTLEVELS * sizeof(*scalelight));
    scalelightfixed = malloc(MAXLIGHTSCALE * sizeof(*scalelightfixed));
    zlight = malloc(LIGHTLEVELS * sizeof(*zlight));

//
// Calculate the light levels to use for each level / distance combination
//
    for (i = 0; i < LIGHTLEVELS; i++)
    {
        startmap = ((LIGHTLEVELS - LIGHTBRIGHT - i) * 2) * NUMCOLORMAPS / LIGHTLEVELS;
        scalelight[i] = malloc(MAXLIGHTSCALE * sizeof(**scalelight));
        zlight[i] = malloc(MAXLIGHTZ * sizeof(**zlight));
        for (j = 0; j < MAXLIGHTZ; j++)
        {
            scale =
                FixedDiv((ORIGWIDTH / 2 * FRACUNIT),
                         (j + 1) << LIGHTZSHIFT);
            scale >>= LIGHTSCALESHIFT;
            level = startmap - scale / DISTMAP;
            if (level < 0)
                level = 0;
            if (level >= NUMCOLORMAPS)
                level = NUMCOLORMAPS - 1;
            zlight[i][j] = colormaps + level * 256;
        }
    }
}


/*
==============
=
= R_SetViewSize
=
= Don't really change anything here, because i might be in the middle of
= a refresh.  The change will take effect next refresh.
=
==============
*/

boolean setsizeneeded;
int setblocks, setdetail;

void R_SetViewSize(int blocks, int detail)
{
    setsizeneeded = true;
    setblocks = blocks;
    setdetail = detail;
}

/*
==============
=
= R_ExecuteSetViewSize
=
==============
*/

void R_ExecuteSetViewSize(void)
{
    fixed_t cosadj, dy;
    int i, j, level, startmap;

    setsizeneeded = false;

    if (setblocks == 11)
    {
        scaledviewwidth_nonwide = NONWIDEWIDTH;
        scaledviewwidth = SCREENWIDTH;
        viewheight = SCREENHEIGHT;
    }
    else
    {
        scaledviewwidth_nonwide = (setblocks * 32) << crispy->hires;
        viewheight = (setblocks * 161 / 10) << crispy->hires;

	// [crispy] regular viewwidth in non-widescreen mode
	if (crispy->widescreen)
	{
		const int widescreen_edge_aligner = (16 << crispy->hires) - 1;

		scaledviewwidth = viewheight*SCREENWIDTH/(SCREENHEIGHT-SBARHEIGHT);
		// [crispy] make sure scaledviewwidth is an integer multiple of the bezel patch width
		scaledviewwidth = (scaledviewwidth + widescreen_edge_aligner) & (int)~widescreen_edge_aligner;
		scaledviewwidth = MIN(scaledviewwidth, SCREENWIDTH);
	}
	else
	{
		scaledviewwidth = scaledviewwidth_nonwide;
	}
    }

    detailshift = setdetail;
    viewwidth = scaledviewwidth >> detailshift;
    viewwidth_nonwide = scaledviewwidth_nonwide >> detailshift;

    centery = viewheight / 2;
    centerx = viewwidth / 2;
    centerxfrac = centerx << FRACBITS;
    centeryfrac = centery << FRACBITS;
    centerxfrac_nonwide = (viewwidth_nonwide / 2) << FRACBITS;
    projection = centerxfrac_nonwide;

    if (!detailshift)
    {
        colfunc = basecolfunc = R_DrawColumn;
        tlcolfunc = R_DrawTLColumn;
        transcolfunc = R_DrawTranslatedColumn;
        spanfunc = R_DrawSpan;
    }
    else
    {
        colfunc = basecolfunc = R_DrawColumnLow;
        tlcolfunc = R_DrawTLColumn;
        transcolfunc = R_DrawTranslatedColumn;
        spanfunc = R_DrawSpanLow;
    }

    R_InitBuffer(scaledviewwidth, viewheight);

    R_InitTextureMapping();

//
// psprite scales
//
    pspritescale = FRACUNIT * viewwidth_nonwide / ORIGWIDTH;
    pspriteiscale = FRACUNIT * ORIGWIDTH / viewwidth_nonwide;

//
// thing clipping
//
    for (i = 0; i < viewwidth; i++)
        screenheightarray[i] = viewheight;

//
// planes
//
    for (i = 0; i < viewheight; i++)
    {
        // [crispy] re-generate lookup-table for yslope[] (free look)
        // whenever "detailshift" or "screenblocks" change
        const fixed_t num = (viewwidth_nonwide<<detailshift)/2*FRACUNIT;
        for (j = 0; j < LOOKDIRS; j++)
        {
        dy = ((i - (viewheight / 2 + ((j - LOOKDIRMIN) * (1 << crispy->hires)) *
                (screenblocks < 11 ? screenblocks : 11) / 10)) << FRACBITS) +
                FRACUNIT / 2;
        dy = abs(dy);
        yslopes[j][i] = FixedDiv(num, dy);
        }
    }
    yslope = yslopes[LOOKDIRMIN];

    for (i = 0; i < viewwidth; i++)
    {
        cosadj = abs(finecosine[xtoviewangle[i] >> ANGLETOFINESHIFT]);
        distscale[i] = FixedDiv(FRACUNIT, cosadj);
    }

//
// Calculate the light levels to use for each level / scale combination
//
    for (i = 0; i < LIGHTLEVELS; i++)
    {
        startmap = ((LIGHTLEVELS - LIGHTBRIGHT - i) * 2) * NUMCOLORMAPS / LIGHTLEVELS;
        for (j = 0; j < MAXLIGHTSCALE; j++)
        {
            level =
                startmap -
                j * NONWIDEWIDTH / (viewwidth_nonwide << detailshift) / DISTMAP;
            if (level < 0)
                level = 0;
            if (level >= NUMCOLORMAPS)
                level = NUMCOLORMAPS - 1;
            scalelight[i][j] = colormaps + level * 256;
        }
    }

//
// draw the border
//
    R_DrawViewBorder();         // erase old menu stuff

    // [crispy] Redraw status bar needed for widescreen HUD
    SB_ForceRedraw();

    pspr_interp = false; // [crispy]
}


/*
==============
=
= R_Init
=
==============
*/

int detailLevel;
int screenblocks = 10;

void R_Init(void)
{
    R_InitData();
    R_InitPointToAngle();
    R_InitTables();
    // viewwidth / viewheight / detailLevel are set by the defaults
    R_SetViewSize(BETWEEN(3, 11, screenblocks), detailLevel);
    R_InitPlanes();
    R_InitLightTables();
    R_InitSkyMap();
    R_InitTranslationTables();
    framecount = 0;
}

/*
==============
=
= R_PointInSubsector
=
==============
*/

subsector_t *R_PointInSubsector(fixed_t x, fixed_t y)
{
    node_t *node;
    int side, nodenum;

    if (!numnodes)              // single subsector is a special case
        return subsectors;

    nodenum = numnodes - 1;

    while (!(nodenum & NF_SUBSECTOR))
    {
        node = &nodes[nodenum];
        side = R_PointOnSide(x, y, node);
        nodenum = node->children[side];
    }

    return &subsectors[nodenum & ~NF_SUBSECTOR];

}

// [crispy]
static inline boolean CheckLocalView(const player_t *player)
{
  return (
    // Don't use localview if the player is spying.
    player == &players[consoleplayer] &&
    // Don't use localview if the player is dead.
    player->playerstate != PST_DEAD &&
    // Don't use localview if the player just teleported.
    !player->mo->reactiontime &&
    // Don't use localview if a demo is playing.
    !demoplayback &&
    // Don't use localview during a netgame (single-player only).
    !netgame
  );
}

//----------------------------------------------------------------------------
//
// PROC R_SetupFrame
//
//----------------------------------------------------------------------------

void R_SetupFrame(player_t * player)
{
    int i;
    int tableAngle;
    int tempCentery;
    int intensity;
    static int x_quake, y_quake, quaketime; // [crispy]
    int pitch; // [crispy]
    int tmpColormap; // [crispy] to overwrite colormap

    //drawbsp = 1;
    viewplayer = player;
    // haleyjd: removed WATCOMC
    // haleyjd FIXME: viewangleoffset handling?
    if (crispy->uncapped && leveltime > 1 && player->mo->interp == true && leveltime > oldleveltime)
    {
        const boolean use_localview = CheckLocalView(player);

        viewx = LerpFixed(player->mo->oldx, player->mo->x);
        viewy = LerpFixed(player->mo->oldy, player->mo->y);
        viewz = LerpFixed(player->oldviewz, player->viewz);
        if (use_localview)
        {
            viewangle = (player->mo->angle + localview.angle -
                        localview.ticangle + LerpAngle(localview.oldticangle,
                                                       localview.ticangle)) + viewangleoffset;
        }
        else
        {
            viewangle = LerpAngle(player->mo->oldangle, player->mo->angle) + viewangleoffset;
        }

        pitch = LerpInt(player->oldlookdir, player->lookdir);
    }
    else
    {
        viewangle = player->mo->angle + viewangleoffset;
        viewx = player->mo->x;
        viewy = player->mo->y;
        viewz = player->viewz;
        pitch = player->lookdir; // [crispy]
    }

    if (localQuakeHappening[displayplayer] && !paused)
    {
        // [crispy] only get new quake values once every gametic
        if (leveltime > quaketime)
        {
            intensity = localQuakeHappening[displayplayer];
            x_quake = ((M_Random() % (intensity << 2))
                           - (intensity << 1)) << FRACBITS;
            y_quake = ((M_Random() % (intensity << 2))
                           - (intensity << 1)) << FRACBITS;
            quaketime = leveltime;
        }

        if (crispy->uncapped)
        {
            viewx += FixedMul(x_quake, fractionaltic);
            viewy += FixedMul(y_quake, fractionaltic);
        }
        else
        {
            viewx += x_quake;
            viewy += y_quake;
        }
    }
    else if (!localQuakeHappening[displayplayer])
    {
        quaketime = 0;
    }

    // [crispy] A11Y - even if not used by Hexen
    if (a11y_weapon_flash)
    {
    extralight = player->extralight;
    }
    else
        extralight = 0;
    // [crispy] A11Y
    extralight += a11y_extra_lighting;

    tableAngle = viewangle >> ANGLETOFINESHIFT;

    // [crispy] apply new yslope[] whenever "lookdir", "detailshift" or
    // "screenblocks" change
    tempCentery = viewheight / 2 + (pitch * (1 << crispy->hires)) *
                    (screenblocks < 11 ? screenblocks : 11) / 10;
    if (centery != tempCentery)
    {
        centery = tempCentery;
        centeryfrac = centery << FRACBITS;
        yslope = yslopes[LOOKDIRMIN + pitch];
    }
    viewsin = finesine[tableAngle];
    viewcos = finecosine[tableAngle];
    sscount = 0;
    if (player->fixedcolormap)
    {
        tmpColormap = player->fixedcolormap;
        // [crispy] A11Y - overwrite to disable torch flickering
        if (!a11y_weapon_flash && player->powers[pw_infrared])
        {
            tmpColormap = 1; // [crispy] A11Y - static infrared map
        }

        fixedcolormap = colormaps + tmpColormap
            * (NUMCOLORMAPS / 32) // [crispy] smooth diminishing lighting
            // [crispy] sizeof(lighttable_t) not needed in paletted render
            // and breaks Torch's fixed colormap indexes in true color render
            * 256 /* * sizeof(lighttable_t)*/;
        walllights = scalelightfixed;
        for (i = 0; i < MAXLIGHTSCALE; i++)
        {
            scalelightfixed[i] = fixedcolormap;
        }
    }
    else
    {
        fixedcolormap = 0;
    }
    framecount++;
    validcount++;
    if (BorderNeedRefresh)
    {
        if (setblocks < 10)
        {
            R_DrawViewBorder();
        }
        BorderNeedRefresh = false;
        BorderTopRefresh = false;
        UpdateState |= I_FULLSCRN;
    }
    if (BorderTopRefresh)
    {
        if (setblocks < 10)
        {
            R_DrawTopBorder();
        }
        BorderTopRefresh = false;
        UpdateState |= I_MESSAGES;
    }

#if 0
    {
        static int frame;
        memset(screen, frame, SCREENWIDTH * SCREENHEIGHT);
        frame++;
    }
#endif
}

/*
==============
=
= R_RenderView
=
==============
*/

void R_RenderPlayerView(player_t * player)
{
    extern void PO_InterpolatePolyObjects(void);
    extern boolean automapactive;

    R_SetupFrame(player);
    R_ClearClipSegs();
    R_ClearDrawSegs();
    R_ClearPlanes();
    R_ClearSprites();

    if (automapactive && !crispy->automapoverlay)
    {
        R_RenderBSPNode(numnodes - 1);
        return;
    }

    NetUpdate();                // check for new console commands
    PO_InterpolatePolyObjects(); // [crispy] Interpolate polyobjects here
    R_InterpolateTextureOffsets(); // [crispy] Smooth texture scrolling

    // Make displayed player invisible locally
    if (localQuakeHappening[displayplayer] && gamestate == GS_LEVEL)
    {
        players[displayplayer].mo->flags2 |= MF2_DONTDRAW;
        R_RenderBSPNode(numnodes - 1);  // head node is the last node output
        players[displayplayer].mo->flags2 &= ~MF2_DONTDRAW;
    }
    else
    {
        R_RenderBSPNode(numnodes - 1);  // head node is the last node output
    }

    NetUpdate();                // check for new console commands
    R_DrawPlanes();
    NetUpdate();                // check for new console commands
    R_DrawMasked();
    NetUpdate();                // check for new console commands
}
