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
//	The actual span/column drawing functions.
//	Here find the main potential for optimization,
//	 e.g. inline assembly, different algorithms.
//




#include "doomdef.h"
#include "deh_main.h"

#include "i_system.h"
#include "z_zone.h"
#include "w_wad.h"

#include "r_local.h"

// Needs access to LFB (guess what).
#include "v_video.h"

// State.
#include "doomstat.h"


// ?
#define MAXWIDTH			1120
#define MAXHEIGHT			832

// status bar height at bottom of screen
// haleyjd 08/31/10: Verified unmodified.
#define SBARHEIGHT              32

//
// All drawing to the view buffer is accomplished in this file.
// The other refresh files only know about ccordinates,
//  not the architecture of the frame buffer.
// Conveniently, the frame buffer is a linear one,
//  and we need only the base address,
//  and the total size == width*height*depth/8.,
//


byte*		viewimage; 
int		viewwidth;
int		scaledviewwidth;
int		viewheight;
int		viewwindowx;
int		viewwindowy; 
byte*		ylookup[MAXHEIGHT]; 
int		columnofs[MAXWIDTH]; 

// Color tables for different players,
//  translate a limited part to another
//  (color ramps used for  suit colors).
//
// [STRIFE] Unused.
//byte          translations[3][256];	
 
// Backing buffer containing the bezel drawn around the screen and 
// surrounding background.

static byte *background_buffer = NULL;

// haleyjd 08/29/10: [STRIFE] Rogue added the ability to customize the view
// border flat by storing it in the configuration file.
char *back_flat = "F_PAVE01";

//
// R_DrawColumn
// Source is the top of the column to scale.
//
lighttable_t*		dc_colormap; 
int			dc_x; 
int			dc_yl; 
int			dc_yh; 
fixed_t			dc_iscale; 
fixed_t			dc_texturemid;

// first pixel in a column (possibly virtual) 
byte*			dc_source;		

// just for profiling 
int			dccount;

//
// A column is a vertical slice/span from a wall texture that,
//  given the DOOM style restrictions on the view orientation,
//  will always have constant z depth.
// Thus a special case loop for very fast rendering can
//  be used. It has also been used with Wolfenstein 3D.
// 
void R_DrawColumn (void) 
{ 
    int			count; 
    byte*		dest; 
    fixed_t		frac;
    fixed_t		fracstep;	 
 
    count = dc_yh - dc_yl; 

    // Zero length, column does not exceed a pixel.
    if (count < 0) 
	return; 
				 
#ifdef RANGECHECK 
    if ((unsigned)dc_x >= SCREENWIDTH
	|| dc_yl < 0
	|| dc_yh >= SCREENHEIGHT) 
	I_Error ("R_DrawColumn: %i to %i at %i", dc_yl, dc_yh, dc_x); 
#endif 

    // Framebuffer destination address.
    // Use ylookup LUT to avoid multiply with ScreenWidth.
    // Use columnofs LUT for subwindows? 
    dest = ylookup[dc_yl] + columnofs[dc_x];  

    // Determine scaling,
    //  which is the only mapping to be done.
    fracstep = dc_iscale; 
    frac = dc_texturemid + (dc_yl-centery)*fracstep; 

    // Inner loop that does the actual texture mapping,
    //  e.g. a DDA-lile scaling.
    // This is as fast as it gets.
    do 
    {
	// Re-map color indices from wall texture column
	//  using a lighting/special effects LUT.
	*dest = dc_colormap[dc_source[(frac>>FRACBITS)&127]];
	
	dest += SCREENWIDTH; 
	frac += fracstep;
	
    } while (count--); 
} 



