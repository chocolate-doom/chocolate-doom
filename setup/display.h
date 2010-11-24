// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// Copyright(C) 2006 Simon Howard
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

#ifndef SETUP_DISPLAY_H 
#define SETUP_DISPLAY_H

extern int autoadjust_video_settings;
extern int aspect_ratio_correct;
extern int fullscreen;
extern int screen_width, screen_height;
extern int screen_bpp;
extern int startup_delay;
extern int show_endoom;
extern char *video_driver;

void ConfigDisplay(void);
void SetDisplayDriver(void);

#endif /* #ifndef SETUP_DISPLAY_H */

