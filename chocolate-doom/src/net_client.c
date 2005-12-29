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
// Revision 1.1  2005/12/29 17:48:25  fraggle
// Add initial client/server connect code.  Reorganise sources list in
// Makefile.am.
//
//
// Network client code
//

#include "doomdef.h"
#include "doomstat.h"
#include "i_system.h"
#include "net_client.h"
#include "net_defs.h"
#include "net_io.h"
#include "net_packet.h"
#include "net_server.h"

static net_addr_t *server_addr;
static net_context_t *client_context;

// connect to a server

boolean NET_ClientConnect(net_addr_t *addr)
{
    net_packet_t *packet;
    int last_send_time = -1;

    server_addr = addr;

    // create a new network I/O context and add just the
    // necessary module

    client_context = NET_NewContext();
    
    // initialise module for client mode

    if (!addr->module->InitClient())
    {
        return false;
    }

    NET_AddModule(client_context, addr->module);

    // try to connect
 
    // construct a SYN packet

    packet = NET_NewPacket(10);

    // packet type
 
    NET_WriteInt16(packet, NET_PACKET_TYPE_SYN);

    // magic number

    NET_WriteInt32(packet, NET_MAGIC_NUMBER);

    while (true)
    {
        if (I_GetTime() - last_send_time > 35)
        {
            // resend packet

            NET_SendPacket(addr, packet);
            last_send_time = I_GetTime();
        }

        // run the server, just incase we are doing a loopback
        // connect

        NET_ServerRun();
    }
}


