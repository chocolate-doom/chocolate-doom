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
// DESCRIPTION:
//	Gamma correction LUT stuff.
//	Functions to draw patches (by post) directly to screen.
//	Functions to blit a block to the screen.
//

#include <stdio.h>
#include <string.h>
#include <math.h>

#include "i_system.h"

#include "doomtype.h"

#include "deh_str.h"
#include "i_input.h"
#include "i_swap.h"
#include "i_video.h"
#include "m_bbox.h"
#include "m_misc.h"
#include "v_video.h"
#include "w_wad.h"
#include "z_zone.h"

#include "config.h"
#ifdef HAVE_LIBPNG
#include <png.h>
#endif

// TODO: There are separate RANGECHECK defines for different games, but this
// is common code. Fix this.
#define RANGECHECK

// Blending table used for fuzzpatch, etc.
// Only used in Heretic/Hexen

byte *tinttable = NULL;

// villsa [STRIFE] Blending table used for Strife
byte *xlatab = NULL;

// The screen buffer that the v_video.c code draws to.

static pixel_t *dest_screen = NULL;

int dirtybox[4]; 

// haleyjd 08/28/10: clipping callback function for patches.
// This is needed for Chocolate Strife, which clips patches to the screen.
static vpatchclipfunc_t patchclip_callback = NULL;

//
// V_MarkRect 
// 
void V_MarkRect(int x, int y, int width, int height) 
{ 
    // If we are temporarily using an alternate screen, do not 
    // affect the update box.

    if (dest_screen == I_VideoBuffer)
    {
        M_AddToBox (dirtybox, x, y); 
        M_AddToBox (dirtybox, x + width-1, y + height-1); 
    }
} 
 

//
// V_CopyRect 
// 
void V_CopyRect(int srcx, int srcy, pixel_t *source,
                int width, int height,
                int destx, int desty)
{ 
    pixel_t *src;
    pixel_t *dest;
 
#ifdef RANGECHECK 
    if (srcx < 0
     || srcx + width > SCREENWIDTH
     || srcy < 0
     || srcy + height > SCREENHEIGHT 
     || destx < 0
     || destx + width > SCREENWIDTH
     || desty < 0
     || desty + height > SCREENHEIGHT)
    {
        I_Error ("Bad V_CopyRect");
    }
#endif 

    V_MarkRect(destx, desty, width, height); 
 
    src = source + SCREENWIDTH * srcy + srcx; 
    dest = dest_screen + SCREENWIDTH * desty + destx; 

    for ( ; height>0 ; height--) 
    { 
        memcpy(dest, src, width * sizeof(*dest));
        src += SCREENWIDTH; 
        dest += SCREENWIDTH; 
    } 
} 
 
//
// V_SetPatchClipCallback
//
// haleyjd 08/28/10: Added for Strife support.
// By calling this function, you can setup runtime error checking for patch 
// clipping. Strife never caused errors by drawing patches partway off-screen.
// Some versions of vanilla DOOM also behaved differently than the default
// implementation, so this could possibly be extended to those as well for
// accurate emulation.
//
void V_SetPatchClipCallback(vpatchclipfunc_t func)
{
    patchclip_callback = func;
}

//
// V_DrawPatch
// Masks a column based masked pic to the screen. 
//

void V_DrawPatch(int x, int y, patch_t *patch)
{ 
    int count;
    int col;
    column_t *column;
    pixel_t *desttop;
    pixel_t *dest;
    byte *source;
    int w;

    y -= SHORT(patch->topoffset);
    x -= SHORT(patch->leftoffset);

    // haleyjd 08/28/10: Strife needs silent error checking here.
    if(patchclip_callback)
    {
        if(!patchclip_callback(patch, x, y))
            return;
    }

#ifdef RANGECHECK
    if (x < 0
     || x + SHORT(patch->width) > SCREENWIDTH
     || y < 0
     || y + SHORT(patch->height) > SCREENHEIGHT)
    {
        I_Error("Bad V_DrawPatch");
    }
#endif

    V_MarkRect(x, y, SHORT(patch->width), SHORT(patch->height));

    col = 0;
    desttop = dest_screen + y * SCREENWIDTH + x;

    w = SHORT(patch->width);

    for ( ; col<w ; x++, col++, desttop++)
    {
        column = (column_t *)((byte *)patch + LONG(patch->columnofs[col]));

        // step through the posts in a column
        while (column->topdelta != 0xff)
        {
            source = (byte *)column + 3;
            dest = desttop + column->topdelta*SCREENWIDTH;
            count = column->length;

            while (count--)
            {
                *dest = *source++;
                dest += SCREENWIDTH;
            }
            column = (column_t *)((byte *)column + column->length + 4);
        }
    }
}

