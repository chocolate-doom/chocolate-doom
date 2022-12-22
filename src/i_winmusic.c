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

char *winmm_midi_device = NULL;
int winmm_reverb_level = -1;
int winmm_chorus_level = -1;

enum
{
    RESET_TYPE_DEFAULT = -1,
    RESET_TYPE_NONE,
    RESET_TYPE_GS,
    RESET_TYPE_GM,
    RESET_TYPE_GM2,
    RESET_TYPE_XG,
};

int winmm_reset_type = RESET_TYPE_DEFAULT;
int winmm_reset_delay = 0;

static const byte gs_reset[] = {
    0xF0, 0x41, 0x10, 0x42, 0x12, 0x40, 0x00, 0x7F, 0x00, 0x41, 0xF7
};

static const byte gm_system_on[] = {
    0xF0, 0x7E, 0x7F, 0x09, 0x01, 0xF7
};

static const byte gm2_system_on[] = {
    0xF0, 0x7E, 0x7F, 0x09, 0x03, 0xF7
};

static const byte xg_system_on[] = {
    0xF0, 0x43, 0x10, 0x4C, 0x00, 0x00, 0x7E, 0x00, 0xF7
};

static const byte ff_loopStart[] = {'l', 'o', 'o', 'p', 'S', 't', 'a', 'r', 't'};
static const byte ff_loopEnd[] = {'l', 'o', 'o', 'p', 'E', 'n', 'd'};

static boolean use_fallback;

#define DEFAULT_VOLUME 100
static int channel_volume[MIDI_CHANNELS_PER_TRACK];
static float volume_factor = 0.0f;
static boolean update_volume = false;

static DWORD timediv;
static DWORD tempo;

static UINT MidiDevice;
static HMIDISTRM hMidiStream;
static MIDIHDR MidiStreamHdr;
static HANDLE hBufferReturnEvent;
static HANDLE hExitEvent;
static HANDLE hPlayerThread;

// MS GS Wavetable Synth Device ID.
static int ms_gs_synth = MIDI_MAPPER;

// EMIDI device for track designation.
static int emidi_device;

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

#define BUFFER_INITIAL_SIZE 1024

typedef struct
{
    byte *data;
    unsigned int size;
    unsigned int position;
} buffer_t;

static buffer_t buffer;

// Maximum of 4 events in the buffer for faster volume updates.

#define STREAM_MAX_EVENTS   4

#define MAKE_EVT(a, b, c, d) ((DWORD)((a) | ((b) << 8) | ((c) << 16) | ((d) << 24)))

#define PADDED_SIZE(x) (((x) + sizeof(DWORD) - 1) & ~(sizeof(DWORD) - 1))

static boolean initial_playback = false;

// Message box for midiStream errors.

