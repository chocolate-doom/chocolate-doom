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
//
// DESCRIPTION:  the automap code
//


#include <stdio.h>

#include "deh_main.h"

#include "z_zone.h"
#include "doomkeys.h"
#include "doomdef.h"
#include "st_stuff.h"
#include "p_local.h"
#include "w_wad.h"

#include "m_cheat.h"
#include "m_controls.h"
#include "m_misc.h"
#include "i_system.h"
#include "i_timer.h"
#include "i_video.h"

// Needs access to LFB.
#include "v_video.h"

// State.
#include "doomstat.h"
#include "r_state.h"

// Data.
#include "dstrings.h"

#include "am_map.h"
extern boolean inhelpscreens; // [crispy]


// For use if I do walls with outsides/insides
#define REDS		(256-5*16)
#define REDRANGE	16
#define BLUES		(256-4*16+8)
#define BLUERANGE	8
#define GREENS		(7*16)
#define GREENRANGE	16
#define GRAYS		(6*16)
#define GRAYSRANGE	16
#define BROWNS		(4*16)
#define BROWNRANGE	16
#define YELLOWS		(256-32+7)
#define YELLOWRANGE	1
#define BLACK		0
#define WHITE		(256-47)

// Automap colors
#define BACKGROUND	BLACK
#define YOURCOLORS	WHITE
#define YOURRANGE	0
#define WALLCOLORS	(crispy->extautomap ? REDS+4 : REDS) // [crispy] slightly darker red
#define WALLRANGE	REDRANGE
#define TSWALLCOLORS	GRAYS
#define TSWALLRANGE	GRAYSRANGE
#define FDWALLCOLORS	(crispy->extautomap ? BROWNS+6 : BROWNS) // [crispy] darker brown
#define FDWALLRANGE	BROWNRANGE
#define CDWALLCOLORS	(crispy->extautomap ? 163 : YELLOWS) // [crispy] golden yellow
#define CDWALLRANGE	YELLOWRANGE
#define THINGCOLORS	GREENS
#define THINGRANGE	GREENRANGE
#define SECRETWALLCOLORS	252 // [crispy] purple
#define CRISPY_HIGHLIGHT_REVEALED_SECRETS
#define REVEALEDSECRETWALLCOLORS	112 // [crispy] green
#define SECRETWALLRANGE WALLRANGE
#define GRIDCOLORS	(GRAYS + GRAYSRANGE/2)
#define GRIDRANGE	0
#define XHAIRCOLORS	GRAYS

// [crispy] FRACTOMAPBITS: overflow-safe coordinate system.
// Written by Andrey Budko (entryway), adapted from prboom-plus/src/am_map.*
#define MAPBITS 12
#define MAPUNIT (1<<MAPBITS)
#define FRACTOMAPBITS (FRACBITS-MAPBITS)

// [crispy] New radius to use with FRACTOMAPBITS, since orginal 
// PLAYERRADIUS macro can't be used in this implementation.
#define MAPPLAYERRADIUS (16*(1<<MAPBITS))

// drawing stuff

#define AM_NUMMARKPOINTS 10

// scale on entry
#define INITSCALEMTOF (.2*FRACUNIT)
// how much the automap moves window per tic in frame-buffer coordinates
// moves 140 pixels in 1 second
#define F_PANINC	4
// [crispy] pan faster by holding run button
#define F2_PANINC	12
// how much zoom-in per tic
// goes to 2x in 1 second
#define M_ZOOMIN        ((int) (1.02*FRACUNIT))
// how much zoom-out per tic
// pulls out to 0.5x in 1 second
#define M_ZOOMOUT       ((int) (FRACUNIT/1.02))
// [crispy] zoom faster with the mouse wheel
#define M2_ZOOMIN       ((int) (1.08*FRACUNIT))
#define M2_ZOOMOUT      ((int) (FRACUNIT/1.08))
#define M2_ZOOMINFAST   ((int) (1.5*FRACUNIT))
#define M2_ZOOMOUTFAST  ((int) (FRACUNIT/1.5))
// [crispy] toggleable pan/zoom speed
static int f_paninc;
static int m_zoomin_kbd;
static int m_zoomout_kbd;
static int m_zoomin_mouse;
static int m_zoomout_mouse;
static boolean mousewheelzoom;

// translates between frame-buffer and map distances
// [crispy] fix int overflow that causes map and grid lines to disappear
#define FTOM(x) (((int64_t)((x)<<FRACBITS) * scale_ftom) >> FRACBITS)
#define MTOF(x) ((((int64_t)(x) * scale_mtof) >> FRACBITS)>>FRACBITS)
// translates between frame-buffer and map coordinates
#define CXMTOF(x)  (f_x + MTOF((x)-m_x))
#define CYMTOF(y)  (f_y + (f_h - MTOF((y)-m_y)))

// the following is crap
#define LINE_NEVERSEE ML_DONTDRAW

typedef struct
{
    int x, y;
} fpoint_t;

typedef struct
{
    fpoint_t a, b;
} fline_t;

typedef struct
{
    int64_t		x,y;
} mpoint_t;

typedef struct
{
    mpoint_t a, b;
} mline_t;

typedef struct
{
    fixed_t slp, islp;
} islope_t;

typedef enum
{
    no_key,
    red_key,
    yellow_key,
    blue_key
} keycolor_t;


//
// The vector graphics for the automap.
//  A line drawing of the player pointing right,
//   starting from the middle.
//
#define R ((8*MAPPLAYERRADIUS)/7)
mline_t player_arrow[] = {
    { { -R+R/8, 0 }, { R, 0 } }, // -----
    { { R, 0 }, { R-R/2, R/4 } },  // ----->
    { { R, 0 }, { R-R/2, -R/4 } },
    { { -R+R/8, 0 }, { -R-R/8, R/4 } }, // >---->
    { { -R+R/8, 0 }, { -R-R/8, -R/4 } },
    { { -R+3*R/8, 0 }, { -R+R/8, R/4 } }, // >>--->
    { { -R+3*R/8, 0 }, { -R+R/8, -R/4 } }
};
#undef R

#define R ((8*MAPPLAYERRADIUS)/7)
mline_t cheat_player_arrow[] = {
    { { -R+R/8, 0 }, { R, 0 } }, // -----
    { { R, 0 }, { R-R/2, R/6 } },  // ----->
    { { R, 0 }, { R-R/2, -R/6 } },
    { { -R+R/8, 0 }, { -R-R/8, R/6 } }, // >----->
    { { -R+R/8, 0 }, { -R-R/8, -R/6 } },
    { { -R+3*R/8, 0 }, { -R+R/8, R/6 } }, // >>----->
    { { -R+3*R/8, 0 }, { -R+R/8, -R/6 } },
    { { -R/2, 0 }, { -R/2, -R/6 } }, // >>-d--->
    { { -R/2, -R/6 }, { -R/2+R/6, -R/6 } },
    { { -R/2+R/6, -R/6 }, { -R/2+R/6, R/4 } },
    { { -R/6, 0 }, { -R/6, -R/6 } }, // >>-dd-->
    { { -R/6, -R/6 }, { 0, -R/6 } },
    { { 0, -R/6 }, { 0, R/4 } },
    { { R/6, R/4 }, { R/6, -R/7 } }, // >>-ddt->
    { { R/6, -R/7 }, { R/6+R/32, -R/7-R/32 } },
    { { R/6+R/32, -R/7-R/32 }, { R/6+R/10, -R/7 } }
};
#undef R

#define R (FRACUNIT)
mline_t triangle_guy[] = {
    { { (fixed_t)(-.867*R), (fixed_t)(-.5*R) }, { (fixed_t)(.867*R ), (fixed_t)(-.5*R) } },
    { { (fixed_t)(.867*R ), (fixed_t)(-.5*R) }, { (fixed_t)(0      ), (fixed_t)(R    ) } },
    { { (fixed_t)(0      ), (fixed_t)(R    ) }, { (fixed_t)(-.867*R), (fixed_t)(-.5*R) } }
};
#undef R