// UNUSED.
// Loop unrolled.
#if 0
void R_DrawColumn (void) 
{ 
    int			count; 
    byte*		source;
    byte*		dest;
    byte*		colormap;
    
    unsigned		frac;
    unsigned		fracstep;
    unsigned		fracstep2;
    unsigned		fracstep3;
    unsigned		fracstep4;	 
 
    count = dc_yh - dc_yl + 1; 

    source = dc_source;
    colormap = dc_colormap;		 
    dest = ylookup[dc_yl] + columnofs[dc_x];  
	 
    fracstep = dc_iscale<<9; 
    frac = (dc_texturemid + (dc_yl-centery)*dc_iscale)<<9; 
 
    fracstep2 = fracstep+fracstep;
    fracstep3 = fracstep2+fracstep;
    fracstep4 = fracstep3+fracstep;
	
    while (count >= 8) 
    { 
	dest[0] = colormap[source[frac>>25]]; 
	dest[SCREENWIDTH] = colormap[source[(frac+fracstep)>>25]]; 
	dest[SCREENWIDTH*2] = colormap[source[(frac+fracstep2)>>25]]; 
	dest[SCREENWIDTH*3] = colormap[source[(frac+fracstep3)>>25]];
	
	frac += fracstep4; 

	dest[SCREENWIDTH*4] = colormap[source[frac>>25]]; 
	dest[SCREENWIDTH*5] = colormap[source[(frac+fracstep)>>25]]; 
	dest[SCREENWIDTH*6] = colormap[source[(frac+fracstep2)>>25]]; 
	dest[SCREENWIDTH*7] = colormap[source[(frac+fracstep3)>>25]]; 

	frac += fracstep4; 
	dest += SCREENWIDTH*8; 
	count -= 8;
    } 
	
    while (count > 0)
    { 
	*dest = colormap[source[frac>>25]]; 
	dest += SCREENWIDTH; 
	frac += fracstep; 
	count--;
    } 
}
#endif

// haleyjd 09/06/10 [STRIFE] Removed low detail

//
// Spectre/Invisibility.
//

// haleyjd 09/06/10: ]STRIFE] replaced fuzzdraw with translucency.

//
// R_DrawMVisTLColumn
//
// villsa [STRIFE] new function
// Replacement for R_DrawFuzzColumn
//
void R_DrawMVisTLColumn(void)
{
    int                 count; 
    byte*               dest; 
    fixed_t             frac;
    fixed_t             fracstep;

    // Adjust borders. Low... 
    if (!dc_yl) 
        dc_yl = 1;

    // .. and high.
    if (dc_yh == viewheight-1) 
        dc_yh = viewheight - 2; 

    count = dc_yh - dc_yl; 

    // Zero length.
    if (count < 0) 
        return; 

#ifdef RANGECHECK 
    if ((unsigned)dc_x >= SCREENWIDTH
        || dc_yl < 0 || dc_yh >= SCREENHEIGHT)
    {
        I_Error ("R_DrawFuzzColumn: %i to %i at %i",
                 dc_yl, dc_yh, dc_x);
    }
#endif
    
    dest = ylookup[dc_yl] + columnofs[dc_x];

    // Looks familiar.
    fracstep = dc_iscale; 
    frac = dc_texturemid + (dc_yl-centery)*fracstep; 

    do
    {
        byte src = dc_colormap[dc_source[(frac>>FRACBITS)&127]];
        byte col = xlatab[*dest + (src << 8)];
        *dest = col;
        dest += SCREENWIDTH;
        frac += fracstep;
    } while(count--);
}

//
// R_DrawTLColumn
//
// villsa [STRIFE] new function
// Achieves a second translucency level using the same lookup table,
// via inversion of the colors in the index computation.
//
void R_DrawTLColumn(void)
{
    int                 count; 
    byte*               dest; 
    fixed_t             frac;
    fixed_t             fracstep;	 

    // Adjust borders. Low... 
    if (!dc_yl) 
        dc_yl = 1;

    // .. and high.
    if (dc_yh == viewheight-1) 
        dc_yh = viewheight - 2; 

    count = dc_yh - dc_yl; 

    // Zero length.
    if (count < 0) 
        return; 

#ifdef RANGECHECK 
    if ((unsigned)dc_x >= SCREENWIDTH
        || dc_yl < 0 || dc_yh >= SCREENHEIGHT)
    {
        I_Error ("R_DrawFuzzColumn2: %i to %i at %i",
                 dc_yl, dc_yh, dc_x);
    }
#endif
    
    dest = ylookup[dc_yl] + columnofs[dc_x];

    // Looks familiar.
    fracstep = dc_iscale; 
    frac = dc_texturemid + (dc_yl-centery)*fracstep; 

    do
    {
        byte src = dc_colormap[dc_source[(frac>>FRACBITS)&127]];
        byte col = xlatab[(*dest << 8) + src];
        *dest = col;
        dest += SCREENWIDTH;
        frac += fracstep;
    } while(count--);
}
  
 

