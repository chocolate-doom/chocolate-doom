#include <stdlib.h>
#include <string.h>

#include "txt_widget.h"

typedef struct
{
    char *signal_name;
    TxtWidgetSignalFunc func;
    void *user_data;
} txt_callback_t;

struct txt_callback_table_s
{
    txt_callback_t *callbacks;
    int num_callbacks;
};

txt_callback_table_t *TXT_NewCallbackTable(void)
{
    txt_callback_table_t *table;

    table = malloc(sizeof(txt_callback_table_t));
    table->callbacks = NULL;
    table->num_callbacks = 0;

    return table;
}

void TXT_DestroyCallbackTable(txt_callback_table_t *table)
{
    int i;

    for (i=0; i<table->num_callbacks; ++i)
    {
        free(table->callbacks[i].signal_name);
    }
    
    free(table->callbacks);
    free(table);
}

void TXT_InitWidget(void *uncast_widget, txt_widget_class_t *widget_class)
{
    txt_widget_t *widget = (txt_widget_t *) uncast_widget;

    widget->widget_class = widget_class;
    widget->callback_table = TXT_NewCallbackTable();

    // Default values: visible and selectable

    widget->selectable = 1;
    widget->visible = 1;
}

void TXT_SignalConnect(txt_widget_t *widget, 
                       char *signal_name,
                       TxtWidgetSignalFunc func, 
                       void *user_data)
{
    txt_callback_table_t *table;
    txt_callback_t *callback;
    int i;

    table = widget->callback_table;

    for (i=0; i<table->num_callbacks; ++i)
    {
        if (!strcmp(signal_name, table->callbacks[i].signal_name))
        {
            // Replace existing signal

            table->callbacks[i].func = func;
            table->callbacks[i].user_data = user_data;
            break;
        }
    }

    // Add a new callback to the table

    table->callbacks 
            = realloc(table->callbacks,
                      sizeof(txt_callback_t) * (table->num_callbacks + 1));
    callback = &table->callbacks[table->num_callbacks];
    ++table->num_callbacks;

    callback->signal_name = strdup(signal_name);
    callback->func = func;
    callback->user_data = user_data;
}

void TXT_EmitSignal(txt_widget_t *widget, char *signal_name)
{
    txt_callback_table_t *table;
    int i;

    table = widget->callback_table;

    for (i=0; i<table->num_callbacks; ++i)
    {
        if (!strcmp(table->callbacks[i].signal_name, signal_name))
        {
            table->callbacks[i].func(widget, table->callbacks[i].user_data);
            break;
        }
    }
}

void TXT_CalcWidgetSize(txt_widget_t *widget, int *w, int *h)
{
    return widget->widget_class->size_calc(widget, w, h);
}

void TXT_DrawWidget(txt_widget_t *widget, int w, int selected)
{
    widget->widget_class->drawer(widget, w, selected);
}

void TXT_DestroyWidget(txt_widget_t *widget)
{
    widget->widget_class->destructor(widget);
    TXT_DestroyCallbackTable(widget->callback_table);
    free(widget);
}

int TXT_WidgetKeyPress(txt_widget_t *widget, int key)
{
    if (widget->widget_class->key_press != NULL)
    {
        return widget->widget_class->key_press(widget, key);
    }

    return 0;
}


