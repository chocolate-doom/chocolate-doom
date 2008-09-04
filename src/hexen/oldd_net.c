// I_pcnet.m

#include "DoomDef.h"

#define	NCMD_EXIT		0x80000000
#define	NCMD_RETRANSMIT	0x40000000
#define	NCMD_SETUP		0x20000000
#define	NCMD_CHECKSUM	0x0fffffff

/*
if more space needs to be crunched out of the protocol...

1	drone
2	player
8	tic
5	numtics

#define	NCMD_EXIT		0x80000000
#define	NCMD_RETRANSMIT	0x40000000			// a retransmit will have 0 tics
#define	NCMD_DRONE		0x20000000
#define	NCMD_PLAYER		0x18000000
#define	NCMD_PLAYERSHIFT	27
#define	NCMD_TIC		0x00ff0000
#define	NCMD_TICSHIFT	16
#define	NCMD_NUMTICS	0x0000ff00
#define	NCMD_NUMTICSSHIFT	8
#define	NCMD_CHECKSUM	0x000000ff

*/





doomcom_t		*doomcom;	
doomdata_t		*netbuffer;		// points inside doomcom


/*
==============================================================================

							NETWORKING

gametic is the tic about to (or currently being) run
maketic is the tick that hasn't had control made for it yet
nettics[] has the maketics for all players 

a gametic cannot be run until nettics[] > gametic for all players

==============================================================================
*/

#define	RESENDCOUNT	10
#define	PL_DRONE	0x80				// bit flag in doomdata->player

ticcmd_t		localcmds[BACKUPTICS];

ticcmd_t        netcmds[MAXPLAYERS][BACKUPTICS];
int         	nettics[MAXNETNODES];
boolean			nodeingame[MAXNETNODES];	// set false as nodes leave game
boolean			remoteresend[MAXNETNODES];	// set when local needs tics
int				resendto[MAXNETNODES];			// set when remote needs tics
int				resendcount[MAXNETNODES];

int				nodeforplayer[MAXPLAYERS];

int             gametime;
int             maketic;
int				lastnettic, skiptics;
int				ticdup;		

void D_ProcessEvents (void);
void G_BuildTiccmd (ticcmd_t *cmd);
void D_DoAdvanceDemo (void);

boolean			reboundpacket;
doomdata_t		reboundstore;


int	NetbufferSize (void)
{
	return (int)&(((doomdata_t *)0)->cmds[netbuffer->numtics]); 
}

unsigned NetbufferChecksum (void)
{
	unsigned		c;
	int		i,l;

	c = 0x1234567;

#ifdef NeXT
	return 0;			// byte order problems
#endif

	l = (NetbufferSize () - (int)&(((doomdata_t *)0)->retransmitfrom))/4;
	for (i=0 ; i<l ; i++)
		c += ((unsigned *)&netbuffer->retransmitfrom)[i] * (i+1);

	return c & NCMD_CHECKSUM;
}

int ExpandTics (int low)
{
	int	delta;
	
	delta = low - (maketic&0xff);
	
	if (delta >= -64 && delta <= 64)
		return (maketic&~0xff) + low;
	if (delta > 64)
		return (maketic&~0xff) - 256 + low;
	if (delta < -64)
		return (maketic&~0xff) + 256 + low;
		
	I_Error ("ExpandTics: strange value %i at maketic %i",low,maketic);
	return 0;
}


//============================================================================


/*
==============
=
= HSendPacket
=
==============
*/

