//
// Copyright(C) 2012 James Haley
// Copyright(C) 2017 Alex Mayfield
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
//
// Win32/SDL_mixer MIDI RPC Server
//
// Uses RPC to communicate with Doom. This allows this separate process to
// have its own independent volume control even under Windows Vista and up's 
// broken, stupid, completely useless mixer model that can't assign separate
// volumes to different devices for the same process.
//
// Seriously, how did they screw up something so fundamental?
//

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdlib.h>

#include "SDL.h"
#include "SDL_mixer.h"

#include "buffer.h"

#include "config.h"
#include "doomtype.h"
#include "net_defs.h"

static HANDLE    midi_process_in;  // Standard In.
static HANDLE    midi_process_out; // Standard Out.
static buffer_t *midi_buffer;      // Data from client.

// Currently playing music track.
static Mix_Music *music  = NULL;

//=============================================================================
//
// Private functions
//

//
// Unregisters the currently playing song.  This is never called from the
// protocol, we simply do this before playing a new song.
//
static void UnregisterSong()
{
    if (music == NULL)
    {
        return;
    }

    Mix_FreeMusic(music);
}

//
// Bookkeeping stuff we need to do when we're shutting off the subprocess.
//
static void ShutdownSDL(void)
{
    UnregisterSong();
    Mix_CloseAudio();
    SDL_Quit();
}

//=============================================================================
//
// SDL_mixer Interface
//

static boolean RegisterSong(const char *filename)
{
    fprintf(stderr, "%s %s\n", __FUNCTION__, filename);

    UnregisterSong();
    music = Mix_LoadMUS(filename);
    fprintf(stderr, "<-- %p\n", music);

    if (music == NULL)
    {
        fprintf(stderr, "Error loading midi: %s\n", Mix_GetError());

        return false;
    }

    return true;
}

static void SetVolume(int vol)
{
    fprintf(stderr, "%s %d\n", __FUNCTION__, vol);

    Mix_VolumeMusic(vol);
}

static void PlaySong(int loops)
{
    fprintf(stderr, "%s %d\n", __FUNCTION__, loops);
    fprintf(stderr, "%s %d\n", "Playing at volume", Mix_VolumeMusic(-1));

    Mix_PlayMusic(music, loops);

    // [AM] BUG: In my testing, setting the volume of a MIDI track while there
    //      is no song playing appears to be a no-op.  This can happen when
    //      you're mixing midiproc with vanilla SDL_Mixer, such as when you
    //      are alternating between a digital music pack (in the parent
    //      process) and MIDI (in this process).
    //
    //      To work around this bug, we set the volume to itself after the MIDI
    //      has started playing.
    Mix_VolumeMusic(Mix_VolumeMusic(-1));
}

static void StopSong()
{
    fprintf(stderr, "%s\n", __FUNCTION__);

    Mix_HaltMusic();
}

//=============================================================================
//
// Pipe Server Interface
//

static boolean MidiPipe_RegisterSong(buffer_reader_t *reader)
{
    char *filename = Reader_ReadString(reader);
    if (filename == NULL)
    {
        return false;
    }

    RegisterSong(filename);

    unsigned int i = NET_MIDIPIPE_PACKET_TYPE_REGISTER_SONG_ACK;
    CHAR buffer[2];
    buffer[0] = (i >> 8) & 0xff;
    buffer[1] = i & 0xff;

    BOOL ok = WriteFile(midi_process_out, buffer, sizeof(buffer),
        NULL, NULL);

    return true;
}

boolean MidiPipe_SetVolume(buffer_reader_t *reader)
{
    int vol;
    boolean ok = Reader_ReadInt32(reader, &vol);
    if (!ok)
    {
        return false;
    }

    SetVolume(vol);

    return true;
}

boolean MidiPipe_PlaySong(buffer_reader_t *reader)
{
    int loops;
    boolean ok = Reader_ReadInt32(reader, &loops);
    if (!ok)
    {
        return false;
    }

    PlaySong(loops);

    return true;
}

boolean MidiPipe_StopSong()
{
    StopSong();

    return true;
}

boolean MidiPipe_Shutdown()
{
    exit(EXIT_SUCCESS);
}

//=============================================================================
//
// Server Implementation
//

