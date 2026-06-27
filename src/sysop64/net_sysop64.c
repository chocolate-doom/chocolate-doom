//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2005-2014 Simon Howard
// Copyright(C) 2026 Sysop-64 contributors
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
//     Sysop-64 network transport shim for Chocolate Doom's netcode.
//

#include "config.h"

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "doomtype.h"
#include "i_system.h"
#include "m_argv.h"
#include "m_misc.h"
#include "net_defs.h"
#include "net_io.h"
#include "net_packet.h"
#include "net_sdl.h"
#include "z_zone.h"

#define DEFAULT_PORT 2342
#define MAX_PACKET_SIZE 1500

static boolean initted = false;
static int port = DEFAULT_PORT;
static int udp_socket = -1;

typedef struct
{
    net_addr_t net_addr;
    struct sockaddr_in sockaddr;
} addrpair_t;

static addrpair_t **addr_table;
static int addr_table_size = -1;

// Allocate the address cache used to give Chocolate Doom stable net_addr_t
// objects for UDP peers.
static void InitAddrTable(void)
{
    addr_table_size = 16;
    addr_table = Z_Malloc(sizeof(addrpair_t *) * addr_table_size, PU_STATIC, 0);
    memset(addr_table, 0, sizeof(addrpair_t *) * addr_table_size);
}

// Compare IPv4 UDP endpoints by address and port.
static boolean AddressesEqual(const struct sockaddr_in *a, const struct sockaddr_in *b)
{
    return a->sin_addr.s_addr == b->sin_addr.s_addr && a->sin_port == b->sin_port;
}

// Find or create the Chocolate Doom net_addr_t wrapper for a UDP endpoint.
static net_addr_t *FindAddress(const struct sockaddr_in *addr)
{
    addrpair_t *new_entry;
    int empty_entry = -1;

    if (addr_table_size < 0) {
        InitAddrTable();
    }

    for (int i = 0; i < addr_table_size; ++i) {
        if (addr_table[i] != NULL && AddressesEqual(addr, &addr_table[i]->sockaddr)) {
            return &addr_table[i]->net_addr;
        }
        if (empty_entry < 0 && addr_table[i] == NULL) {
            empty_entry = i;
        }
    }

    if (empty_entry < 0) {
        int old_size = addr_table_size;
        addrpair_t **new_table;
        addr_table_size *= 2;
        new_table = Z_Malloc(sizeof(addrpair_t *) * addr_table_size, PU_STATIC, 0);
        memset(new_table, 0, sizeof(addrpair_t *) * addr_table_size);
        memcpy(new_table, addr_table, sizeof(addrpair_t *) * old_size);
        Z_Free(addr_table);
        addr_table = new_table;
        empty_entry = old_size;
    }

    new_entry = Z_Malloc(sizeof(addrpair_t), PU_STATIC, 0);
    memset(new_entry, 0, sizeof(*new_entry));
    new_entry->sockaddr = *addr;
    new_entry->net_addr.refcount = 0;
    new_entry->net_addr.handle = &new_entry->sockaddr;
    new_entry->net_addr.module = &net_sdl_module;
    addr_table[empty_entry] = new_entry;

    return &new_entry->net_addr;
}

// Release an address-cache entry when Chocolate Doom drops its last reference.
static void FreeAddress(net_addr_t *addr)
{
    for (int i = 0; i < addr_table_size; ++i) {
        if (addr_table[i] != NULL && addr == &addr_table[i]->net_addr) {
            Z_Free(addr_table[i]);
            addr_table[i] = NULL;
            return;
        }
    }

    I_Error("NET_SYSOP_FreeAddress: Attempted to remove an unused address!");
}

// Read the shared UDP port from Chocolate Doom's standard -port option.
static void ParsePort(void)
{
    int p = M_CheckParmWithArgs("-port", 1);
    if (p > 0) {
        port = atoi(myargv[p + 1]);
    }
}

