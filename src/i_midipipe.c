//
// Copyright(C) 2013 James Haley et al.
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
//     Client Interface to RPC Midi Server
//

#if _WIN32

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <SDL_net.h>

#include "i_midipipe.h"

#include "config.h"
#include "m_misc.h"
#include "net_packet.h"

#if defined(_DEBUG)
#define DEBUGOUT(s) puts(s)
#else
#define DEBUGOUT(s)
#endif

//=============================================================================
//
// Data
//

static HANDLE  midi_process_in_reader;  // Input stream for midi process.
static HANDLE  midi_process_in_writer;
static HANDLE  midi_process_out_reader; // Output stream for midi process.
static HANDLE  midi_process_out_writer;

static boolean server_init = false; // if true, server was started
static boolean client_init = false; // if true, client was bound

//=============================================================================
//
// RPC Wrappers
//

//
// CHECK_RPC_STATUS
//
// If either server or client initialization failed, we don't try to make any
// RPC calls.
//
#define CHECK_RPC_STATUS() \
    if(!server_init) \
        return false

#define MIDIRPC_MAXTRIES 50 // This number * 10 is the amount of time you can try to wait for.

static boolean I_MidiPipeWrite(void *data, int len)
{
    DWORD written;
    if (WriteFile(midi_process_in_writer, data, len, &written, NULL))
    {
        return true;
    }
    else
    {
        return false;
    }
}

static boolean I_MidiPipeWaitForServer()
{
    int tries = 0;
    while(false) // TODO: Is there some way to tell if the server is listening?
    {
        I_Sleep(10);
        if (++tries >= MIDIRPC_MAXTRIES)
        {
            return false;
        }
    }
    return true;
}

//
// I_MidiPipeRegisterSong
//
// Prepare the RPC MIDI engine to receive new song data, and transmit the song
// filename to the server process.
//
boolean I_MidiPipeRegisterSong(const char *filename)
{
    BOOL wok;
    net_packet_t *packet;

    CHECK_RPC_STATUS();

    packet = NET_NewPacket(64);
    NET_WriteInt16(packet, NET_MIDIPIPE_PACKET_TYPE_PREPARE_NEW_SONG);
    NET_WriteInt16(packet, NET_MIDIPIPE_PACKET_TYPE_SET_FILENAME);
    NET_WriteString(packet, filename);
    wok = WriteFile(midi_process_in_writer, packet->data, packet->len,
        NULL, NULL);
    NET_FreePacket(packet);

    if (!wok)
    {
        DEBUGOUT("I_MidiPipeRegisterSong failed");
        return false;
    }

    DEBUGOUT("I_MidiPipeRegisterSong succeeded");
    return true;
}

//
// I_MidiPipePlaySong
//
// Tell the RPC server to start playing a song.
//
boolean I_MidiPipePlaySong(boolean looping)
{
    BOOL wok;
    net_packet_t *packet;

    CHECK_RPC_STATUS();

    packet = NET_NewPacket(3);
    NET_WriteInt16(packet, NET_MIDIPIPE_PACKET_TYPE_PLAY_SONG);
    NET_WriteInt8(packet, looping);
    wok = WriteFile(midi_process_in_writer, packet->data, packet->len,
        NULL, NULL);
    NET_FreePacket(packet);

    if (!wok)
    {
        DEBUGOUT("I_MidiPipePlaySong failed");
        return false;
    }

    DEBUGOUT("I_MidiPipePlaySong succeeded");
    return true;
}

// 
// I_MidiPipeStopSong
//
// Tell the RPC server to stop any currently playing song.
//
boolean I_MidiPipeStopSong()
{
    BOOL wok;
    net_packet_t *packet;

    CHECK_RPC_STATUS();

    packet = NET_NewPacket(2);
    NET_WriteInt16(packet, NET_MIDIPIPE_PACKET_TYPE_STOP_SONG);
    wok = WriteFile(midi_process_in_writer, packet->data, packet->len,
        NULL, NULL);
    NET_FreePacket(packet);

    if (!wok)
    {
        DEBUGOUT("I_MidiPipeStopSong failed");
        return false;
    }

    DEBUGOUT("I_MidiPipeStopSong succeeded");
    return true;
}

//
// I_MidiPipeSetVolume
//
// Change the volume level of music played by the RPC midi server.
//
boolean I_MidiPipeSetVolume(int volume)
{
    BOOL wok;
    net_packet_t *packet;

    CHECK_RPC_STATUS();

    packet = NET_NewPacket(6);
    NET_WriteInt16(packet, NET_MIDIPIPE_PACKET_TYPE_CHANGE_VOLUME);
    NET_WriteInt32(packet, volume);
    wok = WriteFile(midi_process_in_writer, packet->data, packet->len,
        NULL, NULL);
    NET_FreePacket(packet);

    if (!wok)
    {
        DEBUGOUT("I_MidiPipeSetVolume failed");
        return false;
    }

    DEBUGOUT("I_MidiPipeSetVolume succeeded");
    return true;
}

