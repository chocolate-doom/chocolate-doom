//
// Copyright(C) 1993-1996 Id Software, Inc.
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
//	System interface for music.
//


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "memio.h"
#include "mus2mid.h"

#include "deh_main.h"
#include "i_sound.h"
#include "i_swap.h"
#include "m_misc.h"
#include "w_wad.h"
#include "z_zone.h"

#include "opl.h"
#include "midifile.h"

// #define OPL_MIDI_DEBUG

#define MAXMIDLENGTH (96 * 1024)
#define GENMIDI_NUM_INSTRS  128
#define GENMIDI_NUM_PERCUSSION 47

#define GENMIDI_HEADER          "#OPL_II#"
#define GENMIDI_FLAG_FIXED      0x0001         /* fixed pitch */
#define GENMIDI_FLAG_2VOICE     0x0004         /* double voice (OPL3) */

#define PERCUSSION_LOG_LEN 16

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

    genmidi_voice_t voices[2];
} PACKEDATTR genmidi_instr_t;

// Data associated with a channel of a track that is currently playing.

typedef struct
{
    // The instrument currently used for this track.

    genmidi_instr_t *instrument;

    // Volume level

    int volume;

    // Pitch bend value:

    int bend;

} opl_channel_data_t;

// Data associated with a track that is currently playing.

typedef struct
{
    // Data for each channel.

    opl_channel_data_t channels[MIDI_CHANNELS_PER_TRACK];

    // Track iterator used to read new events.

    midi_track_iter_t *iter;
} opl_track_data_t;

typedef struct opl_voice_s opl_voice_t;

struct opl_voice_s
{
    // Index of this voice:
    int index;

    // The operators used by this voice:
    int op1, op2;

    // Currently-loaded instrument data
    genmidi_instr_t *current_instr;

    // The voice number in the instrument to use.
    // This is normally set to zero; if this is a double voice
    // instrument, it may be one.
    unsigned int current_instr_voice;

    // The channel currently using this voice.
    opl_channel_data_t *channel;

    // The midi key that this voice is playing.
    unsigned int key;

    // The note being played.  This is normally the same as
    // the key, but if the instrument is a fixed pitch
    // instrument, it is different.
    unsigned int note;

    // The frequency value being used.
    unsigned int freq;

    // The volume of the note being played on this channel.
    unsigned int note_volume;

    // The current volume (register value) that has been set for this channel.
    unsigned int reg_volume;

    // Next in linked list; a voice is always either in the
    // free list or the allocated list.
    opl_voice_t *next;
};

// Operators used by the different voices.

static const int voice_operators[2][OPL_NUM_VOICES] = {
    { 0x00, 0x01, 0x02, 0x08, 0x09, 0x0a, 0x10, 0x11, 0x12 },
    { 0x03, 0x04, 0x05, 0x0b, 0x0c, 0x0d, 0x13, 0x14, 0x15 }
};

// Frequency values to use for each note.

