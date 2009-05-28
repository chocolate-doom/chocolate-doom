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

// Detect an OPL chip.

int DetectOPL(void)
{
    WriteReg(OPL_REG_TIMER_CTRL, 0x60);
    WriteReg(OPL_REG_TIMER_CTRL, 0x80);
    int val1 = OPL_ReadPort(OPL_REGISTER_PORT) & 0xe0;
    WriteReg(OPL_REG_TIMER1, 0xff);
    WriteReg(OPL_REG_TIMER_CTRL, 0x21);
    SDL_Delay(50);
    int val2 = OPL_ReadPort(OPL_REGISTER_PORT) & 0xe0;
    WriteReg(OPL_REG_TIMER_CTRL, 0x60);
    WriteReg(OPL_REG_TIMER_CTRL, 0x80);

    return val1 == 0 && val2 == 0xc0;
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

    if (!DetectOPL())
    {
        fprintf(stderr, "Adlib not detected\n");
        exit(-1);
    }
}

void Shutdown(void)
{
    OPL_Shutdown();
}

void PlayFile(char *filename)
{
    FILE *stream;
    char buf[8];

    stream = fopen(filename, "rb");

    if (fread(buf, 1, 8, stream) < 8)
    {
        fprintf(stderr, "failed to read raw OPL header\n");
        exit(-1);
    }

    if (strncmp(buf, HEADER_STRING, 8) != 0)
    {
        fprintf(stderr, "Raw OPL header not found\n");
        exit(-1);
    }

    fseek(stream, 28, SEEK_SET);

    while (!feof(stream))
    {
        int reg, val;

        reg = fgetc(stream);
        val = fgetc(stream);

        // Delay?

        if (reg == 0x00)
        {
            SDL_Delay(val);
        }
        else if (reg == 0x01)
        {
            val |= (fgetc(stream) << 8);
            SDL_Delay(val);
        }
        else
        {
            WriteReg(reg, val);
        }
    }

    fclose(stream);
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        printf("Usage: %s <filename>\n", argv[0]);
        exit(-1);
    }

    Init();
    ClearAllRegs();
    SDL_Delay(1000);

    PlayFile(argv[1]);

    ClearAllRegs();
    Shutdown();

    return 0;
}