//
// V_DrawPatchFlipped
// Masks a column based masked pic to the screen.
// Flips horizontally, e.g. to mirror face.
//

void V_DrawPatchFlipped(int x, int y, patch_t *patch)
{
    int count;
    int col; 
    column_t *column; 
    pixel_t *desttop;
    pixel_t *dest;
    byte *source; 
    int w; 
 
    y -= SHORT(patch->topoffset); 
    x -= SHORT(patch->leftoffset); 

    // haleyjd 08/28/10: Strife needs silent error checking here.
    if(patchclip_callback)
    {
        if(!patchclip_callback(patch, x, y))
            return;
    }

#ifdef RANGECHECK 
    if (x < 0
     || x + SHORT(patch->width) > SCREENWIDTH
     || y < 0
     || y + SHORT(patch->height) > SCREENHEIGHT)
    {
        I_Error("Bad V_DrawPatchFlipped");
    }
#endif

    V_MarkRect (x, y, SHORT(patch->width), SHORT(patch->height));

    col = 0;
    desttop = dest_screen + y * SCREENWIDTH + x;

    w = SHORT(patch->width);

    for ( ; col<w ; x++, col++, desttop++)
    {
        column = (column_t *)((byte *)patch + LONG(patch->columnofs[w-1-col]));

        // step through the posts in a column
        while (column->topdelta != 0xff )
        {
            source = (byte *)column + 3;
            dest = desttop + column->topdelta*SCREENWIDTH;
            count = column->length;

            while (count--)
            {
                *dest = *source++;
                dest += SCREENWIDTH;
            }
            column = (column_t *)((byte *)column + column->length + 4);
        }
    }
}



//
// V_DrawPatchDirect
// Draws directly to the screen on the pc. 
//

void V_DrawPatchDirect(int x, int y, patch_t *patch)
{
    V_DrawPatch(x, y, patch); 
} 

//
// V_DrawTLPatch
//
// Masks a column based translucent masked pic to the screen.
//

void V_DrawTLPatch(int x, int y, patch_t * patch)
{
    int count, col;
    column_t *column;
    pixel_t *desttop, *dest;
    byte *source;
    int w;

    y -= SHORT(patch->topoffset);
    x -= SHORT(patch->leftoffset);

    if (x < 0
     || x + SHORT(patch->width) > SCREENWIDTH 
     || y < 0
     || y + SHORT(patch->height) > SCREENHEIGHT)
    {
        I_Error("Bad V_DrawTLPatch");
    }

    col = 0;
    desttop = dest_screen + y * SCREENWIDTH + x;

    w = SHORT(patch->width);
    for (; col < w; x++, col++, desttop++)
    {
        column = (column_t *) ((byte *) patch + LONG(patch->columnofs[col]));

        // step through the posts in a column

        while (column->topdelta != 0xff)
        {
            source = (byte *) column + 3;
            dest = desttop + column->topdelta * SCREENWIDTH;
            count = column->length;

            while (count--)
            {
                *dest = tinttable[((*dest) << 8) + *source++];
                dest += SCREENWIDTH;
            }
            column = (column_t *) ((byte *) column + column->length + 4);
        }
    }
}

//
// V_DrawXlaPatch
//
// villsa [STRIFE] Masks a column based translucent masked pic to the screen.
//

void V_DrawXlaPatch(int x, int y, patch_t * patch)
{
    int count, col;
    column_t *column;
    pixel_t *desttop, *dest;
    byte *source;
    int w;

    y -= SHORT(patch->topoffset);
    x -= SHORT(patch->leftoffset);

    if(patchclip_callback)
    {
        if(!patchclip_callback(patch, x, y))
            return;
    }

    col = 0;
    desttop = dest_screen + y * SCREENWIDTH + x;

    w = SHORT(patch->width);
    for(; col < w; x++, col++, desttop++)
    {
        column = (column_t *) ((byte *) patch + LONG(patch->columnofs[col]));

        // step through the posts in a column

        while(column->topdelta != 0xff)
        {
            source = (byte *) column + 3;
            dest = desttop + column->topdelta * SCREENWIDTH;
            count = column->length;

            while(count--)
            {
                *dest = xlatab[*dest + ((*source) << 8)];
                source++;
                dest += SCREENWIDTH;
            }
            column = (column_t *) ((byte *) column + column->length + 4);
        }
    }
}

