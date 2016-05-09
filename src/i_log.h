//
// Copyright(C) 2016 Adar Arnon
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
// DESCRIPTION:
//     Log interface, to avoid using printf() etc. in the code.
//

#ifndef __I_LOG__
#define __I_LOG__

#include <stdarg.h>

// Initialize the log
void I_InitLog(void);

// Write an info log message
void I_LogInfo(const char *fmt, ...);

// Write an info log message with vargs
void I_LogInfoV(const char *fmt, va_list args);

// Write an error log message
void I_LogError(const char *fmt, ...);

// Write an error log message with vargs
void I_LogErrorV(const char *fmt, va_list args);

#endif
