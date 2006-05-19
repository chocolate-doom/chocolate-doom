#include <stdlib.h>
#include <string.h>

#include "txt_main.h"

#include "txt_button.h"
#include "txt_desktop.h"
#include "txt_separator.h"
#include "txt_window.h"

txt_window_t *firstwin;

void SetupWindow(void)
{
    txt_window_t *window;
    char buf[100];
    int i;
    
    window = TXT_NewWindow("Window test", 40, 12);

    strcpy(buf, "This is a button label: ");

    TXT_AddWidget(window, TXT_NewSeparator("Main Section"));

    for (i=0; i<8; ++i)
    {
        strcat(buf, "a");
        TXT_AddWidget(window, TXT_NewButton(buf));

        if (i == 4)
        {
            TXT_AddWidget(window, TXT_NewSeparator("Section"));
        }

        if (i == 6)
        {
            TXT_AddWidget(window, TXT_NewSeparator(NULL));
        }
    }

    firstwin = window;
}

void Window2(void)
{
    txt_window_t *window;
    int i;
    
    window = TXT_NewWindow("Another test", 50, 7);

    for (i=0; i<5; ++i)
    {
        TXT_AddWidget(window, TXT_NewButton("hello there blah blah blah blah"));
    }
}

int main()
{
    TXT_Init();
    TXT_SetDesktopTitle("Not Chocolate Doom Setup");

    Window2();
    SetupWindow();

    for (;;)
    {
        firstwin->selected = (firstwin->selected + 1) % firstwin->num_widgets;

        TXT_DrawDesktop();
    }
}


