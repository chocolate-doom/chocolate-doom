
#include <stdlib.h>
#include "textscreen.h"

int novert;
int speed;
int accel;
int threshold;

void ConfigMouse(void)
{
    txt_window_t *window;
    txt_table_t *table;

    window = TXT_NewWindow("Mouse configuration");

    TXT_AddWidget(window, TXT_NewSeparator("Mouse motion"));

    table = TXT_NewTable(2);

    TXT_AddWidget(table, TXT_NewLabel("Speed: "));
    TXT_AddWidget(table, TXT_NewIntInputBox(&speed, 8));
    TXT_AddWidget(table, TXT_NewLabel("Acceleration: "));
    TXT_AddWidget(table, TXT_NewIntInputBox(&accel, 8));
    TXT_AddWidget(table, TXT_NewLabel("Acceleration threshold: "));
    TXT_AddWidget(table, TXT_NewIntInputBox(&threshold, 8));
    
    TXT_AddWidget(window, table);
    
    TXT_AddWidget(window, TXT_NewSeparator(NULL));

    TXT_AddWidget(window, 
                  TXT_NewInvertedCheckBox("Allow vertical mouse movement", &novert));

}