#define R (FRACUNIT)
mline_t thintriangle_guy[] = {
    { { (fixed_t)(-.5*R), (fixed_t)(-.7*R) }, { (fixed_t)(R    ), (fixed_t)(0    ) } },
    { { (fixed_t)(R    ), (fixed_t)(0    ) }, { (fixed_t)(-.5*R), (fixed_t)(.7*R ) } },
    { { (fixed_t)(-.5*R), (fixed_t)(.7*R ) }, { (fixed_t)(-.5*R), (fixed_t)(-.7*R) } }
};
// [crispy] print keys as crosses
static mline_t cross_mark[] = {
    { { -R, 0 }, { R, 0 } },
    { { 0, -R }, { 0, R } },
};
static mline_t square_mark[] = {
    { { -R,  0 }, {  0,  R } },
    { {  0,  R }, {  R,  0 } },
    { {  R,  0 }, {  0, -R } },
    { {  0, -R }, { -R,  0 } },
};
#undef R




static int 	cheating = 0;
static int 	grid = 0;

static int 	leveljuststarted = 1; 	// kluge until AM_LevelInit() is called

boolean    	automapactive = false;
//static int 	finit_width = SCREENWIDTH;
//static int 	finit_height = SCREENHEIGHT - (ST_HEIGHT << crispy->hires);

// location of window on screen
static int 	f_x;
static int	f_y;

// size of window on screen
static int 	f_w;
static int	f_h;

static int 	lightlev; 		// used for funky strobing effect
#define fb I_VideoBuffer // [crispy] simplify
//static pixel_t*	fb; 			// pseudo-frame buffer
static int 	amclock;

static mpoint_t m_paninc, m_paninc2; // how far the window pans each tic (map coords)
static mpoint_t m_paninc_target; // [crispy] for interpolation
static fixed_t 	mtof_zoommul; // how far the window zooms in each tic (map coords)
static fixed_t 	ftom_zoommul; // how far the window zooms in each tic (fb coords)

static int64_t 	m_x, m_y;   // LL x,y where the window is on the map (map coords)
static int64_t 	m_x2, m_y2; // UR x,y where the window is on the map (map coords)
static int64_t 	prev_m_x, prev_m_y; // [crispy] for interpolation
static int64_t 	next_m_x, next_m_y; // [crispy] for interpolation

//
// width/height of window on map (map coords)
//
static int64_t 	m_w;
static int64_t 	m_h;

// based on level size
static fixed_t 	min_x;
static fixed_t	min_y; 
static fixed_t 	max_x;
static fixed_t  max_y;

static fixed_t 	max_w; // max_x-min_x,
static fixed_t  max_h; // max_y-min_y

// based on player size
static fixed_t 	min_w;
static fixed_t  min_h;


static fixed_t 	min_scale_mtof; // used to tell when to stop zooming out
static fixed_t 	max_scale_mtof; // used to tell when to stop zooming in

// old stuff for recovery later
static int64_t old_m_w, old_m_h;
static int64_t old_m_x, old_m_y;

// used by MTOF to scale from map-to-frame-buffer coords
static fixed_t scale_mtof = (fixed_t)INITSCALEMTOF;
// used by FTOM to scale from frame-buffer-to-map coords (=1/scale_mtof)
static fixed_t scale_ftom;

static player_t *plr; // the player represented by an arrow

static patch_t *marknums[10]; // numbers used for marking by the automap
static mpoint_t markpoints[AM_NUMMARKPOINTS]; // where the points are
static int markpointnum = 0; // next point to be assigned

static int followplayer = 1; // specifies whether to follow the player around

cheatseq_t cheat_amap = CHEAT("iddt", 0);

static boolean stopped = true;

// [crispy] Antialiased lines from Heretic with more colors
#define NUMSHADES 8
#define NUMSHADES_BITS 3 // log2(NUMSHADES)
static pixel_t color_shades[NUMSHADES * 256];

// Forward declare for AM_LevelInit
static void AM_drawFline_Vanilla(fline_t* fl, int color);
static void AM_drawFline_Smooth(fline_t* fl, int color);
// Indirect through this to avoid having to test crispy->smoothmap for every line
void (*AM_drawFline)(fline_t*, int) = AM_drawFline_Vanilla;

// [crispy] automap rotate mode needs these early on
void AM_rotate (int64_t *x, int64_t *y, angle_t a);
static void AM_rotatePoint (mpoint_t *pt);
static mpoint_t mapcenter;
static angle_t mapangle;

static void AM_drawCrosshair(int color, boolean force);

// Calculates the slope and slope according to the x-axis of a line
// segment in map coordinates (with the upright y-axis n' all) so
// that it can be used with the brain-dead drawing stuff.

void
AM_getIslope
( mline_t*	ml,
  islope_t*	is )
{
    int dx, dy;

    dy = ml->a.y - ml->b.y;
    dx = ml->b.x - ml->a.x;
    if (!dy) is->islp = (dx<0?-INT_MAX:INT_MAX);
    else is->islp = FixedDiv(dx, dy);
    if (!dx) is->slp = (dy<0?-INT_MAX:INT_MAX);
    else is->slp = FixedDiv(dy, dx);

}

//
//
//
void AM_activateNewScale(void)
{
    m_x += m_w/2;
    m_y += m_h/2;
    m_w = FTOM(f_w);
    m_h = FTOM(f_h);
    m_x -= m_w/2;
    m_y -= m_h/2;
    m_x2 = m_x + m_w;
    m_y2 = m_y + m_h;
    next_m_x = m_x; // [crispy]
    next_m_y = m_y; // [crispy]
}

//
//
//
void AM_saveScaleAndLoc(void)
{
    old_m_x = m_x;
    old_m_y = m_y;
    old_m_w = m_w;
    old_m_h = m_h;
}

//
//
//
void AM_restoreScaleAndLoc(void)
{

    m_w = old_m_w;
    m_h = old_m_h;
    if (!followplayer)
    {
	m_x = old_m_x;
	m_y = old_m_y;
    } else {
	m_x = (plr->mo->x >> FRACTOMAPBITS) - m_w/2;
	m_y = (plr->mo->y >> FRACTOMAPBITS) - m_h/2;
    }
    m_x2 = m_x + m_w;
    m_y2 = m_y + m_h;
    next_m_x = m_x; // [crispy]
    next_m_y = m_y; // [crispy]

    // Change the scaling multipliers
    scale_mtof = FixedDiv(f_w<<FRACBITS, m_w);
    scale_ftom = FixedDiv(FRACUNIT, scale_mtof);
}

//
// adds a marker at the current location
//
void AM_addMark(void)
{
    // [crispy] keep the map static in overlay mode
    // if not following the player
    if (!(!followplayer && crispy->automapoverlay))
    {
    markpoints[markpointnum].x = m_x + m_w/2;
    markpoints[markpointnum].y = m_y + m_h/2;
    }
    else
    {
	markpoints[markpointnum].x = plr->mo->x >> FRACTOMAPBITS;
	markpoints[markpointnum].y = plr->mo->y >> FRACTOMAPBITS;
    }
    markpointnum = (markpointnum + 1) % AM_NUMMARKPOINTS;

}

//
// Determines bounding box of all vertices,
// sets global variables controlling zoom range.
//
void AM_findMinMaxBoundaries(void)
{
    int i;
    fixed_t a;
    fixed_t b;

    min_x = min_y =  INT_MAX;
    max_x = max_y = -INT_MAX;
  
    for (i=0;i<numvertexes;i++)
    {
	if (vertexes[i].x < min_x)
	    min_x = vertexes[i].x;
	else if (vertexes[i].x > max_x)
	    max_x = vertexes[i].x;
    
	if (vertexes[i].y < min_y)
	    min_y = vertexes[i].y;
	else if (vertexes[i].y > max_y)
	    max_y = vertexes[i].y;
    }
  
    // [crispy] cope with huge level dimensions which span the entire INT range
    max_w = (max_x >>= FRACTOMAPBITS) - (min_x >>= FRACTOMAPBITS);
    max_h = (max_y >>= FRACTOMAPBITS) - (min_y >>= FRACTOMAPBITS);

    min_w = 2*MAPPLAYERRADIUS; // const? never changed?
    min_h = 2*MAPPLAYERRADIUS;

    a = FixedDiv(f_w<<FRACBITS, max_w);
    b = FixedDiv(f_h<<FRACBITS, max_h);
  
    min_scale_mtof = a < b ? a : b;
    max_scale_mtof = FixedDiv(f_h<<FRACBITS, 2*MAPPLAYERRADIUS);

}

// [crispy] Function called by AM_Ticker for stable panning interpolation
static void AM_changeWindowLocTick(void)
{
    int64_t incx, incy;

    incx = m_paninc_target.x;
    incy = m_paninc_target.y;

    if (m_paninc_target.x || m_paninc_target.y)
    {
        followplayer = 0;
    }

    if (crispy->automaprotate)
    {
        AM_rotate(&incx, &incy, -mapangle);
    }

    next_m_x += incx;
    next_m_y += incy;

    // next_m_x and next_m_y clipping happen in AM_changeWindowLoc
}

