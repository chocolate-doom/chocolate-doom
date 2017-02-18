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

// Currently playing music track
static Mix_Music *music = NULL;
static char *filename = NULL;

static void UnregisterSong();

//=============================================================================
//
// SDL_mixer Interface
//

//
// RegisterSong
//
static void RegisterSong(char* filename)
{
    if (music)
    {
        UnregisterSong();
    }

    music = Mix_LoadMUS(filename);
}

//
// StartSong
//
static void StartSong(boolean loop)
{
    if (music)
    {
        Mix_PlayMusic(music, loop ? -1 : 0);
    }
}

//
// SetVolume
//
static void SetVolume(int volume)
{
    Mix_VolumeMusic((volume * 128) / 15);
}

static int paused_midi_volume;

//
// PauseSong
//
static void PauseSong()
{
    paused_midi_volume = Mix_VolumeMusic(-1);
    Mix_VolumeMusic(0);
}

//
// ResumeSong
//
static void ResumeSong()
{
    Mix_VolumeMusic(paused_midi_volume);
}

//
// StopSong
//
static void StopSong()
{
    if (music)
    {
        Mix_HaltMusic();
    }
}

//
// UnregisterSong
//
static void UnregisterSong()
{
    if (!music)
    {
        return;
    }

    StopSong();
    Mix_FreeMusic(music);
    free(filename);

    filename = NULL;
    music = NULL;
}

//
// ShutdownSDL
//
static void ShutdownSDL()
{
    UnregisterSong();
    Mix_CloseAudio();
    SDL_Quit();
}

//=============================================================================
//
// RPC Server Interface
//

//
// MidiPipe_PrepareNewSong
//
// Prepare the engine to receive new song data from the RPC client.
//
boolean MidiPipe_PrepareNewSong()
{
    // Stop anything currently playing and free it.
    UnregisterSong();

    fprintf(stderr, "%s\n", __FUNCTION__);
    return true;
}

//
// MidiPipe_AddChunk
//
// Set the filename of the song.
//
boolean MidiPipe_SetFilename(buffer_reader_t *reader)
{
    free(filename);
    filename = NULL;

    char* file = Reader_ReadString(reader);
    if (file == NULL)
    {
        return false;
    }

    int size = Reader_BytesRead(reader) - 2;
    if (size <= 0)
    {
        return false;
    }

    filename = malloc(size);
    if (filename == NULL)
    {
        return false;
    }

    memcpy(filename, file, size);

    fprintf(stderr, "%s\n", __FUNCTION__);
    return true;
}

//
// MidiPipe_PlaySong
//
// Start playing the song.
//
boolean MidiPipe_PlaySong(buffer_reader_t *reader)
{
    uint8_t looping;

    if (!Reader_ReadInt8(reader, &looping))
    {
        return false;
    }

    RegisterSong(filename);
    StartSong((boolean)looping);

    fprintf(stderr, "%s\n", __FUNCTION__);
    return true;
}

//
// MidiPipe_StopSong
//
// Stop the song.
//
boolean MidiPipe_StopSong()
{
    StopSong();

    fprintf(stderr, "%s\n", __FUNCTION__);
    return true;
}

//
// MidiPipe_ChangeVolume
//
// Set playback volume level.
//
boolean MidiPipe_ChangeVolume(buffer_reader_t *reader)
{
    int volume;

    if (!Reader_ReadInt32(reader, &volume))
    {
        return false;
    }

    SetVolume(volume);

    fprintf(stderr, "%s\n", __FUNCTION__);
    return true;
}

//
// MidiPipe_PauseSong
//
// Pause the song.
//
boolean MidiPipe_PauseSong()
{
    PauseSong();

    fprintf(stderr, "%s\n", __FUNCTION__);
    return true;
}

//
// MidiPipe_ResumeSong
//
// Resume after pausing.
//
boolean MidiPipe_ResumeSong()
{
    ResumeSong();

    fprintf(stderr, "%s\n", __FUNCTION__);
    return true;
}

//
// MidiPipe_StopServer
//
// Stops the RPC server so the program can shutdown.
//
boolean MidiPipe_StopServer()
{
    // Local shutdown tasks
    ShutdownSDL();
    free(filename);
    filename = NULL;

    fprintf(stderr, "%s\n", __FUNCTION__);
    return true;
}

//=============================================================================
//
// Server Implementation
//

boolean ParseCommand(buffer_reader_t *reader, uint16_t command)
{
    switch (command)
    {
    case NET_MIDIPIPE_PACKET_TYPE_PREPARE_NEW_SONG:
        return MidiPipe_PrepareNewSong();
    case NET_MIDIPIPE_PACKET_TYPE_SET_FILENAME:
        return MidiPipe_SetFilename(reader);
    case NET_MIDIPIPE_PACKET_TYPE_PLAY_SONG:
        return MidiPipe_PlaySong(reader);
    case NET_MIDIPIPE_PACKET_TYPE_STOP_SONG:
        return MidiPipe_StopSong();
    case NET_MIDIPIPE_PACKET_TYPE_CHANGE_VOLUME:
        return MidiPipe_ChangeVolume(reader);
    case NET_MIDIPIPE_PACKET_TYPE_PAUSE_SONG:
        return MidiPipe_PauseSong();
    case NET_MIDIPIPE_PACKET_TYPE_RESUME_SONG:
        return MidiPipe_ResumeSong();
    case NET_MIDIPIPE_PACKET_TYPE_STOP_SERVER:
        return MidiPipe_StopServer();
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

    fprintf(stderr, "%s\n", "In theory we should be reading...");
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
        fprintf(stderr, "%s\n", "ReadFile");
        wok = ReadFile(midi_process_in, pipe_buffer, sizeof(pipe_buffer),
            &pipe_buffer_read, NULL);
        if (!wok)
        {
            return false;
        }

        fprintf(stderr, "%s\n", "Buffer_Push");
        ok = Buffer_Push(buffer, pipe_buffer, pipe_buffer_read);
        if (!ok)
        {
            return false;
        }

        fprintf(stderr, "%s\n", "ParseMessage");
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

