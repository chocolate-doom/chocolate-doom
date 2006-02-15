// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: m_misc.c 368 2006-02-15 12:58:27Z fraggle $
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
// $Log$
// Revision 1.17.2.3  2006/02/15 12:58:27  fraggle
// Remove the savegame buffer entirely.  Keep the old savegame size limit bug add a "vanilla_savegame_limit" config file option which allows
// the limit to be disabled if necessary.
//
// Revision 1.17.2.2  2006/01/20 19:46:14  fraggle
// Fix crash due to buffer not allocated large enough
//
// Revision 1.17.2.1  2006/01/20 00:58:17  fraggle
// Remove new networking code from stable version
//
// Revision 1.17  2006/01/10 22:14:13  fraggle
// Shut up compiler warnings
//
// Revision 1.16  2006/01/09 01:50:51  fraggle
// Deduce a sane player name by examining environment variables.  Add
// a "player_name" setting to chocolate-doom.cfg.  Transmit the name
// to the server and use the names players send in the waiting data list.
//
// Revision 1.15  2006/01/08 18:22:39  fraggle
// Strip carriage returns from the end of lines when reading configuration
// files.
//
// Revision 1.14  2006/01/08 18:13:33  fraggle
// show_endoom config file option to disable the endoom screen
//
// Revision 1.13  2005/10/16 01:18:10  fraggle
// Global "configdir" variable with directory to store config files in.
// Create a function to find the filename for a savegame slot.  Store
// savegames in the config dir.
//
// Revision 1.12  2005/09/17 20:50:46  fraggle
// Mouse acceleration code to emulate old DOS drivers
//
// Revision 1.11  2005/09/17 20:25:56  fraggle
// Set the default values for variables in their initialisers.  Remove the
// "defaultvalue" parameter and associated code from the configuration
// file parsing code.
//
// Revision 1.10  2005/09/17 20:06:45  fraggle
// Rewrite configuration loading code; assign a type to each configuration
// parameter.  Allow float parameters, align all values in the configuration
// files
//
// Revision 1.9  2005/09/11 20:25:56  fraggle
// Second configuration file to allow chocolate doom-specific settings.
// Adjust some existing command line logic (for graphics settings and
// novert) to adjust for this.
//
// Revision 1.8  2005/09/07 21:40:11  fraggle
// Remove non-ANSI C headers and functions
//
// Revision 1.7  2005/09/07 12:34:47  fraggle
// Maintain dos-specific options in config file
//
// Revision 1.6  2005/08/04 21:48:32  fraggle
// Turn on compiler optimisation and warning options
// Add SDL_mixer sound code
//
// Revision 1.5  2005/08/04 18:42:15  fraggle
// Silence compiler warnings
//
// Revision 1.4  2005/07/24 02:14:04  fraggle
// Move to SDL for graphics.
// Translate key scancodes to correct internal format when reading
// settings from config file - backwards compatible with config files
// for original exes
//
// Revision 1.3  2005/07/23 19:17:11  fraggle
// Use ANSI-standard limit constants.  Remove LINUX define.
//
// Revision 1.2  2005/07/23 16:44:55  fraggle
// Update copyright to GNU GPL
//
// Revision 1.1.1.1  2005/07/23 16:19:53  fraggle
// Initial import
//
//
// DESCRIPTION:
//	Main loop menu stuff.
//	Default Config File.
//	PCX Screenshots.
//
//-----------------------------------------------------------------------------

static const char
rcsid[] = "$Id: m_misc.c 368 2006-02-15 12:58:27Z fraggle $";

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

// for mkdir:

#ifdef _WIN32
#include <io.h>
#else
#include <sys/stat.h>
#include <sys/types.h>
#endif

#include "config.h"
#include "doomdef.h"

#include "z_zone.h"

#include "m_swap.h"
#include "m_argv.h"

#include "w_wad.h"

#include "i_system.h"
#include "i_video.h"
#include "v_video.h"

#include "hu_stuff.h"

// State.
#include "doomstat.h"

// Data.
#include "dstrings.h"

#include "m_misc.h"

//
// M_DrawText
// Returns the final X coordinate
// HU_Init must have been called to init the font
//
extern patch_t*		hu_font[HU_FONTSIZE];

int
M_DrawText
( int		x,
  int		y,
  boolean	direct,
  char*		string )
{
    int 	c;
    int		w;

    while (*string)
    {
	c = toupper(*string) - HU_FONTSTART;
	string++;
	if (c < 0 || c> HU_FONTSIZE)
	{
	    x += 4;
	    continue;
	}
		
	w = SHORT (hu_font[c]->width);
	if (x+w > SCREENWIDTH)
	    break;
	if (direct)
	    V_DrawPatchDirect(x, y, 0, hu_font[c]);
	else
	    V_DrawPatch(x, y, 0, hu_font[c]);
	x+=w;
    }

    return x;
}




