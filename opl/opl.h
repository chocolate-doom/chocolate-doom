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
//     OPL interface.
//


#ifndef OPL_OPL_H
#define OPL_OPL_H

#include <inttypes.h>

typedef void (*opl_callback_t)(void *data);

// Result from OPL_Init(), indicating what type of OPL chip was detected,
// if any.
typedef enum
{
    OPL_INIT_NONE,
    OPL_INIT_OPL2,
    OPL_INIT_OPL3,
} opl_init_result_t;

typedef enum
{
    OPL_REGISTER_PORT = 0,
    OPL_DATA_PORT = 1,
    OPL_REGISTER_PORT_OPL3 = 2
} opl_port_t;

#define OPL_NUM_OPERATORS   21
#define OPL_NUM_VOICES      9

#define OPL_REG_WAVEFORM_ENABLE   0x01
#define OPL_REG_TIMER1            0x02
#define OPL_REG_TIMER2            0x03
#define OPL_REG_TIMER_CTRL        0x04
#define OPL_REG_FM_MODE           0x08
#define OPL_REG_NEW               0x105

// Operator registers (21 of each):

#define OPL_REGS_TREMOLO          0x20
#define OPL_REGS_LEVEL            0x40
#define OPL_REGS_ATTACK           0x60
#define OPL_REGS_SUSTAIN          0x80
#define OPL_REGS_WAVEFORM         0xE0

// Voice registers (9 of each):

#define OPL_REGS_FREQ_1           0xA0
#define OPL_REGS_FREQ_2           0xB0
#define OPL_REGS_FEEDBACK         0xC0

// Times

#define OPL_SECOND ((uint64_t) 1000 * 1000)
#define OPL_MS     ((uint64_t) 1000)
#define OPL_US     ((uint64_t) 1)

//
// Low-level functions.
//

// Initialize the OPL subsystem.

opl_init_result_t OPL_Init(unsigned int port_base);

// Shut down the OPL subsystem.

void OPL_Shutdown(void);

// Set the sample rate used for software emulation.

void OPL_SetSampleRate(unsigned int rate);

// Write to one of the OPL I/O ports:

void OPL_WritePort(opl_port_t port, unsigned int value);

// Read from one of the OPL I/O ports:

unsigned int OPL_ReadPort(opl_port_t port);

//
// Higher-level functions.
//

// Read the cuurrent status byte of the OPL chip.

unsigned int OPL_ReadStatus(void);

// Write to an OPL register.

void OPL_WriteRegister(int reg, int value);

// Perform a detection sequence to determine that an
// OPL chip is present.

opl_init_result_t OPL_Detect(void);

// Initialize all registers, performed on startup.

void OPL_InitRegisters(int opl3);

//
// Timer callback functions.
//

// Set a timer callback.  After the specified number of microseconds
// have elapsed, the callback will be invoked.

void OPL_SetCallback(uint64_t us, opl_callback_t callback, void *data);

// Adjust callback times by the specified factor. For example, a value of
// 0.5 will halve all remaining times.

void OPL_AdjustCallbacks(float factor);

// Clear all OPL callbacks that have been set.

void OPL_ClearCallbacks(void);

// Begin critical section, during which, OPL callbacks will not be
// invoked.

void OPL_Lock(void);

// End critical section.

void OPL_Unlock(void);

// Block until the specified number of microseconds have elapsed.

void OPL_Delay(uint64_t us);

// Pause the OPL callbacks.

void OPL_SetPaused(int paused);

#endif