//
// R_DrawTranslatedColumn
// Used to draw player sprites
//  with the green colorramp mapped to others.
// Could be used with different translation
//  tables, e.g. the lighter colored version
//  of the BaronOfHell, the HellKnight, uses
//  identical sprites, kinda brightened up.
//
byte*	dc_translation;
byte*	translationtables;

void R_DrawTranslatedColumn (void) 
{ 
    int                 count; 
    byte*               dest; 
    fixed_t             frac;
    fixed_t             fracstep;

    count = dc_yh - dc_yl; 
    if (count < 0) 
        return; 

#ifdef RANGECHECK 
    if ((unsigned)dc_x >= SCREENWIDTH
        || dc_yl < 0
        || dc_yh >= SCREENHEIGHT)
    {
        I_Error ( "R_DrawColumn: %i to %i at %i",
                 dc_yl, dc_yh, dc_x);
    }

#endif 

    dest = ylookup[dc_yl] + columnofs[dc_x]; 

    // Looks familiar.
    fracstep = dc_iscale; 
    frac = dc_texturemid + (dc_yl-centery)*fracstep; 

    // Here we do an additional index re-mapping.
    do 
    {
        // Translation tables are used
        //  to map certain colorramps to other ones,
        //  used with PLAY sprites.
        // Thus the "green" ramp of the player 0 sprite
        //  is mapped to gray, red, black/indigo. 
        *dest = dc_colormap[dc_translation[dc_source[frac>>FRACBITS]]];
        dest += SCREENWIDTH;
        frac += fracstep; 
    } while (count--); 
} 

// haleyjd 09/06/10 [STRIFE] Removed low detail

//
// R_DrawTRTLColumn
//
// villsa [STRIFE] new function
// Combines translucency and color translation.
//
void R_DrawTRTLColumn(void)
{
    int                 count; 
    byte*               dest; 
    fixed_t             frac;
    fixed_t             fracstep;

    count = dc_yh - dc_yl; 
    if (count < 0) 
        return; 

#ifdef RANGECHECK 
    if ((unsigned)dc_x >= SCREENWIDTH
        || dc_yl < 0
        || dc_yh >= SCREENHEIGHT)
    {
        I_Error ( "R_DrawColumn: %i to %i at %i",
                 dc_yl, dc_yh, dc_x);
    }
#endif 

    dest = ylookup[dc_yl] + columnofs[dc_x]; 

    // Looks familiar.
    fracstep = dc_iscale; 
    frac = dc_texturemid + (dc_yl-centery)*fracstep; 

    // Here we do an additional index re-mapping.
    do 
    {
        byte src = dc_colormap[dc_translation[dc_source[frac>>FRACBITS&127]]];
        byte col = xlatab[(*dest << 8) + src];
        *dest = col;
        dest += SCREENWIDTH;
        frac += fracstep; 
    } while (count--); 
}