// Create the nonblocking UDP socket used by both client and server modes.
static boolean OpenSocket(int bind_port)
{
    struct sockaddr_in bind_addr;
    int flags;
    int yes = 1;

    if (initted) {
        return true;
    }

    ParsePort();
    udp_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (udp_socket < 0) {
        I_Error("NET_SYSOP_OpenSocket: socket failed: %s", strerror(errno));
    }

    setsockopt(udp_socket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
    setsockopt(udp_socket, SOL_SOCKET, SO_BROADCAST, &yes, sizeof(yes));

    flags = fcntl(udp_socket, F_GETFL, 0);
    if (flags >= 0) {
        fcntl(udp_socket, F_SETFL, flags | O_NONBLOCK);
    }

    memset(&bind_addr, 0, sizeof(bind_addr));
    bind_addr.sin_family = AF_INET;
    bind_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    bind_addr.sin_port = htons((uint16_t)bind_port);

    if (bind(udp_socket, (struct sockaddr *)&bind_addr, sizeof(bind_addr)) < 0) {
        I_Error("NET_SYSOP_OpenSocket: bind port %d failed: %s", bind_port, strerror(errno));
    }

    initted = true;
    return true;
}

// Initialize the client transport with an ephemeral local UDP port.
static boolean InitClient(void)
{
    return OpenSocket(0);
}

// Initialize the server transport on the configured game port.
static boolean InitServer(void)
{
    ParsePort();
    return OpenSocket(port);
}

// Send one Chocolate Doom network packet to a resolved peer or broadcast.
static void SendPacket(net_addr_t *addr, net_packet_t *packet)
{
    struct sockaddr_in sockaddr;
    ssize_t result;

    if (addr == &net_broadcast_addr) {
        memset(&sockaddr, 0, sizeof(sockaddr));
        sockaddr.sin_family = AF_INET;
        sockaddr.sin_addr.s_addr = htonl(INADDR_BROADCAST);
        sockaddr.sin_port = htons((uint16_t)port);
    } else {
        sockaddr = *((struct sockaddr_in *)addr->handle);
    }

    result = sendto(udp_socket, packet->data, packet->len, 0,
                    (struct sockaddr *)&sockaddr, sizeof(sockaddr));
    if (result < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
        I_Error("NET_SYSOP_SendPacket: %s", strerror(errno));
    }
}

// Poll the nonblocking UDP socket and wrap received bytes in a net_packet_t.
static boolean RecvPacket(net_addr_t **addr, net_packet_t **packet)
{
    byte buffer[MAX_PACKET_SIZE];
    struct sockaddr_in from;
    socklen_t from_len = sizeof(from);
    ssize_t result;

    result = recvfrom(udp_socket, buffer, sizeof(buffer), 0,
                      (struct sockaddr *)&from, &from_len);
    if (result < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return false;
        }
        I_Error("NET_SYSOP_RecvPacket: %s", strerror(errno));
    }

    if (result == 0) {
        return false;
    }

    *packet = NET_NewPacket((int)result);
    memcpy((*packet)->data, buffer, (size_t)result);
    (*packet)->len = (size_t)result;
    *addr = FindAddress(&from);
    return true;
}

// Format a UDP peer address for status messages and diagnostics.
static void AddrToString(net_addr_t *addr, char *buffer, int buffer_len)
{
    struct sockaddr_in *sockaddr = (struct sockaddr_in *)addr->handle;
    char host[INET_ADDRSTRLEN];
    int addr_port;

    inet_ntop(AF_INET, &sockaddr->sin_addr, host, sizeof(host));
    addr_port = ntohs(sockaddr->sin_port);

    if (addr_port == DEFAULT_PORT) {
        M_snprintf(buffer, buffer_len, "%s", host);
    } else {
        M_snprintf(buffer, buffer_len, "%s:%d", host, addr_port);
    }
}

// Resolve host[:port] text into a cached UDP address object for the netcode.
static net_addr_t *ResolveAddress(const char *address)
{
    char *addr_hostname;
    char *colon;
    int addr_port;
    struct addrinfo hints;
    struct addrinfo *result = NULL;
    struct sockaddr_in sockaddr;
    char portbuf[16];

    ParsePort();
    addr_hostname = M_StringDuplicate(address);
    colon = strrchr(addr_hostname, ':');
    if (colon != NULL) {
        *colon = '\0';
        addr_port = atoi(colon + 1);
    } else {
        addr_port = port;
    }

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    M_snprintf(portbuf, sizeof(portbuf), "%d", addr_port);

    if (getaddrinfo(addr_hostname, portbuf, &hints, &result) != 0 || result == NULL) {
        free(addr_hostname);
        return NULL;
    }

    memcpy(&sockaddr, result->ai_addr, sizeof(sockaddr));
    freeaddrinfo(result);
    free(addr_hostname);
    return FindAddress(&sockaddr);
}

net_module_t net_sdl_module =
{
    InitClient,
    InitServer,
    SendPacket,
    RecvPacket,
    AddrToString,
    FreeAddress,
    ResolveAddress,
};
