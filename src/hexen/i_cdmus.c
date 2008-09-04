
//**************************************************************************
//**
//** i_cdmus.c
//**
//**************************************************************************

// HEADER FILES ------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <dos.h>
#include <stddef.h>
#include <string.h>
#include "h2def.h"
#include "i_sound.h"

// MACROS ------------------------------------------------------------------

#define MAX_AUDIO_TRACKS 25
#define MULTIPLEX_INT 0x2f
#define CDROM_GETDRIVECOUNT 0x1500
#define CDROM_SENDDEVICEREQ 0x1510
#define CDROM_GETVERSION 0x150c
#define HSG_MODE 0
#define RED_MODE 1
#define DRC_IOCTLINPUT 0x03
#define DRC_IOCTLOUTPUT 0x0c
#define DRC_PLAYAUDIO 0x84
#define DRC_STOPAUDIO 0x85
#define DRC_RESUMEAUDIO 0x88

#define DPMI_INT 0x31
#define DPMI_ALLOCREALMEM 0x0100
#define DPMI_FREEREALMEM 0x0101
#define DPMI_SIMREALINT 0x0300

// IOCTL input commands

#define ADRDEVHEAD       0 // Return Address of Device Header
#define HEADLOCATION     1 // Location of Head
#define RESERVED         2 // Reserved
#define ERRSTATISTICS    3 // Error Statistics
#define AUDIOCHANINFO    4 // Audio Channel Info
#define READDRVBYTES     5 // Read Drive Bytes
#define DEVICESTATUS     6 // Device Status
#define GETSECTORSIZE    7 // Return Sector Size
#define GETVOLSIZE       8 // Return Volume Size
#define MEDIACHANGED     9 // Media Changed
#define AUDIODISKINFO   10 // Audio Disk Info
#define AUDIOTRACKINFO  11 // Audio Track Info
#define AUDIOQCHANINFO  12 // Audio Q-Channel Info
#define AUDIOSUBINFO    13 // Audio Sub-Channel Info
#define UPCCODE         14 // UPC Code
#define AUDIOSTATUSINFO 15 // Audio Status Info

// IOCTL output commands

#define EJECTDISK        0 // Eject Disk
#define DOORLOCK         1 // Lock/Unlock Door
#define RESETDRIVE       2 // Reset Drive
#define AUDIOCHANCONTROL 3 // Audio Channel Control
#define WRITEDEVCONTROL  4 // Write Device Control String
#define CLOSETRAY        5 // Close Tray

// TYPES -------------------------------------------------------------------

typedef signed char		S_BYTE;
typedef unsigned char	U_BYTE;
typedef signed short	S_WORD;
typedef unsigned short	U_WORD;
typedef signed int		S_LONG;
typedef unsigned int	U_LONG;

typedef struct {
	U_LONG size;
	void **address;
	U_WORD *segment;
	U_WORD *selector;
} DOSChunk_t;

typedef struct {
	U_LONG edi;
	U_LONG esi;
	U_LONG ebp;
	U_LONG reserved;
	U_LONG ebx;
	U_LONG edx;
	U_LONG ecx;
	U_LONG eax;
	U_WORD flags;
	U_WORD es;
	U_WORD ds;
	U_WORD fs;
	U_WORD gs;
	U_WORD ip;
	U_WORD cs;
	U_WORD sp;
	U_WORD ss;
} RegBlock_t;

typedef struct {
	short lengthMin;
	short lengthSec;
	int redStart;
	int sectorStart;
	int sectorLength;
} AudioTrack_t;

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

static U_WORD InputIOCTL(S_WORD request, U_LONG pctrlblk);
static U_WORD OutputIOCTL(S_WORD request, U_LONG pctrlblk);
static U_LONG RedToSectors(U_LONG red);
static int AllocIOCTLBuffers(void);
static void DPMI_SimRealInt(U_LONG intr, RegBlock_t *rBlock);
static void *DPMI_AllocRealMem(U_LONG size, U_WORD *segment,
	U_WORD *selector);
static void DPMI_FreeRealMem(U_WORD selector);

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

// PUBLIC DATA DEFINITIONS -------------------------------------------------

int cd_Error;

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static int cd_DriveCount;
static int cd_FirstDrive;
static int cd_CurDrive;
static U_WORD cd_Version;
static int cd_FirstTrack;
static int cd_LastTrack;
static int cd_TrackCount;
static U_WORD cd_LeadOutMin;
static U_WORD cd_LeadOutSec;
static U_LONG cd_LeadOutRed;
static U_LONG cd_LeadOutSector;
static U_LONG cd_IOCTLBufferTotal;
static AudioTrack_t cd_AudioTracks[MAX_AUDIO_TRACKS];

