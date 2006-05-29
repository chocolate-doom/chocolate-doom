
#include <stdio.h>
#include <string.h>
#include "txt_button.h"
#include "txt_desktop.h"
#include "txt_label.h"
#include "txt_separator.h"

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
    txt_button_t *button;
    char buf[10];
    int *val_copy;

    val_copy = malloc(sizeof(int));
    *val_copy = value;

    sprintf(buf, "  %i  ", value);

    button = TXT_NewButton(buf);
    TXT_AddWidget(table, button);
    TXT_SignalConnect(button, "pressed", InsertNumber, val_copy);
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
    txt_button_t *button;
    char buf[10];
    operator_t *op_copy;

    op_copy = malloc(sizeof(operator_t));
    *op_copy = op;

    sprintf(buf, "  %s  ", label);
    button = TXT_NewButton(buf);

    TXT_AddWidget(table, button);
    TXT_SignalConnect(button, "pressed", Operator, op_copy);
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
    }

    UpdateInputBox();

    operator = OP_NONE;
    starting_input = 1;
}

void BuildGUI()
{
    txt_window_t *window;
    txt_table_t *table;
    txt_button_t *equals_button;
    
    window = TXT_NewWindow("Calculator");

    input_box = TXT_NewLabel("asdf");
    TXT_SetBGColor(input_box, TXT_COLOR_BLACK);
    TXT_AddWidget(window, input_box);
    TXT_AddWidget(window, TXT_NewSeparator(NULL));
    TXT_AddWidget(window, NULL);

    table = TXT_NewTable(4);
    TXT_AddWidget(window, table);

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
    equals_button = TXT_NewButton("  =  ");
    TXT_SignalConnect(equals_button, "pressed", Calculate, NULL);
    TXT_AddWidget(table, equals_button);
    AddOperatorButton(table, "/", OP_DIV);
    
    TXT_AddWidget(window, NULL);
    UpdateInputBox();
}

int main()
{
    TXT_Init();
    TXT_SetDesktopTitle("Calculator demo");

    BuildGUI();

    TXT_GUIMainLoop();
}