//
// V_DrawAltTLPatch
//
// Masks a column based translucent masked pic to the screen.
//

void V_DrawAltTLPatch(int x, int y, patch_t * patch)
{
    int count, col;
    column_t *column;
    pixel_t *desttop, *dest;
    byte *source;
    int w;

    y -= SHORT(patch->topoffset);
    x -= SHORT(patch->leftoffset);

    if (x < 0
     || x + SHORT(patch->width) > SCREENWIDTH
     || y < 0
     || y + SHORT(patch->height) > SCREENHEIGHT)
    {
        I_Error("Bad V_DrawAltTLPatch");
    }

    col = 0;
    desttop = dest_screen + y * SCREENWIDTH + x;

    w = SHORT(patch->width);
    for (; col < w; x++, col++, desttop++)
    {
        column = (column_t *) ((byte *) patch + LONG(patch->columnofs[col]));

        // step through the posts in a column

        while (column->topdelta != 0xff)
        {
            source = (byte *) column + 3;
            dest = desttop + column->topdelta * SCREENWIDTH;
            count = column->length;

            while (count--)
            {
                *dest = tinttable[((*dest) << 8) + *source++];
                dest += SCREENWIDTH;
            }
            column = (column_t *) ((byte *) column + column->length + 4);
        }
    }
}

//
// V_DrawShadowedPatch
//
// Masks a column based masked pic to the screen.
//

void V_DrawShadowedPatch(int x, int y, patch_t *patch)
{
    int count, col;
    column_t *column;
    pixel_t *desttop, *dest;
    byte *source;
    pixel_t *desttop2, *dest2;
    int w;

    y -= SHORT(patch->topoffset);
    x -= SHORT(patch->leftoffset);

    if (x < 0
     || x + SHORT(patch->width) > SCREENWIDTH
     || y < 0
     || y + SHORT(patch->height) > SCREENHEIGHT)
    {
        I_Error("Bad V_DrawShadowedPatch");
    }

    col = 0;
    desttop = dest_screen + y * SCREENWIDTH + x;
    desttop2 = dest_screen + (y + 2) * SCREENWIDTH + x + 2;

    w = SHORT(patch->width);
    for (; col < w; x++, col++, desttop++, desttop2++)
    {
        column = (column_t *) ((byte *) patch + LONG(patch->columnofs[col]));

        // step through the posts in a column

        while (column->topdelta != 0xff)
        {
            source = (byte *) column + 3;
            dest = desttop + column->topdelta * SCREENWIDTH;
            dest2 = desttop2 + column->topdelta * SCREENWIDTH;
            count = column->length;

            while (count--)
            {
                *dest2 = tinttable[((*dest2) << 8)];
                dest2 += SCREENWIDTH;
                *dest = *source++;
                dest += SCREENWIDTH;

            }
            column = (column_t *) ((byte *) column + column->length + 4);
        }
    }
}

//
// Load tint table from TINTTAB lump.
//

void V_LoadTintTable(void)
{
    tinttable = W_CacheLumpName("TINTTAB", PU_STATIC);
}

//
// V_LoadXlaTable
//
// villsa [STRIFE] Load xla table from XLATAB lump.
//

void V_LoadXlaTable(void)
{
    xlatab = W_CacheLumpName("XLATAB", PU_STATIC);
}

//
// V_DrawBlock
// Draw a linear block of pixels into the view buffer.
//

void V_DrawBlock(int x, int y, int width, int height, pixel_t *src)
{ 
    pixel_t *dest;
 
#ifdef RANGECHECK 
    if (x < 0
     || x + width >SCREENWIDTH
     || y < 0
     || y + height > SCREENHEIGHT)
    {
	I_Error ("Bad V_DrawBlock");
    }
#endif 
 
    V_MarkRect (x, y, width, height); 
 
    dest = dest_screen + y * SCREENWIDTH + x; 

    while (height--) 
    { 
	memcpy (dest, src, width * sizeof(*dest));
	src += width; 
	dest += SCREENWIDTH; 
    } 
} 