static int OkInit = 0;

static RegBlock_t RegBlock;

static struct PlayReq_s {       // CD-ROM Play Audio Device Request Struct
	U_BYTE headerSize;
	U_BYTE subUnitCode;
	U_BYTE command;               // = DRC_PLAYAUDIO
	U_WORD status;
	U_BYTE reserved[8];
	U_BYTE addressMode;
	U_LONG startSector;
	U_LONG numberToRead;
} *cd_PlayReq;
static U_WORD cd_PlayReqSeg;
static U_WORD cd_PlayReqSel;

static struct StopReq_s {       // CD-ROM Stop Audio Device Request Struct
	U_BYTE headerSize;
	U_BYTE subUnitCode;
	U_BYTE command;               // = DRC_STOPAUDIO
	U_WORD status;
	U_BYTE reserved[8];
} *cd_StopReq;
static U_WORD cd_StopReqSeg;
static U_WORD cd_StopReqSel;

static struct ResumeReq_s {     // CD-ROM Resume Audio Device Request Struct
	U_BYTE headerSize;
	U_BYTE subUnitCode;
	U_BYTE command;               // = DRC_RESUMEAUDIO
	U_WORD status;
	U_BYTE reserved[8];
} *cd_ResumeReq;
static U_WORD cd_ResumeReqSeg;
static U_WORD cd_ResumeReqSel;

// IOCTL Input command data buffer structures

static struct IOCTLIn_s {       // IOCTL Input Struct
	U_BYTE headerSize;
	U_BYTE subUnitCode;
	U_BYTE command;               // = DRC_IOCTLINPUT
	U_WORD status;
	U_BYTE reserved[8];
	U_BYTE mediaDescriptor;
	U_LONG ctrlBlkAddr;
	U_WORD tranSize;
	U_WORD startSector;
	U_LONG volPtr;
} *cd_IOCTLIn;
static U_WORD cd_IOCTLInSeg;
static U_WORD cd_IOCTLInSel;

static struct RAddrDevHead_s {
	U_BYTE code;                  // ADRDEVHEAD
	U_LONG devHdrAddr;            // Address of device header
} *cd_RAddrDevHead;
static U_WORD cd_RAddrDevHeadSeg;
static U_WORD cd_RAddrDevHeadSel;

static struct LocHead_s {
	U_BYTE code;                  // HEADLOCATION
	U_BYTE addrMode;              // Addressing mode
	U_LONG headLocation;          // Location of drive head
} *cd_LocHead;
static U_WORD cd_LocHeadSeg;
static U_WORD cd_LocHeadSel;

static struct ErrStat_s {
	U_BYTE code;                  // ERRSTATISTICS
	U_BYTE errVal;                // Error statistics
} *cd_ErrStat;
static U_WORD cd_ErrStatSeg;
static U_WORD cd_ErrStatSel;

static struct AudChanInfo_s {
	U_BYTE code;               // AUDIOCHANINFO
	U_BYTE inChanOut0;         // Input chan(0,1,2,or 3) for output chan 0
	U_BYTE volumeOut0;         // Volume control (0-0xff) for output chan 0
	U_BYTE inChanOut1;         // Input chan(0,1,2,or 3) for output chan 1
	U_BYTE volumeOut1;         // Volume control (0-0xff) for output chan 1
	U_BYTE inChanOut2;         // Input chan(0,1,2,or 3) for output chan 2
	U_BYTE volumeOut2;         // Volume control (0-0xff) for output chan 2
	U_BYTE inChanOut3;         // Input chan(0,1,2,or 3) for output chan 3
	U_BYTE volumeOut3;         // Volume control (0-0xff) for output chan 3
} *cd_AudChanInfo;
static U_WORD cd_AudChanInfoSeg;
static U_WORD cd_AudChanInfoSel;

static struct RDrvBytes_s {
	U_BYTE code;                  // READDRVBYTES
	U_BYTE numBytes;              // Number of bytes to read
	U_BYTE rBuffer[128];          // Read buffer
} *cd_RDrvBytes;
static U_WORD cd_RDrvBytesSeg;
static U_WORD cd_RDrvBytesSel;