//
// I_MidiPipePauseSong
//
// Pause the music being played by the server. In actuality, due to SDL_mixer
// limitations, this just temporarily sets the volume to zero.
//
boolean I_MidiPipePauseSong()
{
    BOOL wok;
    net_packet_t *packet;

    CHECK_RPC_STATUS();

    packet = NET_NewPacket(2);
    NET_WriteInt16(packet, NET_MIDIPIPE_PACKET_TYPE_PAUSE_SONG);
    wok = WriteFile(midi_process_in_writer, packet->data, packet->len,
        NULL, NULL);
    NET_FreePacket(packet);

    if (!wok)
    {
        DEBUGOUT("I_MidiPipePauseSong failed");
        return false;
    }

    DEBUGOUT("I_MidiPipePauseSong succeeded");
    return true;
}

//
// I_MidiPipeResumeSong
//
// Resume a song after having paused it.
//
boolean I_MidiPipeResumeSong()
{
    BOOL wok;
    net_packet_t *packet;

    CHECK_RPC_STATUS();

    packet = NET_NewPacket(2);
    NET_WriteInt16(packet, NET_MIDIPIPE_PACKET_TYPE_RESUME_SONG);
    wok = WriteFile(midi_process_in_writer, packet->data, packet->len,
        NULL, NULL);
    NET_FreePacket(packet);

    if (!wok)
    {
        DEBUGOUT("I_MidiPipeResumeSong failed");
        return false;
    }

    DEBUGOUT("I_MidiPipeResumeSong succeeded");
   return true;
}

//=============================================================================
//
// Public Interface
//

//
// I_MidiPipeInitServer
//
// Start up the MIDI server.
//
boolean I_MidiPipeInitServer()
{
    struct stat sbuf;
    char filename[MAX_PATH+1];

    memset(filename, 0, sizeof(filename));
    size_t filename_len = GetModuleFileName(NULL, filename, MAX_PATH);

    // Remove filespec
    // TODO: Move this to m_misc
    char *fp = &filename[filename_len];
    while (filename <= fp && *fp != DIR_SEPARATOR)
    {
        fp--;
    }
    *(fp + 1) = '\0';
    char* module = M_StringJoin(filename, PROGRAM_PREFIX "midiproc.exe", NULL);
    char* cmdline = M_StringJoin(module, " \"" PACKAGE_STRING "\"", NULL);
    DEBUGOUT(module);
    DEBUGOUT(cmdline);

    // Look for executable file
    if(stat(module, &sbuf))
    {
        DEBUGOUT("Could not find midiproc");
        return false;
    }

    // Set up pipes
    SECURITY_ATTRIBUTES sec_attrs;
    memset(&sec_attrs, 0, sizeof(SECURITY_ATTRIBUTES));
    sec_attrs.nLength = sizeof(SECURITY_ATTRIBUTES);
    sec_attrs.bInheritHandle = TRUE;
    sec_attrs.lpSecurityDescriptor = NULL;

    if (!CreatePipe(&midi_process_in_reader, &midi_process_in_writer, &sec_attrs, 0))
    {
        DEBUGOUT("Could not initialize midiproc stdin");
        return false;
    }

    if (!SetHandleInformation(midi_process_in_writer, HANDLE_FLAG_INHERIT, 0))
    {
        DEBUGOUT("Could not disinherit midiproc stdin");
        return false;
    }

    if (!CreatePipe(&midi_process_out_reader, &midi_process_out_writer, &sec_attrs, 0))
    {
        DEBUGOUT("Could not initialize midiproc stdout/stderr");
        return false;
    }

    if (!SetHandleInformation(midi_process_out_reader, HANDLE_FLAG_INHERIT, 0))
    {
        DEBUGOUT("Could not disinherit midiproc stdin");
        return false;
    }

    // Launch the subprocess
    PROCESS_INFORMATION proc_info;
    memset(&proc_info, 0, sizeof(proc_info));

    STARTUPINFO startup_info;
    memset(&startup_info, 0, sizeof(startup_info));
    startup_info.cb = sizeof(startup_info);
    startup_info.hStdInput = midi_process_in_reader;
    startup_info.hStdOutput = midi_process_out_writer;
    startup_info.dwFlags = STARTF_USESTDHANDLES;

    boolean ok = CreateProcess(TEXT(module), TEXT(cmdline), NULL, NULL, TRUE,
        CREATE_NEW_CONSOLE, NULL, NULL, &startup_info, &proc_info);

    if (ok)
    {
        DEBUGOUT("midiproc started");
        server_init = true;
    }
    else
    {
        DEBUGOUT("failed to start midiproc");
    }

    return ok;
}

//
// I_MidiPipeInitClient
//
// Ensure that we can actually communicate with the subprocess.
//
boolean I_MidiPipeInitClient()
{
    client_init = true;
    return true;
}

//
// I_MidiPipeClientShutDown
//
// Shutdown the RPC Client
//
/* void I_MidiPipeClientShutDown()
{
    // stop the server
    if(server_init)
    {
        net_packet_t *packet;
        packet = NET_NewPacket(2);
        NET_WriteInt16(packet, NET_MIDIPIPE_PACKET_TYPE_STOP_SERVER);
        int len = SDLNet_TCP_Send(midi_socket, packet->data, packet->len);
        NET_FreePacket(packet);
        if (len < packet->len)
        {
            DEBUGOUT("Problem encountered when stopping RPC server");
        }

        server_init = false;
    }

    if (midi_socket)
    {
        SDLNet_TCP_Close(midi_socket);
        midi_socket = NULL;
    }

    client_init = false;
} */

//
// I_MidiPipeReady
//
// Returns true if both server and client initialized successfully.
//
boolean I_MidiPipeReady()
{
    CHECK_RPC_STATUS();

    return true;
}

#endif

