//
// Copyright(C) 2021-2022 Roman Fomin
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
//      Windows native MIDI

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <mmsystem.h>
#include <mmreg.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "doomtype.h"
#include "i_sound.h"
#include "i_system.h"
#include "m_misc.h"
#include "memio.h"
#include "mus2mid.h"
#include "midifile.h"
#include "midifallback.h"

enum
{
    COMP_VANILLA,
    COMP_STANDARD,
    COMP_FULL,
};

enum
{
    RESET_TYPE_NONE,
    RESET_TYPE_GM,
    RESET_TYPE_GS,
    RESET_TYPE_XG,
};

char *winmm_midi_device = NULL;
int winmm_complevel = COMP_STANDARD;
int winmm_reset_type = RESET_TYPE_GM;
int winmm_reset_delay = 0;

static const byte gm_system_on[] = {
    0xF0, 0x7E, 0x7F, 0x09, 0x01, 0xF7
};

static const byte gs_reset[] = {
    0xF0, 0x41, 0x10, 0x42, 0x12, 0x40, 0x00, 0x7F, 0x00, 0x41, 0xF7
};

static const byte xg_system_on[] = {
    0xF0, 0x43, 0x10, 0x4C, 0x00, 0x00, 0x7E, 0x00, 0xF7
};

static const byte ff_loopStart[] = {'l', 'o', 'o', 'p', 'S', 't', 'a', 'r', 't'};
static const byte ff_loopEnd[] = {'l', 'o', 'o', 'p', 'E', 'n', 'd'};

static boolean use_fallback;

#define DEFAULT_VOLUME 100
static byte channel_volume[MIDI_CHANNELS_PER_TRACK];
static float volume_factor = 0.0f;
static boolean update_volume = false;

typedef enum
{
    STATE_STOPPED,
    STATE_PLAYING,
    STATE_PAUSING,
    STATE_PAUSED
} win_midi_state_t;

static win_midi_state_t win_midi_state;

static DWORD timediv;
static DWORD tempo;

static UINT MidiDevice;
static HMIDISTRM hMidiStream;
static MIDIHDR MidiStreamHdr;
static HANDLE hBufferReturnEvent;
static HANDLE hExitEvent;
static HANDLE hPlayerThread;

// This is a reduced Windows MIDIEVENT structure for MEVT_F_SHORT
// type of events.

typedef struct
{
    DWORD dwDeltaTime;
    DWORD dwStreamID; // always 0
    DWORD dwEvent;
} native_event_t;

typedef struct
{
    midi_track_iter_t *iter;
    unsigned int elapsed_time;
    unsigned int saved_elapsed_time;
    boolean end_of_track;
    boolean saved_end_of_track;
    unsigned int emidi_device_flags;
    boolean emidi_designated;
    boolean emidi_program;
    boolean emidi_volume;
    int emidi_loop_count;
} win_midi_track_t;

typedef struct
{
    win_midi_track_t *tracks;
    unsigned int elapsed_time;
    unsigned int saved_elapsed_time;
    unsigned int num_tracks;
    boolean registered;
    boolean looping;
    boolean ff_loop;
    boolean ff_restart;
    boolean rpg_loop;
} win_midi_song_t;

static win_midi_song_t song;

#define BUFFER_INITIAL_SIZE 8192

#define PLAYER_THREAD_WAIT_TIME 3000

typedef struct
{
    byte *data;
    unsigned int size;
    unsigned int position;
} buffer_t;

static buffer_t buffer;

// Maximum of 4 events in the buffer for faster volume updates.

#define STREAM_MAX_EVENTS   4

#define MAKE_EVT(a, b, c, d)                                                   \
    ((DWORD)(a) | ((DWORD)(b) << 8) | ((DWORD)(c) << 16) | ((DWORD)(d) << 24))

#define PADDED_SIZE(x) (((x) + sizeof(DWORD) - 1) & ~(sizeof(DWORD) - 1))

static boolean initial_playback = false;

// Check for midiStream errors.

static void MidiError(const char *prefix, DWORD dwError)
{
    wchar_t werror[MAXERRORLENGTH];
    MMRESULT mmr;

    mmr = midiOutGetErrorTextW(dwError, (LPWSTR) werror, MAXERRORLENGTH);
    if (mmr == MMSYSERR_NOERROR)
    {
        char *error = M_ConvertWideToUtf8(werror);
        fprintf(stderr, "%s: %s.\n", prefix, error);
        free(error);
    }
    else
    {
        fprintf(stderr, "%s: Unknown midiStream error.\n", prefix);
    }
}

// midiStream callback.

static void CALLBACK MidiStreamProc(HMIDIOUT hMidi, UINT uMsg,
                                    DWORD_PTR dwInstance, DWORD_PTR dwParam1,
                                    DWORD_PTR dwParam2)
{
    if (uMsg == MOM_DONE)
    {
        SetEvent(hBufferReturnEvent);
    }
}

static void AllocateBuffer(const unsigned int size)
{
    MIDIHDR *hdr = &MidiStreamHdr;
    MMRESULT mmr;

    if (buffer.data)
    {
        // Windows doesn't always immediately clear the MHDR_INQUEUE flag, even
        // after midiStreamStop() is called. There doesn't seem to be any side
        // effect to just forcing the flag off.
        hdr->dwFlags &= ~MHDR_INQUEUE;
        mmr = midiOutUnprepareHeader((HMIDIOUT)hMidiStream, hdr, sizeof(MIDIHDR));
        if (mmr != MMSYSERR_NOERROR)
        {
            MidiError("midiOutUnprepareHeader", mmr);
        }
    }

    buffer.size = PADDED_SIZE(size);
    buffer.data = I_Realloc(buffer.data, buffer.size);

    hdr->lpData = (LPSTR)buffer.data;
    hdr->dwBytesRecorded = 0;
    hdr->dwBufferLength = buffer.size;
    mmr = midiOutPrepareHeader((HMIDIOUT)hMidiStream, hdr, sizeof(MIDIHDR));
    if (mmr != MMSYSERR_NOERROR)
    {
        MidiError("midiOutPrepareHeader", mmr);
    }
}

static void WriteBufferPad(void)
{
    unsigned int padding = PADDED_SIZE(buffer.position);
    memset(buffer.data + buffer.position, 0, padding - buffer.position);
    buffer.position = padding;
}

static void WriteBuffer(const byte *ptr, unsigned int size)
{
    if (buffer.position + size >= buffer.size)
    {
        AllocateBuffer(size + buffer.size * 2);
    }

    memcpy(buffer.data + buffer.position, ptr, size);
    buffer.position += size;
}