void HSendPacket (int node, int flags)
{
	netbuffer->checksum = NetbufferChecksum () | flags;

	if (!node)
	{
		reboundstore = *netbuffer;
		reboundpacket = true;
		return;
	}

	if (!netgame)
		I_Error ("Tried to transmit to another node");
		
	doomcom->command = CMD_SEND;
	doomcom->remotenode = node;
	doomcom->datalength = NetbufferSize ();
	
if (debugfile)
{
	int		i;
	int		realretrans;
	if (netbuffer->checksum & NCMD_RETRANSMIT)
		realretrans = ExpandTics (netbuffer->retransmitfrom);
	else
		realretrans = -1;
	fprintf (debugfile,"send (%i + %i, R %i) [%i] "
	,ExpandTics(netbuffer->starttic),netbuffer->numtics, realretrans, doomcom->datalength);
	for (i=0 ; i<doomcom->datalength ; i++)
		fprintf (debugfile,"%i ",((byte *)netbuffer)[i]);
	fprintf (debugfile,"\n");
}

	I_NetCmd ();
}

/*
==============
=
= HGetPacket
=
= Returns false if no packet is waiting
=
==============
*/

boolean HGetPacket (void)
{	
	if (reboundpacket)
	{
		*netbuffer = reboundstore;
		doomcom->remotenode = 0;
		reboundpacket = false;
		return true;
	}

	if (!netgame)
		return false;
		
	doomcom->command = CMD_GET;
	I_NetCmd ();
	if (doomcom->remotenode == -1)
		return false;

	if (doomcom->datalength != NetbufferSize ())
	{
		if (debugfile)
			fprintf (debugfile,"bad packet length %i\n",doomcom->datalength);
		return false;
	}
	
	if (NetbufferChecksum () != (netbuffer->checksum&NCMD_CHECKSUM) )
	{
		if (debugfile)
			fprintf (debugfile,"bad packet checksum\n");
		return false;
	}

if (debugfile)
{
	int		realretrans;
			int	i;
			
	if (netbuffer->checksum & NCMD_SETUP)
		fprintf (debugfile,"setup packet\n");
	else
	{
		if (netbuffer->checksum & NCMD_RETRANSMIT)
			realretrans = ExpandTics (netbuffer->retransmitfrom);
		else
			realretrans = -1;
		fprintf (debugfile,"get %i = (%i + %i, R %i)[%i] ",doomcom->remotenode,
		ExpandTics(netbuffer->starttic),netbuffer->numtics, realretrans, doomcom->datalength);
		for (i=0 ; i<doomcom->datalength ; i++)
			fprintf (debugfile,"%i ",((byte *)netbuffer)[i]);
		fprintf (debugfile,"\n");
	}
}
	return true;	
}


/*
===================
=
= GetPackets
=
===================
*/

char    exitmsg[80];

void GetPackets (void)
{
	int		netconsole;
	int		netnode;
	int		netdrone;
	int		j;
	ticcmd_t	*src, *dest;
	int		dupedstart, dupedend;
	int		skiptics;
	int		realstart;
				 
	while (HGetPacket ())
	{
		if (netbuffer->checksum & NCMD_SETUP)
			continue;		// extra setup packet
			
		netdrone = netbuffer->player & PL_DRONE;
		netconsole = netbuffer->player & ~PL_DRONE;
		netnode = doomcom->remotenode;
		//
		// to save bytes, only the low byte of tic numbers are sent
		// Figure out what the rest of the bytes are
		//
		realstart = ExpandTics (netbuffer->starttic);		
		dupedstart = realstart*doomcom->ticdup;
		dupedend = (realstart+netbuffer->numtics)*doomcom->ticdup;
		
		//
		// check for exiting the game
		//
		if (netbuffer->checksum & NCMD_EXIT)
		{
			if (!nodeingame[netnode])
				continue;
			nodeingame[netnode] = false;
			if (!netdrone)
			{
				playeringame[netconsole] = false;
				strcpy (exitmsg, "Player 1 left the game");
				exitmsg[7] += netconsole;
				players[consoleplayer].message = exitmsg;
			}
			continue;
		}

		//
		// drone packets are just notifications
		//
		if (netdrone)
		{
			nettics[netnode] = dupedend;
			continue;
		}

		nodeforplayer[netconsole] = netnode;
		
		//
		// check for retransmit request
		//
		if ( resendcount[netnode] <= 0 
		&& (netbuffer->checksum & NCMD_RETRANSMIT) )
		{
			resendto[netnode] = ExpandTics(netbuffer->retransmitfrom);
if (debugfile)
fprintf (debugfile,"retransmit from %i\n", resendto[netnode]);
			resendcount[netnode] = RESENDCOUNT;
		}
		else
			resendcount[netnode]--;

		//
		// check for out of order / duplicated packet
		//		
		if (dupedend == nettics[netnode])
			continue;
			
		if (dupedend < nettics[netnode])
		{
if (debugfile)
fprintf (debugfile,"out of order packet (%i + %i)\n" ,realstart,netbuffer->numtics);
			continue;
		}

		//
		// check for a missed packet
		//
		if (dupedstart > nettics[netnode])
		{
		// stop processing until the other system resends the missed tics
if (debugfile)
fprintf (debugfile,"missed tics from %i (%i - %i)\n", netnode, dupedstart, nettics[netnode]);
			remoteresend[netnode] = true;
			continue;
		}
	
//
// update command store from the packet
//
		remoteresend[netnode] = false;
		
		skiptics = nettics[netnode]/doomcom->ticdup - realstart;		
		src = &netbuffer->cmds[skiptics];

		while (nettics[netnode] < dupedend)
		{
			for (j=0 ; j<doomcom->ticdup ; j++)
			{
				dest = &netcmds[netconsole][nettics[netnode]%BACKUPTICS];
				nettics[netnode]++;
				*dest = *src;
				src->chatchar = 0;
				if (src->buttons & BT_SPECIAL)
					src->buttons = 0;
			}
			src++;
		}
	}
}

