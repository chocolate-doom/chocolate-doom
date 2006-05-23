
#include <stdlib.h>
#include <string.h>

#include "doomkeys.h"

#include "txt_radiobutton.h"
#include "txt_io.h"
#include "txt_main.h"
#include "txt_window.h"

static void TXT_RadioButtonSizeCalc(TXT_UNCAST_ARG(radiobutton), int *w, int *h)
{
    TXT_CAST_ARG(txt_radiobutton_t, radiobutton);

    // Minimum width is the string length + two spaces for padding

    *w = strlen(radiobutton->label) + 6;
    *h = 1;
}

static void TXT_RadioButtonDrawer(TXT_UNCAST_ARG(radiobutton), int w, int selected)
{
    TXT_CAST_ARG(txt_radiobutton_t, radiobutton);
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

static void TXT_RadioButtonDestructor(TXT_UNCAST_ARG(radiobutton))
{
    TXT_CAST_ARG(txt_radiobutton_t, radiobutton);

    free(radiobutton->label);
}

static int TXT_RadioButtonKeyPress(TXT_UNCAST_ARG(radiobutton), int key)
{
    TXT_CAST_ARG(txt_radiobutton_t, radiobutton);

    if (key == KEY_ENTER || key == ' ')
    {
        if (*radiobutton->variable != radiobutton->value)
        {
            *radiobutton->variable = radiobutton->value;
            TXT_EmitSignal(radiobutton, "selected");
        }
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

    TXT_InitWidget(radiobutton, &txt_radiobutton_class);
    radiobutton->label = strdup(label);
    radiobutton->variable = variable;
    radiobutton->value = value;

    return radiobutton;
}

