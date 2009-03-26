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

#ifndef TXT_INPUTBOX_H
#define TXT_INPUTBOX_H

/**
 * @file txt_inputbox.h
 *
 * Input box widget.
 */

/**
 * Input box widget.
 *
 * An input box is a widget that displays a value, which can be
 * selected to enter a new value.
 *
 * Input box widgets can be of an integer or string type.
 */

typedef struct txt_inputbox_s txt_inputbox_t;

#include "txt_widget.h"

struct txt_inputbox_s
{
    txt_widget_t widget;
    char *buffer;
    unsigned int size;
    int editing;
    void *value;
};

/**
 * Create a new input box widget for controlling a string value.
 *
 * @param value         Pointer to a string variable that contains
 *                      a pointer to the current value of the
 *                      input box.  The value should be allocated
 *                      dynamically; when the string is changed it
 *                      will be freed and the variable set to point
 *                      to the new string value.
 * @param size          Width of the input box, in characters.
 * @return              Pointer to the new input box widget.
 */

txt_inputbox_t *TXT_NewInputBox(char **value, int size);

/**
 * Create a new input box widget for controlling an integer value.
 *
 * @param value         Pointer to an integer variable containing
 *                      the value of the input box.
 * @param size          Width of the input box, in characters.
 * @return              Pointer to the new input box widget.
 */

txt_inputbox_t *TXT_NewIntInputBox(int *value, int size);

#endif /* #ifndef TXT_INPUTBOX_H */


