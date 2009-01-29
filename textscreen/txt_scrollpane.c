// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// Copyright(C) 2009 Simon Howard
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
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
// 02111-1307, USA.
//

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "txt_scrollpane.h"
#include "txt_gui.h"
#include "txt_io.h"
#include "txt_main.h"

#define SCROLLBAR_VERTICAL   (1 << 0)
#define SCROLLBAR_HORIZONTAL (1 << 1)

static int FullWidth(txt_scrollpane_t *scrollpane)
{
    if (scrollpane->child != NULL)
    {
        return scrollpane->child->w;
    }
    else
    {
        return 0;
    }
}

static int FullHeight(txt_scrollpane_t *scrollpane)
{
    if (scrollpane->child != NULL)
    {
        return scrollpane->child->h;
    }
    else
    {
        return 0;
    }
}

// Calculate which scroll bars the pane needs.

static int NeedsScrollbars(txt_scrollpane_t *scrollpane)
{
    int result;

    result = 0;

    if (FullWidth(scrollpane) > scrollpane->w)
    {
        result |= SCROLLBAR_HORIZONTAL;
    }
    if (FullHeight(scrollpane) > scrollpane->h)
    {
        result |= SCROLLBAR_VERTICAL;
    }

    return result;
}

// If a scrollbar isn't needed, the scroll position is reset.

static void SanityCheckScrollbars(txt_scrollpane_t *scrollpane)
{
    int scrollbars;
    int max_x, max_y;

    scrollbars = NeedsScrollbars(scrollpane);

    if ((scrollbars & SCROLLBAR_HORIZONTAL) == 0)
    {
        scrollpane->x = 0;
    }
    if ((scrollbars & SCROLLBAR_VERTICAL) == 0)
    {
        scrollpane->y = 0;
    }

    max_x = FullWidth(scrollpane) - scrollpane->w;
    max_y = FullHeight(scrollpane) - scrollpane->h;

    if (scrollpane->x < 0)
    {
        scrollpane->x = 0;
    }
    else if (scrollpane->x > max_x)
    {
        scrollpane->x = max_x;
    }

    if (scrollpane->y < 0)
    {
        scrollpane->y = 0;
    }
    else if (scrollpane->y > max_y)
    {
        scrollpane->y = max_y;
    }
}

static void TXT_ScrollPaneSizeCalc(TXT_UNCAST_ARG(scrollpane))
{
    TXT_CAST_ARG(txt_scrollpane_t, scrollpane);
    int scrollbars;

    if (scrollpane->child != NULL)
    {
        TXT_CalcWidgetSize(scrollpane->child);
    }

    // Expand as necessary (to ensure that no scrollbars are needed)?

    if (scrollpane->expand_w)
    {
        scrollpane->w = FullWidth(scrollpane);
    }
    if (scrollpane->expand_h)
    {
        scrollpane->h = FullWidth(scrollpane);
    }

    scrollpane->widget.w = scrollpane->w;
    scrollpane->widget.h = scrollpane->h;

    // If we have scroll bars, we need to expand slightly to
    // accomodate them. Eg. if we have a vertical scrollbar, we
    // need to be an extra character wide.

    scrollbars = NeedsScrollbars(scrollpane);

    if (scrollbars & SCROLLBAR_HORIZONTAL)
    {
        ++scrollpane->widget.h;
    }
    if (scrollbars & SCROLLBAR_VERTICAL)
    {
        ++scrollpane->widget.w;
    }
}

static void TXT_ScrollPaneDrawer(TXT_UNCAST_ARG(scrollpane), int selected)
{
    TXT_CAST_ARG(txt_scrollpane_t, scrollpane);
    int x1, y1, x2, y2;
    int scrollbars;

    // We set a clipping area of the scroll pane.

    x1 = scrollpane->widget.x,
    y1 = scrollpane->widget.y,
    x2 = x1 + scrollpane->w,
    y2 = y1 + scrollpane->h;

    scrollbars = NeedsScrollbars(scrollpane);

    if (scrollbars & SCROLLBAR_HORIZONTAL)
    {
        TXT_DrawHorizScrollbar(x1,
                               y1 + scrollpane->h,
                               scrollpane->w,
                               scrollpane->x,
                               FullWidth(scrollpane) - scrollpane->w);
    }

    if (scrollbars & SCROLLBAR_VERTICAL)
    {
        TXT_DrawVertScrollbar(x1 + scrollpane->w,
                              y1,
                              scrollpane->h,
                              scrollpane->y,
                              FullHeight(scrollpane) - scrollpane->h);
    }

    TXT_PushClipArea(x1, x2, y1, y2);

    // Draw the child widget

    if (scrollpane->child != NULL)
    {
        TXT_DrawWidget(scrollpane->child, selected);
    }

    // Restore old clipping area.

    TXT_PopClipArea();
}

