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

#ifndef TXT_WINDOW_ACTION_H
#define TXT_WINDOW_ACTION_H

typedef struct txt_window_action_s txt_window_action_t;

#include "txt_widget.h"
#include "txt_window.h"

struct txt_window_action_s
{
    txt_widget_t widget;
    char *label;
    int key;
};

txt_window_action_t *TXT_NewWindowAction(int key, char *label);

// Creates an "escape" button that closes the window

txt_window_action_t *TXT_NewWindowEscapeAction(txt_window_t *window);

// Same as above, but the button is named "abort"

txt_window_action_t *TXT_NewWindowAbortAction(txt_window_t *window);

// Accept button that does nothing

txt_window_action_t *TXT_NewWindowSelectAction(txt_window_t *window);

#endif /* #ifndef TXT_WINDOW_ACTION_H */

