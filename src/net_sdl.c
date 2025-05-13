//
// Copyright(C) 2005-2014 Simon Howard
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
//     Networking module which uses SDL_net
//

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "doomtype.h"
#include "safe.h"

#include "i_system.h"
#include "m_argv.h"
#include "m_misc.h"
#include "net_defs.h"
#include "net_io.h"
#include "net_packet.h"
#include "net_sdl.h"
#include "z_zone.h"

//
// NETWORKING
//


#ifndef DISABLE_SDL2NET


#include <SDL_net.h>

#define TCPSOCKET_CLOSED ((TCPsocket) -1)
#define DEFAULT_PORT 2342
#define MAX_FRAME_LEN 1024
#define FRAMECHAR 0x70

static boolean tcp_initted, udp_initted = false;
static int port = DEFAULT_PORT;
static UDPsocket udpsocket;
static UDPpacket *recvpacket;

static TCPsocket tcpsocket;
static SDLNet_SocketSet active_sockets;

typedef enum
{
    IP_PROTOCOL_UDP,
    IP_PROTOCOL_TCP,
} ip_protocol_t;

typedef struct
{
    net_addr_t net_addr;
    ip_protocol_t protocol;
    IPaddress sdl_addr;

    // If there is an open TCP connection from this address, this is
    // the socket for the connection; this reuses the address
    // table to avoid maintaining a separate table.
    TCPsocket tcp_sock;
} addrpair_t;

static addrpair_t **addr_table;
static int addr_table_size = -1;

// Initializes the address table

static void NET_SDL_InitAddrTable(void)
{
    addr_table_size = 16;

    addr_table = Z_Malloc(sizeof(addrpair_t *) * addr_table_size,
                          PU_STATIC, 0);
    memset(addr_table, 0, sizeof(addrpair_t *) * addr_table_size);
}

static boolean AddressesEqual(IPaddress *a, IPaddress *b)
{
    return a->host == b->host
        && a->port == b->port;
}

// Finds an address by searching the table.  If the address is not found,
// it is added to the table.

static net_addr_t *FindAddress(ip_protocol_t protocol, IPaddress *addr)
{
    addrpair_t *new_entry;
    int empty_entry = -1;
    int i;

    if (addr_table_size < 0)
    {
        NET_SDL_InitAddrTable();
    }

    for (i=0; i<addr_table_size; ++i)
    {
        if (addr_table[i] != NULL
         && protocol == addr_table[i]->protocol
         && AddressesEqual(addr, &addr_table[i]->sdl_addr))
        {
            return &addr_table[i]->net_addr;
        }

        if (empty_entry < 0 && addr_table[i] == NULL)
            empty_entry = i;
    }

    // Was not found in list.  We need to add it.

    // Is there any space in the table? If not, increase the table size

    if (empty_entry < 0)
    {
        addrpair_t **new_addr_table;
        int new_addr_table_size;

        // after reallocing, we will add this in as the first entry
        // in the new block of memory

        empty_entry = addr_table_size;
        
        // allocate a new array twice the size, init to 0 and copy 
        // the existing table in.  replace the old table.

        new_addr_table_size = addr_table_size * 2;
        new_addr_table = Z_Malloc(sizeof(addrpair_t *) * new_addr_table_size,
                                  PU_STATIC, 0);
        memset(new_addr_table, 0, sizeof(addrpair_t *) * new_addr_table_size);
        memcpy(new_addr_table, addr_table, 
               sizeof(addrpair_t *) * addr_table_size);
        Z_Free(addr_table);
        addr_table = new_addr_table;
        addr_table_size = new_addr_table_size;
    }

    // Add a new entry

    new_entry = Z_Malloc(sizeof(addrpair_t), PU_STATIC, 0);

    new_entry->protocol = protocol;
    new_entry->sdl_addr = *addr;
    new_entry->net_addr.refcount = 0;
    new_entry->net_addr.handle = new_entry;
    new_entry->tcp_sock = NULL;

    switch (protocol)
    {
        case IP_PROTOCOL_UDP:
            new_entry->net_addr.module = &net_udp_module;
            break;
        case IP_PROTOCOL_TCP:
            new_entry->net_addr.module = &net_tcp_module;
            break;
    }

    addr_table[empty_entry] = new_entry;

    return &new_entry->net_addr;
}

static net_addr_t *ResolveAddress(ip_protocol_t protocol, const char *address)
{
    IPaddress ip;
    char *addr_hostname;
    int addr_port;
    int result;
    char *colon;

    colon = strchr(address, ':');

    if (colon != NULL)
    {
	addr_hostname = M_StringDuplicate(address);
	addr_hostname[colon - address] = '\0';
	addr_port = atoi(colon + 1);
    }
    else
    {
	addr_hostname = M_StringDuplicate(address);
	addr_port = port;
    }

    result = SDLNet_ResolveHost(&ip, addr_hostname, addr_port);
    free(addr_hostname);

    if (result)
    {
        // unable to resolve

        return NULL;
    }

    return FindAddress(protocol, &ip);
}

