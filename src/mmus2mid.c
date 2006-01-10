// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: mmus2mid.c 280 2006-01-10 22:14:13Z fraggle $
//
// Copyright(C) 1993-1996 Id Software, Inc.
//  Copyright (C) 1999 by
//  id Software, Chi Hoang, Lee Killough, Jim Flynn, 
//  Rand Phares, Ty Halderman
// Copyright (C) 1999-2000 by
//  Jess Haas, Nicolas Kalkhof, Colin Phipps, Florian Schulze
// Copyright(C) 2005 Simon Howard
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
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
// 02111-1307, USA.
//
// $Log$
// Revision 1.4  2006/01/10 22:14:13  fraggle
// Shut up compiler warnings
//
// Revision 1.3  2006/01/05 02:48:03  fraggle
// Fixes for big endian machines (thanks locust)
//
// Revision 1.2  2005/09/06 21:06:45  fraggle
// Newer versions of mmus2mid.c,h from prboom
//
// Revision 1.1  2005/09/05 22:50:56  fraggle
// Add mmus2mid code from prboom.  Use 'void *' for music handles.  Pass
// length of data when registering music.
//
//
// DESCRIPTION:  
//  MUS to MID conversion utility from PrBoom.
//  
//  This file supports conversion of MUS format music in memory
//  to MIDI format 1 music in memory. 
//
//  The primary routine, mmus2mid, converts a block of memory in MUS format
//  to an Allegro MIDI structure. This supports playing MUS lumps in a wad
//  file with BOOM.
//
//  Another routine, Midi2MIDI, converts a block of memory in MIDI format 1 to
//  an Allegro MIDI structure. This supports playing MIDI lumps in a wad
//  file with BOOM.
//
//  For testing purposes, and to make a utility if desired, if the symbol
//  STANDALONE is defined by uncommenting the definition below, a main
//  routine is compiled that will convert a possibly wildcarded set of MUS
//  files to a similarly named set of MIDI files.
//
//  Much of the code here is thanks to S. Bacquet's source for QMUS2MID.C
//

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#ifdef MSDOS /* proff: I don't use allegro in windows */
#include <allegro.h>
#endif /* !MSDOS */
#include "mmus2mid.h"

//#define STANDALONE  /* uncomment this to make MMUS2MID.EXE */
#ifndef STANDALONE
#include "m_swap.h"
#include "z_zone.h"
#endif

// some macros to decode mus event bit fields

#define last(e)         ((UBYTE)((e) & 0x80))
#define event_type(e)   ((UBYTE)(((e) & 0x7F) >> 4))
#define channel(e)      ((UBYTE)((e) & 0x0F))

// event types

typedef enum
{
  RELEASE_NOTE,
  PLAY_NOTE,
  BEND_NOTE,
  SYS_EVENT,
  CNTL_CHANGE,
  UNKNOWN_EVENT1,
  SCORE_END,
  UNKNOWN_EVENT2,
} mus_event_t;

// MUS format header structure

typedef struct
{
  char        ID[4];            // identifier "MUS"0x1A
  UWORD       ScoreLength;      // length of music portion
  UWORD       ScoreStart;       // offset of music portion
  UWORD       channels;         // count of primary channels
  UWORD       SecChannels;      // count of secondary channels
  UWORD       InstrCnt;         // number of instruments
} MUSheader;

// to keep track of information in a MIDI track

typedef struct Track
{
  char  velocity;
  long  deltaT;
  UBYTE lastEvt;
  long  alloced;
} TrackInfo;

// array of info about tracks

static TrackInfo track[MIDI_TRACKS];

// initial track size allocation
#define TRACKBUFFERSIZE 1024

// lookup table MUS -> MID controls
static UBYTE MUS2MIDcontrol[15] =
{
  0,         // Program change - not a MIDI control change
  0x00,      // Bank select
  0x01,      // Modulation pot
  0x07,      // Volume
  0x0A,      // Pan pot
  0x0B,      // Expression pot
  0x5B,      // Reverb depth
  0x5D,      // Chorus depth
  0x40,      // Sustain pedal
  0x43,      // Soft pedal
  0x78,      // All sounds off
  0x7B,      // All notes off
  0x7E,      // Mono
  0x7F,      // Poly
  0x79       // Reset all controllers
};

