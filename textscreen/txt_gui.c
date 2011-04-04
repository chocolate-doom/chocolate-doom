// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// Copyright(C) 2005,2006 Simon Howard
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

#include <stdlib.h>
#include <string.h>

#include "txt_gui.h"
#include "txt_io.h"
#include "txt_main.h"

typedef struct txt_cliparea_s txt_cliparea_t;

struct txt_cliparea_s
{
    int x1, x2;
    int y1, y2;
    txt_cliparea_t *next;
};

// Array of border characters for drawing windows. The array looks like this:
//
// +-++
// | ||
// +-++
// +-++

static const int borders[4][4] = 
{
    {0xda, 0xc4, 0xc2, 0xbf},
    {0xb3, ' ',  0xb3, 0xb3},
    {0xc3, 0xc4, 0xc5, 0xb4},
    {0xc0, 0xc4, 0xc1, 0xd9},
};

static txt_cliparea_t *cliparea = NULL;

#define VALID_X(x) ((x) >= cliparea->x1 && (x) < cliparea->x2)
#define VALID_Y(y) ((y) >= cliparea->y1 && (y) < cliparea->y2)

void TXT_DrawDesktopBackground(const char *title)
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

    TXT_DrawString(" ");
    TXT_DrawString(title);
}

void TXT_DrawShadow(int x, int y, int w, int h)
{
    unsigned char *screendata;
    unsigned char *p;
    int x1, y1;

    screendata = TXT_GetScreenData();

    for (y1=y; y1<y+h; ++y1)
    {
        p = screendata + (y1 * TXT_SCREEN_W + x) * 2;

        for (x1=x; x1<x+w; ++x1)
        {
            if (VALID_X(x1) && VALID_Y(y1))
            {
                p[1] = TXT_COLOR_DARK_GREY;
            }

            p += 2;
        }
    }
}

void TXT_DrawWindowFrame(const char *title, int x, int y, int w, int h)
{
    int x1, y1;
    int bx, by;

    TXT_FGColor(TXT_COLOR_BRIGHT_CYAN);
    TXT_BGColor(TXT_WINDOW_BACKGROUND, 0);

    for (y1=y; y1<y+h; ++y1)
    {
        // Select the appropriate row and column in the borders
        // array to pick the appropriate character to draw at
        // this location.
        //
        // Draw a horizontal line on the third line down, so we
        // draw a box around the title.

        by = y1 == y ? 0 :
             y1 == y + 2 && title != NULL ? 2 :
             y1 == y + h - 1 ? 3 : 1;

        for (x1=x; x1<x+w; ++x1)
        {
            bx = x1 == x ? 0 :
                 x1 == x + w - 1 ? 3 : 1;
                 
            if (VALID_X(x1) && VALID_Y(y1))
            {
                TXT_GotoXY(x1, y1);
                TXT_PutChar(borders[by][bx]);
            }
        }
    }

    // Draw the title

    if (title != NULL)
    {
        TXT_GotoXY(x + 1, y + 1);
        TXT_BGColor(TXT_COLOR_GREY, 0);
        TXT_FGColor(TXT_COLOR_BLUE);

        for (x1=0; x1<w-2; ++x1)
        {
            TXT_DrawString(" ");
        }
    
        TXT_GotoXY(x + (w - strlen(title)) / 2, y + 1);
        TXT_DrawString(title);
    }

    // Draw the window's shadow.

    TXT_DrawShadow(x + 2, y + h, w, 1);
    TXT_DrawShadow(x + w, y + 1, 2, h);
}

void TXT_DrawSeparator(int x, int y, int w)
{
    unsigned char *data;
    int x1;
    int b;

    data = TXT_GetScreenData();

    TXT_FGColor(TXT_COLOR_BRIGHT_CYAN);
    TXT_BGColor(TXT_WINDOW_BACKGROUND, 0);

    if (!VALID_Y(y))
    {
        return;
    }

    data += (y * TXT_SCREEN_W + x) * 2;

    for (x1=x; x1<x+w; ++x1)
    {
        TXT_GotoXY(x1, y);

        b = x1 == x ? 0 :
            x1 == x + w - 1 ? 3 :
            1;

        if (VALID_X(x1))
        {
            // Read the current value from the screen
            // Check that it matches what the window should look like if
            // there is no separator, then apply the separator

            if (*data == borders[1][b])
            {
                TXT_PutChar(borders[2][b]);
            }
        }

        data += 2;
    }
}

