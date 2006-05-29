
#include <stdlib.h>
#include <string.h>

#include "doomkeys.h"

#include "txt_strut.h"
#include "txt_io.h"
#include "txt_main.h"
#include "txt_window.h"

static void TXT_StrutSizeCalc(TXT_UNCAST_ARG(strut), int *w, int *h)
{
    TXT_CAST_ARG(txt_strut_t, strut);

    // Minimum width is the string length + two spaces for padding

    *w = strut->width;
    *h = strut->height;
}

static void TXT_StrutDrawer(TXT_UNCAST_ARG(strut), int w, int selected)
{
    // Nothing is drawn for a strut.
}

static void TXT_StrutDestructor(TXT_UNCAST_ARG(strut))
{
}

static int TXT_StrutKeyPress(TXT_UNCAST_ARG(strut), int key)
{
}

txt_widget_class_t txt_strut_class =
{
    TXT_StrutSizeCalc,
    TXT_StrutDrawer,
    TXT_StrutKeyPress,
    TXT_StrutDestructor,
};

txt_strut_t *TXT_NewStrut(int width, int height)
{
    txt_strut_t *strut;

    strut = malloc(sizeof(txt_strut_t));

    TXT_InitWidget(strut, &txt_strut_class);
    strut->widget.selectable = 0;
    strut->width = width;
    strut->height = height;

    return strut;
}

