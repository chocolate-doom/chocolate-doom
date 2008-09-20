// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 1993-2008 Raven Software
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
//-----------------------------------------------------------------------------

// M_misc.c

#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdlib.h>

#include <ctype.h>

#include "doomdef.h"
#include "i_swap.h"
#include "i_video.h"
#include "m_argv.h"
#include "s_sound.h"

//---------------------------------------------------------------------------
//
// FUNC M_ValidEpisodeMap
//
//---------------------------------------------------------------------------

boolean M_ValidEpisodeMap(int episode, int map)
{
    if (episode < 1 || map < 1 || map > 9)
    {
        return false;
    }
    if (gamemode == shareware)
    {                           // Shareware version checks
        if (episode != 1)
        {
            return false;
        }
    }
    else if (ExtendedWAD)
    {                           // Extended version checks
        if (episode == 6)
        {
            if (map > 3)
            {
                return false;
            }
        }
        else if (episode > 5)
        {
            return false;
        }
    }
    else
    {                           // Registered version checks
        if (episode == 4)
        {
            if (map != 1)
            {
                return false;
            }
        }
        else if (episode > 3)
        {
            return false;
        }
    }
    return true;
}

/*
===============
=
= M_Random
=
= Returns a 0-255 number
=
===============
*/

unsigned char rndtable[256] = {
    0, 8, 109, 220, 222, 241, 149, 107, 75, 248, 254, 140, 16, 66,
    74, 21, 211, 47, 80, 242, 154, 27, 205, 128, 161, 89, 77, 36,
    95, 110, 85, 48, 212, 140, 211, 249, 22, 79, 200, 50, 28, 188,
    52, 140, 202, 120, 68, 145, 62, 70, 184, 190, 91, 197, 152, 224,
    149, 104, 25, 178, 252, 182, 202, 182, 141, 197, 4, 81, 181, 242,
    145, 42, 39, 227, 156, 198, 225, 193, 219, 93, 122, 175, 249, 0,
    175, 143, 70, 239, 46, 246, 163, 53, 163, 109, 168, 135, 2, 235,
    25, 92, 20, 145, 138, 77, 69, 166, 78, 176, 173, 212, 166, 113,
    94, 161, 41, 50, 239, 49, 111, 164, 70, 60, 2, 37, 171, 75,
    136, 156, 11, 56, 42, 146, 138, 229, 73, 146, 77, 61, 98, 196,
    135, 106, 63, 197, 195, 86, 96, 203, 113, 101, 170, 247, 181, 113,
    80, 250, 108, 7, 255, 237, 129, 226, 79, 107, 112, 166, 103, 241,
    24, 223, 239, 120, 198, 58, 60, 82, 128, 3, 184, 66, 143, 224,
    145, 224, 81, 206, 163, 45, 63, 90, 168, 114, 59, 33, 159, 95,
    28, 139, 123, 98, 125, 196, 15, 70, 194, 253, 54, 14, 109, 226,
    71, 17, 161, 93, 186, 87, 244, 138, 20, 52, 123, 251, 26, 36,
    17, 46, 52, 231, 232, 76, 31, 221, 84, 37, 216, 165, 212, 106,
    197, 242, 98, 43, 39, 175, 254, 145, 190, 84, 118, 222, 187, 136,
    120, 163, 236, 249
};
int rndindex = 0;
int prndindex = 0;

int P_Random(void)
{
    prndindex = (prndindex + 1) & 0xff;
    return rndtable[prndindex];
}

int M_Random(void)
{
    rndindex = (rndindex + 1) & 0xff;
    return rndtable[rndindex];
}

void M_ClearRandom(void)
{
    rndindex = prndindex = 0;
}


void M_ClearBox(fixed_t * box)
{
    box[BOXTOP] = box[BOXRIGHT] = INT_MIN;
    box[BOXBOTTOM] = box[BOXLEFT] = INT_MAX;
}

void M_AddToBox(fixed_t * box, fixed_t x, fixed_t y)
{
    if (x < box[BOXLEFT])
        box[BOXLEFT] = x;
    else if (x > box[BOXRIGHT])
        box[BOXRIGHT] = x;
    if (y < box[BOXBOTTOM])
        box[BOXBOTTOM] = y;
    else if (y > box[BOXTOP])
        box[BOXTOP] = y;
}

//---------------------------------------------------------------------------
//
// PROC M_ForceUppercase
//
// Change string to uppercase.
//
//---------------------------------------------------------------------------

void M_ForceUppercase(char *text)
{
    char c;

    while ((c = *text) != 0)
    {
        if (c >= 'a' && c <= 'z')
        {
            *text++ = c - ('a' - 'A');
        }
        else
        {
            text++;
        }
    }
}

/*
==============================================================================

							DEFAULTS

==============================================================================
*/

int usemouse;
int usejoystick;