//
// R_InitTranslationTables
// Creates the translation tables to map
//  the green color ramp to gray, brown, red.
// Assumes a given structure of the PLAYPAL.
// Could be read from a lump instead.
//
// haleyjd 08/26/10: [STRIFE]
// * Added loading of XLATAB
//
void R_InitTranslationTables (void)
{
    int i;
    byte col1, col2;

    // [STRIFE] Load xlatab. Here's how Rogue did it:
    //   v7 = W_CacheLumpName("XLATAB", PU_CACHE); // note potential cache bug...
    //   HIWORD(v8) = (Z_Malloc(131072, PU_STATIC, NULL) + 65535) >> 16;
    //   LOWORD(v8) = 0; // aligning to a 64K boundary, as if this is Wolf3D.
    //   xlatab = v8;
    //   memcpy(v8, v7, 65536);
    // As you can see, they copypasta'd id's unnecessary 64K boundary alignment
    // from the colormap code. Since this doesn't accomplish anything, and isn't
    // strictly portable, all we need to do is this:

    // villsa [STRIFE] 09/26/10: load table through this function instead
    V_LoadXlaTable();

    // villsa [STRIFE] allocate a larger size for translation tables
    translationtables = Z_Malloc (256*8, PU_STATIC, 0);

    col1 = 0xFA;
    col2 = 0xE0;

    // villsa [STRIFE] setup all translation tables
    for(i = 0; i < 256; i++)
    {
        if(i >= 0x80 && i <= 0x8f)
        {
            translationtables [i      ] = (i & 0x0f) + 64;
            translationtables [i+  256] = (i & 0x0f) - 80;
            translationtables [i+2*256] = (i & 0x0f) + 16;
            translationtables [i+3*256] = (i & 0x0f) + 48;
            translationtables [i+4*256] = (i & 0x0f) + 80;
            translationtables [i+5*256] = (i & 0x0f) + 96;
            translationtables [i+6*256] = (i & 0x0f) - 112;
        }
        else if(i >= 0x50 && i<= 0x5f)
        {
            translationtables [i      ] = i;
            translationtables [i+  256] = i;
            translationtables [i+2*256] = i;
            translationtables [i+3*256] = i;
            translationtables [i+4*256] = (i & 0x0f) + 0x80;
            translationtables [i+5*256] = (i & 0x0f) + 16;
            translationtables [i+6*256] = (i & 0x0f) + 64;
        }
        else if(i >= 0xd0 && i<= 0xdf)
        {
            translationtables [i      ] = i;
            translationtables [i+  256] = i;
            translationtables [i+2*256] = i;
            translationtables [i+3*256] = i;
            translationtables [i+4*256] = (i & 0x0f) - 80;
            translationtables [i+5*256] = (i & 0x0f) + 48;
            translationtables [i+6*256] = (i & 0x0f) + 16;
        }
        else if(i >= 0xc0 && i<= 0xcf)
        {
            translationtables [i      ] = i;
            translationtables [i+  256] = i;
            translationtables [i+2*256] = i;
            translationtables [i+3*256] = i;
            translationtables [i+4*256] = (i & 0x0f) - 96;
            translationtables [i+5*256] = (i & 0x0f) + 32;
            translationtables [i+6*256] = (i & 0x0f);
            // haleyjd 20110213: missing code:
            if(!(i & 0x0f))
                translationtables[i+6*256] = 1;
        }
        else if(i >= 0xf7 && i<= 0xfb)
        {
            translationtables [i      ] = col1;
            translationtables [i+  256] = i;
            translationtables [i+2*256] = i;
            translationtables [i+3*256] = i;
            translationtables [i+4*256] = i;
            translationtables [i+5*256] = i;
            translationtables [i+6*256] = i;
        }
        else if(i >= 0xf1 && i<= 0xf6)
        {
            translationtables [i      ] = (i & 0x0f) - 33;
            translationtables [i+  256] = i;
            translationtables [i+2*256] = i;
            translationtables [i+3*256] = i;
            translationtables [i+4*256] = i;
            translationtables [i+5*256] = i;
            translationtables [i+6*256] = i;
        }
        else if(i >= 0x20 && i <= 0x3f) // haleyjd 20110213: fixed upper end
        {
            translationtables [i      ] = col2;
            translationtables [i+  256] = col2;
            translationtables [i+2*256] = (i & 0x0f) - 48;
            translationtables [i+3*256] = (i & 0x0f) - 48;
            translationtables [i+4*256] = col2;
            translationtables [i+5*256] = col2;
            translationtables [i+6*256] = col2;
        }
        else  // Keep all other colors as is.
        {
            translationtables[i]=
            translationtables[i+256]=
            translationtables[i+2*256]=
            translationtables[i+3*256]=
            translationtables[i+4*256]=
            translationtables[i+5*256]=
            translationtables[i+6*256]=i;
        }

        ++col1;
        ++col2;
    }
}