static void NET_SDL_AddrToString(net_addr_t *addr, char *buffer,
                                 int buffer_len)
{
    IPaddress *ip;
    uint32_t host;
    uint16_t port;

    ip = &((addrpair_t *) addr->handle)->sdl_addr;
    host = SDLNet_Read32(&ip->host);
    port = SDLNet_Read16(&ip->port);

    X_snprintf(buffer, buffer_len, "%i.%i.%i.%i",
               (host >> 24) & 0xff, (host >> 16) & 0xff,
               (host >> 8) & 0xff, host & 0xff);

    // If we are using the default port we just need to show the IP address,
    // but otherwise we need to include the port. This is important because
    // we use the string representation in the setup tool to provided an
    // address to connect to.
    if (port != DEFAULT_PORT)
    {
        char portbuf[10];
        X_snprintf(portbuf, sizeof(portbuf), ":%i", port);
        M_StringConcat(buffer, portbuf, buffer_len);
    }
}

static void NET_SDL_FreeAddress(net_addr_t *addr)
{
    addrpair_t *ap;
    int i;

    ap = addr->handle;

    for (i = 0; i < addr_table_size; ++i)
    {
        if (addr_table[i] == ap)
        {
            addr_table[i] = NULL;

            // Freeing an address associated with an open TCP socket
            // implicitly closes that socket.
            if (ap->tcp_sock != NULL && ap->tcp_sock != TCPSOCKET_CLOSED)
            {
                SDLNet_TCP_Close(ap->tcp_sock);
            }
            Z_Free(ap);
            return;
        }
    }

    I_Error("NET_SDL_FreeAddress: Attempted to remove an unused address!");
}

static void InitCommonParams(void)
{
    int p;

    //!
    // @category net
    // @arg <n>
    //
    // Use the specified UDP port for communications, instead of 
    // the default (2342).
    //

    p = M_CheckParmWithArgs("-port", 1);
    if (p > 0)
        port = atoi(myargv[p+1]);

    SDLNet_Init();
    active_sockets = SDLNet_AllocSocketSet(MAXNETNODES);
    if (active_sockets == NULL)
    {
        I_Error("Failed to create socket set: %s", SDLNet_GetError());
    }
}

static boolean NET_UDP_InitClient(void)
{
    if (udp_initted)
        return true;

    InitCommonParams();

    udpsocket = SDLNet_UDP_Open(0);

    if (udpsocket == NULL)
    {
        I_Error("NET_UDP_InitClient: Unable to open a socket: %s",
                SDLNet_GetError());
    }

    recvpacket = SDLNet_AllocPacket(MAX_FRAME_LEN);

#ifdef DROP_PACKETS
    srand(time(NULL));
#endif

    udp_initted = true;

    return true;
}

static boolean NET_UDP_InitServer(void)
{
    if (udp_initted)
        return true;

    InitCommonParams();

    udpsocket = SDLNet_UDP_Open(port);

    if (udpsocket == NULL)
    {
        I_Error("NET_UDP_InitServer: Unable to bind to port %i", port);
    }

    recvpacket = SDLNet_AllocPacket(1500);
#ifdef DROP_PACKETS
    srand(time(NULL));
#endif

    udp_initted = true;

    return true;
}

static void NET_UDP_SendPacket(net_addr_t *addr, net_packet_t *packet)
{
    UDPpacket sdl_packet;
    IPaddress ip;

    if (addr == &net_broadcast_addr)
    {
        SDLNet_ResolveHost(&ip, NULL, port);
        ip.host = INADDR_BROADCAST;
    }
    else
    {
        ip = ((addrpair_t *) addr->handle)->sdl_addr;
    }

#if 0
    {
        static int this_second_sent = 0;
        static int lasttime;

        this_second_sent += packet->len + 64;

        if (I_GetTime() - lasttime > TICRATE)
        {
            printf("%i bytes sent in the last second\n", this_second_sent);
            lasttime = I_GetTime();
            this_second_sent = 0;
        }
    }
#endif

#ifdef DROP_PACKETS
    if ((rand() % 4) == 0)
        return;
#endif

    sdl_packet.channel = 0;
    sdl_packet.data = packet->data;
    sdl_packet.len = packet->len;
    sdl_packet.address = ip;

    if (SDLNet_UDP_Send(udpsocket, -1, &sdl_packet) <= 0)
    {
        I_Error("NET_UDP_SendPacket: Error transmitting packet: %s",
                SDLNet_GetError());
    }
}

