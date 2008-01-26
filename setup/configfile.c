// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
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
//
// DESCRIPTION:
//     Configuration file load/save code.
//
//-----------------------------------------------------------------------------


#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

// for mkdir:

#ifdef _WIN32
#include <io.h>
#else
#include <sys/stat.h>
#include <sys/types.h>
#endif

#include "config.h"

#include "doomfeatures.h"
#include "doomkeys.h"
#include "doomtype.h"
#include "d_englsh.h"

#include "m_argv.h"

#include "compatibility.h"
#include "display.h"
#include "joystick.h"
#include "keyboard.h"
#include "mouse.h"
#include "multiplayer.h"
#include "sound.h"

char *configdir;

//
// Create a directory
//

void M_MakeDirectory(char *path)
{
#ifdef _WIN32
    mkdir(path);
#else
    mkdir(path, 0755);
#endif
}


// 
// SetConfigDir:
//
// Sets the location of the configuration directory, where configuration
// files are stored - default.cfg, chocolate-doom.cfg, savegames, etc.
//

void M_SetConfigDir(void)
{
#ifndef _WIN32
    // Ignore the HOME environment variable on Windows - just behave
    // like Vanilla Doom.

    char *homedir;

    homedir = getenv("HOME");

    if (homedir != NULL)
    {
        // put all configuration in a config directory off the
        // homedir

        configdir = malloc(strlen(homedir) + strlen(PACKAGE_TARNAME) + 5);

        sprintf(configdir, "%s/.%s/", homedir, PACKAGE_TARNAME);

        // make the directory if it doesnt already exist

        M_MakeDirectory(configdir);
    }
    else
#endif /* #ifndef _WIN32 */
    {
#ifdef _WIN32
        // when given the -cdrom option, save config+savegames in 
        // c:\doomdata.  This only applies under Windows.

        if (M_CheckParm("-cdrom") > 0)
        {
            printf(D_CDROM);
            configdir = strdup("c:\\doomdata\\");

            M_MakeDirectory(configdir);
        }
        else
#endif
        {
            configdir = strdup("");
        }
    }
}



//
// DEFAULTS
//

// These are options that are not controlled by setup.

static int showMessages = 1;
static int screenblocks = 9;
static int detailLevel = 0;
static int usegamma = 0;

// dos specific options: these are unused but should be maintained
// so that the config file can be shared between chocolate
// doom and doom.exe

static int snd_sbport = 0;
static int snd_sbirq = 0;
static int snd_sbdma = 0;
static int snd_mport = 0;

typedef enum 
{
    DEFAULT_INT,
    DEFAULT_STRING,
    DEFAULT_FLOAT,
    DEFAULT_KEY,
} default_type_t;

typedef struct
{
    // Name of the variable
    char *         name;

    // Pointer to the location in memory of the variable
    void *         location;

    // Type of the variable
    default_type_t type;

    // If this is a key value, the original integer scancode we read from
    // the config file before translating it to the internal key value.
    // If zero, we didn't read this value from a config file.
    int            untranslated;

    // The value we translated the scancode into when we read the 
    // config file on startup.  If the variable value is different from
    // this, it has been changed and needs to be converted; otherwise,
    // use the 'untranslated' value.
    int            original_translated;
} default_t;

typedef struct
{
    default_t *defaults;
    int        numdefaults;
    char      *filename;
} default_collection_t;

