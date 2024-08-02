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
#include "m_misc.h"
#include "st_stuff.h"
#include "p_local.h"
#include "w_wad.h"

#include "m_cheat.h"
#include "m_controls.h"
#include "i_system.h"
#include "i_timer.h"

// Needs access to LFB.
#include "v_video.h"

// State.
#include "doomstat.h"
#include "r_state.h"

// Data.
#include "dstrings.h"

#include "am_map.h"
extern boolean inhelpscreens; // [crispy]


// Automap colors
#define BACKGROUND      240         // haleyjd [STRIFE]
#define WALLCOLORS      5           // villsa [STRIFE]
#define WALLRANGE       16
#define TSWALLCOLORS    16
#define FDWALLCOLORS    122         // villsa [STRIFE]
#define CDWALLCOLORS    116
#define CTWALLCOLORS    19          // villsa [STRIFE]
#define SPWALLCOLORS    243         // villsa [STRIFE]
#define THINGCOLORS     233         // villsa [STRIFE]
#define MISSILECOLORS   227         // villsa [STRIFE]
#define SHOOTABLECOLORS 235         // villsa [STRIFE]

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
#undef R




static int 	cheating = 0;
//static int 	grid = 0;     [STRIFE]: no such variable

boolean    	automapactive = false;
//static int 	finit_width = SCREENWIDTH;
//static int 	finit_height = SCREENHEIGHT - (32 << crispy->hires);

// location of window on screen
static int 	f_x;
static int	f_y;

// size of window on screen
static int 	f_w;
static int	f_h;

static int 	lightlev; 		// used for funky strobing effect
#define fb I_VideoBuffer // [crispy] simplify
//static byte*	fb; 			// pseudo-frame buffer
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
static int mapmarknum = 0;  // villsa [STRIFE] unused but this was meant to be used for objective based markers
static int followplayer = 1; // specifies whether to follow the player around

cheatseq_t cheat_amap = CHEAT("topo", 0);   // villsa [STRIFE]

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
    markpoints[markpointnum].x = plr->mo->x >> FRACTOMAPBITS; // 20160306 [STRIFE]: use player position
    markpoints[markpointnum].y = plr->mo->y >> FRACTOMAPBITS;
    //markpointnum = (markpointnum + 1) % AM_NUMMARKPOINTS;
    ++markpointnum; // haleyjd 20141101: [STRIFE] does not wrap around
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
// AM_loadPics
//
// haleyjd 08/27/10: [STRIFE] Changed marknums to PLMNUM%d
//
void AM_loadPics(void)
{
    int i;
    char namebuf[9];
  
    for (i=0;i<10;i++)
    {
        DEH_snprintf(namebuf, 9, "PLMNUM%d", i);
        marknums[i] = W_CacheLumpName(namebuf, PU_STATIC);
    }

}

