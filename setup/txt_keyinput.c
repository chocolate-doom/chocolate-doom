
#include <stdlib.h>
#include <string.h>

#include "doomkeys.h"

#include "txt_keyinput.h"
#include "txt_io.h"
#include "txt_label.h"
#include "txt_window.h"

#define KEY_INPUT_WIDTH 10

static int KeyPressCallback(txt_window_t *window, int key, 
                            TXT_UNCAST_ARG(key_input))
{
    TXT_CAST_ARG(txt_key_input_t, key_input);

    if (key != KEY_ESCAPE)
    {
        // Got the key press.  Save to the variable and close the window.

        *key_input->variable = key;
        TXT_EmitSignal(key_input, "set");
        TXT_CloseWindow(window);
        return 1;
    }
    else
    {
        return 0;
    }
}

static void OpenPromptWindow(txt_key_input_t *key_input)
{
    txt_window_t *window;
    txt_label_t *label;

    window = TXT_NewWindow(NULL);
    TXT_SetWindowAction(window, TXT_HORIZ_LEFT, NULL);
    TXT_SetWindowAction(window, TXT_HORIZ_CENTER, 
                        TXT_NewWindowAbortAction(window));
    TXT_SetWindowAction(window, TXT_HORIZ_RIGHT, NULL);
    
    label = TXT_NewLabel("Press the new key...");

    TXT_AddWidget(window, label);
    TXT_SetWidgetAlign(label, TXT_HORIZ_CENTER);

    TXT_SetKeyListener(window, KeyPressCallback, key_input);
}

static void TXT_KeyInputSizeCalc(TXT_UNCAST_ARG(key_input))
{
    TXT_CAST_ARG(txt_key_input_t, key_input);

    // All keyinputs are the same size.

    key_input->widget.w = KEY_INPUT_WIDTH;
    key_input->widget.h = 1;
}


static void TXT_KeyInputDrawer(TXT_UNCAST_ARG(key_input), int selected)
{
    TXT_CAST_ARG(txt_key_input_t, key_input);
    char buf[20];
    int i;

    if (*key_input->variable == 0)
    {
        strcpy(buf, "");
    }
    else
    {
        TXT_GetKeyDescription(*key_input->variable, buf);
    }

    if (selected)
    {
        TXT_BGColor(TXT_COLOR_GREY, 0);
    }
    else
    {
        TXT_BGColor(TXT_COLOR_BLUE, 0);
    }

    TXT_FGColor(TXT_COLOR_BRIGHT_WHITE);
    
    TXT_DrawString(buf);
    
    for (i=strlen(buf); i<KEY_INPUT_WIDTH; ++i)
    {
        TXT_DrawString(" ");
    }
}

static void TXT_KeyInputDestructor(TXT_UNCAST_ARG(key_input))
{
    TXT_CAST_ARG(txt_key_input_t, key_input);
}

static int TXT_KeyInputKeyPress(TXT_UNCAST_ARG(key_input), int key)
{
    TXT_CAST_ARG(txt_key_input_t, key_input);

    if (key == KEY_ENTER)
    {
        // Open a window to prompt for the new key press

        OpenPromptWindow(key_input);

        return 1;
    }

    return 0;
}

static void TXT_KeyInputMousePress(TXT_UNCAST_ARG(widget), int x, int y, int b)
{
    TXT_CAST_ARG(txt_key_input_t, widget);
            
    // Clicking is like pressing enter

    if (b == TXT_MOUSE_LEFT)
    {
        TXT_KeyInputKeyPress(widget, KEY_ENTER);
    }
}

txt_widget_class_t txt_key_input_class =
{
    TXT_KeyInputSizeCalc,
    TXT_KeyInputDrawer,
    TXT_KeyInputKeyPress,
    TXT_KeyInputDestructor,
    TXT_KeyInputMousePress,
};

txt_key_input_t *TXT_NewKeyInput(int *variable)
{
    txt_key_input_t *key_input;

    key_input = malloc(sizeof(txt_key_input_t));

    TXT_InitWidget(key_input, &txt_key_input_class);
    key_input->variable = variable;

    return key_input;
}

