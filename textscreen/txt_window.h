// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id$
//
// Copyright(C) 1993-1996 Id Software, Inc.
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

#ifndef TXT_WINDOW_H
#define TXT_WINDOW_H

typedef struct txt_window_s txt_window_t;

#include "txt_widget.h" 

struct txt_window_s
{
    // Window title

    char *title;

    // Screen coordinates of the centerpoint of the window

    int x, y;

    // Widgets in this window

    txt_widget_t **widgets;
    int num_widgets;

    // Index of the current selected widget.

    int selected;
};

txt_window_t *TXT_NewWindow(char *title, int x, int y);
void TXT_CloseWindow(txt_window_t *window);
void TXT_AddWidget(txt_window_t *window, void *widget);
void TXT_SetDesktopTitle(char *title);
void TXT_DrawAllWindows(void);

#endif /* #ifndef TXT_WINDOW_T */


