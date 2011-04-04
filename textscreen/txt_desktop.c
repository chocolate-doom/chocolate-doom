// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// Copyright(C) 2006 Simon Howard
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "doomkeys.h"

#include "txt_desktop.h"
#include "txt_gui.h"
#include "txt_io.h"
#include "txt_main.h"
#include "txt_separator.h"
#include "txt_window.h"

#define MAXWINDOWS 128

static char *desktop_title;
static txt_window_t *all_windows[MAXWINDOWS];
static int num_windows = 0;
static int main_loop_running = 0;

void TXT_AddDesktopWindow(txt_window_t *win)
{
    all_windows[num_windows] = win;
    ++num_windows;
}

void TXT_RemoveDesktopWindow(txt_window_t *win)
{
    int from, to;

    for (from=0, to=0; from<num_windows; ++from)
    {
        if (all_windows[from] != win)
        {
            all_windows[to] = all_windows[from];
            ++to;
        }
    }
    
    num_windows = to;
}

txt_window_t *TXT_GetActiveWindow(void)
{
    if (num_windows == 0)
    {
        return NULL;
    }

    return all_windows[num_windows - 1];
}

static void DrawDesktopBackground(const char *title)
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

void TXT_SetDesktopTitle(char *title)
{
    free(desktop_title);
    desktop_title = strdup(title);
    TXT_SetWindowTitle(title);
}

void TXT_DrawDesktop(void)
{
    int i;
    const char *title;

    TXT_InitClipArea();

    if (desktop_title == NULL)
        title = "";
    else
        title = desktop_title;

    DrawDesktopBackground(title);

    for (i=0; i<num_windows; ++i)
    {
        TXT_DrawWindow(all_windows[i], i == num_windows - 1);
    }

    TXT_UpdateScreen();
}

void TXT_DispatchEvents(void)
{
    int c;

    while ((c = TXT_GetChar()) > 0)
    {
        if (num_windows > 0)
        {
            // Send the keypress to the top window

            TXT_WindowKeyPress(all_windows[num_windows - 1], c);
        }
    }
}

void TXT_ExitMainLoop(void)
{
    main_loop_running = 0;
}

void TXT_DrawASCIITable(void)
{
    unsigned char *screendata;
    char buf[10];
    int x, y;
    int n;

    screendata = TXT_GetScreenData();

    TXT_FGColor(TXT_COLOR_BRIGHT_WHITE);
    TXT_BGColor(TXT_COLOR_BLACK, 0);

    for (y=0; y<16; ++y)
    {
        for (x=0; x<16; ++x)
        {
            n = y * 16 + x;

            TXT_GotoXY(x * 5, y);
            sprintf(buf, "%02x   ", n);
            TXT_Puts(buf);

            // Write the character directly to the screen memory buffer:

            screendata[(y * TXT_SCREEN_W + x * 5 + 3) * 2] = n;
        }
    }

    TXT_UpdateScreen();
}

void TXT_GUIMainLoop(void)
{
    main_loop_running = 1;

    while (main_loop_running)
    {
        TXT_DispatchEvents();

        // After the last window is closed, exit the loop

        if (num_windows <= 0)
        {
            TXT_ExitMainLoop();
        }
        
        TXT_DrawDesktop();
//        TXT_DrawASCIITable();
        TXT_Sleep(0);
    }
}

