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
//-----------------------------------------------------------------------------
//
// Text mode I/O functions, similar to C stdio
//
//-----------------------------------------------------------------------------

#include <stdlib.h>
#include <string.h>

#include "txt_io.h"
#include "txt_main.h"

static struct 
{
    txt_color_t color;
    const char *name;
} colors[] = {
    {TXT_COLOR_BLACK,           "black"},
    {TXT_COLOR_BLUE,            "blue"},
    {TXT_COLOR_GREEN,           "green"},
    {TXT_COLOR_CYAN,            "cyan"},
    {TXT_COLOR_RED,             "red"},
    {TXT_COLOR_MAGENTA,         "magenta"},
    {TXT_COLOR_BROWN,           "brown"},
    {TXT_COLOR_GREY,            "grey"},
    {TXT_COLOR_DARK_GREY,       "darkgrey"},
    {TXT_COLOR_BRIGHT_BLUE,     "brightblue"},
    {TXT_COLOR_BRIGHT_GREEN,    "brightgreen"},
    {TXT_COLOR_BRIGHT_CYAN,     "brightcyan"},
    {TXT_COLOR_BRIGHT_RED,      "brightred"},
    {TXT_COLOR_BRIGHT_MAGENTA,  "brightmagenta"},
    {TXT_COLOR_YELLOW,          "yellow"},
    {TXT_COLOR_BRIGHT_WHITE,    "brightwhite"},
};

static int cur_x = 0, cur_y = 0;
static txt_color_t fgcolor = TXT_COLOR_GREY;
static txt_color_t bgcolor = TXT_COLOR_BLACK;

static int GetColorForName(char *s)
{
    size_t i;

    for (i=0; i<sizeof(colors) / sizeof(*colors); ++i)
    {
        if (!strcmp(s, colors[i].name))
        {
            return colors[i].color;
        }
    }

    return -1;
}

static void NewLine(unsigned char *screendata)
{
    int i;
    unsigned char *p;

    cur_x = 0;
    ++cur_y;

    if (cur_y >= TXT_SCREEN_H)
    {
        // Scroll the screen up

        cur_y = TXT_SCREEN_H - 1;

        memcpy(screendata, screendata + TXT_SCREEN_W * 2,
               TXT_SCREEN_W * 2 * (TXT_SCREEN_H -1));

        // Clear the bottom line

        p = screendata + (TXT_SCREEN_H - 1) * 2 * TXT_SCREEN_W;

        for (i=0; i<TXT_SCREEN_W; ++i) 
        {
            *p++ = ' ';
            *p++ = fgcolor | (bgcolor << 4);
        }
    }
}

static void PutChar(unsigned char *screendata, int c)
{
    unsigned char *p;
   
    p = screendata + cur_y * TXT_SCREEN_W * 2 +  cur_x * 2;

    switch (c)
    {
        case '\n':
            NewLine(screendata);
            break;

        case '\b':
            // backspace
            --cur_x;
            if (cur_x < 0)
                cur_x = 0;
            break;

        default:

            // Add a new character to the buffer

            p[0] = c;
            p[1] = fgcolor | (bgcolor << 4);

            ++cur_x;

            if (cur_x >= TXT_SCREEN_W) 
            {
                NewLine(screendata);
            }

            break;
    }
}

void TXT_PutChar(int c)
{
    unsigned char *screen;

    screen = TXT_GetScreenData();

    PutChar(screen, c);
}

void TXT_Puts(const char *s)
{
    int previous_color = TXT_COLOR_BLACK;
    unsigned char *screen;
    const char *p;
    char colorname_buf[20];
    char *ending;
    int col;

    screen = TXT_GetScreenData();

    for (p=s; *p != '\0'; ++p)
    {
        if (*p == '<')
        {
            ++p;

            if (*p == '<')
            {
                PutChar(screen, '<');
            }
            else
            {
                ending = strchr(p, '>');

                if (ending == NULL)
                {
                    return;
                }

                strncpy(colorname_buf, p, 19);
                colorname_buf[ending-p] = '\0';

                if (!strcmp(colorname_buf, "/"))
                {
                    // End of color block

                    col = previous_color;
                }
                else
                {
                    col = GetColorForName(colorname_buf);

                    if (col < 0)
                    {
                        return;
                    }

                    // Save the color for the ending marker

                    previous_color = fgcolor;
                }

                TXT_FGColor(col);
                
                p = ending;
            }
        }
        else
        {
            PutChar(screen, *p);
        }
    }

    PutChar(screen, '\n');
}

void TXT_GotoXY(int x, int y)
{
    cur_x = x;
    cur_y = y;
}

void TXT_GetXY(int *x, int *y)
{
    *x = cur_x;
    *y = cur_y;
}

void TXT_FGColor(txt_color_t color)
{
    fgcolor = color;
}

void TXT_BGColor(int color, int blinking)
{
    bgcolor = color;
    if (blinking)
        bgcolor |= TXT_COLOR_BLINKING;
}

void TXT_ClearScreen(void)
{
    unsigned char *screen;
    int i;

    screen = TXT_GetScreenData();

    for (i=0; i<TXT_SCREEN_W * TXT_SCREEN_H; ++i)
    {
        screen[i * 2] = ' ';
        screen[i * 2 +  1] = (bgcolor << 4) | fgcolor;
    }

    cur_x = 0;
    cur_y = 0;
}