static void StreamOut(void)
{
    MIDIHDR *hdr = &MidiStreamHdr;
    MMRESULT mmr;

    hdr->lpData = (LPSTR)buffer.data;
    hdr->dwBytesRecorded = buffer.position;

    mmr = midiStreamOut(hMidiStream, hdr, sizeof(MIDIHDR));
    if (mmr != MMSYSERR_NOERROR)
    {
        MidiError("midiStreamOut", mmr);
    }
}

static void SendShortMsg(unsigned int delta_time, byte status, byte channel,
                         byte param1, byte param2)
{
    native_event_t native_event;
    native_event.dwDeltaTime = delta_time;
    native_event.dwStreamID = 0;
    native_event.dwEvent = MAKE_EVT(status | channel, param1, param2, MEVT_SHORTMSG);
    WriteBuffer((byte *)&native_event, sizeof(native_event_t));
}

static void SendChannelMsg(unsigned int delta_time, const midi_event_t *event,
                           boolean use_param2)
{
    SendShortMsg(delta_time, event->event_type, event->data.channel.channel,
                 event->data.channel.param1,
                 use_param2 ? event->data.channel.param2 : 0);
}

static void SendLongMsg(unsigned int delta_time, const byte *ptr,
                        unsigned int length)
{
    native_event_t native_event;
    native_event.dwDeltaTime = delta_time;
    native_event.dwStreamID = 0;
    native_event.dwEvent = MAKE_EVT(length, 0, 0, MEVT_LONGMSG);
    WriteBuffer((byte *)&native_event, sizeof(native_event_t));
    WriteBuffer(ptr, length);
    WriteBufferPad();
}

static void SendNullRPN(unsigned int delta_time, const midi_event_t *event)
{
    const byte channel = event->data.channel.channel;
    SendShortMsg(delta_time, MIDI_EVENT_CONTROLLER, channel,
                 MIDI_CONTROLLER_RPN_LSB, MIDI_RPN_NULL);
    SendShortMsg(0, MIDI_EVENT_CONTROLLER, channel,
                 MIDI_CONTROLLER_RPN_MSB, MIDI_RPN_NULL);
}

static void SendNOPMsg(unsigned int delta_time)
{
    native_event_t native_event;
    native_event.dwDeltaTime = delta_time;
    native_event.dwStreamID = 0;
    native_event.dwEvent = MAKE_EVT(0, 0, 0, MEVT_NOP);
    WriteBuffer((byte *)&native_event, sizeof(native_event_t));
}

static void SendDelayMsg(unsigned int time_ms)
{
    // Convert ms to ticks (see "Standard MIDI Files 1.0" page 14).
    const unsigned int ticks = (float) time_ms * 1000 * timediv / tempo + 0.5f;
    SendNOPMsg(ticks);
}

static void UpdateTempo(unsigned int delta_time, const midi_event_t *event)
{
    native_event_t native_event;

    tempo = MAKE_EVT(event->data.meta.data[2], event->data.meta.data[1],
                     event->data.meta.data[0], 0);

    native_event.dwDeltaTime = delta_time;
    native_event.dwStreamID = 0;
    native_event.dwEvent = MAKE_EVT(tempo, 0, 0, MEVT_TEMPO);
    WriteBuffer((byte *)&native_event, sizeof(native_event_t));
}

static void SendManualVolumeMsg(unsigned int delta_time, byte channel,
                                byte volume)
{
    unsigned int scaled_volume;

    scaled_volume = volume * volume_factor + 0.5f;

    if (scaled_volume > 127)
    {
        scaled_volume = 127;
    }

    SendShortMsg(delta_time, MIDI_EVENT_CONTROLLER, channel,
                 MIDI_CONTROLLER_VOLUME_MSB, scaled_volume);

    channel_volume[channel] = volume;
}

static void SendVolumeMsg(unsigned int delta_time, const midi_event_t *event)
{
    SendManualVolumeMsg(delta_time, event->data.channel.channel,
                        event->data.channel.param2);
}

static void UpdateVolume(void)
{
    int i;

    for (i = 0; i < MIDI_CHANNELS_PER_TRACK; ++i)
    {
        SendManualVolumeMsg(0, i, channel_volume[i]);
    }
}

static void ResetVolume(void)
{
    int i;

    for (i = 0; i < MIDI_CHANNELS_PER_TRACK; ++i)
    {
        SendManualVolumeMsg(0, i, DEFAULT_VOLUME);
    }
}

static void SendNotesSoundOff(void)
{
    int i;

    for (i = 0; i < MIDI_CHANNELS_PER_TRACK; ++i)
    {
        SendShortMsg(0, MIDI_EVENT_CONTROLLER, i, MIDI_CONTROLLER_ALL_NOTES_OFF, 0);
        SendShortMsg(0, MIDI_EVENT_CONTROLLER, i, MIDI_CONTROLLER_ALL_SOUND_OFF, 0);
    }
}

static void ResetControllers(void)
{
    int i;

    for (i = 0; i < MIDI_CHANNELS_PER_TRACK; ++i)
    {
        // Reset commonly used controllers.
        SendShortMsg(0, MIDI_EVENT_CONTROLLER, i, MIDI_CONTROLLER_RESET_ALL_CTRLS, 0);
        SendShortMsg(0, MIDI_EVENT_CONTROLLER, i, MIDI_CONTROLLER_PAN, 64);
        SendShortMsg(0, MIDI_EVENT_CONTROLLER, i, MIDI_CONTROLLER_BANK_SELECT_MSB, 0);
        SendShortMsg(0, MIDI_EVENT_CONTROLLER, i, MIDI_CONTROLLER_BANK_SELECT_LSB, 0);
        SendShortMsg(0, MIDI_EVENT_PROGRAM_CHANGE, i, 0, 0);
        SendShortMsg(0, MIDI_EVENT_CONTROLLER, i, MIDI_CONTROLLER_REVERB, 40);
        SendShortMsg(0, MIDI_EVENT_CONTROLLER, i, MIDI_CONTROLLER_CHORUS, 0);
    }
}