//
// M_WriteFile
//
boolean M_WriteFile(char const *name, void *source, int	length)
{
    FILE *handle;
    int	count;
	
    handle = fopen(name, "wb");

    if (handle == NULL)
	return false;

    count = fwrite(source, 1, length, handle);
    fclose(handle);
	
    if (count < length)
	return false;
		
    return true;
}


//
// M_ReadFile
//
int M_ReadFile(char const *name, byte **buffer)
{
    FILE *handle;
    int	count, length;
    byte *buf;
	
    handle = fopen(name, "rb");
    if (handle == NULL)
	I_Error ("Couldn't read file %s", name);

    // find the size of the file by seeking to the end and
    // reading the current position

    fseek(handle, 0, SEEK_END);
    length = ftell(handle);
    fseek(handle, 0, SEEK_SET);
    
    buf = Z_Malloc (length, PU_STATIC, NULL);
    count = fread(buf, 1, length, handle);
    fclose (handle);
	
    if (count < length)
	I_Error ("Couldn't read file %s", name);
		
    *buffer = buf;
    return length;
}


//
// DEFAULTS
//

// locations of config files

int		usemouse = 1;
int		usejoystick = 0;

extern int	key_right;
extern int	key_left;
extern int	key_up;
extern int	key_down;

extern int	key_strafeleft;
extern int	key_straferight;

extern int	key_fire;
extern int	key_use;
extern int	key_strafe;
extern int	key_speed;

extern int	mousebfire;
extern int	mousebstrafe;
extern int	mousebforward;

extern int	joybfire;
extern int	joybstrafe;
extern int	joybuse;
extern int	joybspeed;

extern int	viewwidth;
extern int	viewheight;

extern int	mouseSensitivity;
extern int	showMessages;

extern int	detailLevel;

extern int	screenblocks;

extern int	showMessages;

// machine-independent sound params
extern	int	numChannels;


extern char*	chat_macros[];

extern int      show_endoom;
extern int      vanilla_savegame_limit;

// dos specific options: these are unused but should be maintained
// so that the config file can be shared between chocolate
// doom and doom.exe

static int snd_musicdevice = 0;
static int snd_sfxdevice = 0;
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
    char *         name;
    void *         location;
    default_type_t type;
    int            untranslated;
} default_t;

typedef struct
{
    default_t *defaults;
    int        numdefaults;
    char      *filename;
} default_collection_t;

static default_t	doom_defaults_list[] =
{
    {"mouse_sensitivity", &mouseSensitivity},
    {"sfx_volume",&snd_SfxVolume},
    {"music_volume",&snd_MusicVolume},
    {"show_messages",&showMessages},

    {"key_right",&key_right, DEFAULT_KEY},
    {"key_left",&key_left, DEFAULT_KEY},
    {"key_up",&key_up, DEFAULT_KEY},
    {"key_down",&key_down, DEFAULT_KEY},
    {"key_strafeleft",&key_strafeleft, DEFAULT_KEY},
    {"key_straferight",&key_straferight, DEFAULT_KEY},

    {"key_fire",&key_fire, DEFAULT_KEY},
    {"key_use",&key_use, DEFAULT_KEY},
    {"key_strafe",&key_strafe, DEFAULT_KEY},
    {"key_speed",&key_speed, DEFAULT_KEY},

    {"use_mouse",&usemouse},
    {"mouseb_fire",&mousebfire},
    {"mouseb_strafe",&mousebstrafe},
    {"mouseb_forward",&mousebforward},

    {"use_joystick",&usejoystick},
    {"joyb_fire",&joybfire},
    {"joyb_strafe",&joybstrafe},
    {"joyb_use",&joybuse},
    {"joyb_speed",&joybspeed},

    {"screenblocks",&screenblocks},
    {"detaillevel",&detailLevel},

    {"snd_channels",&numChannels},

    {"snd_musicdevice", &snd_musicdevice},
    {"snd_sfxdevice", &snd_sfxdevice},
    {"snd_sbport", &snd_sbport},
    {"snd_sbirq", &snd_sbirq},
    {"snd_sbdma", &snd_sbdma},
    {"snd_mport", &snd_mport},

    {"usegamma",&usegamma},

    {"chatmacro0", &chat_macros[0], DEFAULT_STRING },
    {"chatmacro1", &chat_macros[1], DEFAULT_STRING },
    {"chatmacro2", &chat_macros[2], DEFAULT_STRING },
    {"chatmacro3", &chat_macros[3], DEFAULT_STRING },
    {"chatmacro4", &chat_macros[4], DEFAULT_STRING },
    {"chatmacro5", &chat_macros[5], DEFAULT_STRING },
    {"chatmacro6", &chat_macros[6], DEFAULT_STRING },
    {"chatmacro7", &chat_macros[7], DEFAULT_STRING },
    {"chatmacro8", &chat_macros[8], DEFAULT_STRING },
    {"chatmacro9", &chat_macros[9], DEFAULT_STRING },
};

