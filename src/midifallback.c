//
// Copyright(C) 2022 ceski
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
//      MIDI instrument fallback support
//

#ifdef _WIN32

#include "doomtype.h"
#include "midifile.h"
#include "midifallback.h"

static const byte drums_table[128] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
    0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10,
    0x18, 0x19, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18,
    0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
    0x28, 0x28, 0x28, 0x28, 0x28, 0x28, 0x28, 0x28,
    0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30,
    0x38, 0x38, 0x38, 0x38, 0x38, 0x38, 0x38, 0x38,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7F
};

static byte variation[128][128];
static byte bank_msb[MIDI_CHANNELS_PER_TRACK];
static byte drum_map[MIDI_CHANNELS_PER_TRACK];
static boolean selected[MIDI_CHANNELS_PER_TRACK];

static void UpdateDrumMap(const byte *msg, unsigned int length)
{
    byte idx;
    byte checksum;

    // GS allows drums on any channel using SysEx messages.
    // The message format is F0 followed by:
    //
    // 41 10 42 12 40 <ch> 15 <map> <sum> F7
    //
    // <ch> is [11-19, 10, 1A-1F] for channels 1-16. Note the position of 10.
    // <map> is 00-02 for off (normal part), drum map 1, or drum map 2.
    // <sum> is checksum.

    if (length == 10 &&
        msg[0] == 0x41 && // Roland
        msg[1] == 0x10 && // Device ID
        msg[2] == 0x42 && // GS
        msg[3] == 0x12 && // DT1
        msg[4] == 0x40 && // Address MSB
        msg[6] == 0x15 && // Address LSB
        msg[9] == 0xF7)   // SysEx EOX
    {
        checksum = 128 - ((int)msg[4] + msg[5] + msg[6] + msg[7]) % 128;

        if (msg[8] != checksum)
        {
            return;
        }

        if (msg[5] == 0x10) // Channel 10
        {
            idx = 9;
        }
        else if (msg[5] < 0x1A) // Channels 1-9
        {
            idx = (msg[5] & 0x0F) - 1;
        }
        else // Channels 11-16
        {
            idx = msg[5] & 0x0F;
        }

        drum_map[idx] = msg[7];
    }
}

static boolean GetProgramFallback(byte idx, byte program,
                                  midi_fallback_t *fallback)
{
    if (drum_map[idx] == 0) // Normal channel
    {
        if (bank_msb[idx] == 0 || variation[bank_msb[idx]][program])
        {
            // Found a capital or variation for this bank select MSB.
            selected[idx] = true;
            return false;
        }

        fallback->type = FALLBACK_BANK_MSB;

        if (!selected[idx] || bank_msb[idx] > 63)
        {
            // Fall to capital when no instrument has (successfully)
            // selected this variation or if the variation is above 63.
            fallback->value = 0;
            return true;
        }

        // A previous instrument used this variation but it's not
        // valid for the current instrument. Fall to the next valid
        // "sub-capital" (next variation that is a multiple of 8).
        fallback->value = (bank_msb[idx] / 8) * 8;
        while (fallback->value > 0)
        {
            if (variation[fallback->value][program])
            {
                break;
            }
            fallback->value -= 8;
        }
        return true;
    }
    else // Drums channel
    {
        if (program != drums_table[program])
        {
            // Use drum set from drums fallback table.
            // Drums 0-63 and 127: same as original SC-55 (1.00 - 1.21).
            // Drums 64-126: standard drum set (0).
            fallback->type = FALLBACK_DRUMS;
            fallback->value = drums_table[program];
            selected[idx] = true;
            return true;
        }
    }

    return false;
}

void MIDI_CheckFallback(const midi_event_t *event, midi_fallback_t *fallback,
                        boolean allow_sysex)
{
    byte idx;
    byte program;

    switch ((int)event->event_type)
    {
        case MIDI_EVENT_SYSEX:
            if (allow_sysex)
            {
                UpdateDrumMap(event->data.sysex.data, event->data.sysex.length);
            }
            break;

        case MIDI_EVENT_CONTROLLER:
            idx = event->data.channel.channel;
            switch (event->data.channel.param1)
            {
                case MIDI_CONTROLLER_BANK_SELECT_MSB:
                    bank_msb[idx] = event->data.channel.param2;
                    selected[idx] = false;
                    break;

                case MIDI_CONTROLLER_BANK_SELECT_LSB:
                    selected[idx] = false;
                    if (event->data.channel.param2 > 0)
                    {
                        // Bank select LSB > 0 not supported. This also
                        // preserves user's current SC-XX map.
                        fallback->type = FALLBACK_BANK_LSB;
                        fallback->value = 0;
                        return;
                    }
                    break;

                case EMIDI_CONTROLLER_PROGRAM_CHANGE:
                    program = event->data.channel.param2;
                    if (GetProgramFallback(idx, program, fallback))
                    {
                        return;
                    }
                    break;
            }
            break;

        case MIDI_EVENT_PROGRAM_CHANGE:
            idx = event->data.channel.channel;
            program = event->data.channel.param1;
            if (GetProgramFallback(idx, program, fallback))
            {
                return;
            }
            break;
    }

    fallback->type = FALLBACK_NONE;
    fallback->value = 0;
}

