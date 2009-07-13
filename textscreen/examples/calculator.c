// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// Copyright(C) 2006-2009 Simon Howard
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
//-----------------------------------------------------------------------------
//
// Example program: desktop calculator
//
//-----------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "textscreen.h"

typedef enum
{
    OP_NONE,
    OP_PLUS,
    OP_MINUS,
    OP_MULT,
    OP_DIV,
} operator_t;

int starting_input = 0;
int input_value = 0;
txt_label_t *input_box;
int first_operand;
operator_t operator = OP_NONE;

void UpdateInputBox(void)
{
    char buf[20];

    sprintf(buf, "  %i", input_value);
    TXT_SetLabel(input_box, buf);
}

void InsertNumber(TXT_UNCAST_ARG(button), TXT_UNCAST_ARG(value))
{
    TXT_CAST_ARG(int, value);

    if (starting_input)
    {
        input_value = 0;
        starting_input = 0;
    }
    
    input_value *= 10;
    input_value += *value;
    UpdateInputBox();
}

void AddNumberButton(txt_table_t *table, int value)
{
    char buf[10];
    int *val_copy;

    val_copy = malloc(sizeof(int));
    *val_copy = value;

    sprintf(buf, "  %i  ", value);

    TXT_AddWidget(table, TXT_NewButton2(buf, InsertNumber, val_copy));
}

void Operator(TXT_UNCAST_ARG(button), TXT_UNCAST_ARG(op))
{
    TXT_CAST_ARG(operator_t, op);

    first_operand = input_value;
    operator = *op;
    starting_input = 1;
}

void AddOperatorButton(txt_table_t *table, char *label, operator_t op)
{
    char buf[10];
    operator_t *op_copy;

    op_copy = malloc(sizeof(operator_t));
    *op_copy = op;

    sprintf(buf, "  %s  ", label);

    TXT_AddWidget(table, TXT_NewButton2(buf, Operator, op_copy));
}

void Calculate(TXT_UNCAST_ARG(button), void *unused)
{
    switch (operator)
    {
        case OP_PLUS:
            input_value = first_operand + input_value;
            break;
        case OP_MINUS:
            input_value = first_operand - input_value;
            break;
        case OP_MULT:
            input_value = first_operand * input_value;
            break;
        case OP_DIV:
            input_value = first_operand / input_value;
            break;
        case OP_NONE:
            break;
    }

    UpdateInputBox();

    operator = OP_NONE;
    starting_input = 1;
}

void BuildGUI()
{
    txt_window_t *window;
    txt_table_t *table;
    
    window = TXT_NewWindow("Calculator");

    input_box = TXT_NewLabel("asdf");
    TXT_SetBGColor(input_box, TXT_COLOR_BLACK);
    TXT_AddWidget(window, input_box);
    TXT_AddWidget(window, TXT_NewSeparator(NULL));
    TXT_AddWidget(window, TXT_NewStrut(0, 1));

    table = TXT_NewTable(4);
    TXT_AddWidget(window, table);
    TXT_SetWidgetAlign(table, TXT_HORIZ_CENTER);

    AddNumberButton(table, 7);
    AddNumberButton(table, 8);
    AddNumberButton(table, 9);
    AddOperatorButton(table, "*", OP_MULT);
    AddNumberButton(table, 4);
    AddNumberButton(table, 5);
    AddNumberButton(table, 6);
    AddOperatorButton(table, "-", OP_MINUS);
    AddNumberButton(table, 1);
    AddNumberButton(table, 2);
    AddNumberButton(table, 3);
    AddOperatorButton(table, "+", OP_PLUS);
    AddNumberButton(table, 0);
    TXT_AddWidget(table, NULL);

    TXT_AddWidget(table, TXT_NewButton2("  =  ", Calculate, NULL));
    AddOperatorButton(table, "/", OP_DIV);
    
    TXT_AddWidget(window, TXT_NewStrut(0, 1));
    UpdateInputBox();
}

int main(int argc, char *argv[])
{
    if (!TXT_Init())
    {
        fprintf(stderr, "Failed to initialise GUI\n");
        exit(-1);
    }
    
    TXT_SetDesktopTitle("Calculator demo");

    BuildGUI();

    TXT_GUIMainLoop();

    TXT_Shutdown();

    return 0;
}