static default_collection_t doom_defaults = 
{
    doom_defaults_list,
    sizeof(doom_defaults_list) / sizeof(*doom_defaults_list),
};

static default_t extra_defaults_list[] = 
{
    {"grabmouse",              &grabmouse},
    {"fullscreen",             &fullscreen},
    {"screenmultiply",         &screenmultiply},
    {"novert",                 &novert},
    {"mouse_acceleration",     &mouse_acceleration,   DEFAULT_FLOAT},
    {"show_endoom",            &show_endoom},
    {"vanilla_savegame_limit", &vanilla_savegame_limit},
};

static default_collection_t extra_defaults =
{
    extra_defaults_list,
    sizeof(extra_defaults_list) / sizeof(*extra_defaults_list),
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
    KEYP_UPARROW,KEY_PGUP,KEYP_MINUS,KEYP_LEFTARROW,KEYP_5,KEYP_RIGHTARROW,KEYP_PLUS,KEY_END,
    KEYP_DOWNARROW,KEY_PGDN,KEY_INS,KEY_DEL,0,   0,      0,      KEY_F11,
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

                if (defaults[i].untranslated)
                {
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

    printf("saving config in %s\n", doom_defaults.filename);

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
// SCREEN SHOTS
//


typedef struct
{
    char		manufacturer;
    char		version;
    char		encoding;
    char		bits_per_pixel;

    unsigned short	xmin;
    unsigned short	ymin;
    unsigned short	xmax;
    unsigned short	ymax;
    
    unsigned short	hres;
    unsigned short	vres;

    unsigned char	palette[48];
    
    char		reserved;
    char		color_planes;
    unsigned short	bytes_per_line;
    unsigned short	palette_type;
    
    char		filler[58];
    unsigned char	data;		// unbounded
} pcx_t;


//
// WritePCXfile
//
void
WritePCXfile
( char*		filename,
  byte*		data,
  int		width,
  int		height,
  byte*		palette )
{
    int		i;
    int		length;
    pcx_t*	pcx;
    byte*	pack;
	
    pcx = Z_Malloc (width*height*2+1000, PU_STATIC, NULL);

    pcx->manufacturer = 0x0a;		// PCX id
    pcx->version = 5;			// 256 color
    pcx->encoding = 1;			// uncompressed
    pcx->bits_per_pixel = 8;		// 256 color
    pcx->xmin = 0;
    pcx->ymin = 0;
    pcx->xmax = SHORT(width-1);
    pcx->ymax = SHORT(height-1);
    pcx->hres = SHORT(width);
    pcx->vres = SHORT(height);
    memset (pcx->palette,0,sizeof(pcx->palette));
    pcx->color_planes = 1;		// chunky image
    pcx->bytes_per_line = SHORT(width);
    pcx->palette_type = SHORT(2);	// not a grey scale
    memset (pcx->filler,0,sizeof(pcx->filler));


    // pack the image
    pack = &pcx->data;
	
    for (i=0 ; i<width*height ; i++)
    {
	if ( (*data & 0xc0) != 0xc0)
	    *pack++ = *data++;
	else
	{
	    *pack++ = 0xc1;
	    *pack++ = *data++;
	}
    }
    
    // write the palette
    *pack++ = 0x0c;	// palette ID byte
    for (i=0 ; i<768 ; i++)
	*pack++ = *palette++;
    
    // write output file
    length = pack - (byte *)pcx;
    M_WriteFile (filename, pcx, length);

    Z_Free (pcx);
}

static boolean FileExists(char *filename)
{
    FILE *handle;

    handle = fopen(filename, "rb");

    if (handle != NULL)
    {
        fclose(handle);
        return true;
    }
    else
    {
        return false;
    }
}

//
// M_ScreenShot
//
void M_ScreenShot (void)
{
    int		i;
    byte*	linear;
    char	lbmname[12];
    
    // munge planar buffer to linear
    linear = screens[2];
    I_ReadScreen (linear);
    
    // find a file name to save it to
    strcpy(lbmname,"DOOM00.pcx");
		
    for (i=0 ; i<=99 ; i++)
    {
	lbmname[4] = i/10 + '0';
	lbmname[5] = i%10 + '0';
	if (!FileExists(lbmname))
	    break;	// file doesn't exist
    }
    if (i==100)
	I_Error ("M_ScreenShot: Couldn't create a PCX");
    
    // save the pcx file
    WritePCXfile (lbmname, linear,
		  SCREENWIDTH, SCREENHEIGHT,
		  W_CacheLumpName ("PLAYPAL",PU_CACHE));
	
    players[consoleplayer].message = "screen shot";
}


