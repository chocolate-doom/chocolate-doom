// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// Copyright (C) 2013 James Haley et al.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/
//
//----------------------------------------------------------------------------
//
// DESCRIPTION:
//
// Client Interface to RPC Midi Server
//
//-----------------------------------------------------------------------------

#ifdef EE_FEATURE_MIDIRPC

#include <windows.h>
#include "midiproc.h"

#include "../hal/i_timer.h"
#include "../m_qstr.h"

#if defined(_DEBUG) && defined(EE_RPC_DEBUG)
#define DEBUGOUT(s) puts(s)
#else
#define DEBUGOUT(s)
#endif

//=============================================================================
//
// Data
//

static unsigned char *szStringBinding; // RPC client binding string
static bool serverInit = false;        // if true, server was started
static bool clientInit = false;        // if true, client was bound

// server process information
static STARTUPINFO         si;
static PROCESS_INFORMATION pi;

//=============================================================================
//
// RPC Memory Management
//

void __RPC_FAR * __RPC_USER midl_user_allocate(size_t size)
{
   return malloc(size);
}

void __RPC_USER midl_user_free(void __RPC_FAR *p)
{
   free(p);
}

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
#define CHECK_RPC_STATUS()        \
   if(!serverInit || !clientInit) \
      return false

#define MIDIRPC_MAXTRIES 50 // This number * 10 is the amount of time you can try to wait for.

static bool I_MidiRPCWaitForServer()
{
   int tries = 0;
   while(RpcMgmtIsServerListening(hMidiRPCBinding) != RPC_S_OK)
   {
      i_haltimer.Sleep(10);
      if(++tries >= MIDIRPC_MAXTRIES)
         return false;
   }
   return true;
}

//
// I_MidiRPCRegisterSong
//
// Prepare the RPC MIDI engine to receive new song data, and transmit the song
// data to the server process.
//
bool I_MidiRPCRegisterSong(void *data, int size)
{
   unsigned int rpcSize = static_cast<unsigned int>(size);

   CHECK_RPC_STATUS();

   RpcTryExcept
   {
      MidiRPC_PrepareNewSong();

      // TODO: Try passing it as one chunk; if this ends up not working, 
      // I'll have to stream it in as smaller divisions.
      MidiRPC_AddChunk(rpcSize, static_cast<byte *>(data));
   }
   RpcExcept(1)
   {
      DEBUGOUT("I_MidiRPCRegisterSong failed");
      return false;
   }
   RpcEndExcept

   DEBUGOUT("I_MidiRPCRegisterSong succeeded");
   return true;
}

//
// I_MidiRPCPlaySong
//
// Tell the RPC server to start playing a song.
//
bool I_MidiRPCPlaySong(bool looping)
{
   CHECK_RPC_STATUS();

   RpcTryExcept
   {
      MidiRPC_PlaySong(looping ? TRUE : FALSE);
   }
   RpcExcept(1)
   {
      DEBUGOUT("I_MidiRPCPlaySong failed");
      return false;
   }
   RpcEndExcept

   DEBUGOUT("I_MidiRPCPlaySong succeeded");
   return true;
}

// 
// I_MidiRPCStopSong
//
// Tell the RPC server to stop any currently playing song.
//
bool I_MidiRPCStopSong()
{
   CHECK_RPC_STATUS();

   RpcTryExcept
   {
      MidiRPC_StopSong();
   }
   RpcExcept(1)
   {
      DEBUGOUT("I_MidiRPCStopSong failed");
      return false;
   }
   RpcEndExcept

   DEBUGOUT("I_MidiRPCStopSong succeeded");
   return true;
}

//
// I_MidiRPCSetVolume
//
// Change the volume level of music played by the RPC midi server.
//
bool I_MidiRPCSetVolume(int volume)
{
   CHECK_RPC_STATUS();
   
   RpcTryExcept
   {
      MidiRPC_ChangeVolume(volume);
   }
   RpcExcept(1)
   {
      DEBUGOUT("I_MidiRPCSetVolume failed");
      return false;
   }
   RpcEndExcept

   DEBUGOUT("I_MidiRPCSetVolume succeeded");
   return true;
}

