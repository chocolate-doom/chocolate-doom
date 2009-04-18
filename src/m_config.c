// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 1993-2008 Raven Software
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
// DESCRIPTION:
//    Configuration file interface.
//
//-----------------------------------------------------------------------------


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#include "config.h"

#include "doomtype.h"
#include "doomkeys.h"
#include "doomfeatures.h"
#include "i_system.h"
#include "m_argv.h"
#include "m_misc.h"

#include "z_zone.h"

//
// DEFAULTS
//

// Location where all configuration data is stored - 
// default.cfg, savegames, etc.

char *configdir;

// Default filenames for configuration files.

static char *default_main_config;
static char *default_extra_config;

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
    char *name;

    // Pointer to the location in memory of the variable
    void *location;

    // Type of the variable
    default_type_t type;

    // If this is a key value, the original integer scancode we read from
    // the config file before translating it to the internal key value.
    // If zero, we didn't read this value from a config file.
    int untranslated;

    // The value we translated the scancode into when we read the 
    // config file on startup.  If the variable value is different from
    // this, it has been changed and needs to be converted; otherwise,
    // use the 'untranslated' value.
    int original_translated;

    // If true, this config variable has been bound to a variable
    // and is being used.
    boolean bound;
} default_t;

typedef struct
{
    default_t *defaults;
    int numdefaults;
    char *filename;
} default_collection_t;

