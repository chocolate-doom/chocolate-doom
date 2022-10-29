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

#include <stdio.h>

#include "h2def.h"
#include "doomkeys.h"
#include "i_video.h"
#include "i_swap.h"
#include "i_timer.h"
#include "m_controls.h"
#include "m_misc.h"
#include "p_local.h"
#include "am_map.h"
#include "am_data.h"
#include "v_video.h"

#define NUMALIAS 3              // Number of antialiased lines.

int cheating = 0;
static int grid = 0;

static int leveljuststarted = 1;        // kluge until AM_LevelInit() is called

boolean automapactive = false;
static int finit_width;// = SCREENWIDTH;
static int finit_height;// = SCREENHEIGHT - SBARHEIGHT - (3 << crispy->hires);
static int f_x, f_y;            // location of window on screen
static int f_w, f_h;            // size of window on screen
static int lightlev;            // used for funky strobing effect
static byte *fb;                // pseudo-frame buffer
static int amclock;

static mpoint_t m_paninc;       // how far the window pans each tic (map coords)
static mpoint_t m_paninc2;      // [crispy] mouse map panning
static mpoint_t m_paninc_target; // [crispy] for interpolation
static fixed_t mtof_zoommul;    // how far the window zooms in each tic (map coords)
static fixed_t ftom_zoommul;    // how far the window zooms in each tic (fb coords)

static fixed_t m_x, m_y;        // LL x,y where the window is on the map (map coords)
static fixed_t m_x2, m_y2;      // UR x,y where the window is on the map (map coords)
static fixed_t prev_m_x, prev_m_y; // [crispy] for interpolation
static fixed_t next_m_x, next_m_y; // [crispy] for interpolation

// width/height of window on map (map coords)
static fixed_t m_w, m_h;
static fixed_t min_x, min_y;    // based on level size
static fixed_t max_x, max_y;    // based on level size
static fixed_t max_w, max_h;    // max_x-min_x, max_y-min_y
static fixed_t min_w, min_h;    // based on player size
static fixed_t min_scale_mtof;  // used to tell when to stop zooming out
static fixed_t max_scale_mtof;  // used to tell when to stop zooming in

// old stuff for recovery later
static fixed_t old_m_w, old_m_h;
static fixed_t old_m_x, old_m_y;

// used by MTOF to scale from map-to-frame-buffer coords
static fixed_t scale_mtof = INITSCALEMTOF;
// used by FTOM to scale from frame-buffer-to-map coords (=1/scale_mtof)
static fixed_t scale_ftom;

static player_t *plr;           // the player represented by an arrow

// [crispy] toggleable pan/zoom speed
static int f_paninc;
static int m_zoomin_kbd;
static int m_zoomout_kbd;
static int m_zoomin_mouse;
static int m_zoomout_mouse;
static boolean mousewheelzoom;

//static patch_t *marknums[10]; // numbers used for marking by the automap
//static mpoint_t markpoints[AM_NUMMARKPOINTS]; // where the points are
//static int markpointnum = 0; // next point to be assigned

static int followplayer = 1;    // specifies whether to follow the player around

static char cheat_kills[] = { 'k', 'i', 'l', 'l', 's' };
static boolean ShowKills = 0;
static unsigned ShowKillsCount = 0;

// [crispy] gradient table for map normal mode
static byte antialias_normal[NUMALIAS][8] = {
    {83, 84, 85, 86, 87, 88, 89, 90},
    {96, 96, 95, 94, 93, 92, 91, 90},
    {107, 108, 109, 110, 111, 112, 89, 90}
};

// [crispy] gradient table for map overlay mode
static byte antialias_overlay[NUMALIAS][8] = {
    {86, 85, 84, 83, 82, 81, 100, 99},
    {96, 93, 90, 87, 85, 83, 81, 99},
    {107, 105, 104, 103, 102, 101, 100, 99}
};

static byte (*antialias)[NUMALIAS][8]; // [crispy]
/*
static byte *aliasmax[NUMALIAS] = {
	&antialias[0][7], &antialias[1][7], &antialias[2][7]
};*/

static byte *maplump;           // pointer to the raw data for the automap background.
static short mapystart = 0;     // y-value for the start of the map bitmap...used in
                                                                                //the parallax stuff.
static short mapxstart = 0;     //x-value for the bitmap.
static short prev_mapxstart, prev_mapystart; // [crispy] for interpolation
static short next_mapxstart, next_mapystart; // [crispy] for interpolation

// [crispy] Used for automap background tiling and scrolling
#define MAPBGROUNDWIDTH ORIGWIDTH
#define MAPBGROUNDHEIGHT (ORIGHEIGHT - ORIGSBARHEIGHT - 3)

// [crispy] automap rotate
void AM_rotate(fixed_t* x, fixed_t* y, angle_t a);
static void AM_rotatePoint(mpoint_t *pt);
static mpoint_t mapcenter;
static angle_t mapangle;

// [AM] Fractional part of the current tic, in the half-open
//      range of [0.0, 1.0).  Used for interpolation.
extern fixed_t          fractionaltic;

//byte screens[][SCREENWIDTH*SCREENHEIGHT];
//void V_MarkRect (int x, int y, int width, int height);

// Functions

void DrawWuLine(int X0, int Y0, int X1, int Y1, byte * BaseColor,
                int NumLevels, unsigned short IntensityBits);

void AM_DrawDeathmatchStats(void);
static void DrawWorldTimer(void);

// Calculates the slope and slope according to the x-axis of a line
// segment in map coordinates (with the upright y-axis n' all) so
// that it can be used with the brain-dead drawing stuff.

// Ripped out for Heretic
/*
void AM_getIslope(mline_t *ml, islope_t *is)
{
  int dx, dy;

  dy = ml->a.y - ml->b.y;
  dx = ml->b.x - ml->a.x;
  if (!dy) is->islp = (dx<0?-INT_MAX:INT_MAX);
  else is->islp = FixedDiv(dx, dy);
  if (!dx) is->slp = (dy<0?-INT_MAX:INT_MAX);
  else is->slp = FixedDiv(dy, dx);
}
*/

