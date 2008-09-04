
// i_cdmus.h

#ifndef __ICDMUS__
#define __ICDMUS__

#define CDERR_NOTINSTALLED   10     // MSCDEX not installed
#define CDERR_NOAUDIOSUPPORT 11     // CD-ROM Doesn't support audio
#define CDERR_NOAUDIOTRACKS  12     // Current CD has no audio tracks
#define CDERR_BADDRIVE       20     // Bad drive number
#define CDERR_BADTRACK       21     // Bad track number
#define CDERR_IOCTLBUFFMEM   22     // Not enough low memory for IOCTL
#define CDERR_DEVREQBASE     100    // DevReq errors

extern int cd_Error;

int I_CDMusInit(void);
int I_CDMusPlay(int track);
int I_CDMusStop(void);
int I_CDMusResume(void);
int I_CDMusSetVolume(int volume);
int I_CDMusFirstTrack(void);
int I_CDMusLastTrack(void);
int I_CDMusTrackLength(int track);

#endif
