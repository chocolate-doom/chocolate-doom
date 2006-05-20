#include <stdlib.h>
#include <string.h>

#include "txt_main.h"

#include "txt_checkbox.h"
#include "txt_button.h"
#include "txt_desktop.h"
#include "txt_separator.h"
#include "txt_table.h"
#include "txt_window.h"

txt_window_t *firstwin;
int checkbox_value;

void SetupWindow(void)
{
    txt_window_t *window;
    txt_table_t *table;
    char buf[100];
    int i;
    
    window = TXT_NewWindow("Window test");

    table = TXT_NewTable(3);

    TXT_AddWidget(window, TXT_NewLabel(" This is a multiline label.\n"
                                       " A single label object contains \n"
                                       " all three of these lines.\n"));

    TXT_AddWidget(window, table);

    for (i=0; i<5; ++i)
    {
        sprintf(buf, " Option %i in a table:", i + 1);
        TXT_AddWidget(table, TXT_NewLabel(buf));
        sprintf(buf, "Button %i-1", i + 1);
        TXT_AddWidget(table, TXT_NewButton(buf));
        sprintf(buf, "Button %i-2", i + 1);
        TXT_AddWidget(table, TXT_NewButton(buf));
    }

    TXT_AddWidget(window, TXT_NewLabel(""));
    TXT_AddWidget(window, TXT_NewCheckBox("Checkbox", &checkbox_value));

    firstwin = window;
}

void Window2(void)
{
    txt_window_t *window;
    int i;
    
    window = TXT_NewWindow("Another test");
    TXT_SetWindowPosition(window, 
                          TXT_HORIZ_RIGHT, 
                          TXT_VERT_TOP, 
                          TXT_SCREEN_W - 1, 1);

    for (i=0; i<5; ++i)
    {
        TXT_AddWidget(window, TXT_NewButton("hello there blah blah blah blah"));
    }
}

void DrawASCIIChart()
{
    int x, y;

    TXT_ClearScreen();

    for (y=0; y<16; ++y)
    {
        for (x=0; x<16; ++x)
        {
            TXT_PutChar(' ');
            TXT_PutChar(' ');
            TXT_PutChar(' ');
            TXT_PutChar(y * 16 + x);
        }
        TXT_PutChar('\n');
    }
}

int main()
{
    TXT_Init();

    TXT_SetDesktopTitle("Not Chocolate Doom Setup");

    Window2();
    SetupWindow();

    TXT_GUIMainLoop();
}