// some strings of bytes used in the midi format

static UBYTE midikey[]   =
{0x00,0xff,0x59,0x02,0x00,0x00};        // C major
static UBYTE miditempo[] =
{0x00,0xff,0x51,0x03,0x09,0xa3,0x1a};   // uS/qnote
static UBYTE midihdr[]   =
{'M','T','h','d',0,0,0,6,0,1,0,0,0,0};  // header (length 6, format 1)
static UBYTE trackhdr[]  =
{'M','T','r','k'};                      // track header

// static routine prototypes

static int TWriteByte(MIDI *mididata, int MIDItrack, UBYTE byte);
static int TWriteVarLen(MIDI *mididata, int MIDItrack, register ULONG value);
static ULONG ReadTime(const UBYTE **musptrp);
static int FirstChannelAvailable(int MUS2MIDchannel[]);
static UBYTE MidiEvent(MIDI *mididata,UBYTE midicode,UBYTE MIDIchannel,
               UBYTE MIDItrack,int nocomp);

//
// TWriteByte()
//
// write one byte to the selected MIDItrack, update current position
// if track allocation exceeded, double it
// if track not allocated, initially allocate TRACKBUFFERSIZE bytes
//
// Passed pointer to Allegro MIDI structure, number of the MIDI track being
// written, and the byte to write.
//
// Returns 0 on success, MEMALLOC if a memory allocation error occurs
//
static int TWriteByte(MIDI *mididata, int MIDItrack, UBYTE byte)
{
  ULONG pos ;

  pos = mididata->track[MIDItrack].len;
  if (pos >= (ULONG)track[MIDItrack].alloced)
  {
    track[MIDItrack].alloced =        // double allocation
      track[MIDItrack].alloced?       // or set initial TRACKBUFFERSIZE
        2*track[MIDItrack].alloced :
        TRACKBUFFERSIZE;

    if (!(mididata->track[MIDItrack].data =     // attempt to reallocate
      realloc(mididata->track[MIDItrack].data,
              track[MIDItrack].alloced)))
      return MEMALLOC;
  }
  mididata->track[MIDItrack].data[pos] = byte;
  mididata->track[MIDItrack].len++;
  return 0;
}

//
// TWriteVarLen()
//
// write the ULONG value to tracknum-th track, in midi format, which is
// big endian, 7 bits per byte, with all bytes but the last flagged by
// bit 8 being set, allowing the length to vary.
//
// Passed the Allegro MIDI structure, the track number to write,
// and the ULONG value to encode in midi format there
//
// Returns 0 if sucessful, MEMALLOC if a memory allocation error occurs
//
static int TWriteVarLen(MIDI *mididata, int tracknum, register ULONG value)
{
  register ULONG buffer;

  buffer = value & 0x7f;
  while ((value >>= 7))         // terminates because value unsigned
  {
    buffer <<= 8;               // note first value shifted in has bit 8 clear
    buffer |= 0x80;             // all succeeding values do not
    buffer += (value & 0x7f);
  }
  while (1)                     // write bytes out in opposite order
  {
    if (TWriteByte(mididata, tracknum, (UBYTE)(buffer&0xff))) // insure buffer masked
      return MEMALLOC;

    if (buffer & 0x80)
      buffer >>= 8;
    else                        // terminate on the byte with bit 8 clear
      break;
  }
  return 0;
}

//
// ReadTime()
//
// Read a time value from the MUS buffer, advancing the position in it
//
// A time value is a variable length sequence of 8 bit bytes, with all
// but the last having bit 8 set.
//
// Passed a pointer to the pointer to the MUS buffer
// Returns the integer unsigned long time value there and advances the pointer
//
static ULONG ReadTime(const UBYTE **musptrp)
{
  register ULONG timeval = 0;
  int byte;

  do    // shift each byte read up in the result until a byte with bit 8 clear
  {
    byte = *(*musptrp)++;
    timeval = (timeval << 7) + (byte & 0x7F);
  }
  while(byte & 0x80);

  return timeval;
}