static const unsigned short frequency_curve[] = {

    0x133, 0x133, 0x134, 0x134, 0x135, 0x136, 0x136, 0x137,   // -1
    0x137, 0x138, 0x138, 0x139, 0x139, 0x13a, 0x13b, 0x13b,
    0x13c, 0x13c, 0x13d, 0x13d, 0x13e, 0x13f, 0x13f, 0x140,
    0x140, 0x141, 0x142, 0x142, 0x143, 0x143, 0x144, 0x144,

    0x145, 0x146, 0x146, 0x147, 0x147, 0x148, 0x149, 0x149,   // -2
    0x14a, 0x14a, 0x14b, 0x14c, 0x14c, 0x14d, 0x14d, 0x14e,
    0x14f, 0x14f, 0x150, 0x150, 0x151, 0x152, 0x152, 0x153,
    0x153, 0x154, 0x155, 0x155, 0x156, 0x157, 0x157, 0x158,

    // These are used for the first seven MIDI note values:

    0x158, 0x159, 0x15a, 0x15a, 0x15b, 0x15b, 0x15c, 0x15d,   // 0
    0x15d, 0x15e, 0x15f, 0x15f, 0x160, 0x161, 0x161, 0x162,
    0x162, 0x163, 0x164, 0x164, 0x165, 0x166, 0x166, 0x167,
    0x168, 0x168, 0x169, 0x16a, 0x16a, 0x16b, 0x16c, 0x16c,

    0x16d, 0x16e, 0x16e, 0x16f, 0x170, 0x170, 0x171, 0x172,   // 1
    0x172, 0x173, 0x174, 0x174, 0x175, 0x176, 0x176, 0x177,
    0x178, 0x178, 0x179, 0x17a, 0x17a, 0x17b, 0x17c, 0x17c,
    0x17d, 0x17e, 0x17e, 0x17f, 0x180, 0x181, 0x181, 0x182,

    0x183, 0x183, 0x184, 0x185, 0x185, 0x186, 0x187, 0x188,   // 2
    0x188, 0x189, 0x18a, 0x18a, 0x18b, 0x18c, 0x18d, 0x18d,
    0x18e, 0x18f, 0x18f, 0x190, 0x191, 0x192, 0x192, 0x193,
    0x194, 0x194, 0x195, 0x196, 0x197, 0x197, 0x198, 0x199,

    0x19a, 0x19a, 0x19b, 0x19c, 0x19d, 0x19d, 0x19e, 0x19f,   // 3
    0x1a0, 0x1a0, 0x1a1, 0x1a2, 0x1a3, 0x1a3, 0x1a4, 0x1a5,
    0x1a6, 0x1a6, 0x1a7, 0x1a8, 0x1a9, 0x1a9, 0x1aa, 0x1ab,
    0x1ac, 0x1ad, 0x1ad, 0x1ae, 0x1af, 0x1b0, 0x1b0, 0x1b1,

    0x1b2, 0x1b3, 0x1b4, 0x1b4, 0x1b5, 0x1b6, 0x1b7, 0x1b8,   // 4
    0x1b8, 0x1b9, 0x1ba, 0x1bb, 0x1bc, 0x1bc, 0x1bd, 0x1be,
    0x1bf, 0x1c0, 0x1c0, 0x1c1, 0x1c2, 0x1c3, 0x1c4, 0x1c4,
    0x1c5, 0x1c6, 0x1c7, 0x1c8, 0x1c9, 0x1c9, 0x1ca, 0x1cb,

    0x1cc, 0x1cd, 0x1ce, 0x1ce, 0x1cf, 0x1d0, 0x1d1, 0x1d2,   // 5
    0x1d3, 0x1d3, 0x1d4, 0x1d5, 0x1d6, 0x1d7, 0x1d8, 0x1d8,
    0x1d9, 0x1da, 0x1db, 0x1dc, 0x1dd, 0x1de, 0x1de, 0x1df,
    0x1e0, 0x1e1, 0x1e2, 0x1e3, 0x1e4, 0x1e5, 0x1e5, 0x1e6,

    0x1e7, 0x1e8, 0x1e9, 0x1ea, 0x1eb, 0x1ec, 0x1ed, 0x1ed,   // 6
    0x1ee, 0x1ef, 0x1f0, 0x1f1, 0x1f2, 0x1f3, 0x1f4, 0x1f5,
    0x1f6, 0x1f6, 0x1f7, 0x1f8, 0x1f9, 0x1fa, 0x1fb, 0x1fc,
    0x1fd, 0x1fe, 0x1ff, 0x200, 0x201, 0x201, 0x202, 0x203,

    // First note of looped range used for all octaves:

    0x204, 0x205, 0x206, 0x207, 0x208, 0x209, 0x20a, 0x20b,   // 7
    0x20c, 0x20d, 0x20e, 0x20f, 0x210, 0x210, 0x211, 0x212,
    0x213, 0x214, 0x215, 0x216, 0x217, 0x218, 0x219, 0x21a,
    0x21b, 0x21c, 0x21d, 0x21e, 0x21f, 0x220, 0x221, 0x222,

    0x223, 0x224, 0x225, 0x226, 0x227, 0x228, 0x229, 0x22a,   // 8
    0x22b, 0x22c, 0x22d, 0x22e, 0x22f, 0x230, 0x231, 0x232,
    0x233, 0x234, 0x235, 0x236, 0x237, 0x238, 0x239, 0x23a,
    0x23b, 0x23c, 0x23d, 0x23e, 0x23f, 0x240, 0x241, 0x242,

    0x244, 0x245, 0x246, 0x247, 0x248, 0x249, 0x24a, 0x24b,   // 9
    0x24c, 0x24d, 0x24e, 0x24f, 0x250, 0x251, 0x252, 0x253,
    0x254, 0x256, 0x257, 0x258, 0x259, 0x25a, 0x25b, 0x25c,
    0x25d, 0x25e, 0x25f, 0x260, 0x262, 0x263, 0x264, 0x265,

    0x266, 0x267, 0x268, 0x269, 0x26a, 0x26c, 0x26d, 0x26e,   // 10
    0x26f, 0x270, 0x271, 0x272, 0x273, 0x275, 0x276, 0x277,
    0x278, 0x279, 0x27a, 0x27b, 0x27d, 0x27e, 0x27f, 0x280,
    0x281, 0x282, 0x284, 0x285, 0x286, 0x287, 0x288, 0x289,

    0x28b, 0x28c, 0x28d, 0x28e, 0x28f, 0x290, 0x292, 0x293,   // 11
    0x294, 0x295, 0x296, 0x298, 0x299, 0x29a, 0x29b, 0x29c,
    0x29e, 0x29f, 0x2a0, 0x2a1, 0x2a2, 0x2a4, 0x2a5, 0x2a6,
    0x2a7, 0x2a9, 0x2aa, 0x2ab, 0x2ac, 0x2ae, 0x2af, 0x2b0,

    0x2b1, 0x2b2, 0x2b4, 0x2b5, 0x2b6, 0x2b7, 0x2b9, 0x2ba,   // 12
    0x2bb, 0x2bd, 0x2be, 0x2bf, 0x2c0, 0x2c2, 0x2c3, 0x2c4,
    0x2c5, 0x2c7, 0x2c8, 0x2c9, 0x2cb, 0x2cc, 0x2cd, 0x2ce,
    0x2d0, 0x2d1, 0x2d2, 0x2d4, 0x2d5, 0x2d6, 0x2d8, 0x2d9,

    0x2da, 0x2dc, 0x2dd, 0x2de, 0x2e0, 0x2e1, 0x2e2, 0x2e4,   // 13
    0x2e5, 0x2e6, 0x2e8, 0x2e9, 0x2ea, 0x2ec, 0x2ed, 0x2ee,
    0x2f0, 0x2f1, 0x2f2, 0x2f4, 0x2f5, 0x2f6, 0x2f8, 0x2f9,
    0x2fb, 0x2fc, 0x2fd, 0x2ff, 0x300, 0x302, 0x303, 0x304,

    0x306, 0x307, 0x309, 0x30a, 0x30b, 0x30d, 0x30e, 0x310,   // 14
    0x311, 0x312, 0x314, 0x315, 0x317, 0x318, 0x31a, 0x31b,
    0x31c, 0x31e, 0x31f, 0x321, 0x322, 0x324, 0x325, 0x327,
    0x328, 0x329, 0x32b, 0x32c, 0x32e, 0x32f, 0x331, 0x332,

    0x334, 0x335, 0x337, 0x338, 0x33a, 0x33b, 0x33d, 0x33e,   // 15
    0x340, 0x341, 0x343, 0x344, 0x346, 0x347, 0x349, 0x34a,
    0x34c, 0x34d, 0x34f, 0x350, 0x352, 0x353, 0x355, 0x357,
    0x358, 0x35a, 0x35b, 0x35d, 0x35e, 0x360, 0x361, 0x363,

    0x365, 0x366, 0x368, 0x369, 0x36b, 0x36c, 0x36e, 0x370,   // 16
    0x371, 0x373, 0x374, 0x376, 0x378, 0x379, 0x37b, 0x37c,
    0x37e, 0x380, 0x381, 0x383, 0x384, 0x386, 0x388, 0x389,
    0x38b, 0x38d, 0x38e, 0x390, 0x392, 0x393, 0x395, 0x397,

    0x398, 0x39a, 0x39c, 0x39d, 0x39f, 0x3a1, 0x3a2, 0x3a4,   // 17
    0x3a6, 0x3a7, 0x3a9, 0x3ab, 0x3ac, 0x3ae, 0x3b0, 0x3b1,
    0x3b3, 0x3b5, 0x3b7, 0x3b8, 0x3ba, 0x3bc, 0x3bd, 0x3bf,
    0x3c1, 0x3c3, 0x3c4, 0x3c6, 0x3c8, 0x3ca, 0x3cb, 0x3cd,

    // The last note has an incomplete range, and loops round back to
    // the start.  Note that the last value is actually a buffer overrun
    // and does not fit with the other values.

    0x3cf, 0x3d1, 0x3d2, 0x3d4, 0x3d6, 0x3d8, 0x3da, 0x3db,   // 18
    0x3dd, 0x3df, 0x3e1, 0x3e3, 0x3e4, 0x3e6, 0x3e8, 0x3ea,
    0x3ec, 0x3ed, 0x3ef, 0x3f1, 0x3f3, 0x3f5, 0x3f6, 0x3f8,
    0x3fa, 0x3fc, 0x3fe, 0x36c,
};