void AM_activateNewScale(void)
{
    m_x += m_w / 2;
    m_y += m_h / 2;
    m_w = FTOM(f_w);
    m_h = FTOM(f_h);
    m_x -= m_w / 2;
    m_y -= m_h / 2;
    m_x2 = m_x + m_w;
    m_y2 = m_y + m_h;
    next_m_x = m_x; // [crispy]
    next_m_y = m_y; // [crispy]
}

void AM_saveScaleAndLoc(void)
{
    old_m_x = m_x;
    old_m_y = m_y;
    old_m_w = m_w;
    old_m_h = m_h;
}

void AM_restoreScaleAndLoc(void)
{

    m_w = old_m_w;
    m_h = old_m_h;
    if (!followplayer)
    {
        m_x = old_m_x;
        m_y = old_m_y;
    }
    else
    {
        m_x = (plr->mo->x >> FRACTOMAPBITS) - m_w / 2;
        m_y = (plr->mo->y >> FRACTOMAPBITS) - m_h / 2;
    }
    m_x2 = m_x + m_w;
    m_y2 = m_y + m_h;
    next_m_x = m_x; // [crispy]
    next_m_y = m_y; // [crispy]

    // Change the scaling multipliers
    scale_mtof = FixedDiv(f_w << FRACBITS, m_w);
    scale_ftom = FixedDiv(FRACUNIT, scale_mtof);
}

// adds a marker at the current location

/*
void AM_addMark(void)
{
  markpoints[markpointnum].x = m_x + m_w/2;
  markpoints[markpointnum].y = m_y + m_h/2;
  markpointnum = (markpointnum + 1) % AM_NUMMARKPOINTS;

}
*/
void AM_findMinMaxBoundaries(void)
{
    int i;
    fixed_t a, b;

    min_x = min_y = INT_MAX;
    max_x = max_y = -INT_MAX;
    for (i = 0; i < numvertexes; i++)
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
    min_w = 2 * MAPPLAYERRADIUS;
    min_h = 2 * MAPPLAYERRADIUS;

    a = FixedDiv(f_w << FRACBITS, max_w);
    b = FixedDiv(f_h << FRACBITS, max_h);
    min_scale_mtof = a < b ? a : b;

    max_scale_mtof = FixedDiv(f_h << FRACBITS, 2 * MAPPLAYERRADIUS);

}

// [crispy] Function called by AM_Ticker for stable panning interpolation
void AM_changeWindowLocTick(void)
{
    fixed_t incx, incy;

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

    // [crispy] Disable map background scroll in non-follow + rotate mode.
    // The combination of the two effects is unappealing and slightly
    // nauseating.
    if (!crispy->automaprotate)
    {
        if (incx)
            next_mapxstart += MTOF(incx+MAPUNIT/2);
        if (incy)
            next_mapystart -= MTOF(incy+MAPUNIT/2);
    }

    // [crispy] Change background tile dimensions for hi-res
    if(next_mapxstart >= MAPBGROUNDWIDTH << crispy->hires)
        next_mapxstart -= MAPBGROUNDWIDTH << crispy->hires;
    if(next_mapxstart < 0)
        next_mapxstart += MAPBGROUNDWIDTH << crispy->hires;
    if(next_mapystart >= MAPBGROUNDHEIGHT >> crispy->hires)
        next_mapystart -= MAPBGROUNDHEIGHT >> crispy->hires;
    if(next_mapystart < 0)
        next_mapystart += MAPBGROUNDHEIGHT >> crispy->hires;
}

void AM_changeWindowLoc(void)
{
    fixed_t incx, incy;

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

    if (m_x + m_w / 2 > max_x)
    {
        next_m_x = m_x = max_x - m_w / 2;
        next_mapxstart = mapxstart;
        incx = 0;
    }
    else if (m_x + m_w / 2 < min_x)
    {
        next_m_x = m_x = min_x - m_w / 2;
        next_mapxstart = mapxstart;
        incx = 0;
    }
    if (m_y + m_h / 2 > max_y)
    {
        next_m_y = m_y = max_y - m_h / 2;
        next_mapystart = mapystart;
        incy = 0;
    }
    else if (m_y + m_h / 2 < min_y)
    {
        next_m_y = m_y = min_y - m_h / 2;
        next_mapystart = mapystart;
        incy = 0;
    }

    // [crispy] Disable map background scroll in non-follow + rotate mode.
    // The combination of the two effects is unappealing and slightly
    // nauseating.
    if (!crispy->automaprotate)
    {
        mapxstart = incx ? prev_mapxstart + MTOF(incx+MAPUNIT/2) : mapxstart;
        mapystart = incy ? prev_mapystart - MTOF(incy+MAPUNIT/2) : mapystart;
    }

    // The following code was commented out in the released Hexen source,
    // but I believe we need to do this here to stop the background moving
    // when we reach the map boundaries. (In the released source it's done
    // in AM_clearFB).

    // [crispy] Change background tile dimensions for hi-res
    if(mapxstart >= MAPBGROUNDWIDTH << crispy->hires)
        mapxstart -= MAPBGROUNDWIDTH << crispy->hires;
    if(mapxstart < 0)
        mapxstart += MAPBGROUNDWIDTH << crispy->hires;
    if(mapystart >= MAPBGROUNDHEIGHT >> crispy->hires)
        mapystart -= MAPBGROUNDHEIGHT >> crispy->hires;
    if(mapystart < 0)
        mapystart += MAPBGROUNDHEIGHT >> crispy->hires;
    // - end of code that was commented-out

    m_x2 = m_x + m_w;
    m_y2 = m_y + m_h;

}