//
// FirstChannelAvailable()
//
// Return the next unassigned MIDI channel number
//
// The assignment for MUS channel 15 is not counted in the caculation, that
// being percussion and always assigned to MIDI channel 9 (base 0).
//
// Passed the array of MIDI channels assigned to MUS channels
// Returns the maximum channel number unassigned unless that is 9 in which
// case 10 is returned.
//
// killough 10/7/98: changed char parameter, return values to int

static int FirstChannelAvailable(int MUS2MIDchannel[])
{
  int i ;
  int max = -1 ;

  // find the largest MIDI channel assigned so far
  for (i = 0; i < 15; i++)
    if (MUS2MIDchannel[i] > max)
      max = MUS2MIDchannel[i];

  return (max == 8 ? 10 : max+1); // skip MIDI channel 9 (percussion)
}

//
// MidiEvent()
//
// Constructs a MIDI event code, and writes it to the current MIDI track
// unless its the same as the last event code and compressio is enabled
// in which case nothing is written.
//
// Passed the Allegro MIDI structure, the midi event code, the current
// MIDI channel number, the current MIDI track number, and whether compression
// (running status) is enabled.
//
// Returns the new event code if successful, 0 if a memory allocation error
//
static UBYTE MidiEvent(MIDI *mididata,UBYTE midicode,UBYTE MIDIchannel,
        UBYTE MIDItrack,int nocomp)
{
  UBYTE newevent;

  newevent = midicode | MIDIchannel;
  if ((newevent != track[MIDItrack].lastEvt) || nocomp)
  {
    if (TWriteByte(mididata,MIDItrack, newevent))
      return 0;                                    // indicates MEMALLOC error
    track[MIDItrack].lastEvt = newevent;
  }
  return newevent;
}

