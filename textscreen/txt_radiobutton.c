
#include <string.h>

#include "doomkeys.h"

#include "txt_radiobutton.h"
#include "txt_io.h"
#include "txt_main.h"
#include "txt_window.h"

static void TXT_RadioButtonSizeCalc(txt_widget_t *widget, int *w, int *h)
{
    txt_radiobutton_t *radiobutton = (txt_radiobutton_t *) widget;

    // Minimum width is the string length + two spaces for padding

    *w = strlen(radiobutton->label) + 6;
    *h = 1;
}

static void TXT_RadioButtonDrawer(txt_widget_t *widget, int w, int selected)
{
    txt_radiobutton_t *radiobutton = (txt_radiobutton_t *) widget;
    int i;

    TXT_BGColor(TXT_COLOR_BLUE, 0);
    TXT_FGColor(TXT_COLOR_BRIGHT_CYAN);
    TXT_DrawString(" (");

    TXT_FGColor(TXT_COLOR_BRIGHT_WHITE);

    if (*radiobutton->variable == radiobutton->value)
    {
        TXT_DrawString("\x07");
    }
    else
    {
        TXT_DrawString(" ");
    }

    TXT_FGColor(TXT_COLOR_BRIGHT_CYAN);

    TXT_DrawString(") ");

    if (selected)
    {
        TXT_BGColor(TXT_COLOR_GREY, 0);
    }

    TXT_FGColor(TXT_COLOR_BRIGHT_WHITE);

    TXT_DrawString(radiobutton->label);
    
    for (i=strlen(radiobutton->label); i < w-6; ++i)
    {
        TXT_DrawString(" ");
    }
}

static void TXT_RadioButtonDestructor(txt_widget_t *widget)
{
    txt_radiobutton_t *radiobutton = (txt_radiobutton_t *) widget;

    free(radiobutton->label);
}

static int TXT_RadioButtonKeyPress(txt_widget_t *widget, int key)
{
    txt_radiobutton_t *radiobutton = (txt_radiobutton_t *) widget;

    if (key == KEY_ENTER || key == ' ')
    {
        *radiobutton->variable = radiobutton->value;
        return 1;
    }
    
    return 0;
}

txt_widget_class_t txt_radiobutton_class =
{
    TXT_RadioButtonSizeCalc,
    TXT_RadioButtonDrawer,
    TXT_RadioButtonKeyPress,
    TXT_RadioButtonDestructor,
};

txt_radiobutton_t *TXT_NewRadioButton(char *label, int *variable, int value)
{
    txt_radiobutton_t *radiobutton;

    radiobutton = malloc(sizeof(txt_radiobutton_t));

    radiobutton->widget.widget_class = &txt_radiobutton_class;
    radiobutton->widget.selectable = 1;
    radiobutton->widget.visible = 1;
    radiobutton->label = strdup(label);
    radiobutton->variable = variable;
    radiobutton->value = value;

    return radiobutton;
}