static void ResetPitchBendSensitivity(void)
{
    int i;

    for (i = 0; i < MIDI_CHANNELS_PER_TRACK; ++i)
    {
        // Set RPN MSB/LSB to pitch bend sensitivity.
        SendShortMsg(0, MIDI_EVENT_CONTROLLER, i, MIDI_CONTROLLER_RPN_LSB, 0);
        SendShortMsg(0, MIDI_EVENT_CONTROLLER, i, MIDI_CONTROLLER_RPN_MSB, 0);

        // Reset pitch bend sensitivity to +/- 2 semitones and 0 cents.
        SendShortMsg(0, MIDI_EVENT_CONTROLLER, i, MIDI_CONTROLLER_DATA_ENTRY_MSB, 2);
        SendShortMsg(0, MIDI_EVENT_CONTROLLER, i, MIDI_CONTROLLER_DATA_ENTRY_LSB, 0);

        // Set RPN MSB/LSB to null value after data entry.
        SendShortMsg(0, MIDI_EVENT_CONTROLLER, i, MIDI_CONTROLLER_RPN_LSB, 127);
        SendShortMsg(0, MIDI_EVENT_CONTROLLER, i, MIDI_CONTROLLER_RPN_MSB, 127);
    }
}

static void ResetDevice(void)
{
    // Send notes/sound off prior to reset to prevent volume spikes.
    SendNotesSoundOff();

    MIDI_ResetFallback();
    use_fallback = false;

    switch (winmm_reset_type)
    {
        case RESET_TYPE_NONE:
            ResetControllers();
            break;

        case RESET_TYPE_GS:
            SendLongMsg(0, gs_reset, sizeof(gs_reset));
            use_fallback = (winmm_complevel != COMP_VANILLA);
            break;

        case RESET_TYPE_XG:
            SendLongMsg(0, xg_system_on, sizeof(xg_system_on));
            break;

        default:
            SendLongMsg(0, gm_system_on, sizeof(gm_system_on));
            break;
    }

    // MS GS Wavetable Synth doesn't reset pitch bend sensitivity.
    ResetPitchBendSensitivity();

    // Reset volume (initial playback or on shutdown if no SysEx reset).
    if (initial_playback || winmm_reset_type == RESET_TYPE_NONE)
    {
        // Scale by slider on initial playback, max on shutdown.
        volume_factor = initial_playback ? volume_factor : 1.0f;
        ResetVolume();
    }

    // Send delay after reset. This is for hardware devices only (e.g. SC-55).
    if (winmm_reset_delay > 0)
    {
        SendDelayMsg(winmm_reset_delay);
    }
}

// Normally, volume is controlled by channel volume messages. Roland defined a
// special SysEx message called "part level" that is equivalent to this. MS GS
// Wavetable Synth ignores these messages, but other MIDI devices support them.

static boolean IsPartLevel(const byte *msg, unsigned int length)
{
    if (length == 10 &&
        msg[0] == 0x41 && // Roland
        msg[2] == 0x42 && // GS
        msg[3] == 0x12 && // DT1
        msg[4] == 0x40 && // Address MSB
        msg[5] >= 0x10 && // Address
        msg[5] <= 0x1F && // Address
        msg[6] == 0x19 && // Address LSB
        msg[9] == 0xF7)   // SysEx EOX
    {
        const byte checksum =
            128 - ((int) msg[4] + msg[5] + msg[6] + msg[7]) % 128;

        if (msg[8] == checksum)
        {
            // GS Part Level (aka Channel Volume)
            // 41 <dev> 42 12 40 <ch> 19 <vol> <sum> F7
            return true;
        }
    }

    return false;
}

static boolean IsSysExReset(const byte *msg, unsigned int length)
{
    if (length < 5)
    {
        return false;
    }

    switch (msg[0])
    {
        case 0x41: // Roland
            switch (msg[2])
            {
                case 0x42: // GS
                    switch (msg[3])
                    {
                        case 0x12: // DT1
                            if (length == 10 &&
                                msg[4] == 0x00 &&  // Address MSB
                                msg[5] == 0x00 &&  // Address
                                msg[6] == 0x7F &&  // Address LSB
                              ((msg[7] == 0x00 &&  // Data     (MODE-1)
                                msg[8] == 0x01) || // Checksum (MODE-1)
                               (msg[7] == 0x01 &&  // Data     (MODE-2)
                                msg[8] == 0x00)))  // Checksum (MODE-2)
                            {
                                // SC-88 System Mode Set
                                // 41 <dev> 42 12 00 00 7F 00 01 F7 (MODE-1)
                                // 41 <dev> 42 12 00 00 7F 01 00 F7 (MODE-2)
                                return true;
                            }
                            else if (length == 10 &&
                                     msg[4] == 0x40 && // Address MSB
                                     msg[5] == 0x00 && // Address
                                     msg[6] == 0x7F && // Address LSB
                                     msg[7] == 0x00 && // Data (GS Reset)
                                     msg[8] == 0x41)   // Checksum
                            {
                                // GS Reset
                                // 41 <dev> 42 12 40 00 7F 00 41 F7
                                return true;
                            }
                            break;
                    }
                    break;
            }
            break;

        case 0x43: // Yamaha
            switch (msg[2])
            {
                case 0x2B: // TG300
                    if (length == 9 &&
                        msg[3] == 0x00 && // Start Address b20 - b14
                        msg[4] == 0x00 && // Start Address b13 - b7
                        msg[5] == 0x7F && // Start Address b6 - b0
                        msg[6] == 0x00 && // Data
                        msg[7] == 0x01)   // Checksum
                    {
                        // TG300 All Parameter Reset
                        // 43 <dev> 2B 00 00 7F 00 01 F7
                        return true;
                    }
                    break;

                case 0x4C: // XG
                    if (length == 8 &&
                        msg[3] == 0x00 &&  // Address High
                        msg[4] == 0x00 &&  // Address Mid
                       (msg[5] == 0x7E ||  // Address Low (System On)
                        msg[5] == 0x7F) && // Address Low (All Parameter Reset)
                        msg[6] == 0x00)    // Data
                    {
                        // XG System On, XG All Parameter Reset
                        // 43 <dev> 4C 00 00 7E 00 F7
                        // 43 <dev> 4C 00 00 7F 00 F7
                        return true;
                    }
                    break;
            }
            break;

        case 0x7E: // Universal Non-Real Time
            switch (msg[2])
            {
                case 0x09: // General Midi
                    if (length == 5 &&
                       (msg[3] == 0x01 || // GM System On
                        msg[3] == 0x02 || // GM System Off
                        msg[3] == 0x03))  // GM2 System On
                    {
                        // GM System On/Off, GM2 System On
                        // 7E <dev> 09 01 F7
                        // 7E <dev> 09 02 F7
                        // 7E <dev> 09 03 F7
                        return true;
                    }
                    break;
            }
            break;
    }
    return false;
}