//
// mmus2mid()
//
// Convert a memory buffer contain MUS data to an Allegro MIDI structure
// with specified time division and compression.
//
// Passed a pointer to the buffer containing MUS data, a pointer to the
// Allegro MIDI structure, the divisions, and a flag whether to compress.
//
// Returns 0 if successful, otherwise an error code (see mmus2mid.h).
//
int mmus2mid(const UBYTE *mus, MIDI *mididata, UWORD division, int nocomp)
{
  UWORD TrackCnt = 0;
  UBYTE evt, MUSchannel, MIDIchannel, MIDItrack=0, NewEvent;
  int i, event, data;
  const UBYTE *musptr;
  size_t muslen;
  static MUSheader MUSh;
  UBYTE MIDIchan2track[MIDI_TRACKS];  // killough 10/7/98: fix too small array
  int MUS2MIDchannel[MIDI_TRACKS];    // killough 10/7/98: fix too small array

  // copy the MUS header from the MUS buffer to the MUSh header structure

  memcpy(&MUSh,mus,sizeof(MUSheader));
  MUSh.ScoreLength = SHORT(MUSh.ScoreLength);
  MUSh.ScoreStart  = SHORT(MUSh.ScoreStart);
  MUSh.channels    = SHORT(MUSh.channels);
  MUSh.SecChannels = SHORT(MUSh.SecChannels);
  MUSh.InstrCnt    = SHORT(MUSh.InstrCnt);

  // check some things and set length of MUS buffer from internal data

  if (!(muslen = MUSh.ScoreLength + MUSh.ScoreStart))
    return MUSDATAMT;     // MUS file empty

  if (MUSh.channels > 15)       // MUSchannels + drum channel > 16
    return TOOMCHAN ;

  musptr = mus+MUSh.ScoreStart; // init musptr to start of score

  for (i = 0; i < MIDI_TRACKS; i++)   // init the track structure's tracks
  {
    MUS2MIDchannel[i] = -1;       // flag for channel not used yet
    track[i].velocity = 64;
    track[i].deltaT = 0;
    track[i].lastEvt = 0;
    //free(mididata->track[i].data);//jff 3/5/98 remove old allocations
    mididata->track[i].data=NULL;
    track[i].alloced = 0;
    mididata->track[i].len = 0;
  }

  if (!division)
    division = 70;

  // allocate the first track which is a special tempo/key track
  // note multiple tracks means midi format 1

  // set the divisions (ticks per quarter note)
  mididata->divisions = division;

  // allocate for midi tempo/key track, allow for end of track
  if (!(mididata->track[0].data =
      realloc(mididata->track[0].data,sizeof(midikey)+sizeof(miditempo)+4)))
    return MEMALLOC;

  // key C major
  memcpy(mididata->track[0].data,midikey,sizeof(midikey));
  // tempo uS/qnote
  memcpy(mididata->track[0].data+sizeof(midikey),miditempo,sizeof(miditempo));
  mididata->track[0].len = sizeof(midikey)+sizeof(miditempo);

  TrackCnt++;   // music tracks start at 1

  // process the MUS events in the MUS buffer

  do
  {
    // get a mus event, decode its type and channel fields

    event = *musptr++;
    if ((evt = event_type(event)) == SCORE_END) //jff 1/23/98 use symbol
      break;  // if end of score event, leave
    MUSchannel = channel(event);

    // if this channel not initialized, do so

    if (MUS2MIDchannel[MUSchannel] == -1)
    {
      // set MIDIchannel and MIDItrack

      MIDIchannel = MUS2MIDchannel[MUSchannel] =
        (MUSchannel == 15 ? 9 : FirstChannelAvailable(MUS2MIDchannel));
      MIDItrack = MIDIchan2track[MIDIchannel] = (UBYTE)TrackCnt++;
    }
    else // channel already allocated as a track, use those values
    {
      MIDIchannel = MUS2MIDchannel[MUSchannel];
      MIDItrack   = MIDIchan2track[MIDIchannel];
    }

    if (TWriteVarLen(mididata, MIDItrack, track[MIDItrack].deltaT))
      return MEMALLOC;
    track[MIDItrack].deltaT = 0;

    switch(evt)
    {
      case RELEASE_NOTE:
      // killough 10/7/98: Fix noise problems by not allowing compression
      if (!(NewEvent=MidiEvent(mididata,0x90,MIDIchannel,MIDItrack,1)))
        return MEMALLOC;

          data = *musptr++;
          if (TWriteByte(mididata, MIDItrack, (UBYTE)(data & 0x7F)))
            return MEMALLOC;
          if (TWriteByte(mididata, MIDItrack, 0))
            return MEMALLOC;
          break;

      case PLAY_NOTE:
      if (!(NewEvent=MidiEvent(mididata,0x90,MIDIchannel,MIDItrack,nocomp)))
        return MEMALLOC;

          data = *musptr++;
          if (TWriteByte(mididata, MIDItrack, (UBYTE)(data & 0x7F)))
            return MEMALLOC;
          if( data & 0x80 )
            track[MIDItrack].velocity = (*musptr++) & 0x7f;
          if (TWriteByte(mididata, MIDItrack, track[MIDItrack].velocity))
            return MEMALLOC;
          break;

      case BEND_NOTE:
      if (!(NewEvent=MidiEvent(mididata,0xE0,MIDIchannel,MIDItrack,nocomp)))
        return MEMALLOC;

          data = *musptr++;
          if (TWriteByte(mididata, MIDItrack, (UBYTE)((data & 1) << 6)))
            return MEMALLOC;
          if (TWriteByte(mididata, MIDItrack, (UBYTE)(data >> 1)))
            return MEMALLOC;
          break;

      case SYS_EVENT:
      if (!(NewEvent=MidiEvent(mididata,0xB0,MIDIchannel,MIDItrack,nocomp)))
        return MEMALLOC;

          data = *musptr++;
      if (data<10 || data>14)
        return BADSYSEVT;

          if (TWriteByte(mididata, MIDItrack, MUS2MIDcontrol[data]))
            return MEMALLOC;
          if (data == 12)
      {
              if (TWriteByte(mididata, MIDItrack, (UBYTE)(MUSh.channels+1)))
                return MEMALLOC;
      }
          else
      if (TWriteByte(mididata, MIDItrack, 0))
        return MEMALLOC;
          break;

      case CNTL_CHANGE:
          data = *musptr++;
      if (data>9)
        return BADCTLCHG;

          if (data)
          {
        if (!(NewEvent=MidiEvent(mididata,0xB0,MIDIchannel,MIDItrack,nocomp)))
                return MEMALLOC;

              if (TWriteByte(mididata, MIDItrack, MUS2MIDcontrol[data]))
                return MEMALLOC;
          }
          else
          {
        if (!(NewEvent=MidiEvent(mididata,0xC0,MIDIchannel,MIDItrack,nocomp)))
                return MEMALLOC;
          }
          data = *musptr++;
          if (TWriteByte(mididata, MIDItrack, (UBYTE)(data & 0x7F)))
            return MEMALLOC;
          break;

    case UNKNOWN_EVENT1:   // mus events 5 and 7
    case UNKNOWN_EVENT2:   // meaning not known
      return BADMUSCTL;

    case SCORE_END:
      break;

      default:
          return BADMUSCTL;   // exit with error
    }
    if (last(event))
    {
          ULONG DeltaTime = ReadTime(&musptr); // killough 10/7/98: make local
    for (i = 0;i < MIDI_TRACKS; i++) //jff 3/13/98 update all tracks
      track[i].deltaT += DeltaTime;  //whether allocated yet or not
    }

    }
  while ((evt != SCORE_END) && ((size_t)(musptr-mus) < muslen));

  if (evt!=SCORE_END)
    return MUSDATACOR;

  // Now add an end of track to each mididata track, correct allocation

  for (i = 0; i < MIDI_TRACKS; i++)
    if (mididata->track[i].len)
    {       // killough 10/7/98: simplify code
      if (TWriteByte(mididata, i, 0x00) || // midi end of track code
        TWriteByte(mididata, i, 0xFF) ||
        TWriteByte(mididata, i, 0x2F) ||
        TWriteByte(mididata, i, 0x00))
       return MEMALLOC;

      // jff 1/23/98 fix failure to set data NULL, len 0 for unused tracks
      // shorten allocation to proper length (important for Allegro)
      if (!(mididata->track[i].data =
        realloc(mididata->track[i].data,mididata->track[i].len)))
        return MEMALLOC;
    }
    else
    {
      free(mididata->track[i].data);
      mididata->track[i].data = NULL;
    }

  return 0;
}

