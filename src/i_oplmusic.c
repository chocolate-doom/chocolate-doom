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

//#define TEST

#define MAXMIDLENGTH (96 * 1024)
#define GENMIDI_NUM_INSTRS  128

#define GENMIDI_HEADER          "#OPL_II#"
#define GENMIDI_FLAG_FIXED      0x0001         /* fixed pitch */
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

// Data associated with a channel of a track that is currently playing.

typedef struct
{
    // The instrument currently used for this track.

    genmidi_instr_t *instrument;

    // Volume level

    int volume;
} opl_channel_data_t;

// Data associated with a track that is currently playing.

typedef struct
{
    // Data for each channel.

    opl_channel_data_t channels[MIDI_CHANNELS_PER_TRACK];

    // Track iterator used to read new events.

    midi_track_iter_t *iter;

    // Tempo control variables

    unsigned int ticks_per_beat;
    unsigned int ms_per_beat;
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

    // The channel currently using this voice.
    opl_channel_data_t *channel;

    // The note that this voice is playing.
    unsigned int note;

    // The frequency value being used.
    unsigned int freq;

    // The volume of the note being played on this channel.
    unsigned int note_volume;

    // The current volume that has been set for this channel.
    unsigned int volume;

    // Next in freelist
    opl_voice_t *next;
};

// Operators used by the different voices.

static const int voice_operators[2][OPL_NUM_VOICES] = {
    { 0x00, 0x01, 0x02, 0x08, 0x09, 0x0a, 0x10, 0x11, 0x12 },
    { 0x03, 0x04, 0x05, 0x0b, 0x0c, 0x0d, 0x13, 0x14, 0x15 }
};

// Frequency values to use for each note.

static const unsigned int note_frequencies[] = {

    // These frequencies are only used for the first seven
    // MIDI note values:

    0x158, 0x16d, 0x183, 0x19a, 0x1b2, 0x1cc, 0x1e7, 

    // These frequencies are used repeatedly, cycling around
    // for each octave:

    0x204, 0x223, 0x244, 0x266, 0x28b, 0x2b1,
    0x2da, 0x306, 0x334, 0x365, 0x398, 0x3cf,
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

static boolean music_initialised = false;

//static boolean musicpaused = false;
static int current_music_volume;

// GENMIDI lump instrument data:

static genmidi_instr_t *main_instrs;
static genmidi_instr_t *percussion_instrs;

// Voices:

static opl_voice_t voices[OPL_NUM_VOICES];
static opl_voice_t *voice_free_list;

// Track data for playing tracks:

static opl_track_data_t *tracks;

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

    voice->channel = NULL;
    voice->note = 0;

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

    WriteRegister(OPL_REGS_LEVEL + operator, level);
    WriteRegister(OPL_REGS_TREMOLO + operator, data->tremolo);
    WriteRegister(OPL_REGS_ATTACK + operator, data->attack);
    WriteRegister(OPL_REGS_SUSTAIN + operator, data->sustain);
    WriteRegister(OPL_REGS_WAVEFORM + operator, data->waveform);
}

// Set the instrument for a particular voice.

static void SetVoiceInstrument(opl_voice_t *voice, genmidi_instr_t *instr)
{
    genmidi_voice_t *data;

    voice->current_instr = instr;
    data = &instr->opl2_voice;

    // Doom loads the second operator first, then the first.

    LoadOperatorData(voice->op2, &data->carrier, true);
    LoadOperatorData(voice->op1, &data->modulator, false);

    // Set feedback register that control the connection between the
    // two operators.  Turn on bits in the upper nybble; I think this
    // is for OPL3, where it turns on channel A/B.

    WriteRegister(OPL_REGS_FEEDBACK + voice->index,
                  data->feedback | 0x30);

    // Hack to force a volume update.

    voice->volume = 999;
}