//
//
//
void AM_changeWindowLoc(void)
{
    int64_t incx, incy;

    // [crispy] accumulate automap panning by keyboard and mouse
    if (crispy->uncapped && leveltime > oldleveltime)
    {
        incx = FixedMul(m_paninc_target.x, fractionaltic);
        incy = FixedMul(m_paninc_target.y, fractionaltic);
    }
    else
    {
        incx = m_paninc_target.x;
        incy = m_paninc_target.y;
    }
    if (crispy->automaprotate)
    {
	AM_rotate(&incx, &incy, -mapangle);
    }
    m_x = prev_m_x + incx;
    m_y = prev_m_y + incy;

    if (m_x + m_w/2 > max_x)
	next_m_x = m_x = max_x - m_w/2;
    else if (m_x + m_w/2 < min_x)
	next_m_x = m_x = min_x - m_w/2;
  
    if (m_y + m_h/2 > max_y)
	next_m_y = m_y = max_y - m_h/2;
    else if (m_y + m_h/2 < min_y)
	next_m_y = m_y = min_y - m_h/2;

    m_x2 = m_x + m_w;
    m_y2 = m_y + m_h;
}


//
//
//
void AM_initVariables(void)
{
    static event_t st_notify = { ev_keyup, AM_MSGENTERED, 0, 0 };

    automapactive = true;
//  fb = I_VideoBuffer; // [crispy] simplify

    amclock = 0;
    lightlev = 0;

    m_paninc.x = m_paninc.y = m_paninc2.x = m_paninc2.y = 0;
    ftom_zoommul = FRACUNIT;
    mtof_zoommul = FRACUNIT;
    mousewheelzoom = false; // [crispy]

    m_w = FTOM(f_w);
    m_h = FTOM(f_h);

    // [crispy] find player to center
    plr = &players[displayplayer];

    next_m_x = (plr->mo->x >> FRACTOMAPBITS) - m_w/2;
    next_m_y = (plr->mo->y >> FRACTOMAPBITS) - m_h/2;

    AM_Ticker(); // [crispy] initialize variables for interpolation
    AM_changeWindowLoc();

    // for saving & restoring
    old_m_x = m_x;
    old_m_y = m_y;
    old_m_w = m_w;
    old_m_h = m_h;

    // inform the status bar of the change
    ST_Responder(&st_notify);

}

//
// 
//
void AM_loadPics(void)
{
    int i;
    char namebuf[9];
  
    for (i=0;i<10;i++)
    {
	DEH_snprintf(namebuf, 9, "AMMNUM%d", i);
	marknums[i] = W_CacheLumpName(namebuf, PU_STATIC);
    }

}

void AM_unloadPics(void)
{
    int i;
    char namebuf[9];
  
    for (i=0;i<10;i++)
    {
	DEH_snprintf(namebuf, 9, "AMMNUM%d", i);
	W_ReleaseLumpName(namebuf);
    }
}

void AM_clearMarks(void)
{
    int i;

    for (i=0;i<AM_NUMMARKPOINTS;i++)
	markpoints[i].x = -1; // means empty
    markpointnum = 0;
}

//
// should be called at the start of every level
// right now, i figure it out myself
//
void AM_LevelInit(boolean reinit)
{
    fixed_t a, b;
    static int f_h_old;
    // [crispy] Only need to precalculate color lookup tables once
    static int precalc_once;

    leveljuststarted = 0;

    f_x = f_y = 0;
    f_w = SCREENWIDTH;
    f_h = SCREENHEIGHT - (ST_HEIGHT << crispy->hires);

    AM_drawFline = crispy->smoothmap ? AM_drawFline_Smooth : AM_drawFline_Vanilla;

    if (!reinit)
    AM_clearMarks();

    AM_findMinMaxBoundaries();
    // [crispy] preserve map scale when re-initializing
    if (reinit && f_h_old)
    {
	scale_mtof = scale_mtof * f_h / f_h_old;
	AM_drawCrosshair(XHAIRCOLORS, true);
    }
    else
    {
    // [crispy] initialize zoomlevel on all maps so that a 4096 units
    // square map would just fit in (MAP01 is 3376x3648 units)
    a = FixedDiv(f_w, (max_w>>MAPBITS < 2048) ? 2*(max_w>>MAPBITS) : 4096);
    b = FixedDiv(f_h, (max_h>>MAPBITS < 2048) ? 2*(max_h>>MAPBITS) : 4096);
    scale_mtof = FixedDiv(a < b ? a : b, (int) (0.7*MAPUNIT));
    }
    if (scale_mtof > max_scale_mtof)
	scale_mtof = min_scale_mtof;
    scale_ftom = FixedDiv(FRACUNIT, scale_mtof);

    f_h_old = f_h;

    // [crispy] Precalculate color lookup tables for antialised line drawing using COLORMAP
    if (!precalc_once)
    {
        precalc_once = 1;
        for (int color = 0; color < 256; ++color)
        {
#define REINDEX(I) (color + I * 256)
            // Pick a range of shades for a steep gradient to keep lines thin
            int shade_index[NUMSHADES] =
            {
                REINDEX(0), REINDEX(1), REINDEX(2), REINDEX(3), REINDEX(7), REINDEX(15), REINDEX(23), REINDEX(31),
            };
#undef REINDEX
            for (int shade = 0; shade < NUMSHADES; ++shade)
            {
                color_shades[color * NUMSHADES + shade] = colormaps[shade_index[shade]];
            }
        }
    }
}




//
//
//
void AM_Stop (void)
{
    static event_t st_notify = { 0, ev_keyup, AM_MSGEXITED, 0 };

    AM_unloadPics();
    automapactive = false;
    ST_Responder(&st_notify);
    stopped = true;
}

//
//
//
// [crispy] moved here for extended savegames
static int lastlevel = -1, lastepisode = -1;
void AM_Start (void)
{
    if (!stopped) AM_Stop();
    stopped = false;
    if (lastlevel != gamemap || lastepisode != gameepisode)
    {
	AM_LevelInit(false);
	lastlevel = gamemap;
	lastepisode = gameepisode;
    }
    AM_initVariables();
    AM_loadPics();
}

// [crispy] reset IDDT cheat when re-starting map during demo recording
void AM_ResetIDDTcheat (void)
{
	cheating = 0;
}

//
// set the window scale to the maximum size
//
void AM_minOutWindowScale(void)
{
    scale_mtof = min_scale_mtof;
    scale_ftom = FixedDiv(FRACUNIT, scale_mtof);
    AM_activateNewScale();
}

//
// set the window scale to the minimum size
//
void AM_maxOutWindowScale(void)
{
    scale_mtof = max_scale_mtof;
    scale_ftom = FixedDiv(FRACUNIT, scale_mtof);
    AM_activateNewScale();
}


