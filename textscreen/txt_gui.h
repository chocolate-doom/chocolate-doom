// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// Copyright(C) 2005,2006 Simon Howard
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
//-----------------------------------------------------------------------------
//
// Text mode emulation in SDL
//
//-----------------------------------------------------------------------------

#ifndef TXT_GUI_H
#define TXT_GUI_H

#define TXT_WINDOW_BACKGROUND TXT_COLOR_BLUE
#define TXT_HOVER_BACKGROUND  TXT_COLOR_CYAN

void TXT_DrawDesktopBackground(const char *title);
void TXT_DrawWindowFrame(const char *title, int x, int y, int w, int h);
void TXT_DrawSeparator(int x, int y, int w);
void TXT_DrawString(const char *s);

void TXT_DrawHorizScrollbar(int x, int y, int w, int cursor, int range);
void TXT_DrawVertScrollbar(int x, int y, int h, int cursor, int range);


void TXT_InitClipArea(void);
void TXT_PushClipArea(int x1, int x2, int y1, int y2);
void TXT_PopClipArea(void);

#endif /* #ifndef TXT_GUI_H */

