//
// Copyright(C) 2005-2014 Simon Howard
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
//
// Text mode emulation in SDL
//

#ifndef TXT_GUI_H
#define TXT_GUI_H

#define TXT_INACTIVE_WINDOW_BACKGROUND   TXT_COLOR_BLACK
#define TXT_ACTIVE_WINDOW_BACKGROUND     TXT_COLOR_BLUE
#define TXT_HOVER_BACKGROUND             TXT_COLOR_CYAN

void TXT_DrawDesktopBackground(const char *title);
void TXT_DrawWindowFrame(const char *title, int x, int y, int w, int h);
void TXT_DrawSeparator(int x, int y, int w);
void TXT_DrawCodePageString(const char *s);
void TXT_DrawString(const char *s);
int TXT_CanDrawCharacter(unsigned int c);

void TXT_DrawHorizScrollbar(int x, int y, int w, int cursor, int range);
void TXT_DrawVertScrollbar(int x, int y, int h, int cursor, int range);

void TXT_InitClipArea(void);
void TXT_PushClipArea(int x1, int x2, int y1, int y2);
void TXT_PopClipArea(void);

#endif /* #ifndef TXT_GUI_H */