//
// Handle events (user inputs) in automap mode
//
boolean
AM_Responder
( event_t*	ev )
{

    int rc;
    static int bigstate=0;
    static char buffer[20];
    int key;
    extern boolean speedkeydown (void);

    // [crispy] toggleable pan/zoom speed
    if (speedkeydown())
    {
        f_paninc = F2_PANINC;
        m_zoomin_kbd = M2_ZOOMIN;
        m_zoomout_kbd = M2_ZOOMOUT;
        m_zoomin_mouse = M2_ZOOMINFAST;
        m_zoomout_mouse = M2_ZOOMOUTFAST;
    }
    else
    {
        f_paninc = F_PANINC;
        m_zoomin_kbd = M_ZOOMIN;
        m_zoomout_kbd = M_ZOOMOUT;
        m_zoomin_mouse = M2_ZOOMIN;
        m_zoomout_mouse = M2_ZOOMOUT;
    }

    rc = false;

    if (ev->type == ev_joystick && joybautomap >= 0
        && (ev->data1 & (1 << joybautomap)) != 0)
    {
        joywait = I_GetTime() + 5;

        if (!automapactive)
        {
            AM_Start ();
            viewactive = false;
        }
        else
        {
            bigstate = 0;
            viewactive = true;
            AM_Stop ();
        }

        return true;
    }

    if (!automapactive)
    {
	if (ev->type == ev_keydown && ev->data1 == key_map_toggle)
	{
	    AM_Start ();
	    viewactive = false;
	    rc = true;
	}
    }
    // [crispy] zoom and move Automap with the mouse (wheel)
    else if (ev->type == ev_mouse && !crispy->automapoverlay && !menuactive && !inhelpscreens)
    {
	if (mousebmapzoomout >= 0 && ev->data1 & (1 << mousebmapzoomout))
	{
		mtof_zoommul = m_zoomout_mouse;
		ftom_zoommul = m_zoomin_mouse;
		mousewheelzoom = true;
		rc = true;
	}
	else
	if (mousebmapzoomin >= 0 && ev->data1 & (1 << mousebmapzoomin))
	{
		mtof_zoommul = m_zoomin_mouse;
		ftom_zoommul = m_zoomout_mouse;
		mousewheelzoom = true;
		rc = true;
	}
	else
	if (mousebmapmaxzoom >= 0 && ev->data1 & (1 << mousebmapmaxzoom))
	{
		bigstate = !bigstate;
		if (bigstate)
		{
			AM_saveScaleAndLoc();
			AM_minOutWindowScale();
		}
		else AM_restoreScaleAndLoc();
	}
	else
	if (mousebmapfollow >= 0 && ev->data1 & (1 << mousebmapfollow))
	{
		followplayer = !followplayer;
		if (followplayer)
			plr->message = DEH_String(AMSTR_FOLLOWON);
		else
			plr->message = DEH_String(AMSTR_FOLLOWOFF);
	}
	else
	if (!followplayer && (ev->data2 || ev->data3))
	{
		// [crispy] mouse sensitivity for strafe
		m_paninc2.x = FTOM(ev->data2*(mouseSensitivity_x2+5)/(160 >> crispy->hires));
		m_paninc2.y = FTOM(ev->data3*(mouseSensitivity_x2+5)/(160 >> crispy->hires));
		rc = true;
	}
    }
    else if (ev->type == ev_keydown)
    {
	rc = true;
        key = ev->data1;

        if (key == key_map_east)          // pan right
        {
            // [crispy] keep the map static in overlay mode
            // if not following the player
            if (!followplayer)
                m_paninc.x = crispy->fliplevels ?
                    -FTOM(f_paninc << crispy->hires) : FTOM(f_paninc << crispy->hires);
            else rc = false;
        }
        else if (key == key_map_west)     // pan left
        {
            if (!followplayer)
                m_paninc.x = crispy->fliplevels ?
                    FTOM(f_paninc << crispy->hires) : -FTOM(f_paninc << crispy->hires);
            else rc = false;
        }
        else if (key == key_map_north)    // pan up
        {
            if (!followplayer)
                m_paninc.y = FTOM(f_paninc << crispy->hires);
            else rc = false;
        }
        else if (key == key_map_south)    // pan down
        {
            if (!followplayer)
                m_paninc.y = -FTOM(f_paninc << crispy->hires);
            else rc = false;
        }
        else if (key == key_map_zoomout)  // zoom out
        {
            mtof_zoommul = m_zoomout_kbd;
            ftom_zoommul = m_zoomin_kbd;
        }
        else if (key == key_map_zoomin)   // zoom in
        {
            mtof_zoommul = m_zoomin_kbd;
            ftom_zoommul = m_zoomout_kbd;
        }
        else if (key == key_map_toggle)
        {
            bigstate = 0;
            viewactive = true;
            AM_Stop ();
        }
        else if (key == key_map_maxzoom)
        {
            bigstate = !bigstate;
            if (bigstate)
            {
                AM_saveScaleAndLoc();
                AM_minOutWindowScale();
            }
            else AM_restoreScaleAndLoc();
        }
        else if (key == key_map_follow)
        {
            followplayer = !followplayer;
            if (followplayer)
                plr->message = DEH_String(AMSTR_FOLLOWON);
            else
                plr->message = DEH_String(AMSTR_FOLLOWOFF);
        }
        else if (key == key_map_grid)
        {
            grid = !grid;
            if (grid)
                plr->message = DEH_String(AMSTR_GRIDON);
            else
                plr->message = DEH_String(AMSTR_GRIDOFF);
        }
        else if (key == key_map_mark)
        {
            M_snprintf(buffer, sizeof(buffer), "%s %d",
                       DEH_String(AMSTR_MARKEDSPOT), markpointnum);
            plr->message = buffer;
            AM_addMark();
        }
        else if (key == key_map_clearmark)
        {
            AM_clearMarks();
            plr->message = DEH_String(AMSTR_MARKSCLEARED);
        }
        else if (key == key_map_overlay)
        {
            // [crispy] force redraw status bar
            inhelpscreens = true;

            crispy->automapoverlay = !crispy->automapoverlay;
            if (crispy->automapoverlay)
                plr->message = DEH_String(AMSTR_OVERLAYON);
            else
                plr->message = DEH_String(AMSTR_OVERLAYOFF);
        }
        else if (key == key_map_rotate)
        {
            crispy->automaprotate = !crispy->automaprotate;
            if (crispy->automaprotate)
                plr->message = DEH_String(AMSTR_ROTATEON);
            else
                plr->message = DEH_String(AMSTR_ROTATEOFF);
        }
        else
        {
            rc = false;
        }

        if ((!deathmatch || gameversion <= exe_doom_1_8)
         && cht_CheckCheat(&cheat_amap, ev->data2))
        {
            rc = false;
            cheating = (cheating + 1) % 3;
        }
    }
    else if (ev->type == ev_keyup)
    {
        rc = false;
        key = ev->data1;

        if (key == key_map_east)
        {
            if (!followplayer) m_paninc.x = 0;
        }
        else if (key == key_map_west)
        {
            if (!followplayer) m_paninc.x = 0;
        }
        else if (key == key_map_north)
        {
            if (!followplayer) m_paninc.y = 0;
        }
        else if (key == key_map_south)
        {
            if (!followplayer) m_paninc.y = 0;
        }
        else if (key == key_map_zoomout || key == key_map_zoomin)
        {
            mtof_zoommul = FRACUNIT;
            ftom_zoommul = FRACUNIT;
        }
    }

    return rc;

}


//
// Zooming
//
void AM_changeWindowScale(void)
{

    // Change the scaling multipliers
    scale_mtof = FixedMul(scale_mtof, mtof_zoommul);
    scale_ftom = FixedDiv(FRACUNIT, scale_mtof);

    // [crispy] reset after zooming with the mouse wheel
    if (mousewheelzoom)
    {
	mtof_zoommul = FRACUNIT;
	ftom_zoommul = FRACUNIT;
	mousewheelzoom = false;
    }

    if (scale_mtof < min_scale_mtof)
	AM_minOutWindowScale();
    else if (scale_mtof > max_scale_mtof)
	AM_maxOutWindowScale();
    else
	AM_activateNewScale();
}


//
//
//
void AM_doFollowPlayer(void)
{
    // [crispy] FTOM(MTOF()) is needed to fix map line jitter in follow mode.
    if (crispy->hires)
    {
        m_x = (viewx >> FRACTOMAPBITS) - m_w/2;
        m_y = (viewy >> FRACTOMAPBITS) - m_h/2;
    }
    else
    {
        m_x = FTOM(MTOF(viewx >> FRACTOMAPBITS)) - m_w/2;
        m_y = FTOM(MTOF(viewy >> FRACTOMAPBITS)) - m_h/2;
    }
        next_m_x = m_x;
        next_m_y = m_y;
	m_x2 = m_x + m_w;
	m_y2 = m_y + m_h;

	//  m_x = FTOM(MTOF(plr->mo->x - m_w/2));
	//  m_y = FTOM(MTOF(plr->mo->y - m_h/2));
	//  m_x = plr->mo->x - m_w/2;
	//  m_y = plr->mo->y - m_h/2;


}

//
//
//
void AM_updateLightLev(void)
{
    static int nexttic = 0;
    //static int litelevels[] = { 0, 3, 5, 6, 6, 7, 7, 7 };
    static int litelevels[] = { 0, 4, 7, 10, 12, 14, 15, 15 };
    static int litelevelscnt = 0;
   
    // Change light level
    if (amclock>nexttic)
    {
	lightlev = litelevels[litelevelscnt++];
	if (litelevelscnt == arrlen(litelevels)) litelevelscnt = 0;
	nexttic = amclock + 6 - (amclock % 6);
    }

}


