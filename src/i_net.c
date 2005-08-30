// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: i_net.c 58 2005-08-30 22:15:11Z fraggle $
//
// Copyright(C) 1993-1996 Id Software, Inc.
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
// Revision 1.7  2005/08/30 22:15:11  fraggle
// More Windows fixes
//
// Revision 1.6  2005/08/30 22:11:10  fraggle
// Windows fixes
//
// Revision 1.5  2005/08/12 16:54:15  fraggle
// Port network code to use SDL_net
//
// Revision 1.4  2005/08/04 18:42:15  fraggle
// Silence compiler warnings
//
// Revision 1.3  2005/07/23 18:56:07  fraggle
// Remove unneccessary pragmas
//
// Revision 1.2  2005/07/23 16:44:55  fraggle
// Update copyright to GNU GPL
//
// Revision 1.1.1.1  2005/07/23 16:20:32  fraggle
// Initial import
//
//
// DESCRIPTION:
//
//-----------------------------------------------------------------------------

static const char
rcsid[] = "$Id: i_net.c 58 2005-08-30 22:15:11Z fraggle $";

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <SDL_net.h>

#include "i_system.h"
#include "d_event.h"
#include "d_net.h"
#include "m_argv.h"

#include "doomstat.h"

#include "i_net.h"





void	NetSend (void);
boolean NetListen (void);


//
// NETWORKING
//

int	DOOMPORT = 8626;

#ifndef NO_SDL_NET

static UDPsocket udpsocket;
static UDPpacket *packet;

static IPaddress        sendaddress[MAXNETNODES];

void	(*netget) (void);
void	(*netsend) (void);


unsigned short host_to_net16(unsigned int value)
{
    union 
    {
        unsigned short s;
        char b[2];
    } data;

    SDLNet_Write16(value, data.b);

    return data.s;
}

unsigned short net_to_host16(unsigned int value)
{
    unsigned short s = value;

    return SDLNet_Read16(&s);
}

unsigned long host_to_net32(unsigned int value)
{
    union
    {
        unsigned long l;
        char b[4];
    } data;

    SDLNet_Write32(value, data.b);

    return data.l;
}

unsigned long net_to_host32(unsigned int value)
{
    union
    {
        unsigned long l;
        char b[4];
    } data;

    data.l = value;

    return SDLNet_Read32(data.b);
}


//
// PacketSend
//
void PacketSend (void)
{
    int		c;
    doomdata_t	*sw;

    sw = (doomdata_t *) packet->data;
				
    // byte swap
    sw->checksum = host_to_net32(netbuffer->checksum);
    sw->player = netbuffer->player;
    sw->retransmitfrom = netbuffer->retransmitfrom;
    sw->starttic = netbuffer->starttic;
    sw->numtics = netbuffer->numtics;

    for (c=0 ; c< netbuffer->numtics ; c++)
    {
	sw->cmds[c].forwardmove = netbuffer->cmds[c].forwardmove;
	sw->cmds[c].sidemove = netbuffer->cmds[c].sidemove;
	sw->cmds[c].angleturn = host_to_net16(netbuffer->cmds[c].angleturn);
	sw->cmds[c].consistancy = host_to_net16(netbuffer->cmds[c].consistancy);
	sw->cmds[c].chatchar = netbuffer->cmds[c].chatchar;
	sw->cmds[c].buttons = netbuffer->cmds[c].buttons;
    }

    packet->len = doomcom->datalength;
    packet->address = sendaddress[doomcom->remotenode];

    if (!SDLNet_UDP_Send(udpsocket, -1, packet))
    {
	I_Error("Error sending packet: %s", SDLNet_GetError());
    }
}