static struct DevStat_s {
	U_BYTE code;                  // DEVICESTATUS
	U_LONG devParams;             // Device parameters
} *cd_DevStat;
static U_WORD cd_DevStatSeg;
static U_WORD cd_DevStatSel;

static struct SectSize_s {
	U_BYTE code;                  // GETSECTORSIZE
	U_BYTE readMode;              // Read mode
	U_WORD sectorSize;           // Sector size
} *cd_SectSize;
static U_WORD cd_SectSizeSeg;
static U_WORD cd_SectSizeSel;

static struct VolSize_s {
	U_BYTE code;                  // GETVOLSIZE
	U_LONG volumeSize;            // Volume size
} *cd_VolSize;
static U_WORD cd_VolSizeSeg;
static U_WORD cd_VolSizeSel;

static struct MedChng_s {
	U_BYTE code;                  // MEDIACHANGED
	U_BYTE changed;               // Media byte
} *cd_MedChng;
static U_WORD cd_MedChngSeg;
static U_WORD cd_MedChngSel;

static struct DiskInfo_s {
	U_BYTE code;                  // AUDIODISKINFO
	U_BYTE lowTrack;              // Lowest track number
	U_BYTE highTrack;             // Highest track number
	U_LONG startLeadOut;          // Starting point of the lead-out track
} *cd_DiskInfo;
static U_WORD cd_DiskInfoSeg;
static U_WORD cd_DiskInfoSel;

static struct TrackInfo_s {
	U_BYTE code;                  // AUDIOTRACKINFO
	U_BYTE track;                 // Track number
	U_LONG start;                 // Starting point of the track
	U_BYTE ctrlInfo;              // Track control information
} *cd_TrackInfo;
static U_WORD cd_TrackInfoSeg;
static U_WORD cd_TrackInfoSel;

static struct QInfo_s {
	U_BYTE code;                  // AUDIOQCHANINFO
	U_BYTE control;               // CONTROL and ADR byte
	U_BYTE tno;                   // Track number (TNO)
	U_BYTE index;                 // (POINT) or Index(X)
	U_BYTE min;                   // (MIN) Running time within a track
	U_BYTE sec;                   // (SEC)    "      "    "    "   "
	U_BYTE frame;                 // (FRAME)  "      "    "    "   "
	U_BYTE zero;                  // (ZERO)   "      "    "    "   "
	U_BYTE aMin;                  // (AMIN) or (PMIN) Running time on disk
	U_BYTE aSec;                  // (ASEC) or (PSEC)    "      "   "   "
	U_BYTE aFrame;                // (AFRAME) or (PFRAME)"      "   "   "
} *cd_QInfo;
static U_WORD cd_QInfoSeg;
static U_WORD cd_QInfoSel;

static struct SubChanInfo_s {
	U_BYTE code;                  // AUDIOSUBINFO
	U_LONG startSectAddr;         // Starting sector address
	U_LONG transAddr;             // Transfer address
	U_LONG numSects;              // Number of sectors to read
} *cd_SubChanInfo;
static U_WORD cd_SubChanInfoSeg;
static U_WORD cd_SubChanInfoSel;

static struct UPCCode_s {
	U_BYTE code;                  // UPCCODE
	U_BYTE control;               // CONTROL and ADR byte
	U_BYTE upc[7];                // UPC/EAN code
	U_BYTE zero;                  // Zero
	U_BYTE aFrame;                // Aframe
} *cd_UPCCode;
static U_WORD cd_UPCCodeSeg;
static U_WORD cd_UPCCodeSel;

static struct AudStat_s {
	U_BYTE code;                  // AUDIOSTATUSINFO
	U_WORD status;                // Audio status bits
	U_LONG startPlay;             // Starting location of last Play/Resume
	U_LONG endPlay;               // Ending location for last Play/Resume
} *cd_AudStat;
static U_WORD cd_AudStatSeg;
static U_WORD cd_AudStatSel;

// IOCTL Output command data buffer structures

static struct IOCTLOut_s {      // IOCTL Output struct
	U_BYTE headerSize;
	U_BYTE subUnitCode;
	U_BYTE command;               // = DRC_IOCTLOUTPUT
	U_WORD status;
	U_BYTE reserved[8];
	U_BYTE mediaDescriptor;
	U_LONG ctrlBlkAddr;
	U_WORD tranSize;
	U_WORD startSector;
	U_LONG volPtr;
} *cd_IOCTLOut;
static U_WORD cd_IOCTLOutSeg;
static U_WORD cd_IOCTLOutSel;