void MIDI_ResetFallback(void)
{
    int i;

    for (i = 0; i < MIDI_CHANNELS_PER_TRACK; i++)
    {
        bank_msb[i] = 0;
        drum_map[i] = 0;
        selected[i] = false;
    }

    // Channel 10 (index 9) is set to drum map 1 by default.
    drum_map[9] = 1;
}

void MIDI_InitFallback(void)
{
    byte program;

    MIDI_ResetFallback();

    // Capital
    for (program = 0; program < 128; program++)
    {
        variation[0][program] = 1;
    }

    // Variation #1
    variation[1][38] = 1;
    variation[1][57] = 1;
    variation[1][60] = 1;
    variation[1][80] = 1;
    variation[1][81] = 1;
    variation[1][98] = 1;
    variation[1][102] = 1;
    variation[1][104] = 1;
    variation[1][120] = 1;
    variation[1][121] = 1;
    variation[1][122] = 1;
    variation[1][123] = 1;
    variation[1][124] = 1;
    variation[1][125] = 1;
    variation[1][126] = 1;
    variation[1][127] = 1;

    // Variation #2
    variation[2][102] = 1;
    variation[2][120] = 1;
    variation[2][122] = 1;
    variation[2][123] = 1;
    variation[2][124] = 1;
    variation[2][125] = 1;
    variation[2][126] = 1;
    variation[2][127] = 1;

    // Variation #3
    variation[3][122] = 1;
    variation[3][123] = 1;
    variation[3][124] = 1;
    variation[3][125] = 1;
    variation[3][126] = 1;
    variation[3][127] = 1;

    // Variation #4
    variation[4][122] = 1;
    variation[4][124] = 1;
    variation[4][125] = 1;
    variation[4][126] = 1;

    // Variation #5
    variation[5][122] = 1;
    variation[5][124] = 1;
    variation[5][125] = 1;
    variation[5][126] = 1;

    // Variation #6
    variation[6][125] = 1;

    // Variation #7
    variation[7][125] = 1;

    // Variation #8
    variation[8][0] = 1;
    variation[8][1] = 1;
    variation[8][2] = 1;
    variation[8][3] = 1;
    variation[8][4] = 1;
    variation[8][5] = 1;
    variation[8][6] = 1;
    variation[8][11] = 1;
    variation[8][12] = 1;
    variation[8][14] = 1;
    variation[8][16] = 1;
    variation[8][17] = 1;
    variation[8][19] = 1;
    variation[8][21] = 1;
    variation[8][24] = 1;
    variation[8][25] = 1;
    variation[8][26] = 1;
    variation[8][27] = 1;
    variation[8][28] = 1;
    variation[8][30] = 1;
    variation[8][31] = 1;
    variation[8][38] = 1;
    variation[8][39] = 1;
    variation[8][40] = 1;
    variation[8][48] = 1;
    variation[8][50] = 1;
    variation[8][61] = 1;
    variation[8][62] = 1;
    variation[8][63] = 1;
    variation[8][80] = 1;
    variation[8][81] = 1;
    variation[8][107] = 1;
    variation[8][115] = 1;
    variation[8][116] = 1;
    variation[8][117] = 1;
    variation[8][118] = 1;
    variation[8][125] = 1;

    // Variation #9
    variation[9][14] = 1;
    variation[9][118] = 1;
    variation[9][125] = 1;

    // Variation #16
    variation[16][0] = 1;
    variation[16][4] = 1;
    variation[16][5] = 1;
    variation[16][6] = 1;
    variation[16][16] = 1;
    variation[16][19] = 1;
    variation[16][24] = 1;
    variation[16][25] = 1;
    variation[16][28] = 1;
    variation[16][39] = 1;
    variation[16][62] = 1;
    variation[16][63] = 1;

    // Variation #24
    variation[24][4] = 1;
    variation[24][6] = 1;

    // Variation #32
    variation[32][16] = 1;
    variation[32][17] = 1;
    variation[32][24] = 1;
    variation[32][52] = 1;

    // CM-64 Map (PCM)
    for (program = 0; program < 64; program++)
    {
        variation[126][program] = 1;
    }

    // CM-64 Map (LA)
    for (program = 0; program < 128; program++)
    {
        variation[127][program] = 1;
    }
}

#endif
