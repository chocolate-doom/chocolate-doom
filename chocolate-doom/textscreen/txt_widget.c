#include <stdlib.h>

#include "txt_widget.h"

int TXT_WidgetWidth(txt_widget_t *widget)
{
    return widget->widget_class->size_calc(widget);
}

void TXT_DrawWidget(txt_widget_t *widget, int w, int selected)
{
    widget->widget_class->drawer(widget, w, selected);
}

void TXT_DestroyWidget(txt_widget_t *widget)
{
    widget->widget_class->destructor(widget);
}

void TXT_WidgetKeyPress(txt_widget_t *widget, int key)
{
    if (widget->widget_class->key_press != NULL)
    {
        widget->widget_class->key_press(widget, key);
    }
}