static struct Eject_s {
	U_BYTE code;                  // EJECTDISK
} *cd_Eject;
static U_WORD cd_EjectSeg;
static U_WORD cd_EjectSel;

static struct LockDoor_s {
	U_BYTE code;                  // DOORLOCK
	U_BYTE lock;                  // Lock function : 0 = unlock, 1 = lock
} *cd_LockDoor;
static U_WORD cd_LockDoorSeg;
static U_WORD cd_LockDoorSel;

static struct ResetDrv_s {
	U_BYTE code;                  // RESETDRIVE
} *cd_ResetDrv;
static U_WORD cd_ResetDrvSeg;
static U_WORD cd_ResetDrvSel;

static struct AudInfo_s {
	U_BYTE code;               // AUDIOCHANCONTROL
	U_BYTE inChanOut0;         // Input chan(0,1,2,or 3) for output chan 0
	U_BYTE volumeOut0;         // Volume control (0-0xff) for output chan 0
	U_BYTE inChanOut1;         // Input chan(0,1,2,or 3) for output chan 1
	U_BYTE volumeOut1;         // Volume control (0-0xff) for output chan 1
	U_BYTE inChanOut2;         // Input chan(0,1,2,or 3) for output chan 2
	U_BYTE volumeOut2;         // Volume control (0-0xff) for output chan 2
	U_BYTE inChanOut3;         // Input chan(0,1,2,or 3) for output chan 3
	U_BYTE volumeOut3;         // Volume control (0-0xff) for output chan 3
} *cd_AudInfo;
static U_WORD cd_AudInfoSeg;
static U_WORD cd_AudInfoSel;

static struct WDrvBytes_s {
	U_BYTE code;                  // WRITEDEVCONTROL
	U_BYTE buf[5];                // Write buffer - size ??
} *cd_WDrvBytes;
static U_WORD cd_WDrvBytesSeg;
static U_WORD cd_WDrvBytesSel;

static struct CloseTray_s {
	U_BYTE code;                  // CLOSETRAY
} *cd_CloseTray;
static U_WORD cd_CloseTraySeg;
static U_WORD cd_CloseTraySel;

static U_WORD InCtrlBlkSize[16] = {
	0x05, 0x06, 0x00, 0x00,
	0x09, 0x82, 0x05, 0x04,
	0x05, 0x02, 0x07, 0x07,
	0x0b, 0x0d, 0x0b, 0x0b
};

static U_WORD OutCtrlBlkSize[6] = {
	0x01, 0x02,
	0x01, 0x09,
	0x06, 0x01
};

// Structures for allocating conventional memory