void AM_initVariables(void)
{
    int pnum;
    thinker_t *think;

    //static event_t st_notify = { ev_keyup, AM_MSGENTERED };

    automapactive = true;
    fb = I_VideoBuffer;

    amclock = 0;
    lightlev = 0;

    m_paninc.x = m_paninc.y = m_paninc2.x = m_paninc2.y = 0;
    ftom_zoommul = FRACUNIT;
    mtof_zoommul = FRACUNIT;
    mousewheelzoom = false; // [crispy]

    m_w = FTOM(f_w);
    m_h = FTOM(f_h);

    // find player to center on initially
    if (!playeringame[pnum = consoleplayer])
        for (pnum = 0; pnum < maxplayers; pnum++)
            if (playeringame[pnum])
                break;
    plr = &players[pnum];
    next_m_x = (plr->mo->x >> FRACTOMAPBITS) - m_w / 2;
    next_m_y = (plr->mo->y >> FRACTOMAPBITS) - m_h / 2;
    AM_Ticker(); // [crispy] initialize variables for interpolation
    AM_changeWindowLoc();

    // for saving & restoring
    old_m_x = m_x;
    old_m_y = m_y;
    old_m_w = m_w;
    old_m_h = m_h;

    // load in the location of keys, if in baby mode

//      memset(KeyPoints, 0, sizeof(vertex_t)*3);
    if (gameskill == sk_baby)
    {
        for (think = thinkercap.next; think != &thinkercap;
             think = think->next)
        {
            if (think->function != P_MobjThinker)
            {                   //not a mobj
                continue;
            }
        }
    }

    // [crispy]
    antialias = crispy->automapoverlay
                 ? &antialias_overlay : &antialias_normal;

    // inform the status bar of the change
//c  ST_Responder(&st_notify);
}

void AM_loadPics(void)
{
    maplump = W_CacheLumpName("AUTOPAGE", PU_STATIC);
}


/*
void AM_clearMarks(void)
{
  int i;
  for (i=0;i<AM_NUMMARKPOINTS;i++) markpoints[i].x = -1; // means empty
  markpointnum = 0;
}
*/

// should be called at the start of every level
// right now, i figure it out myself

void AM_LevelInit(boolean reinit)
{
    // [crispy] Used for reinit
    static int f_h_old;

    leveljuststarted = 0;

    finit_width = SCREENWIDTH;
    finit_height = SCREENHEIGHT - ((ORIGSBARHEIGHT + 3) << crispy->hires);

    f_x = f_y = 0;
    f_w = finit_width;
    f_h = finit_height;
    next_mapxstart = next_mapystart = mapxstart = mapystart = 0;


//  AM_clearMarks();

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
    fixed_t a = FixedDiv(f_w, (max_w>>MAPBITS < 2048) ? 2*(max_w>>MAPBITS) : 4096);
    fixed_t b = FixedDiv(f_h, (max_h>>MAPBITS < 2048) ? 2*(max_h>>MAPBITS) : 4096);
    scale_mtof = FixedDiv(a < b ? a : b, (int) (0.7*MAPUNIT));
    }
    if (scale_mtof > max_scale_mtof)
        scale_mtof = min_scale_mtof;
    scale_ftom = FixedDiv(FRACUNIT, scale_mtof);

    f_h_old = f_h;
}

static boolean stopped = true;

void AM_Stop(void)
{
    //static event_t st_notify = { 0, ev_keyup, AM_MSGEXITED };

//  AM_unloadPics();
    automapactive = false;
//  ST_Responder(&st_notify);
    stopped = true;
    BorderNeedRefresh = true;
}

void AM_Start(void)
{
    static int lastlevel = -1, lastepisode = -1;

    if (!stopped)
        AM_Stop();
    stopped = false;
    if (gamestate != GS_LEVEL)
    {
        return;                 // don't show automap if we aren't in a game!
    }
    if (lastlevel != gamemap || lastepisode != gameepisode)
    {
        AM_LevelInit(false);
        lastlevel = gamemap;
        lastepisode = gameepisode;
    }
    AM_initVariables();
    AM_loadPics();
}

// set the window scale to the maximum size

void AM_minOutWindowScale(void)
{
    scale_mtof = min_scale_mtof;
    scale_ftom = FixedDiv(FRACUNIT, scale_mtof);
    AM_activateNewScale();
}

// set the window scale to the minimum size

void AM_maxOutWindowScale(void)
{
    scale_mtof = max_scale_mtof;
    scale_ftom = FixedDiv(FRACUNIT, scale_mtof);
    AM_activateNewScale();
}