#define CONFIG_VARIABLE_KEY(name) \
    { #name, NULL, DEFAULT_KEY, 0, 0, false }
#define CONFIG_VARIABLE_INT(name) \
    { #name, NULL, DEFAULT_INT, 0, 0, false }
#define CONFIG_VARIABLE_FLOAT(name) \
    { #name, NULL, DEFAULT_FLOAT, 0, 0, false }
#define CONFIG_VARIABLE_STRING(name) \
    { #name, NULL, DEFAULT_STRING, 0, 0, false }

//! @begin_config_file default.cfg

static default_t	doom_defaults_list[] =
{
    //! 
    // Mouse sensitivity.  This value is used to multiply input mouse
    // movement to control the effect of moving the mouse.
    //
    // The "normal" maximum value available for this through the 
    // in-game options menu is 9. A value of 31 or greater will cause
    // the game to crash when entering the options menu.
    //

    CONFIG_VARIABLE_INT(mouse_sensitivity),

    //!
    // Volume of sound effects, range 0-15.
    //

    CONFIG_VARIABLE_INT(sfx_volume),

    //!
    // Volume of in-game music, range 0-15.
    //

    CONFIG_VARIABLE_INT(music_volume),

    //!
    // If non-zero, messages are displayed on the heads-up display
    // in the game ("picked up a clip", etc).  If zero, these messages
    // are not displayed.
    //

    CONFIG_VARIABLE_INT(show_messages),

    //! 
    // Keyboard key to turn right.
    //

    CONFIG_VARIABLE_KEY(key_right),

    //!
    // Keyboard key to turn left.
    //

    CONFIG_VARIABLE_KEY(key_left),

    //!
    // Keyboard key to move forward.
    //

    CONFIG_VARIABLE_KEY(key_up),

    //!
    // Keyboard key to move backward.
    //

    CONFIG_VARIABLE_KEY(key_down),

    //!
    // Keyboard key to strafe left.
    //

    CONFIG_VARIABLE_KEY(key_strafeleft),

    //!
    // Keyboard key to strafe right.
    //

    CONFIG_VARIABLE_KEY(key_straferight),

    //!
    // Keyboard key to jump.
    //

    CONFIG_VARIABLE_KEY(key_jump),

    //!
    // Keyboard key to fly upward.
    //

    CONFIG_VARIABLE_KEY(key_flyup),

    //!
    // Keyboard key to fly downwards.
    //

    CONFIG_VARIABLE_KEY(key_flydown),
    
    //!
    // Keyboard key to center flying.
    //

    CONFIG_VARIABLE_KEY(key_flycenter),

    //!
    // Keyboard key to look up.
    //

    CONFIG_VARIABLE_KEY(key_lookup),

    //!
    // Keyboard key to look down.
    //

    CONFIG_VARIABLE_KEY(key_lookdown),

    //!
    // Keyboard key to center the view.
    //

    CONFIG_VARIABLE_KEY(key_lookcenter),

    //!
    // Keyboard key to scroll left in the inventory.
    //

    CONFIG_VARIABLE_KEY(key_invleft),

    //!
    // Keyboard key to scroll right in the inventory.
    //

    CONFIG_VARIABLE_KEY(key_invright),

    //!
    // Keyboard key to use the current item in the inventory.
    //

    CONFIG_VARIABLE_KEY(key_useartifact),

    //!
    // Keyboard key to fire the currently selected weapon.
    //

    CONFIG_VARIABLE_KEY(key_fire),

    //!
    // Keyboard key to "use" an object, eg. a door or switch.
    //

    CONFIG_VARIABLE_KEY(key_use),

    //!
    // Keyboard key to turn on strafing.  When held down, pressing the
    // key to turn left or right causes the player to strafe left or
    // right instead.
    //

    CONFIG_VARIABLE_KEY(key_strafe),

    //!
    // Keyboard key to make the player run.
    //

    CONFIG_VARIABLE_KEY(key_speed),

    //!
    // If non-zero, mouse input is enabled.  If zero, mouse input is
    // disabled.
    //

    CONFIG_VARIABLE_INT(use_mouse), 

    //!
    // Mouse button to fire the currently selected weapon.
    //

    CONFIG_VARIABLE_INT(mouseb_fire),

    //!
    // Mouse button to turn on strafing.  When held down, the player
    // will strafe left and right instead of turning left and right.
    //

    CONFIG_VARIABLE_INT(mouseb_strafe),

    //!
    // Mouse button to move forward.
    //

    CONFIG_VARIABLE_INT(mouseb_forward),

    //!
    // Mouse button to jump.
    //

    CONFIG_VARIABLE_INT(mouseb_jump),

    //!
    // If non-zero, joystick input is enabled.
    //

    CONFIG_VARIABLE_INT(use_joystick),

    //!
    // Joystick button to fire the current weapon.
    //

    CONFIG_VARIABLE_INT(joyb_fire),

    //!
    // Joystick button to fire the current weapon.
    //

    CONFIG_VARIABLE_INT(joyb_strafe),

    //!
    // Joystick button to "use" an object, eg. a door or switch.
    //

    CONFIG_VARIABLE_INT(joyb_use),

    //!
    // Joystick button to make the player run.
    //
    // If this has a value of 20 or greater, the player will always run.
    //

    CONFIG_VARIABLE_INT(joyb_speed),

    //!
    // Joystick button to jump.
    //

    CONFIG_VARIABLE_INT(joyb_jump),

    //!
    // Screen size, range 3-11.
    //
    // A value of 11 gives a full-screen view with the status bar not 
    // displayed.  A value of 10 gives a full-screen view with the
    // status bar displayed.
    //

    CONFIG_VARIABLE_INT(screenblocks),

    //!
    // Screen detail.  Zero gives normal "high detail" mode, while
    // a non-zero value gives "low detail" mode.
    //

    CONFIG_VARIABLE_INT(detaillevel),

    //!
    // Number of sounds that will be played simultaneously.
    //

    CONFIG_VARIABLE_INT(snd_channels),

    //!
    // Music output device.  A non-zero value gives MIDI sound output,
    // while a value of zero disables music.
    //

    CONFIG_VARIABLE_INT(snd_musicdevice),

    //!
    // Sound effects device.  A value of zero disables in-game sound 
    // effects, a value of 1 enables PC speaker sound effects, while 
    // a value in the range 2-9 enables the "normal" digital sound 
    // effects.
    //

    CONFIG_VARIABLE_INT(snd_sfxdevice),

    //!
    // SoundBlaster I/O port. Unused.
    //

    CONFIG_VARIABLE_INT(snd_sbport),

    //!
    // SoundBlaster IRQ.  Unused.
    //

    CONFIG_VARIABLE_INT(snd_sbirq),

    //!
    // SoundBlaster DMA channel.  Unused.
    //

    CONFIG_VARIABLE_INT(snd_sbdma), 

    //!
    // Output port to use for OPL MIDI playback.  Unused.
    //

    CONFIG_VARIABLE_INT(snd_mport),

    //!
    // Gamma correction level.  A value of zero disables gamma 
    // correction, while a value in the range 1-4 gives increasing
    // levels of gamma correction.
    //

    CONFIG_VARIABLE_INT(usegamma),

    //!
    // Directory in which to store savegames.
    //

    CONFIG_VARIABLE_STRING(savedir),

    //!
    // Controls whether messages are displayed in the heads-up display.
    // If this has a non-zero value, messages are displayed.
    //

    CONFIG_VARIABLE_INT(messageson),


    //!
    // Multiplayer chat macro: message to send when alt+0 is pressed.
    //

    CONFIG_VARIABLE_STRING(chatmacro0),

    //!
    // Multiplayer chat macro: message to send when alt+1 is pressed.
    //

    CONFIG_VARIABLE_STRING(chatmacro1),

    //!
    // Multiplayer chat macro: message to send when alt+2 is pressed.
    //

    CONFIG_VARIABLE_STRING(chatmacro2),

    //!
    // Multiplayer chat macro: message to send when alt+3 is pressed.
    //

    CONFIG_VARIABLE_STRING(chatmacro3),

    //!
    // Multiplayer chat macro: message to send when alt+4 is pressed.
    //

    CONFIG_VARIABLE_STRING(chatmacro4), 

    //!
    // Multiplayer chat macro: message to send when alt+5 is pressed.
    //

    CONFIG_VARIABLE_STRING(chatmacro5), 

    //!
    // Multiplayer chat macro: message to send when alt+6 is pressed.
    //

    CONFIG_VARIABLE_STRING(chatmacro6),

    //!
    // Multiplayer chat macro: message to send when alt+7 is pressed.
    //

    CONFIG_VARIABLE_STRING(chatmacro7),

    //!
    // Multiplayer chat macro: message to send when alt+8 is pressed.
    //

    CONFIG_VARIABLE_STRING(chatmacro8),

    //!
    // Multiplayer chat macro: message to send when alt+9 is pressed.
    //

    CONFIG_VARIABLE_STRING(chatmacro9),
};

static default_collection_t doom_defaults = 
{
    doom_defaults_list,
    arrlen(doom_defaults_list),
    NULL,
};

//! @begin_config_file chocolate-doom.cfg

static default_t extra_defaults_list[] = 
{
    //!
    // If non-zero, use the graphical startup mode for Heretic and
    // Hexen.
    //

    CONFIG_VARIABLE_INT(graphical_startup),

    //!
    // If non-zero, video settings will be autoadjusted to a valid 
    // configuration when the screen_width and screen_height variables
    // do not match any valid configuration.
    //

    CONFIG_VARIABLE_INT(autoadjust_video_settings), 

    //!
    // If non-zero, the game will run in full screen mode.  If zero,
    // the game will run in a window.
    //

    CONFIG_VARIABLE_INT(fullscreen), 

    //!
    // If non-zero, the screen will be stretched vertically to display
    // correctly on a square pixel video mode.
    //

    CONFIG_VARIABLE_INT(aspect_ratio_correct),

    //!
    // Number of milliseconds to wait on startup after the video mode
    // has been set, before the game will start.  This allows the 
    // screen to settle on some monitors that do not display an image 
    // for a brief interval after changing video modes.
    //

    CONFIG_VARIABLE_INT(startup_delay), 

    //!
    // Screen width in pixels.  If running in full screen mode, this is
    // the X dimension of the video mode to use.  If running in
    // windowed mode, this is the width of the window in which the game
    // will run.
    //

    CONFIG_VARIABLE_INT(screen_width),

    //!
    // Screen height in pixels.  If running in full screen mode, this is
    // the Y dimension of the video mode to use.  If running in
    // windowed mode, this is the height of the window in which the game
    // will run.
    //

    CONFIG_VARIABLE_INT(screen_height), 

    //!
    // If this is non-zero, the mouse will be "grabbed" when running
    // in windowed mode so that it can be used as an input device.
    // When running full screen, this has no effect.
    //

    CONFIG_VARIABLE_INT(grabmouse),

    //!
    // If non-zero, all vertical mouse movement is ignored.  This 
    // emulates the behavior of the "novert" tool available under DOS
    // that performs the same function.
    //

    CONFIG_VARIABLE_INT(novert),

    //!
    // Mouse acceleration factor.  When the speed of mouse movement
    // exceeds the threshold value (mouse_threshold), the speed is
    // multiplied by this value.
    //

    CONFIG_VARIABLE_FLOAT(mouse_acceleration), 

    //!
    // Mouse acceleration threshold.  When the speed of mouse movement
    // exceeds this threshold value, the speed is multiplied by an 
    // acceleration factor (mouse_acceleration).
    //

    CONFIG_VARIABLE_INT(mouse_threshold),

    //!
    // Sound output sample rate, in Hz.  Typical values to use are 
    // 11025, 22050, 44100 and 48000.
    //

    CONFIG_VARIABLE_INT(snd_samplerate),

    //!
    // If non-zero, the ENDOOM screen is displayed when exiting the
    // game.  If zero, the ENDOOM screen is not displayed.
    //

    CONFIG_VARIABLE_INT(show_endoom),

    //!
    // If non-zero, the Vanilla savegame limit is enforced; if the 
    // savegame exceeds 180224 bytes in size, the game will exit with
    // an error.  If this has a value of zero, there is no limit to
    // the size of savegames.
    //

    CONFIG_VARIABLE_INT(vanilla_savegame_limit),

    //!
    // If non-zero, the Vanilla demo size limit is enforced; the game 
    // exits with an error when a demo exceeds the demo size limit
    // (128KiB by default).  If this has a value of zero, there is no
    // limit to the size of demos.
    //

    CONFIG_VARIABLE_INT(vanilla_demo_limit),

    //!
    // If non-zero, the game behaves like Vanilla Doom, always assuming
    // an American keyboard mapping.  If this has a value of zero, the 
    // native keyboard mapping of the keyboard is used.
    //

    CONFIG_VARIABLE_INT(vanilla_keyboard_mapping),

    //!
    // Name of the SDL video driver to use.  If this is an empty string,
    // the default video driver is used.
    //

    CONFIG_VARIABLE_STRING(video_driver), 

#ifdef FEATURE_MULTIPLAYER

    //!
    // Name to use in network games for identification.  This is only 
    // used on the "waiting" screen while waiting for the game to start.
    //

    CONFIG_VARIABLE_STRING(player_name),

#endif

    //!
    // Joystick number to use; '0' is the first joystick.  A negative
    // value ('-1') indicates that no joystick is configured.
    //

    CONFIG_VARIABLE_INT(joystick_index),

    //!
    // Joystick axis to use to for horizontal (X) movement.
    //

    CONFIG_VARIABLE_INT(joystick_x_axis),

    //!
    // If non-zero, movement on the horizontal joystick axis is inverted.
    //

    CONFIG_VARIABLE_INT(joystick_x_invert),

    //!
    // Joystick axis to use to for vertical (Y) movement.
    //

    CONFIG_VARIABLE_INT(joystick_y_axis),

    //!
    // If non-zero, movement on the vertical joystick axis is inverted.
    //

    CONFIG_VARIABLE_INT(joystick_y_invert), 

    //!
    // Joystick button to strafe left.
    //

    CONFIG_VARIABLE_INT(joyb_strafeleft),

    //!
    // Joystick button to strafe right.
    //

    CONFIG_VARIABLE_INT(joyb_straferight),

    //!
    // Mouse button to strafe left.
    //

    CONFIG_VARIABLE_INT(mouseb_strafeleft),

    //!
    // Mouse button to strafe right.
    //

    CONFIG_VARIABLE_INT(mouseb_straferight),

    //!
    // Mouse button to "use" an object, eg. a door or switch.
    //

    CONFIG_VARIABLE_INT(mouseb_use),

    //!
    // Mouse button to move backwards.
    //

    CONFIG_VARIABLE_INT(mouseb_backward),

    //!
    // If non-zero, double-clicking a mouse button acts like pressing
    // the "use" key to use an object in-game, eg. a door or switch.
    //

    CONFIG_VARIABLE_INT(dclick_use),

    //!
    // Controls whether libsamplerate support is used for performing
    // sample rate conversions of sound effects.  Support for this
    // must be compiled into the program.
    //
    // If zero, libsamplerate support is disabled.  If non-zero, 
    // libsamplerate is enabled. Increasing values roughly correspond
    // to higher quality conversion; the higher the quality, the 
    // slower the conversion process.  Linear conversion = 1; 
    // Zero order hold = 2; Fast Sinc filter = 3; Medium quality
    // Sinc filter = 4; High quality Sinc filter = 5.
    //

    CONFIG_VARIABLE_INT(use_libsamplerate),
};

static default_collection_t extra_defaults =
{
    extra_defaults_list,
    arrlen(extra_defaults_list),
    NULL,
};

// Search a collection for a variable

static default_t *SearchCollection(default_collection_t *collection, char *name)
{
    int i;

    for (i=0; i<collection->numdefaults; ++i) 
    {
        if (!strcmp(name, collection->defaults[i].name))
        {
            return &collection->defaults[i];
        }
    }

    return NULL;
}

static const int scantokey[128] =
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
    KEY_UPARROW,KEY_PGUP,KEY_MINUS,KEY_LEFTARROW,KEYP_5,KEY_RIGHTARROW,KEYP_PLUS,KEY_END,
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

        // Ignore unbound variables

        if (!defaults[i].bound)
        {
            continue;
        }

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
    default_t *def;
    FILE *f;
    char defname[80];
    char strparm[100];
    char *s;
    int intparm;

    // read the file in, overriding any set defaults
    f = fopen(collection->filename, "r");

    if (f == NULL)
    {
        // File not opened, but don't complain. 
        // It's probably just the first time they ran the game.

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
       
        def = SearchCollection(collection, defname);

        if (def == NULL || !def->bound)
        {
            // Unknown variable?  Unbound variables are also treated
            // as unknown.

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
                def->untranslated = intparm;
                intparm = scantokey[intparm];

                def->original_translated = intparm;
                * (int *) def->location = intparm;
                break;

            case DEFAULT_FLOAT:
                * (float *) def->location = (float) atof(strparm);
                break;
        }
    }
            
    fclose (f);
}

// Set the default filenames to use for configuration files.

void M_SetConfigFilenames(char *main_config, char *extra_config)
{
    default_main_config = main_config;
    default_extra_config = extra_config;
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
// Save defaults to alternate filenames
//

void M_SaveDefaultsAlternate(char *main, char *extra)
{
    char *orig_main;
    char *orig_extra;

    // Temporarily change the filenames

    orig_main = doom_defaults.filename;
    orig_extra = extra_defaults.filename;

    doom_defaults.filename = main;
    extra_defaults.filename = extra;

    M_SaveDefaults();

    // Restore normal filenames

    doom_defaults.filename = orig_main;
    extra_defaults.filename = orig_extra;
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
    // @vanilla
    //
    // Load configuration from the specified file.  The default 
    // configuration file (for Doom) is named default.cfg.
    //

    i = M_CheckParm ("-config");

    if (i && i<myargc-1)
    {
	doom_defaults.filename = myargv[i+1];
	printf ("	default file: %s\n",doom_defaults.filename);
    }
    else
    {
        doom_defaults.filename
            = malloc(strlen(configdir) + strlen(default_main_config) + 1);
        sprintf(doom_defaults.filename, "%s%s", configdir, default_main_config);
    }

    printf("saving config in %s\n", doom_defaults.filename);

    //!
    // @arg <file>
    //
    // Load extra configuration from the specified file.  The default
    // configuration file for Doom is named chocolate-doom.cfg.
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
            = malloc(strlen(configdir) + strlen(default_extra_config) + 1);
        sprintf(extra_defaults.filename, "%s%s", 
                configdir, default_extra_config);
    }

    LoadDefaultCollection(&doom_defaults);
    LoadDefaultCollection(&extra_defaults);
}

// Get a configuration file variable by its name

static default_t *GetDefaultForName(char *name)
{
    default_t *result;

    // Try the main list and the extras

    result = SearchCollection(&doom_defaults, name);

    if (result == NULL)
    {
        result = SearchCollection(&extra_defaults, name);
    }

    // Not found? Internal error.

    if (result == NULL)
    {
        I_Error("Unknown configuration variable: '%s'", name);
    }

    return result;
}

//
// Bind a variable to a given configuration file variable, by name.
//

void M_BindVariable(char *name, void *location)
{
    default_t *variable;

    variable = GetDefaultForName(name);

    variable->location = location;
    variable->bound = true;
}

// Get the path to the default configuration dir to use, if NULL
// is passed to M_SetConfigDir.

static char *GetDefaultConfigDir(void)
{
#ifndef _WIN32
    // On Unix systems we put configuration into ~/.chocolate-doom, but
    // on Windows we just use the current directory, like Vanilla.

    char *homedir;
    char *result;

    homedir = getenv("HOME");

    if (homedir != NULL)
    {
        // put all configuration in a config directory off the
        // homedir

        result = malloc(strlen(homedir) + strlen(PACKAGE_TARNAME) + 5);

        sprintf(result, "%s%c.%s%c", homedir, DIR_SEPARATOR,
                                     PACKAGE_TARNAME, DIR_SEPARATOR);

        return result;
    }
    else
#endif /* #ifndef _WIN32 */
    {
        // On Windows, we just use the current directory.

        return strdup("");
    }
}

// 
// SetConfigDir:
//
// Sets the location of the configuration directory, where configuration
// files are stored - default.cfg, chocolate-doom.cfg, savegames, etc.
//

void M_SetConfigDir(char *dir)
{
    // Use the directory that was passed, or find the default.

    if (dir != NULL)
    {
        configdir = dir;
    }
    else
    {
        configdir = GetDefaultConfigDir();
    }

    printf("Using %s for configuration and saves\n", configdir);

    // Make the directory if it doesn't already exist:

    M_MakeDirectory(configdir);
}