// Mapping from MIDI volume level to OPL level value.

static const unsigned int volume_mapping_table[] = {
    0, 1, 3, 5, 6, 8, 10, 11,
    13, 14, 16, 17, 19, 20, 22, 23,
    25, 26, 27, 29, 30, 32, 33, 34,
    36, 37, 39, 41, 43, 45, 47, 49,
    50, 52, 54, 55, 57, 59, 60, 61,
    63, 64, 66, 67, 68, 69, 71, 72,
    73, 74, 75, 76, 77, 79, 80, 81,
    82, 83, 84, 84, 85, 86, 87, 88,
    89, 90, 91, 92, 92, 93, 94, 95,
    96, 96, 97, 98, 99, 99, 100, 101,
    101, 102, 103, 103, 104, 105, 105, 106,
    107, 107, 108, 109, 109, 110, 110, 111,
    112, 112, 113, 113, 114, 114, 115, 115,
    116, 117, 117, 118, 118, 119, 119, 120,
    120, 121, 121, 122, 122, 123, 123, 123,
    124, 124, 125, 125, 126, 126, 127, 127
};

static boolean music_initialized = false;

//static boolean musicpaused = false;
static int current_music_volume;

// GENMIDI lump instrument data:

static genmidi_instr_t *main_instrs;
static genmidi_instr_t *percussion_instrs;
static char (*main_instr_names)[32];
static char (*percussion_names)[32];

// Voices:

static opl_voice_t voices[OPL_NUM_VOICES];
static opl_voice_t *voice_free_list;
static opl_voice_t *voice_alloced_list;

// Track data for playing tracks:

static opl_track_data_t *tracks;
static unsigned int num_tracks = 0;
static unsigned int running_tracks = 0;
static boolean song_looping;

// Tempo control variables

static unsigned int ticks_per_beat;
static unsigned int us_per_beat;

// Mini-log of recently played percussion instruments:

static uint8_t last_perc[PERCUSSION_LOG_LEN];
static unsigned int last_perc_count;

// Configuration file variable, containing the port number for the
// adlib chip.

int opl_io_port = 0x388;

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
    main_instr_names = (char (*)[32]) (percussion_instrs + GENMIDI_NUM_PERCUSSION);
    percussion_names = main_instr_names + GENMIDI_NUM_INSTRS;

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

    // Remove from free list

    result = voice_free_list;
    voice_free_list = voice_free_list->next;

    // Add to allocated list

    result->next = voice_alloced_list;
    voice_alloced_list = result;

    return result;
}