//
// R_DrawSpan 
// With DOOM style restrictions on view orientation,
//  the floors and ceilings consist of horizontal slices
//  or spans with constant z depth.
// However, rotation around the world z axis is possible,
//  thus this mapping, while simpler and faster than
//  perspective correct texture mapping, has to traverse
//  the texture at an angle in all but a few cases.
// In consequence, flats are not stored by column (like walls),
//  and the inner loop has to step in texture space u and v.
//
int			ds_y; 
int			ds_x1; 
int			ds_x2;

lighttable_t*		ds_colormap; 

fixed_t			ds_xfrac; 
fixed_t			ds_yfrac; 
fixed_t			ds_xstep; 
fixed_t			ds_ystep;

// start of a 64*64 tile image 
byte*			ds_source;	

// just for profiling
int			dscount;


//
// Draws the actual span.
void R_DrawSpan (void) 
{ 
    unsigned int position, step;
    byte *dest;
    int count;
    int spot;
    unsigned int xtemp, ytemp;

#ifdef RANGECHECK
    if (ds_x2 < ds_x1
	|| ds_x1<0
	|| ds_x2>=SCREENWIDTH
	|| (unsigned)ds_y>SCREENHEIGHT)
    {
	I_Error( "R_DrawSpan: %i to %i at %i",
		 ds_x1,ds_x2,ds_y);
    }
//	dscount++;
#endif

    // Pack position and step variables into a single 32-bit integer,
    // with x in the top 16 bits and y in the bottom 16 bits.  For
    // each 16-bit part, the top 6 bits are the integer part and the
    // bottom 10 bits are the fractional part of the pixel position.

    position = ((ds_xfrac << 10) & 0xffff0000)
             | ((ds_yfrac >> 6)  & 0x0000ffff);
    step = ((ds_xstep << 10) & 0xffff0000)
         | ((ds_ystep >> 6)  & 0x0000ffff);

    dest = ylookup[ds_y] + columnofs[ds_x1];

    // We do not check for zero spans here?
    count = ds_x2 - ds_x1;

    do
    {
	// Calculate current texture index in u,v.
        ytemp = (position >> 4) & 0x0fc0;
        xtemp = (position >> 26);
        spot = xtemp | ytemp;

	// Lookup pixel from flat texture tile,
	//  re-index using light/colormap.
	*dest++ = ds_colormap[ds_source[spot]];

        position += step;

    } while (count--);
}



// UNUSED.
// Loop unrolled by 4.
#if 0
void R_DrawSpan (void) 
{ 
    unsigned	position, step;

    byte*	source;
    byte*	colormap;
    byte*	dest;
    
    unsigned	count;
    usingned	spot; 
    unsigned	value;
    unsigned	temp;
    unsigned	xtemp;
    unsigned	ytemp;
		
    position = ((ds_xfrac<<10)&0xffff0000) | ((ds_yfrac>>6)&0xffff);
    step = ((ds_xstep<<10)&0xffff0000) | ((ds_ystep>>6)&0xffff);
		
    source = ds_source;
    colormap = ds_colormap;
    dest = ylookup[ds_y] + columnofs[ds_x1];	 
    count = ds_x2 - ds_x1 + 1; 
	
    while (count >= 4) 
    { 
	ytemp = position>>4;
	ytemp = ytemp & 4032;
	xtemp = position>>26;
	spot = xtemp | ytemp;
	position += step;
	dest[0] = colormap[source[spot]]; 

	ytemp = position>>4;
	ytemp = ytemp & 4032;
	xtemp = position>>26;
	spot = xtemp | ytemp;
	position += step;
	dest[1] = colormap[source[spot]];
	
	ytemp = position>>4;
	ytemp = ytemp & 4032;
	xtemp = position>>26;
	spot = xtemp | ytemp;
	position += step;
	dest[2] = colormap[source[spot]];
	
	ytemp = position>>4;
	ytemp = ytemp & 4032;
	xtemp = position>>26;
	spot = xtemp | ytemp;
	position += step;
	dest[3] = colormap[source[spot]]; 
		
	count -= 4;
	dest += 4;
    } 
    while (count > 0) 
    { 
	ytemp = position>>4;
	ytemp = ytemp & 4032;
	xtemp = position>>26;
	spot = xtemp | ytemp;
	position += step;
	*dest++ = colormap[source[spot]]; 
	count--;
    } 
} 
#endif


