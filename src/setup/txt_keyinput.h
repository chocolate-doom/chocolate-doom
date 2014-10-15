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

#ifndef TXT_KEY_INPUT_H
#define TXT_KEY_INPUT_H

typedef struct txt_key_input_s txt_key_input_t;

#include "txt_widget.h"

//
// A key input is like an input box.  When selected, a box pops up
// allowing a key to be selected.
//

struct txt_key_input_s
{
    txt_widget_t widget;
    int *variable;
    int check_conflicts;
};

txt_key_input_t *TXT_NewKeyInput(int *variable);

#endif /* #ifndef TXT_KEY_INPUT_H */