boolean AM_Responder(event_t * ev)
{
    int rc;
    int key;
    static int bigstate = 0;
    static int joywait = 0;
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

    key = ev->data1;

    if (ev->type == ev_joystick && joybautomap >= 0
        && (ev->data1 & (1 << joybautomap)) != 0 && joywait < I_GetTime())
    {
        joywait = I_GetTime() + 5;

        if (!automapactive)
        {
            AM_Start ();
            SB_state = -1;
            viewactive = false;
        }
        else
        {
            bigstate = 0;
            viewactive = true;
            AM_Stop ();
            SB_state = -1;
        }

        return true;
    }


    rc = false;
    if (!automapactive)
    {
        if (ev->type == ev_keydown && key == key_map_toggle
            && gamestate == GS_LEVEL)
        {
            AM_Start();
            SB_state = -1;
            viewactive = false;
            rc = true;
        }
    }
    // [crispy] automap mouse controls
    else if (ev->type == ev_mouse && !crispy->automapoverlay)
    {
        if (mousebmapzoomout >= 0 && ev->data1 & (1 << mousebmapzoomout))
        {
            mtof_zoommul = m_zoomout_mouse;
            ftom_zoommul = m_zoomin_mouse;
            mousewheelzoom = true;
            rc = true;
        }
        else if (mousebmapzoomin >= 0 && ev->data1 & (1 << mousebmapzoomin))
        {
            mtof_zoommul = m_zoomin_mouse;
            ftom_zoommul = m_zoomout_mouse;
            mousewheelzoom = true;
            rc = true;
        }
        else if (mousebmapmaxzoom >= 0 && ev->data1 & (1 << mousebmapmaxzoom))
        {
            bigstate = !bigstate;
            if (bigstate)
            {
                AM_saveScaleAndLoc();
                AM_minOutWindowScale();
            }
            else
                AM_restoreScaleAndLoc();
        }
        else if (mousebmapfollow >= 0 && ev->data1 &(1 << mousebmapfollow))
        {
            followplayer = !followplayer;
            P_SetMessage(plr,
                         followplayer ? AMSTR_FOLLOWON : AMSTR_FOLLOWOFF,
                         true);
        }
        else if (!followplayer && (ev->data2 || ev->data3))
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

        if (key == key_map_east)                 // pan right
        {
            if (!followplayer)
                m_paninc.x = FTOM(f_paninc << crispy->hires);
            else
                rc = false;
        }
        else if (key == key_map_west)                   // pan left
        {
            if (!followplayer)
                m_paninc.x = -FTOM(f_paninc << crispy->hires);
            else
                rc = false;
        }
        else if (key == key_map_north)             // pan up
        {
            if (!followplayer)
                m_paninc.y = FTOM(f_paninc << crispy->hires);
            else
                rc = false;
        }
        else if (key == key_map_south)                   // pan down
        {
            if (!followplayer)
                m_paninc.y = -FTOM(f_paninc << crispy->hires);
            else
                rc = false;
        }
        else if (key == key_map_zoomout)                   // zoom out
        {
            mtof_zoommul = m_zoomout_kbd;
            ftom_zoommul = m_zoomin_kbd;
        }
        else if (key == key_map_zoomin)            // zoom in
        {
            mtof_zoommul = m_zoomin_kbd;
            ftom_zoommul = m_zoomout_kbd;
        }
        else if (key == key_map_toggle)
        {
            bigstate = 0;
            viewactive = true;
            AM_Stop();
            SB_state = -1;
        }
        else if (key == key_map_maxzoom)
        {
            bigstate = !bigstate;
            if (bigstate)
            {
                AM_saveScaleAndLoc();
                AM_minOutWindowScale();
            }
            else
                AM_restoreScaleAndLoc();
        }
        else if (key == key_map_follow)
        {
            followplayer = !followplayer;
            P_SetMessage(plr,
                         followplayer ? AMSTR_FOLLOWON : AMSTR_FOLLOWOFF,
                         true);
        }
        else if (key == key_map_grid)
        {
            // [crispy] support for automap grid
            grid = !grid;
            P_SetMessage(plr,
                         grid ? AMSTR_GRIDON : AMSTR_GRIDOFF,
                         true);
        }
        else if (key == key_map_overlay)
        {
            // [crispy] force redraw status bar
            UpdateState |= I_FULLSCRN;
            SB_state = -1;

            crispy->automapoverlay = !crispy->automapoverlay;
            if (crispy->automapoverlay)
            {
                P_SetMessage(plr, AMSTR_OVERLAYON, true);
                antialias = &antialias_overlay;
            }
            else
            {
                P_SetMessage(plr, AMSTR_OVERLAYOFF, true);
                antialias = &antialias_normal;
            }
        }
        else if (key == key_map_rotate)
        {
            crispy->automaprotate = !crispy->automaprotate;
            if (crispy->automaprotate)
                P_SetMessage(plr, AMSTR_ROTATEON, true);
            else
                P_SetMessage(plr, AMSTR_ROTATEOFF, true);
        }
        else
        {
            rc = false;
        }

        if (cheat_kills[ShowKillsCount] == ev->data1 && netgame && deathmatch)
        {
            ShowKillsCount++;
            if (ShowKillsCount == 5)
            {
                ShowKillsCount = 0;
                rc = false;
                ShowKills ^= 1;
            }
        }
        else
        {
            ShowKillsCount = 0;
        }
    }
    else if (ev->type == ev_keyup)
    {
        rc = false;

        if (key == key_map_east)
        {
            if (!followplayer)
                m_paninc.x = 0;
        }
        else if (key == key_map_west)
        {
            if (!followplayer)
                m_paninc.x = 0;
        }
        else if (key == key_map_north)
        {
            if (!followplayer)
                m_paninc.y = 0;
        }
        else if (key == key_map_south)
        {
            if (!followplayer)
                m_paninc.y = 0;
        }
        else if (key == key_map_zoomin || key == key_map_zoomout)
        {
            mtof_zoommul = FRACUNIT;
            ftom_zoommul = FRACUNIT;
        }
    }
    return rc;
}

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

        // do the parallax parchment scrolling.
/*
	 dmapx = (MTOF(plr->mo->x)-MTOF(f_oldloc.x)); //fixed point
	 dmapy = (MTOF(f_oldloc.y)-MTOF(plr->mo->y));

	 if(f_oldloc.x == INT_MAX) //to eliminate an error when the user first
		dmapx=0;  //goes into the automap.
	 mapxstart += dmapx;
	 mapystart += dmapy;

  	 while(mapxstart >= finit_width)
			mapxstart -= finit_width;
    while(mapxstart < 0)
			mapxstart += finit_width;
    while(mapystart >= finit_height)
			mapystart -= finit_height;
    while(mapystart < 0)
			mapystart += finit_height;
*/
}

// Ripped out for Heretic
/*
void AM_updateLightLev(void)
{
  static nexttic = 0;
//static int litelevels[] = { 0, 3, 5, 6, 6, 7, 7, 7 };
  static int litelevels[] = { 0, 4, 7, 10, 12, 14, 15, 15 };
  static int litelevelscnt = 0;

  // Change light level
  if (amclock>nexttic)
  {
    lightlev = litelevels[litelevelscnt++];
    if (litelevelscnt == sizeof(litelevels)/sizeof(int)) litelevelscnt = 0;
    nexttic = amclock + 6 - (amclock % 6);
  }
}
*/

