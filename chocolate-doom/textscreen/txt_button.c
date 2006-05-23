
#include <string.h>

#include "doomkeys.h"

#include "txt_button.h"
#include "txt_io.h"
#include "txt_main.h"
#include "txt_window.h"

static void TXT_ButtonSizeCalc(TXT_UNCAST_ARG(button), int *w, int *h)
{
    TXT_CAST_ARG(txt_button_t, button);

    // Minimum width is the string length + two spaces for padding

    *w = strlen(button->label) + 2;
    *h = 1;
}

static void TXT_ButtonDrawer(TXT_UNCAST_ARG(button), int w, int selected)
{
    TXT_CAST_ARG(txt_button_t, button);
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

static void TXT_ButtonDestructor(TXT_UNCAST_ARG(button))
{
    TXT_CAST_ARG(txt_button_t, button);

    free(button->label);
}

static int TXT_ButtonKeyPress(TXT_UNCAST_ARG(button), int key)
{
    TXT_CAST_ARG(txt_button_t, button);

    if (key == KEY_ENTER)
    {
        TXT_EmitSignal(button, "pressed");
        return 1;
    }
    
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

    TXT_InitWidget(button, &txt_button_class);
    button->label = strdup(label);

    return button;
}

