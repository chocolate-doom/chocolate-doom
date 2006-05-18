#include <stdlib.h>


#include "txt_main.h"

#include "txt_button.h"
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

    {
        txt_separator_t *sep;
        sep = TXT_NewSeparator("Main Section");
        TXT_AddWidget(window, &sep->widget);
    }

    for (i=0; i<8; ++i)
    {
        txt_button_t *button;

        button = TXT_NewButton(buf);
        strcat(buf, "a");
        TXT_AddWidget(window, &button->widget);

        if (i == 4)
        {
            txt_separator_t *sep;

            sep = TXT_NewSeparator("Section");
            TXT_AddWidget(window, &sep->widget);
        }

        if (i == 6)
        {
            txt_separator_t *sep;

            sep = TXT_NewSeparator(NULL);
            TXT_AddWidget(window, &sep->widget);
        }
    }

    firstwin = window;
}

void Window2(void)
{
    txt_window_t *window;
    int i;
    
    window = TXT_NewWindow("Another test", 30, 7);

    for (i=0; i<5; ++i)
    {
        txt_button_t *button;

        button = TXT_NewButton("hello there blah blah blah blah");
        TXT_AddWidget(window, &button->widget);
    }
}

int main()
{
    TXT_Init();

    Window2();
    SetupWindow();

    for (;;)
    {
        firstwin->selected = (firstwin->selected + 1) % firstwin->num_widgets;

    TXT_DrawAllWindows();

    }
}


