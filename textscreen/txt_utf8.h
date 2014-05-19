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

#ifndef TXT_UTF8_H
#define TXT_UTF8_H

#include <stdarg.h>

char *TXT_EncodeUTF8(char *p, unsigned int c);
unsigned int TXT_DecodeUTF8(const char **ptr);
unsigned int TXT_UTF8_Strlen(const char *s);
char *TXT_UTF8_SkipChars(const char *s, unsigned int n);

#endif /* #ifndef TXT_UTF8_H */