static default_t doom_defaults_list[] =
{
    {"mouse_sensitivity", &mouseSensitivity, DEFAULT_INT, 0, 0},
    {"sfx_volume",&sfxVolume, DEFAULT_INT, 0, 0},
    {"music_volume",&musicVolume, DEFAULT_INT, 0, 0},
    {"show_messages",&showMessages, DEFAULT_INT, 0, 0},

    {"key_right",&key_right, DEFAULT_KEY, 0, 0},
    {"key_left",&key_left, DEFAULT_KEY, 0, 0},
    {"key_up",&key_up, DEFAULT_KEY, 0, 0},
    {"key_down",&key_down, DEFAULT_KEY, 0, 0},
    {"key_strafeleft",&key_strafeleft, DEFAULT_KEY, 0, 0},
    {"key_straferight",&key_straferight, DEFAULT_KEY, 0, 0},

    {"key_fire",&key_fire, DEFAULT_KEY, 0, 0},
    {"key_use",&key_use, DEFAULT_KEY, 0, 0},
    {"key_strafe",&key_strafe, DEFAULT_KEY, 0, 0},
    {"key_speed",&key_speed, DEFAULT_KEY, 0, 0},

    {"use_mouse",&usemouse, DEFAULT_INT, 0, 0},
    {"mouseb_fire",&mousebfire, DEFAULT_INT, 0, 0},
    {"mouseb_strafe",&mousebstrafe, DEFAULT_INT, 0, 0},
    {"mouseb_forward",&mousebforward, DEFAULT_INT, 0, 0},

    {"use_joystick",&usejoystick, DEFAULT_INT, 0, 0},
    {"joyb_fire",&joybfire, DEFAULT_INT, 0, 0},
    {"joyb_strafe",&joybstrafe, DEFAULT_INT, 0, 0},
    {"joyb_use",&joybuse, DEFAULT_INT, 0, 0},
    {"joyb_speed",&joybspeed, DEFAULT_INT, 0, 0},

    {"screenblocks",&screenblocks, DEFAULT_INT, 0, 0},
    {"detaillevel",&detailLevel, DEFAULT_INT, 0, 0},

    {"snd_channels",&numChannels, DEFAULT_INT, 0, 0},

    {"snd_musicdevice", &snd_musicdevice, DEFAULT_INT, 0, 0},
    {"snd_sfxdevice", &snd_sfxdevice, DEFAULT_INT, 0, 0},
    {"snd_sbport", &snd_sbport, DEFAULT_INT, 0, 0},
    {"snd_sbirq", &snd_sbirq, DEFAULT_INT, 0, 0},
    {"snd_sbdma", &snd_sbdma, DEFAULT_INT, 0, 0},
    {"snd_mport", &snd_mport, DEFAULT_INT, 0, 0},

    {"usegamma", &usegamma, DEFAULT_INT, 0, 0},

    {"chatmacro0", &chat_macros[0], DEFAULT_STRING, 0, 0 },
    {"chatmacro1", &chat_macros[1], DEFAULT_STRING, 0, 0 },
    {"chatmacro2", &chat_macros[2], DEFAULT_STRING, 0, 0 },
    {"chatmacro3", &chat_macros[3], DEFAULT_STRING, 0, 0 },
    {"chatmacro4", &chat_macros[4], DEFAULT_STRING, 0, 0 },
    {"chatmacro5", &chat_macros[5], DEFAULT_STRING, 0, 0 },
    {"chatmacro6", &chat_macros[6], DEFAULT_STRING, 0, 0 },
    {"chatmacro7", &chat_macros[7], DEFAULT_STRING, 0, 0 },
    {"chatmacro8", &chat_macros[8], DEFAULT_STRING, 0, 0 },
    {"chatmacro9", &chat_macros[9], DEFAULT_STRING, 0, 0 },
};

static default_collection_t doom_defaults = 
{
    doom_defaults_list,
    arrlen(doom_defaults_list),
    NULL,
};

static default_t extra_defaults_list[] = 
{
    {"autoadjust_video_settings",   &autoadjust_video_settings, DEFAULT_INT, 0, 0},
    {"fullscreen",                  &fullscreen, DEFAULT_INT, 0, 0},
    {"aspect_ratio_correct",        &aspect_ratio_correct, DEFAULT_INT, 0, 0},
    {"startup_delay",               &startup_delay, DEFAULT_INT, 0, 0},
    {"screenmultiply",              &screenmultiply, DEFAULT_INT, 0, 0},
    {"grabmouse",                   &grabmouse, DEFAULT_INT, 0, 0},
    {"novert",                      &novert, DEFAULT_INT, 0, 0},
    {"mouse_acceleration",          &mouse_acceleration, DEFAULT_FLOAT, 0, 0},
    {"mouse_threshold",             &mouse_threshold, DEFAULT_INT, 0, 0},
    {"snd_samplerate",              &snd_samplerate, DEFAULT_INT, 0, 0},
    {"show_endoom",                 &show_endoom, DEFAULT_INT, 0, 0},
    {"vanilla_savegame_limit",      &vanilla_savegame_limit, DEFAULT_INT, 0, 0},
    {"vanilla_demo_limit",          &vanilla_demo_limit, DEFAULT_INT, 0, 0},
    {"vanilla_keyboard_mapping",    &vanilla_keyboard_mapping, DEFAULT_INT, 0, 0},
#ifdef FEATURE_MULTIPLAYER
    {"player_name",                 &net_player_name,          DEFAULT_STRING, 0, 0},
    {"player_color",				&net_player_color,		   DEFAULT_INT, 0, 0},
#endif
    {"video_driver",                &video_driver,             DEFAULT_STRING, 0, 0},
    {"joystick_index",              &joystick_index, DEFAULT_INT, 0, 0},
    {"joystick_x_axis",             &joystick_x_axis, DEFAULT_INT, 0, 0},
    {"joystick_x_invert",           &joystick_x_invert, DEFAULT_INT, 0, 0},
    {"joystick_y_axis",             &joystick_y_axis, DEFAULT_INT, 0, 0},
    {"joystick_y_invert",           &joystick_y_invert, DEFAULT_INT, 0, 0},
    {"joyb_strafeleft",             &joybstrafeleft, DEFAULT_INT, 0, 0},
    {"joyb_straferight",            &joybstraferight, DEFAULT_INT, 0, 0},
    {"dclick_use",                  &dclick_use, DEFAULT_INT, 0, 0},
    {"mouseb_strafeleft",           &mousebstrafeleft, DEFAULT_INT, 0, 0},
    {"mouseb_straferight",          &mousebstraferight, DEFAULT_INT, 0, 0},
    {"mouseb_use",                  &mousebuse, DEFAULT_INT, 0, 0},
    {"mouseb_backward",             &mousebbackward, DEFAULT_INT, 0, 0},
    
    {"key_showscores",				&key_showscores, DEFAULT_KEY, 0, 0},
    {"supercoopspy",				&supercoopspy, DEFAULT_INT, 0, 0},
};