static void SetVoiceVolume(opl_voice_t *voice, unsigned int volume)
{
    unsigned int full_volume;
    unsigned int instr_volume;
    unsigned int reg_volume;

    voice->note_volume = volume;

    // Multiply note volume and channel volume to get the actual volume.

    full_volume = (voice->note_volume * voice->channel->volume) / 127;

    // The volume of each instrument can be controlled via GENMIDI:

    instr_volume = 0x3f - voice->current_instr->opl2_voice.carrier.level;

    // The volume value to use in the register:

    reg_volume = ((instr_volume * volume_mapping_table[full_volume]) / 128);
    reg_volume = (0x3f - reg_volume)
               | voice->current_instr->opl2_voice.carrier.scale;

    // Update the register, if necessary:

    if (voice->volume != reg_volume)
    {
        voice->volume = reg_volume;

        WriteRegister(OPL_REGS_LEVEL + voice->op2, reg_volume);
    }
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

static opl_voice_t *FindVoiceForNote(opl_channel_data_t *channel, int note)
{
    unsigned int i;

    for (i=0; i<OPL_NUM_VOICES; ++i)
    {
        if (voices[i].channel == channel && voices[i].note == note)
        {
            return &voices[i];
        }
    }

    return NULL;
}

static void VoiceNoteOff(opl_voice_t *voice)
{
    WriteRegister(OPL_REGS_FREQ_2 + voice->index, voice->freq >> 8);
}

static void NoteOffEvent(opl_track_data_t *track, midi_event_t *event)
{
    opl_voice_t *voice;
    opl_channel_data_t *channel;

    printf("note off: channel %i, %i, %i\n",
           event->data.channel.channel,
           event->data.channel.param1,
           event->data.channel.param2);

    channel = &track->channels[event->data.channel.channel];

    // Find the voice being used to play the note.

    voice = FindVoiceForNote(channel, event->data.channel.param1);

    if (voice == NULL)
    {
        return;
    }

    VoiceNoteOff(voice);

    // Finished with this voice now.

    ReleaseVoice(voice);
}

// Given a MIDI note number, get the corresponding OPL
// frequency value to use.

static unsigned int FrequencyForNote(unsigned int note)
{
    unsigned int octave;
    unsigned int key_num;

    // The first seven frequencies in the frequencies array are used
    // only for the first seven MIDI notes.  After this, the frequency
    // value loops around the same twelve notes, increasing the
    // octave.

    if (note < 7)
    {
        return note_frequencies[note];
    }
    else
    {
        octave = (note - 7) / 12;
        key_num = (note - 7) % 12;

        return note_frequencies[key_num + 7] | (octave << 10);
    }
}

static void NoteOnEvent(opl_track_data_t *track, midi_event_t *event)
{
    genmidi_instr_t *instrument;
    opl_voice_t *voice;
    opl_channel_data_t *channel;
    unsigned int note;
    unsigned int volume;

    printf("note on: channel %i, %i, %i\n",
           event->data.channel.channel,
           event->data.channel.param1,
           event->data.channel.param2);

    // The channel.

    channel = &track->channels[event->data.channel.channel];
    note = event->data.channel.param1;
    volume = event->data.channel.param2;

    // Percussion channel (10) is treated differently to normal notes.

    if (event->data.channel.channel == 9)
    {
        if (note < 35 || note > 81)
        {
            return;
        }

        instrument = &percussion_instrs[note - 35];
    }
    else
    {
        instrument = channel->instrument;
    }

    // Find a voice to use for this new note.

    voice = GetFreeVoice();

    if (voice == NULL)
    {
        printf("\tno free voice\n");
        return;
    }

    voice->channel = channel;
    voice->note = note;

    // Program the voice with the instrument data:

    SetVoiceInstrument(voice, instrument);

    // Set the volume level.

    SetVoiceVolume(voice, volume);

    // Fixed pitch?

    if ((instrument->flags & GENMIDI_FLAG_FIXED) != 0)
    {
        note = instrument->fixed_note;
    }

    // Write the frequency value to turn the note on.

    voice->freq = FrequencyForNote(note);

    WriteRegister(OPL_REGS_FREQ_1 + voice->index, voice->freq & 0xff);
    WriteRegister(OPL_REGS_FREQ_2 + voice->index, (voice->freq >> 8) | 0x20);
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

static void ControllerEvent(opl_track_data_t *track, midi_event_t *event)
{
    unsigned int controller;
    unsigned int param;
    opl_channel_data_t *channel;

    printf("change controller: channel %i, %i, %i\n",
           event->data.channel.channel,
           event->data.channel.param1,
           event->data.channel.param2);

    channel = &track->channels[event->data.channel.channel];
    controller = event->data.channel.param1;
    param = event->data.channel.param2;

    switch (controller)
    {
        case MIDI_CONTROLLER_MAIN_VOLUME:
            SetChannelVolume(channel, param);
            break;

        case MIDI_CONTROLLER_PAN:
            break;

        default:
            fprintf(stderr, "Unknown MIDI controller type: %i\n", controller);
            break;
    }
}

// Process a MIDI event from a track.

static void ProcessEvent(opl_track_data_t *track, midi_event_t *event)
{
    switch (event->event_type)
    {
        case MIDI_EVENT_NOTE_OFF:
            NoteOffEvent(track, event);
            break;

        case MIDI_EVENT_NOTE_ON:
            NoteOnEvent(track, event);
            break;

        case MIDI_EVENT_CONTROLLER:
            ControllerEvent(track, event);
            break;

        case MIDI_EVENT_PROGRAM_CHANGE:
            ProgramChangeEvent(track, event);
            break;

        default:
            fprintf(stderr, "Unknown MIDI event type %i\n", event->event_type);
            break;
    }
}

static void ScheduleTrack(opl_track_data_t *track);

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

    // Reschedule the callback for the next event in the track.

    ScheduleTrack(track);
}

static void ScheduleTrack(opl_track_data_t *track)
{
    unsigned int nticks;
    unsigned int ms;
    static int total = 0;

    // Get the number of milliseconds until the next event.

    nticks = MIDI_GetDeltaTime(track->iter);
    ms = (nticks * track->ms_per_beat) / track->ticks_per_beat;
    total += ms;

    // Set a timer to be invoked when the next event is
    // ready to play.

    OPL_SetCallback(ms, TrackTimerCallback, track);
}

// Initialise a channel.

static void InitChannel(opl_track_data_t *track, opl_channel_data_t *channel)
{
    // TODO: Work out sensible defaults?

    channel->instrument = &main_instrs[0];
    channel->volume = 127;
}

// Start a MIDI track playing:

static void StartTrack(midi_file_t *file, unsigned int track_num)
{
    opl_track_data_t *track;
    unsigned int i;

    track = &tracks[track_num];
    track->iter = MIDI_IterateTrack(file, track_num);
    track->ticks_per_beat = MIDI_GetFileTimeDivision(file);

    // Default is 120 bpm.
    // TODO: this is wrong

    track->ms_per_beat = 500 * 260;

    for (i=0; i<MIDI_CHANNELS_PER_TRACK; ++i)
    {
        InitChannel(track, &track->channels[i]);
    }

    // Schedule the first event.

    ScheduleTrack(track);
}

// Start playing a mid

static void I_OPL_PlaySong(void *handle, int looping)
{
    midi_file_t *file;
    unsigned int i;

    if (!music_initialised || handle == NULL)
    {
        return;
    }

    file = handle;

    // Allocate track data.

    tracks = malloc(MIDI_NumTracks(file) * sizeof(opl_track_data_t));

    for (i=0; i<MIDI_NumTracks(file); ++i)
    {
        StartTrack(file, i);
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
    unsigned int i;

    if (!music_initialised)
    {
        return;
    }

    // Stop all playback.

    OPL_ClearCallbacks();

    // Free all voices.

    for (i=0; i<OPL_NUM_VOICES; ++i)
    {
        if (voices[i].channel != NULL)
        {
            VoiceNoteOff(&voices[i]);
            ReleaseVoice(&voices[i]);
        }
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