void V_DrawFilledBox(int x, int y, int w, int h, int c)
{
    uint8_t *buf, *buf1;
    int x1, y1;

    buf = I_VideoBuffer + SCREENWIDTH * y + x;

    for (y1 = 0; y1 < h; ++y1)
    {
        buf1 = buf;

        for (x1 = 0; x1 < w; ++x1)
        {
            *buf1++ = c;
        }

        buf += SCREENWIDTH;
    }
}

void V_DrawHorizLine(int x, int y, int w, int c)
{
    uint8_t *buf;
    int x1;

    buf = I_VideoBuffer + SCREENWIDTH * y + x;

    for (x1 = 0; x1 < w; ++x1)
    {
        *buf++ = c;
    }
}

void V_DrawVertLine(int x, int y, int h, int c)
{
    uint8_t *buf;
    int y1;

    buf = I_VideoBuffer + SCREENWIDTH * y + x;

    for (y1 = 0; y1 < h; ++y1)
    {
        *buf = c;
        buf += SCREENWIDTH;
    }
}

void V_DrawBox(int x, int y, int w, int h, int c)
{
    V_DrawHorizLine(x, y, w, c);
    V_DrawHorizLine(x, y+h-1, w, c);
    V_DrawVertLine(x, y, h, c);
    V_DrawVertLine(x+w-1, y, h, c);
}

//
// Draw a "raw" screen (lump containing raw data to blit directly
// to the screen)
//
 
void V_DrawRawScreen(byte *raw)
{
    memcpy(dest_screen, raw, SCREENWIDTH * SCREENHEIGHT);
}

//
// V_Init
// 
void V_Init (void) 
{ 
    // no-op!
    // There used to be separate screens that could be drawn to; these are
    // now handled in the upper layers.
}

// Set the buffer that the code draws to.

void V_UseBuffer(pixel_t *buffer)
{
    dest_screen = buffer;
}

// Restore screen buffer to the i_video screen buffer.

void V_RestoreBuffer(void)
{
    dest_screen = I_VideoBuffer;
}

//
// SCREEN SHOTS
//

typedef PACKED_STRUCT (
{
    char		manufacturer;
    char		version;
    char		encoding;
    char		bits_per_pixel;

    unsigned short	xmin;
    unsigned short	ymin;
    unsigned short	xmax;
    unsigned short	ymax;
    
    unsigned short	hres;
    unsigned short	vres;

    unsigned char	palette[48];
    
    char		reserved;
    char		color_planes;
    unsigned short	bytes_per_line;
    unsigned short	palette_type;
    
    char		filler[58];
    unsigned char	data;		// unbounded
}) pcx_t;


//
// WritePCXfile
//

void WritePCXfile(char *filename, byte *data,
                  int width, int height,
                  byte *palette)
{
    int		i;
    int		length;
    pcx_t*	pcx;
    byte*	pack;
	
    pcx = Z_Malloc (width*height*2+1000, PU_STATIC, NULL);

    pcx->manufacturer = 0x0a;		// PCX id
    pcx->version = 5;			// 256 color
    pcx->encoding = 1;			// uncompressed
    pcx->bits_per_pixel = 8;		// 256 color
    pcx->xmin = 0;
    pcx->ymin = 0;
    pcx->xmax = SHORT(width-1);
    pcx->ymax = SHORT(height-1);
    pcx->hres = SHORT(1);
    pcx->vres = SHORT(1);
    memset (pcx->palette,0,sizeof(pcx->palette));
    pcx->reserved = 0;                  // PCX spec: reserved byte must be zero
    pcx->color_planes = 1;		// chunky image
    pcx->bytes_per_line = SHORT(width);
    pcx->palette_type = SHORT(2);	// not a grey scale
    memset (pcx->filler,0,sizeof(pcx->filler));

    // pack the image
    pack = &pcx->data;
	
    for (i=0 ; i<width*height ; i++)
    {
	if ( (*data & 0xc0) != 0xc0)
	    *pack++ = *data++;
	else
	{
	    *pack++ = 0xc1;
	    *pack++ = *data++;
	}
    }
    
    // write the palette
    *pack++ = 0x0c;	// palette ID byte
    for (i=0 ; i<768 ; i++)
	*pack++ = *palette++;
    
    // write output file
    length = pack - (byte *)pcx;
    M_WriteFile (filename, pcx, length);

    Z_Free (pcx);
}