static void TXT_ScrollPaneDestructor(TXT_UNCAST_ARG(scrollpane))
{
    TXT_CAST_ARG(txt_scrollpane_t, scrollpane);

    if (scrollpane->child != NULL)
    {
        TXT_DestroyWidget(scrollpane->child);
    }
}

static int TXT_ScrollPaneKeyPress(TXT_UNCAST_ARG(scrollpane), int key)
{
    TXT_CAST_ARG(txt_scrollpane_t, scrollpane);

    if (scrollpane->child != NULL)
    {
        return TXT_WidgetKeyPress(scrollpane->child, key);
    }
    else
    {
        return 0;
    }
}

static void TXT_ScrollPaneMousePress(TXT_UNCAST_ARG(scrollpane),
                                     int x, int y, int b)
{
    TXT_CAST_ARG(txt_scrollpane_t, scrollpane);
    int scrollbars;
    int rel_x, rel_y;

    scrollbars = NeedsScrollbars(scrollpane);

    rel_x = x - scrollpane->widget.x;
    rel_y = y - scrollpane->widget.y;

    // Click on the horizontal scrollbar?
    if ((scrollbars & SCROLLBAR_HORIZONTAL) && rel_y == scrollpane->h)
    {
        if (rel_x == 0)
        {
            --scrollpane->x;
        }
        else if (rel_x == scrollpane->w - 1)
        {
            ++scrollpane->x;
        }
        else
        {
            int range = FullWidth(scrollpane) - scrollpane->w;

            scrollpane->x = ((rel_x - 1) * range) / (scrollpane->w - 3);
        }

        return;
    }

    // Click on the horizontal scrollbar?
    if ((scrollbars & SCROLLBAR_VERTICAL) && rel_x == scrollpane->w)
    {
        if (rel_y == 0)
        {
            --scrollpane->y;
        }
        else if (rel_y == scrollpane->h - 1)
        {
            ++scrollpane->y;
        }
        else
        {
            int range = FullHeight(scrollpane) - scrollpane->h;

            scrollpane->y = ((rel_y - 1) * range) / (scrollpane->h - 3);
        }

        return;
    }

    if (scrollpane->child != NULL)
    {
        TXT_WidgetMousePress(scrollpane->child,
                             x - scrollpane->x,
                             y - scrollpane->y,
                             b);
    }

}

static void TXT_ScrollPaneLayout(TXT_UNCAST_ARG(scrollpane))
{
    TXT_CAST_ARG(txt_scrollpane_t, scrollpane);

    SanityCheckScrollbars(scrollpane);

    // The child widget takes the same position as the scroll pane
    // itself, but is offset by the scroll position.

    if (scrollpane->child != NULL)
    {
        scrollpane->child->x = scrollpane->widget.x - scrollpane->x;
        scrollpane->child->y = scrollpane->widget.y - scrollpane->y;
    }
}

txt_widget_class_t txt_scrollpane_class =
{
    TXT_ScrollPaneSizeCalc,
    TXT_ScrollPaneDrawer,
    TXT_ScrollPaneKeyPress,
    TXT_ScrollPaneDestructor,
    TXT_ScrollPaneMousePress,
    TXT_ScrollPaneLayout,
};

txt_scrollpane_t *TXT_NewScrollPane(int w, int h, TXT_UNCAST_ARG(target))
{
    TXT_CAST_ARG(txt_widget_t, target);
    txt_scrollpane_t *scrollpane;

    scrollpane = malloc(sizeof(txt_scrollpane_t));
    TXT_InitWidget(scrollpane, &txt_scrollpane_class);
    scrollpane->w = w;
    scrollpane->h = h;
    scrollpane->x = 0;
    scrollpane->y = 0;
    scrollpane->child = target;
    scrollpane->expand_w = w <= 0;
    scrollpane->expand_h = h <= 0;

    return scrollpane;
}