static DOSChunk_t DOSChunks[] = {
	{
		sizeof(struct PlayReq_s),
		&cd_PlayReq, &cd_PlayReqSeg, &cd_PlayReqSel
	},
	{
		sizeof(struct StopReq_s),
		&cd_StopReq, &cd_StopReqSeg, &cd_StopReqSel
	},
	{
		sizeof(struct ResumeReq_s),
		&cd_ResumeReq, &cd_ResumeReqSeg, &cd_ResumeReqSel
	},
	{
		sizeof(struct IOCTLOut_s),
		&cd_IOCTLOut, &cd_IOCTLOutSeg, &cd_IOCTLOutSel
	},
	{
		sizeof(struct Eject_s),
		&cd_Eject, &cd_EjectSeg, &cd_EjectSel
	},
	{
		sizeof(struct LockDoor_s),
		&cd_LockDoor, &cd_LockDoorSeg, &cd_LockDoorSel
	},
	{
		sizeof(struct ResetDrv_s),
		&cd_ResetDrv, &cd_ResetDrvSeg, &cd_ResetDrvSel
	},
	{
		sizeof(struct AudInfo_s),
		&cd_AudInfo, &cd_AudInfoSeg, &cd_AudInfoSel
	},
	{
		sizeof(struct WDrvBytes_s),
		&cd_WDrvBytes, &cd_WDrvBytesSeg, &cd_WDrvBytesSel
	},
	{
		sizeof(struct CloseTray_s),
		&cd_CloseTray, &cd_CloseTraySeg, &cd_CloseTraySel
	},
	{
		sizeof(struct IOCTLIn_s),
		&cd_IOCTLIn, &cd_IOCTLInSeg, &cd_IOCTLInSel
	},
	{
		sizeof(struct RAddrDevHead_s),
		&cd_RAddrDevHead, &cd_RAddrDevHeadSeg, &cd_RAddrDevHeadSel
	},
	{
		sizeof(struct LocHead_s),
		&cd_LocHead, &cd_LocHeadSeg, &cd_LocHeadSel
	},
	{
		sizeof(struct ErrStat_s),
		&cd_ErrStat, &cd_ErrStatSeg, &cd_ErrStatSel
	},
	{
		sizeof(struct AudChanInfo_s),
		&cd_AudChanInfo, &cd_AudChanInfoSeg, &cd_AudChanInfoSel
	},
	{
		sizeof(struct RDrvBytes_s),
		&cd_RDrvBytes, &cd_RDrvBytesSeg, &cd_RDrvBytesSel
	},
	{
		sizeof(struct DevStat_s),
		&cd_DevStat, &cd_DevStatSeg, &cd_DevStatSel
	},
	{
		sizeof(struct SectSize_s),
		&cd_SectSize, &cd_SectSizeSeg, &cd_SectSizeSel
	},
	{
		sizeof(struct VolSize_s),
		&cd_VolSize, &cd_VolSizeSeg, &cd_VolSizeSel
	},
	{
		sizeof(struct MedChng_s),
		&cd_MedChng, &cd_MedChngSeg, &cd_MedChngSel
	},
	{
		sizeof(struct DiskInfo_s),
		&cd_DiskInfo, &cd_DiskInfoSeg, &cd_DiskInfoSel
	},
	{
		sizeof(struct TrackInfo_s),
		&cd_TrackInfo, &cd_TrackInfoSeg, &cd_TrackInfoSel
	},
	{
		sizeof(struct QInfo_s),
		&cd_QInfo, &cd_QInfoSeg, &cd_QInfoSel
	},
	{
		sizeof(struct SubChanInfo_s),
		&cd_SubChanInfo, &cd_SubChanInfoSeg, &cd_SubChanInfoSel
	},
	{
		sizeof(struct UPCCode_s),
		&cd_UPCCode, &cd_UPCCodeSeg, &cd_UPCCodeSel
	},
	{
		sizeof(struct AudStat_s),
		&cd_AudStat, &cd_AudStatSeg, &cd_AudStatSel
	},
	{
		0, NULL, NULL, NULL
	}
};

// CODE --------------------------------------------------------------------

//==========================================================================
//
// I_CDMusInit
//
// Initializes the CD audio system.  Must be called before using any
// other I_CDMus functions.
//
// Returns: 0 (ok) or -1 (error, in cd_Error).
//
//==========================================================================

