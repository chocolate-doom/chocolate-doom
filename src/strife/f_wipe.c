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
//	Mission begin melt/wipe screen special effect.
//

#include <string.h>

#include "z_zone.h"
#include "i_video.h"
#include "v_trans.h" // [crispy] blending functions
#include "v_video.h"
#include "m_random.h"

#include "doomtype.h"

#include "r_defs.h"   // haleyjd [STRIFE]
#include "r_draw.h"

#include "f_wipe.h"

//
//                       SCREEN WIPE PACKAGE
//

// when zero, stop the wipe
static boolean	go = 0;

static pixel_t*	wipe_scr_start;
static pixel_t*	wipe_scr_end;
static pixel_t*	wipe_scr;

// [crispy] Additional fail-safe counter for performing crossfade effect.
// Vanilla Strife goes into an infinite loop when using an imprecise XLATAB
// during a crossfade, so we stop it when the counter reaches zero. It also
// acts as an opacity multiplier in TrueColor mode.
static int fade_counter;

void
wipe_shittyColMajorXform
( dpixel_t*	array,
  int		width,
  int		height )
{
    int		x;
    int		y;
    dpixel_t*	dest;

    dest = (dpixel_t*) Z_Malloc(width*height*sizeof(*dest), PU_STATIC, 0);

    for(y=0;y<height;y++)
	for(x=0;x<width;x++)
	    dest[x*height+y] = array[y*width+x];

    memcpy(array, dest, width*height*sizeof(*dest));

    Z_Free(dest);

}

// haleyjd 08/26/10: [STRIFE] Verified unmodified.
int
wipe_initColorXForm
( int	width,
  int	height,
  int	ticks )
{
    memcpy(wipe_scr, wipe_scr_start, width*height*sizeof(*wipe_scr));
    // [crispy] arm fail-safe crossfade counter with 13 screen transitions
    // to make crossfading duration identical to Vanilla Strife
    fade_counter = 13;
    return 0;
}

//
// wipe_doColorXForm
//
// haleyjd 08/26/10: [STRIFE]
// * Rogue modified the unused ColorXForm wipe in-place in order to implement 
//   their distinctive crossfade wipe.
//
int
wipe_doColorXForm
( int	width,
  int	height,
  int	ticks )
{
    pixel_t *cur_screen = wipe_scr;
    pixel_t *end_screen = wipe_scr_end;
    int   pix = width*height;
    int   i;
    boolean changed = false;

    // [crispy] reduce fail-safe crossfade counter tics
    if (fade_counter--)
    {
    for(i = pix; i > 0; i--)
    {
        if(*cur_screen != *end_screen)
        {
            changed = true;
#ifndef CRISPY_TRUECOLOR
            *cur_screen = xlatab[(*cur_screen << 8) + *end_screen];
#else
            // [crispy] perform TrueColour blending with the following 13 opacity levels:
            // 250, 230, 210, 190, 170, 150, 130, 110, 90, 70, 50, 30, 10
            *cur_screen = I_BlendOver(*end_screen, *cur_screen, (fade_counter * 20) + 10);
#endif
        }
        ++cur_screen;
        ++end_screen;
    }
    }

    return !changed;
}

// haleyjd 08/26/10: [STRIFE] Verified unmodified.
int
wipe_exitColorXForm
( int	width,
  int	height,
  int	ticks )
{
    // [crispy] fix memory leak - crossfade calls wipe_exitColorXForm instead of
    // wipe_exitMelt, so we need to do a memory cleanup in this function
    Z_Free(wipe_scr_start);
    Z_Free(wipe_scr_end);
    Z_Free(wipe_scr);
    return 0;
}


static int*	y;

int
wipe_initMelt
( int	width,
  int	height,
  int	ticks )
{
    int i, r;
    
    // copy start screen to main screen
    memcpy(wipe_scr, wipe_scr_start, width*height*sizeof(*wipe_scr));
    
    // makes this wipe faster (in theory)
    // to have stuff in column-major format
    wipe_shittyColMajorXform((dpixel_t*)wipe_scr_start, width/2, height);
    wipe_shittyColMajorXform((dpixel_t*)wipe_scr_end, width/2, height);
    
    // setup initial column positions
    // (y<0 => not ready to scroll yet)
    y = (int *) Z_Malloc(width*sizeof(int), PU_STATIC, 0);
    y[0] = -(M_Random()%16);
    for (i=1;i<width;i++)
    {
	r = (M_Random()%3) - 1;
	y[i] = y[i-1] + r;
	if (y[i] > 0) y[i] = 0;
	else if (y[i] == -16) y[i] = -15;
    }

    return 0;
}

