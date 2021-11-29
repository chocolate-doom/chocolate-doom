//
// Copyright(C) 2021 Roman Fomin
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
#include <stdio.h>
#include <stdlib.h>

#include "doomtype.h"
#include "m_misc.h"
#include "midifile.h"

static HMIDISTRM hMidiStream;
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
    native_event_t *native_events;
    int num_events;
    int position;
    boolean looping;
} win_midi_song_t;

static win_midi_song_t song;

typedef struct
{
    midi_track_iter_t *iter;
    int absolute_time;
} win_midi_track_t;

static float volume_factor = 1.0;

// Save the last volume for each MIDI channel.

static int channel_volume[MIDI_CHANNELS_PER_TRACK];

// Macros for use with the Windows MIDIEVENT dwEvent field.

#define MIDIEVENT_CHANNEL(x)    (x & 0x0000000F)
#define MIDIEVENT_TYPE(x)       (x & 0x000000F0)
#define MIDIEVENT_DATA1(x)     ((x & 0x0000FF00) >> 8)
#define MIDIEVENT_VOLUME(x)    ((x & 0x007F0000) >> 16)

// Maximum of 4 events in the buffer for faster volume updates.

#define STREAM_MAX_EVENTS   4

typedef struct
{
    native_event_t events[STREAM_MAX_EVENTS];
    int num_events;
    MIDIHDR MidiStreamHdr;
} buffer_t;

static buffer_t buffer;

static const int volume_correction[] = {
    0,   4,   7,   11,  13,  14,  16,  18,
    21,  22,  23,  24,  24,  24,  25,  25,
    25,  26,  26,  27,  27,  27,  28,  28,
    29,  29,  29,  30,  30,  31,  31,  32,
    32,  32,  33,  33,  34,  34,  35,  35,
    36,  37,  37,  38,  38,  39,  39,  40,
    40,  41,  42,  42,  43,  43,  44,  45,
    45,  46,  47,  47,  48,  49,  49,  50,
    51,  52,  52,  53,  54,  55,  56,  56,
    57,  58,  59,  60,  61,  62,  62,  63,
    64,  65,  66,  67,  68,  69,  70,  71,
    72,  73,  74,  75,  77,  78,  79,  80,
    81,  82,  84,  85,  86,  87,  89,  90,
    91,  92,  94,  95,  96,  98,  99,  101,
    102, 104, 105, 107, 108, 110, 112, 113,
    115, 117, 118, 120, 122, 123, 125, 127
};

// Message box for midiStream errors.

static void MidiErrorMessageBox(DWORD dwError)
{
    char szErrorBuf[MAXERRORLENGTH];
    MMRESULT mmr;

    mmr = midiOutGetErrorText(dwError, (LPSTR) szErrorBuf, MAXERRORLENGTH);
    if (mmr == MMSYSERR_NOERROR)
    {
        MessageBox(NULL, szErrorBuf, "midiStream Error", MB_ICONEXCLAMATION);
    }
    else
    {
        fprintf(stderr, "Unknown midiStream error.\n");
    }
}

// Fill the buffer with MIDI events, adjusting the volume as needed.

static void FillBuffer(void)
{
    int i;

    for (i = 0; i < STREAM_MAX_EVENTS; ++i)
    {
        native_event_t *event = &buffer.events[i];

        if (song.position >= song.num_events)
        {
            if (song.looping)
            {
                song.position = 0;
            }
            else
            {
                break;
            }
        }

        *event = song.native_events[song.position];

        if (MIDIEVENT_TYPE(event->dwEvent) == MIDI_EVENT_CONTROLLER &&
            MIDIEVENT_DATA1(event->dwEvent) == MIDI_CONTROLLER_MAIN_VOLUME)
        {
            int volume = MIDIEVENT_VOLUME(event->dwEvent);

            channel_volume[MIDIEVENT_CHANNEL(event->dwEvent)] = volume;

            volume *= volume_factor;

            volume = volume_correction[volume];

            event->dwEvent = (event->dwEvent & 0xFF00FFFF) |
                             ((volume & 0x7F) << 16);
        }

        song.position++;
    }

    buffer.num_events = i;
}

// Queue MIDI events.

static void StreamOut(void)
{
    MIDIHDR *hdr = &buffer.MidiStreamHdr;
    MMRESULT mmr;

    int num_events = buffer.num_events;

    if (num_events == 0)
    {
        return;
    }

    hdr->lpData = (LPSTR)buffer.events;
    hdr->dwBytesRecorded = num_events * sizeof(native_event_t);

    mmr = midiStreamOut(hMidiStream, hdr, sizeof(MIDIHDR));
    if (mmr != MMSYSERR_NOERROR)
    {
        MidiErrorMessageBox(mmr);
    }
}