static void MidiError(const char *prefix, DWORD dwError)
{
    char szErrorBuf[MAXERRORLENGTH];
    MMRESULT mmr;

    mmr = midiOutGetErrorText(dwError, (LPSTR) szErrorBuf, MAXERRORLENGTH);
    if (mmr == MMSYSERR_NOERROR)
    {
        char *msg = M_StringJoin(prefix, ": ", szErrorBuf, NULL);
        MessageBox(NULL, msg, "midiStream Error", MB_ICONEXCLAMATION);
        free(msg);
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

static void SendShortMsg(int time, int status, int channel, int param1, int param2)
{
    native_event_t native_event;
    native_event.dwDeltaTime = time;
    native_event.dwStreamID = 0;
    native_event.dwEvent = MAKE_EVT(status | channel, param1, param2, MEVT_SHORTMSG);
    WriteBuffer((byte *)&native_event, sizeof(native_event_t));
}

static void SendLongMsg(int time, const byte *ptr, int length)
{
    native_event_t native_event;
    native_event.dwDeltaTime = time;
    native_event.dwStreamID = 0;
    native_event.dwEvent = MAKE_EVT(length, 0, 0, MEVT_LONGMSG);
    WriteBuffer((byte *)&native_event, sizeof(native_event_t));
    WriteBuffer(ptr, length);
    WriteBufferPad();
}

static void SendNOPMsg(int time)
{
    native_event_t native_event;
    native_event.dwDeltaTime = time;
    native_event.dwStreamID = 0;
    native_event.dwEvent = MAKE_EVT(0, 0, 0, MEVT_NOP);
    WriteBuffer((byte *)&native_event, sizeof(native_event_t));
}

static void SendDelayMsg(int time_ms)
{
    // Convert ms to ticks (see "Standard MIDI Files 1.0" page 14).
    int time_ticks = (float)time_ms * 1000 * timediv / tempo + 0.5f;
    SendNOPMsg(time_ticks);
}

static void UpdateTempo(int time, midi_event_t *event)
{
    native_event_t native_event;

    tempo = MAKE_EVT(event->data.meta.data[2], event->data.meta.data[1],
                     event->data.meta.data[0], 0);

    native_event.dwDeltaTime = time;
    native_event.dwStreamID = 0;
    native_event.dwEvent = MAKE_EVT(tempo, 0, 0, MEVT_TEMPO);
    WriteBuffer((byte *)&native_event, sizeof(native_event_t));
}

static void SendVolumeMsg(int time, int channel, int volume)
{
    int scaled_volume = volume * volume_factor + 0.5f;
    SendShortMsg(time, MIDI_EVENT_CONTROLLER, channel,
                 MIDI_CONTROLLER_VOLUME_MSB, scaled_volume);
    channel_volume[channel] = volume;
}

static void UpdateVolume(void)
{
    int i;

    for (i = 0; i < MIDI_CHANNELS_PER_TRACK; ++i)
    {
        SendVolumeMsg(0, i, channel_volume[i]);
    }
}

static void ResetVolume(void)
{
    int i;

    for (i = 0; i < MIDI_CHANNELS_PER_TRACK; ++i)
    {
        SendVolumeMsg(0, i, DEFAULT_VOLUME);
    }
}

static void ResetReverb(int reset_type)
{
    int i;
    int reverb = winmm_reverb_level;

    if (reverb == -1 && reset_type == RESET_TYPE_NONE)
    {
        // No reverb specified and no SysEx reset selected. Use GM default.
        reverb = 40;
    }

    if (reverb > -1)
    {
        for (i = 0; i < MIDI_CHANNELS_PER_TRACK; ++i)
        {
            SendShortMsg(0, MIDI_EVENT_CONTROLLER, i, MIDI_CONTROLLER_REVERB, reverb);
        }
    }
}

static void ResetChorus(int reset_type)
{
    int i;
    int chorus = winmm_chorus_level;

    if (chorus == -1 && reset_type == RESET_TYPE_NONE)
    {
        // No chorus specified and no SysEx reset selected. Use GM default.
        chorus = 0;
    }

    if (chorus > -1)
    {
        for (i = 0; i < MIDI_CHANNELS_PER_TRACK; ++i)
        {
            SendShortMsg(0, MIDI_EVENT_CONTROLLER, i, MIDI_CONTROLLER_CHORUS, chorus);
        }
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
    int i;
    int reset_type;

    for (i = 0; i < MIDI_CHANNELS_PER_TRACK; ++i)
    {
        // Stop sound prior to reset to prevent volume spikes.
        SendShortMsg(0, MIDI_EVENT_CONTROLLER, i, MIDI_CONTROLLER_ALL_NOTES_OFF, 0);
        SendShortMsg(0, MIDI_EVENT_CONTROLLER, i, MIDI_CONTROLLER_ALL_SOUND_OFF, 0);
    }

    if (MidiDevice == ms_gs_synth)
    {
        // MS GS Wavetable Synth lacks instrument fallback in GS mode which can
        // cause wrong or silent notes (MAYhem19.wad D_DM2TTL). It also responds
        // to XG System On when it should ignore it.
        switch (winmm_reset_type)
        {
            case RESET_TYPE_NONE:
                reset_type = RESET_TYPE_NONE;
                break;

            case RESET_TYPE_GS:
                reset_type = RESET_TYPE_GS;
                break;

            default:
                reset_type = RESET_TYPE_GM;
                break;
        }
    }
    else // Unknown device
    {
        // Most devices support GS mode. Exceptions are some older hardware and
        // a few older VSTis. Some devices lack instrument fallback in GS mode.
        switch (winmm_reset_type)
        {
            case RESET_TYPE_NONE:
            case RESET_TYPE_GM:
            case RESET_TYPE_GM2:
            case RESET_TYPE_XG:
                reset_type = winmm_reset_type;
                break;

            default:
                reset_type = RESET_TYPE_GS;
                break;
        }
    }

    // Use instrument fallback in GS mode.
    MIDI_ResetFallback();
    use_fallback = (reset_type == RESET_TYPE_GS);

    // Assign EMIDI device for track designation.
    emidi_device = (reset_type == RESET_TYPE_GS);

    switch (reset_type)
    {
        case RESET_TYPE_NONE:
            ResetControllers();
            break;

        case RESET_TYPE_GS:
            SendLongMsg(0, gs_reset, sizeof(gs_reset));
            break;

        case RESET_TYPE_GM:
            SendLongMsg(0, gm_system_on, sizeof(gm_system_on));
            break;

        case RESET_TYPE_GM2:
            SendLongMsg(0, gm2_system_on, sizeof(gm2_system_on));
            break;

        case RESET_TYPE_XG:
            SendLongMsg(0, xg_system_on, sizeof(xg_system_on));
            break;
    }

    if (reset_type == RESET_TYPE_NONE || MidiDevice == ms_gs_synth)
    {
        // MS GS Wavetable Synth doesn't reset pitch bend sensitivity, even
        // when sending a GM/GS reset, so do it manually.
        ResetPitchBendSensitivity();
    }

    ResetReverb(reset_type);
    ResetChorus(reset_type);

    // Reset volume (initial playback or on shutdown if no SysEx reset).
    if (initial_playback || reset_type == RESET_TYPE_NONE)
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

static boolean IsSysExReset(const byte *msg, int length)
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

static void SendSysExMsg(int time, const byte *data, int length)
{
    native_event_t native_event;
    boolean is_sysex_reset;
    const byte event_type = MIDI_EVENT_SYSEX;

    is_sysex_reset = IsSysExReset(data, length);

    if (is_sysex_reset && MidiDevice == ms_gs_synth)
    {
        // Ignore SysEx reset from MIDI file for MS GS Wavetable Synth.
        SendNOPMsg(time);
        return;
    }

    // Send the SysEx message.
    native_event.dwDeltaTime = time;
    native_event.dwStreamID = 0;
    native_event.dwEvent = MAKE_EVT(length + sizeof(byte), 0, 0, MEVT_LONGMSG);
    WriteBuffer((byte *)&native_event, sizeof(native_event_t));
    WriteBuffer(&event_type, sizeof(byte));
    WriteBuffer(data, length);
    WriteBufferPad();

    if (is_sysex_reset)
    {
        // SysEx reset also resets volume. Take the default channel volumes
        // and scale them by the user's volume slider.
        ResetVolume();

        // Disable instrument fallback and give priority to MIDI file. Fallback
        // assumes GS (SC-55 level) and the MIDI file could be GM, GM2, XG, or
        // GS (SC-88 or higher). Preserve the composer's intent.
        MIDI_ResetFallback();
        use_fallback = false;

        // Use default device for EMIDI.
        emidi_device = EMIDI_DEVICE_GENERAL_MIDI;
    }
}

static void SendProgramMsg(int time, int channel, int program,
                           midi_fallback_t *fallback)
{
    switch ((int)fallback->type)
    {
        case FALLBACK_BANK_MSB:
            SendShortMsg(time, MIDI_EVENT_CONTROLLER, channel,
                         MIDI_CONTROLLER_BANK_SELECT_MSB, fallback->value);
            SendShortMsg(0, MIDI_EVENT_PROGRAM_CHANGE, channel, program, 0);
            break;

        case FALLBACK_DRUMS:
            SendShortMsg(time, MIDI_EVENT_PROGRAM_CHANGE, channel,
                         fallback->value, 0);
            break;

        default:
            SendShortMsg(time, MIDI_EVENT_PROGRAM_CHANGE, channel, program, 0);
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

static void CheckFFLoop(midi_event_t *event)
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

static boolean AddToBuffer(unsigned int delta_time, midi_event_t *event,
                           win_midi_track_t *track)
{
    unsigned int i;
    unsigned int flag;
    int count;
    midi_fallback_t fallback = {FALLBACK_NONE, 0};

    if (use_fallback)
    {
        MIDI_CheckFallback(event, &fallback);
    }

    switch ((int)event->event_type)
    {
        case MIDI_EVENT_SYSEX:
            SendSysExMsg(delta_time, event->data.sysex.data,
                         event->data.sysex.length);
            return false;

        case MIDI_EVENT_META:
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
                    CheckFFLoop(event);
                    SendNOPMsg(delta_time);
                    break;

                default:
                    SendNOPMsg(delta_time);
                    break;
            }
            return true;
    }

    if (track->emidi_designated && (emidi_device & ~track->emidi_device_flags))
    {
        // Send NOP if this device has been excluded from this track.
        SendNOPMsg(delta_time);
        return true;
    }

    switch ((int)event->event_type)
    {
        case MIDI_EVENT_CONTROLLER:
            switch (event->data.channel.param1)
            {
                case MIDI_CONTROLLER_VOLUME_MSB:
                    if (track->emidi_volume)
                    {
                        SendNOPMsg(delta_time);
                    }
                    else
                    {
                        SendVolumeMsg(delta_time, event->data.channel.channel,
                                      event->data.channel.param2);
                    }
                    break;

                case MIDI_CONTROLLER_VOLUME_LSB:
                    SendNOPMsg(delta_time);
                    break;

                case MIDI_CONTROLLER_BANK_SELECT_LSB:
                    if (fallback.type == FALLBACK_BANK_LSB)
                    {
                        SendShortMsg(delta_time, MIDI_EVENT_CONTROLLER,
                                     event->data.channel.channel,
                                     MIDI_CONTROLLER_BANK_SELECT_LSB,
                                     fallback.value);
                    }
                    else
                    {
                        SendShortMsg(delta_time, MIDI_EVENT_CONTROLLER,
                                     event->data.channel.channel,
                                     MIDI_CONTROLLER_BANK_SELECT_LSB,
                                     event->data.channel.param2);
                    }
                    break;

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
                                       event->data.channel.param2, &fallback);
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
                        SendVolumeMsg(delta_time, event->data.channel.channel,
                                      event->data.channel.param2);
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
                    }
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
                    SendShortMsg(delta_time, MIDI_EVENT_CONTROLLER,
                                 event->data.channel.channel,
                                 event->data.channel.param1,
                                 event->data.channel.param2);
                    break;
            }
            break;

        case MIDI_EVENT_NOTE_OFF:
        case MIDI_EVENT_NOTE_ON:
        case MIDI_EVENT_AFTERTOUCH:
        case MIDI_EVENT_PITCH_BEND:
            SendShortMsg(delta_time, event->event_type,
                         event->data.channel.channel,
                         event->data.channel.param1,
                         event->data.channel.param2);
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

        case MIDI_EVENT_CHAN_AFTERTOUCH:
            SendShortMsg(delta_time, MIDI_EVENT_CHAN_AFTERTOUCH,
                         event->data.channel.channel,
                         event->data.channel.param1, 0);
            break;

        default:
            SendNOPMsg(delta_time);
            break;
    }

    return true;
}

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

    // Is this device MS GS Synth?
    {
        const char pname[] = "Microsoft GS Wavetable";
        if (!strncasecmp(pname, mcaps.szPname, sizeof(pname) - 1))
        {
            ms_gs_synth = MidiDevice;
        }
    }

    mmr = midiStreamOpen(&hMidiStream, &MidiDevice, (DWORD)1,
                         (DWORD_PTR)MidiStreamProc, (DWORD_PTR)NULL,
                         CALLBACK_FUNCTION);
    if (mmr != MMSYSERR_NOERROR)
    {
        MidiError("midiStreamOpen", mmr);
        return false;
    }

    AllocateBuffer(BUFFER_INITIAL_SIZE);

    hBufferReturnEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    hExitEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

    MIDI_InitFallback();

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
    WaitForSingleObject(hPlayerThread, INFINITE);
    CloseHandle(hPlayerThread);
    hPlayerThread = NULL;

    mmr = midiStreamStop(hMidiStream);
    if (mmr != MMSYSERR_NOERROR)
    {
        MidiError("midiStreamStop", mmr);
    }
}

static void I_WIN_PlaySong(void *handle, boolean looping)
{
    MMRESULT mmr;

    song.looping = looping;

    hPlayerThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)PlayerProc,
                                 0, 0, 0);
    SetThreadPriority(hPlayerThread, THREAD_PRIORITY_TIME_CRITICAL);

    initial_playback = true;

    SetEvent(hBufferReturnEvent);

    mmr = midiStreamRestart(hMidiStream);
    if (mmr != MMSYSERR_NOERROR)
    {
        MidiError("midiStreamRestart", mmr);
    }
}