static void SendSysExMsg(unsigned int delta_time, const midi_event_t *event)
{
    native_event_t native_event;
    const byte event_type = MIDI_EVENT_SYSEX;
    const byte *data = event->data.sysex.data;
    const unsigned int length = event->data.sysex.length;

    if (IsPartLevel(data, length))
    {
        byte channel;

        // Convert "block number" to a channel number.
        if (data[5] == 0x10) // Channel 10
        {
            channel = 9;
        }
        else if (data[5] < 0x1A) // Channels 1-9
        {
            channel = (data[5] & 0x0F) - 1;
        }
        else // Channels 11-16
        {
            channel = data[5] & 0x0F;
        }

        // Replace SysEx part level message with channel volume message.
        SendManualVolumeMsg(delta_time, channel, data[7]);
        return;
    }

    // Send the SysEx message.
    native_event.dwDeltaTime = delta_time;
    native_event.dwStreamID = 0;
    native_event.dwEvent = MAKE_EVT(length + sizeof(byte), 0, 0, MEVT_LONGMSG);
    WriteBuffer((byte *)&native_event, sizeof(native_event_t));
    WriteBuffer(&event_type, sizeof(byte));
    WriteBuffer(data, length);
    WriteBufferPad();

    if (IsSysExReset(data, length))
    {
        // SysEx reset also resets volume. Take the default channel volumes
        // and scale them by the user's volume slider.
        ResetVolume();

        // Disable instrument fallback and give priority to MIDI file. Fallback
        // assumes GS (SC-55 level) and the MIDI file could be GM, GM2, XG, or
        // GS (SC-88 or higher). Preserve the composer's intent.
        if (use_fallback)
        {
            MIDI_ResetFallback();
            use_fallback = false;
        }
    }
}

static void SendProgramMsg(unsigned int delta_time, byte channel, byte program,
                           const midi_fallback_t *fallback)
{
    switch ((int)fallback->type)
    {
        case FALLBACK_BANK_MSB:
            SendShortMsg(delta_time, MIDI_EVENT_CONTROLLER, channel,
                         MIDI_CONTROLLER_BANK_SELECT_MSB, fallback->value);
            SendShortMsg(0, MIDI_EVENT_PROGRAM_CHANGE, channel, program, 0);
            break;

        case FALLBACK_DRUMS:
            SendShortMsg(delta_time, MIDI_EVENT_PROGRAM_CHANGE, channel,
                         fallback->value, 0);
            break;

        default:
            SendShortMsg(delta_time, MIDI_EVENT_PROGRAM_CHANGE, channel,
                         program, 0);
            break;
    }
}

static void SetLoopPoint(void)
{
    unsigned int i;

    for (i = 0; i < song.num_tracks; ++i)
    {
        MIDI_SetLoopPoint(song.tracks[i].iter);
        song.tracks[i].saved_end_of_track = song.tracks[i].end_of_track;
        song.tracks[i].saved_elapsed_time = song.tracks[i].elapsed_time;
    }
    song.saved_elapsed_time = song.elapsed_time;
}

static void CheckFFLoop(const midi_event_t *event)
{
    if (event->data.meta.length == sizeof(ff_loopStart) &&
        !memcmp(event->data.meta.data, ff_loopStart, sizeof(ff_loopStart)))
    {
        SetLoopPoint();
        song.ff_loop = true;
    }
    else if (song.ff_loop && event->data.meta.length == sizeof(ff_loopEnd) &&
             !memcmp(event->data.meta.data, ff_loopEnd, sizeof(ff_loopEnd)))
    {
        song.ff_restart = true;
    }
}

static void SendEMIDI(unsigned int delta_time, const midi_event_t *event,
                      win_midi_track_t *track, const midi_fallback_t *fallback)
{
    unsigned int i;
    unsigned int flag;
    int count;

    switch ((int) event->event_type)
    {
        case EMIDI_CONTROLLER_TRACK_DESIGNATION:
            if (track->elapsed_time < timediv)
            {
                flag = event->data.channel.param2;

                if (flag == EMIDI_DEVICE_ALL)
                {
                    track->emidi_device_flags = UINT_MAX;
                    track->emidi_designated = true;
                }
                else if (flag <= EMIDI_DEVICE_ULTRASOUND)
                {
                    track->emidi_device_flags |= 1 << flag;
                    track->emidi_designated = true;
                }
            }
            SendNOPMsg(delta_time);
            break;

        case EMIDI_CONTROLLER_TRACK_EXCLUSION:
            if (song.rpg_loop)
            {
                SetLoopPoint();
            }
            else if (track->elapsed_time < timediv)
            {
                flag = event->data.channel.param2;

                if (!track->emidi_designated)
                {
                    track->emidi_device_flags = UINT_MAX;
                    track->emidi_designated = true;
                }

                if (flag <= EMIDI_DEVICE_ULTRASOUND)
                {
                    track->emidi_device_flags &= ~(1 << flag);
                }
            }
            SendNOPMsg(delta_time);
            break;

        case EMIDI_CONTROLLER_PROGRAM_CHANGE:
            if (track->emidi_program || track->elapsed_time < timediv)
            {
                track->emidi_program = true;
                SendProgramMsg(delta_time, event->data.channel.channel,
                               event->data.channel.param2, fallback);
            }
            else
            {
                SendNOPMsg(delta_time);
            }
            break;

        case EMIDI_CONTROLLER_VOLUME:
            if (track->emidi_volume || track->elapsed_time < timediv)
            {
                track->emidi_volume = true;
                SendVolumeMsg(delta_time, event);
            }
            else
            {
                SendNOPMsg(delta_time);
            }
            break;

        case EMIDI_CONTROLLER_LOOP_BEGIN:
            count = event->data.channel.param2;
            count = (count == 0) ? (-1) : count;
            track->emidi_loop_count = count;
            MIDI_SetLoopPoint(track->iter);
            SendNOPMsg(delta_time);
            break;

        case EMIDI_CONTROLLER_LOOP_END:
            if (event->data.channel.param2 == EMIDI_LOOP_FLAG)
            {
                if (track->emidi_loop_count != 0)
                {
                    MIDI_RestartAtLoopPoint(track->iter);
                }

                if (track->emidi_loop_count > 0)
                {
                    track->emidi_loop_count--;
                }
            }
            SendNOPMsg(delta_time);
            break;

        case EMIDI_CONTROLLER_GLOBAL_LOOP_BEGIN:
            count = event->data.channel.param2;
            count = (count == 0) ? (-1) : count;
            for (i = 0; i < song.num_tracks; ++i)
            {
                song.tracks[i].emidi_loop_count = count;
                MIDI_SetLoopPoint(song.tracks[i].iter);
                song.tracks[i].saved_end_of_track = song.tracks[i].end_of_track;
                song.tracks[i].saved_elapsed_time = song.tracks[i].elapsed_time;
            }
            song.saved_elapsed_time = song.elapsed_time;
            SendNOPMsg(delta_time);
            break;

        case EMIDI_CONTROLLER_GLOBAL_LOOP_END:
            if (event->data.channel.param2 == EMIDI_LOOP_FLAG)
            {
                for (i = 0; i < song.num_tracks; ++i)
                {
                    if (song.tracks[i].emidi_loop_count != 0)
                    {
                        MIDI_RestartAtLoopPoint(song.tracks[i].iter);
                        song.tracks[i].end_of_track =
                            song.tracks[i].saved_end_of_track;
                        song.tracks[i].elapsed_time =
                            song.tracks[i].saved_elapsed_time;
                        song.elapsed_time = song.saved_elapsed_time;
                    }

                    if (song.tracks[i].emidi_loop_count > 0)
                    {
                        song.tracks[i].emidi_loop_count--;
                    }
                }
            }
            SendNOPMsg(delta_time);
            break;

        default:
            SendNOPMsg(delta_time);
            break;
    }
}

