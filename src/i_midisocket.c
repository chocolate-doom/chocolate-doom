//
// Copyright(C) 2013 James Haley et al.
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

#include "i_midisocket.h"

#include "m_misc.h"
#include "net_packet.h"

#if defined(_DEBUG)
#define DEBUGOUT(s) puts(s)
#else
#define DEBUGOUT(s)
#endif

typedef enum {
	NET_MIDISOCKET_PACKET_TYPE_PREPARE_NEW_SONG,
    NET_MIDISOCKET_PACKET_TYPE_REGISTER_SONG,
	NET_MIDISOCKET_PACKET_TYPE_PLAY_SONG,
	NET_MIDISOCKET_PACKET_TYPE_STOP_SONG,
	NET_MIDISOCKET_PACKET_TYPE_CHANGE_VOLUME,
	NET_MIDISOCKET_PACKET_TYPE_PAUSE_SONG,
	NET_MIDISOCKET_PACKET_TYPE_RESUME_SONG,
	NET_MIDISOCKET_PACKET_TYPE_STOP_SERVER
} net_midisocket_packet_type_t;

//=============================================================================
//
// Data
//

static TCPsocket midi_socket;       // Client socket
static boolean server_init = false; // if true, server was started
static boolean client_init = false; // if true, client was bound

// server process information
static STARTUPINFO         si;
static PROCESS_INFORMATION pi;

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
    if(!server_init || !client_init) \
        return false

#define MIDIRPC_MAXTRIES 50 // This number * 10 is the amount of time you can try to wait for.

static boolean I_MidiSocketWaitForServer()
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
// I_MidiSocketRegisterSong
//
// Prepare the RPC MIDI engine to receive new song data, and transmit the song
// filename to the server process.
//
boolean I_MidiSocketRegisterSong(const char *filename)
{
    CHECK_RPC_STATUS();

    net_packet_t *packet;
    packet = NET_NewPacket(2);
    NET_WriteInt16(packet, NET_MIDISOCKET_PACKET_TYPE_PREPARE_NEW_SONG);
    int len = SDLNet_TCP_Send(midi_socket, packet->data, packet->len);
    if (len < packet->len)
    {
        goto fail;
    }
    NET_FreePacket(packet);

    packet = NET_NewPacket(64);
    NET_WriteInt16(packet, NET_MIDISOCKET_PACKET_TYPE_REGISTER_SONG);
    NET_WriteString(packet, filename);
    len = SDLNet_TCP_Send(midi_socket, packet->data, packet->len);
    if (len < packet->len)
    {
        goto fail;
    }
    NET_FreePacket(packet);

    DEBUGOUT("I_MidiSocketRegisterSong succeeded");
    return true;

fail:
    NET_FreePacket(packet);

    DEBUGOUT("I_MidiSocketRegisterSong failed");
    return false;
}

//
// I_MidiSocketPlaySong
//
// Tell the RPC server to start playing a song.
//
boolean I_MidiSocketPlaySong(boolean looping)
{
    CHECK_RPC_STATUS();

    net_packet_t *packet;
    packet = NET_NewPacket(3);
    NET_WriteInt16(packet, NET_MIDISOCKET_PACKET_TYPE_PLAY_SONG);
    NET_WriteInt8(packet, looping);
    int len = SDLNet_TCP_Send(midi_socket, packet->data, packet->len);
    NET_FreePacket(packet);
    if (len < packet->len)
    {
        DEBUGOUT("I_MidiSocketPlaySong failed");
        return false;
    }

    DEBUGOUT("I_MidiSocketPlaySong succeeded");
    return true;
}

// 
// I_MidiSocketStopSong
//
// Tell the RPC server to stop any currently playing song.
//
boolean I_MidiSocketStopSong()
{
    CHECK_RPC_STATUS();

    net_packet_t *packet;
    packet = NET_NewPacket(2);
    NET_WriteInt16(packet, NET_MIDISOCKET_PACKET_TYPE_STOP_SONG);
    int len = SDLNet_TCP_Send(midi_socket, packet->data, packet->len);
    NET_FreePacket(packet);
    if (len < packet->len)
    {
        DEBUGOUT("I_MidiSocketStopSong failed");
        return false;
    }

    DEBUGOUT("I_MidiSocketStopSong succeeded");
    return true;
}

