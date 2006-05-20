
#include <string.h>

#include "txt_button.h"
#include "txt_io.h"
#include "txt_main.h"
#include "txt_widget.h"
#include "txt_window.h"

static int TXT_ButtonSizeCalc(txt_widget_t *widget)
{
    txt_button_t *button = (txt_button_t *) widget;

    // Minimum width is the string length + two spaces for padding

    return strlen(button->label) + 2;
}

static void TXT_ButtonDrawer(txt_widget_t *widget, int w, int selected)
{
    txt_button_t *button = (txt_button_t *) widget;
    int i;

    TXT_BGColor(TXT_COLOR_BLUE, 0);
    TXT_FGColor(TXT_COLOR_BRIGHT_WHITE);
    TXT_DrawString(" ");

    if (selected)
    {
        TXT_BGColor(TXT_COLOR_GREY, 0);
    }

    TXT_DrawString(button->label);
    
    for (i=strlen(button->label); i < w-2; ++i)
    {
        TXT_DrawString(" ");
    }
}

static void TXT_ButtonDestructor(txt_widget_t *widget)
{
    txt_button_t *button = (txt_button_t *) widget;

    free(button->label);
}

static int TXT_ButtonKeyPress(txt_widget_t *widget, int key)
{
    return 0;
}

txt_widget_class_t txt_button_class =
{
    TXT_ButtonSizeCalc,
    TXT_ButtonDrawer,
    TXT_ButtonKeyPress,
    TXT_ButtonDestructor,
};

txt_button_t *TXT_NewButton(char *label)
{
    txt_button_t *button;

    button = malloc(sizeof(txt_button_t));

    button->widget.widget_class = &txt_button_class;
    button->widget.selectable = 1;
    button->widget.visible = 1;
    button->label = strdup(label);

    return button;
}