static void SendMetaMsg(unsigned int delta_time, const midi_event_t *event,
                        win_midi_track_t *track)
{
    switch (event->data.meta.type)
    {
        case MIDI_META_END_OF_TRACK:
            track->end_of_track = true;
            SendNOPMsg(delta_time);
            break;

        case MIDI_META_SET_TEMPO:
            UpdateTempo(delta_time, event);
            break;

        case MIDI_META_MARKER:
            if (winmm_complevel != COMP_VANILLA)
            {
                CheckFFLoop(event);
            }
            SendNOPMsg(delta_time);
            break;

        default:
            SendNOPMsg(delta_time);
            break;
    }
}

static boolean AddToBuffer_Vanilla(unsigned int delta_time,
                                   const midi_event_t *event,
                                   win_midi_track_t *track)
{
    switch ((int) event->event_type)
    {
        case MIDI_EVENT_SYSEX:
            SendNOPMsg(delta_time);
            return false;

        case MIDI_EVENT_META:
            SendMetaMsg(delta_time, event, track);
            break;

        case MIDI_EVENT_CONTROLLER:
            switch (event->data.channel.param1)
            {
                case MIDI_CONTROLLER_BANK_SELECT_MSB:
                case MIDI_CONTROLLER_BANK_SELECT_LSB:
                    // DMX has broken bank select support and runs in GM mode.
                    SendChannelMsg(delta_time, event, false);
                    break;

                case MIDI_CONTROLLER_MODULATION:
                case MIDI_CONTROLLER_PAN:
                case MIDI_CONTROLLER_EXPRESSION:
                case MIDI_CONTROLLER_HOLD1_PEDAL:
                case MIDI_CONTROLLER_SOFT_PEDAL:
                case MIDI_CONTROLLER_REVERB:
                case MIDI_CONTROLLER_CHORUS:
                case MIDI_CONTROLLER_ALL_SOUND_OFF:
                case MIDI_CONTROLLER_ALL_NOTES_OFF:
                    SendChannelMsg(delta_time, event, true);
                    break;

                case MIDI_CONTROLLER_VOLUME_MSB:
                    SendVolumeMsg(delta_time, event);
                    break;

                case MIDI_CONTROLLER_RESET_ALL_CTRLS:
                    // MS GS Wavetable Synth resets volume if param2 isn't zero.
                    SendChannelMsg(delta_time, event, false);
                    break;

                default:
                    SendNOPMsg(delta_time);
                    break;
            }
            break;

        case MIDI_EVENT_NOTE_OFF:
        case MIDI_EVENT_NOTE_ON:
        case MIDI_EVENT_PITCH_BEND:
            SendChannelMsg(delta_time, event, true);
            break;

        case MIDI_EVENT_PROGRAM_CHANGE:
            SendChannelMsg(delta_time, event, false);
            break;

        default:
            SendNOPMsg(delta_time);
            break;
    }

    return true;
}

