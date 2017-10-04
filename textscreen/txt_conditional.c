//
// Copyright(C) 2016 Simon Howard
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//

#include <stdlib.h>
#include <string.h>

#include "txt_conditional.h"
#include "txt_strut.h"

struct txt_conditional_s
{
    txt_widget_t widget;
    int *var;
    int expected_value;
    txt_widget_t *child;
};

static int ConditionTrue(txt_conditional_t *conditional)
{
    return *conditional->var == conditional->expected_value;
}

static int TXT_CondSelectable(TXT_UNCAST_ARG(conditional))
{
    TXT_CAST_ARG(txt_conditional_t, conditional);
    return ConditionTrue(conditional)
        && TXT_SelectableWidget(conditional->child);
}

static void TXT_CondSizeCalc(TXT_UNCAST_ARG(conditional))
{
    TXT_CAST_ARG(txt_conditional_t, conditional);

    if (!ConditionTrue(conditional))
    {
        conditional->widget.w = 0;
        conditional->widget.h = 0;
    }
    else
    {
        TXT_CalcWidgetSize(conditional->child);
        conditional->widget.w = conditional->child->w;
        conditional->widget.h = conditional->child->h;
    }
}

static void TXT_CondLayout(TXT_UNCAST_ARG(conditional))
{
    TXT_CAST_ARG(txt_conditional_t, conditional);

    if (ConditionTrue(conditional))
    {
        conditional->child->x = conditional->widget.x;
        conditional->child->y = conditional->widget.y;
        TXT_LayoutWidget(conditional->child);
    }
}

static void TXT_CondDrawer(TXT_UNCAST_ARG(conditional))
{
    TXT_CAST_ARG(txt_conditional_t, conditional);

    if (ConditionTrue(conditional))
    {
        TXT_DrawWidget(conditional->child);
    }
}

static void TXT_CondDestructor(TXT_UNCAST_ARG(conditional))
{
    TXT_CAST_ARG(txt_conditional_t, conditional);
    TXT_DestroyWidget(conditional->child);
}

static void TXT_CondFocused(TXT_UNCAST_ARG(conditional), int focused)
{
    TXT_CAST_ARG(txt_conditional_t, conditional);

    if (ConditionTrue(conditional))
    {
        TXT_SetWidgetFocus(conditional->child, focused);
    }
}

static int TXT_CondKeyPress(TXT_UNCAST_ARG(conditional), int key)
{
    TXT_CAST_ARG(txt_conditional_t, conditional);

    if (ConditionTrue(conditional))
    {
        return TXT_WidgetKeyPress(conditional->child, key);
    }

    return 0;
}

static void TXT_CondMousePress(TXT_UNCAST_ARG(conditional),
                               int x, int y, int b)
{
    TXT_CAST_ARG(txt_conditional_t, conditional);

    if (ConditionTrue(conditional))
    {
        TXT_WidgetMousePress(conditional->child, x, y, b);
    }
}

txt_widget_class_t txt_conditional_class =
{
    TXT_CondSelectable,
    TXT_CondSizeCalc,
    TXT_CondDrawer,
    TXT_CondKeyPress,
    TXT_CondDestructor,
    TXT_CondMousePress,
    TXT_CondLayout,
    TXT_CondFocused,
};

txt_conditional_t *TXT_NewConditional(int *var, int expected_value,
                                      TXT_UNCAST_ARG(child))
{
    TXT_CAST_ARG(txt_widget_t, child);
    txt_conditional_t *conditional;

    conditional = malloc(sizeof(txt_conditional_t));

    TXT_InitWidget(conditional, &txt_conditional_class);
    conditional->var = var;
    conditional->expected_value = expected_value;
    conditional->child = child;

    child->parent = &conditional->widget;

    return conditional;
}

// "Static" conditional that returns an empty strut if the given static
// value is false. Kind of like a conditional but we only evaluate it at
// creation time.
txt_widget_t *TXT_If(int conditional, TXT_UNCAST_ARG(child))
{
    TXT_CAST_ARG(txt_widget_t, child);

    if (conditional)
    {
        return child;
    }
    else
    {
        txt_strut_t *nullwidget;
        TXT_DestroyWidget(child);
        nullwidget = TXT_NewStrut(0, 0);
        return &nullwidget->widget;
    }
}