//
// Updates on Game Tick
//
void AM_Ticker (void)
{

    if (!automapactive)
	return;

    amclock++;

    // [crispy] sync up for interpolation
    m_x = prev_m_x = next_m_x;
    m_y = prev_m_y = next_m_y;

    m_paninc_target.x = m_paninc.x + m_paninc2.x;
    m_paninc_target.y = m_paninc.y + m_paninc2.y;

    // [crispy] reset after moving with the mouse
    m_paninc2.x = m_paninc2.y = 0;

    if (followplayer)
	AM_doFollowPlayer();

    // Change the zoom if necessary
    if (ftom_zoommul != FRACUNIT)
	AM_changeWindowScale();

    if (m_paninc_target.x || m_paninc_target.y)
        AM_changeWindowLocTick();

    // Update light level
    // AM_updateLightLev();

}


//
// Clear automap frame buffer.
//
void AM_clearFB(int color)
{
    memset(fb, color, f_w*f_h*sizeof(*fb));
}


//
// Automap clipping of lines.
//
// Based on Cohen-Sutherland clipping algorithm but with a slightly
// faster reject and precalculated slopes.  If the speed is needed,
// use a hash algorithm to handle  the common cases.
//
boolean
AM_clipMline
( mline_t*	ml,
  fline_t*	fl )
{
    enum
    {
	LEFT	=1,
	RIGHT	=2,
	BOTTOM	=4,
	TOP	=8
    };
    
    register int	outcode1 = 0;
    register int	outcode2 = 0;
    register int	outside;
    
    fpoint_t	tmp;
    int		dx;
    int		dy;

    
#define DOOUTCODE(oc, mx, my) \
    (oc) = 0; \
    if ((my) < 0) (oc) |= TOP; \
    else if ((my) >= f_h) (oc) |= BOTTOM; \
    if ((mx) < 0) (oc) |= LEFT; \
    else if ((mx) >= f_w) (oc) |= RIGHT;

    
    // do trivial rejects and outcodes
    if (ml->a.y > m_y2)
	outcode1 = TOP;
    else if (ml->a.y < m_y)
	outcode1 = BOTTOM;

    if (ml->b.y > m_y2)
	outcode2 = TOP;
    else if (ml->b.y < m_y)
	outcode2 = BOTTOM;
    
    if (outcode1 & outcode2)
	return false; // trivially outside

    if (ml->a.x < m_x)
	outcode1 |= LEFT;
    else if (ml->a.x > m_x2)
	outcode1 |= RIGHT;
    
    if (ml->b.x < m_x)
	outcode2 |= LEFT;
    else if (ml->b.x > m_x2)
	outcode2 |= RIGHT;
    
    if (outcode1 & outcode2)
	return false; // trivially outside

    // transform to frame-buffer coordinates.
    fl->a.x = CXMTOF(ml->a.x);
    fl->a.y = CYMTOF(ml->a.y);
    fl->b.x = CXMTOF(ml->b.x);
    fl->b.y = CYMTOF(ml->b.y);

    DOOUTCODE(outcode1, fl->a.x, fl->a.y);
    DOOUTCODE(outcode2, fl->b.x, fl->b.y);

    if (outcode1 & outcode2)
	return false;

    while (outcode1 | outcode2)
    {
	// may be partially inside box
	// find an outside point
	if (outcode1)
	    outside = outcode1;
	else
	    outside = outcode2;
	
	// clip to each side
	if (outside & TOP)
	{
	    dy = fl->a.y - fl->b.y;
	    dx = fl->b.x - fl->a.x;
	    // [crispy] 'int64_t' math to avoid overflows on long lines.
	    tmp.x = fl->a.x + (fixed_t)(((int64_t)dx*(fl->a.y-f_y))/dy);
	    tmp.y = 0;
	}
	else if (outside & BOTTOM)
	{
	    dy = fl->a.y - fl->b.y;
	    dx = fl->b.x - fl->a.x;
	    tmp.x = fl->a.x + (fixed_t)(((int64_t)dx*(fl->a.y-(f_y+f_h)))/dy);
	    tmp.y = f_h-1;
	}
	else if (outside & RIGHT)
	{
	    dy = fl->b.y - fl->a.y;
	    dx = fl->b.x - fl->a.x;
	    tmp.y = fl->a.y + (fixed_t)(((int64_t)dy*(f_x+f_w-1 - fl->a.x))/dx);
	    tmp.x = f_w-1;
	}
	else if (outside & LEFT)
	{
	    dy = fl->b.y - fl->a.y;
	    dx = fl->b.x - fl->a.x;
	    tmp.y = fl->a.y + (fixed_t)(((int64_t)dy*(f_x-fl->a.x))/dx);
	    tmp.x = 0;
	}
        else
        {
            tmp.x = 0;
            tmp.y = 0;
        }

	if (outside == outcode1)
	{
	    fl->a = tmp;
	    DOOUTCODE(outcode1, fl->a.x, fl->a.y);
	}
	else
	{
	    fl->b = tmp;
	    DOOUTCODE(outcode2, fl->b.x, fl->b.y);
	}
	
	if (outcode1 & outcode2)
	    return false; // trivially outside
    }

    return true;
}
#undef DOOUTCODE


//
// Classic Bresenham w/ whatever optimizations needed for speed
//
static void
AM_drawFline_Vanilla
( fline_t*	fl,
  int		color )
{
    register int x;
    register int y;
    register int dx;
    register int dy;
    register int sx;
    register int sy;
    register int ax;
    register int ay;
    register int d;
    
    static int fuck = 0;

    // For debugging only
    if (      fl->a.x < 0 || fl->a.x >= f_w
	   || fl->a.y < 0 || fl->a.y >= f_h
	   || fl->b.x < 0 || fl->b.x >= f_w
	   || fl->b.y < 0 || fl->b.y >= f_h)
    {
        DEH_fprintf(stderr, "fuck %d \r", fuck++);
	return;
    }

#define PUTDOT_RAW(xx,yy,cc) fb[(yy)*f_w+(flipscreenwidth[xx])]=(cc)
#ifndef CRISPY_TRUECOLOR
#define PUTDOT(xx,yy,cc) PUTDOT_RAW(xx,yy,cc)
#else
#define PUTDOT(xx,yy,cc) PUTDOT_RAW(xx,yy,(colormaps[(cc)]))
#endif

    dx = fl->b.x - fl->a.x;
    ax = 2 * (dx<0 ? -dx : dx);
    sx = dx<0 ? -1 : 1;

    dy = fl->b.y - fl->a.y;
    ay = 2 * (dy<0 ? -dy : dy);
    sy = dy<0 ? -1 : 1;

    x = fl->a.x;
    y = fl->a.y;

    if (ax > ay)
    {
	d = ay - ax/2;
	while (1)
	{
	    PUTDOT(x,y,color);
	    if (x == fl->b.x) return;
	    if (d>=0)
	    {
		y += sy;
		d -= ax;
	    }
	    x += sx;
	    d += ay;
	}
    }
    else
    {
	d = ax - ay/2;
	while (1)
	{
	    PUTDOT(x, y, color);
	    if (y == fl->b.y) return;
	    if (d >= 0)
	    {
		x += sx;
		d -= ay;
	    }
	    y += sy;
	    d += ax;
	}
    }
}

