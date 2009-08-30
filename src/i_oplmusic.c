// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2005 Simon Howard
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
//	System interface for music.
//
//-----------------------------------------------------------------------------


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "doomdef.h"
#include "memio.h"
#include "mus2mid.h"

#include "deh_main.h"
#include "m_misc.h"
#include "s_sound.h"
#include "w_wad.h"
#include "z_zone.h"

#include "opl.h"
#include "midifile.h"

#define TEST

#define MAXMIDLENGTH (96 * 1024)
#define GENMIDI_NUM_INSTRS  128

#define GENMIDI_HEADER          "#OPL_II#"
#define GENMIDI_FLAG_FIXED      0x0000         /* fixed pitch */
#define GENMIDI_FLAG_2VOICE     0x0002         /* double voice (OPL3) */

typedef struct
{
    byte tremolo;
    byte attack;
    byte sustain;
    byte waveform;
    byte scale;
    byte level;
} PACKEDATTR genmidi_op_t;

typedef struct
{
    genmidi_op_t modulator;
    byte feedback;
    genmidi_op_t carrier;
    byte unused;
    short base_note_offset;
} PACKEDATTR genmidi_voice_t;

typedef struct
{
    unsigned short flags;
    byte fine_tuning;
    byte fixed_note;

    genmidi_voice_t opl2_voice;
    genmidi_voice_t opl3_voice;
} PACKEDATTR genmidi_instr_t;

typedef struct opl_voice_s opl_voice_t;

struct opl_voice_s
{
    // Index of this voice:
    int index;

    // The operators used by this voice:
    int op1, op2;

    // Currently-loaded instrument data
    genmidi_instr_t *current_instr;

    // Next in freelist
    opl_voice_t *next;
};

// Operators used by the different voices.

static const int voice_operators[2][OPL_NUM_VOICES] = {
    { 0x00, 0x01, 0x02, 0x08, 0x09, 0x0a, 0x10, 0x11, 0x12 },
    { 0x03, 0x04, 0x05, 0x0b, 0x0c, 0x0d, 0x13, 0x14, 0x15 }
};

static boolean music_initialised = false;

//static boolean musicpaused = false;
static int current_music_volume;

// GENMIDI lump instrument data:

static genmidi_instr_t *main_instrs;
static genmidi_instr_t *percussion_instrs;

// Voices:

static opl_voice_t voices[OPL_NUM_VOICES];
static opl_voice_t *voice_free_list;

// In the initialisation stage, register writes are spaced by reading
// from the register port (0).  After initialisation, spacing is
// peformed by reading from the data port instead.  I have no idea
// why.

static boolean init_stage_reg_writes = false;

// Configuration file variable, containing the port number for the
// adlib chip.

int snd_mport = 0x388;

static unsigned int GetStatus(void)
{
    return OPL_ReadPort(OPL_REGISTER_PORT);
}

// Write an OPL register value

static void WriteRegister(int reg, int value)
{
    int i;

    OPL_WritePort(OPL_REGISTER_PORT, reg);

    // For timing, read the register port six times after writing the
    // register number to cause the appropriate delay

    for (i=0; i<6; ++i)
    {
        // An oddity of the Doom OPL code: at startup initialisation,
        // the spacing here is performed by reading from the register
        // port; after initialisation, the data port is read, instead.

        if (init_stage_reg_writes)
        {
            OPL_ReadPort(OPL_REGISTER_PORT);
        }
        else
        {
            OPL_ReadPort(OPL_DATA_PORT);
        }
    }

    OPL_WritePort(OPL_DATA_PORT, value);

    // Read the register port 25 times after writing the value to
    // cause the appropriate delay

    for (i=0; i<24; ++i)
    {
        GetStatus();
    }
}

// Detect the presence of an OPL chip

static boolean DetectOPL(void)
{
    int result1, result2;
    int i;

    // Reset both timers:
    WriteRegister(OPL_REG_TIMER_CTRL, 0x60);

    // Enable interrupts:
    WriteRegister(OPL_REG_TIMER_CTRL, 0x80);

    // Read status
    result1 = GetStatus();

    // Set timer:
    WriteRegister(OPL_REG_TIMER1, 0xff);

    // Start timer 1:
    WriteRegister(OPL_REG_TIMER_CTRL, 0x21);

    // Wait for 80 microseconds
    // This is how Doom does it:

    for (i=0; i<200; ++i)
    {
        GetStatus();
    }

    OPL_Delay(1);

    // Read status
    result2 = GetStatus();

    // Reset both timers:
    WriteRegister(OPL_REG_TIMER_CTRL, 0x60);

    // Enable interrupts:
    WriteRegister(OPL_REG_TIMER_CTRL, 0x80);

    return (result1 & 0xe0) == 0x00
        && (result2 & 0xe0) == 0xc0;
}

// Initialise registers on startup