static boolean NET_UDP_RecvPacket(net_addr_t **addr, net_packet_t **packet)
{
    int result;

    result = SDLNet_UDP_Recv(udpsocket, recvpacket);

    if (result < 0)
    {
        I_Error("NET_UDP_RecvPacket: Error receiving packet: %s",
                SDLNet_GetError());
    }

    // no packets received

    if (result == 0)
        return false;

    // Put the data into a new packet structure

    *packet = NET_NewPacket(recvpacket->len);
    memcpy((*packet)->data, recvpacket->data, recvpacket->len);
    (*packet)->len = recvpacket->len;

    // Address

    *addr = FindAddress(IP_PROTOCOL_UDP, &recvpacket->address);

    return true;
}

static net_addr_t *NET_UDP_ResolveAddress(const char *address)
{
    return ResolveAddress(IP_PROTOCOL_UDP, address);
}

static net_addr_t *NET_TCP_ResolveAddress(const char *address)
{
    return ResolveAddress(IP_PROTOCOL_TCP, address);
}

//
// TCP module implementation
//
// The following code sends "packets" over a TCP connection. The framing
// is the same framing format used by sersetup.exe, so that it can be
// used in conjunction with net_vanilla.c to play games against vanilla
// Doom.
//

static boolean NET_TCP_InitClient(void)
{
    if (tcp_initted)
        return true;

    InitCommonParams();
    tcp_initted = true;

    // In client mode, we do not open any socket; rather, trying to send
    // to a new address implicitly opens a TCP connection to it.

    return true;
}

static boolean NET_TCP_InitServer(void)
{
    IPaddress addr;

    if (tcp_initted)
        return true;

    InitCommonParams();

    SDLNet_ResolveHost(&addr, NULL, port);
    tcpsocket = SDLNet_TCP_Open(&addr);
    if (tcpsocket == NULL)
    {
        I_Error("NET_TCP_InitServer: Unable to bind to port %i", port);
    }

    tcp_initted = true;
    SDLNet_TCP_AddSocket(active_sockets, tcpsocket);

    return true;
}

static void OpenClientConnection(net_addr_t *addr)
{
    addrpair_t *ap = addr->handle;
    TCPsocket sock;
    char buf[32];

    sock = SDLNet_TCP_Open(&ap->sdl_addr);
    if (sock == NULL)
    {
        NET_SDL_AddrToString(addr, buf, sizeof(buf));
        I_Error("NET_TCP_SendPacket: Failed to connect to server %s: "
                "%s", buf, SDLNet_GetError());
    }

    SDLNet_TCP_AddSocket(active_sockets, sock);
    ap->tcp_sock = sock;
}

// "Escape" packet into framing format used by sersetup.exe. Packet data
// is sent as normal, but FRAMECHAR+0 indicates end-of-packet.
// FRAMECHAR+FRAMECHAR indicates escaped FRAMECHAR.
static net_packet_t *EscapePacket(net_packet_t *packet)
{
    net_packet_t *result = NET_NewPacket(packet->len + 10);
    unsigned int b;

    NET_SetPosition(packet, 0);

    while (NET_ReadInt8(packet, &b))
    {
        NET_WriteInt8(result, b);
        if (b == FRAMECHAR)
        {
            NET_WriteInt8(result, FRAMECHAR);
        }
    }

    NET_WriteInt8(result, FRAMECHAR);
    NET_WriteInt8(result, 0);

    return result;
}

// When a TCP connection is closed, we close it and replace the socket
// with the TCPSOCKET_CLOSED tombstone marker. It remains in the address
// table and the higher-level code can try to send packets to it, but
// they will just be blackholed.
static void LostConnection(addrpair_t *ap)
{
    char buf[32];
    NET_SDL_AddrToString(&ap->net_addr, buf, sizeof(buf));
    printf("Lost TCP connection to %s.\n", buf);
    SDLNet_TCP_DelSocket(active_sockets, ap->tcp_sock);
    SDLNet_TCP_Close(ap->tcp_sock);
    ap->tcp_sock = TCPSOCKET_CLOSED;
}

static void NET_TCP_SendPacket(net_addr_t *addr, net_packet_t *packet)
{
    addrpair_t *ap = addr->handle;
    net_packet_t *escaped;

    if (ap->tcp_sock == NULL)
    {
        if (tcpsocket != NULL)
        {
            I_Error("NET_TCP_SendPacket: Tried to send to address without "
                    "an open socket");
        }

        OpenClientConnection(addr);
    }

    // If we try to send to an address for which we've lost the TCP
    // connection, the packet just gets discarded.
    if (ap->tcp_sock == TCPSOCKET_CLOSED)
    {
        return;
    }

    escaped = EscapePacket(packet);
    if (SDLNet_TCP_Send(ap->tcp_sock,
                        escaped->data, escaped->len) < escaped->len)
    {
        LostConnection(ap);
    }
    NET_FreePacket(escaped);
}