int I_CDMusInit(void)
{
	int i;
	int sect;
	int maxTrack;
	S_BYTE startMin1 = 0;
	S_BYTE startSec1 = 0;
	S_BYTE startMin2 = 0;
	S_BYTE startSec2 = 0;
	S_BYTE lengthMin = 0;
	S_BYTE lengthSec = 0;

	if(OkInit != 1)
	{ // Only execute if uninitialized

		// Get number of CD-ROM drives and first drive
		memset(&RegBlock, 0, sizeof(RegBlock));
		RegBlock.eax = CDROM_GETDRIVECOUNT;
		RegBlock.ebx = 0;
		DPMI_SimRealInt(MULTIPLEX_INT, &RegBlock);
		cd_DriveCount = RegBlock.ebx;

		// MSCDEX not installed if number of drives = 0
		if(cd_DriveCount == 0)
		{
			cd_Error = CDERR_NOTINSTALLED;
			return -1;
		}
		cd_FirstDrive = RegBlock.ecx;
		cd_CurDrive = cd_FirstDrive;

		// Allocate the IOCTL buffers
		if(AllocIOCTLBuffers() == -1)
		{
			cd_Error = CDERR_IOCTLBUFFMEM;
			return -1;
		}

		// Get MSCDEX version
		// Major version in upper byte, minor version in lower byte
		memset(&RegBlock, 0, sizeof(RegBlock));
		RegBlock.eax = CDROM_GETVERSION;
		DPMI_SimRealInt(MULTIPLEX_INT, &RegBlock);
		cd_Version = RegBlock.ebx;

		// Check device status to make sure we can read Audio CD's
		InputIOCTL(DEVICESTATUS, cd_DevStatSeg);
		if((cd_DevStat->devParams & 0x0010) == 0)
		{
			cd_Error = CDERR_NOAUDIOSUPPORT;
			return -1;
		}
	}

	// Force audio to stop playing
	I_CDMusStop();

	// Make sure we have the current TOC
	InputIOCTL(MEDIACHANGED, cd_MedChngSeg);

	// Set track variables
	InputIOCTL(AUDIODISKINFO, cd_DiskInfoSeg);
	cd_FirstTrack = cd_DiskInfo->lowTrack;
	cd_LastTrack = cd_DiskInfo->highTrack;
	if(cd_FirstTrack == 0 && cd_FirstTrack == cd_LastTrack)
	{
		cd_Error = CDERR_NOAUDIOTRACKS;
		return -1;
	}
	cd_TrackCount = cd_LastTrack-cd_FirstTrack+1;
	cd_LeadOutMin = cd_DiskInfo->startLeadOut>>16 & 0xFF;
	cd_LeadOutSec = cd_DiskInfo->startLeadOut>>8 & 0xFF;
	cd_LeadOutRed = cd_DiskInfo->startLeadOut;
	cd_LeadOutSector = RedToSectors(cd_DiskInfo->startLeadOut);

	// Create Red Book start, sector start, and sector length
	// for all tracks
	sect = cd_LeadOutSector;
	for(i = cd_LastTrack; i >= cd_FirstTrack; i--)
	{
		cd_TrackInfo->track = i;
		InputIOCTL(AUDIOTRACKINFO, cd_TrackInfoSeg);
		if(i < MAX_AUDIO_TRACKS)
		{
			cd_AudioTracks[i].redStart = cd_TrackInfo->start;
			cd_AudioTracks[i].sectorStart =
				RedToSectors(cd_TrackInfo->start);
			cd_AudioTracks[i].sectorLength =
				sect-RedToSectors(cd_TrackInfo->start);
		}
		sect = RedToSectors(cd_TrackInfo->start);
	}

	// Create track lengths in minutes and seconds
	if(cd_LastTrack >= MAX_AUDIO_TRACKS)
	{
		maxTrack = MAX_AUDIO_TRACKS-1;
	}
	else
	{
		maxTrack = cd_LastTrack;
	}
	cd_TrackInfo->track = cd_FirstTrack;
	InputIOCTL(AUDIOTRACKINFO, cd_TrackInfoSeg);
	startMin1 = (cd_TrackInfo->start >> 16);
	startSec1 = (cd_TrackInfo->start >> 8);
	for(i = cd_FirstTrack; i <= maxTrack; i++)
	{
		cd_TrackInfo->track = i+1;
		if(i < cd_LastTrack)
		{
			InputIOCTL(AUDIOTRACKINFO, cd_TrackInfoSeg);
			startMin2 = (cd_TrackInfo->start >> 16);
			startSec2 = (cd_TrackInfo->start >> 8);
		}
		else
		{
			startMin2 = cd_LeadOutRed>>16;
			startSec2 = cd_LeadOutRed>>8;
		}
		lengthSec = startSec2 - startSec1;
		lengthMin = startMin2 - startMin1;
		if(lengthSec < 0)
		{
			lengthSec += 60;
			lengthMin--;
		}
		cd_AudioTracks[i].lengthMin = lengthMin;
		cd_AudioTracks[i].lengthSec = lengthSec;
		startMin1 = startMin2;
		startSec1 = startSec2;
	}

	// Clip high tracks
	cd_LastTrack = maxTrack;

	OkInit = 1;
	return 0;
}

//==========================================================================
//
// I_CDMusPlay
//
// Play an audio CD track.
//
// Returns: 0 (ok) or -1 (error, in cd_Error).
//
//==========================================================================

int I_CDMusPlay(int track)
{
	int start;
	int len;

	if(track < cd_FirstTrack || track > cd_LastTrack)
	{
		cd_Error = CDERR_BADTRACK;
		return(-1);
	}
	I_CDMusStop();
	start = cd_AudioTracks[track].redStart;
	len = cd_AudioTracks[track].sectorLength;
	cd_PlayReq->addressMode = RED_MODE;
	cd_PlayReq->startSector = start;
	cd_PlayReq->numberToRead = len;
	memset(&RegBlock, 0, sizeof(RegBlock));
	RegBlock.eax = CDROM_SENDDEVICEREQ;
	RegBlock.ecx = cd_CurDrive;
	RegBlock.ebx = 0;
	RegBlock.es = cd_PlayReqSeg;
	DPMI_SimRealInt(MULTIPLEX_INT, &RegBlock);
	if(cd_PlayReq->status&0x8000)
	{
		cd_Error = CDERR_DEVREQBASE+(cd_PlayReq->status)&0x00ff;
		return(-1);
	}
	return(0);
}