/*
=============
=
= NetUpdate
=
= Builds ticcmds for console player
= sends out a packet
=============
*/

void NetUpdate (void)
{
	int             nowtime;
	int             newtics;
	int				i,j;
	int				gameticdiv;
	int				realstart;
		
	if (singletics)
		return;         // singletic update is syncronous
		
//
// check time
//      
	nowtime = I_GetTime ()/doomcom->ticdup;
	newtics = nowtime - gametime;
	gametime = nowtime;
	if (newtics <= 0)                       // nothing new to update
		goto listen; 

	if (skiptics <= newtics)
	{
		newtics -= skiptics;
		skiptics = 0;
	}
	else
	{
		skiptics -= newtics;
		newtics = 0;
	}
	
		
	netbuffer->player = consoleplayer;
	if (doomcom->drone)
		netbuffer->player |= PL_DRONE;
	
//
// drone packets
//
	if (doomcom->drone)
	{
		I_StartTic ();
		D_ProcessEvents ();
		goto sendit;
	}
	
//
// build new ticcmds for console player
//
	gameticdiv = (gametic+doomcom->ticdup-1)/doomcom->ticdup;
	for (i=0 ; i<newtics ; i++)
	{
		I_StartTic ();
		D_ProcessEvents ();
		if (maketic - gameticdiv >= BACKUPTICS/2 /* /doomcom->ticdup */- 1)
		{
			newtics = i;
			break;          // can't hold any more
		}
//printf ("mk:%i ",maketic);
		G_BuildTiccmd (&localcmds[maketic%BACKUPTICS]);
		maketic++;
	}

//
// send the packet to the other nodes
//
sendit:
	for (i=0 ; i<doomcom->numnodes ; i++)
		if (nodeingame[i])
		{
			if (doomcom->drone)
			{
				netbuffer->starttic = realstart = maketic + BACKUPTICS/2;
				netbuffer->numtics = 0;
			}
			else
			{
				netbuffer->starttic = realstart = resendto[i];
				netbuffer->numtics = maketic - realstart;
				resendto[i] = maketic - doomcom->extratics;
			}
	
			if (netbuffer->numtics > BACKUPTICS)
				I_Error ("NetUpdate: netbuffer->numtics > BACKUPTICS");

			for (j=0 ; j< netbuffer->numtics ; j++)
				netbuffer->cmds[j] = 
					localcmds[(realstart+j)%BACKUPTICS];
					
			if (remoteresend[i])
			{
				netbuffer->retransmitfrom = nettics[i]/doomcom->ticdup;
				HSendPacket (i, NCMD_RETRANSMIT);
			}
			else
			{
				netbuffer->retransmitfrom = 0;
				HSendPacket (i, 0);
			}
		}

//
// listen for other packets
//		
listen:

	GetPackets ();
}