static default_collection_t extra_defaults =
{
    extra_defaults_list,
    arrlen(extra_defaults_list),
    NULL,
};

static int scantokey[128] =
{
    0  ,    27,     '1',    '2',    '3',    '4',    '5',    '6',
    '7',    '8',    '9',    '0',    '-',    '=',    KEY_BACKSPACE, 9,
    'q',    'w',    'e',    'r',    't',    'y',    'u',    'i',
    'o',    'p',    '[',    ']',    13,		KEY_RCTRL, 'a',    's',
    'd',    'f',    'g',    'h',    'j',    'k',    'l',    ';',
    '\'',   '`',    KEY_RSHIFT,'\\',   'z',    'x',    'c',    'v',
    'b',    'n',    'm',    ',',    '.',    '/',    KEY_RSHIFT,KEYP_MULTIPLY,
    KEY_RALT,  ' ',  KEY_CAPSLOCK,KEY_F1,  KEY_F2,   KEY_F3,   KEY_F4,   KEY_F5,
    KEY_F6,   KEY_F7,   KEY_F8,   KEY_F9,   KEY_F10,  KEY_PAUSE,KEY_SCRLCK,KEY_HOME,
    KEY_UPARROW,KEY_PGUP,KEYP_MINUS,KEY_LEFTARROW,KEYP_5,KEY_RIGHTARROW,KEYP_PLUS,KEY_END,
    KEY_DOWNARROW,KEY_PGDN,KEY_INS,KEY_DEL,0,   0,      0,      KEY_F11,
    KEY_F12,  0,      0,      0,      0,      0,      0,      0,
    0,      0,      0,      0,      0,      0,      0,      0,
    0,      0,      0,      0,      0,      0,      0,      0,
    0,      0,      0,      0,      0,      0,      0,      0,
    0,      0,      0,      0,      0,      0,      0,      0
};


static void SaveDefaultCollection(default_collection_t *collection)
{
    default_t *defaults;
    int i, v;
    FILE *f;
	
    f = fopen (collection->filename, "w");
    if (!f)
	return; // can't write the file, but don't complain

    defaults = collection->defaults;
		
    for (i=0 ; i<collection->numdefaults ; i++)
    {
        int chars_written;

        // Print the name and line up all values at 30 characters

        chars_written = fprintf(f, "%s ", defaults[i].name);

        for (; chars_written < 30; ++chars_written)
            fprintf(f, " ");

        // Print the value

        switch (defaults[i].type) 
        {
            case DEFAULT_KEY:

                // use the untranslated version if we can, to reduce
                // the possibility of screwing up the user's config
                // file
                
                v = * (int *) defaults[i].location;

                if (defaults[i].untranslated
                 && v == defaults[i].original_translated)
                {
                    // Has not been changed since the last time we
                    // read the config file.

                    v = defaults[i].untranslated;
                }
                else
                {
                    // search for a reverse mapping back to a scancode
                    // in the scantokey table

                    int s;

                    for (s=0; s<128; ++s)
                    {
                        if (scantokey[s] == v)
                        {
                            v = s;
                            break;
                        }
                    }
                }

	        fprintf(f, "%i", v);
                break;

            case DEFAULT_INT:
	        fprintf(f, "%i", * (int *) defaults[i].location);
                break;

            case DEFAULT_FLOAT:
                fprintf(f, "%f", * (float *) defaults[i].location);
                break;

            case DEFAULT_STRING:
	        fprintf(f,"\"%s\"", * (char **) (defaults[i].location));
                break;
        }

        fprintf(f, "\n");
    }

    fclose (f);
}

