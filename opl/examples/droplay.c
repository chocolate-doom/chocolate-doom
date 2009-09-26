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
// DESCRIPTION:
//     Demonstration program for OPL library to play back DRO
//     format files.
//
//-----------------------------------------------------------------------------


#include <stdio.h>
#include <stdlib.h>

#include "SDL.h"

#include "opl.h"

#define HEADER_STRING "DBRAWOPL"
#define ADLIB_PORT 0x388

void WriteReg(unsigned int reg, unsigned int val)
{
    int i;

    // This was recorded from an OPL2, but we are probably playing
    // back on an OPL3, so we need to enable the original OPL2
    // channels.  Doom does this already, but other games don't.

    if ((reg & 0xf0) == OPL_REGS_FEEDBACK)
    {
        val |= 0x30;
    }

    OPL_WritePort(OPL_REGISTER_PORT, reg);

    for (i=0; i<6; ++i)
    {
        OPL_ReadPort(OPL_REGISTER_PORT);
    }

    OPL_WritePort(OPL_DATA_PORT, val);

    for (i=0; i<35; ++i)
    {
        OPL_ReadPort(OPL_REGISTER_PORT);
    }
}

void ClearAllRegs(void)
{
    int i;

    for (i=0; i<=0xff; ++i)
    {
	WriteReg(i, 0x00);
    }
}

void Init(void)
{
    if (SDL_Init(SDL_INIT_TIMER) < 0)
    {
        fprintf(stderr, "Unable to initialise SDL timer\n");
        exit(-1);
    }

    if (!OPL_Init(ADLIB_PORT))
    {
        fprintf(stderr, "Unable to initialise OPL layer\n");
        exit(-1);
    }
}

void Shutdown(void)
{
    OPL_Shutdown();
}

struct timer_data
{
    int running;
    FILE *fstream;
};

void TimerCallback(void *data)
{
    struct timer_data *timer_data = data;
    int delay;

    if (!timer_data->running)
    {
        return;
    }

    // Read data until we must make a delay.

    for (;;)
    {
        int reg, val;

        // End of file?

        if (feof(timer_data->fstream))
        {
            timer_data->running = 0;
            return;
        }

        reg = fgetc(timer_data->fstream);
        val = fgetc(timer_data->fstream);

        // Register value of 0 or 1 indicates a delay.

        if (reg == 0x00)
        {
            delay = val;
            break;
        }
        else if (reg == 0x01)
        {
            val |= (fgetc(timer_data->fstream) << 8);
            delay = val;
            break;
        }
        else
        {
            WriteReg(reg, val);
        }
    }

    // Schedule the next timer callback.

    OPL_SetCallback(delay, TimerCallback, timer_data);
}

void PlayFile(char *filename)
{
    struct timer_data timer_data;
    int running;
    char buf[8];

    timer_data.fstream = fopen(filename, "rb");

    if (timer_data.fstream == NULL)
    {
        fprintf(stderr, "Failed to open %s\n", filename);
        exit(-1);
    }

    if (fread(buf, 1, 8, timer_data.fstream) < 8)
    {
        fprintf(stderr, "failed to read raw OPL header\n");
        exit(-1);
    }

    if (strncmp(buf, HEADER_STRING, 8) != 0)
    {
        fprintf(stderr, "Raw OPL header not found\n");
        exit(-1);
    }

    fseek(timer_data.fstream, 28, SEEK_SET);
    timer_data.running = 1;

    // Start callback loop sequence.

    OPL_SetCallback(0, TimerCallback, &timer_data);

    // Sleep until the playback finishes.

    do
    {
        OPL_Lock();
        running = timer_data.running;
        OPL_Unlock();

        SDL_Delay(100);
    } while (running);

    fclose(timer_data.fstream);
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        printf("Usage: %s <filename>\n", argv[0]);
        exit(-1);
    }

    Init();

    PlayFile(argv[1]);

    ClearAllRegs();
    Shutdown();

    return 0;
}