// [crispy] Adapted from Heretic's DrawWuLine
static void AM_drawFline_Smooth(fline_t* fl, int color)
{
    int X0 = fl->a.x, Y0 = fl->a.y, X1 = fl->b.x, Y1 = fl->b.y;
    pixel_t* BaseColor = &color_shades[color * NUMSHADES];

    unsigned short IntensityShift, ErrorAdj, ErrorAcc;
    unsigned short ErrorAccTemp, Weighting, WeightingComplementMask;
    short DeltaX, DeltaY, Temp, XDir;

    /* Make sure the line runs top to bottom */
    if (Y0 > Y1)
    {
        Temp = Y0;
        Y0 = Y1;
        Y1 = Temp;
        Temp = X0;
        X0 = X1;
        X1 = Temp;
    }

    /* Draw the initial pixel, which is always exactly intersected by
       the line and so needs no weighting */
    /* Always write the raw color value because we've already performed the necessary lookup
     * into colormap */
    PUTDOT_RAW(X0, Y0, BaseColor[0]);

    if ((DeltaX = X1 - X0) >= 0)
    {
        XDir = 1;
    }
    else
    {
        XDir = -1;
        DeltaX = -DeltaX;       /* make DeltaX positive */
    }
    /* Special-case horizontal, vertical, and diagonal lines, which
       require no weighting because they go right through the center of
       every pixel */
    if ((DeltaY = Y1 - Y0) == 0)
    {
        /* Horizontal line */
        while (DeltaX-- != 0)
        {
            X0 += XDir;
            PUTDOT_RAW(X0, Y0, BaseColor[0]);
        }
        return;
    }
    if (DeltaX == 0)
    {
        /* Vertical line */
        do
        {
            Y0++;
            PUTDOT_RAW(X0, Y0, BaseColor[0]);
        }
        while (--DeltaY != 0);
        return;
    }
    //diagonal line.
    if (DeltaX == DeltaY)
    {
        do
        {
            X0 += XDir;
            Y0++;
            PUTDOT_RAW(X0, Y0, BaseColor[0]);
        }
        while (--DeltaY != 0);
        return;
    }
    /* Line is not horizontal, diagonal, or vertical */
    ErrorAcc = 0;               /* initialize the line error accumulator to 0 */
    /* # of bits by which to shift ErrorAcc to get intensity level */
    IntensityShift = 16 - NUMSHADES_BITS;
    /* Mask used to flip all bits in an intensity weighting, producing the
       result (1 - intensity weighting) */
    WeightingComplementMask = NUMSHADES - 1;
    /* Is this an X-major or Y-major line? */
    if (DeltaY > DeltaX)
    {
        /* Y-major line; calculate 16-bit fixed-point fractional part of a
           pixel that X advances each time Y advances 1 pixel, truncating the
           result so that we won't overrun the endpoint along the X axis */
        ErrorAdj = ((unsigned int) DeltaX << 16) / (unsigned int) DeltaY;
        /* Draw all pixels other than the first and last */
        while (--DeltaY)
        {
            ErrorAccTemp = ErrorAcc;    /* remember currrent accumulated error */
            ErrorAcc += ErrorAdj;       /* calculate error for next pixel */
            if (ErrorAcc <= ErrorAccTemp)
            {
                /* The error accumulator turned over, so advance the X coord */
                X0 += XDir;
            }
            Y0++;               /* Y-major, so always advance Y */
            /* The IntensityBits most significant bits of ErrorAcc give us the
               intensity weighting for this pixel, and the complement of the
               weighting for the paired pixel */
            Weighting = ErrorAcc >> IntensityShift;
            PUTDOT_RAW(X0, Y0, BaseColor[Weighting]);
            PUTDOT_RAW(X0 + XDir, Y0, BaseColor[(Weighting ^ WeightingComplementMask)]);
        }
        /* Draw the final pixel, which is always exactly intersected by the line
           and so needs no weighting */
        PUTDOT_RAW(X1, Y1, BaseColor[0]);
        return;
    }
    /* It's an X-major line; calculate 16-bit fixed-point fractional part of a
       pixel that Y advances each time X advances 1 pixel, truncating the
       result to avoid overrunning the endpoint along the X axis */
    ErrorAdj = ((unsigned int) DeltaY << 16) / (unsigned int) DeltaX;
    /* Draw all pixels other than the first and last */
    while (--DeltaX)
    {
        ErrorAccTemp = ErrorAcc;        /* remember currrent accumulated error */
        ErrorAcc += ErrorAdj;   /* calculate error for next pixel */
        if (ErrorAcc <= ErrorAccTemp)
        {
            /* The error accumulator turned over, so advance the Y coord */
            Y0++;
        }
        X0 += XDir;             /* X-major, so always advance X */
        /* The IntensityBits most significant bits of ErrorAcc give us the
           intensity weighting for this pixel, and the complement of the
           weighting for the paired pixel */
        Weighting = ErrorAcc >> IntensityShift;
        PUTDOT_RAW(X0, Y0, BaseColor[Weighting]);
        PUTDOT_RAW(X0, Y0 + 1, BaseColor[(Weighting ^ WeightingComplementMask)]);

    }
    /* Draw the final pixel, which is always exactly intersected by the line
       and so needs no weighting */
    PUTDOT_RAW(X1, Y1, BaseColor[0]);
}

//
// Clip lines, draw visible part sof lines.
//
void
AM_drawMline
( mline_t*	ml,
  int		color )
{
    static fline_t fl;

    if (AM_clipMline(ml, &fl))
	AM_drawFline(&fl, color); // draws it on frame buffer using fb coords
}



//
// Draws flat (floor/ceiling tile) aligned grid lines.
//
void AM_drawGrid(int color)
{
    int64_t x, y;
    int64_t start, end;
    const fixed_t gridsize = MAPBLOCKUNITS << MAPBITS;
    mline_t ml;

    // Figure out start of vertical gridlines
    start = m_x;
    if (crispy->automaprotate)
    {
	start -= m_h / 2;
    }
    // [crispy] fix losing grid lines near the automap boundary
    if ((start-(bmaporgx>>FRACTOMAPBITS))%gridsize)
	start += // (MAPBLOCKUNITS<<FRACBITS)
	    - ((start-(bmaporgx>>FRACTOMAPBITS))%gridsize);
    end = m_x + m_w;
    if (crispy->automaprotate)
    {
	end += m_h / 2;
    }

    // draw vertical gridlines
    for (x=start; x<end; x+=gridsize)
    {
	ml.a.x = x;
	ml.b.x = x;
	// [crispy] moved here
	ml.a.y = m_y;
	ml.b.y = m_y+m_h;
	if (crispy->automaprotate)
	{
	    ml.a.y -= m_w / 2;
	    ml.b.y += m_w / 2;
	    AM_rotatePoint(&ml.a);
	    AM_rotatePoint(&ml.b);
	}
	AM_drawMline(&ml, color);
    }

    // Figure out start of horizontal gridlines
    start = m_y;
    if (crispy->automaprotate)
    {
	start -= m_w / 2;
    }
    // [crispy] fix losing grid lines near the automap boundary
    if ((start-(bmaporgy>>FRACTOMAPBITS))%gridsize)
	start += // (MAPBLOCKUNITS<<FRACBITS)
	    - ((start-(bmaporgy>>FRACTOMAPBITS))%gridsize);
    end = m_y + m_h;
    if (crispy->automaprotate)
    {
	end += m_w / 2;
    }

    // draw horizontal gridlines
    for (y=start; y<end; y+=gridsize)
    {
	ml.a.y = y;
	ml.b.y = y;
	// [crispy] moved here
	ml.a.x = m_x;
	ml.b.x = m_x + m_w;
	if (crispy->automaprotate)
	{
	    ml.a.x -= m_h / 2;
	    ml.b.x += m_h / 2;
	    AM_rotatePoint(&ml.a);
	    AM_rotatePoint(&ml.b);
	}
	AM_drawMline(&ml, color);
    }

}

//
// Determines visible lines, draws them.
// This is LineDef based, not LineSeg based.
//

// [crispy] keyed linedefs (PR, P1, SR, S1)
static keycolor_t AM_DoorColor(int type)
{
  if (crispy->extautomap)
  {
    switch (type)
    {
	case 26:
	case 32:
	case 99:
	case 133:
	    return blue_key;
	case 27:
	case 34:
	case 136:
	case 137:
	    return yellow_key;
	case 28:
	case 33:
	case 134:
	case 135:
	    return red_key;
    }
  }
  return no_key;
}