void AM_unloadPics(void)
{
    int i;
    char namebuf[9];
  
    for (i=0;i<10;i++)
    {
        DEH_snprintf(namebuf, 9, "PLMNUM%d", i);
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
void AM_Start (void)
{
    static int lastlevel = -1;
    //static int lastepisode = -1;

    if (!stopped) AM_Stop();
    stopped = false;
    if (lastlevel != gamemap  /*|| lastepisode != gameepisode*/)
    {
	AM_LevelInit(false);
	lastlevel = gamemap;
	//lastepisode = gameepisode;
    }
    AM_initVariables();
    AM_loadPics();
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
    static int joywait = 0;
    static char buffer[20];
    int key;

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
        && (ev->data1 & (1 << joybautomap)) != 0 && joywait < I_GetTime())
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
            else
            {
                AM_restoreScaleAndLoc();
            }
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
            if (!followplayer) m_paninc.x = FTOM(f_paninc << crispy->hires);
            else rc = false;
        }
        else if (key == key_map_west)     // pan left
        {
            if (!followplayer) m_paninc.x = -FTOM(f_paninc << crispy->hires);
            else rc = false;
        }
        else if (key == key_map_north)    // pan up
        {
            if (!followplayer) m_paninc.y = FTOM(f_paninc << crispy->hires);
            else rc = false;
        }
        else if (key == key_map_south)    // pan down
        {
            if (!followplayer) m_paninc.y = -FTOM(f_paninc << crispy->hires);
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
        // haleyjd 20141101: [STRIFE] grid is not supported
        /*
        else if (key == key_map_grid)
        {
            grid = !grid;
            if (grid)
                plr->message = DEH_String(AMSTR_GRIDON);
            else
                plr->message = DEH_String(AMSTR_GRIDOFF);
        }
        */
        else if (key == key_map_mark)
        {
            // haleyjd 20141101: [STRIFE] if full, mark 9 is replaced
            if(markpointnum == AM_NUMMARKPOINTS)
                --markpointnum;
            M_snprintf(buffer, sizeof(buffer),
                       "%s %d", DEH_String(AMSTR_MARKEDSPOT), markpointnum + 1); // [STRIFE]
            plr->message = buffer;
            AM_addMark();
        }
        else if (key == key_map_clearmark)
        {
            // haleyjd 20141101: [STRIFE] clears last mark only
            if(markpointnum > 0)
            {
                markpoints[markpointnum - 1].x = -1;
                --markpointnum;
                plr->message = DEH_String(AMSTR_MARKSCLEARED);
            }
        }
        else if (key == key_map_overlay)
        {
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

	if (!deathmatch && cht_CheckCheat(&cheat_amap, ev->data2))
	{
	    rc = false;
	    cheating = (cheating+1) % 3;
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

#define PUTDOT_RAW(xx,yy,cc) fb[(yy)*f_w+(xx)]=(cc)
#ifndef CRISPY_TRUECOLOR
#define PUTDOT(xx,yy,cc) PUTDOT_RAW(xx,yy,cc)
#else
#define PUTDOT(xx,yy,cc) PUTDOT_RAW(xx,yy,(pal_color[(cc)]))
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
/*void AM_drawGrid(int color)
{
    fixed_t x, y;
    fixed_t start, end;
    mline_t ml;

    // Figure out start of vertical gridlines
    start = m_x;
    if ((start-bmaporgx)%(MAPBLOCKUNITS<<FRACBITS))
	start += (MAPBLOCKUNITS<<FRACBITS)
	    - ((start-bmaporgx)%(MAPBLOCKUNITS<<FRACBITS));
    end = m_x + m_w;

    // draw vertical gridlines
    ml.a.y = m_y;
    ml.b.y = m_y+m_h;
    for (x=start; x<end; x+=(MAPBLOCKUNITS<<FRACBITS))
    {
	ml.a.x = x;
	ml.b.x = x;
	AM_drawMline(&ml, color);
    }

    // Figure out start of horizontal gridlines
    start = m_y;
    if ((start-bmaporgy)%(MAPBLOCKUNITS<<FRACBITS))
	start += (MAPBLOCKUNITS<<FRACBITS)
	    - ((start-bmaporgy)%(MAPBLOCKUNITS<<FRACBITS));
    end = m_y + m_h;

    // draw horizontal gridlines
    ml.a.x = m_x;
    ml.b.x = m_x + m_w;
    for (y=start; y<end; y+=(MAPBLOCKUNITS<<FRACBITS))
    {
	ml.a.y = y;
	ml.b.y = y;
	AM_drawMline(&ml, color);
    }

}*/

//
// Determines visible lines, draws them.
// This is LineDef based, not LineSeg based.
//
void AM_drawWalls(void)
{
    int i;
    line_t* line;
    static mline_t l;

    for(i = 0; i < numlines; i++)
    {
        line = &lines[i];

        l.a.x = line->v1->x >> FRACTOMAPBITS;
        l.a.y = line->v1->y >> FRACTOMAPBITS;
        l.b.x = line->v2->x >> FRACTOMAPBITS;
        l.b.y = line->v2->y >> FRACTOMAPBITS;

        if (crispy->automaprotate)
        {
            AM_rotatePoint(&l.a);
            AM_rotatePoint(&l.b);
        }

        if(cheating || (line->flags & ML_MAPPED))
        {
            if((line->flags & LINE_NEVERSEE) && !cheating)
                continue;

            // villsa [STRIFE]
            if(line->special == 145 || line->special == 186)
            {
                AM_drawMline(&l, SPWALLCOLORS);
            }
            // villsa [STRIFE] lightlev is unused here
            else if(!line->backsector)
            {
                AM_drawMline(&l, WALLCOLORS);
            }
            else
            {
                if(line->special == 39)
                { // teleporters
                    AM_drawMline(&l, WALLCOLORS+WALLRANGE/2);
                }
                else if (line->flags & ML_SECRET) // secret door
                {
                    // villsa [STRIFE] just draw the wall as is!
                    AM_drawMline(&l, WALLCOLORS);
                }
                else if(line->backsector->floorheight != line->frontsector->floorheight)
                {
                    AM_drawMline(&l, FDWALLCOLORS); // floor level change
                }
                else if(line->backsector->ceilingheight != line->frontsector->ceilingheight)
                {
                        AM_drawMline(&l, CDWALLCOLORS); // ceiling level change
                }
                else if (cheating)
                {
                    AM_drawMline(&l, TSWALLCOLORS);
                }
            }
        }
        // villsa [STRIFE] show all of the map on map 15
        else if(plr->powers[pw_allmap] || gamemap == 15)
        {
            if(!(line->flags & LINE_NEVERSEE))
                AM_drawMline(&l, CTWALLCOLORS);
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
    // villsa [STRIFE] updated array
    static int 	their_colors[] = { 0x80, 0x40, 0xB0, 0x10, 0x30, 0x50, 0xA0, 0x60, 0x90 };
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

        // villsa [STRIFE] don't draw cheating player arrow.
        // Player arrow is yellow instead of white
        AM_drawLineCharacter
		(player_arrow, arrlen(player_arrow), 0, smoothangle,
		 224, pt.x, pt.y);
	return;
    }

    for(i = 0; i < MAXPLAYERS; i++)
    {
	// [crispy] interpolate other player arrows angle
	angle_t theirangle;

	their_color++;
	p = &players[i];

        // villsa [STRIFE] check for gameskill??
	if((gameskill && deathmatch && !singledemo) && p != plr)
	    continue;

	if(!playeringame[i])
	    continue;

        // villsa [STRIFE] change to 27
	if(p->powers[pw_invisibility])
	    color = 27; // *close* to black
	else
	    color = their_colors[their_color];
	
	// [crispy] interpolate other player arrows
	if (crispy->uncapped && leveltime > oldleveltime)
	{
	    pt.x = LerpFixed(p->mo->oldx, p->mo->x) >> FRACTOMAPBITS;
	    pt.y = LerpFixed(p->mo->oldy, p->mo->y) >> FRACTOMAPBITS;
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
        theirangle = LerpAngle(p->mo->oldangle, p->mo->angle);
	}

	AM_drawLineCharacter
	    (player_arrow, arrlen(player_arrow), 0, theirangle,
	     color, pt.x, pt.y);
    }

}

//
// AM_drawThings
//
// villsa [STRIFE] no arguments
//
void AM_drawThings(void)
{
    int         i;
    mobj_t*     t;
    int         colors;
    fixed_t     radius;
    mpoint_t    pt;

    // villsa [STRIFE] almost re-written
    // specific things can have different radius, color and appearence
    for(i = 0; i < numsectors; i++)
    {
        t = sectors[i].thinglist;
        while(t)
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
                pt.x = LerpFixed(t->oldx, t->x) >> FRACTOMAPBITS;
                pt.y = LerpFixed(t->oldy, t->y) >> FRACTOMAPBITS;
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

            radius = t->radius >> FRACTOMAPBITS;

            if(t->flags & (MF_MISSILE|MF_SPECIAL))
            {
                colors = MISSILECOLORS;
            }
            else if(t->flags & MF_COUNTKILL)
            {
                colors = SHOOTABLECOLORS;
            }
            else
            {
                colors = THINGCOLORS;
                radius = (16<<MAPBITS);
            }

            AM_drawLineCharacter (thintriangle_guy, arrlen(thintriangle_guy),
                radius, t->angle, colors, pt.x, pt.y);

            t = t->snext;
        }
    }
}

//
// AM_drawMarks
//
void AM_drawMarks(void)
{
    int i, fx, fy, w, h;
    mpoint_t pt;

    for(i = 0; i < AM_NUMMARKPOINTS; i++)
    {
        if(markpoints[i].x != -1)
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
            fx = (CXMTOF(pt.x) >> crispy->hires) - 3;
            fy = (CYMTOF(pt.y) >> crispy->hires) - 3;
            if (fx >= f_x && fx <= (f_w >> crispy->hires) - w && fy >= f_y && fy <= (f_h >> crispy->hires) - h)
            {
                // villsa [STRIFE]
                if(i >= mapmarknum)
                    V_DrawPatch(fx - WIDESCREENDELTA, fy, marknums[i]);
            }
        }
    }

}

// villsa [STRIFE] unused
/*void AM_drawCrosshair(int color)
{
    fb[(f_w*(f_h+1))/2] = color; // single point for now
}*/

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
#ifndef CRISPY_TRUECOLOR
        AM_clearFB(BACKGROUND);
#else
        AM_clearFB(pal_color[BACKGROUND]);
#endif
        pspr_interp = false; // [crispy] interpolate weapon bobbing
    }

    // villsa [STRIFE] not used
    /*if(grid)
        AM_drawGrid(GRIDCOLORS);*/

    AM_drawWalls();
    AM_drawPlayers();

    // villsa [STRIFE] draw things when map powerup is enabled
    if(cheating == 2 || plr->powers[pw_allmap] > 1)
        AM_drawThings();

    // villsa [STRIFE] not used
    //AM_drawCrosshair(XHAIRCOLORS);

    AM_drawMarks();
    V_MarkRect(f_x, f_y, f_w, f_h);

}