void AM_Ticker(void)
{

    if (!automapactive)
        return;

    amclock++;

    // [crispy] sync up for interpolation
    m_x = prev_m_x = next_m_x;
    m_y = prev_m_y = next_m_y;
    mapxstart = prev_mapxstart = next_mapxstart;
    mapystart = prev_mapystart = next_mapystart;

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

void AM_clearFB(int color)
{
    int i, j;
    int dmapx;
    int dmapy;
    int x1, x2, x3;

    if (followplayer && !paused)
    {
        dmapx = (MTOF(plr->mo->x) >> FRACTOMAPBITS) - (MTOF(plr->mo->oldx) >> FRACTOMAPBITS);
        dmapy = (MTOF(plr->mo->oldy) >> FRACTOMAPBITS) - (MTOF(plr->mo->y) >> FRACTOMAPBITS);

//              if(f_oldloc.x == INT_MAX) //to eliminate an error when the user first
//                      dmapx=0;  //goes into the automap.

        // [crispy] Disable map background scroll in rotate mode. The
        // combination of the two effects is unappealing and slightly
        // nauseating.
        if (!crispy->automaprotate)
        {
            mapxstart = prev_mapxstart + (dmapx >> 1);
            mapystart = prev_mapystart + (dmapy >> 1);
        }

        // [crispy] Change background tile dimensions for hi-res
        while (mapxstart >= MAPBGROUNDWIDTH << crispy->hires)
            mapxstart -= MAPBGROUNDWIDTH << crispy->hires;
        while (mapxstart < 0)
            mapxstart += MAPBGROUNDWIDTH << crispy->hires;
        while (mapystart >= MAPBGROUNDHEIGHT >> crispy->hires)
            mapystart -= MAPBGROUNDHEIGHT >> crispy->hires;
        while (mapystart < 0)
            mapystart += MAPBGROUNDHEIGHT >> crispy->hires;

        // [crispy] Follow mode interpolation does not need the special
        // treatment that non-follow mode gets.
        next_mapxstart = mapxstart;
        next_mapystart = mapystart;
    }
    else
    {
        // The released Hexen source does this here, but this causes a bug
        // where the map background keeps moving when we reach the map
        // boundaries. This is instead done in AM_changeWindowLoc.
        /*
        mapxstart += (MTOF(m_paninc.x) >> 1);
        mapystart -= (MTOF(m_paninc.y) >> 1);
        if (mapxstart >= (finit_width >> crispy->hires))
            mapxstart -= (finit_width >> crispy->hires);
        if (mapxstart < 0)
            mapxstart += (finit_width >> crispy->hires);
        if (mapystart >= (finit_height >> crispy->hires))
            mapystart -= (finit_height >> crispy->hires);
        if (mapystart < 0)
            mapystart += (finit_height >> crispy->hires);
        */
    }

    //blit the automap background to the screen.

    // [crispy] To support widescreen, increase the number of possible
    // background tiles from 2 to 3. To support rendering at 2x resolution,
    // treat original 320 x 158 tile image as 640 x 79.
    j = mapystart * (MAPBGROUNDWIDTH << crispy->hires);

    x1 = mapxstart;
    x2 = finit_width - x1;

    if (x2 > MAPBGROUNDWIDTH << crispy->hires)
        x2 = MAPBGROUNDWIDTH << crispy->hires;

    x3 = finit_width - x2 - x1;

    for (i = 0; i < SCREENHEIGHT - SBARHEIGHT; i++)
    {
        memcpy(I_VideoBuffer + i * finit_width,
               maplump + j + (MAPBGROUNDWIDTH << crispy->hires) - x3, x3);

        memcpy(I_VideoBuffer + i * finit_width + x3,
               maplump + j + (MAPBGROUNDWIDTH << crispy->hires) - x2, x2);

        memcpy(I_VideoBuffer + i * finit_width + x2 + x3,
               maplump + j, x1);

        j += MAPBGROUNDWIDTH << crispy->hires;
        if (j >= MAPBGROUNDHEIGHT * MAPBGROUNDWIDTH)
            j = 0;
    }

//       memcpy(I_VideoBuffer, maplump, finit_width*finit_height);
//  memset(fb, color, f_w*f_h);
}

// Based on Cohen-Sutherland clipping algorithm but with a slightly
// faster reject and precalculated slopes.  If I need the speed, will
// hash algorithm to the common cases.

boolean AM_clipMline(mline_t * ml, fline_t * fl)
{
    enum
    { LEFT = 1, RIGHT = 2, BOTTOM = 4, TOP = 8 };
    int outcode1 = 0, outcode2 = 0, outside;
    fpoint_t tmp = { 0, 0 };
    int dx, dy;

#define DOOUTCODE(oc, mx, my) \
  (oc) = 0; \
  if ((my) < 0) (oc) |= TOP; \
  else if ((my) >= f_h) (oc) |= BOTTOM; \
  if ((mx) < 0) (oc) |= LEFT; \
  else if ((mx) >= f_w) (oc) |= RIGHT

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
        return false;           // trivially outside

    if (ml->a.x < m_x)
        outcode1 |= LEFT;
    else if (ml->a.x > m_x2)
        outcode1 |= RIGHT;
    if (ml->b.x < m_x)
        outcode2 |= LEFT;
    else if (ml->b.x > m_x2)
        outcode2 |= RIGHT;
    if (outcode1 & outcode2)
        return false;           // trivially outside

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
            tmp.x = fl->a.x + (fixed_t)(((int64_t)dx * (fl->a.y - f_y)) / dy);
            tmp.y = 0;
        }
        else if (outside & BOTTOM)
        {
            dy = fl->a.y - fl->b.y;
            dx = fl->b.x - fl->a.x;
            tmp.x = fl->a.x + (fixed_t)(((int64_t)dx * (fl->a.y - (f_y+f_h))) / dy);
            tmp.y = f_h - 1;
        }
        else if (outside & RIGHT)
        {
            dy = fl->b.y - fl->a.y;
            dx = fl->b.x - fl->a.x;
            tmp.y = fl->a.y + (fixed_t)(((int64_t)dy * (f_x + f_w - 1 - fl->a.x)) / dx);
            tmp.x = f_w - 1;
        }
        else if (outside & LEFT)
        {
            dy = fl->b.y - fl->a.y;
            dx = fl->b.x - fl->a.x;
            tmp.y = fl->a.y + (fixed_t)(((int64_t)dy * (f_x - fl->a.x)) / dx);
            tmp.x = 0;
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
            return false;       // trivially outside
    }

    return true;
}

#undef DOOUTCODE

// Classic Bresenham w/ whatever optimizations I need for speed

