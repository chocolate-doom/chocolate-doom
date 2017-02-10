// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// Copyright(C) 2012 James Haley
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//-----------------------------------------------------------------------------
//
// DESCRIPTION:
//
// Win32/SDL_mixer MIDI RPC Server
//
// Uses RPC to communicate with Eternity. This allows this separate process to
// have its own independent volume control even under Windows Vista and up's 
// broken, stupid, completely useless mixer model that can't assign separate
// volumes to different devices for the same process.
//
// Seriously, how did they screw up something so fundamental?
//
//-----------------------------------------------------------------------------

#include <windows.h>
#include <stdlib.h>
#include "SDL.h"
#include "SDL_mixer.h"
#include "midiproc.h"

// Currently playing music track
static Mix_Music *music = NULL;
static SDL_RWops *rw    = NULL;

static void UnregisterSong();

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
// SDL_mixer Interface
//

//
// InitSDL
//
// Start up SDL and SDL_mixer.
//
static bool InitSDL()
{
   if(SDL_Init(SDL_INIT_AUDIO) == -1)
      return false;

   if(Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0)
      return false;

   return true;
}

//
// RegisterSong
//
static void RegisterSong(void *data, size_t size)
{
   if(music)
      UnregisterSong();

   rw    = SDL_RWFromMem(data, size);
   music = Mix_LoadMUS_RW(rw);
}

//
// StartSong
//
static void StartSong(bool loop)
{
   if(music)
      Mix_PlayMusic(music, loop ? -1 : 0);
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
   if(music)
      Mix_HaltMusic();
}

//
// UnregisterSong
//
static void UnregisterSong()
{
   if(!music)
      return;

   StopSong();
   Mix_FreeMusic(music);
   rw    = NULL;
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
// Song Buffer
//
// The MIDI program will be transmitted by the client across RPC in fixed-size
// chunks until all data has been transmitted.
//

typedef unsigned char midibyte;

class SongBuffer
{
protected:
   midibyte *buffer;    // accumulated input
   size_t    size;      // size of input
   size_t    allocated; // amount of memory allocated (>= size)

   static const int defaultSize = 128*1024; // 128 KB

public:
   // Constructor
   // Start out with an empty 128 KB buffer.
   SongBuffer()
   {
      buffer = static_cast<midibyte *>(calloc(1, defaultSize));
      size = 0;
      allocated = defaultSize;
   }

   // Destructor.
   // Release the buffer.
   ~SongBuffer()
   {
      if(buffer)
      {
         free(buffer);
         buffer = NULL;
         size = allocated = 0;
      }
   }

   //
   // addChunk
   //
   // Add a chunk of MIDI data to the buffer.
   //
   void addChunk(midibyte *data, size_t newsize)
   {
      if(size + newsize > allocated)
      {
         allocated += newsize * 2;
         buffer = static_cast<midibyte *>(realloc(buffer, allocated));
      }

      memcpy(buffer + size, data, newsize);
      size += newsize;
   }

   // Accessors

   midibyte *getBuffer() const { return buffer; }
   size_t    getSize()   const { return size;   }
};

static SongBuffer *song;

//=============================================================================
//
// RPC Server Interface
//

//
// MidiRPC_PrepareNewSong
//
// Prepare the engine to receive new song data from the RPC client.
//
void MidiRPC_PrepareNewSong()
{
   // Stop anything currently playing and free it.
   UnregisterSong();

   // free any previous song buffer
   delete song;

   // prep new song buffer
   song = new SongBuffer();
}

//
// MidiRPC_AddChunk
//
// Add a chunk of data to the song.
//
void MidiRPC_AddChunk(unsigned int count, byte *pBuf)
{
   song->addChunk(pBuf, static_cast<size_t>(count));
}

//
// MidiRPC_PlaySong
//
// Start playing the song.
//
void MidiRPC_PlaySong(boolean looping)
{
   RegisterSong(song->getBuffer(), song->getSize());
   StartSong(!!looping);
}

//
// MidiRPC_StopSong
//
// Stop the song.
//
void MidiRPC_StopSong()
{
   StopSong();
}

//
// MidiRPC_ChangeVolume
//
// Set playback volume level.
//
void MidiRPC_ChangeVolume(int volume)
{
   SetVolume(volume);
}

//
// MidiRPC_PauseSong
//
// Pause the song.
//
void MidiRPC_PauseSong()
{
   PauseSong();
}

//
// MidiRPC_ResumeSong
//
// Resume after pausing.
//
void MidiRPC_ResumeSong()
{
   ResumeSong();
}

//
// MidiRPC_StopServer
//
// Stops the RPC server so the program can shutdown.
//
void MidiRPC_StopServer()
{
   // Local shutdown tasks
   ShutdownSDL();
   delete song;
   song = NULL;

   // Stop RPC server
   RpcMgmtStopServerListening(NULL);
}

//
// RPC Server Init
//
static bool MidiRPC_InitServer()
{
   RPC_STATUS status;

   // Initialize RPC protocol
   status = 
      RpcServerUseProtseqEp
      (
         (RPC_CSTR)("ncalrpc"),
         RPC_C_PROTSEQ_MAX_REQS_DEFAULT,
         (RPC_CSTR)("2d4dc2f9-ce90-4080-8a00-1cb819086970"),
         NULL
      );

   if(status)
      return false;

   // Register server
   status = RpcServerRegisterIf(MidiRPC_v1_0_s_ifspec, NULL, NULL);

   if(status)
      return false;

   // Start listening
   status = RpcServerListen(1, RPC_C_LISTEN_MAX_CALLS_DEFAULT, FALSE);

   return !status;
}

//=============================================================================
//
// Main Program
//

//
// WinMain
//
// Application entry point.
//
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, 
                     LPSTR lpCmdLine, int nCmdShow)
{
   // Initialize SDL
   if(!InitSDL())
      return -1;

   // Initialize RPC Server
   if(!MidiRPC_InitServer())
      return -1;

   return 0;
}

// EOF

