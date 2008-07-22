// Emacs style mode select   -*- C++ -*- 
// -----------------------------------------------------------------------------
// ########   ###### #####   #####  ######   ######  ######
// ##     ##  ##     ##  ## ##  ## ##    ## ##    ## ##   ##
// ##     ##  ##     ##   ###   ## ##    ## ##    ## ##    ##
// ########   ####   ##    #    ## ##    ## ##    ## ##    ##
// ##    ##   ##     ##         ## ##    ## ##    ## ##    ##
// ##     ##  ##     ##         ## ##    ## ##    ## ##   ##
// ##      ## ###### ##         ##  ######   ######  ######
//                      http://remood.sourceforge.net/
// -----------------------------------------------------------------------------
// Project Leader:    GhostlyDeath           (ghostlydeath@gmail.com)
// Project Co-Leader: RedZTag                (jostol27@gmail.com)
// Members:           Demyx                  (demyx@endgameftw.com)
//                    StealthCP              (?)
// -----------------------------------------------------------------------------
// Copyright (C) 1998-2000 by DooM Legacy Team.
// Portions Copyright (C) 2008 ReMooD Team.
// -----------------------------------------------------------------------------
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// -----------------------------------------------------------------------------
// DESCRIPTION:  Macro to read/write from/to a char*, used for packet cration and such...

#ifndef __BYTEPTR_H__
#define __BYTEPTR_H__

#include "doomtype.h"
#include <stddef.h>				// for size_t

Int8 ReadInt8(Int8 ** Ptr);
Int16 ReadInt16(Int16 ** Ptr);
Int32 ReadInt32(Int32 ** Ptr);
Int64 ReadInt64(Int64 ** Ptr);
UInt8 ReadUInt8(UInt8 ** Ptr);
UInt16 ReadUInt16(UInt16 ** Ptr);
UInt32 ReadUInt32(UInt32 ** Ptr);
UInt64 ReadUInt64(UInt64 ** Ptr);

void WriteInt8(Int8 ** Ptr, Int8 Val);
void WriteInt16(Int16 ** Ptr, Int16 Val);
void WriteInt32(Int32 ** Ptr, Int32 Val);
void WriteInt64(Int64 ** Ptr, Int64 Val);
void WriteUInt8(UInt8 ** Ptr, UInt8 Val);
void WriteUInt16(UInt16 ** Ptr, UInt16 Val);
void WriteUInt32(UInt32 ** Ptr, UInt32 Val);
void WriteUInt64(UInt64 ** Ptr, UInt64 Val);
void WriteString(char **Ptr, char *Val);
void WriteStringN(char **Ptr, char *Val, size_t n);

#ifndef __BIG_ENDIAN__
//
// Little-endian machines
//
#define writeshort(p,b)     *(Int16*)  (p)   = b
#define writelong(p,b)      *(Int32 *)  (p)   = b

#define WRITECHAR(p,b)		WriteInt8((Int8**)&(p), (Int8)(b))
#define WRITEBYTE(p,b)		WriteUInt8((UInt8**)&(p), (UInt8)(b))
#define WRITESHORT(p,b)		WriteInt16((Int16**)&(p), (Int16)(b))
#define WRITEUSHORT(p,b)	WriteUInt16((UInt16**)&(p), (UInt16)(b))
#define WRITELONG(p,b)		WriteInt32((Int32**)&(p), (Int32)(b))
#define WRITEULONG(p,b)		WriteUInt32((UInt32**)&(p), (UInt32)(b))
#define WRITEFIXED(p,b)		WriteInt32((Int32**)&(p), (Int32)(b))
#define WRITEANGLE(p,b)		WriteUInt32((UInt32**)&(p), (UInt32)(b))

/*#define WRITEBYTE(p,b)      *((byte   *)p)++ = b
#define WRITECHAR(p,b)      *((char   *)p)++ = b
#define WRITESHORT(p,b)     *((short  *)p)++ = b
#define WRITEUSHORT(p,b)    *((USHORT *)p)++ = b
#define WRITELONG(p,b)      *((long   *)p)++ = b
#define WRITEULONG(p,b)     *((ULONG  *)p)++ = b
#define WRITEFIXED(p,b)     *((fixed_t*)p)++ = b
#define WRITEANGLE(p,b)     *((angle_t*)p)++ = b*/
#define WRITESTRING(p,b)    { int tmp_i=0; do { WRITECHAR(p,b[tmp_i]); } while(b[tmp_i++]); }
#define WRITESTRINGN(p,b,n) { int tmp_i=0; do { WRITECHAR(p,b[tmp_i]); if(!b[tmp_i]) break;tmp_i++; } while(tmp_i<n); }
#define WRITEMEM(p,s,n)     memcpy(p, s, n);p+=n

