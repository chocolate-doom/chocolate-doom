
#include <stdlib.h>
#include <string.h>

#include "doomkeys.h"

#include "txt_window_action.h"
#include "txt_io.h"
#include "txt_main.h"
#include "txt_window.h"

static void TXT_WindowActionSizeCalc(TXT_UNCAST_ARG(action), 
                                     int *w, int *h)
{
    TXT_CAST_ARG(txt_window_action_t, action);
    char buf[10];

    TXT_GetKeyDescription(action->key, buf);

    // Minimum width is the string length + two spaces for padding

    *w = strlen(action->label) + strlen(buf) + 1;
    *h = 1;
}

static void TXT_WindowActionDrawer(TXT_UNCAST_ARG(action), int w, int selected)
{
    TXT_CAST_ARG(txt_window_action_t, action);
    int i;
    char buf[10];

    TXT_GetKeyDescription(action->key, buf);

    TXT_FGColor(TXT_COLOR_BRIGHT_GREEN);
    TXT_DrawString(buf);
    TXT_FGColor(TXT_COLOR_BRIGHT_CYAN);
    TXT_DrawString("=");
    TXT_FGColor(TXT_COLOR_BRIGHT_WHITE);
    TXT_DrawString(action->label);
}

static void TXT_WindowActionDestructor(TXT_UNCAST_ARG(action))
{
    TXT_CAST_ARG(txt_window_action_t, action);

    free(action->label);
}

static int TXT_WindowActionKeyPress(TXT_UNCAST_ARG(action), int key)
{
    TXT_CAST_ARG(txt_window_action_t, action);

    if (key == action->key)
    {
        TXT_EmitSignal(action, "pressed");
        return 1;
    }
    
    return 0;
}

txt_widget_class_t txt_window_action_class =
{
    TXT_WindowActionSizeCalc,
    TXT_WindowActionDrawer,
    TXT_WindowActionKeyPress,
    TXT_WindowActionDestructor,
};

txt_window_action_t *TXT_NewWindowAction(int key, char *label)
{
    txt_window_action_t *action;

    action = malloc(sizeof(txt_window_action_t));

    TXT_InitWidget(action, &txt_window_action_class);
    action->key = key;
    action->label = strdup(label);

    return action;
}