//
// I_MidiRPCPauseSong
//
// Pause the music being played by the server. In actuality, due to SDL_mixer
// limitations, this just temporarily sets the volume to zero.
//
bool I_MidiRPCPauseSong()
{
   CHECK_RPC_STATUS();

   RpcTryExcept
   {
      MidiRPC_PauseSong();
   }
   RpcExcept(1)
   {
      DEBUGOUT("I_MidiRPCPauseSong failed");
      return false;
   }
   RpcEndExcept

   DEBUGOUT("I_MidiRPCPauseSong succeeded");
   return true;
}

//
// I_MidiRPCResumeSong
//
// Resume a song after having paused it.
//
bool I_MidiRPCResumeSong()
{
   CHECK_RPC_STATUS();

   RpcTryExcept
   {
      MidiRPC_ResumeSong();
   }
   RpcExcept(1)
   {
      DEBUGOUT("I_MidiRPCResumeSong failed");
      return false;
   }
   RpcEndExcept

   DEBUGOUT("I_MidiRPCResumeSong succeeded");
   return true;
}

//=============================================================================
//
// Public Interface
//

//
// I_MidiRPCInitServer
//
// Start up the RPC MIDI server.
//
bool I_MidiRPCInitServer()
{
   struct stat sbuf;
   char filename[MAX_PATH+1];

   memset(filename, 0, sizeof(filename));
   GetModuleFileName(NULL, filename, MAX_PATH);

   qstring module;

   module = filename;
   module.removeFileSpec();
   module.pathConcatenate("midiproc.exe");
   DEBUGOUT(module.constPtr());

   // Look for executable file
   if(stat(module.constPtr(), &sbuf))
   {
      DEBUGOUT("Could not find midiproc");
      return false;
   }

   si.cb = sizeof(si);

   BOOL result = CreateProcess(module.constPtr(), NULL, NULL, NULL, FALSE,
                               0, NULL, NULL, &si, &pi);

   if(result)
   {
      DEBUGOUT("RPC server started");
      serverInit = true;
   }
   else
      DEBUGOUT("CreateProcess failed to start midiproc");

   return !!result;
}

//
// I_MidiRPCInitClient
//
// Initialize client RPC bindings and bind to the server.
//
bool I_MidiRPCInitClient()
{
   RPC_STATUS status;

   // If server didn't start, client cannot be bound.
   if(!serverInit)
      return false;

   // Compose binding string
   status =
      RpcStringBindingCompose
      (
         NULL,
         (RPC_CSTR)("ncalrpc"),
         NULL,
         (RPC_CSTR)("2d4dc2f9-ce90-4080-8a00-1cb819086970"),
         NULL,
         &szStringBinding
      );

   if(status)
   {
      DEBUGOUT("RPC binding composition failed");
      return false;
   }

   // Create binding handle
   status = RpcBindingFromStringBinding(szStringBinding, &hMidiRPCBinding);

   if(status)
   {
      DEBUGOUT("RPC client binding failed");
      return false;
   }

   DEBUGOUT("RPC client initialized");
   clientInit = true;

   return I_MidiRPCWaitForServer();
}

//
// I_MidiRPCClientShutDown
//
// Shutdown the RPC Client
//
void I_MidiRPCClientShutDown()
{
   // stop the server
   if(serverInit)
   {
      RpcTryExcept
      {
         MidiRPC_StopServer();
      }
      RpcExcept(1)
      {
         DEBUGOUT("Exception thrown when stopping RPC server");
      }
      RpcEndExcept

      serverInit = false;
   }

   if(szStringBinding)
   {
      RpcStringFree(&szStringBinding);
      szStringBinding = NULL;
   }

   if(hMidiRPCBinding)
   {
      RpcBindingFree(&hMidiRPCBinding);
      hMidiRPCBinding = NULL;
   }

   clientInit = false;
}

//
// I_MidiRPCReady
//
// Returns true if both server and client initialized successfully.
//
bool I_MidiRPCReady()
{
   CHECK_RPC_STATUS();

   return true;
}

#endif

// EOF