extern int usegamma;
extern int key_right, key_left, key_up, key_down;
extern int key_strafeleft, key_straferight;
extern int key_fire, key_use, key_strafe, key_speed;
extern int key_flyup, key_flydown, key_flycenter;
extern int key_lookup, key_lookdown, key_lookcenter;
extern int key_invleft, key_invright, key_useartifact;

extern int mousebfire;
extern int mousebstrafe;
extern int mousebforward;

extern int joybfire;
extern int joybstrafe;
extern int joybuse;
extern int joybspeed;

extern int viewwidth, viewheight;

int mouseSensitivity;

extern int screenblocks;

extern char *chat_macros[10];

typedef struct
{
    char *name;
    int *location;
    int defaultvalue;
    int scantranslate;          // PC scan code hack
    int untranslated;           // lousy hack
} default_t;

#ifndef __NeXT__
extern int snd_Channels;
extern int snd_DesiredMusicDevice, snd_DesiredSfxDevice;
extern int snd_MusicDevice,     // current music card # (index to dmxCodes)
  snd_SfxDevice;                // current sfx card # (index to dmxCodes)

extern int snd_SBport, snd_SBirq, snd_SBdma;    // sound blaster variables
extern int snd_Mport;           // midi variables
#endif

default_t defaults[] = {
    {"mouse_sensitivity", &mouseSensitivity, 5},

#ifndef __NeXT__
    {"sfx_volume", &snd_MaxVolume, 10},
    {"music_volume", &snd_MusicVolume, 10},
#endif

#ifdef __WATCOMC__
#define SC_UPARROW              0x48
#define SC_DOWNARROW            0x50
#define SC_LEFTARROW            0x4b
#define SC_RIGHTARROW           0x4d
#define SC_RCTRL                0x1d
#define SC_RALT                 0x38
#define SC_RSHIFT               0x36
#define SC_SPACE                0x39
#define SC_COMMA                0x33
#define SC_PERIOD               0x34
#define SC_PAGEUP				0x49
#define SC_INSERT				0x52
#define SC_HOME					0x47
#define SC_PAGEDOWN				0x51
#define SC_DELETE				0x53
#define SC_END					0x4f
#define SC_ENTER				0x1c

    {"key_right", &key_right, SC_RIGHTARROW, 1},
    {"key_left", &key_left, SC_LEFTARROW, 1},
    {"key_up", &key_up, SC_UPARROW, 1},
    {"key_down", &key_down, SC_DOWNARROW, 1},
    {"key_strafeleft", &key_strafeleft, SC_COMMA, 1},
    {"key_straferight", &key_straferight, SC_PERIOD, 1},
    {"key_flyup", &key_flyup, SC_PAGEUP, 1},
    {"key_flydown", &key_flydown, SC_INSERT, 1},
    {"key_flycenter", &key_flycenter, SC_HOME, 1},
    {"key_lookup", &key_lookup, SC_PAGEDOWN, 1},
    {"key_lookdown", &key_lookdown, SC_DELETE, 1},
    {"key_lookcenter", &key_lookcenter, SC_END, 1},
    {"key_invleft", &key_invleft, 0x1a, 1},
    {"key_invright", &key_invright, 0x1b, 1},
    {"key_useartifact", &key_useartifact, SC_ENTER, 1},

    {"key_fire", &key_fire, SC_RCTRL, 1},
    {"key_use", &key_use, SC_SPACE, 1},
    {"key_strafe", &key_strafe, SC_RALT, 1},
    {"key_speed", &key_speed, SC_RSHIFT, 1},
#endif

#ifdef __NeXT__
    {"key_right", &key_right, KEY_RIGHTARROW},
    {"key_left", &key_left, KEY_LEFTARROW},
    {"key_up", &key_up, KEY_UPARROW},
    {"key_down", &key_down, KEY_DOWNARROW},
    {"key_strafeleft", &key_strafeleft, ','},
    {"key_straferight", &key_straferight, '.'},
    {"key_flyup", &key_flyup, 'u'},
    {"key_flydown", &key_flydown, 'j'},
    {"key_flycenter", &key_flycenter, 'k'},
    {"key_lookup", &key_lookup, 'm'},
    {"key_lookdown", &key_lookdown, 'b'},
    {"key_lookcenter", &key_lookcenter, 'n'},
    {"key_invleft", &key_invleft, '['},
    {"key_invright", &key_invright, ']'},
    {"key_useartifact", &key_useartifact, 13},

    {"key_fire", &key_fire, ' ', 1},
    {"key_use", &key_use, 'x', 1},
    {"key_strafe", &key_strafe, 'c', 1},
    {"key_speed", &key_speed, 'z', 1},
#endif

    {"use_mouse", &usemouse, 1},
    {"mouseb_fire", &mousebfire, 0},
    {"mouseb_strafe", &mousebstrafe, 1},
    {"mouseb_forward", &mousebforward, 2},

    {"use_joystick", &usejoystick, 0},
    {"joyb_fire", &joybfire, 0},
    {"joyb_strafe", &joybstrafe, 1},
    {"joyb_use", &joybuse, 3},
    {"joyb_speed", &joybspeed, 2},

    {"screenblocks", &screenblocks, 10},

#ifndef __NeXT__
    {"snd_channels", &snd_Channels, 3},
    {"snd_musicdevice", &snd_DesiredMusicDevice, 0},
    {"snd_sfxdevice", &snd_DesiredSfxDevice, 0},
    {"snd_sbport", &snd_SBport, 544},
    {"snd_sbirq", &snd_SBirq, -1},
    {"snd_sbdma", &snd_SBdma, -1},
    {"snd_mport", &snd_Mport, -1},
#endif

    {"usegamma", &usegamma, 0},

    {"chatmacro0", (int *) &chat_macros[0], (int) HUSTR_CHATMACRO0},
    {"chatmacro1", (int *) &chat_macros[1], (int) HUSTR_CHATMACRO1},
    {"chatmacro2", (int *) &chat_macros[2], (int) HUSTR_CHATMACRO2},
    {"chatmacro3", (int *) &chat_macros[3], (int) HUSTR_CHATMACRO3},
    {"chatmacro4", (int *) &chat_macros[4], (int) HUSTR_CHATMACRO4},
    {"chatmacro5", (int *) &chat_macros[5], (int) HUSTR_CHATMACRO5},
    {"chatmacro6", (int *) &chat_macros[6], (int) HUSTR_CHATMACRO6},
    {"chatmacro7", (int *) &chat_macros[7], (int) HUSTR_CHATMACRO7},
    {"chatmacro8", (int *) &chat_macros[8], (int) HUSTR_CHATMACRO8},
    {"chatmacro9", (int *) &chat_macros[9], (int) HUSTR_CHATMACRO9}
};