#define readshort(p)	    *((Int16  *)(p))
#define readlong(p)	    *((Int32   *)(p))

#define READCHAR(p)		ReadInt8((Int8**)(&(p)))
#define READBYTE(p)		ReadUInt8((UInt8**)(&(p)))
#define READSHORT(p)	ReadInt16((Int16**)(&(p)))
#define READUSHORT(p)	ReadUInt16((UInt16**)(&(p)))
#define READLONG(p)		ReadInt32((Int32**)(&(p)))
#define	READULONG(p)	ReadUInt32((UInt32**)(&(p)))
#define READFIXED(p)		ReadInt32((Int32**)(&(p)))
#define READANGLE(p)		ReadUInt32((UInt32**)(&(p)))

/*#define READBYTE(p)         *((byte   *)p)++
#define READCHAR(p)         *((char   *)p)++
#define READSHORT(p)        *((short  *)p)++
#define READUSHORT(p)       *((USHORT *)p)++
#define READLONG(p)         *((long   *)p)++
#define READULONG(p)        *((ULONG  *)p)++*/
//#define READFIXED(p)        *((fixed_t*)p)++
//#define READANGLE(p)        *((angle_t*)p)++
#define READSTRING(p,s)     { int tmp_i=0; do { s[tmp_i]=READBYTE(p);  } while(s[tmp_i++]); }
#define SKIPSTRING(p)       while(READBYTE(p))
#define READMEM(p,s,n)      memcpy(s, p, n);p+=n

#else
//
// definitions for big-endian machines with alignment constraints.
//
// Write a value to a little-endian, unaligned destination.
//
static inline void writeshort(void *ptr, int val)
{
	char *cp = ptr;
	cp[0] = val;
	val >>= 8;
	cp[1] = val;
}

static inline void writelong(void *ptr, int val)
{
	char *cp = ptr;
	cp[0] = val;
	val >>= 8;
	cp[1] = val;
	val >>= 8;
	cp[2] = val;
	val >>= 8;
	cp[3] = val;
}

#define WRITEBYTE(p,b)      *((byte   *)p)++ = (b)
#define WRITECHAR(p,b)      *((char   *)p)++ = (b)
#define WRITESHORT(p,b)     writeshort(((short *)p)++,  (b))
#define WRITEUSHORT(p,b)    writeshort(((u_short*)p)++, (b))
#define WRITELONG(p,b)      writelong (((long  *)p)++,  (b))
#define WRITEULONG(p,b)     writelong (((u_long *)p)++, (b))
#define WRITEFIXED(p,b)     writelong (((fixed_t*)p)++,  (b))
#define WRITEANGLE(p,b)     writelong (((angle_t*)p)++, (long) (b))
#define WRITESTRING(p,b)    { int tmp_i=0; do { WRITECHAR(p,b[tmp_i]); } while(b[tmp_i++]); }
#define WRITESTRINGN(p,b,n) { int tmp_i=0; do { WRITECHAR(p,b[tmp_i]); if(!b[tmp_i]) break;tmp_i++; } while(tmp_i<n); }
#define WRITEMEM(p,s,n)     memcpy(p, s, n);p+=n

// Read a signed quantity from little-endian, unaligned data.
// 
static inline short readshort(void *ptr)
{
	char *cp = ptr;
	u_char *ucp = ptr;
	return (cp[1] << 8) | ucp[0];
}

static inline u_short readushort(void *ptr)
{
	u_char *ucp = ptr;
	return (ucp[1] << 8) | ucp[0];
}

static inline long readlong(void *ptr)
{
	char *cp = ptr;
	u_char *ucp = ptr;
	return (cp[3] << 24) | (ucp[2] << 16) | (ucp[1] << 8) | ucp[0];
}

static inline u_long readulong(void *ptr)
{
	u_char *ucp = ptr;
	return (ucp[3] << 24) | (ucp[2] << 16) | (ucp[1] << 8) | ucp[0];
}

#define READBYTE(p)         *((byte   *)p)++
#define READCHAR(p)         *((char   *)p)++
#define READSHORT(p)        readshort ( ((short*) p)++)
#define READUSHORT(p)       readushort(((USHORT*) p)++)
#define READLONG(p)         readlong  (  ((long*) p)++)
#define READULONG(p)        readulong ( ((ULONG*) p)++)
#define READFIXED(p)        readlong  (  ((long*) p)++)
#define READANGLE(p)        readulong ( ((ULONG*) p)++)
#define READSTRING(p,s)     { int tmp_i=0; do { s[tmp_i]=READBYTE(p);  } while(s[tmp_i++]); }
#define SKIPSTRING(p)       while(READBYTE(p))
#define READMEM(p,s,n)      memcpy(s, p, n);p+=n
#endif							//__BIG_ENDIAN__

#endif							/* __BYTEPTR_H__ */