//
// PacketGet
//
void PacketGet (void)
{
    int			i;
    int			c;
    doomdata_t	       *sw;
    int                 packets_read;
				
    packets_read = SDLNet_UDP_Recv(udpsocket, packet);

    if (packets_read < 0)
    {
	I_Error("Error reading packet: %s\n", SDLNet_GetError());
    }

    if (packets_read == 0)
    {
	doomcom->remotenode = -1;
	return;
    }

    // find remote node number
    for (i=0 ; i<doomcom->numnodes ; i++)
	if (packet->address.host == sendaddress[i].host
	 && packet->address.port == sendaddress[i].port)
	    break;

    if (i == doomcom->numnodes)
    {
	// packet is not from one of the players (new game broadcast)
	doomcom->remotenode = -1;		// no packet
	return;
    }
	
    doomcom->remotenode = i;		// good packet from a game player
    doomcom->datalength = packet->len;

    sw = (doomdata_t *) packet->data;
	
    // byte swap
    netbuffer->checksum = net_to_host32(sw->checksum);
    netbuffer->player = sw->player;
    netbuffer->retransmitfrom = sw->retransmitfrom;
    netbuffer->starttic = sw->starttic;
    netbuffer->numtics = sw->numtics;

    for (c=0 ; c< netbuffer->numtics ; c++)
    {
	netbuffer->cmds[c].forwardmove = sw->cmds[c].forwardmove;
	netbuffer->cmds[c].sidemove = sw->cmds[c].sidemove;
	netbuffer->cmds[c].angleturn = net_to_host16(sw->cmds[c].angleturn);
	netbuffer->cmds[c].consistancy = net_to_host16(sw->cmds[c].consistancy);
	netbuffer->cmds[c].chatchar = sw->cmds[c].chatchar;
	netbuffer->cmds[c].buttons = sw->cmds[c].buttons;
    }
}


//
// I_InitNetwork
//
void I_InitNetwork (void)
{
    int			i;
    int			p;
	
    doomcom = malloc (sizeof (*doomcom) );
    memset (doomcom, 0, sizeof(*doomcom) );
    
    // set up for network
    i = M_CheckParm ("-dup");
    if (i && i< myargc-1)
    {
	doomcom->ticdup = myargv[i+1][0]-'0';
	if (doomcom->ticdup < 1)
	    doomcom->ticdup = 1;
	if (doomcom->ticdup > 9)
	    doomcom->ticdup = 9;
    }
    else
	doomcom-> ticdup = 1;
	
    if (M_CheckParm ("-extratic"))
	doomcom-> extratics = 1;
    else
	doomcom-> extratics = 0;
		
    p = M_CheckParm ("-port");
    if (p && p<myargc-1)
    {
	DOOMPORT = atoi (myargv[p+1]);
	printf ("using alternate port %i\n",DOOMPORT);
    }
    
    // parse network game options,
    //  -net <consoleplayer> <host> <host> ...
    i = M_CheckParm ("-net");
    if (!i)
    {
	// single player game
	netgame = false;
	doomcom->id = DOOMCOM_ID;
	doomcom->numplayers = doomcom->numnodes = 1;
	doomcom->deathmatch = false;
	doomcom->consoleplayer = 0;
	return;
    }

    netsend = PacketSend;
    netget = PacketGet;
    netgame = true;

    // parse player number and host list
    doomcom->consoleplayer = myargv[i+1][0]-'1';

    doomcom->numnodes = 1;	// this node for sure

    SDLNet_Init();
	
    i++;
    while (++i < myargc && myargv[i][0] != '-')
    {
        if (SDLNet_ResolveHost(&sendaddress[doomcom->numnodes], 
                               myargv[i], DOOMPORT))
        {
            I_Error("Unable to resolve %s", myargv[i]);
        }
        
	doomcom->numnodes++;
    }
	
    doomcom->id = DOOMCOM_ID;
    doomcom->numplayers = doomcom->numnodes;
    
    // build message to receive

    udpsocket = SDLNet_UDP_Open(DOOMPORT);

    packet = SDLNet_AllocPacket(5000);
}


void I_NetCmd (void)
{
    if (doomcom->command == CMD_SEND)
    {
	netsend ();
    }
    else if (doomcom->command == CMD_GET)
    {
	netget ();
    }
    else
	I_Error ("Bad net cmd: %i\n",doomcom->command);
}

#else 

void I_NetCmd(void)
{
}


void I_InitNetwork (void)
{
    doomcom = malloc (sizeof (*doomcom) );
    memset (doomcom, 0, sizeof(*doomcom) );

    doomcom->ticdup = 1;
    doomcom->extratics = 0;

    
    // single player game
    netgame = false;
    doomcom->id = DOOMCOM_ID;
    doomcom->numplayers = doomcom->numnodes = 1;
    doomcom->deathmatch = false;
    doomcom->consoleplayer = 0;
    return;
}


#endif /* #ifndef NO_SDL_NET */