void AM_drawWalls(void)
{
    int i;
    static mline_t l;

    for (i=0;i<numlines;i++)
    {
	l.a.x = lines[i].v1->x >> FRACTOMAPBITS;
	l.a.y = lines[i].v1->y >> FRACTOMAPBITS;
	l.b.x = lines[i].v2->x >> FRACTOMAPBITS;
	l.b.y = lines[i].v2->y >> FRACTOMAPBITS;
	if (crispy->automaprotate)
	{
	    AM_rotatePoint(&l.a);
	    AM_rotatePoint(&l.b);
	}
	if (cheating || (lines[i].flags & ML_MAPPED))
	{
	    if ((lines[i].flags & LINE_NEVERSEE) && !cheating)
		continue;
	    {
		// [crispy] draw keyed doors in their respective colors
		// (no Boom multiple keys)
		// make keyed doors flash for easier visibility
		keycolor_t amd;
		if (!(lines[i].flags & ML_SECRET) &&
		    (amd = AM_DoorColor(lines[i].special)) > no_key)
		{
		    switch (amd)
		    {
			case blue_key:
			    AM_drawMline(&l, ((leveltime & 16) ? BLUES : GRIDCOLORS));
			    continue;
			case yellow_key:
			    AM_drawMline(&l, ((leveltime & 16) ? (YELLOWS-2) : GRIDCOLORS));
			    continue;
			case red_key:
			    AM_drawMline(&l, ((leveltime & 16) ? (REDS-2) : GRIDCOLORS));
			    continue;
			default:
			    // [crispy] it should be impossible to reach here
			    break;
		    }
		}
	    }
	    // [crispy] draw exit lines in white (no Boom exit lines 197, 198)
	    // NB: Choco does not have this at all, Boom/PrBoom+ have this disabled by default
	    if (crispy->extautomap && (
	        lines[i].special == 11 ||
	        lines[i].special == 51 ||
	        lines[i].special == 52 ||
	        lines[i].special == 124))
	    {
		AM_drawMline(&l, WHITE);
		continue;
	    }
	    if (!lines[i].backsector)
	    {
		// [crispy] draw 1S secret sector boundaries in purple
		if (crispy->extautomap &&
		    cheating && (lines[i].frontsector->special == 9))
		    AM_drawMline(&l, SECRETWALLCOLORS);
#if defined CRISPY_HIGHLIGHT_REVEALED_SECRETS
		// [crispy] draw revealed secret sector boundaries in green
		else
		if (crispy->extautomap &&
		    crispy->secretmessage && (lines[i].frontsector->oldspecial == 9))
		    AM_drawMline(&l, REVEALEDSECRETWALLCOLORS);
#endif
		else
		AM_drawMline(&l, WALLCOLORS+lightlev);
	    }
	    else
	    {
		// [crispy] draw teleporters in green
		// and also WR teleporters 97 if they are not secret
		// (no monsters-only teleporters 125, 126; no Boom teleporters)
		if (lines[i].special == 39 ||
		    (crispy->extautomap && !(lines[i].flags & ML_SECRET) && lines[i].special == 97))
		{ // teleporters
		    AM_drawMline(&l, crispy->extautomap ? (GREENS+GREENRANGE/2) : (WALLCOLORS+WALLRANGE/2));
		}
		else if (lines[i].flags & ML_SECRET) // secret door
		{
		    // [crispy] NB: Choco has this check, but (SECRETWALLCOLORS == WALLCOLORS)
		    // Boom/PrBoom+ does not have this check at all
		    if (false && cheating) AM_drawMline(&l, SECRETWALLCOLORS + lightlev);
		    else AM_drawMline(&l, WALLCOLORS+lightlev);
		}
#if defined CRISPY_HIGHLIGHT_REVEALED_SECRETS
		// [crispy] draw revealed secret sector boundaries in green
		else if (crispy->extautomap && crispy->secretmessage &&
		    (lines[i].backsector->oldspecial == 9 ||
		    lines[i].frontsector->oldspecial == 9))
		{
		    AM_drawMline(&l, REVEALEDSECRETWALLCOLORS);
		}
#endif
		// [crispy] draw 2S secret sector boundaries in purple
		else if (crispy->extautomap && cheating &&
		    (lines[i].backsector->special == 9 ||
		    lines[i].frontsector->special == 9))
		{
		    AM_drawMline(&l, SECRETWALLCOLORS);
		}
		else if (lines[i].backsector->floorheight
			   != lines[i].frontsector->floorheight) {
		    AM_drawMline(&l, FDWALLCOLORS + lightlev); // floor level change
		}
		else if (lines[i].backsector->ceilingheight
			   != lines[i].frontsector->ceilingheight) {
		    AM_drawMline(&l, CDWALLCOLORS+lightlev); // ceiling level change
		}
		else if (cheating) {
		    AM_drawMline(&l, TSWALLCOLORS+lightlev);
		}
	    }
	}
	else if (plr->powers[pw_allmap])
	{
	    if (!(lines[i].flags & LINE_NEVERSEE)) AM_drawMline(&l, GRAYS+3);
	}
    }
}


//
// Rotation in 2D.
// Used to rotate player arrow line character.
//
void
AM_rotate
( int64_t*	x,
  int64_t*	y,
  angle_t	a )
{
    int64_t tmpx;

    tmpx =
	FixedMul(*x,finecosine[a>>ANGLETOFINESHIFT])
	- FixedMul(*y,finesine[a>>ANGLETOFINESHIFT]);
    
    *y   =
	FixedMul(*x,finesine[a>>ANGLETOFINESHIFT])
	+ FixedMul(*y,finecosine[a>>ANGLETOFINESHIFT]);

    *x = tmpx;
}

// [crispy] rotate point around map center
// adapted from prboom-plus/src/am_map.c:898-920
static void AM_rotatePoint (mpoint_t *pt)
{
    int64_t tmpx;
    // [crispy] smooth automap rotation
    const angle_t smoothangle = followplayer ? ANG90 - viewangle : mapangle;

    pt->x -= mapcenter.x;
    pt->y -= mapcenter.y;

    tmpx = (int64_t)FixedMul(pt->x, finecosine[smoothangle>>ANGLETOFINESHIFT])
         - (int64_t)FixedMul(pt->y, finesine[smoothangle>>ANGLETOFINESHIFT])
         + mapcenter.x;

    pt->y = (int64_t)FixedMul(pt->x, finesine[smoothangle>>ANGLETOFINESHIFT])
          + (int64_t)FixedMul(pt->y, finecosine[smoothangle>>ANGLETOFINESHIFT])
          + mapcenter.y;

    pt->x = tmpx;
}

void
AM_drawLineCharacter
( mline_t*	lineguy,
  int		lineguylines,
  fixed_t	scale,
  angle_t	angle,
  int		color,
  fixed_t	x,
  fixed_t	y )
{
    int		i;
    mline_t	l;

    if (crispy->automaprotate)
    {
	angle += mapangle;
    }

    for (i=0;i<lineguylines;i++)
    {
	l.a.x = lineguy[i].a.x;
	l.a.y = lineguy[i].a.y;

	if (scale)
	{
	    l.a.x = FixedMul(scale, l.a.x);
	    l.a.y = FixedMul(scale, l.a.y);
	}

	if (angle)
	    AM_rotate(&l.a.x, &l.a.y, angle);

	l.a.x += x;
	l.a.y += y;

	l.b.x = lineguy[i].b.x;
	l.b.y = lineguy[i].b.y;

	if (scale)
	{
	    l.b.x = FixedMul(scale, l.b.x);
	    l.b.y = FixedMul(scale, l.b.y);
	}

	if (angle)
	    AM_rotate(&l.b.x, &l.b.y, angle);
	
	l.b.x += x;
	l.b.y += y;

	AM_drawMline(&l, color);
    }
}

void AM_drawPlayers(void)
{
    int		i;
    player_t*	p;
    static int 	their_colors[] = { GREENS, GRAYS, BROWNS, REDS };
    int		their_color = -1;
    int		color;
    mpoint_t	pt;

    if (!netgame)
    {
	// [crispy] smooth player arrow rotation
	const angle_t smoothangle = crispy->automaprotate ? plr->mo->angle : viewangle;

	// [crispy] interpolate player arrow
	if (crispy->uncapped && leveltime > oldleveltime)
	{
	pt.x = viewx >> FRACTOMAPBITS;
	pt.y = viewy >> FRACTOMAPBITS;
	}
	else
	{
	pt.x = plr->mo->x >> FRACTOMAPBITS;
	pt.y = plr->mo->y >> FRACTOMAPBITS;
	}
	if (crispy->automaprotate)
	{
	    AM_rotatePoint(&pt);
	}

	if (cheating)
	    AM_drawLineCharacter
		(cheat_player_arrow, arrlen(cheat_player_arrow), 0,
		 smoothangle, WHITE, pt.x, pt.y);
	else
	    AM_drawLineCharacter
		(player_arrow, arrlen(player_arrow), 0, smoothangle,
		 WHITE, pt.x, pt.y);
	return;
    }

    for (i=0;i<MAXPLAYERS;i++)
    {
	// [crispy] interpolate other player arrows angle
	angle_t theirangle;

	their_color++;
	p = &players[i];

	if ( (deathmatch && !singledemo) && p != plr)
	    continue;

	if (!playeringame[i])
	    continue;

	if (p->powers[pw_invisibility])
	    color = 246; // *close* to black
	else
	    color = their_colors[their_color];
	
	// [crispy] interpolate other player arrows
	if (crispy->uncapped && leveltime > oldleveltime)
	{
	    pt.x = (p->mo->oldx + FixedMul(p->mo->x - p->mo->oldx, fractionaltic)) >> FRACTOMAPBITS;
	    pt.y = (p->mo->oldy + FixedMul(p->mo->y - p->mo->oldy, fractionaltic)) >> FRACTOMAPBITS;
	}
	else
	{
	    pt.x = p->mo->x >> FRACTOMAPBITS;
	    pt.y = p->mo->y >> FRACTOMAPBITS;
	}

	if (crispy->automaprotate)
	{
	    AM_rotatePoint(&pt);
	    theirangle = p->mo->angle;
	}
	else
	{
        theirangle = R_InterpolateAngle(p->mo->oldangle, p->mo->angle, fractionaltic);
	}

	AM_drawLineCharacter
	    (player_arrow, arrlen(player_arrow), 0, theirangle,
	     color, pt.x, pt.y);
    }

}

