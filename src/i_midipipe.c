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

#include "i_midipipe.h"

#include "config.h"
#include "i_timer.h"
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

#define MIDIPIPE_MAX_WAIT 500 // Max amount of ms to wait for expected data.

static HANDLE  midi_process_in_reader;  // Input stream for midi process.
static HANDLE  midi_process_in_writer;
static HANDLE  midi_process_out_reader; // Output stream for midi process.
static HANDLE  midi_process_out_writer;

static boolean server_init = false; // if true, server was started
static boolean client_init = false; // if true, client was bound

//=============================================================================
//
// Private functions
//

//
// WritePipe
//
// Writes packet data to the subprocess' standard in.
//
static boolean WritePipe(net_packet_t *packet)
{
    BOOL ok = WriteFile(midi_process_in_writer, packet->data, packet->len,
        NULL, NULL);

    if (!ok)
    {
        return false;
    }

    return true;
}

//
// ExpectPipe
//
// Expect the contents of a packet off of the subprocess' stdout.  If the
// response is unexpected, or doesn't arrive within a specific amuont of time,
// assume the subprocess is in an unknown state.
//
static boolean ExpectPipe(net_packet_t *packet)
{
    BOOL ok;
    CHAR pipe_buffer[8192];
    DWORD pipe_buffer_read = 0;

    if (packet->len > sizeof(pipe_buffer))
    {
        // The size of the packet we're expecting is larger than our buffer
        // size, so bail out now.
        return false;
    }

    int start = I_GetTimeMS();

    do
    {
        // Wait until we see exactly the amount of data we expect on the pipe.
        ok = PeekNamedPipe(midi_process_out_reader, NULL, 0, NULL,
            &pipe_buffer_read, NULL);
        if (!ok)
        {
            goto fail;
        }
        else if (pipe_buffer_read < packet->len)
        {
            I_Sleep(1);
            continue;
        }

        // Read precisely the number of bytes we're expecting, and no more.
        ok = ReadFile(midi_process_out_reader, pipe_buffer, packet->len,
            &pipe_buffer_read, NULL);
        if (!ok || pipe_buffer_read != packet->len)
        {
            goto fail;
        }

        // Compare our data buffer to the packet.
        if (memcmp(packet->data, pipe_buffer, packet->len) != 0)
        {
            goto fail;
        }

        return true;

        // Continue looping as long as we don't exceed our maximum wait time.
    } while (start + MIDIPIPE_MAX_WAIT > I_GetTimeMS());
fail:

    // TODO: Deal with the wedged process.
    return false;
}

//=============================================================================
//
// Protocol Commands
//

//
// I_MidiPipe_RegisterSong
//
// Tells the MIDI subprocess to load a specific filename for playing.  This
// function blocks until there is an acknowledgement from the server.
//
Mix_Music *I_MidiPipe_RegisterSong(const char *filename)
{
    boolean ok;
    net_packet_t *packet;

    packet = NET_NewPacket(64);
    NET_WriteInt16(packet, NET_MIDIPIPE_PACKET_TYPE_REGISTER_SONG);
    NET_WriteString(packet, filename);
    ok = WritePipe(packet);
    NET_FreePacket(packet);

    if (!ok)
    {
        DEBUGOUT("I_MidiPipe_RegisterSong failed");
        return false;
    }

    packet = NET_NewPacket(2);
    NET_WriteInt16(packet, NET_MIDIPIPE_PACKET_TYPE_REGISTER_SONG_ACK);
    ok = ExpectPipe(packet);
    NET_FreePacket(packet);

    if (!ok)
    {
        DEBUGOUT("I_MidiPipe_RegisterSong ack failed");
        return false;
    }

    DEBUGOUT("I_MidiPipe_RegisterSong succeeded");
    return true;
}

//
// I_MidiPipe_SetVolume
//
// Tells the MIDI subprocess to set a specific volume for the song.
//
void I_MidiPipe_SetVolume(int vol)
{
    boolean ok;
    net_packet_t *packet;

    packet = NET_NewPacket(6);
    NET_WriteInt16(packet, NET_MIDIPIPE_PACKET_TYPE_SET_VOLUME);
    NET_WriteInt32(packet, vol);
    ok = WritePipe(packet);
    NET_FreePacket(packet);

    if (!ok)
    {
        DEBUGOUT("I_MidiPipe_SetVolume failed");
        return;
    }

    DEBUGOUT("I_MidiPipe_SetVolume succeeded");
}

//
// I_MidiPipe_PlaySong
//
// Tells the MIDI subprocess to play the currently loaded song.
//
void I_MidiPipe_PlaySong(int loops)
{
    boolean ok;
    net_packet_t *packet;

    packet = NET_NewPacket(6);
    NET_WriteInt16(packet, NET_MIDIPIPE_PACKET_TYPE_PLAY_SONG);
    NET_WriteInt32(packet, loops);
    ok = WritePipe(packet);
    NET_FreePacket(packet);

    if (!ok)
    {
        DEBUGOUT("I_MidiPipe_PlaySong failed");
        return;
    }

    DEBUGOUT("I_MidiPipe_PlaySong succeeded");
}

//
// I_MidiPipe_StopSong
//
// Tells the MIDI subprocess to stop playing the currently loaded song.
//
void I_MidiPipe_StopSong()
{
    boolean ok;
    net_packet_t *packet;

    packet = NET_NewPacket(2);
    NET_WriteInt16(packet, NET_MIDIPIPE_PACKET_TYPE_STOP_SONG);
    ok = WritePipe(packet);
    NET_FreePacket(packet);

    if (!ok)
    {
        DEBUGOUT("I_MidiPipe_StopSong failed");
        return;
    }

    DEBUGOUT("I_MidiPipe_StopSong succeeded");
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
boolean I_MidiPipe_InitServer()
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

#endif

