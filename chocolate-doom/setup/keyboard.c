#include "textscreen.h"

int dummy;

void ConfigKeyboard(void)
{
    txt_window_t *window;
    txt_table_t *table;

    window = TXT_NewWindow("Keyboard configuration");

    table = TXT_NewTable(2);
    TXT_AddWidget(window, table);

    TXT_AddWidget(table, TXT_NewLabel("Move Forward"));
    TXT_AddWidget(table, TXT_NewIntInputBox(&dummy, 7));
    TXT_AddWidget(table, TXT_NewLabel("Move Backward"));
    TXT_AddWidget(table, TXT_NewIntInputBox(&dummy, 7));
    TXT_AddWidget(table, TXT_NewLabel("Turn Left"));
    TXT_AddWidget(table, TXT_NewIntInputBox(&dummy, 7));
    TXT_AddWidget(table, TXT_NewLabel("Turn Right"));
}