void AM_drawFline(fline_t * fl, int color)
{
    register int x, y, dx, dy, sx, sy, ax, ay, d;
    //static fuck = 0;

    switch (color)
    {
        case WALLCOLORS:
            DrawWuLine(fl->a.x, fl->a.y, fl->b.x, fl->b.y,
                       &(*antialias)[0][0], 8, 3);
            break;
        case FDWALLCOLORS:
            DrawWuLine(fl->a.x, fl->a.y, fl->b.x, fl->b.y,
                       &(*antialias)[1][0], 8, 3);
            break;
        case CDWALLCOLORS:
            DrawWuLine(fl->a.x, fl->a.y, fl->b.x, fl->b.y,
                       &(*antialias)[2][0], 8, 3);
            break;
        default:
            {
                // For debugging only
                if (fl->a.x < 0 || fl->a.x >= f_w
                    || fl->a.y < 0 || fl->a.y >= f_h
                    || fl->b.x < 0 || fl->b.x >= f_w
                    || fl->b.y < 0 || fl->b.y >= f_h)
                {
                    //fprintf(stderr, "fuck %d \r", fuck++);
                    return;
                }

#define DOT(xx,yy,cc) fb[(yy)*f_w+(xx)]=(cc)    //the MACRO!

                dx = fl->b.x - fl->a.x;
                ax = 2 * (dx < 0 ? -dx : dx);
                sx = dx < 0 ? -1 : 1;

                dy = fl->b.y - fl->a.y;
                ay = 2 * (dy < 0 ? -dy : dy);
                sy = dy < 0 ? -1 : 1;

                x = fl->a.x;
                y = fl->a.y;

                if (ax > ay)
                {
                    d = ay - ax / 2;
                    while (1)
                    {
                        DOT(x, y, color);
                        if (x == fl->b.x)
                            return;
                        if (d >= 0)
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
                    d = ax - ay / 2;
                    while (1)
                    {
                        DOT(x, y, color);
                        if (y == fl->b.y)
                            return;
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
    }
}

/* Wu antialiased line drawer.
 * (X0,Y0),(X1,Y1) = line to draw
 * BaseColor = color # of first color in block used for antialiasing, the
 *          100% intensity version of the drawing color
 * NumLevels = size of color block, with BaseColor+NumLevels-1 being the
 *          0% intensity version of the drawing color
 * IntensityBits = log base 2 of NumLevels; the # of bits used to describe
 *          the intensity of the drawing color. 2**IntensityBits==NumLevels
 */
void PUTDOT(short xx, short yy, byte * cc, byte * cm)
{
    static int oldyy;
    static int oldyyshifted;
    byte *oldcc = cc;

    if (xx < 32)
        cc += 7 - (xx >> 2);
    else if (xx > (finit_width - 32))
        cc += 7 - ((finit_width - xx) >> 2);
//      if(cc==oldcc) //make sure that we don't double fade the corners.
//      {
    if (yy < 32)
        cc += 7 - (yy >> 2);
    else if (yy > (finit_height - 32))
        cc += 7 - ((finit_height - yy) >> 2);
//      }
    if (cc > cm && cm != NULL)
    {
        cc = cm;
    }
    else if (cc > oldcc + 6)    // don't let the color escape from the fade table...
    {
        cc = oldcc + 6;
    }
    if (yy == oldyy + 1)
    {
        oldyy++;
        oldyyshifted += f_w;
    }
    else if (yy == oldyy - 1)
    {
        oldyy--;
        oldyyshifted -= f_w;
    }
    else if (yy != oldyy)
    {
        oldyy = yy;
        oldyyshifted = yy * f_w;
    }
    fb[oldyyshifted + xx] = *(cc);
//      fb[(yy)*f_w+(xx)]=*(cc);
}

void DrawWuLine(int X0, int Y0, int X1, int Y1, byte * BaseColor,
                int NumLevels, unsigned short IntensityBits)
{
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
    PUTDOT(X0, Y0, &BaseColor[0], NULL);

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
            PUTDOT(X0, Y0, &BaseColor[0], NULL);
        }
        return;
    }
    if (DeltaX == 0)
    {
        /* Vertical line */
        do
        {
            Y0++;
            PUTDOT(X0, Y0, &BaseColor[0], NULL);
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
            PUTDOT(X0, Y0, &BaseColor[0], NULL);
        }
        while (--DeltaY != 0);
        return;
    }
    /* Line is not horizontal, diagonal, or vertical */
    ErrorAcc = 0;               /* initialize the line error accumulator to 0 */
    /* # of bits by which to shift ErrorAcc to get intensity level */
    IntensityShift = 16 - IntensityBits;
    /* Mask used to flip all bits in an intensity weighting, producing the
       result (1 - intensity weighting) */
    WeightingComplementMask = NumLevels - 1;
    /* Is this an X-major or Y-major line? */
    if (DeltaY > DeltaX)
    {
        /* Y-major line; calculate 16-bit fixed-point fractional part of a
           pixel that X advances each time Y advances 1 pixel, truncating the
           result so that we won't overrun the endpoint along the X axis */
        ErrorAdj = ((unsigned long) DeltaX << 16) / (unsigned long) DeltaY;
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
            PUTDOT(X0, Y0, &BaseColor[Weighting], &BaseColor[7]);
            PUTDOT(X0 + XDir, Y0,
                   &BaseColor[(Weighting ^ WeightingComplementMask)],
                   &BaseColor[7]);
        }
        /* Draw the final pixel, which is always exactly intersected by the line
           and so needs no weighting */
        PUTDOT(X1, Y1, &BaseColor[0], NULL);
        return;
    }
    /* It's an X-major line; calculate 16-bit fixed-point fractional part of a
       pixel that Y advances each time X advances 1 pixel, truncating the
       result to avoid overrunning the endpoint along the X axis */
    ErrorAdj = ((unsigned long) DeltaY << 16) / (unsigned long) DeltaX;
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
        PUTDOT(X0, Y0, &BaseColor[Weighting], &BaseColor[7]);
        PUTDOT(X0, Y0 + 1,
               &BaseColor[(Weighting ^ WeightingComplementMask)],
               &BaseColor[7]);

    }
    /* Draw the final pixel, which is always exactly intersected by the line
       and so needs no weighting */
    PUTDOT(X1, Y1, &BaseColor[0], NULL);
}

void AM_drawMline(mline_t * ml, int color)
{
    static fline_t fl;

    if (AM_clipMline(ml, &fl))
        AM_drawFline(&fl, color);       // draws it on frame buffer using fb coords

}