void free_mididata(MIDI *mid)
{
  int i;

  for (i = 0; i < MIDI_TRACKS; i++)
    if (mid->track[i].data)
      free(mid->track[i].data);
}

//
// ReadLength()
//
// Reads the length of a chunk in a midi buffer, advancing the pointer
// 4 bytes, bigendian
//
// Passed a pointer to the pointer to a MIDI buffer
// Returns the chunk length at the pointer position
//
size_t ReadLength(UBYTE **mid)
{
  UBYTE *midptr = *mid;

  size_t length = (*midptr++)<<24;
  length += (*midptr++)<<16;
  length += (*midptr++)<<8;
  length += *midptr++;
  *mid = midptr;
  return length;
}

//
// MidiToMIDI()
//
// Convert an in-memory copy of a MIDI format 0 or 1 file to
// an Allegro MIDI structure, that is valid or has been zeroed
//
// Passed a pointer to a memory buffer with MIDI format music in it and a
// pointer to an Allegro MIDI structure.
//
// Returns 0 if successful, BADMIDHDR if the buffer is not MIDI format
//
int MidiToMIDI(UBYTE *mid,MIDI *mididata)
{
  int i;
  int ntracks;

  // read the midi header

  if (memcmp(mid,midihdr,4))
    return BADMIDHDR;

  mididata->divisions = (mid[12]<<8)+mid[13];
  ntracks = (mid[10]<<8)+mid[11];

  if (ntracks>=MIDI_TRACKS)
    return BADMIDHDR;

  mid += 4;
  {  // killough 10/7/98: fix mid from being modified twice before sequence pt.
    size_t t = ReadLength(&mid);            // seek past header
    mid += t;
  }

  // now read each track

  for (i=0;i<ntracks;i++)
  {
    while (memcmp(mid,trackhdr,4))    // simply skip non-track data
    {
      mid += 4;
      {
        size_t t = ReadLength(&mid);            // seek past header
        mid += t;  // killough 10/7/98: prevent mid undefined behavior
      }
    }
    mid += 4;
    mididata->track[i].len = ReadLength(&mid);  // get length, move mid past it

    // read a track
    mididata->track[i].data = realloc(mididata->track[i].data,mididata->track[i].len);
    memcpy(mididata->track[i].data,mid,mididata->track[i].len);
    mid += mididata->track[i].len;
  }
  for (;i<MIDI_TRACKS;i++)
    if (mididata->track[i].len)
    {
      free(mididata->track[i].data);
      mididata->track[i].data = NULL;
      mididata->track[i].len = 0;
    }
  return 0;
}