void
AM_drawThings
( int	colors,
  int 	colorrange)
{
    int		i;
    mobj_t*	t;
    keycolor_t	key;
    mpoint_t	pt;

    for (i=0;i<numsectors;i++)
    {
	t = sectors[i].thinglist;
	while (t)
	{
	    // [crispy] do not draw an extra triangle for the player
	    if (t == plr->mo)
	    {
		t = t->snext;
		continue;
	    }

	    // [crispy] interpolate thing triangles movement
	    if (leveltime > oldleveltime)
	    {
	    pt.x = (t->oldx + FixedMul(t->x - t->oldx, fractionaltic)) >> FRACTOMAPBITS;
	    pt.y = (t->oldy + FixedMul(t->y - t->oldy, fractionaltic)) >> FRACTOMAPBITS;
	    }
	    else
	    {
	    pt.x = t->x >> FRACTOMAPBITS;
	    pt.y = t->y >> FRACTOMAPBITS;
	    }
	    if (crispy->automaprotate)
	    {
		AM_rotatePoint(&pt);
	    }

	  if (crispy->extautomap)
	  {
	    // [crispy] skull keys and key cards
	    switch (t->info->doomednum)
	    {
		case 38:
		case 13:
		    key = red_key;
		    break;
		case 39:
		case 6:
		    key = yellow_key;
		    break;
		case 40:
		case 5:
		    key = blue_key;
		    break;
		default:
		    key = no_key;
		    break;
	    }

	    // [crispy] draw keys as crosses in their respective colors
	    if (key > no_key)
	    {
	    AM_drawLineCharacter
		(cross_mark, arrlen(cross_mark),
		 16<<MAPBITS, t->angle,
		 (key == red_key) ? REDS :
		 (key == yellow_key) ? YELLOWS :
		 (key == blue_key) ? BLUES :
		 colors+lightlev,
		 pt.x, pt.y);
	    }
	    else
	    // [crispy] draw blood splats and puffs as small squares
	    if (t->type == MT_BLOOD || t->type == MT_PUFF)
	    {
	    AM_drawLineCharacter
		(square_mark, arrlen(square_mark),
		 (t->radius >> 2) >> FRACTOMAPBITS, t->angle,
		 (t->type == MT_BLOOD) ? REDS : GRAYS,
		 pt.x, pt.y);
	    }
	    else
	    {
	    AM_drawLineCharacter
		(thintriangle_guy, arrlen(thintriangle_guy),
		// [crispy] triangle size represents actual thing size
		 t->radius >> FRACTOMAPBITS, t->angle,
		// [crispy] show countable kills in red ...
		 ((t->flags & (MF_COUNTKILL | MF_CORPSE)) == MF_COUNTKILL) ? REDS :
		// [crispy] ... show Lost Souls and missiles in orange ...
                (t->flags & (MF_FLOAT | MF_MISSILE)) ? 216 :
		// [crispy] ... show other shootable items in dark gold ...
		 (t->flags & MF_SHOOTABLE) ? 164 :
		// [crispy] ... corpses in gray ...
		 (t->flags & MF_CORPSE) ? GRAYS :
		// [crispy] ... and countable items in yellow
		 (t->flags & MF_COUNTITEM) ? YELLOWS :
		 colors+lightlev,
		 pt.x, pt.y);
	    }
	  }
	  else
	  {
	    AM_drawLineCharacter
		(thintriangle_guy, arrlen(thintriangle_guy),
		 16<<MAPBITS, t->angle, colors+lightlev, pt.x, pt.y);
	  }
	    t = t->snext;
	}
    }
}

void AM_drawMarks(void)
{
    int i, fx, fy, w, h;
    mpoint_t pt;

    for (i=0;i<AM_NUMMARKPOINTS;i++)
    {
	if (markpoints[i].x != -1)
	{
	    //      w = SHORT(marknums[i]->width);
	    //      h = SHORT(marknums[i]->height);
	    w = 5; // because something's wrong with the wad, i guess
	    h = 6; // because something's wrong with the wad, i guess
	    // [crispy] center marks around player
	    pt.x = markpoints[i].x;
	    pt.y = markpoints[i].y;
	    if (crispy->automaprotate)
	    {
		AM_rotatePoint(&pt);
	    }
	    fx = (flipscreenwidth[CXMTOF(pt.x)] >> crispy->hires) - 1 - WIDESCREENDELTA;
	    fy = (CYMTOF(pt.y) >> crispy->hires) - 2;
	    if (fx >= f_x && fx <= (f_w >> crispy->hires) - w && fy >= f_y && fy <= (f_h >> crispy->hires) - h)
		V_DrawPatch(fx, fy, marknums[i]);
	}
    }

}

static void AM_drawCrosshair(int color, boolean force)
{
    // [crispy] draw an actual crosshair
    if (!followplayer || force)
    {
	static fline_t h, v;

	if (!h.a.x || force)
	{
	    h.a.x = h.b.x = v.a.x = v.b.x = f_x + f_w / 2;
	    h.a.y = h.b.y = v.a.y = v.b.y = f_y + f_h / 2;
	    h.a.x -= 2; h.b.x += 2;
	    v.a.y -= 2; v.b.y += 2;
	}

	AM_drawFline(&h, color);
	AM_drawFline(&v, color);
    }
// [crispy] do not draw the useless dot on the player arrow
/*
    else
#ifndef CRISPY_TRUECOLOR
    fb[(f_w*(f_h+1))/2] = color; // single point for now
#else
    fb[(f_w*(f_h+1))/2] = colormaps[color]; // single point for now
#endif
*/

}

void AM_Drawer (void)
{
    if (!automapactive) return;

    // [crispy] move AM_doFollowPlayer and AM_changeWindowLoc
    // from AM_Ticker for interpolation

    if (followplayer)
    {
        AM_doFollowPlayer();
    }

    // Change x,y location
    if (m_paninc_target.x || m_paninc_target.y)
    {
        AM_changeWindowLoc();
    }

    // [crispy] required for AM_rotatePoint()
    if (crispy->automaprotate)
    {
	mapcenter.x = m_x + m_w / 2;
	mapcenter.y = m_y + m_h / 2;
	// [crispy] keep the map static in overlay mode
	// if not following the player
	if (!(!followplayer && crispy->automapoverlay))
	mapangle = ANG90 - plr->mo->angle;
    }

    if (!crispy->automapoverlay)
    {
        AM_clearFB(BACKGROUND);
        pspr_interp = false; // interpolate weapon bobbing
    }
    if (grid)
	AM_drawGrid(GRIDCOLORS);
    AM_drawWalls();
    AM_drawPlayers();
    if (cheating==2)
	AM_drawThings(THINGCOLORS, THINGRANGE);
    AM_drawCrosshair(XHAIRCOLORS, false);

    AM_drawMarks();

    V_MarkRect(f_x, f_y, f_w, f_h);

}

// [crispy] extended savegames
void AM_GetMarkPoints (int *n, long *p)
{
	int i;

	*n = markpointnum;
	*p = -1L;

	// [crispy] prevent saving markpoints from previous map
	if (lastlevel == gamemap && lastepisode == gameepisode)
	{
		for (i = 0; i < AM_NUMMARKPOINTS; i++)
		{
			*p++ = (long)markpoints[i].x;
			*p++ = (markpoints[i].x == -1) ? 0L : (long)markpoints[i].y;
		}
	}
}

void AM_SetMarkPoints (int n, long *p)
{
	int i;

	AM_LevelInit(false);
	lastlevel = gamemap;
	lastepisode = gameepisode;

	markpointnum = n;

	for (i = 0; i < AM_NUMMARKPOINTS; i++)
	{
		markpoints[i].x = (int64_t)*p++;
		markpoints[i].y = (int64_t)*p++;
	}
}
