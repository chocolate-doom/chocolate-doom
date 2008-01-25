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

#ifndef TXT_WINDOW_H
#define TXT_WINDOW_H

typedef struct txt_window_s txt_window_t;

#include "txt_widget.h" 
#include "txt_table.h"
#include "txt_window_action.h"

// Callback function for window key presses

typedef int (*TxtWindowKeyPress)(txt_window_t *window, int key, void *user_data);
typedef int (*TxtWindowMousePress)(txt_window_t *window, 
                                   int x, int y, int b, 
                                   void *user_data);

struct txt_window_s
{
    // Base class: all windows are tables with one column.

    txt_table_t table;
    
    // Window title

    char *title;

    // Screen coordinates of the window

    txt_vert_align_t vert_align;
    txt_horiz_align_t horiz_align;
    int x, y;

    // Actions that appear in the box at the bottom of the window

    txt_window_action_t *actions[3];

    // Callback functions to invoke when keys/mouse buttons are pressed

    TxtWindowKeyPress key_listener;
    void *key_listener_data;
    TxtWindowMousePress mouse_listener;
    void *mouse_listener_data;

    // These are set automatically when the window is drawn

    int window_x, window_y;
    unsigned int window_w, window_h;
};

txt_window_t *TXT_NewWindow(char *title);
void TXT_CloseWindow(txt_window_t *window);
void TXT_SetWindowPosition(txt_window_t *window, 
                           txt_horiz_align_t horiz_align,
                           txt_vert_align_t vert_align,
                           int x, int y);
void TXT_SetWindowAction(txt_window_t *window, txt_horiz_align_t position, 
                         txt_window_action_t *action);
void TXT_SetKeyListener(txt_window_t *window, 
                        TxtWindowKeyPress key_listener, 
                        void *user_data);
void TXT_SetMouseListener(txt_window_t *window, 
                          TxtWindowMousePress mouse_listener,
                          void *user_data);

#endif /* #ifndef TXT_WINDOW_T */