/*
=====================
=
= CheckAbort
=
=====================
*/

void CheckAbort (void)
{
	event_t *ev;
	
	I_WaitVBL(2);
	
	I_StartTic ();
	for ( ; eventtail != eventhead 
	; eventtail = (++eventtail)&(MAXEVENTS-1) )
	{
		ev = &events[eventtail];
		if (ev->type == ev_keydown && ev->data1 == KEY_ESCAPE)
			I_Error ("Network game synchronization aborted.");
	}
}

/*
=====================
=
= D_ArbitrateNetStart
=
=====================
*/

void D_ArbitrateNetStart (void)
{
	int		i;
	boolean	gotinfo[MAXNETNODES];
	
	autostart = true;
	memset (gotinfo,0,sizeof(gotinfo));
	
	if (doomcom->consoleplayer)
	{	// listen for setup info from key player
		printf ("listening for network start info...\n");
		while (1)
		{
			CheckAbort ();
			if (!HGetPacket ())
				continue;
			if (netbuffer->checksum & NCMD_SETUP)
			{
				if (netbuffer->player != VERSION)
					I_Error ("Different DOOM versions cannot play a net game!");
				startskill = netbuffer->retransmitfrom & 15;
				deathmatch = (netbuffer->retransmitfrom & 0x80) > 0;
				nomonsters = (netbuffer->retransmitfrom & 0x40) > 0;
				respawnparm = (netbuffer->retransmitfrom & 0x20) > 0;
				startmap = netbuffer->starttic & 15;
				startepisode = netbuffer->starttic >> 4;
				return;
			}
		}
	}
	else
	{	// key player, send the setup info
		printf ("sending network start info...\n");
		do
		{
			CheckAbort ();
			for (i=0 ; i<doomcom->numnodes ; i++)
			{
				netbuffer->retransmitfrom = startskill;
				if (deathmatch)
					netbuffer->retransmitfrom |= 0x80;
				if (nomonsters)
					netbuffer->retransmitfrom |= 0x40;
				if (respawnparm)
					netbuffer->retransmitfrom |= 0x20;
				netbuffer->starttic = startepisode * 16 + startmap;
				netbuffer->player = VERSION;
				netbuffer->numtics = 0;
				HSendPacket (i, NCMD_SETUP);
			}
	
			while (HGetPacket ())
			{
				gotinfo[netbuffer->player&0x7f] = true;
			}

			for (i=1 ; i<doomcom->numnodes ; i++)
				if (!gotinfo[i])
					break;
		} while (i < doomcom->numnodes);
	}
}

/*
===================
=
= D_CheckNetGame
=
= Works out player numbers among the net participants
===================
*/

extern	int			viewangleoffset;

void D_CheckNetGame (void)
{
	int             i;
	
	for (i=0 ; i<MAXNETNODES ; i++)
	{
		nodeingame[i] = false;
       	nettics[i] = 0;
		remoteresend[i] = false;	// set when local needs tics
		resendto[i] = 0;			// which tic to start sending
	}
	
// I_InitNetwork sets doomcom and netgame
	I_InitNetwork ();
	if (doomcom->id != DOOMCOM_ID)
		I_Error ("Doomcom buffer invalid!");
	netbuffer = &doomcom->data;
	consoleplayer = displayplayer = doomcom->consoleplayer;
	if (netgame)
		D_ArbitrateNetStart ();
printf ("startskill %i  deathmatch: %i  startmap: %i  startepisode: %i\n", startskill, deathmatch, startmap, startepisode);
	
// read values out of doomcom
	ticdup = doomcom->ticdup;
					
	for (i=0 ; i<doomcom->numplayers ; i++)
		playeringame[i] = true;
	for (i=0 ; i<doomcom->numnodes ; i++)
		nodeingame[i] = true;
	
printf ("player %i of %i (%i nodes)\n", consoleplayer+1, doomcom->numplayers, doomcom->numnodes);

}

