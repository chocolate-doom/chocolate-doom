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
//	Main loop menu stuff.
//	Default Config File.
//	PCX Screenshots.
//
//-----------------------------------------------------------------------------


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
#include "deh_main.h"
#include "doomdef.h"
#include "doomfeatures.h"

#include "z_zone.h"

#include "m_menu.h"
#include "m_argv.h"
#include "net_client.h"

#include "w_wad.h"

#include "i_joystick.h"
#include "i_swap.h"
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

// Check if a file exists

boolean M_FileExists(char *filename)
{
    FILE *fstream;

    fstream = fopen(filename, "r");

    if (fstream != NULL)
    {
        fclose(fstream);
        return true;
    }
    else
    {
        return false;
    }
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

// Returns the path to a temporary file of the given name, stored
// inside the system temporary directory.
//
// The returned value must be freed with Z_Free after use.

char *M_TempFile(char *s)
{
    char *result;
    char *tempdir;

#ifdef _WIN32

    // Check the TEMP environment variable to find the location.

    tempdir = getenv("TEMP");

    if (tempdir == NULL)
    {
        tempdir = ".";
    }
#else
    // In Unix, just use /tmp.

    tempdir = "/tmp";
#endif

    result = Z_Malloc(strlen(tempdir) + strlen(s) + 2, PU_STATIC, 0);
    sprintf(result, "%s%c%s", tempdir, DIR_SEPARATOR, s);

    return result;
}

//
// DEFAULTS
//

// Location where all configuration data is stored - 
// default.cfg, savegames, etc.

char *          configdir;


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

// machine-independent sound params
extern	int	numChannels;


extern char*	chat_macros[];

extern int      show_endoom;
extern int      vanilla_savegame_limit;
extern int      vanilla_demo_limit;

extern int snd_musicdevice;
extern int snd_sfxdevice;

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

static default_t	doom_defaults_list[] =
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
    sizeof(doom_defaults_list) / sizeof(*doom_defaults_list),
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
    {"show_endoom",                 &show_endoom, DEFAULT_INT, 0, 0},
    {"vanilla_savegame_limit",      &vanilla_savegame_limit, DEFAULT_INT, 0, 0},
    {"vanilla_demo_limit",          &vanilla_demo_limit, DEFAULT_INT, 0, 0},
    {"vanilla_keyboard_mapping",    &vanilla_keyboard_mapping, DEFAULT_INT, 0, 0},
    {"video_driver",                &video_driver, DEFAULT_STRING, 0, 0},
#ifdef FEATURE_MULTIPLAYER
    {"player_name",                 &net_player_name,          DEFAULT_STRING, 0, 0},
#endif

    {"joystick_index",              &joystick_index, DEFAULT_INT, 0, 0},
    {"joystick_x_axis",             &joystick_x_axis, DEFAULT_INT, 0, 0},
    {"joystick_x_invert",           &joystick_x_invert, DEFAULT_INT, 0, 0},
    {"joystick_y_axis",             &joystick_y_axis, DEFAULT_INT, 0, 0},
    {"joystick_y_invert",           &joystick_y_invert, DEFAULT_INT, 0, 0},
};

static default_collection_t extra_defaults =
{
    extra_defaults_list,
    sizeof(extra_defaults_list) / sizeof(*extra_defaults_list),
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

    //!
    // @arg <file>
    //
    // Load configuration from the specified file, instead of
    // default.cfg.
    //

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

    //!
    // Load extra configuration from the specified file, instead 
    // of chocolate-doom.cfg.
    //

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

        sprintf(configdir, "%s%c.%s%c", homedir, DIR_SEPARATOR,
			                PACKAGE_TARNAME, DIR_SEPARATOR);

        // make the directory if it doesnt already exist

        M_MakeDirectory(configdir);
    }
    else
#endif /* #ifndef _WIN32 */
    {
#ifdef _WIN32
        //!
        // @platform windows
        //
        // Save configuration data and savegames in c:\doomdata,
        // allowing play from CD.
        //

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
		  W_CacheLumpName (DEH_String("PLAYPAL"),PU_CACHE));
	
    players[consoleplayer].message = DEH_String("screen shot");
}