static boolean AddToBuffer_Standard(unsigned int delta_time,
                                    const midi_event_t *event,
                                    win_midi_track_t *track)
{
    midi_fallback_t fallback = {FALLBACK_NONE, 0};

    if (use_fallback)
    {
        MIDI_CheckFallback(event, &fallback, winmm_complevel == COMP_FULL);
    }

    switch ((int) event->event_type)
    {
        case MIDI_EVENT_SYSEX:
            if (winmm_complevel == COMP_FULL)
            {
                SendSysExMsg(delta_time, event);
            }
            else
            {
                SendNOPMsg(delta_time);
            }
            return false;

        case MIDI_EVENT_META:
            SendMetaMsg(delta_time, event, track);
            return true;
    }

    if (track->emidi_designated &&
        (EMIDI_DEVICE_GENERAL_MIDI & ~track->emidi_device_flags))
    {
        // Send NOP if this device has been excluded from this track.
        SendNOPMsg(delta_time);
        return true;
    }

    switch ((int) event->event_type)
    {
        case MIDI_EVENT_CONTROLLER:
            switch (event->data.channel.param1)
            {
                case MIDI_CONTROLLER_BANK_SELECT_MSB:
                case MIDI_CONTROLLER_MODULATION:
                case MIDI_CONTROLLER_DATA_ENTRY_MSB:
                case MIDI_CONTROLLER_PAN:
                case MIDI_CONTROLLER_EXPRESSION:
                case MIDI_CONTROLLER_DATA_ENTRY_LSB:
                case MIDI_CONTROLLER_HOLD1_PEDAL:
                case MIDI_CONTROLLER_SOFT_PEDAL:
                case MIDI_CONTROLLER_REVERB:
                case MIDI_CONTROLLER_CHORUS:
                case MIDI_CONTROLLER_ALL_SOUND_OFF:
                case MIDI_CONTROLLER_ALL_NOTES_OFF:
                case MIDI_CONTROLLER_POLY_MODE_OFF:
                case MIDI_CONTROLLER_POLY_MODE_ON:
                    SendChannelMsg(delta_time, event, true);
                    break;

                case MIDI_CONTROLLER_VOLUME_MSB:
                    if (track->emidi_volume)
                    {
                        SendNOPMsg(delta_time);
                    }
                    else
                    {
                        SendVolumeMsg(delta_time, event);
                    }
                    break;

                case MIDI_CONTROLLER_VOLUME_LSB:
                    SendNOPMsg(delta_time);
                    break;

                case MIDI_CONTROLLER_BANK_SELECT_LSB:
                    SendChannelMsg(delta_time, event,
                                   fallback.type != FALLBACK_BANK_LSB);
                    break;

                case MIDI_CONTROLLER_NRPN_LSB:
                case MIDI_CONTROLLER_NRPN_MSB:
                    if (winmm_complevel == COMP_FULL)
                    {
                        SendChannelMsg(delta_time, event, true);
                    }
                    else
                    {
                        // MS GS Wavetable Synth nulls RPN for any NRPN.
                        SendNullRPN(delta_time, event);
                    }
                    break;

                case MIDI_CONTROLLER_RPN_LSB:
                    switch (event->data.channel.param2)
                    {
                        case MIDI_RPN_PITCH_BEND_SENS_LSB:
                        case MIDI_RPN_FINE_TUNING_LSB:
                        case MIDI_RPN_COARSE_TUNING_LSB:
                        case MIDI_RPN_NULL:
                            SendChannelMsg(delta_time, event, true);
                            break;

                        default:
                            if (winmm_complevel == COMP_FULL)
                            {
                                SendChannelMsg(delta_time, event, true);
                            }
                            else
                            {
                                // MS GS Wavetable Synth ignores other RPNs.
                                SendNullRPN(delta_time, event);
                            }
                            break;
                    }
                    break;

                case MIDI_CONTROLLER_RPN_MSB:
                    switch (event->data.channel.param2)
                    {
                        case MIDI_RPN_MSB:
                        case MIDI_RPN_NULL:
                            SendChannelMsg(delta_time, event, true);
                            break;

                        default:
                            if (winmm_complevel == COMP_FULL)
                            {
                                SendChannelMsg(delta_time, event, true);
                            }
                            else
                            {
                                // MS GS Wavetable Synth ignores other RPNs.
                                SendNullRPN(delta_time, event);
                            }
                            break;
                    }
                    break;

                case EMIDI_CONTROLLER_TRACK_DESIGNATION:
                case EMIDI_CONTROLLER_TRACK_EXCLUSION:
                case EMIDI_CONTROLLER_PROGRAM_CHANGE:
                case EMIDI_CONTROLLER_VOLUME:
                case EMIDI_CONTROLLER_LOOP_BEGIN:
                case EMIDI_CONTROLLER_LOOP_END:
                case EMIDI_CONTROLLER_GLOBAL_LOOP_BEGIN:
                case EMIDI_CONTROLLER_GLOBAL_LOOP_END:
                    SendEMIDI(delta_time, event, track, &fallback);
                    break;

                case MIDI_CONTROLLER_RESET_ALL_CTRLS:
                    // MS GS Wavetable Synth resets volume if param2 isn't zero.
                    SendChannelMsg(delta_time, event, false);
                    break;

                default:
                    if (winmm_complevel == COMP_FULL)
                    {
                        SendChannelMsg(delta_time, event, true);
                    }
                    else
                    {
                        SendNOPMsg(delta_time);
                    }
                    break;
            }
            break;

        case MIDI_EVENT_NOTE_OFF:
        case MIDI_EVENT_NOTE_ON:
        case MIDI_EVENT_PITCH_BEND:
            SendChannelMsg(delta_time, event, true);
            break;

        case MIDI_EVENT_PROGRAM_CHANGE:
            if (track->emidi_program)
            {
                SendNOPMsg(delta_time);
            }
            else
            {
                SendProgramMsg(delta_time, event->data.channel.channel,
                               event->data.channel.param1, &fallback);
            }
            break;

        case MIDI_EVENT_AFTERTOUCH:
            if (winmm_complevel == COMP_FULL)
            {
                SendChannelMsg(delta_time, event, true);
            }
            else
            {
                SendNOPMsg(delta_time);
            }
            break;

        case MIDI_EVENT_CHAN_AFTERTOUCH:
            if (winmm_complevel == COMP_FULL)
            {
                SendChannelMsg(delta_time, event, false);
            }
            else
            {
                SendNOPMsg(delta_time);
            }
            break;

        default:
            SendNOPMsg(delta_time);
            break;
    }

    return true;
}

static boolean (*AddToBuffer)(unsigned int delta_time,
                              const midi_event_t *event,
                              win_midi_track_t *track) = AddToBuffer_Standard;

static void RestartLoop(void)
{
    unsigned int i;

    for (i = 0; i < song.num_tracks; ++i)
    {
        MIDI_RestartAtLoopPoint(song.tracks[i].iter);
        song.tracks[i].end_of_track = song.tracks[i].saved_end_of_track;
        song.tracks[i].elapsed_time = song.tracks[i].saved_elapsed_time;
    }
    song.elapsed_time = song.saved_elapsed_time;
}

static void RestartTracks(void)
{
    unsigned int i;

    for (i = 0; i < song.num_tracks; ++i)
    {
        MIDI_RestartIterator(song.tracks[i].iter);
        song.tracks[i].elapsed_time = 0;
        song.tracks[i].end_of_track = false;
        song.tracks[i].emidi_device_flags = 0;
        song.tracks[i].emidi_designated = false;
        song.tracks[i].emidi_program = false;
        song.tracks[i].emidi_volume = false;
        song.tracks[i].emidi_loop_count = 0;
    }
    song.elapsed_time = 0;
}

static boolean IsRPGLoop(void)
{
    unsigned int i;
    unsigned int num_rpg_events = 0;
    unsigned int num_emidi_events = 0;
    midi_event_t *event = NULL;

    for (i = 0; i < song.num_tracks; ++i)
    {
        while (MIDI_GetNextEvent(song.tracks[i].iter, &event))
        {
            if (event->event_type == MIDI_EVENT_CONTROLLER)
            {
                switch (event->data.channel.param1)
                {
                    case EMIDI_CONTROLLER_TRACK_EXCLUSION:
                        num_rpg_events++;
                        break;

                    case EMIDI_CONTROLLER_TRACK_DESIGNATION:
                    case EMIDI_CONTROLLER_PROGRAM_CHANGE:
                    case EMIDI_CONTROLLER_VOLUME:
                    case EMIDI_CONTROLLER_LOOP_BEGIN:
                    case EMIDI_CONTROLLER_LOOP_END:
                    case EMIDI_CONTROLLER_GLOBAL_LOOP_BEGIN:
                    case EMIDI_CONTROLLER_GLOBAL_LOOP_END:
                        num_emidi_events++;
                        break;
                }
            }
        }

        MIDI_RestartIterator(song.tracks[i].iter);
    }

    return (num_rpg_events == 1 && num_emidi_events == 0);
}