void TXT_DrawString(const char *s)
{
    int x, y;
    int x1;
    const char *p;

    TXT_GetXY(&x, &y);

    if (VALID_Y(y))
    {
        x1 = x;

        for (p = s; *p != '\0'; ++p)
        {
            if (VALID_X(x1))
            {
                TXT_GotoXY(x1, y);
                TXT_PutChar(*p);
            }

            x1 += 1;
        }
    }

    TXT_GotoXY(x + strlen(s), y);
}

void TXT_DrawHorizScrollbar(int x, int y, int w, int cursor, int range)
{
    int x1;
    int cursor_x;

    if (!VALID_Y(y))
    {
        return;
    }

    TXT_FGColor(TXT_COLOR_BLACK);
    TXT_BGColor(TXT_COLOR_GREY, 0);

    TXT_GotoXY(x, y);
    TXT_PutChar('\x1b');

    cursor_x = x + 1;

    if (range > 1)
    {
        cursor_x += (cursor * (w - 3)) / (range - 1);
    }

    if (cursor_x > x + w - 2)
    {
        cursor_x = x + w - 2;
    }

    for (x1=x+1; x1<x+w-1; ++x1)
    {
        if (VALID_X(x1))
        {
            if (x1 == cursor_x)
            {
                TXT_PutChar('\xdb');
            }
            else
            {
                TXT_PutChar('\xb1');
            }
        }
    }

    TXT_PutChar('\x1a');
}

void TXT_DrawVertScrollbar(int x, int y, int h, int cursor, int range)
{
    int y1;
    int cursor_y;

    if (!VALID_X(x))
    {
        return;
    }

    TXT_FGColor(TXT_COLOR_BLACK);
    TXT_BGColor(TXT_COLOR_GREY, 0);

    TXT_GotoXY(x, y);
    TXT_PutChar('\x18');

    cursor_y = y + 1;

    if (cursor_y > y + h - 2)
    {
        cursor_y = y + h - 2;
    }

    if (range > 1)
    {
        cursor_y += (cursor * (h - 3)) / (range - 1);
    }

    for (y1=y+1; y1<y+h-1; ++y1)
    {
        if (VALID_Y(y1))
        {
            TXT_GotoXY(x, y1);

            if (y1 == cursor_y)
            {
                TXT_PutChar('\xdb');
            }
            else
            {
                TXT_PutChar('\xb1');
            }
        }
    }

    TXT_GotoXY(x, y + h - 1);
    TXT_PutChar('\x19');
}

void TXT_InitClipArea(void)
{
    if (cliparea == NULL)
    {
        cliparea = malloc(sizeof(txt_cliparea_t));
        cliparea->x1 = 0;
        cliparea->x2 = TXT_SCREEN_W;
        cliparea->y1 = 1;
        cliparea->y2 = TXT_SCREEN_H - 1;
        cliparea->next = NULL;
    }
}

void TXT_PushClipArea(int x1, int x2, int y1, int y2)
{
    txt_cliparea_t *newarea;

    newarea = malloc(sizeof(txt_cliparea_t));

    // Set the new clip area to the intersection of the old
    // area and the new one.

    newarea->x1 = cliparea->x1;
    newarea->x2 = cliparea->x2;
    newarea->y1 = cliparea->y1;
    newarea->y2 = cliparea->y2;

    if (x1 > newarea->x1)
        newarea->x1 = x1;
    if (x2 < newarea->x2)
        newarea->x2 = x2;
    if (y1 > newarea->y1)
        newarea->y1 = y1;
    if (y2 < newarea->y2)
        newarea->y2 = y2;

#if 0
    printf("New scrollable area: %i,%i-%i,%i\n", x1, y1, x2, y2);
#endif

    // Hook into the list

    newarea->next = cliparea;
    cliparea = newarea;
}

void TXT_PopClipArea(void)
{
    txt_cliparea_t *next_cliparea;

    // Never pop the last entry

    if (cliparea->next == NULL)
        return;

    // Unlink the last entry and delete
   
    next_cliparea = cliparea->next;
    free(cliparea);
    cliparea = next_cliparea;
}