// midiStream callback.

static void CALLBACK MidiStreamProc(HMIDIIN hMidi, UINT uMsg,
                                    DWORD_PTR dwInstance, DWORD_PTR dwParam1,
                                    DWORD_PTR dwParam2)
{
    if (uMsg == MOM_DONE)
    {
        SetEvent(hBufferReturnEvent);
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
                StreamOut();
                break;

            case WAIT_OBJECT_0 + 1:
                return 0;
        }
    }
    return 0;
}

// Convert a multi-track MIDI file to an array of Windows MIDIEVENT structures.

static void MIDItoStream(midi_file_t *file)
{
    int i;

    int num_tracks =  MIDI_NumTracks(file);
    win_midi_track_t *tracks = malloc(num_tracks * sizeof(win_midi_track_t));

    int current_time = 0;

    for (i = 0; i < num_tracks; ++i)
    {
        tracks[i].iter = MIDI_IterateTrack(file, i);
        tracks[i].absolute_time = 0;
    }

    song.native_events = calloc(MIDI_NumEvents(file), sizeof(native_event_t));

    while (1)
    {
        midi_event_t *event;
        DWORD data = 0;
        int min_time = INT_MAX;
        int idx = -1;

        // Look for an event with a minimal delta time.
        for (i = 0; i < num_tracks; ++i)
        {
            int time = 0;

            if (tracks[i].iter == NULL)
            {
                continue;
            }

            time = tracks[i].absolute_time + MIDI_GetDeltaTime(tracks[i].iter);

            if (time < min_time)
            {
                min_time = time;
                idx = i;
            }
        }

        // No more MIDI events left, end the loop.
        if (idx == -1)
        {
            break;
        }

        tracks[idx].absolute_time = min_time;

        if (!MIDI_GetNextEvent(tracks[idx].iter, &event))
        {
            MIDI_FreeIterator(tracks[idx].iter);
            tracks[idx].iter = NULL;
            continue;
        }

        switch ((int)event->event_type)
        {
            case MIDI_EVENT_META:
                if (event->data.meta.type == MIDI_META_SET_TEMPO)
                {
                    data = event->data.meta.data[2] |
                        (event->data.meta.data[1] << 8) |
                        (event->data.meta.data[0] << 16) |
                        (MEVT_TEMPO << 24);
                }
                break;

            case MIDI_EVENT_NOTE_OFF:
            case MIDI_EVENT_NOTE_ON:
            case MIDI_EVENT_AFTERTOUCH:
            case MIDI_EVENT_CONTROLLER:
            case MIDI_EVENT_PITCH_BEND:
                data = event->event_type |
                    event->data.channel.channel |
                    (event->data.channel.param1 << 8) |
                    (event->data.channel.param2 << 16) |
                    (MEVT_SHORTMSG << 24);
                break;

            case MIDI_EVENT_PROGRAM_CHANGE:
            case MIDI_EVENT_CHAN_AFTERTOUCH:
                data = event->event_type |
                    event->data.channel.channel |
                    (event->data.channel.param1 << 8) |
                    (0 << 16) |
                    (MEVT_SHORTMSG << 24);
                break;
        }

        if (data)
        {
            native_event_t *native_event = &song.native_events[song.num_events];

            native_event->dwDeltaTime = min_time - current_time;
            native_event->dwStreamID = 0;
            native_event->dwEvent = data;

            song.num_events++;
            current_time = min_time;
        }
    }

    if (tracks)
    {
        free(tracks);
    }
}

boolean I_WIN_InitMusic(void)
{
    UINT MidiDevice = MIDI_MAPPER;
    MIDIHDR *hdr = &buffer.MidiStreamHdr;
    MMRESULT mmr;

    mmr = midiStreamOpen(&hMidiStream, &MidiDevice, (DWORD)1,
                         (DWORD_PTR)MidiStreamProc, (DWORD_PTR)NULL,
                         CALLBACK_FUNCTION);
    if (mmr != MMSYSERR_NOERROR)
    {
        MidiErrorMessageBox(mmr);
        return false;
    }

    hdr->lpData = (LPSTR)buffer.events;
    hdr->dwBytesRecorded = 0;
    hdr->dwBufferLength = STREAM_MAX_EVENTS * sizeof(native_event_t);
    hdr->dwFlags = 0;
    hdr->dwOffset = 0;

    mmr = midiOutPrepareHeader((HMIDIOUT)hMidiStream, hdr, sizeof(MIDIHDR));
    if (mmr != MMSYSERR_NOERROR)
    {
        MidiErrorMessageBox(mmr);
        return false;
    }

    hBufferReturnEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    hExitEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

    return true;
}