// Remove a voice from the allocated voices list.

static void RemoveVoiceFromAllocedList(opl_voice_t *voice)
{
    opl_voice_t **rover;

    rover = &voice_alloced_list;

    // Search the list until we find the voice, then remove it.

    while (*rover != NULL)
    {
        if (*rover == voice)
        {
            *rover = voice->next;
            voice->next = NULL;
            break;
        }

        rover = &(*rover)->next;
    }
}

// Release a voice back to the freelist.

static void ReleaseVoice(opl_voice_t *voice)
{
    opl_voice_t **rover;

    voice->channel = NULL;
    voice->note = 0;

    // Remove from alloced list.

    RemoveVoiceFromAllocedList(voice);

    // Search to the end of the freelist (This is how Doom behaves!)

    rover = &voice_free_list;

    while (*rover != NULL)
    {
        rover = &(*rover)->next;
    }

    *rover = voice;
    voice->next = NULL;
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

    OPL_WriteRegister(OPL_REGS_LEVEL + operator, level);
    OPL_WriteRegister(OPL_REGS_TREMOLO + operator, data->tremolo);
    OPL_WriteRegister(OPL_REGS_ATTACK + operator, data->attack);
    OPL_WriteRegister(OPL_REGS_SUSTAIN + operator, data->sustain);
    OPL_WriteRegister(OPL_REGS_WAVEFORM + operator, data->waveform);
}

// Set the instrument for a particular voice.

static void SetVoiceInstrument(opl_voice_t *voice,
                               genmidi_instr_t *instr,
                               unsigned int instr_voice)
{
    genmidi_voice_t *data;
    unsigned int modulating;

    // Instrument already set for this channel?

    if (voice->current_instr == instr
     && voice->current_instr_voice == instr_voice)
    {
        return;
    }

    voice->current_instr = instr;
    voice->current_instr_voice = instr_voice;

    data = &instr->voices[instr_voice];

    // Are we usind modulated feedback mode?

    modulating = (data->feedback & 0x01) == 0;

    // Doom loads the second operator first, then the first.
    // The carrier is set to minimum volume until the voice volume
    // is set in SetVoiceVolume (below).  If we are not using
    // modulating mode, we must set both to minimum volume.

    LoadOperatorData(voice->op2, &data->carrier, true);
    LoadOperatorData(voice->op1, &data->modulator, !modulating);

    // Set feedback register that control the connection between the
    // two operators.  Turn on bits in the upper nybble; I think this
    // is for OPL3, where it turns on channel A/B.

    OPL_WriteRegister(OPL_REGS_FEEDBACK + voice->index,
                      data->feedback | 0x30);

    // Hack to force a volume update.

    voice->reg_volume = 999;
}

static void SetVoiceVolume(opl_voice_t *voice, unsigned int volume)
{
    genmidi_voice_t *opl_voice;
    unsigned int full_volume;
    unsigned int op_volume;
    unsigned int reg_volume;

    voice->note_volume = volume;

    opl_voice = &voice->current_instr->voices[voice->current_instr_voice];

    // Multiply note volume and channel volume to get the actual volume.

    full_volume = (volume_mapping_table[voice->note_volume]
                   * volume_mapping_table[voice->channel->volume]
                   * volume_mapping_table[current_music_volume]) / (127 * 127);

    // The volume of each instrument can be controlled via GENMIDI:

    op_volume = 0x3f - opl_voice->carrier.level;

    // The volume value to use in the register:

    reg_volume = (op_volume * full_volume) / 128;
    reg_volume = (0x3f - reg_volume) | opl_voice->carrier.scale;

    // Update the volume register(s) if necessary.

    if (reg_volume != voice->reg_volume)
    {
        voice->reg_volume = reg_volume;

        OPL_WriteRegister(OPL_REGS_LEVEL + voice->op2, reg_volume);

        // If we are using non-modulated feedback mode, we must set the
        // volume for both voices.
        // Note that the same register volume value is written for
        // both voices, always calculated from the carrier's level
        // value.

        if ((opl_voice->feedback & 0x01) != 0)
        {
            OPL_WriteRegister(OPL_REGS_LEVEL + voice->op1, reg_volume);
        }
    }
}

// Initialize the voice table and freelist

