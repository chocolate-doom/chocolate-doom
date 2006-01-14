// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id$
//
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
// Revision 1.9  2006/01/14 02:06:48  fraggle
// Include the game version in the settings structure.
//
// Revision 1.8  2006/01/11 01:37:53  fraggle
// ticcmd diffs: allow compare and patching ticcmds, and reading/writing
// ticdiffs to packets.
//
// Revision 1.7  2006/01/10 19:59:26  fraggle
// Reliable packet transport mechanism
//
// Revision 1.6  2006/01/08 04:52:26  fraggle
// Allow the server to reject clients
//
// Revision 1.5  2006/01/08 02:53:05  fraggle
// Send keepalives if the connection is not doing anything else.
// Send all packets using a new NET_Conn_SendPacket to support this.
//
// Revision 1.4  2006/01/01 23:54:31  fraggle
// Client disconnect code
//
// Revision 1.3  2005/12/30 18:58:22  fraggle
// Fix client code to correctly send reply to server on connection.
// Add "waiting screen" while waiting for the game to start.
// Hook in the new networking code into the main game code.
//
// Revision 1.2  2005/12/29 17:48:25  fraggle
// Add initial client/server connect code.  Reorganise sources list in
// Makefile.am.
//
// Revision 1.1  2005/10/30 19:56:15  fraggle
// Add foundation code for the new networking system
//
//
// DESCRIPTION:
//     Definitions for use in networking code.
//
//-----------------------------------------------------------------------------

#ifndef NET_DEFS_H
#define NET_DEFS_H 

#include "doomtype.h"
#include "d_ticcmd.h"

typedef struct _net_module_s net_module_t;
typedef struct _net_packet_s net_packet_t;
typedef struct _net_addr_s net_addr_t;
typedef struct _net_context_s net_context_t;

struct _net_packet_s
{
    byte *data;
    int len;
    int alloced;
    int pos;
};

struct _net_module_s
{
    // Initialise this module for use as a client

    boolean (*InitClient)(void);

    // Initialise this module for use as a server

    boolean (*InitServer)(void);

    // Send a packet

    void (*SendPacket)(net_addr_t *addr, net_packet_t *packet);

    // Check for new packets to receive
    //
    // Returns true if packet received

    boolean (*RecvPacket)(net_addr_t **addr, net_packet_t **packet);

    // Converts an address to a string

    void (*AddrToString)(net_addr_t *addr, char *buffer, int buffer_len);

    // Free back an address when no longer in use

    void (*FreeAddress)(net_addr_t *addr);

    // Try to resolve a name to an address

    net_addr_t *(*ResolveAddress)(char *addr);
};

// net_addr_t

struct _net_addr_s
{
    net_module_t *module;
    void *handle;
};

// magic number sent when connecting to check this is a valid client

#define NET_MAGIC_NUMBER 3436803284U

// header field value indicating that the packet is a reliable packet

#define NET_RELIABLE_PACKET (1 << 15)

// packet types

typedef enum 
{
    NET_PACKET_TYPE_SYN,
    NET_PACKET_TYPE_ACK,
    NET_PACKET_TYPE_REJECTED,
    NET_PACKET_TYPE_KEEPALIVE,
    NET_PACKET_TYPE_WAITING_DATA,
    NET_PACKET_TYPE_GAMESTART,
    NET_PACKET_TYPE_GAMEDATA,
    NET_PACKET_TYPE_DISCONNECT,
    NET_PACKET_TYPE_DISCONNECT_ACK,
    NET_PACKET_TYPE_RELIABLE_ACK,
} net_packet_type_t;

typedef struct 
{
    int ticdup;
    int extratics;
    int deathmatch;
    int episode;
    int map;
    int skill;
    int gameversion;
} net_gamesettings_t;

#define NET_TICDIFF_FORWARD      (1 << 0)
#define NET_TICDIFF_SIDE         (1 << 1)
#define NET_TICDIFF_TURN         (1 << 2)
#define NET_TICDIFF_BUTTONS      (1 << 3)
#define NET_TICDIFF_CONSISTANCY  (1 << 4)
#define NET_TICDIFF_CHATCHAR     (1 << 5)

typedef struct
{
    unsigned int diff;
    ticcmd_t cmd;
} net_ticdiff_t;

#endif /* #ifndef NET_DEFS_H */