/*
==================
=
= D_QuitNetGame
=
= Called before quitting to leave a net game without hanging the
= other players
=
==================
*/

void D_QuitNetGame (void)
{
	int             i, j;
	
	if (debugfile)
		fclose (debugfile);
		
	if (!netgame || !usergame || consoleplayer == -1)
		return;
	
// send a bunch of packets for security
	netbuffer->player = consoleplayer;
	if (doomcom->drone)
		netbuffer->player |= PL_DRONE;
	netbuffer->numtics = 0;
	for (i=0 ; i<4 ; i++)
	{
		for (j=1 ; j<doomcom->numnodes ; j++)
			if (nodeingame[j])
				HSendPacket (j, NCMD_EXIT);
		I_WaitVBL (1);
	}
}



/*
===============
=
= TryRunTics
=
===============
*/

int	frametics[4], frameon;
int	frameskip[4];
int		oldnettics;
extern	boolean	advancedemo;

void TryRunTics (void)
{
	int             i;
	int             lowtic, nextlowest;
	int             entertic;
	int	static		oldentertics;
	int				realtics, availabletics;
	int				counts;
	int				numplaying;

//
// get real tics
//			
	entertic = I_GetTime ();
	realtics = entertic - oldentertics;
	oldentertics = entertic;

//
// get available tics
//
	NetUpdate ();
	
	lowtic = nextlowest = MAXINT;
	numplaying = 0;
	for (i=0 ; i<doomcom->numnodes ; i++)
		if (nodeingame[i])
		{
			numplaying++;
			if (nettics[i] < lowtic)
			{
				nextlowest = lowtic;
				lowtic = nettics[i];
			}
			else if (nettics[i] < nextlowest)
				nextlowest = nettics[i]; 
		}
	availabletics = lowtic - gametic;
	

//
// decide how many tics to run
//
	if (realtics < availabletics-1)
		counts = realtics+1;
	else if (realtics < availabletics)
		counts = realtics;
	else
		counts = availabletics;
	if (counts < 1)
		counts = 1;
		
	frameon++;
	
if (debugfile)
	fprintf (debugfile,"=======real: %i  avail: %i  game: %i\n",realtics, availabletics,counts);

//=============================================================================
//
//	ideally nettics[0] should be 1 - 3 tics above lowtic
//	if we are consistantly slower, speed up time
//	drones should never hold up the other players
//
	for (i=0 ; i<MAXPLAYERS ; i++)
		if (playeringame[i])
			break;
	if (consoleplayer == i)
	{	// the key player does not adapt
	}
	else
	{
		if (nettics[0] <= nettics[nodeforplayer[i]])
		{
			gametime--;
//			printf ("-");
		}
		frameskip[frameon&3] = (oldnettics > nettics[nodeforplayer[i]]);
		oldnettics = nettics[0];
		if (frameskip[0] && frameskip[1] && frameskip[2] && frameskip[3])
		{
			skiptics = 1;
//			printf ("+");
		}
	}
//=============================================================================

//
// wait for new tics if needed
//
	while (lowtic < gametic + counts)	
	{

		NetUpdate ();   
		lowtic = MAXINT;
		
		for (i=0 ; i<doomcom->numnodes ; i++)
			if (nodeingame[i] && nettics[i] < lowtic)
				lowtic = nettics[i];

		if (lowtic < gametic)
			I_Error ("TryRunTics: lowtic < gametic");
			
		// don't stay in here forever -- give the menu a chance to work
		if (I_GetTime () - entertic >= 20)
			return;
	}
			

//
// run the tics
//	
	while (counts--)
	{
		G_Ticker ();
		NetUpdate ();					// check for new console commands
		gametic++;
	}
}
