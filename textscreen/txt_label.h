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

#ifndef TXT_LABEL_H
#define TXT_LABEL_H

typedef struct txt_label_s txt_label_t;

#include "txt_main.h"
#include "txt_widget.h"

struct txt_label_s
{
    txt_widget_t widget;
    char *label;
    char **lines;
    unsigned int w, h;
    txt_color_t fgcolor;
    txt_color_t bgcolor;
};

txt_label_t *TXT_NewLabel(char *label);
void TXT_SetLabel(txt_label_t *label, char *value);
void TXT_SetBGColor(txt_label_t *label, txt_color_t color);
void TXT_SetFGColor(txt_label_t *label, txt_color_t color);

#endif /* #ifndef TXT_LABEL_H */


