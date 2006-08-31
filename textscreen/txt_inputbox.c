
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "doomkeys.h"

#include "txt_inputbox.h"
#include "txt_io.h"
#include "txt_main.h"
#include "txt_window.h"

static void SetBufferFromValue(txt_inputbox_t *inputbox);

static void TXT_InputBoxSizeCalc(TXT_UNCAST_ARG(inputbox))
{
    TXT_CAST_ARG(txt_inputbox_t, inputbox);

    // Enough space for the box + cursor

    inputbox->widget.w = inputbox->size + 1;
    inputbox->widget.h = 1;
}

static void TXT_InputBoxDrawer(TXT_UNCAST_ARG(inputbox), int selected)
{
    TXT_CAST_ARG(txt_inputbox_t, inputbox);
    int i;
    int chars;
    int w;

    w = inputbox->widget.w;

    // Select the background colour based on whether we are currently
    // editing, and if not, whether the widget is selected.

    if (inputbox->editing && selected)
    {
        TXT_BGColor(TXT_COLOR_BLACK, 0);
    }
    else if (selected)
    {
        TXT_BGColor(TXT_COLOR_GREY, 0);
    }
    else
    {
        // Not even selected

        TXT_BGColor(TXT_COLOR_BLUE, 0);
    }

    TXT_FGColor(TXT_COLOR_BRIGHT_WHITE);

    if (!inputbox->editing)
    {
        // If not editing, use the current value from inputbox->value.
 
        SetBufferFromValue(inputbox);
    }
    
    TXT_DrawString(inputbox->buffer);

    chars = strlen(inputbox->buffer);

    if (chars < w && inputbox->editing && selected)
    {
        TXT_BGColor(TXT_COLOR_BLACK, 1);
        TXT_DrawString("_");
        ++chars;
    }
    
    for (i=chars; i < w; ++i)
    {
        TXT_DrawString(" ");
    }
}

static void TXT_InputBoxDestructor(TXT_UNCAST_ARG(inputbox))
{
    TXT_CAST_ARG(txt_inputbox_t, inputbox);

    free(inputbox->buffer);
}

static void Backspace(txt_inputbox_t *inputbox)
{
    if (strlen(inputbox->buffer) > 0)
    {
        inputbox->buffer[strlen(inputbox->buffer) - 1] = '\0';
    }
}

static void AddCharacter(txt_inputbox_t *inputbox, int key)
{
    if (strlen(inputbox->buffer) < inputbox->size)
    {
        // Add character to the buffer

        inputbox->buffer[strlen(inputbox->buffer) + 1] = '\0';
        inputbox->buffer[strlen(inputbox->buffer)] = key;
    }
}

static int TXT_InputBoxKeyPress(TXT_UNCAST_ARG(inputbox), int key)
{
    TXT_CAST_ARG(txt_inputbox_t, inputbox);

    if (!inputbox->editing)
    {
        if (key == KEY_ENTER)
        {
            SetBufferFromValue(inputbox);
            inputbox->editing = 1;
            return 1;
        }

        return 0;
    }

    if (key == KEY_ENTER)
    {
        free(*((char **)inputbox->value));
        *((char **) inputbox->value) = strdup(inputbox->buffer);

        inputbox->editing = 0;
    }

    if (key == KEY_ESCAPE)
    {
        inputbox->editing = 0;
    }

    if (isprint(key))
    {
        // Add character to the buffer

        AddCharacter(inputbox, key);
    }

    if (key == KEY_BACKSPACE)
    {
        Backspace(inputbox);
    }
    
    return 1;
}

static int TXT_IntInputBoxKeyPress(TXT_UNCAST_ARG(inputbox), int key)
{
    TXT_CAST_ARG(txt_inputbox_t, inputbox);

    if (!inputbox->editing)
    {
        if (key == KEY_ENTER)
        {
            SetBufferFromValue(inputbox);
            inputbox->editing = 1;
            return 1;
        }

        return 0;
    }

    if (key == KEY_ENTER)
    {
        *((int *) inputbox->value) = atoi(inputbox->buffer);

        inputbox->editing = 0;
    }

    if (key == KEY_ESCAPE)
    {
        inputbox->editing = 0;
    }

    if (isdigit(key))
    {
        // Add character to the buffer

        AddCharacter(inputbox, key);
    }

    if (key == KEY_BACKSPACE)
    {
        Backspace(inputbox);
    }
    
    return 1;
}

static void TXT_InputBoxMousePress(TXT_UNCAST_ARG(inputbox),
                                   int x, int y, int b)
{
    TXT_CAST_ARG(txt_inputbox_t, inputbox);

    if (b == TXT_MOUSE_LEFT)
    {
        // Make mouse clicks start editing the box

        if (!inputbox->editing)
        {
            // Send a simulated keypress to start editing

            TXT_WidgetKeyPress(inputbox, KEY_ENTER);
        }
    }
}

txt_widget_class_t txt_inputbox_class =
{
    TXT_InputBoxSizeCalc,
    TXT_InputBoxDrawer,
    TXT_InputBoxKeyPress,
    TXT_InputBoxDestructor,
    TXT_InputBoxMousePress,
};

txt_widget_class_t txt_int_inputbox_class =
{
    TXT_InputBoxSizeCalc,
    TXT_InputBoxDrawer,
    TXT_IntInputBoxKeyPress,
    TXT_InputBoxDestructor,
    TXT_InputBoxMousePress,
};

static void SetBufferFromValue(txt_inputbox_t *inputbox)
{
    if (inputbox->widget.widget_class == &txt_inputbox_class)
    {
        char **value = (char **) inputbox->value;

        if (*value != NULL)
        {
            strncpy(inputbox->buffer, *value, inputbox->size);
            inputbox->buffer[inputbox->size] = '\0';
        }
        else
        {
            strcpy(inputbox->buffer, "");
        }
    }
    else if (inputbox->widget.widget_class == &txt_int_inputbox_class)
    {
        int *value = (int *) inputbox->value;
        sprintf(inputbox->buffer, "%i", *value);
    }
}

txt_inputbox_t *TXT_NewInputBox(char **value, int size)
{
    txt_inputbox_t *inputbox;

    inputbox = malloc(sizeof(txt_inputbox_t));

    TXT_InitWidget(inputbox, &txt_inputbox_class);
    inputbox->value = value;
    inputbox->size = size;
    inputbox->buffer = malloc(size + 1);
    inputbox->editing = 0;

    return inputbox;
}

txt_inputbox_t *TXT_NewIntInputBox(int *value, int size)
{
    txt_inputbox_t *inputbox;

    inputbox = malloc(sizeof(txt_inputbox_t));

    TXT_InitWidget(inputbox, &txt_int_inputbox_class);
    inputbox->value = value;
    inputbox->size = size;
    inputbox->buffer = malloc(15);
    inputbox->editing = 0;
    return inputbox;
}

