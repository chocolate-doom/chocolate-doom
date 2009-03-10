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
//     OPL interface.
//
//-----------------------------------------------------------------------------


#ifndef OPL_OPL_H
#define OPL_OPL_H

typedef enum
{
    OPL_REGISTER_PORT = 0,
    OPL_DATA_PORT = 1
} opl_port_t;

#define OPL_NUM_OPERATORS   21
#define OPL_NUM_VOICES      9

#define OPL_REG_TIMER1            0x02
#define OPL_REG_TIMER2            0x03
#define OPL_REG_TIMER_CTRL        0x04
#define OPL_REG_FM_MODE           0x08

// Operator registers (21 of each):

#define OPL_REGS_TREMOLO          0x20
#define OPL_REGS_LEVEL            0x40
#define OPL_REGS_ATTACK           0x60
#define OPL_REGS_SUSTAIN          0x80

// Voice registers (9 of each):

#define OPL_REGS_FREQ_1           0xA0
#define OPL_REGS_FREQ_2           0xB0

// Initialise the OPL subsystem.

int OPL_Init(unsigned int port_base);

// Shut down the OPL subsystem.

void OPL_Shutdown(void);

// Write to one of the OPL I/O ports:

void OPL_WritePort(opl_port_t port, unsigned int value);

// Read from one of the OPL I/O ports:

unsigned int OPL_ReadPort(opl_port_t port);

#endif