//
// I_MidiSocketSetVolume
//
// Change the volume level of music played by the RPC midi server.
//
boolean I_MidiSocketSetVolume(int volume)
{
    CHECK_RPC_STATUS();

    net_packet_t *packet;
    packet = NET_NewPacket(6);
    NET_WriteInt16(packet, NET_MIDISOCKET_PACKET_TYPE_CHANGE_VOLUME);
    NET_WriteInt32(packet, volume);
    int len = SDLNet_TCP_Send(midi_socket, packet->data, packet->len);
    NET_FreePacket(packet);
    if (len < packet->len)
    {
        DEBUGOUT("I_MidiSocketSetVolume failed");
        return false;
    }

    DEBUGOUT("I_MidiSocketSetVolume succeeded");
    return true;
}

//
// I_MidiSocketPauseSong
//
// Pause the music being played by the server. In actuality, due to SDL_mixer
// limitations, this just temporarily sets the volume to zero.
//
boolean I_MidiSocketPauseSong()
{
    CHECK_RPC_STATUS();

    net_packet_t *packet;
    packet = NET_NewPacket(2);
    NET_WriteInt16(packet, NET_MIDISOCKET_PACKET_TYPE_PAUSE_SONG);
    int len = SDLNet_TCP_Send(midi_socket, packet->data, packet->len);
    NET_FreePacket(packet);
    if (len < packet->len)
    {
        DEBUGOUT("I_MidiSocketPauseSong failed");
        return false;
    }

    DEBUGOUT("I_MidiSocketPauseSong succeeded");
    return true;
}

//
// I_MidiSocketResumeSong
//
// Resume a song after having paused it.
//
boolean I_MidiSocketResumeSong()
{
    CHECK_RPC_STATUS();

    net_packet_t *packet;
    packet = NET_NewPacket(2);
    NET_WriteInt16(packet, NET_MIDISOCKET_PACKET_TYPE_RESUME_SONG);
    int len = SDLNet_TCP_Send(midi_socket, packet->data, packet->len);
    NET_FreePacket(packet);
    if (len < packet->len)
    {
        DEBUGOUT("I_MidiSocketResumeSong failed");
        return false;
    }

    DEBUGOUT("I_MidiSocketResumeSong succeeded");
   return true;
}

//=============================================================================
//
// Public Interface
//

//
// I_MidiSocketInitServer
//
// Start up the RPC MIDI server.
//
boolean I_MidiSocketInitServer()
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
    char* module = M_StringJoin(filename, "midiproc.exe", NULL);
    DEBUGOUT(module);

    // Look for executable file
    if(stat(module, &sbuf))
    {
        DEBUGOUT("Could not find midiproc");
        return false;
    }

    si.cb = sizeof(si);

    boolean result = CreateProcess(module, NULL, NULL, NULL, FALSE,
                                   0, NULL, NULL, &si, &pi);

    if (result)
    {
        DEBUGOUT("RPC server started");
        server_init = true;
    }
    else
    {
        DEBUGOUT("CreateProcess failed to start midiproc");
    }

    return result;
}

//
// I_MidiSocketInitClient
//
// Initialize client RPC bindings and bind to the server.
//
boolean I_MidiSocketInitClient()
{
    IPaddress ipaddress;

    // If server didn't start, client cannot be bound.
    if (!server_init)
    {
        goto fail;
    }

    // Resolve localhost to an IP address.
    if (SDLNet_ResolveHost(&ipaddress, "localhost", 2993) == -1)
    {
        goto fail;
    }

    // Connect to the server.
    midi_socket = SDLNet_TCP_Open(&ipaddress);
    if (midi_socket == NULL)
    {
        goto fail;
    }

    DEBUGOUT("RPC client initialized");
    client_init = true;

    return I_MidiSocketWaitForServer();

fail:
    DEBUGOUT("RPC client binding failed");
    return false;
}

//
// I_MidiSocketClientShutDown
//
// Shutdown the RPC Client
//
void I_MidiSocketClientShutDown()
{
    // stop the server
    if(server_init)
    {
        net_packet_t *packet;
        packet = NET_NewPacket(2);
        NET_WriteInt16(packet, NET_MIDISOCKET_PACKET_TYPE_STOP_SERVER);
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
}

//
// I_MidiSocketReady
//
// Returns true if both server and client initialized successfully.
//
boolean I_MidiSocketReady()
{
    CHECK_RPC_STATUS();

    return true;
}

#endif