static void AcceptNewConnection(void)
{
    TCPsocket sock;
    addrpair_t *ap;
    net_addr_t *addr;

    sock = SDLNet_TCP_Accept(tcpsocket);
    if (sock == NULL)
    {
        I_Error("NET_TCP_RecvPacket: Failed to accept new connection: %s",
                SDLNet_GetError());
    }

    SDLNet_TCP_AddSocket(active_sockets, sock);

    addr = FindAddress(IP_PROTOCOL_TCP, SDLNet_TCP_GetPeerAddress(sock));
    ap = addr->handle;
    ap->tcp_sock = sock;
}

// Read a packet from the given TCP socket, using the sersetup.exe
// framing format. A new net_packet_t is returned; NULL indicates that
// an error occurred and the socket should be closed. Note that this
// function has the potential to block forever if a full packet is not
// received.
static net_packet_t *ReadPacketFrom(TCPsocket sock)
{
    net_packet_t *packet;
    byte b;

    packet = NET_NewPacket(32);

    for (;;)
    {
        if (SDLNet_TCP_Recv(sock, &b, 1) < 1)
        {
            NET_FreePacket(packet);
            return NULL;
        }
        if (b == FRAMECHAR)
        {
            if (SDLNet_TCP_Recv(sock, &b, 1) < 1)
            {
                NET_FreePacket(packet);
                return NULL;
            }
            if (b != FRAMECHAR)
            {
                break;
            }
        }
        NET_WriteInt8(packet, b);
    }

    NET_SetPosition(packet, 0);
    return packet;
}

static boolean NET_TCP_RecvPacket(net_addr_t **addr, net_packet_t **packet)
{
    int result;
    int i;

    result = SDLNet_CheckSockets(active_sockets, 0);
    if (result <= 0)
    {
        return false;
    }

    if (tcpsocket != NULL && SDLNet_SocketReady(tcpsocket))
    {
        AcceptNewConnection();
    }

    // Find the first open socket we can read from.
    for (i = 0; i < addr_table_size; ++i)
    {
        if (addr_table[i] != NULL
         && addr_table[i]->tcp_sock != NULL
         && addr_table[i]->tcp_sock != TCPSOCKET_CLOSED
         && SDLNet_SocketReady(addr_table[i]->tcp_sock))
        {
            break;
        }
    }
    if (i >= addr_table_size)
    {
        return false;
    }

    // Read a full packet from the TCP socket. If we fail, this is because
    // the TCP connection was closed.
    *packet = ReadPacketFrom(addr_table[i]->tcp_sock);
    if (*packet == NULL)
    {
        LostConnection(addr_table[i]);
        return false;
    }

    *addr = &addr_table[i]->net_addr;
    return true;
}

net_module_t net_udp_module =
{
    NET_UDP_InitClient,
    NET_UDP_InitServer,
    NET_UDP_SendPacket,
    NET_UDP_RecvPacket,
    NET_SDL_AddrToString,
    NET_SDL_FreeAddress,
    NET_UDP_ResolveAddress,
};

net_module_t net_tcp_module =
{
    NET_TCP_InitClient,
    NET_TCP_InitServer,
    NET_TCP_SendPacket,
    NET_TCP_RecvPacket,
    NET_SDL_AddrToString,
    NET_SDL_FreeAddress,
    NET_TCP_ResolveAddress,
};


#else // DISABLE_SDL2NET

// no-op implementation


static boolean NET_NULL_InitClient(void)
{
    return false;
}


static boolean NET_NULL_InitServer(void)
{
    return false;
}


static void NET_NULL_SendPacket(net_addr_t *addr, net_packet_t *packet)
{
}


static boolean NET_NULL_RecvPacket(net_addr_t **addr, net_packet_t **packet)
{
    return false;
}


static void NET_NULL_AddrToString(net_addr_t *addr, char *buffer, int buffer_len)
{

}


static void NET_NULL_FreeAddress(net_addr_t *addr)
{
}


net_addr_t *NET_NULL_ResolveAddress(const char *address)
{
    return NULL;
}


net_module_t net_sdl_module =
{
    NET_NULL_InitClient,
    NET_NULL_InitServer,
    NET_NULL_SendPacket,
    NET_NULL_RecvPacket,
    NET_NULL_AddrToString,
    NET_NULL_FreeAddress,
    NET_NULL_ResolveAddress,
};


#endif // DISABLE_SDL2NET
