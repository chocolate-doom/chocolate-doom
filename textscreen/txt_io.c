//
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
// Text mode I/O functions, similar to C stdio
//

#include <stdlib.h>
#include <string.h>

#include "txt_io.h"
#include "txt_main.h"

static int cur_x = 0, cur_y = 0;
static txt_color_t fgcolor = TXT_COLOR_GREY;
static txt_color_t bgcolor = TXT_COLOR_BLACK;

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

        memmove(screendata, screendata + TXT_SCREEN_W * 2,
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

static void PutSymbol(unsigned char *screendata, int c)
{
    unsigned char *p;

    p = screendata + cur_y * TXT_SCREEN_W * 2 +  cur_x * 2;

    // Add a new character to the buffer

    p[0] = c;
    p[1] = fgcolor | (bgcolor << 4);

    ++cur_x;

    if (cur_x >= TXT_SCREEN_W)
    {
        NewLine(screendata);
    }
}

// "Blind" version of TXT_PutChar() below which doesn't do any interpretation
// of control signals. Just write a particular symbol to the screen buffer.
void TXT_PutSymbol(int c)
{
    PutSymbol(TXT_GetScreenData(), c);
}

static void PutChar(unsigned char *screendata, int c)
{
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
            PutSymbol(screendata, c);
            break;
    }
}

void TXT_PutChar(int c)
{
    PutChar(TXT_GetScreenData(), c);
}

void TXT_Puts(const char *s)
{
    unsigned char *screen;
    const char *p;

    screen = TXT_GetScreenData();

    for (p=s; *p != '\0'; ++p)
    {
        PutChar(screen, *p);
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

void TXT_SaveColors(txt_saved_colors_t *save)
{
    save->bgcolor = bgcolor;
    save->fgcolor = fgcolor;
}

void TXT_RestoreColors(txt_saved_colors_t *save)
{
    bgcolor = save->bgcolor;
    fgcolor = save->fgcolor;
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

