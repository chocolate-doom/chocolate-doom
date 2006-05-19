// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id$
//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2005 Simon Howard
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
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
// 02111-1307, USA.
//
// $Log$
// Revision 1.1  2006/01/13 23:56:00  fraggle
// Add text-mode I/O functions.
// Use text-mode screen for the waiting screen.
//
//

#include <string.h>

#include "txt_io.h"
#include "txt_main.h"

// Array of border characters for drawing windows. The array looks like this:
//
// +-++
// | ||
// +-++
// +-++

static int borders[4][4] = 
{
    {0xda, 0xc4, 0xc2, 0xbf},
    {0xb3, ' ',  0xb3, 0xb3},
    {0xc3, 0xc4, 0xc5, 0xb4},
    {0xc0, 0xc4, 0xc1, 0xd9},
};

void TXT_DrawDesktopBackground(char *title)
{
    int i;
    unsigned char *screendata;
    unsigned char *p;

    screendata = TXT_GetScreenData();
    
    // Fill the screen with gradient characters

    p = screendata;
    
    for (i=0; i<TXT_SCREEN_W * TXT_SCREEN_H; ++i)
    {
        *p++ = 0xb1;
        *p++ = TXT_COLOR_GREY | (TXT_COLOR_BLUE << 4);
    }

    // Draw the top and bottom banners

    p = screendata;

    for (i=0; i<TXT_SCREEN_W; ++i)
    {
        *p++ = ' ';
        *p++ = TXT_COLOR_BLACK | (TXT_COLOR_GREY << 4);
    }

    p = screendata + (TXT_SCREEN_H - 1) * TXT_SCREEN_W * 2;

    for (i=0; i<TXT_SCREEN_W; ++i)
    {
        *p++ = ' ';
        *p++ = TXT_COLOR_BLACK | (TXT_COLOR_GREY << 4);
    }

    // Print the title

    TXT_GotoXY(0, 0);
    TXT_FGColor(TXT_COLOR_BLACK);
    TXT_BGColor(TXT_COLOR_GREY, 0);

    TXT_PutChar(' ');
    TXT_Puts(title);
}

void TXT_DrawShadow(int x, int y, int w, int h)
{
    unsigned char *screendata;
    unsigned char *p;
    int x1, y1;

    screendata = TXT_GetScreenData();

    for (y1=y; y1<y+h; ++y1)
    {
        p = screendata + y1 * TXT_SCREEN_W * 2 + x * 2;

        for (x1=0; x1<w; ++x1)
        {
            p[1] = TXT_COLOR_DARK_GREY;
            p += 2;
        }
    }
}

void TXT_DrawWindowFrame(char *title, int x, int y, int w, int h)
{
    int x1, y1;
    int bx, by;

    TXT_FGColor(TXT_COLOR_BRIGHT_CYAN);
    TXT_BGColor(TXT_COLOR_BLUE, 0);

    for (y1=y; y1<y+h; ++y1)
    {
        TXT_GotoXY(x, y1);

        // Select the appropriate row and column in the borders
        // array to pick the appropriate character to draw at
        // this location.
        //
        // Draw a horizontal line on the third line down, so we
        // draw a box around the title.

        by = y1 == y ? 0 :
             y1 == y + 2 ? 2 :
             y1 == y + h - 1 ? 3 : 1;

        for (x1=x; x1<x+w; ++x1)
        {
            bx = x1 == x ? 0 :
                 x1 == x + w - 1 ? 3 : 1;
                 
            TXT_PutChar(borders[by][bx]);
        }
    }

    // Draw the title

    TXT_GotoXY(x + 1, y + 1);
    TXT_BGColor(TXT_COLOR_GREY, 0);
    TXT_FGColor(TXT_COLOR_BLUE);

    for (x1=0; x1<w-2; ++x1)
    {
        TXT_PutChar(' ');
    }

    TXT_GotoXY(x + (w - strlen(title)) / 2, y + 1);
    TXT_Puts(title);

    // Draw the window's shadow.

    TXT_DrawShadow(x + 2, y + h, w, 1);
    TXT_DrawShadow(x + w, y + 1, 2, h);
}

void TXT_DrawSeparator(int x, int y, int w)
{
    int x1;
    int c;

    TXT_FGColor(TXT_COLOR_BRIGHT_CYAN);
    TXT_BGColor(TXT_COLOR_BLUE, 0);

    for (x1=x; x1<x+w; ++x1)
    {
        TXT_GotoXY(x1, y);

        c = x1 == x ? borders[2][0] :
            x1 == x + w - 1 ? borders[2][3] :
            borders[2][1];

        TXT_PutChar(c);
    }
}