static void FillBuffer(void)
{
    unsigned int i;
    int num_events;

    buffer.position = 0;

    if (initial_playback)
    {
        ResetDevice();
        StreamOut();
        song.rpg_loop = IsRPGLoop();
        initial_playback = false;
        return;
    }

    if (update_volume)
    {
        update_volume = false;
        UpdateVolume();
        StreamOut();
        return;
    }

    switch (win_midi_state)
    {
        case STATE_PLAYING:
            break;

        case STATE_PAUSING:
            // Send notes/sound off to prevent hanging notes.
            SendNotesSoundOff();
            StreamOut();
            win_midi_state = STATE_PAUSED;
            return;

        case STATE_PAUSED:
            // Send a NOP every 100 ms while paused.
            SendDelayMsg(100);
            StreamOut();
            return;

        case STATE_STOPPED:
            return;
    }

    for (num_events = 0; num_events < STREAM_MAX_EVENTS; )
    {
        midi_event_t *event = NULL;
        win_midi_track_t *track = NULL;
        unsigned int min_time = UINT_MAX;
        unsigned int delta_time;

        // Find next event across all tracks.
        for (i = 0; i < song.num_tracks; ++i)
        {
            if (!song.tracks[i].end_of_track)
            {
                unsigned int time = song.tracks[i].elapsed_time +
                                    MIDI_GetDeltaTime(song.tracks[i].iter);
                if (time < min_time)
                {
                    min_time = time;
                    track = &song.tracks[i];
                }
            }
        }

        // No more events. Restart or stop song.
        if (track == NULL)
        {
            if (song.elapsed_time)
            {
                if (song.ff_restart || song.rpg_loop)
                {
                    song.ff_restart = false;
                    RestartLoop();
                    continue;
                }
                else if (song.looping)
                {
                    for (i = 0; i < MIDI_CHANNELS_PER_TRACK; ++i)
                    {
                        SendShortMsg(0, MIDI_EVENT_CONTROLLER, i, MIDI_CONTROLLER_RESET_ALL_CTRLS, 0);
                    }
                    RestartTracks();
                    continue;
                }
            }
            break;
        }

        track->elapsed_time = min_time;
        delta_time = min_time - song.elapsed_time;
        song.elapsed_time = min_time;

        if (!MIDI_GetNextEvent(track->iter, &event))
        {
            track->end_of_track = true;
            continue;
        }

        // Restart FF loop after sending all events that share same timediv.
        if (song.ff_restart && MIDI_GetDeltaTime(track->iter) > 0)
        {
            song.ff_restart = false;
            RestartLoop();
            continue;
        }

        if (!AddToBuffer(delta_time, event, track))
        {
            StreamOut();
            return;
        }

        num_events++;
    }

    if (num_events)
    {
        StreamOut();
    }
}

// The Windows API documentation states: "Applications should not call any
// multimedia functions from inside the callback function, as doing so can
// cause a deadlock." We use thread to avoid possible deadlocks.

static DWORD WINAPI PlayerProc(void)
{
    HANDLE events[2] = { hBufferReturnEvent, hExitEvent };

    while (1)
    {
        switch (WaitForMultipleObjects(2, events, FALSE, INFINITE))
        {
            case WAIT_OBJECT_0:
                FillBuffer();
                break;

            case WAIT_OBJECT_0 + 1:
                return 0;
        }
    }
    return 0;
}

static boolean I_WIN_InitMusic(void)
{
    int all_devices;
    int i;
    MIDIOUTCAPS mcaps;
    MMRESULT mmr;

    // find the midi device that matches the saved one
    if (winmm_midi_device != NULL)
    {
        all_devices = midiOutGetNumDevs() + 1; // include MIDI_MAPPER
        for (i = 0; i < all_devices; ++i)
        {
            // start from device id -1 (MIDI_MAPPER)
            mmr = midiOutGetDevCaps(i - 1, &mcaps, sizeof(mcaps));
            if (mmr == MMSYSERR_NOERROR)
            {
                if (strstr(winmm_midi_device, mcaps.szPname))
                {
                    MidiDevice = i - 1;
                    break;
                }
            }

            if (i == all_devices - 1)
            {
                // give up and use MIDI_MAPPER
                free(winmm_midi_device);
                winmm_midi_device = NULL;
            }
        }
    }

    if (winmm_midi_device == NULL)
    {
        MidiDevice = MIDI_MAPPER;
        mmr = midiOutGetDevCaps(MIDI_MAPPER, &mcaps, sizeof(mcaps));
        if (mmr == MMSYSERR_NOERROR)
        {
            winmm_midi_device = M_StringDuplicate(mcaps.szPname);
        }
    }

    mmr = midiStreamOpen(&hMidiStream, &MidiDevice, (DWORD)1,
                         (DWORD_PTR)MidiStreamProc, (DWORD_PTR)NULL,
                         CALLBACK_FUNCTION);
    if (mmr != MMSYSERR_NOERROR)
    {
        MidiError("midiStreamOpen", mmr);
        hMidiStream = NULL;
        return false;
    }

    AllocateBuffer(BUFFER_INITIAL_SIZE);

    hBufferReturnEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    hExitEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

    AddToBuffer = (winmm_complevel == COMP_VANILLA) ? AddToBuffer_Vanilla
                                                    : AddToBuffer_Standard;
    MIDI_InitFallback();

    win_midi_state = STATE_STOPPED;

    return true;
}

static void I_WIN_SetMusicVolume(int volume)
{
    static int last_volume = -1;

    if (last_volume == volume)
    {
        // Ignore holding key down in volume menu.
        return;
    }

    last_volume = volume;

    volume_factor = sqrtf((float)volume / 120);

    update_volume = song.registered;
}

static void I_WIN_StopSong(void)
{
    MMRESULT mmr;

    if (!hPlayerThread)
    {
        return;
    }

    SetEvent(hExitEvent);
    WaitForSingleObject(hPlayerThread, PLAYER_THREAD_WAIT_TIME);
    CloseHandle(hPlayerThread);
    hPlayerThread = NULL;
    win_midi_state = STATE_STOPPED;

    if (!hMidiStream)
    {
        return;
    }

    mmr = midiStreamStop(hMidiStream);
    if (mmr != MMSYSERR_NOERROR)
    {
        MidiError("midiStreamStop", mmr);
    }
}

