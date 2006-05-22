
#include <string.h>

#include "txt_label.h"
#include "txt_io.h"
#include "txt_main.h"
#include "txt_window.h"

static void TXT_LabelSizeCalc(TXT_UNCAST_ARG(label), int *w, int *h)
{
    TXT_CAST_ARG(txt_label_t, label);

    *w = label->w;
    *h = label->h;
}

static void TXT_LabelDrawer(TXT_UNCAST_ARG(label), int w, int selected)
{
    TXT_CAST_ARG(txt_label_t, label);
    int i;
    int origin_x, origin_y;

    TXT_BGColor(TXT_COLOR_BLUE, 0);
    TXT_FGColor(TXT_COLOR_BRIGHT_WHITE);

    TXT_GetXY(&origin_x, &origin_y);

    for (i=0; i<label->h; ++i)
    {
        TXT_GotoXY(origin_x, origin_y + i);
        TXT_DrawString(label->lines[i]);
    }
}

static void TXT_LabelDestructor(TXT_UNCAST_ARG(label))
{
    TXT_CAST_ARG(txt_label_t, label);

    free(label->label);
    free(label->lines);
}

txt_widget_class_t txt_label_class =
{
    TXT_LabelSizeCalc,
    TXT_LabelDrawer,
    NULL,
    TXT_LabelDestructor,
};

static void TXT_SplitLabel(txt_label_t *label)
{
    char *p;
    int y;

    // Work out how many lines in this label

    label->h = 1;

    for (p = label->label; *p != '\0'; ++p)
    {
        if (*p == '\n')
        {
            ++label->h;
        }
    }

    // Split into lines

    label->lines = malloc(sizeof(char *) * label->h);
    label->lines[0] = label->label;
    y = 1;
    
    for (p = label->label; *p != '\0'; ++p)
    {
        if (*p == '\n')
        {
            label->lines[y] = p + 1;
            *p = '\0';
            ++y;
        }
    }

    label->w = 0;

    for (y=0; y<label->h; ++y)
    {
        if (strlen(label->lines[y]) > label->w)
            label->w = strlen(label->lines[y]);
    }
}

txt_label_t *TXT_NewLabel(char *text)
{
    txt_label_t *label;

    label = malloc(sizeof(txt_label_t));

    TXT_InitWidget(label, &txt_label_class);
    label->widget.selectable = 0;
    label->label = strdup(text);

    TXT_SplitLabel(label);

    return label;
}