void AM_drawGrid(int color)
{
    fixed_t x, y;
    fixed_t start, end;
    const fixed_t gridsize = MAPBLOCKUNITS << MAPBITS;
    mline_t ml;

    // Figure out start of vertical gridlines
    start = m_x;
    if (crispy->automaprotate)
    {
        start -= m_h / 2;
    }
    // [crispy] fix losing grid lines near the automap boundary
    if ((start - (bmaporgx >> FRACTOMAPBITS)) % gridsize)
        start += // (MAPBLOCKUNITS << FRACBITS)
            - ((start - (bmaporgx >> FRACTOMAPBITS)) % gridsize);
    end = m_x + m_w;
    if (crispy->automaprotate)
    {
        end += m_h / 2;
    }

    // draw vertical gridlines
    for (x = start; x < end; x += gridsize)
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
    if ((start - (bmaporgy >> FRACTOMAPBITS)) % gridsize)
        start += // (MAPBLOCKUNITS << FRACBITS)
            - ((start - (bmaporgy >> FRACTOMAPBITS)) % gridsize);
    end = m_y + m_h;
    if (crispy->automaprotate)
    {
        end += m_w / 2;
    }

    // draw horizontal gridlines
    for (y = start; y < end; y += gridsize)
    {
        ml.a.y = y;
        ml.b.y = y;
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

void AM_drawWalls(void)
{
    int i;
    static mline_t l;

    for (i = 0; i < numlines; i++)
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
            if (!lines[i].backsector)
            {
                AM_drawMline(&l, WALLCOLORS + lightlev);
            }
            else
            {
                if (lines[i].flags & ML_SECRET) // secret door
                {
                    if (cheating)
                        AM_drawMline(&l, 0);
                    else
                        AM_drawMline(&l, WALLCOLORS + lightlev);
                }
                else if (lines[i].special == 13 || lines[i].special == 83)
                {               // Locked door line -- all locked doors are greed
                    AM_drawMline(&l, GREENKEY);
                }
                else if (lines[i].special == 70 || lines[i].special == 71)
                {               // intra-level teleports are blue
                    AM_drawMline(&l, BLUEKEY);
                }
                else if (lines[i].special == 74 || lines[i].special == 75)
                {               // inter-level teleport/game-winning exit -- both are red
                    AM_drawMline(&l, BLOODRED);
                }
                else if (lines[i].backsector->floorheight
                         != lines[i].frontsector->floorheight)
                {
                    AM_drawMline(&l, FDWALLCOLORS + lightlev);  // floor level change
                }
                else if (lines[i].backsector->ceilingheight
                         != lines[i].frontsector->ceilingheight)
                {
                    AM_drawMline(&l, CDWALLCOLORS + lightlev);  // ceiling level change
                }
                else if (cheating)
                {
                    AM_drawMline(&l, TSWALLCOLORS + lightlev);
                }
            }
        }
        else if (plr->powers[pw_allmap])
        {
            if (!(lines[i].flags & LINE_NEVERSEE))
                AM_drawMline(&l, GRAYS + 3);
        }
    }

}

void AM_rotate(fixed_t * x, fixed_t * y, angle_t a)
{
    fixed_t tmpx;

    tmpx = FixedMul(*x, finecosine[a >> ANGLETOFINESHIFT])
        - FixedMul(*y, finesine[a >> ANGLETOFINESHIFT]);
    *y = FixedMul(*x, finesine[a >> ANGLETOFINESHIFT])
        + FixedMul(*y, finecosine[a >> ANGLETOFINESHIFT]);
    *x = tmpx;
}

// [crispy] rotate point around map center
// adapted from prboom-plus/src/am_map.c
static void AM_rotatePoint(mpoint_t *pt)
{
    fixed_t tmpx;
    // [crispy] smooth automap rotation
    const angle_t smoothangle = followplayer ? ANG90 - viewangle : mapangle;

    pt->x -= mapcenter.x;
    pt->y -= mapcenter.y;

    tmpx = FixedMul(pt->x, finecosine[smoothangle>>ANGLETOFINESHIFT])
         - FixedMul(pt->y, finesine[smoothangle>>ANGLETOFINESHIFT])
         + mapcenter.x;

    pt->y = FixedMul(pt->x, finesine[smoothangle>>ANGLETOFINESHIFT])
          + FixedMul(pt->y, finecosine[smoothangle>>ANGLETOFINESHIFT])
          + mapcenter.y;

    pt->x = tmpx;
}

void AM_drawLineCharacter(mline_t * lineguy, int lineguylines, fixed_t scale,
                          angle_t angle, int color, fixed_t x, fixed_t y)
{
    int i;
    mline_t l;

    if (crispy->automaprotate)
    {
        angle += mapangle;
    }

    for (i = 0; i < lineguylines; i++)
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
    int i;
    player_t *p;
    static int their_colors[] = {
        AM_PLR1_COLOR,
        AM_PLR2_COLOR,
        AM_PLR3_COLOR,
        AM_PLR4_COLOR,
        AM_PLR5_COLOR,
        AM_PLR6_COLOR,
        AM_PLR7_COLOR,
        AM_PLR8_COLOR
    };
    int their_color = -1;
    int color;
    mpoint_t pt; // [crispy]
    // [crispy] smooth player arrow rotation
    const angle_t smoothangle = crispy->automaprotate ? plr->mo->angle : viewangle;

    if (!netgame)
    {
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

        AM_drawLineCharacter(player_arrow, NUMPLYRLINES, 0, smoothangle,
                             WHITE, pt.x, pt.y);
        return;
    }

    for (i = 0; i < maxplayers; i++)
    {
        their_color++;
        p = &players[i];
        if (deathmatch && !singledemo && p != plr)
        {
            continue;
        }
        if (!playeringame[i])
            continue;
        color = their_colors[their_color];

        pt.x = p->mo->x >> FRACTOMAPBITS;
        pt.y = p->mo->y >> FRACTOMAPBITS;
        if (crispy->automaprotate)
        {
            AM_rotatePoint(&pt);
        }
        AM_drawLineCharacter(player_arrow, NUMPLYRLINES, 0, p->mo->angle,
                             color, pt.x, pt.y);
    }
}

