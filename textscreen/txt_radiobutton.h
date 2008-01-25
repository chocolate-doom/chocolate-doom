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

#ifndef TXT_RADIOBUTTON_H
#define TXT_RADIOBUTTON_H

typedef struct txt_radiobutton_s txt_radiobutton_t;

#include "txt_widget.h"

struct txt_radiobutton_s
{
    txt_widget_t widget;
    char *label;
    int *variable;
    int value;
};

txt_radiobutton_t *TXT_NewRadioButton(char *label, int *variable, int value);
void TXT_SetRadioButtonLabel(txt_radiobutton_t *radiobutton, char *value);

#endif /* #ifndef TXT_RADIOBUTTON_H */