//==========================================================================
//
// I_CDMusStop
//
// Stops the playing of an audio CD.
//
// Returns: 0 (ok) or -1 (error, in cd_Error).
//
//==========================================================================

int I_CDMusStop(void)
{
	memset(&RegBlock, 0, sizeof(RegBlock));
	RegBlock.eax = CDROM_SENDDEVICEREQ;
	RegBlock.ecx = cd_CurDrive;
	RegBlock.ebx = 0;
	RegBlock.es = cd_StopReqSeg;
	DPMI_SimRealInt(MULTIPLEX_INT, &RegBlock);
	if(cd_StopReq->status&0x8000)
	{
		cd_Error = CDERR_DEVREQBASE+(cd_StopReq->status)&0x00ff;
		return -1;
	}
	return 0;
}

//==========================================================================
//
// I_CDMusResume
//
// Resumes the playing of an audio CD.
//
// Returns: 0 (ok) or -1 (error, in cd_Error).
//
//==========================================================================

int I_CDMusResume(void)
{
	memset(&RegBlock, 0, sizeof(RegBlock));
	RegBlock.eax = CDROM_SENDDEVICEREQ;
	RegBlock.ecx = cd_CurDrive;
	RegBlock.ebx = 0;
	RegBlock.es = cd_ResumeReqSeg;
	DPMI_SimRealInt(MULTIPLEX_INT, &RegBlock);
	if(cd_ResumeReq->status&0x8000)
	{
		cd_Error = CDERR_DEVREQBASE+(cd_ResumeReq->status)&0x00ff;
		return -1;
	}
	return 0;
}

//==========================================================================
//
// I_CDMusSetVolume
//
// Sets the CD audio volume (0 - 255).
//
// Returns: 0 (ok) or -1 (error, in cd_Error).
//
//==========================================================================

int I_CDMusSetVolume(int volume)
{

	if(!OkInit)
	{
		cd_Error = CDERR_NOTINSTALLED;
	   	return -1;
	}

	// Read current channel info
	InputIOCTL(AUDIOCHANINFO, cd_AudChanInfoSeg);

	// Change the volumes
	cd_AudChanInfo->volumeOut0 =
		cd_AudChanInfo->volumeOut1 =
		cd_AudChanInfo->volumeOut2 =
		cd_AudChanInfo->volumeOut3 = volume;

	// Write modified channel info
	OutputIOCTL(AUDIOCHANCONTROL, cd_AudChanInfoSeg);

	return 0;
}

//==========================================================================
//
// I_CDMusFirstTrack
//
// Returns: the number of the first track.
//
//==========================================================================

int I_CDMusFirstTrack(void)
{
	return cd_FirstTrack;
}

//==========================================================================
//
// I_CDMusLastTrack
//
// Returns: the number of the last track.
//
//==========================================================================

int I_CDMusLastTrack(void)
{
	return cd_LastTrack;
}

//==========================================================================
//
// I_CDMusTrackLength
//
// Returns: Length of the given track in seconds, or -1 (error, in
// cd_Error).
//
//==========================================================================

int I_CDMusTrackLength(int track)
{
	if(track < cd_FirstTrack || track > cd_LastTrack)
	{
		cd_Error = CDERR_BADTRACK;
		return -1;
	}
	return cd_AudioTracks[track].lengthMin*60
		+cd_AudioTracks[track].lengthSec;
}

//==========================================================================
//
// AllocIOCTLBuffers
//
// Allocates conventional memory for the IOCTL input and output buffers.
// Sets cd_IOCTLBufferTotal to the total allocated.
//
// Returns: 0 (ok) or -1 (error, in cd_Error).
//
//==========================================================================

static int AllocIOCTLBuffers(void)
{
	int i;
	int size;
	DOSChunk_t *ck;

	cd_IOCTLBufferTotal = 0;
	for(i = 0; DOSChunks[i].size != 0; i++)
	{
		ck = &DOSChunks[i];
		size = ck->size;
		cd_IOCTLBufferTotal += (size+15)&0xfffffff0;
		*ck->address = DPMI_AllocRealMem(size, ck->segment, ck->selector);
		if(*ck->address == NULL)
		{
			return -1;
		}
		memset(*ck->address, 0, size);
	}
	cd_IOCTLIn->headerSize = sizeof(struct IOCTLIn_s);
	cd_IOCTLIn->command = DRC_IOCTLINPUT;
	cd_IOCTLOut->headerSize = sizeof(struct IOCTLOut_s);
	cd_IOCTLOut->command = DRC_IOCTLOUTPUT;
	cd_PlayReq->headerSize = sizeof(struct PlayReq_s);
	cd_PlayReq->command = DRC_PLAYAUDIO;
	cd_StopReq->headerSize = sizeof(struct StopReq_s);
	cd_StopReq->command = DRC_STOPAUDIO;
	cd_ResumeReq->headerSize = sizeof(struct ResumeReq_s);
	cd_ResumeReq->command = DRC_RESUMEAUDIO;
	return 0;
}