//
// Parses a command and directs to the proper read function.
//
boolean ParseCommand(buffer_reader_t *reader, uint16_t command)
{
    switch (command)
    {
    case NET_MIDIPIPE_PACKET_TYPE_REGISTER_SONG:
        return MidiPipe_RegisterSong(reader);
    case NET_MIDIPIPE_PACKET_TYPE_SET_VOLUME:
        return MidiPipe_SetVolume(reader);
    case NET_MIDIPIPE_PACKET_TYPE_PLAY_SONG:
        return MidiPipe_PlaySong(reader);
    case NET_MIDIPIPE_PACKET_TYPE_STOP_SONG:
        return MidiPipe_StopSong();
    case NET_MIDIPIPE_PACKET_TYPE_SHUTDOWN:
        return MidiPipe_Shutdown(reader);
    default:
        return false;
    }
}

//
// Server packet parser
//
boolean ParseMessage(buffer_t *buf)
{
    uint16_t command;
    buffer_reader_t *reader = NewReader(buf);

    // Attempt to read a command out of the buffer.
    if (!Reader_ReadInt16(reader, &command))
    {
        goto fail;
    }

    // Attempt to parse a complete message.
    if (!ParseCommand(reader, command))
    {
        goto fail;
    }

    // We parsed a complete message!  We can now safely shift
    // the prior message off the front of the buffer.
    int bytes_read = Reader_BytesRead(reader);
    DeleteReader(reader);
    Buffer_Shift(buf, bytes_read);

    return true;

fail:
    // We did not read a complete packet.  Delete our reader and try again
    // with more data.
    DeleteReader(reader);
    return false;
}

boolean ListenForever()
{
    BOOL wok = FALSE;
    CHAR pipe_buffer[8192];
    DWORD pipe_buffer_read = 0;

    boolean ok = false;
    buffer_t *buffer = NewBuffer();

    for (;;)
    {
        // Wait until we see some data on the pipe.
        wok = PeekNamedPipe(midi_process_in, NULL, 0, NULL,
            &pipe_buffer_read, NULL);
        if (!wok)
        {
            return false;
        }
        else if (pipe_buffer_read == 0)
        {
            SDL_Delay(1);
            continue;
        }

        // Read data off the pipe and add it to the buffer.
        wok = ReadFile(midi_process_in, pipe_buffer, sizeof(pipe_buffer),
            &pipe_buffer_read, NULL);
        if (!wok)
        {
            return false;
        }

        ok = Buffer_Push(buffer, pipe_buffer, pipe_buffer_read);
        if (!ok)
        {
            return false;
        }

        do
        {
            // Read messages off the buffer until we can't anymore.
            ok = ParseMessage(buffer);
        } while (ok);
    }

    return false;
}

//=============================================================================
//
// Main Program
//

//
// InitSDL
//
// Start up SDL and SDL_mixer.
//
boolean InitSDL()
{
    if (SDL_Init(SDL_INIT_AUDIO) == -1)
    {
        return false;
    }

    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0)
    {
        return false;
    }

    atexit(ShutdownSDL);

    return true;
}

//
// InitPipes
//
// Ensure that we can communicate.
//
boolean InitPipes()
{
    midi_process_in = GetStdHandle(STD_INPUT_HANDLE);
    if (midi_process_in == INVALID_HANDLE_VALUE)
    {
        return false;
    }

    midi_process_out = GetStdHandle(STD_OUTPUT_HANDLE);
    if (midi_process_out == INVALID_HANDLE_VALUE)
    {
        return false;
    }

    return true;
}

//
// main
//
// Application entry point.
//
int main(int argc, char *argv[])
{
    // Make sure we're not launching this process by itself.
    if (argc < 2)
    {
        MessageBox(NULL, TEXT("This program is tasked with playing Native ")
            TEXT("MIDI music, and is intended to be launched by ")
            TEXT(PACKAGE_NAME) TEXT("."),
            TEXT(PACKAGE_STRING), MB_OK | MB_ICONASTERISK);

        return EXIT_FAILURE;
    }

    // Make sure our Choccolate Doom and midiproc version are lined up.
    if (strcmp(PACKAGE_STRING, argv[1]) != 0)
    {
        MessageBox(NULL, TEXT("It appears that the version of ")
            TEXT(PACKAGE_NAME) TEXT(" and ") TEXT(PROGRAM_PREFIX)
            TEXT("midiproc are out of sync.  Please reinstall ")
            TEXT(PACKAGE_NAME) TEXT("."),
            TEXT(PACKAGE_STRING), MB_OK | MB_ICONASTERISK);

        return EXIT_FAILURE;
    }

    if (!InitPipes())
    {
        return EXIT_FAILURE;
    }

    if (!InitSDL())
    {
        return EXIT_FAILURE;
    }

    if (!ListenForever())
    {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