#ifdef HAVE_LIBPNG
//
// WritePNGfile
//

static void error_fn(png_structp p, png_const_charp s)
{
    printf("libpng error: %s\n", s);
}

static void warning_fn(png_structp p, png_const_charp s)
{
    printf("libpng warning: %s\n", s);
}

void WritePNGfile(char *filename, byte *data,
                  int width, int height,
                  byte *palette)
{
    png_structp ppng;
    png_infop pinfo;
    png_colorp pcolor;
    FILE *handle;
    int i, j;
    int w_factor, h_factor;
    byte *rowbuf;

    if (aspect_ratio_correct)
    {
        // scale up to accommodate aspect ratio correction
        w_factor = 5;
        h_factor = 6;

        width *= w_factor;
        height *= h_factor;
    }
    else
    {
        w_factor = 1;
        h_factor = 1;
    }

    handle = fopen(filename, "wb");
    if (!handle)
    {
        return;
    }

    ppng = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL,
                                   error_fn, warning_fn);
    if (!ppng)
    {
        fclose(handle);
        return;
    }

    pinfo = png_create_info_struct(ppng);
    if (!pinfo)
    {
        fclose(handle);
        png_destroy_write_struct(&ppng, NULL);
        return;
    }

    png_init_io(ppng, handle);

    png_set_IHDR(ppng, pinfo, width, height,
                 8, PNG_COLOR_TYPE_PALETTE, PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

    pcolor = malloc(sizeof(*pcolor) * 256);
    if (!pcolor)
    {
        fclose(handle);
        png_destroy_write_struct(&ppng, &pinfo);
        return;
    }

    for (i = 0; i < 256; i++)
    {
        pcolor[i].red   = *(palette + 3 * i);
        pcolor[i].green = *(palette + 3 * i + 1);
        pcolor[i].blue  = *(palette + 3 * i + 2);
    }

    png_set_PLTE(ppng, pinfo, pcolor, 256);
    free(pcolor);

    png_write_info(ppng, pinfo);

    rowbuf = malloc(width);

    if (rowbuf)
    {
        for (i = 0; i < SCREENHEIGHT; i++)
        {
            // expand the row 5x
            for (j = 0; j < SCREENWIDTH; j++)
            {
                memset(rowbuf + j * w_factor, *(data + i*SCREENWIDTH + j), w_factor);
            }

            // write the row 6 times
            for (j = 0; j < h_factor; j++)
            {
                png_write_row(ppng, rowbuf);
            }
        }

        free(rowbuf);
    }

    png_write_end(ppng, pinfo);
    png_destroy_write_struct(&ppng, &pinfo);
    fclose(handle);
}
#endif

//
// V_ScreenShot
//

void V_ScreenShot(char *format)
{
    int i;
    char lbmname[16]; // haleyjd 20110213: BUG FIX - 12 is too small!
    char *ext;
    
    // find a file name to save it to

#ifdef HAVE_LIBPNG
    extern int png_screenshots;
    if (png_screenshots)
    {
        ext = "png";
    }
    else
#endif
    {
        ext = "pcx";
    }

    for (i=0; i<=99; i++)
    {
        M_snprintf(lbmname, sizeof(lbmname), format, i, ext);

        if (!M_FileExists(lbmname))
        {
            break;      // file doesn't exist
        }
    }

    if (i == 100)
    {
#ifdef HAVE_LIBPNG
        if (png_screenshots)
        {
            I_Error ("V_ScreenShot: Couldn't create a PNG");
        }
        else
#endif
        {
            I_Error ("V_ScreenShot: Couldn't create a PCX");
        }
    }

#ifdef HAVE_LIBPNG
    if (png_screenshots)
    {
    WritePNGfile(lbmname, I_VideoBuffer,
                 SCREENWIDTH, SCREENHEIGHT,
                 W_CacheLumpName (DEH_String("PLAYPAL"), PU_CACHE));
    }
    else
#endif
    {
    // save the pcx file
    WritePCXfile(lbmname, I_VideoBuffer,
                 SCREENWIDTH, SCREENHEIGHT,
                 W_CacheLumpName (DEH_String("PLAYPAL"), PU_CACHE));
    }
}