// Parses integer values in the configuration file

static int ParseIntParameter(char *strparm)
{
    int parm;

    if (strparm[0] == '0' && strparm[1] == 'x')
        sscanf(strparm+2, "%x", &parm);
    else
        sscanf(strparm, "%i", &parm);

    return parm;
}

static void LoadDefaultCollection(default_collection_t *collection)
{
    default_t  *defaults = collection->defaults;
    int		i;
    FILE*	f;
    char	defname[80];
    char	strparm[100];

    // read the file in, overriding any set defaults
    f = fopen(collection->filename, "r");

    if (!f)
    {
        // File not opened, but don't complain

        return;
    }
    
    while (!feof(f))
    {
        if (fscanf (f, "%79s %[^\n]\n", defname, strparm) != 2)
        {
            // This line doesn't match
          
            continue;
        }

        // Strip off trailing non-printable characters (\r characters
        // from DOS text files)

        while (strlen(strparm) > 0 && !isprint(strparm[strlen(strparm)-1]))
        {
            strparm[strlen(strparm)-1] = '\0';
        }
        
        // Find the setting in the list
       
        for (i=0; i<collection->numdefaults; ++i)
        {
            default_t *def = &collection->defaults[i];
            char *s;
            int intparm;

            if (strcmp(defname, def->name) != 0)
            {
                // not this one
                continue;
            }

            // parameter found

            switch (def->type)
            {
                case DEFAULT_STRING:
                    s = strdup(strparm + 1);
                    s[strlen(s) - 1] = '\0';
                    * (char **) def->location = s;
                    break;

                case DEFAULT_INT:
                    * (int *) def->location = ParseIntParameter(strparm);
                    break;

                case DEFAULT_KEY:

                    // translate scancodes read from config
                    // file (save the old value in untranslated)

                    intparm = ParseIntParameter(strparm);
                    defaults[i].untranslated = intparm;
                    intparm = scantokey[intparm];

                    defaults[i].original_translated = intparm;
                    * (int *) def->location = intparm;
                    break;

                case DEFAULT_FLOAT:
                    * (float *) def->location = atof(strparm);
                    break;
            }

            // finish

            break; 
        }
    }
            
    fclose (f);
}

//
// M_SaveDefaults
//

void M_SaveDefaults (void)
{
    SaveDefaultCollection(&doom_defaults);
    SaveDefaultCollection(&extra_defaults);
}


//
// M_LoadDefaults
//

void M_LoadDefaults (void)
{
    int i;
 
    // check for a custom default file
    i = M_CheckParm ("-config");

    if (i && i<myargc-1)
    {
	doom_defaults.filename = myargv[i+1];
	printf ("	default file: %s\n",doom_defaults.filename);
    }
    else
    {
        doom_defaults.filename = malloc(strlen(configdir) + 20);
        sprintf(doom_defaults.filename, "%sdefault.cfg", configdir);
    }

//    printf("saving config in %s\n", doom_defaults.filename);

    i = M_CheckParm("-extraconfig");

    if (i && i<myargc-1)
    {
        extra_defaults.filename = myargv[i+1];
        printf("        extra configuration file: %s\n", 
               extra_defaults.filename);
    }
    else
    {
        extra_defaults.filename 
            = malloc(strlen(configdir) + strlen(PACKAGE_TARNAME) + 10);
        sprintf(extra_defaults.filename, "%s%s.cfg", 
                configdir, PACKAGE_TARNAME);
    }

    LoadDefaultCollection(&doom_defaults);
    LoadDefaultCollection(&extra_defaults);
}

// 
// Save normal (default.cfg) defaults to a given file
// 

void M_SaveMainDefaults(char *filename)
{
    char *main_filename;

    // Save the normal filename and set this one

    main_filename = doom_defaults.filename;
    doom_defaults.filename = filename;

    // Save the file

    SaveDefaultCollection(&doom_defaults);

    // Restore the normal filename

    doom_defaults.filename = main_filename;
}

// 
// Save extra (chocolate-doom.cfg) defaults to a given file
// 

void M_SaveExtraDefaults(char *filename)
{
    char *main_filename;

    // Save the normal filename and set this one

    main_filename = extra_defaults.filename;
    extra_defaults.filename = filename;

    // Save the file

    SaveDefaultCollection(&extra_defaults);

    // Restore the normal filename

    extra_defaults.filename = main_filename;
}