void AM_drawThings(int colors, int colorrange)
{
    int i;
    mobj_t *t;
    mpoint_t pt; // [crispy]

    for (i = 0; i < numsectors; i++)
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
            AM_drawLineCharacter(thintriangle_guy, NUMTHINTRIANGLEGUYLINES,
                                 16 << MAPBITS, t->angle, colors + lightlev,
                                 pt.x, pt.y);
            t = t->snext;
        }
    }
}

/*
void AM_drawMarks(void)
{
  int i, fx, fy, w, h;

  for (i=0;i<AM_NUMMARKPOINTS;i++)
  {
    if (markpoints[i].x != -1)
    {
      w = SHORT(marknums[i]->width);
      h = SHORT(marknums[i]->height);
      fx = CXMTOF(markpoints[i].x);
      fy = CYMTOF(markpoints[i].y);
      if (fx >= f_x && fx <= f_w - w && fy >= f_y && fy <= f_h - h)
  			V_DrawPatch(fx, fy, marknums[i]);
    }
  }
}
*/
/*
void AM_drawkeys(void)
{
	if(KeyPoints[0].x != 0 || KeyPoints[0].y != 0)
	{
		AM_drawLineCharacter(keysquare, NUMKEYSQUARELINES, 0, 0, YELLOWKEY,
			KeyPoints[0].x, KeyPoints[0].y);
	}
	if(KeyPoints[1].x != 0 || KeyPoints[1].y != 0)
	{
		AM_drawLineCharacter(keysquare, NUMKEYSQUARELINES, 0, 0, GREENKEY,
			KeyPoints[1].x, KeyPoints[1].y);
	}
	if(KeyPoints[2].x != 0 || KeyPoints[2].y != 0)
	{
		AM_drawLineCharacter(keysquare, NUMKEYSQUARELINES, 0, 0, BLUEKEY,
			KeyPoints[2].x, KeyPoints[2].y);
	}
}
*/

/*
void AM_drawCrosshair(int color)
{
  fb[(f_w*(f_h+1))/2] = color; // single point for now
}
*/

void AM_Drawer(void)
{
    if (!automapactive)
        return;

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
    UpdateState |= I_FULLSCRN;
    if (!crispy->automapoverlay)
    {
        AM_clearFB(BACKGROUND);
    }
    if (grid)
        AM_drawGrid(GRIDCOLORS);
    AM_drawWalls();
    AM_drawPlayers();
    DrawWorldTimer();

    if (cheating == 2)
        AM_drawThings(THINGCOLORS, THINGRANGE);

//  AM_drawCrosshair(XHAIRCOLORS);
//  AM_drawMarks();
//      if(gameskill == sk_baby) AM_drawkeys();

    MN_DrTextA(P_GetMapName(gamemap), 38, 144);
    if (ShowKills && netgame && deathmatch)
    {
        AM_DrawDeathmatchStats();
    }
//  I_Update();
//  V_MarkRect(f_x, f_y, f_w, f_h);

}

//===========================================================================
//
// AM_DrawDeathmatchStats
//
//===========================================================================

// 8-player note:  Proper player color names here, too

const char *PlayerColorText[MAXPLAYERS] = {
    "BLUE:",
    "RED:",
    "YELLOW:",
    "GREEN:",
    "JADE:",
    "WHITE:",
    "HAZEL:",
    "PURPLE:"
};

void AM_DrawDeathmatchStats(void)
{
    int i, j, k, m;
    int fragCount[MAXPLAYERS];
    int order[MAXPLAYERS];
    char textBuffer[80];
    int yPosition;

    for (i = 0; i < maxplayers; i++)
    {
        fragCount[i] = 0;
        order[i] = -1;
    }
    for (i = 0; i < maxplayers; i++)
    {
        if (!playeringame[i])
        {
            continue;
        }
        else
        {
            for (j = 0; j < maxplayers; j++)
            {
                if (playeringame[j])
                {
                    fragCount[i] += players[i].frags[j];
                }
            }
            for (k = 0; k < maxplayers; k++)
            {
                if (order[k] == -1)
                {
                    order[k] = i;
                    break;
                }
                else if (fragCount[i] > fragCount[order[k]])
                {
                    for (m = maxplayers - 1; m > k; m--)
                    {
                        order[m] = order[m - 1];
                    }
                    order[k] = i;
                    break;
                }
            }
        }
    }
    yPosition = 15;
    for (i = 0; i < maxplayers; i++)
    {
        if (!playeringame[order[i]])
        {
            continue;
        }
        else
        {
            MN_DrTextA(PlayerColorText[order[i]], 8, yPosition);
            M_snprintf(textBuffer, sizeof(textBuffer),
                       "%d", fragCount[order[i]]);
            MN_DrTextA(textBuffer, 80, yPosition);
            yPosition += 10;
        }
    }
}

//===========================================================================
//
// DrawWorldTimer
//
//===========================================================================

static void DrawWorldTimer(void)
{
    int days;
    int hours;
    int minutes;
    int seconds;
    int worldTimer;
    char timeBuffer[15];
    char dayBuffer[20];

    worldTimer = players[consoleplayer].worldTimer;

    worldTimer /= 35;
    days = worldTimer / 86400;
    worldTimer -= days * 86400;
    hours = worldTimer / 3600;
    worldTimer -= hours * 3600;
    minutes = worldTimer / 60;
    worldTimer -= minutes * 60;
    seconds = worldTimer;

    M_snprintf(timeBuffer, sizeof(timeBuffer),
               "%.2d : %.2d : %.2d", hours, minutes, seconds);
    MN_DrTextA(timeBuffer, 240, 8);

    if (days)
    {
        if (days == 1)
        {
            M_snprintf(dayBuffer, sizeof(dayBuffer), "%.2d DAY", days);
        }
        else
        {
            M_snprintf(dayBuffer, sizeof(dayBuffer), "%.2d DAYS", days);
        }
        MN_DrTextA(dayBuffer, 240, 20);
        if (days >= 5)
        {
            MN_DrTextA("YOU FREAK!!!", 230, 35);
        }
    }
}
