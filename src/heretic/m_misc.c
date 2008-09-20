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

extern int snd_Channels;
extern int snd_DesiredMusicDevice, snd_DesiredSfxDevice;
extern int snd_MusicDevice,     // current music card # (index to dmxCodes)
  snd_SfxDevice;                // current sfx card # (index to dmxCodes)

extern int snd_SBport, snd_SBirq, snd_SBdma;    // sound blaster variables
extern int snd_Mport;           // midi variables

default_t defaults[] = {
    {"mouse_sensitivity", &mouseSensitivity, 5},

    {"sfx_volume", &snd_MaxVolume, 10},
    {"music_volume", &snd_MusicVolume, 10},

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

    {"snd_channels", &snd_Channels, 3},
    {"snd_musicdevice", &snd_DesiredMusicDevice, 0},
    {"snd_sfxdevice", &snd_DesiredSfxDevice, 0},
    {"snd_sbport", &snd_SBport, 544},
    {"snd_sbirq", &snd_SBirq, -1},
    {"snd_sbdma", &snd_SBdma, -1},
    {"snd_mport", &snd_Mport, -1},

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