static void InitVoices(void)
{
    int i;

    // Start with an empty free list.

    voice_free_list = NULL;

    // Initialize each voice.

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

// Set music volume (0 - 127)

static void I_OPL_SetMusicVolume(int volume)
{
    unsigned int i;

    // Internal state variable.

    current_music_volume = volume;

    // Update the volume of all voices.

    for (i=0; i<OPL_NUM_VOICES; ++i)
    {
        if (voices[i].channel != NULL)
        {
            SetVoiceVolume(&voices[i], voices[i].note_volume);
        }
    }
}

static void VoiceKeyOff(opl_voice_t *voice)
{
    OPL_WriteRegister(OPL_REGS_FREQ_2 + voice->index, voice->freq >> 8);
}

// Get the frequency that we should be using for a voice.

static void KeyOffEvent(opl_track_data_t *track, midi_event_t *event)
{
    opl_channel_data_t *channel;
    unsigned int key;
    unsigned int i;

/*
    printf("note off: channel %i, %i, %i\n",
           event->data.channel.channel,
           event->data.channel.param1,
           event->data.channel.param2);
*/

    channel = &track->channels[event->data.channel.channel];
    key = event->data.channel.param1;

    // Turn off voices being used to play this key.
    // If it is a double voice instrument there will be two.

    for (i=0; i<OPL_NUM_VOICES; ++i)
    {
        if (voices[i].channel == channel && voices[i].key == key)
        {
            VoiceKeyOff(&voices[i]);

            // Finished with this voice now.

            ReleaseVoice(&voices[i]);
        }
    }
}

// Compare the priorities of channels, returning either -1, 0 or 1.

static int CompareChannelPriorities(opl_channel_data_t *chan1,
                                    opl_channel_data_t *chan2)
{
    // TODO ...

    return 1;
}

// When all voices are in use, we must discard an existing voice to
// play a new note.  Find and free an existing voice.  The channel
// passed to the function is the channel for the new note to be
// played.

static opl_voice_t *ReplaceExistingVoice(opl_channel_data_t *channel)
{
    opl_voice_t *rover;
    opl_voice_t *result;

    // Check the allocated voices, if we find an instrument that is
    // of a lower priority to the new instrument, discard it.
    // If a voice is being used to play the second voice of an instrument,
    // use that, as second voices are non-essential.
    // Lower numbered MIDI channels implicitly have a higher priority
    // than higher-numbered channels, eg. MIDI channel 1 is never
    // discarded for MIDI channel 2.

    result = NULL;

    for (rover = voice_alloced_list; rover != NULL; rover = rover->next)
    {
        if (rover->current_instr_voice != 0
         || (rover->channel > channel
             && CompareChannelPriorities(channel, rover->channel) > 0))
        {
            result = rover;
            break;
        }
    }

    // If we didn't find a voice, find an existing voice being used to
    // play a note on the same channel, and use that.

    if (result == NULL)
    {
        for (rover = voice_alloced_list; rover != NULL; rover = rover->next)
        {
            if (rover->channel == channel)
            {
                result = rover;
                break;
            }
        }
    }

    // Still nothing found?  Give up and just use the first voice in
    // the list.

    if (result == NULL)
    {
        result = voice_alloced_list;
    }

    // Stop playing this voice playing and release it back to the free
    // list.

    VoiceKeyOff(result);
    ReleaseVoice(result);

    // Re-allocate the voice again and return it.

    return GetFreeVoice();
}


static unsigned int FrequencyForVoice(opl_voice_t *voice)
{
    genmidi_voice_t *gm_voice;
    unsigned int freq_index;
    unsigned int octave;
    unsigned int sub_index;
    unsigned int note;

    note = voice->note;

    // Apply note offset.
    // Don't apply offset if the instrument is a fixed note instrument.

    gm_voice = &voice->current_instr->voices[voice->current_instr_voice];

    if ((SHORT(voice->current_instr->flags) & GENMIDI_FLAG_FIXED) == 0)
    {
        note += (signed short) SHORT(gm_voice->base_note_offset);
    }

    // Avoid possible overflow due to base note offset:

    if (note > 0x7f)
    {
        note = voice->note;
    }

    freq_index = 64 + 32 * note + voice->channel->bend;

    // If this is the second voice of a double voice instrument, the
    // frequency index can be adjusted by the fine tuning field.

    if (voice->current_instr_voice != 0)
    {
        freq_index += (voice->current_instr->fine_tuning / 2) - 64;
    }

    // The first 7 notes use the start of the table, while
    // consecutive notes loop around the latter part.

    if (freq_index < 284)
    {
        return frequency_curve[freq_index];
    }

    sub_index = (freq_index - 284) % (12 * 32);
    octave = (freq_index - 284) / (12 * 32);

    // Once the seventh octave is reached, things break down.
    // We can only go up to octave 7 as a maximum anyway (the OPL
    // register only has three bits for octave number), but for the
    // notes in octave 7, the first five bits have octave=7, the
    // following notes have octave=6.  This 7/6 pattern repeats in
    // following octaves (which are technically impossible to
    // represent anyway).

    if (octave >= 7)
    {
        if (sub_index < 5)
        {
            octave = 7;
        }
        else
        {
            octave = 6;
        }
    }

    // Calculate the resulting register value to use for the frequency.

    return frequency_curve[sub_index + 284] | (octave << 10);
}

// Update the frequency that a voice is programmed to use.

static void UpdateVoiceFrequency(opl_voice_t *voice)
{
    unsigned int freq;

    // Calculate the frequency to use for this voice and update it
    // if neccessary.

    freq = FrequencyForVoice(voice);

    if (voice->freq != freq)
    {
        OPL_WriteRegister(OPL_REGS_FREQ_1 + voice->index, freq & 0xff);
        OPL_WriteRegister(OPL_REGS_FREQ_2 + voice->index, (freq >> 8) | 0x20);

        voice->freq = freq;
    }
}

// Program a single voice for an instrument.  For a double voice 
// instrument (GENMIDI_FLAG_2VOICE), this is called twice for each
// key on event.

static void VoiceKeyOn(opl_channel_data_t *channel,
                       genmidi_instr_t *instrument,
                       unsigned int instrument_voice,
                       unsigned int key,
                       unsigned int volume)
{
    opl_voice_t *voice;

    // Find a voice to use for this new note.

    voice = GetFreeVoice();

    // If there are no more voices left, we must decide what to do.
    // If this is the first voice of the instrument, free an existing
    // voice and use that.  Otherwise, if this is the second voice,
    // it isn't as important; just discard it.

    if (voice == NULL)
    {
        if (instrument_voice == 0)
        {
            voice = ReplaceExistingVoice(channel);
        }
        else
        {
            return;
        }
    }

    voice->channel = channel;
    voice->key = key;

    // Work out the note to use.  This is normally the same as
    // the key, unless it is a fixed pitch instrument.

    if ((SHORT(instrument->flags) & GENMIDI_FLAG_FIXED) != 0)
    {
        voice->note = instrument->fixed_note;
    }
    else
    {
        voice->note = key;
    }

    // Program the voice with the instrument data:

    SetVoiceInstrument(voice, instrument, instrument_voice);

    // Set the volume level.

    SetVoiceVolume(voice, volume);

    // Write the frequency value to turn the note on.

    voice->freq = 0;
    UpdateVoiceFrequency(voice);
}

static void KeyOnEvent(opl_track_data_t *track, midi_event_t *event)
{
    genmidi_instr_t *instrument;
    opl_channel_data_t *channel;
    unsigned int key;
    unsigned int volume;

/*
    printf("note on: channel %i, %i, %i\n",
           event->data.channel.channel,
           event->data.channel.param1,
           event->data.channel.param2);
*/

    key = event->data.channel.param1;
    volume = event->data.channel.param2;

    // A volume of zero means key off. Some MIDI tracks, eg. the ones
    // in AV.wad, use a second key on with a volume of zero to mean
    // key off.
    if (volume <= 0)
    {
        KeyOffEvent(track, event);
        return;
    }

    // The channel.
    channel = &track->channels[event->data.channel.channel];

    // Percussion channel (10) is treated differently.

    if (event->data.channel.channel == 9)
    {
        if (key < 35 || key > 81)
        {
            return;
        }

        instrument = &percussion_instrs[key - 35];

        last_perc[last_perc_count] = key;
        last_perc_count = (last_perc_count + 1) % PERCUSSION_LOG_LEN;
    }
    else
    {
        instrument = channel->instrument;
    }

    // Find and program a voice for this instrument.  If this
    // is a double voice instrument, we must do this twice.

    VoiceKeyOn(channel, instrument, 0, key, volume);

    if ((SHORT(instrument->flags) & GENMIDI_FLAG_2VOICE) != 0)
    {
        VoiceKeyOn(channel, instrument, 1, key, volume);
    }
}

static void ProgramChangeEvent(opl_track_data_t *track, midi_event_t *event)
{
    int channel;
    int instrument;

    // Set the instrument used on this channel.

    channel = event->data.channel.channel;
    instrument = event->data.channel.param1;
    track->channels[channel].instrument = &main_instrs[instrument];

    // TODO: Look through existing voices that are turned on on this
    // channel, and change the instrument.
}

static void SetChannelVolume(opl_channel_data_t *channel, unsigned int volume)
{
    unsigned int i;

    channel->volume = volume;

    // Update all voices that this channel is using.

    for (i=0; i<OPL_NUM_VOICES; ++i)
    {
        if (voices[i].channel == channel)
        {
            SetVoiceVolume(&voices[i], voices[i].note_volume);
        }
    }
}

// Handler for the MIDI_CONTROLLER_ALL_NOTES_OFF channel event.
static void AllNotesOff(opl_channel_data_t *channel, unsigned int param)
{
    unsigned int i;

    for (i = 0; i < OPL_NUM_VOICES; ++i)
    {
        if (voices[i].channel == channel)
        {
            VoiceKeyOff(&voices[i]);
            ReleaseVoice(&voices[i]);
        }
    }
}

static void ControllerEvent(opl_track_data_t *track, midi_event_t *event)
{
    unsigned int controller;
    unsigned int param;
    opl_channel_data_t *channel;

/*
    printf("change controller: channel %i, %i, %i\n",
           event->data.channel.channel,
           event->data.channel.param1,
           event->data.channel.param2);
*/

    channel = &track->channels[event->data.channel.channel];
    controller = event->data.channel.param1;
    param = event->data.channel.param2;

    switch (controller)
    {
        case MIDI_CONTROLLER_MAIN_VOLUME:
            SetChannelVolume(channel, param);
            break;

        case MIDI_CONTROLLER_ALL_NOTES_OFF:
            AllNotesOff(channel, param);
            break;

        default:
#ifdef OPL_MIDI_DEBUG
            fprintf(stderr, "Unknown MIDI controller type: %i\n", controller);
#endif
            break;
    }
}

// Process a pitch bend event.

static void PitchBendEvent(opl_track_data_t *track, midi_event_t *event)
{
    opl_channel_data_t *channel;
    unsigned int i;

    // Update the channel bend value.  Only the MSB of the pitch bend
    // value is considered: this is what Doom does.

    channel = &track->channels[event->data.channel.channel];
    channel->bend = event->data.channel.param2 - 64;

    // Update all voices for this channel.

    for (i=0; i<OPL_NUM_VOICES; ++i)
    {
        if (voices[i].channel == channel)
        {
            UpdateVoiceFrequency(&voices[i]);
        }
    }
}

static void MetaSetTempo(unsigned int tempo)
{
    OPL_AdjustCallbacks((float) us_per_beat / tempo);
    us_per_beat = tempo;
}

// Process a meta event.

static void MetaEvent(opl_track_data_t *track, midi_event_t *event)
{
    byte *data = event->data.meta.data;
    unsigned int data_len = event->data.meta.length;

    switch (event->data.meta.type)
    {
        // Things we can just ignore.

        case MIDI_META_SEQUENCE_NUMBER:
        case MIDI_META_TEXT:
        case MIDI_META_COPYRIGHT:
        case MIDI_META_TRACK_NAME:
        case MIDI_META_INSTR_NAME:
        case MIDI_META_LYRICS:
        case MIDI_META_MARKER:
        case MIDI_META_CUE_POINT:
        case MIDI_META_SEQUENCER_SPECIFIC:
            break;

        case MIDI_META_SET_TEMPO:
            if (data_len == 3)
            {
                MetaSetTempo((data[0] << 16) | (data[1] << 8) | data[2]);
            }
            break;

        // End of track - actually handled when we run out of events
        // in the track, see below.

        case MIDI_META_END_OF_TRACK:
            break;

        default:
#ifdef OPL_MIDI_DEBUG
            fprintf(stderr, "Unknown MIDI meta event type: %i\n",
                            event->data.meta.type);
#endif
            break;
    }
}

// Process a MIDI event from a track.

static void ProcessEvent(opl_track_data_t *track, midi_event_t *event)
{
    switch (event->event_type)
    {
        case MIDI_EVENT_NOTE_OFF:
            KeyOffEvent(track, event);
            break;

        case MIDI_EVENT_NOTE_ON:
            KeyOnEvent(track, event);
            break;

        case MIDI_EVENT_CONTROLLER:
            ControllerEvent(track, event);
            break;

        case MIDI_EVENT_PROGRAM_CHANGE:
            ProgramChangeEvent(track, event);
            break;

        case MIDI_EVENT_PITCH_BEND:
            PitchBendEvent(track, event);
            break;

        case MIDI_EVENT_META:
            MetaEvent(track, event);
            break;

        // SysEx events can be ignored.

        case MIDI_EVENT_SYSEX:
        case MIDI_EVENT_SYSEX_SPLIT:
            break;

        default:
#ifdef OPL_MIDI_DEBUG
            fprintf(stderr, "Unknown MIDI event type %i\n", event->event_type);
#endif
            break;
    }
}

static void ScheduleTrack(opl_track_data_t *track);

// Restart a song from the beginning.

static void RestartSong(void *unused)
{
    unsigned int i;

    running_tracks = num_tracks;

    for (i=0; i<num_tracks; ++i)
    {
        MIDI_RestartIterator(tracks[i].iter);
        ScheduleTrack(&tracks[i]);
    }
}

// Callback function invoked when another event needs to be read from
// a track.

static void TrackTimerCallback(void *arg)
{
    opl_track_data_t *track = arg;
    midi_event_t *event;

    // Get the next event and process it.

    if (!MIDI_GetNextEvent(track->iter, &event))
    {
        return;
    }

    ProcessEvent(track, event);

    // End of track?

    if (event->event_type == MIDI_EVENT_META
     && event->data.meta.type == MIDI_META_END_OF_TRACK)
    {
        --running_tracks;

        // When all tracks have finished, restart the song.
        // Don't restart the song immediately, but wait for 5ms
        // before triggering a restart.  Otherwise it is possible
        // to construct an empty MIDI file that causes the game
        // to lock up in an infinite loop. (5ms should be short
        // enough not to be noticeable by the listener).

        if (running_tracks <= 0 && song_looping)
        {
            OPL_SetCallback(5000, RestartSong, NULL);
        }

        return;
    }

    // Reschedule the callback for the next event in the track.

    ScheduleTrack(track);
}

static void ScheduleTrack(opl_track_data_t *track)
{
    unsigned int nticks;
    uint64_t us;

    // Get the number of microseconds until the next event.

    nticks = MIDI_GetDeltaTime(track->iter);
    us = ((uint64_t) nticks * us_per_beat) / ticks_per_beat;

    // Set a timer to be invoked when the next event is
    // ready to play.

    OPL_SetCallback(us, TrackTimerCallback, track);
}

// Initialize a channel.

static void InitChannel(opl_track_data_t *track, opl_channel_data_t *channel)
{
    // TODO: Work out sensible defaults?

    channel->instrument = &main_instrs[0];
    channel->volume = 127;
    channel->bend = 0;
}

// Start a MIDI track playing:

static void StartTrack(midi_file_t *file, unsigned int track_num)
{
    opl_track_data_t *track;
    unsigned int i;

    track = &tracks[track_num];
    track->iter = MIDI_IterateTrack(file, track_num);

    for (i=0; i<MIDI_CHANNELS_PER_TRACK; ++i)
    {
        InitChannel(track, &track->channels[i]);
    }

    // Schedule the first event.

    ScheduleTrack(track);
}

// Start playing a mid

static void I_OPL_PlaySong(void *handle, boolean looping)
{
    midi_file_t *file;
    unsigned int i;

    if (!music_initialized || handle == NULL)
    {
        return;
    }

    file = handle;

    // Allocate track data.

    tracks = malloc(MIDI_NumTracks(file) * sizeof(opl_track_data_t));

    num_tracks = MIDI_NumTracks(file);
    running_tracks = num_tracks;
    song_looping = looping;

    ticks_per_beat = MIDI_GetFileTimeDivision(file);

    // Default is 120 bpm.
    // TODO: this is wrong

    us_per_beat = 500 * 1000;

    for (i=0; i<num_tracks; ++i)
    {
        StartTrack(file, i);
    }
}

static void I_OPL_PauseSong(void)
{
    unsigned int i;

    if (!music_initialized)
    {
        return;
    }

    // Pause OPL callbacks.

    OPL_SetPaused(1);

    // Turn off all main instrument voices (not percussion).
    // This is what Vanilla does.

    for (i=0; i<OPL_NUM_VOICES; ++i)
    {
        if (voices[i].channel != NULL
         && voices[i].current_instr < percussion_instrs)
        {
            VoiceKeyOff(&voices[i]);
        }
    }
}

static void I_OPL_ResumeSong(void)
{
    if (!music_initialized)
    {
        return;
    }

    OPL_SetPaused(0);
}

static void I_OPL_StopSong(void)
{
    unsigned int i;

    if (!music_initialized)
    {
        return;
    }

    OPL_Lock();

    // Stop all playback.

    OPL_ClearCallbacks();

    // Free all voices.

    for (i=0; i<OPL_NUM_VOICES; ++i)
    {
        if (voices[i].channel != NULL)
        {
            VoiceKeyOff(&voices[i]);
            ReleaseVoice(&voices[i]);
        }
    }

    // Free all track data.

    for (i=0; i<num_tracks; ++i)
    {
        MIDI_FreeIterator(tracks[i].iter);
    }

    free(tracks);

    tracks = NULL;
    num_tracks = 0;

    OPL_Unlock();
}

static void I_OPL_UnRegisterSong(void *handle)
{
    if (!music_initialized)
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

    if (!music_initialized)
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
    free(filename);

    return result;
}

// Is the song playing?

static boolean I_OPL_MusicIsPlaying(void)
{
    if (!music_initialized)
    {
        return false;
    }

    return num_tracks > 0;
}

// Shutdown music

static void I_OPL_ShutdownMusic(void)
{
    if (music_initialized)
    {
        // Stop currently-playing track, if there is one:

        I_OPL_StopSong();

        OPL_Shutdown();

        // Release GENMIDI lump

        W_ReleaseLumpName("GENMIDI");

        music_initialized = false;
    }
}

// Initialize music subsystem

static boolean I_OPL_InitMusic(void)
{
    OPL_SetSampleRate(snd_samplerate);

    if (!OPL_Init(opl_io_port))
    {
        printf("Dude.  The Adlib isn't responding.\n");
        return false;
    }

    // Load instruments from GENMIDI lump:

    if (!LoadInstrumentTable())
    {
        OPL_Shutdown();
        return false;
    }

    InitVoices();

    tracks = NULL;
    num_tracks = 0;
    music_initialized = true;

    return true;
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
    NULL,  // Poll
};

//----------------------------------------------------------------------
//
// Development / debug message generation, to help developing GENMIDI
// lumps.
//
//----------------------------------------------------------------------

static int NumActiveChannels(void)
{
    int i;

    for (i = MIDI_CHANNELS_PER_TRACK - 1; i >= 0; --i)
    {
        if (tracks[0].channels[i].instrument != &main_instrs[0])
        {
            return i + 1;
        }
    }

    return 0;
}

static int ChannelInUse(opl_channel_data_t *channel)
{
    opl_voice_t *voice;

    for (voice = voice_alloced_list; voice != NULL; voice = voice->next)
    {
        if (voice->channel == channel)
        {
            return 1;
        }
    }

    return 0;
}

void I_OPL_DevMessages(char *result, size_t result_len)
{
    char tmp[80];
    int instr_num;
    int lines;
    int i;

    if (num_tracks == 0)
    {
        M_snprintf(result, result_len, "No OPL track!");
        return;
    }

    M_snprintf(result, result_len, "Tracks:\n");
    lines = 1;

    for (i = 0; i < NumActiveChannels(); ++i)
    {
        if (tracks[0].channels[i].instrument == NULL)
        {
            continue;
        }

        instr_num = tracks[0].channels[i].instrument - main_instrs;

        M_snprintf(tmp, sizeof(tmp),
                   "chan %i: %c i#%i (%s)\n",
                   i,
                   ChannelInUse(&tracks[0].channels[i]) ? '\'' : ' ',
                   instr_num + 1,
                   main_instr_names[instr_num]);
        M_StringConcat(result, tmp, result_len);

        ++lines;
    }

    M_snprintf(tmp, sizeof(tmp), "\nLast percussion:\n");
    M_StringConcat(result, tmp, result_len);
    lines += 2;

    i = (last_perc_count + PERCUSSION_LOG_LEN - 1) % PERCUSSION_LOG_LEN;

    do {
        if (last_perc[i] == 0)
        {
            break;
        }

        M_snprintf(tmp, sizeof(tmp),
                   "%cp#%i (%s)\n",
                   i == 0 ? '\'' : ' ',
                   last_perc[i],
                   percussion_names[last_perc[i] - 35]);
        M_StringConcat(result, tmp, result_len);
        ++lines;

        i = (i + PERCUSSION_LOG_LEN - 1) % PERCUSSION_LOG_LEN;
    } while (lines < 25 && i != last_perc_count);
}