#define MOUSE_SPEED_BOX_WIDTH  120
#define MOUSE_SPEED_BOX_HEIGHT 9

//
// V_DrawMouseSpeedBox
//

// If box is only to calibrate speed, testing relative speed (as a measure
// of game pixels to movement units) is important whether physical mouse DPI
// is high or low. Line resolution starts at 1 pixel per 1 move-unit: if
// line maxes out, resolution becomes 1 pixel per 2 move-units, then per
// 3 move-units, etc.

static int linelen_multiplier = 1;

void V_DrawMouseSpeedBox(int speed)
{
    extern int usemouse;
    int bgcolor, bordercolor, red, black, white, yellow;
    int box_x, box_y;
    int original_speed;
    int redline_x;
    int linelen;
    int i;
    boolean draw_acceleration = false;

    // Get palette indices for colors for widget. These depend on the
    // palette of the game being played.

    bgcolor = I_GetPaletteIndex(0x77, 0x77, 0x77);
    bordercolor = I_GetPaletteIndex(0x55, 0x55, 0x55);
    red = I_GetPaletteIndex(0xff, 0x00, 0x00);
    black = I_GetPaletteIndex(0x00, 0x00, 0x00);
    yellow = I_GetPaletteIndex(0xff, 0xff, 0x00);
    white = I_GetPaletteIndex(0xff, 0xff, 0xff);

    // If the mouse is turned off, don't draw the box at all.
    if (!usemouse)
    {
        return;
    }

    // If acceleration is used, draw a box that helps to calibrate the
    // threshold point.
    if (mouse_threshold > 0 && fabs(mouse_acceleration - 1) > 0.01)
    {
        draw_acceleration = true;
    }

    // Calculate box position

    box_x = SCREENWIDTH - MOUSE_SPEED_BOX_WIDTH - 10;
    box_y = 15;

    V_DrawFilledBox(box_x, box_y,
                    MOUSE_SPEED_BOX_WIDTH, MOUSE_SPEED_BOX_HEIGHT, bgcolor);
    V_DrawBox(box_x, box_y,
              MOUSE_SPEED_BOX_WIDTH, MOUSE_SPEED_BOX_HEIGHT, bordercolor);

    // Calculate the position of the red threshold line when calibrating
    // acceleration.  This is 1/3 of the way along the box.

    redline_x = MOUSE_SPEED_BOX_WIDTH / 3;

    // Calculate line length

    if (draw_acceleration && speed >= mouse_threshold)
    {
        // Undo acceleration and get back the original mouse speed
        original_speed = speed - mouse_threshold;
        original_speed = (int) (original_speed / mouse_acceleration);
        original_speed += mouse_threshold;

        linelen = (original_speed * redline_x) / mouse_threshold;
    }
    else
    {
        linelen = speed / linelen_multiplier;
    }

    // Draw horizontal "thermometer" 

    if (linelen > MOUSE_SPEED_BOX_WIDTH - 1)
    {
        linelen = MOUSE_SPEED_BOX_WIDTH - 1;
        if (!draw_acceleration)
        {
            linelen_multiplier++;
        }
    }

    V_DrawHorizLine(box_x + 1, box_y + 4, MOUSE_SPEED_BOX_WIDTH - 2, black);

    if (!draw_acceleration || linelen < redline_x)
    {
        V_DrawHorizLine(box_x + 1, box_y + MOUSE_SPEED_BOX_HEIGHT / 2,
                        linelen, white);
    }
    else
    {
        V_DrawHorizLine(box_x + 1, box_y + MOUSE_SPEED_BOX_HEIGHT / 2,
                        redline_x, white);
        V_DrawHorizLine(box_x + redline_x, box_y + MOUSE_SPEED_BOX_HEIGHT / 2,
                        linelen - redline_x, yellow);
    }

    if (draw_acceleration)
    {
        // Draw acceleration threshold line
        V_DrawVertLine(box_x + redline_x, box_y + 1,
                       MOUSE_SPEED_BOX_HEIGHT - 2, red);
    }
    else
    {
        // Draw multiplier lines to indicate current resolution
        for (i = 1; i < linelen_multiplier; i++)
        {
            V_DrawVertLine(
                box_x + (i * MOUSE_SPEED_BOX_WIDTH / linelen_multiplier),
                box_y + 1, MOUSE_SPEED_BOX_HEIGHT - 2, yellow);
        }
    }
}

