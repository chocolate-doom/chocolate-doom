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
// DESCRIPTION:
//    PC speaker interface.
//

#ifndef PCSOUND_INTERNAL_H
#define PCSOUND_INTERNAL_H

#include "pcsound.h"

#ifdef HAVE_DEV_ISA_SPKRIO_H
#define HAVE_BSD_SPEAKER
#endif
#ifdef HAVE_DEV_SPEAKER_SPEAKER_H
#define HAVE_BSD_SPEAKER
#endif

#define PCSOUND_8253_FREQUENCY 1193280

typedef struct pcsound_driver_s pcsound_driver_t;
typedef int (*pcsound_init_func)(pcsound_callback_func callback);
typedef void (*pcsound_shutdown_func)(void);

struct pcsound_driver_s
{
    const char *name;
    pcsound_init_func init_func;
    pcsound_shutdown_func shutdown_func;
};

extern int pcsound_sample_rate;


#ifdef _WIN32
extern pcsound_driver_t pcsound_win32_driver;
#endif

#ifdef HAVE_BSD_SPEAKER
extern pcsound_driver_t pcsound_bsd_driver;
#endif

#ifdef HAVE_LINUX_KD_H
extern pcsound_driver_t pcsound_linux_driver;
#endif

extern pcsound_driver_t pcsound_sdl_driver;


#endif /* #ifndef PCSOUND_INTERNAL_H */