static void I_WIN_PauseSong(void)
{
    MMRESULT mmr;

    mmr = midiStreamPause(hMidiStream);
    if (mmr != MMSYSERR_NOERROR)
    {
        MidiError("midiStreamPause", mmr);
    }
}

static void I_WIN_ResumeSong(void)
{
    MMRESULT mmr;

    mmr = midiStreamRestart(hMidiStream);
    if (mmr != MMSYSERR_NOERROR)
    {
        MidiError("midiStreamRestart", mmr);
    }
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
        int i;
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
    WaitForSingleObject(hBufferReturnEvent, INFINITE);
    mmr = midiStreamStop(hMidiStream);
    if (mmr != MMSYSERR_NOERROR)
    {
        MidiError("midiStreamStop", mmr);
    }

    if (buffer.data)
    {
        mmr = midiOutUnprepareHeader((HMIDIOUT)hMidiStream, &MidiStreamHdr,
                                     sizeof(MIDIHDR));
        if (mmr != MMSYSERR_NOERROR)
        {
            MidiError("midiOutUnprepareHeader", mmr);
        }
        free(buffer.data);
        buffer.data = NULL;
        buffer.size = 0;
        buffer.position = 0;
    }

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

static snddevice_t music_win_devices[] =
{
    SNDDEVICE_PAS,
    SNDDEVICE_WAVEBLASTER,
    SNDDEVICE_SOUNDCANVAS,
    SNDDEVICE_GENMIDI,
    SNDDEVICE_AWE32,
};

music_module_t music_win_module =
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
