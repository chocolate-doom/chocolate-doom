
#include <stdlib.h>
#include <string.h>

#include "doomkeys.h"

#include "txt_checkbox.h"
#include "txt_io.h"
#include "txt_main.h"
#include "txt_window.h"

static void TXT_CheckBoxSizeCalc(TXT_UNCAST_ARG(checkbox))
{
    TXT_CAST_ARG(txt_checkbox_t, checkbox);

    // Minimum width is the string length + two spaces for padding

    checkbox->widget.w = strlen(checkbox->label) + 6;
    checkbox->widget.h = 1;
}

static void TXT_CheckBoxDrawer(TXT_UNCAST_ARG(checkbox), int selected)
{
    TXT_CAST_ARG(txt_checkbox_t, checkbox);
    int i;
    int w;

    w = checkbox->widget.w;

    TXT_BGColor(TXT_COLOR_BLUE, 0);
    TXT_FGColor(TXT_COLOR_BRIGHT_CYAN);
    TXT_DrawString(" (");

    TXT_FGColor(TXT_COLOR_BRIGHT_WHITE);

    if ((*checkbox->variable != 0) ^ checkbox->inverted)
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

    TXT_DrawString(checkbox->label);
    
    for (i=strlen(checkbox->label); i < w-6; ++i)
    {
        TXT_DrawString(" ");
    }
}

static void TXT_CheckBoxDestructor(TXT_UNCAST_ARG(checkbox))
{
    TXT_CAST_ARG(txt_checkbox_t, checkbox);

    free(checkbox->label);
}

static int TXT_CheckBoxKeyPress(TXT_UNCAST_ARG(checkbox), int key)
{
    TXT_CAST_ARG(txt_checkbox_t, checkbox);

    if (key == KEY_ENTER || key == ' ')
    {
        *checkbox->variable = !*checkbox->variable;
        TXT_EmitSignal(checkbox, "changed");
        return 1;
    }
    
    return 0;
}

static void TXT_CheckBoxMousePress(TXT_UNCAST_ARG(checkbox), int x, int y, int b)
{
    TXT_CAST_ARG(txt_checkbox_t, checkbox);

    if (b == TXT_MOUSE_LEFT)
    {
        // Equivalent to pressing enter

        TXT_CheckBoxKeyPress(checkbox, KEY_ENTER);
    }
}

txt_widget_class_t txt_checkbox_class =
{
    TXT_CheckBoxSizeCalc,
    TXT_CheckBoxDrawer,
    TXT_CheckBoxKeyPress,
    TXT_CheckBoxDestructor,
    TXT_CheckBoxMousePress,
};

txt_checkbox_t *TXT_NewCheckBox(char *label, int *variable)
{
    txt_checkbox_t *checkbox;

    checkbox = malloc(sizeof(txt_checkbox_t));

    TXT_InitWidget(checkbox, &txt_checkbox_class);
    checkbox->label = strdup(label);
    checkbox->variable = variable;
    checkbox->inverted = 0;

    return checkbox;
}

txt_checkbox_t *TXT_NewInvertedCheckBox(char *label, int *variable)
{
    txt_checkbox_t *result;

    result = TXT_NewCheckBox(label, variable);
    result->inverted = 1;

    return result;
}