//
// Again..
//
void R_DrawSpanLow (void)
{
    unsigned int position, step;
    unsigned int xtemp, ytemp;
    byte *dest;
    int count;
    int spot;

#ifdef RANGECHECK
    if (ds_x2 < ds_x1
	|| ds_x1<0
	|| ds_x2>=SCREENWIDTH
	|| (unsigned)ds_y>SCREENHEIGHT)
    {
	I_Error( "R_DrawSpan: %i to %i at %i",
		 ds_x1,ds_x2,ds_y);
    }
//	dscount++; 
#endif

    position = ((ds_xfrac << 10) & 0xffff0000)
             | ((ds_yfrac >> 6)  & 0x0000ffff);
    step = ((ds_xstep << 10) & 0xffff0000)
         | ((ds_ystep >> 6)  & 0x0000ffff);

    count = (ds_x2 - ds_x1);

    // Blocky mode, need to multiply by 2.
    ds_x1 <<= 1;
    ds_x2 <<= 1;

    dest = ylookup[ds_y] + columnofs[ds_x1];

    do
    {
	// Calculate current texture index in u,v.
        ytemp = (position >> 4) & 0x0fc0;
        xtemp = (position >> 26);
        spot = xtemp | ytemp;

	// Lowres/blocky mode does it twice,
	//  while scale is adjusted appropriately.
	*dest++ = ds_colormap[ds_source[spot]];
	*dest++ = ds_colormap[ds_source[spot]];

	position += step;

    } while (count--);
}

//
// R_InitBuffer 
// Creats lookup tables that avoid
//  multiplies and other hazzles
//  for getting the framebuffer address
//  of a pixel to draw.
//
void
R_InitBuffer
( int		width,
  int		height ) 
{ 
    int		i; 

    // Handle resize,
    //  e.g. smaller view windows
    //  with border and/or status bar.
    viewwindowx = (SCREENWIDTH-width) >> 1; 

    // Column offset. For windows.
    for (i=0 ; i<width ; i++) 
	columnofs[i] = viewwindowx + i;

    // Samw with base row offset.
    if (width == SCREENWIDTH) 
	viewwindowy = 0; 
    else 
	viewwindowy = (SCREENHEIGHT-SBARHEIGHT-height) >> 1; 

    // Preclaculate all row offsets.
    for (i=0 ; i<height ; i++) 
	ylookup[i] = I_VideoBuffer + (i+viewwindowy)*SCREENWIDTH; 
} 
 
 


