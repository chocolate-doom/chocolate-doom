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

#include "SDL.h"

#include <stdio.h>

#include "i_log.h"


// Prints to stdout/stderr, depending on the message priority
void DefaultLogOutputFunction(void *userdata, int category,
                             SDL_LogPriority priority, const char *message)
{
    if (priority <= SDL_LOG_PRIORITY_INFO)
    {
        fprintf(stdout, "%s", message);
    }
    else
    {
        fprintf(stderr, "%s", message);
    }
}


void I_InitLog(void)
{
    SDL_LogSetOutputFunction(&DefaultLogOutputFunction, NULL);
}

void I_LogInfo(const char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    I_LogInfoV(fmt, args);
    va_end(args);
}

void I_LogInfoV(const char *fmt, va_list args)
{
    SDL_LogMessageV(
            SDL_LOG_CATEGORY_APPLICATION,
            SDL_LOG_PRIORITY_INFO,
            fmt,
            args);
}

void I_LogError(const char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    I_LogErrorV(fmt, args);
    va_end(args);
}

void I_LogErrorV(const char *fmt, va_list args)
{
    SDL_LogMessageV(
            SDL_LOG_CATEGORY_APPLICATION,
            SDL_LOG_PRIORITY_ERROR,
            fmt,
            args);
}