int numdefaults;
char *defaultfile;

/*
==============
=
= M_SaveDefaults
=
==============
*/

void M_SaveDefaults(void)
{
    int i, v;
    FILE *f;

    f = fopen(defaultfile, "w");
    if (!f)
        return;                 // can't write the file, but don't complain

    for (i = 0; i < numdefaults; i++)
    {
#ifdef __WATCOMC__
        if (defaults[i].scantranslate)
            defaults[i].location = &defaults[i].untranslated;
#endif
        if (defaults[i].defaultvalue > -0xfff
            && defaults[i].defaultvalue < 0xfff)
        {
            v = *defaults[i].location;
            fprintf(f, "%s\t\t%i\n", defaults[i].name, v);
        }
        else
        {
            fprintf(f, "%s\t\t\"%s\"\n", defaults[i].name,
                    *(char **) (defaults[i].location));
        }
    }

    fclose(f);
}


/*
==============
=
= M_LoadDefaults
=
==============
*/

extern byte scantokey[128];
extern char *basedefault;

void M_LoadDefaults(void)
{
    int i, len;
    FILE *f;
    char def[80];
    char strparm[100];
    char *newstring;
    int parm;
    boolean isstring;

//
// set everything to base values
//
    numdefaults = sizeof(defaults) / sizeof(defaults[0]);
    for (i = 0; i < numdefaults; i++)
        *defaults[i].location = defaults[i].defaultvalue;

//
// check for a custom default file
//
    i = M_CheckParm("-config");
    if (i && i < myargc - 1)
    {
        defaultfile = myargv[i + 1];
        printf("default file: %s\n", defaultfile);
    }
    else if (cdrom)
    {
        defaultfile = "c:\\heretic.cd\\heretic.cfg";
    }
    else
    {
        defaultfile = basedefault;
    }

//
// read the file in, overriding any set defaults
//
    f = fopen(defaultfile, "r");
    if (f)
    {
        while (!feof(f))
        {
            isstring = false;
            if (fscanf(f, "%79s %[^\n]\n", def, strparm) == 2)
            {
                if (strparm[0] == '"')
                {
                    // get a string default
                    isstring = true;
                    len = strlen(strparm);
                    newstring = (char *) malloc(len);
                    strparm[len - 1] = 0;
                    strcpy(newstring, strparm + 1);
                }
                else if (strparm[0] == '0' && strparm[1] == 'x')
                    sscanf(strparm + 2, "%x", &parm);
                else
                    sscanf(strparm, "%i", &parm);
                for (i = 0; i < numdefaults; i++)
                    if (!strcmp(def, defaults[i].name))
                    {
                        if (!isstring)
                            *defaults[i].location = parm;
                        else
                            *defaults[i].location = (int) newstring;
                        break;
                    }
            }
        }

        fclose(f);
    }


#ifdef __WATCOMC__
    for (i = 0; i < numdefaults; i++)
    {
        if (defaults[i].scantranslate)
        {
            parm = *defaults[i].location;
            defaults[i].untranslated = parm;
            *defaults[i].location = scantokey[parm];
        }
    }
#endif
}