//
// R_FillBackScreen
// Fills the back screen with a pattern
//  for variable screen sizes
// Also draws a beveled edge.
//
// haleyjd 08/29/10: [STRIFE] Added support for configurable back_flat.
//
void R_FillBackScreen (void) 
{ 
    byte*	src;
    byte*	dest; 
    int		x;
    int		y; 
    patch_t*	patch;

    char *name;

    // If we are running full screen, there is no need to do any of this,
    // and the background buffer can be freed if it was previously in use.

    if (scaledviewwidth == SCREENWIDTH)
    {
        if (background_buffer != NULL)
        {
            Z_Free(background_buffer);
            background_buffer = NULL;
        }

	return;
    }

    // Allocate the background buffer if necessary
	
    if (background_buffer == NULL)
    {
        background_buffer = Z_Malloc(SCREENWIDTH * (SCREENHEIGHT - SBARHEIGHT),
                                     PU_STATIC, NULL);
    }

    // haleyjd 08/29/10: [STRIFE] Use configurable back_flat
    name = back_flat;
    
    src = W_CacheLumpName(name, PU_CACHE); 
    dest = background_buffer;
	 
    for (y=0 ; y<SCREENHEIGHT-SBARHEIGHT ; y++) 
    { 
	for (x=0 ; x<SCREENWIDTH/64 ; x++) 
	{ 
	    memcpy (dest, src+((y&63)<<6), 64); 
	    dest += 64; 
	} 

	if (SCREENWIDTH&63) 
	{ 
	    memcpy (dest, src+((y&63)<<6), SCREENWIDTH&63); 
	    dest += (SCREENWIDTH&63); 
	} 
    } 
     
    // Draw screen and bezel; this is done to a separate screen buffer.

    V_UseBuffer(background_buffer);

    patch = W_CacheLumpName(DEH_String("brdr_t"),PU_CACHE);

    for (x=0 ; x<scaledviewwidth ; x+=8)
	V_DrawPatch(viewwindowx+x, viewwindowy-8, patch);
    patch = W_CacheLumpName(DEH_String("brdr_b"),PU_CACHE);

    for (x=0 ; x<scaledviewwidth ; x+=8)
	V_DrawPatch(viewwindowx+x, viewwindowy+viewheight, patch);
    patch = W_CacheLumpName(DEH_String("brdr_l"),PU_CACHE);

    for (y=0 ; y<viewheight ; y+=8)
	V_DrawPatch(viewwindowx-8, viewwindowy+y, patch);
    patch = W_CacheLumpName(DEH_String("brdr_r"),PU_CACHE);

    for (y=0 ; y<viewheight ; y+=8)
	V_DrawPatch(viewwindowx+scaledviewwidth, viewwindowy+y, patch);

    // Draw beveled edge. 
    V_DrawPatch(viewwindowx-8,
                viewwindowy-8,
                W_CacheLumpName(DEH_String("brdr_tl"),PU_CACHE));
    
    V_DrawPatch(viewwindowx+scaledviewwidth,
                viewwindowy-8,
                W_CacheLumpName(DEH_String("brdr_tr"),PU_CACHE));
    
    V_DrawPatch(viewwindowx-8,
                viewwindowy+viewheight,
                W_CacheLumpName(DEH_String("brdr_bl"),PU_CACHE));
    
    V_DrawPatch(viewwindowx+scaledviewwidth,
                viewwindowy+viewheight,
                W_CacheLumpName(DEH_String("brdr_br"),PU_CACHE));

    V_RestoreBuffer();
} 
 

//
// Copy a screen buffer.
//
void
R_VideoErase
( unsigned	ofs,
  int		count ) 
{ 
  // LFB copy.
  // This might not be a good idea if memcpy
  //  is not optiomal, e.g. byte by byte on
  //  a 32bit CPU, as GNU GCC/Linux libc did
  //  at one point.

    if (background_buffer != NULL)
    {
        memcpy(I_VideoBuffer + ofs, background_buffer + ofs, count); 
    }
} 


//
// R_DrawViewBorder
// Draws the border around the view
//  for different size windows?
//
void R_DrawViewBorder (void) 
{ 
    int		top;
    int		side;
    int		ofs;
    int		i; 
 
    if (scaledviewwidth == SCREENWIDTH) 
	return; 
  
    top = ((SCREENHEIGHT-SBARHEIGHT)-viewheight)/2; 
    side = (SCREENWIDTH-scaledviewwidth)/2; 
 
    // copy top and one line of left side 
    R_VideoErase (0, top*SCREENWIDTH+side); 
 
    // copy one line of right side and bottom 
    ofs = (viewheight+top)*SCREENWIDTH-side; 
    R_VideoErase (ofs, top*SCREENWIDTH+side); 
 
    // copy sides using wraparound 
    ofs = top*SCREENWIDTH + SCREENWIDTH-side; 
    side <<= 1;
    
    for (i=1 ; i<viewheight ; i++) 
    { 
	R_VideoErase (ofs, side); 
	ofs += SCREENWIDTH; 
    } 

    // ? 
    V_MarkRect (0,0,SCREENWIDTH, SCREENHEIGHT-SBARHEIGHT); 
} 
 
 
