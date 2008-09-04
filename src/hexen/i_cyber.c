// I_cyber.c

#include <dos.h>
#include <stdlib.h>
#include <string.h>
#include "st_start.h"	// For ST_Message()


// Prototypes
unsigned char *I_AllocLow (int length);


/*
====================================================

Doom control structure

The keybaord and joystick will add to the values set by the cyberman,
to a maximum of 0x19000 for forwardmove and sidemove.  Angleturn is
not bounded at all.

parm                    normal          fast
-----           ------          ----
forwardmove             0xc800          0x19000
sidemove                0xc000          0x14000
angleturn               0x2800000       0x5000000

The keyboard and joystick have a 1/3 second slow turn of 0x1400000 under
normal speed to help aiming.



====================================================
*/
/* old ticcmd_t
typedef struct
{
	char            forwardmove;            // *2048 for move
	char            sidemove;                       // *2048 for move
	short           angleturn;                      // <<16 for angle delta
	short           consistancy;            // checks for net game
	unsigned char            chatchar;
	unsigned char           buttons;
} ticcmd_t;
*/
// ticcmd_t as it appears in h2def.h
typedef struct
{
	char forwardmove;
	char sidemove;
	short angleturn;
	short consistancy;
	unsigned char chatchar;
	unsigned char buttons;
	unsigned char lookfly;
	unsigned char arti;
}ticcmd_t;


#define BT_ATTACK               1
#define BT_USE                  2
#define BT_CHANGE               4                       // if true, the next 3 bits hold weapon num
#define BT_WEAPONMASK   (8+16+32)
#define BT_WEAPONSHIFT  3

//==================================================
//
// CyberMan detection and usage info
//
//==================================================
#define DPMI_INT        0x31
#define MOUSE_INT       0x33

#define DOSMEMSIZE      64      // enough for any SWIFT structure

typedef struct {
   short        x;
   short        y;
   short        z;
   short        pitch;
   short        roll;
   short        yaw;
   short        buttons;
} SWIFT_3DStatus;

// DPMI real mode interrupt structure
static struct rminfo {
	long EDI;
	long ESI;
	long EBP;
	long reserved_by_system;
	long EBX;
	long EDX;
	long ECX;
	long EAX;
	short flags;
	short ES,DS,FS,GS,IP,CS,SP,SS;
} RMI;

typedef struct {
   unsigned char        deviceType;
   unsigned char        majorVersion;
   unsigned char        minorVersion;
   unsigned char        absRelFlags;
   unsigned char        centeringFlags;
   unsigned char        reserved[5];
} StaticDeviceData;

// values for deviceType:
#define DEVTYPE_CYBERMAN        1

short                   selector;
unsigned short  segment;                // segment of DOS memory block
SWIFT_3DStatus  *cyberstat;
int                             isCyberPresent;         // is CyberMan present?


static  union REGS regs;
static  struct SREGS sregs;


extern  int mousepresent;

//===========================================================
//
// I_StartupCyberMan
//
// If a cyberman is present, init it and set isCyberPresent to 1
//===========================================================
void I_StartupCyberMan(void)
{
   StaticDeviceData *pbuf;

   ST_Message("  CyberMan: ");
   isCyberPresent = 0;

   cyberstat = (SWIFT_3DStatus *)I_AllocLow (DOSMEMSIZE);
   segment = (int)cyberstat>>4;

   pbuf = (StaticDeviceData *)cyberstat;
   memset(pbuf, 0, sizeof (StaticDeviceData));


   // Use DPMI call 300h to issue mouse interrupt
   memset(&RMI, 0, sizeof(RMI));
   RMI.EAX = 0x53C1;            // SWIFT: Get Static Device Data
   RMI.ES = segment;
   RMI.EDX = 0;
   memset(&sregs, 0, sizeof (sregs));
   regs.w.ax = 0x0300;          // DPMI: simulate interrupt
   regs.w.bx = MOUSE_INT;
   regs.w.cx = 0;
   regs.x.edi = FP_OFF(&RMI);
   sregs.es = FP_SEG(&RMI);
   int386x( DPMI_INT, &regs, &regs, &sregs );

   if ((short)RMI.EAX != 1)
   {
	  // SWIFT functions not present
	  ST_Message("Wrong mouse driver - no SWIFT support (AX=%04x).\n",
			 (unsigned)(short)RMI.EAX);
   }
   else if (pbuf->deviceType != DEVTYPE_CYBERMAN)
   {
	  // no SWIFT device, or not CyberMan
	  if (pbuf->deviceType == 0)
	  {
		 ST_Message("no SWIFT device connected.\n");
	  }
	  else
	  {
		 ST_Message("SWIFT device is not a CyberMan! (type=%d)\n",
				pbuf->deviceType);
	  }
   }
   else
   {
	  ST_Message("CyberMan %d.%02d connected.\n",
			 pbuf->majorVersion, pbuf->minorVersion);
	  isCyberPresent = 1;
	  mousepresent = 0;
   }
}



/*
===============
=
= I_ReadCyberCmds
=
===============
*/


int             oldpos;

void I_ReadCyberCmd (ticcmd_t *cmd)
{
	int             delta;

	// Use DPMI call 300h to issue mouse interrupt
	memset(&RMI, 0, sizeof(RMI));
	RMI.EAX = 0x5301;            // SWIFT: Get Position and Buttons
	RMI.ES = segment;
	RMI.EDX = 0;
	memset(&sregs, 0, sizeof (sregs));
	regs.w.ax = 0x0300;          // DPMI: simulate interrupt
	regs.w.bx = MOUSE_INT;
	regs.w.cx = 0;
	regs.x.edi = FP_OFF(&RMI);
	sregs.es = FP_SEG(&RMI);
	int386x( DPMI_INT, &regs, &regs, &sregs );

	if (cyberstat->y < -7900)
		cmd->forwardmove = 0xc800/2048;
	else if (cyberstat->y > 7900)
		cmd->forwardmove = -0xc800/2048;

	if (cyberstat->buttons & 4)
		cmd->buttons |= BT_ATTACK;
	if (cyberstat->buttons & 2)
		cmd->buttons |= BT_USE;

	delta = cyberstat->x - oldpos;
	oldpos = cyberstat->x;

	if (cyberstat->buttons & 1)
	{       // strafe
		if (cyberstat->x < -7900)
			cmd->sidemove = -0xc800/2048;
		else if (cyberstat->x > 7900)
			cmd->sidemove = 0xc800/2048;
		else
			cmd->sidemove = delta*40/2048;
	}
	else
	{
		if (cyberstat->x < -7900)
			cmd->angleturn = 0x280;
		else if (cyberstat->x > 7900)
			cmd->angleturn = -0x280;
		else
			cmd->angleturn = -delta*0xa/16;

	}

}


void I_Tactile (int on, int off, int total)
{
	if (!isCyberPresent)
		return;

	on /= 5;
	off /= 5;
	total /= 40;
	if (on > 255)
		on = 255;
	if (off > 255)
		off = 255;
	if (total > 255)
		total = 255;

	memset(&RMI, 0, sizeof(RMI));
	RMI.EAX = 0x5330;            // SWIFT: Get Position and Buttons
	RMI.EBX = on*256+off;
	RMI.ECX = total;
	memset(&sregs, 0, sizeof (sregs));
	regs.w.ax = 0x0300;          // DPMI: simulate interrupt
	regs.w.bx = MOUSE_INT;
	regs.w.cx = 0;
	regs.x.edi = FP_OFF(&RMI);
	sregs.es = FP_SEG(&RMI);
	int386x( DPMI_INT, &regs, &regs, &sregs );
}