//#ifdef STANDALONE /* this code unused by BOOM provided for future portability */
//                  /* it also provides a MUS to MID file converter*/
// proff: I moved this down, because I need MIDItoMidi

static void TWriteLength(UBYTE **midiptr,ULONG length);

//
// TWriteLength()
//
// Write the length of a MIDI chunk to a midi buffer. The length is four
// bytes and is written byte-reversed for bigendian. The pointer to the
// midi buffer is advanced.
//
// Passed a pointer to the pointer to a midi buffer, and the length to write
// Returns nothing
//
static void TWriteLength(UBYTE **midiptr,ULONG length)
{
// proff: Added typecast to avoid warning
  *(*midiptr)++ = (unsigned char)((length>>24)&0xff);
  *(*midiptr)++ = (unsigned char)((length>>16)&0xff);
  *(*midiptr)++ = (unsigned char)((length>>8)&0xff);
  *(*midiptr)++ = (unsigned char)((length)&0xff);
}

//
// MIDIToMidi()
//
// This routine converts an Allegro MIDI structure to a midi 1 format file
// in memory. It is used to support memory MUS -> MIDI conversion
//
// Passed a pointer to an Allegro MIDI structure, a pointer to a pointer to
// a buffer containing midi data, and a pointer to a length return.
// Returns 0 if successful, MEMALLOC if a memory allocation error occurs
//
int MIDIToMidi(MIDI *mididata,UBYTE **mid,int *midlen)
{
  size_t total;
  int i,ntrks;
  UBYTE *midiptr;

  // calculate how long the mid buffer must be, and allocate

  total = sizeof(midihdr);
  for (i=0,ntrks=0;i<MIDI_TRACKS;i++)
    if (mididata->track[i].len)
    {
      total += 8 + mididata->track[i].len; // Track hdr + track length
      ntrks++;
    }
  if ((*mid = malloc(total))==NULL)
    return MEMALLOC;


  // fill in number of tracks and bigendian divisions (ticks/qnote)

  midihdr[10] = 0;
  midihdr[11] = (UBYTE)ntrks;   // set number of tracks in header
  midihdr[12] = (mididata->divisions>>8) & 0x7f;
  midihdr[13] = (mididata->divisions) & 0xff;

  // write the midi header

  midiptr = *mid;
  memcpy(midiptr,midihdr,sizeof(midihdr));
  midiptr += sizeof(midihdr);

  // write the tracks

  for (i=0;i<MIDI_TRACKS;i++)
  {
    if (mididata->track[i].len)
    {
      memcpy(midiptr,trackhdr,sizeof(trackhdr));    // header
      midiptr += sizeof(trackhdr);
      TWriteLength(&midiptr,mididata->track[i].len);  // track length
      // data
      memcpy(midiptr,mididata->track[i].data,mididata->track[i].len);
      midiptr += mididata->track[i].len;
    }
  }

  // return length information

  *midlen = midiptr - *mid;

  return 0;
}

