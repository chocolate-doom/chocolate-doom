#include "textscreen.h"

int dummy;

void ConfigKeyboard(void)
{
    txt_window_t *window;
    txt_table_t *table;

    window = TXT_NewWindow("Keyboard configuration");

    TXT_AddWidget(window, TXT_NewSeparator("Movement"));

    table = TXT_NewTable(2);
    TXT_AddWidget(table, TXT_NewStrut(20, 0));
    TXT_AddWidget(table, TXT_NewStrut(8, 0));

    TXT_AddWidget(table, TXT_NewLabel("Move Forward"));
    TXT_AddWidget(table, TXT_NewIntInputBox(&dummy, 7));
    TXT_AddWidget(table, TXT_NewLabel("Move Backward"));
    TXT_AddWidget(table, TXT_NewIntInputBox(&dummy, 7));
    TXT_AddWidget(table, TXT_NewLabel("Turn Left"));
    TXT_AddWidget(table, TXT_NewIntInputBox(&dummy, 7));
    TXT_AddWidget(table, TXT_NewLabel("Turn Right"));
    TXT_AddWidget(table, TXT_NewIntInputBox(&dummy, 7));
    TXT_AddWidget(table, TXT_NewLabel("Strafe Left"));
    TXT_AddWidget(table, TXT_NewIntInputBox(&dummy, 7));
    TXT_AddWidget(table, TXT_NewLabel("Strafe Right"));
    TXT_AddWidget(table, TXT_NewIntInputBox(&dummy, 7));
    TXT_AddWidget(table, TXT_NewLabel("Speed On"));
    TXT_AddWidget(table, TXT_NewIntInputBox(&dummy, 7));
    TXT_AddWidget(table, TXT_NewLabel("Strafe On"));
    TXT_AddWidget(table, TXT_NewIntInputBox(&dummy, 7));

    TXT_AddWidget(window, table);

    TXT_AddWidget(window, TXT_NewSeparator("Action"));

    table = TXT_NewTable(2);
    TXT_AddWidget(table, TXT_NewStrut(20, 0));
    TXT_AddWidget(table, TXT_NewStrut(8, 0));

    TXT_AddWidget(table, TXT_NewLabel("Use"));
    TXT_AddWidget(table, TXT_NewIntInputBox(&dummy, 7));
    TXT_AddWidget(table, TXT_NewLabel("Fire"));
    TXT_AddWidget(table, TXT_NewIntInputBox(&dummy, 7));

    TXT_AddWidget(window, table);
}

