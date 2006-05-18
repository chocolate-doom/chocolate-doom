
#include <string.h>

#include "txt_separator.h"
#include "txt_io.h"
#include "txt_main.h"
#include "txt_widget.h"
#include "txt_window.h"

static int TXT_SeparatorSizeCalc(txt_widget_t *widget)
{
    txt_separator_t *separator = (txt_separator_t *) widget;

    if (separator->label != NULL)
    {
        // Minimum width is the string length + two spaces for padding

        return strlen(separator->label) + 2;
    }
    else
    {
        return 0;
    }
}

static void TXT_SeparatorDrawer(txt_widget_t *widget, int w, int selected)
{
    txt_separator_t *separator = (txt_separator_t *) widget;
    int i;
    int x, y;

    TXT_GetXY(&x, &y);

    // Draw separator.  Go back one character and draw two extra
    // to overlap the window borders.

    TXT_DrawSeparator(x-1, y, w + 2);
    
    if (separator->label != NULL)
    {
        TXT_GotoXY(x, y);

        TXT_BGColor(TXT_COLOR_BLUE, 0);
        TXT_FGColor(TXT_COLOR_BRIGHT_GREEN);
        TXT_PutChar(' ');

        for (i=0; i<strlen(separator->label); ++i)
        {
            TXT_PutChar(separator->label[i]);
        }
    
        TXT_PutChar(' ');
    }
}

static void TXT_SeparatorDestructor(txt_widget_t *widget)
{
    txt_separator_t *separator = (txt_separator_t *) widget;

    free(separator->label);
}

txt_widget_class_t txt_separator_class =
{
    TXT_SeparatorSizeCalc,
    TXT_SeparatorDrawer,
    TXT_SeparatorDestructor,
};

txt_separator_t *TXT_NewSeparator(char *label)
{
    txt_separator_t *separator;

    separator = malloc(sizeof(txt_separator_t));

    separator->widget.widget_class = &txt_separator_class;

    if (label != NULL)
    {
        separator->label = strdup(label);
    }
    else
    {
        separator->label = NULL;
    }

    return separator;
}