#ifdef STANDALONE /* this code unused by BOOM provided for future portability */
                  /* it also provides a MUS to MID file converter*/
// proff: I moved this down, because I need MIDItoMidi

//
// FreeTracks()
//
// Free all track allocations in the MIDI structure
//
// Passed a pointer to an Allegro MIDI structure
// Returns nothing
//
static void FreeTracks(MIDI *mididata)
{
  int i;

  for (i=0; i<MIDI_TRACKS; i++)
  {
    free(mididata->track[i].data);
    mididata->track[i].data = NULL;
    mididata->track[i].len = 0;
  }
}

//
// main()
//
// Main routine that will convert a globbed set of MUS files to the
// correspondingly named MID files using mmus2mid(). Only compiled
// if the STANDALONE symbol is defined.
//
// Passed the command line arguments, returns 0 if successful
//
int main(int argc,char **argv)
{
  FILE *musst,*midst;
  char musfile[FILENAME_MAX],midfile[FILENAME_MAX];
  MUSheader MUSh;
  UBYTE *mus,*mid;
  static MIDI mididata;
  int err,midlen;
  char *p,*q;
  int i;

  if (argc<2)
  {
    //jff 8/3/98 use logical output routine
    lprintf(LO_INFO,"Usage: MMUS2MID musfile[.MUS]\n");
    lprintf(LO_INFO,"writes musfile.MID as output\n");
    lprintf(LO_INFO,"musfile may contain wildcards\n");
    exit(1);
  }

  for (i=1;i<argc;i++)
  {
    strcpy(musfile,argv[i]);
    p = strrchr(musfile,'.');
    q = strrchr(musfile,'\\');
    if (p && (!q || q<p)) *p='\0';
    strcpy(midfile,musfile);
    strcat(musfile,".MUS");
    strcat(midfile,".MID");

    musst = fopen(musfile,"rb");
    if (musst)
    {
      fread(&MUSh,sizeof(MUSheader),1,musst);
      mus = malloc(MUSh.ScoreLength+MUSh.ScoreStart);
      if (mus)
      {
        fseek(musst,0,SEEK_SET);
        if (!fread(mus,MUSh.ScoreLength+MUSh.ScoreStart,1,musst))
        {
          //jff 8/3/98 use logical output routine
          lprintf(LO_FATAL,"Error reading MUS file\n");
          free(mus);
          exit(1);
        }
        fclose(musst);
      }
      else
      {
        //jff 8/3/98 use logical output routine
        lprintf(LO_FATAL,"Out of memory\n");
        free(mus);
        exit(1);
      }

      err = mmus2mid(mus,&mididata,89,1);
      if (err)
      {
        //jff 8/3/98 use logical output routine
        lprintf(LO_FATAL,"Error converting MUS file to MIDI: %d\n",err);
        exit(1);
      }
      free(mus);

      MIDIToMidi(&mididata,&mid,&midlen);

      midst = fopen(midfile,"wb");
      if (midst)
      {
        if (!fwrite(mid,midlen,1,midst))
        {
          //jff 8/3/98 use logical output routine
          lprintf(LO_FATAL,"Error writing MIDI file\n");
          FreeTracks(&mididata);
          free(mid);
          exit(1);
        }
        fclose(midst);
      }
      else
      {
        //jff 8/3/98 use logical output routine
        lprintf(LO_FATAL,"Can't open MIDI file for output: %s\n", midfile);
        FreeTracks(&mididata);
        free(mid);
        exit(1);
      }
    }
    else
    {
      //jff 8/3/98 use logical output routine
      lprintf(LO_FATAL,"Can't open MUS file for input: %s\n", midfile);
      exit(1);
    }

    //jff 8/3/98 use logical output routine
    lprintf(LO_CONFIRM,"MUS file %s converted to MIDI file %s\n",musfile,midfile);
    FreeTracks(&mididata);
    free(mid);
  }
  exit(0);
}

#endif
