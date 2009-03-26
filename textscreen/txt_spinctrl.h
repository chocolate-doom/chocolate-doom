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

#ifndef TXT_SPINCONTROL_H
#define TXT_SPINCONTROL_H

/**
 * @file txt_spinctrl.h
 *
 * Spin control widget.
 */

/**
 * Spin control widget.
 *
 * A spin control widget works as an input box that can be used to
 * set numeric values, but also has buttons that allow its value
 * to be increased or decreased.
 */

typedef struct txt_spincontrol_s txt_spincontrol_t;

typedef enum
{
    TXT_SPINCONTROL_INT,
    TXT_SPINCONTROL_FLOAT,
} txt_spincontrol_type_t;

#include "txt_widget.h"

struct txt_spincontrol_s
{
    txt_widget_t widget;
    txt_spincontrol_type_t type;
    union { float f; int i; } min, max, *value, step; 
    int editing;
    char *buffer;
};

/**
 * Create a new spin control widget tracking an integer value.
 *
 * @param value        Pointer to the variable containing the value
 *                     displayed in the widget.
 * @param min          Minimum value that may be set.
 * @param max          Maximum value that may be set.
 * @return             Pointer to the new spin control widget.
 */

txt_spincontrol_t *TXT_NewSpinControl(int *value, int min, int max);

/**
 * Create a new spin control widget tracking a float value.
 *
 * @param value        Pointer to the variable containing the value
 *                     displayed in the widget.
 * @param min          Minimum value that may be set.
 * @param max          Maximum value that may be set.
 * @return             Pointer to the new spin control widget.
 */

txt_spincontrol_t *TXT_NewFloatSpinControl(float *value, float min, float max);

#endif /* #ifndef TXT_SPINCONTROL_H */