int
wipe_doMelt
( int	width,
  int	height,
  int	ticks )
{
    int		i;
    int		j;
    int		dy;
    int		idx;
    
    dpixel_t*	s;
    dpixel_t*	d;
    boolean	done = true;

    width/=2;

    while (ticks--)
    {
	for (i=0;i<width;i++)
	{
	    if (y[i]<0)
	    {
		y[i]++; done = false;
	    }
	    else if (y[i] < height)
	    {
		dy = (y[i] < 16) ? y[i]+1 : 8;
		if (y[i]+dy >= height) dy = height - y[i];
		s = &((dpixel_t *)wipe_scr_end)[i*height+y[i]];
		d = &((dpixel_t *)wipe_scr)[y[i]*width+i];
		idx = 0;
		for (j=dy;j;j--)
		{
		    d[idx] = *(s++);
		    idx += width;
		}
		y[i] += dy;
		s = &((dpixel_t *)wipe_scr_start)[i*height];
		d = &((dpixel_t *)wipe_scr)[y[i]*width+i];
		idx = 0;
		for (j=height-y[i];j;j--)
		{
		    d[idx] = *(s++);
		    idx += width;
		}
		done = false;
	    }
	}
    }

    return done;

}

int
wipe_exitMelt
( int	width,
  int	height,
  int	ticks )
{
    Z_Free(y);
    Z_Free(wipe_scr_start);
    Z_Free(wipe_scr_end);
    return 0;
}

// haleyjd 08/26/10: [STRIFE] Verified unmodified.
int
wipe_StartScreen
( int	x,
  int	y,
  int	width,
  int	height )
{
    wipe_scr_start = Z_Malloc(SCREENWIDTH * SCREENHEIGHT * sizeof(*wipe_scr_start), PU_STATIC, NULL);
    I_ReadScreen(wipe_scr_start);
    return 0;
}

// haleyjd 08/26/10: [STRIFE] Verified unmodified.
int
wipe_EndScreen
( int	x,
  int	y,
  int	width,
  int	height )
{
    wipe_scr_end = Z_Malloc(SCREENWIDTH * SCREENHEIGHT * sizeof(*wipe_scr_end), PU_STATIC, NULL);
    I_ReadScreen(wipe_scr_end);
    V_DrawBlock(x, y, width, height, wipe_scr_start); // restore start scr.
    return 0;
}

// haleyjd 08/26/10: [STRIFE] Verified unmodified.
int
wipe_ScreenWipe
( int	wipeno,
  int	x,
  int	y,
  int	width,
  int	height,
  int	ticks )
{
    int rc;
    static int (*wipes[])(int, int, int) =
    {
	wipe_initColorXForm, wipe_doColorXForm, wipe_exitColorXForm,
	wipe_initMelt, wipe_doMelt, wipe_exitMelt
    };

    // initial stuff
    if (!go)
    {
	go = 1;
        // haleyjd 20110629 [STRIFE]: We *must* use a temp buffer here.
	wipe_scr = (pixel_t *) Z_Malloc(width*height*sizeof(*wipe_scr), PU_STATIC, 0); // DEBUG
	//wipe_scr = I_VideoBuffer;
	(*wipes[wipeno*3])(width, height, ticks);
    }

    // do a piece of wipe-in
    V_MarkRect(0, 0, width, height);
    rc = (*wipes[wipeno*3+1])(width, height, ticks);

    // haleyjd 20110629 [STRIFE]: Copy temp buffer to the real screen.
    V_DrawBlock(x, y, width, height, wipe_scr);

    // final stuff
    if (rc)
    {
	go = 0;
	(*wipes[wipeno*3+2])(width, height, ticks);
    }

    return !go;
}