static void InitRegisters(void)
{
    int r;

    // Initialise level registers

    for (r=OPL_REGS_LEVEL; r <= OPL_REGS_LEVEL + OPL_NUM_OPERATORS; ++r)
    {
        WriteRegister(r, 0x3f);
    }

    // Initialise other registers
    // These two loops write to registers that actually don't exist,
    // but this is what Doom does ...
    // Similarly, the <= is also intenational.

    for (r=OPL_REGS_ATTACK; r <= OPL_REGS_WAVEFORM + OPL_NUM_OPERATORS; ++r)
    {
        WriteRegister(r, 0x00);
    }

    // More registers ...

    for (r=1; r < OPL_REGS_LEVEL; ++r)
    {
        WriteRegister(r, 0x00);
    }

    // Re-initialise the low registers:

    // Reset both timers and enable interrupts:
    WriteRegister(OPL_REG_TIMER_CTRL,      0x60);
    WriteRegister(OPL_REG_TIMER_CTRL,      0x80);

    // "Allow FM chips to control the waveform of each operator":
    WriteRegister(OPL_REG_WAVEFORM_ENABLE, 0x20);

    // Keyboard split point on (?)
    WriteRegister(OPL_REG_FM_MODE,         0x40);
}

// Load instrument table from GENMIDI lump:

static boolean LoadInstrumentTable(void)
{
    byte *lump;

    lump = W_CacheLumpName("GENMIDI", PU_STATIC);

    // Check header

    if (strncmp((char *) lump, GENMIDI_HEADER, strlen(GENMIDI_HEADER)) != 0)
    {
        W_ReleaseLumpName("GENMIDI");

        return false;
    }

    main_instrs = (genmidi_instr_t *) (lump + strlen(GENMIDI_HEADER));
    percussion_instrs = main_instrs + GENMIDI_NUM_INSTRS;

    return true;
}

// Get the next available voice from the freelist.

static opl_voice_t *GetFreeVoice(void)
{
    opl_voice_t *result;

    // None available?

    if (voice_free_list == NULL)
    {
        return NULL;
    }

    result = voice_free_list;
    voice_free_list = voice_free_list->next;

    return result;
}

// Release a voice back to the freelist.

static void ReleaseVoice(opl_voice_t *voice)
{
    opl_voice_t **rover;

    // Search to the end of the freelist (This is how Doom behaves!)

    rover = &voice_free_list;

    while (*rover != NULL)
    {
        rover = &(*rover)->next;
    }

    *rover = voice;
}

// Load data to the specified operator

static void LoadOperatorData(int operator, genmidi_op_t *data,
                             boolean max_level)
{
    int level;

    // The scale and level fields must be combined for the level register.
    // For the carrier wave we always set the maximum level.

    level = (data->scale & 0xc0) | (data->level & 0x3f);

    if (max_level)
    {
        level |= 0x3f;
    }

    WriteRegister(OPL_REGS_LEVEL + operator, level);
    WriteRegister(OPL_REGS_TREMOLO + operator, data->tremolo);
    WriteRegister(OPL_REGS_ATTACK + operator, data->attack);
    WriteRegister(OPL_REGS_SUSTAIN + operator, data->sustain);
    WriteRegister(OPL_REGS_WAVEFORM + operator, data->waveform);
}

// Set the instrument for a particular voice.

static void SetVoiceInstrument(opl_voice_t *voice, genmidi_voice_t *data)
{
    // Doom loads the second operator first, then the first.

    LoadOperatorData(voice->op2, &data->carrier, true);
    LoadOperatorData(voice->op1, &data->modulator, false);

    // Set feedback register that control the connection between the
    // two operators.  Turn on bits in the upper nybble; I think this
    // is for OPL3, where it turns on channel A/B.

    WriteRegister(OPL_REGS_FEEDBACK + voice->index,
                  data->feedback | 0x30);
}

// Initialise the voice table and freelist

static void InitVoices(void)
{
    int i;

    // Start with an empty free list.

    voice_free_list = NULL;

    // Initialise each voice.

    for (i=0; i<OPL_NUM_VOICES; ++i)
    {
        voices[i].index = i;
        voices[i].op1 = voice_operators[0][i];
        voices[i].op2 = voice_operators[1][i];
        voices[i].current_instr = NULL;

        // Add this voice to the freelist.

        ReleaseVoice(&voices[i]);
    }
}

// Shutdown music

static void I_OPL_ShutdownMusic(void)
{
    if (music_initialised)
    {
#ifdef TEST
        InitRegisters();
#endif
        OPL_Shutdown();

        // Release GENMIDI lump

        W_ReleaseLumpName("GENMIDI");

        music_initialised = false;
    }
}