static void I_WIN_PlaySong(void *handle, boolean looping)
{
    MMRESULT mmr;

    if (!hMidiStream)
    {
        return;
    }

    song.looping = looping;

    hPlayerThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)PlayerProc,
                                 0, 0, 0);
    SetThreadPriority(hPlayerThread, THREAD_PRIORITY_TIME_CRITICAL);

    initial_playback = true;
    win_midi_state = STATE_PLAYING;

    SetEvent(hBufferReturnEvent);

    mmr = midiStreamRestart(hMidiStream);
    if (mmr != MMSYSERR_NOERROR)
    {
        MidiError("midiStreamRestart", mmr);
    }
}

static void I_WIN_PauseSong(void)
{
    if (!hMidiStream)
    {
        return;
    }

    win_midi_state = STATE_PAUSING;
}

static void I_WIN_ResumeSong(void)
{
    if (!hMidiStream)
    {
        return;
    }

    win_midi_state = STATE_PLAYING;
}

static boolean ConvertMus(byte *musdata, int len, const char *filename)
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

static void *I_WIN_RegisterSong(void *data, int len)
{
    unsigned int i;
    char *filename;
    midi_file_t *file;

    MIDIPROPTIMEDIV prop_timediv;
    MIDIPROPTEMPO prop_tempo;
    MMRESULT mmr;

    if (!hMidiStream)
    {
        return NULL;
    }

    // MUS files begin with "MUS"
    // Reject anything which doesnt have this signature

    filename = M_TempFile("doom.mid");

    if (IsMid(data, len))
    {
        M_WriteFile(filename, data, len);
    }
    else
    {
        // Assume a MUS file and try to convert

        ConvertMus(data, len, filename);
    }

    file = MIDI_LoadFile(filename);

    M_remove(filename);
    free(filename);

    if (file == NULL)
    {
        fprintf(stderr, "I_WIN_RegisterSong: Failed to load MID.\n");
        return NULL;
    }

    prop_timediv.cbStruct = sizeof(MIDIPROPTIMEDIV);
    prop_timediv.dwTimeDiv = MIDI_GetFileTimeDivision(file);
    mmr = midiStreamProperty(hMidiStream, (LPBYTE)&prop_timediv,
                             MIDIPROP_SET | MIDIPROP_TIMEDIV);
    if (mmr != MMSYSERR_NOERROR)
    {
        MidiError("midiStreamProperty", mmr);
        return NULL;
    }
    timediv = prop_timediv.dwTimeDiv;

    // Set initial tempo.
    prop_tempo.cbStruct = sizeof(MIDIPROPTIMEDIV);
    prop_tempo.dwTempo = 500000; // 120 BPM
    mmr = midiStreamProperty(hMidiStream, (LPBYTE)&prop_tempo,
                             MIDIPROP_SET | MIDIPROP_TEMPO);
    if (mmr != MMSYSERR_NOERROR)
    {
        MidiError("midiStreamProperty", mmr);
        return NULL;
    }
    tempo = prop_tempo.dwTempo;

    song.num_tracks = MIDI_NumTracks(file);
    song.tracks = calloc(song.num_tracks, sizeof(win_midi_track_t));
    for (i = 0; i < song.num_tracks; ++i)
    {
        song.tracks[i].iter = MIDI_IterateTrack(file, i);
    }
    song.registered = true;

    ResetEvent(hBufferReturnEvent);
    ResetEvent(hExitEvent);

    return file;
}

static void I_WIN_UnRegisterSong(void *handle)
{
    if (song.tracks)
    {
        unsigned int i;
        for (i = 0; i < song.num_tracks; ++i)
        {
            MIDI_FreeIterator(song.tracks[i].iter);
            song.tracks[i].iter = NULL;
        }
        free(song.tracks);
        song.tracks = NULL;
    }
    if (handle)
    {
        MIDI_FreeFile(handle);
    }
    song.elapsed_time = 0;
    song.saved_elapsed_time = 0;
    song.num_tracks = 0;
    song.registered = false;
    song.looping = false;
    song.ff_loop = false;
    song.ff_restart = false;
    song.rpg_loop = false;
}

static void I_WIN_ShutdownMusic(void)
{
    MMRESULT mmr;

    if (!hMidiStream)
    {
        return;
    }

    I_WIN_StopSong();
    I_WIN_UnRegisterSong(NULL);

    // Reset device at shutdown.
    buffer.position = 0;
    ResetDevice();
    StreamOut();
    mmr = midiStreamRestart(hMidiStream);
    if (mmr != MMSYSERR_NOERROR)
    {
        MidiError("midiStreamRestart", mmr);
    }
    WaitForSingleObject(hBufferReturnEvent, PLAYER_THREAD_WAIT_TIME);
    mmr = midiStreamStop(hMidiStream);
    if (mmr != MMSYSERR_NOERROR)
    {
        MidiError("midiStreamStop", mmr);
    }

    // Don't free the buffer to avoid calling midiOutUnprepareHeader() which
    // contains a memory error (detected by ASan).

    mmr = midiStreamClose(hMidiStream);
    if (mmr != MMSYSERR_NOERROR)
    {
        MidiError("midiStreamClose", mmr);
    }
    hMidiStream = NULL;

    CloseHandle(hBufferReturnEvent);
    CloseHandle(hExitEvent);
}

static boolean I_WIN_MusicIsPlaying(void)
{
    return (song.num_tracks > 0);
}

static const snddevice_t music_win_devices[] =
{
    SNDDEVICE_PAS,
    SNDDEVICE_WAVEBLASTER,
    SNDDEVICE_SOUNDCANVAS,
    SNDDEVICE_GENMIDI,
    SNDDEVICE_AWE32,
};

const music_module_t music_win_module =
{
    music_win_devices,
    arrlen(music_win_devices),
    I_WIN_InitMusic,
    I_WIN_ShutdownMusic,
    I_WIN_SetMusicVolume,
    I_WIN_PauseSong,
    I_WIN_ResumeSong,
    I_WIN_RegisterSong,
    I_WIN_UnRegisterSong,
    I_WIN_PlaySong,
    I_WIN_StopSong,
    I_WIN_MusicIsPlaying,
    NULL,  // Poll
};

#endif
