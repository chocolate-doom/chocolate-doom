// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// Copyright(C) 2012 Simon Howard
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

#ifndef TXT_UTF8_H
#define TXT_UTF8_H

char *TXT_EncodeUTF8(char *p, unsigned int c);
unsigned int TXT_DecodeUTF8(const char **ptr);
unsigned int TXT_UTF8_Strlen(const char *s);
char *TXT_UTF8_SkipChars(const char *s, unsigned int n);

#endif /* #ifndef TXT_UTF8_H */