#ifdef TEST
static void TestCallback(void *arg)
{
    opl_voice_t *voice = arg;
    int note;
    int wait_time;

    // Set level:
    WriteRegister(OPL_REGS_LEVEL + voice->op2, 0);

    // Note off:

    WriteRegister(OPL_REGS_FREQ_2 + voice->index, 0x00);
    // Note on:

    note = (rand() % (0x2ae - 0x16b)) + 0x16b;
    WriteRegister(OPL_REGS_FREQ_1 + voice->index, note & 0xff);
    WriteRegister(OPL_REGS_FREQ_2 + voice->index, 0x30 + (note >> 8));

    wait_time = (rand() % 700) + 50;
    OPL_SetCallback(wait_time, TestCallback, arg);
}
#endif

// Initialise music subsystem

static boolean I_OPL_InitMusic(void)
{
    if (!OPL_Init(snd_mport))
    {
        return false;
    }

    init_stage_reg_writes = true;

    // Doom does the detection sequence twice, for some reason:

    if (!DetectOPL() || !DetectOPL())
    {
        printf("Dude.  The Adlib isn't responding.\n");
        OPL_Shutdown();
        return false;
    }

    // Load instruments from GENMIDI lump:

    if (!LoadInstrumentTable())
    {
        OPL_Shutdown();
        return false;
    }

    InitRegisters();
    InitVoices();

    // Now that initialisation has finished, switch the
    // register writing mode:

    init_stage_reg_writes = false;

#ifdef TEST
    {
        int i;
        opl_voice_t *voice;
        int instr_num;

        for (i=0; i<3; ++i)
        {
            voice = GetFreeVoice();
            instr_num = rand() % 100;

            SetVoiceInstrument(voice, &main_instrs[instr_num].opl2_voice);

            OPL_SetCallback(0, TestCallback, voice);
        }
    }
#endif

    music_initialised = true;

    return true;
}

// Set music volume (0 - 127)

static void I_OPL_SetMusicVolume(int volume)
{
    // Internal state variable.
    current_music_volume = volume;
}

// Start playing a mid

static void I_OPL_PlaySong(void *handle, int looping)
{
    if (!music_initialised)
    {
        return;
    }
}

static void I_OPL_PauseSong(void)
{
    if (!music_initialised)
    {
        return;
    }
}

static void I_OPL_ResumeSong(void)
{
    if (!music_initialised)
    {
        return;
    }
}

static void I_OPL_StopSong(void)
{
    if (!music_initialised)
    {
        return;
    }
}

static void I_OPL_UnRegisterSong(void *handle)
{
    if (!music_initialised)
    {
        return;
    }

    if (handle != NULL)
    {
        MIDI_FreeFile(handle);
    }
}

// Determine whether memory block is a .mid file 

static boolean IsMid(byte *mem, int len)
{
    return len > 4 && !memcmp(mem, "MThd", 4);
}

static boolean ConvertMus(byte *musdata, int len, char *filename)
{
    MEMFILE *instream;
    MEMFILE *outstream;
    void *outbuf;
    size_t outbuf_len;
    int result;

    instream = mem_fopen_read(musdata, len);
    outstream = mem_fopen_write();

    result = mus2mid(instream, outstream);

    if (result == 0)
    {
        mem_get_buf(outstream, &outbuf, &outbuf_len);

        M_WriteFile(filename, outbuf, outbuf_len);
    }

    mem_fclose(instream);
    mem_fclose(outstream);

    return result;
}

static void *I_OPL_RegisterSong(void *data, int len)
{
    midi_file_t *result;
    char *filename;

    if (!music_initialised)
    {
        return NULL;
    }

    // MUS files begin with "MUS"
    // Reject anything which doesnt have this signature

    filename = M_TempFile("doom.mid");

    if (IsMid(data, len) && len < MAXMIDLENGTH)
    {
        M_WriteFile(filename, data, len);
    }
    else 
    {
	// Assume a MUS file and try to convert

        ConvertMus(data, len, filename);
    }

    result = MIDI_LoadFile(filename);

    if (result == NULL)
    {
        fprintf(stderr, "I_OPL_RegisterSong: Failed to load MID.\n");
    }

    // remove file now

    remove(filename);

    Z_Free(filename);

    return result;
}

// Is the song playing?
static boolean I_OPL_MusicIsPlaying(void)
{
    if (!music_initialised)
    {
        return false;
    }

    return false;
}

static snddevice_t music_opl_devices[] =
{
    SNDDEVICE_ADLIB,
    SNDDEVICE_SB,
};

music_module_t music_opl_module =
{
    music_opl_devices,
    arrlen(music_opl_devices),
    I_OPL_InitMusic,
    I_OPL_ShutdownMusic,
    I_OPL_SetMusicVolume,
    I_OPL_PauseSong,
    I_OPL_ResumeSong,
    I_OPL_RegisterSong,
    I_OPL_UnRegisterSong,
    I_OPL_PlaySong,
    I_OPL_StopSong,
    I_OPL_MusicIsPlaying,
};