void I_WIN_SetMusicVolume(int volume)
{
    int i;

    volume_factor = (float)volume / 127;

    // Send MIDI controller events to adjust the volume.
    for (i = 0; i < MIDI_CHANNELS_PER_TRACK; ++i)
    {
        DWORD msg = 0;

        int value = channel_volume[i] * volume_factor;

        value = volume_correction[value];

        msg = MIDI_EVENT_CONTROLLER | i | (MIDI_CONTROLLER_MAIN_VOLUME << 8) |
              (value << 16);

        midiOutShortMsg((HMIDIOUT)hMidiStream, msg);
    }
}

void I_WIN_StopSong(void)
{
    MMRESULT mmr;

    if (hPlayerThread)
    {
        SetEvent(hExitEvent);
        WaitForSingleObject(hPlayerThread, INFINITE);

        CloseHandle(hPlayerThread);
        hPlayerThread = NULL;
    }

    mmr = midiStreamStop(hMidiStream);
    if (mmr != MMSYSERR_NOERROR)
    {
        MidiErrorMessageBox(mmr);
    }
    mmr = midiOutReset((HMIDIOUT)hMidiStream);
    if (mmr != MMSYSERR_NOERROR)
    {
        MidiErrorMessageBox(mmr);
    }
}

void I_WIN_PlaySong(boolean looping)
{
    MMRESULT mmr;

    song.looping = looping;

    hPlayerThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)PlayerProc,
                                 0, 0, 0);
    SetThreadPriority(hPlayerThread, THREAD_PRIORITY_TIME_CRITICAL);

    mmr = midiStreamRestart(hMidiStream);
    if (mmr != MMSYSERR_NOERROR)
    {
        MidiErrorMessageBox(mmr);
    }
}

boolean I_WIN_RegisterSong(char *filename)
{
    int i;
    midi_file_t *file;
    MIDIPROPTIMEDIV timediv;
    MIDIPROPTEMPO tempo;
    MMRESULT mmr;

    file = MIDI_LoadFile(filename);

    if (file == NULL)
    {
        fprintf(stderr, "I_WIN_RegisterSong: Failed to load MID.\n");
        return false;
    }

    // Initialize channels volume.
    for (i = 0; i < MIDI_CHANNELS_PER_TRACK; ++i)
    {
        channel_volume[i] = 100;
    }

    timediv.cbStruct = sizeof(MIDIPROPTIMEDIV);
    timediv.dwTimeDiv = MIDI_GetFileTimeDivision(file);
    mmr = midiStreamProperty(hMidiStream, (LPBYTE)&timediv,
                             MIDIPROP_SET | MIDIPROP_TIMEDIV);
    if (mmr != MMSYSERR_NOERROR)
    {
        MidiErrorMessageBox(mmr);
        return false;
    }

    // Set initial tempo.
    tempo.cbStruct = sizeof(MIDIPROPTIMEDIV);
    tempo.dwTempo = 500000; // 120 bmp
    mmr = midiStreamProperty(hMidiStream, (LPBYTE)&tempo,
                             MIDIPROP_SET | MIDIPROP_TEMPO);
    if (mmr != MMSYSERR_NOERROR)
    {
        MidiErrorMessageBox(mmr);
        return false;
    }

    MIDItoStream(file);

    MIDI_FreeFile(file);

    ResetEvent(hBufferReturnEvent);
    ResetEvent(hExitEvent);

    FillBuffer();
    StreamOut();

    return true;
}

void I_WIN_UnRegisterSong(void)
{
    if (song.native_events)
    {
        free(song.native_events);
        song.native_events = NULL;
    }
    song.num_events = 0;
    song.position = 0;
}

void I_WIN_ShutdownMusic(void)
{
    MIDIHDR *hdr = &buffer.MidiStreamHdr;
    MMRESULT mmr;

    I_WIN_StopSong();

    mmr = midiOutUnprepareHeader((HMIDIOUT)hMidiStream, hdr, sizeof(MIDIHDR));
    if (mmr != MMSYSERR_NOERROR)
    {
        MidiErrorMessageBox(mmr);
    }

    mmr = midiStreamClose(hMidiStream);
    if (mmr != MMSYSERR_NOERROR)
    {
        MidiErrorMessageBox(mmr);
    }

    hMidiStream = NULL;

    CloseHandle(hBufferReturnEvent);
    CloseHandle(hExitEvent);
}

#endif