//==========================================================================
//
// InputIOCTL
//
// Sends an IOCTL input device request command.
//
// Returns: the status of the request.
//
//==========================================================================

static U_WORD InputIOCTL(S_LONG request, U_WORD buffSeg)
{
	U_BYTE *code;

	code = (U_BYTE *)(buffSeg<<4);
	*code = (U_BYTE)request;
	cd_IOCTLIn->ctrlBlkAddr = buffSeg<<16;
	cd_IOCTLIn->tranSize = InCtrlBlkSize[request];
	memset(&RegBlock, 0, sizeof(RegBlock));
	RegBlock.eax = CDROM_SENDDEVICEREQ;
	RegBlock.ecx = cd_CurDrive;
	RegBlock.ebx = 0;
	RegBlock.es = cd_IOCTLInSeg;
	DPMI_SimRealInt(MULTIPLEX_INT, &RegBlock);
	return cd_IOCTLIn->status;
}

//==========================================================================
//
// OutputIOCTL
//
// Sends an IOCTL output device request command.
//
// Returns: the status of the request.
//
//==========================================================================

static U_WORD OutputIOCTL(S_LONG request, U_WORD buffSeg)
{
	U_BYTE *code;

	code = (U_BYTE *)(buffSeg<<4);
	*code = (U_BYTE)request;
	cd_IOCTLOut->ctrlBlkAddr = buffSeg<<16;
	cd_IOCTLOut->tranSize = OutCtrlBlkSize[request];
	RegBlock.eax = CDROM_SENDDEVICEREQ;
	RegBlock.ecx = cd_CurDrive;
	RegBlock.ebx = 0;
	RegBlock.es = cd_IOCTLOutSeg;
	DPMI_SimRealInt(MULTIPLEX_INT, &RegBlock);
	return cd_IOCTLOut->status;
}

//==========================================================================
//
// RedToSectors
//
// Converts Red Book addresses to HSG sectors.
// Sectors = Minutes * 60 * 75 + Seconds * 75 + Frame - 150
//
// Returns: HSG sectors.
//
//==========================================================================

static U_LONG RedToSectors(U_LONG red)
{
	U_LONG sector;

	sector = ((red&0x00ff0000) >> 16) * 60 * 75;
	sector += ((red&0x0000ff00) >> 8) * 75;
	sector += (red&0x000000ff);
	return sector-150;
}

//==========================================================================
//
// DPMI_SimRealInt
//
//==========================================================================

static void DPMI_SimRealInt(U_LONG intr, RegBlock_t *rBlock)
{
	union REGS regs;
	struct SREGS sRegs;

	regs.x.eax = DPMI_SIMREALINT;
	regs.x.ebx = intr;
	regs.x.ecx = 0;
	regs.x.edi = FP_OFF((void far *)rBlock);
	sRegs.es = FP_SEG((void far *)rBlock);
	sRegs.ds = FP_SEG((void far *)rBlock);
	int386x(DPMI_INT, &regs, &regs, &sRegs);
}

//==========================================================================
//
// DPMI_AllocRealMem
//
//==========================================================================

static void *DPMI_AllocRealMem(U_LONG size, U_WORD *segment,
	U_WORD *selector)
{
	union REGS inRegs;
	union REGS outRegs;

	inRegs.x.eax = DPMI_ALLOCREALMEM;
	inRegs.x.ebx = (size+15)/16;
	int386(DPMI_INT, &inRegs, &outRegs);
	if(outRegs.x.cflag)
	{
		return NULL;
	}
	*segment = outRegs.x.eax&0xffff;
	*selector = outRegs.x.edx&0xffff;
	return (void *)(outRegs.x.eax<<4);
}

//==========================================================================
//
// DPMI_FreeRealMem
//
//==========================================================================

static void DPMI_FreeRealMem(U_WORD selector)
{
	union REGS regs;

	regs.x.eax = DPMI_FREEREALMEM;
	regs.x.edx = selector;
	int386(DPMI_INT, &regs, &regs);
}
