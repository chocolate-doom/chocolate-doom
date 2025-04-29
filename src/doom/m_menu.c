//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2005-2014 Simon Howard
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
// DESCRIPTION:
//	DOOM selection menu, options, episode etc.
//	Sliders and icons. Kinda widget stuff.
//


#include <stdlib.h>
#include <ctype.h>
#include <time.h> // [crispy] strftime, localtime


#include "doomdef.h"
#include "doomkeys.h"
#include "dstrings.h"

#include "d_main.h"
#include "deh_main.h"

#include "i_input.h"
#include "i_joystick.h"
#include "i_swap.h"
#include "i_system.h"
#include "i_timer.h"
#include "i_video.h"
#include "m_misc.h"
#include "v_video.h"
#include "w_wad.h"
#include "z_zone.h"

#include "r_local.h"


#include "hu_stuff.h"

#include "g_game.h"

#include "m_argv.h"
#include "m_controls.h"
#include "p_saveg.h"
#include "p_setup.h"
#include "p_extsaveg.h" // [crispy] savewadfilename

#include "s_sound.h"

#include "doomstat.h"

// Data.
#include "sounds.h"

#include "m_menu.h"
#include "m_crispy.h" // [crispy] Crispness menu

#include "v_trans.h" // [crispy] colored "invert mouse" message

//
// defaulted values
//
int			mouseSensitivity = 5;
int			mouseSensitivity_x2 = 5; // [crispy] mouse sensitivity menu
int			mouseSensitivity_y = 5; // [crispy] mouse sensitivity menu

// Show messages has default, 0 = off, 1 = on
int			showMessages = 1;
	

// Blocky mode, has default, 0 = high, 1 = normal
int			detailLevel = 0;
int			screenblocks = 10; // [crispy] increased

// temp for screenblocks (0-9)
int			screenSize;

// -1 = no quicksave slot picked!
int			quickSaveSlot;

 // 1 = message to be printed
int			messageToPrint;
// ...and here is the message string!
const char		*messageString;

// message x & y
int			messx;
int			messy;
int			messageLastMenuActive;

// timed message = no input from user
boolean			messageNeedsInput;

void    (*messageRoutine)(int response);

// [crispy] intermediate gamma levels
char gammamsg[5+13][26+2] =
{
    GAMMALVL050,
    GAMMALVL055,
    GAMMALVL060,
    GAMMALVL065,
    GAMMALVL070,
    GAMMALVL075,
    GAMMALVL080,
    GAMMALVL085,
    GAMMALVL090,
    GAMMALVL0,
    GAMMALVL05,
    GAMMALVL1,
    GAMMALVL15,
    GAMMALVL2,
    GAMMALVL25,
    GAMMALVL3,
    GAMMALVL35,
    GAMMALVL4
};

// we are going to be entering a savegame string
int			saveStringEnter;              
int             	saveSlot;	// which slot to save in
int			saveCharIndex;	// which char we're editing
static boolean          joypadSave = false; // was the save action initiated by joypad?
// old save description before edit
char			saveOldString[SAVESTRINGSIZE];  

// [crispy] for entering numeric values
#define NUMERIC_ENTRY_NUMDIGITS 3
boolean numeric_enter;
int numeric_entry;
static char numeric_entry_str[NUMERIC_ENTRY_NUMDIGITS + 1];
static int numeric_entry_index;

boolean			inhelpscreens;
boolean			menuactive;

#define SKULLXOFF		-32
#define LINEHEIGHT		16
#define CRISPY_LINEHEIGHT	10 // [crispy] Crispness menu

char			savegamestrings[10][SAVESTRINGSIZE];

// [FG] support up to 8 pages of savegames
int savepage = 0;
static const int savepage_max = 7;

char	endstring[160];

static boolean opldev;

extern boolean speedkeydown (void);

//
// MENU TYPEDEFS
//
typedef struct
{
    // 0 = no cursor here, 1 = ok, 2 = arrows ok
    // [crispy] 3 = arrows ok, no mouse x
    // [crispy] 4 = arrows ok, enter for numeric entry, no mouse x
    short	status;
    
    char	name[10];
    
    // choice = menu item #.
    // if status = 2 or 3,
    //   choice=0:leftarrow,1:rightarrow
    // [crispy] if status = 4,
    //   choice=0:leftarrow,1:rightarrow,2:enter
    void	(*routine)(int choice);
    
    // hotkey in menu
    char	alphaKey;			
    const char	*alttext; // [crispy] alternative text for menu items
} menuitem_t;



typedef struct menu_s
{
    short		numitems;	// # of menu items
    struct menu_s*	prevMenu;	// previous menu
    menuitem_t*		menuitems;	// menu items
    void		(*routine)();	// draw routine
    short		x;
    short		y;		// x,y of menu
    short		lastOn;		// last item user was on in menu
    short		lumps_missing;	// [crispy] indicate missing menu graphics lumps
} menu_t;

short		itemOn;			// menu item skull is on
short		skullAnimCounter;	// skull animation counter
short		whichSkull;		// which skull to draw

// graphic name of skulls
// warning: initializer-string for array of chars is too long
const char *skullName[2] = {"M_SKULL1","M_SKULL2"};

// current menudef
menu_t*	currentMenu;                          

//
// PROTOTYPES
//
static void M_NewGame(int choice);
static void M_Episode(int choice);
static void M_ChooseSkill(int choice);
static void M_LoadGame(int choice);
static void M_SaveGame(int choice);
static void M_Options(int choice);
static void M_EndGame(int choice);
static void M_ReadThis(int choice);
static void M_ReadThis2(int choice);
static void M_QuitDOOM(int choice);

static void M_ChangeMessages(int choice);
static void M_ChangeSensitivity(int choice);
static void M_ChangeSensitivity_x2(int choice); // [crispy] mouse sensitivity menu
static void M_ChangeSensitivity_y(int choice); // [crispy] mouse sensitivity menu
static void M_MouseInvert(int choice); // [crispy] mouse sensitivity menu
static void M_SfxVol(int choice);
static void M_MusicVol(int choice);
static void M_ChangeDetail(int choice);
static void M_SizeDisplay(int choice);
static void M_Mouse(int choice); // [crispy] mouse sensitivity menu
static void M_Sound(int choice);

static void M_FinishReadThis(int choice);
static void M_LoadSelect(int choice);
static void M_SaveSelect(int choice);
static void M_ReadSaveStrings(void);
static void M_QuickSave(void);
static void M_QuickLoad(void);

static void M_DrawMainMenu(void);
static void M_DrawReadThis1(void);
static void M_DrawReadThis2(void);
static void M_DrawNewGame(void);
static void M_DrawEpisode(void);
static void M_DrawOptions(void);
static void M_DrawMouse(void); // [crispy] mouse sensitivity menu
static void M_DrawSound(void);
static void M_DrawLoad(void);
static void M_DrawSave(void);

static void M_DrawSaveLoadBorder(int x,int y);
static void M_SetupNextMenu(menu_t *menudef);
static void M_DrawThermo(int x,int y,int thermWidth,int thermDot);
static void M_WriteText(int x, int y, const char *string);
int  M_StringWidth(const char *string); // [crispy] un-static
static int  M_StringHeight(const char *string);
static void M_StartMessage(const char *string, void *routine, boolean input);
static void M_ClearMenus (void);

// [crispy] Crispness menu
static void M_CrispnessCur(int choice);
static void M_CrispnessNext(int choice);
static void M_CrispnessPrev(int choice);
static void M_DrawCrispness1(void);
static void M_DrawCrispness2(void);
static void M_DrawCrispness3(void);
static void M_DrawCrispness4(void);



//
// DOOM MENU
//
enum
{
    newgame = 0,
    options,
    loadgame,
    savegame,
    readthis,
    quitdoom,
    main_end
} main_e;

menuitem_t MainMenu[]=
{
    {1,"M_NGAME",M_NewGame,'n'},
    {1,"M_OPTION",M_Options,'o'},
    {1,"M_LOADG",M_LoadGame,'l'},
    {1,"M_SAVEG",M_SaveGame,'s'},
    // Another hickup with Special edition.
    {1,"M_RDTHIS",M_ReadThis,'r'},
    {1,"M_QUITG",M_QuitDOOM,'q'}
};

menu_t  MainDef =
{
    main_end,
    NULL,
    MainMenu,
    M_DrawMainMenu,
    97,64,
    0
};


//
// EPISODE SELECT
//
enum
{
    ep1,
    ep2,
    ep3,
    ep4,
    ep5, // [crispy] Sigil
    ep6, // [crispy] Sigil II
    ep_end
} episodes_e;

menuitem_t EpisodeMenu[]=
{
    {1,"M_EPI1", M_Episode,'k'},
    {1,"M_EPI2", M_Episode,'t'},
    {1,"M_EPI3", M_Episode,'i'},
    {1,"M_EPI4", M_Episode,'t'}
   ,{1,"M_EPI5", M_Episode,'s'} // [crispy] Sigil
   ,{1,"M_EPI6", M_Episode,'s'} // [crispy] Sigil II
};

// [crispy] have Sigil II but not Sigil
menuitem_t EpisodeMenuSII[]=
{
    {1,"M_EPI1", M_Episode,'k'},
    {1,"M_EPI2", M_Episode,'t'},
    {1,"M_EPI3", M_Episode,'i'},
    {1,"M_EPI4", M_Episode,'t'}
   ,{1,"M_EPI6", M_Episode,'s'} // [crispy] Sigil II
};

menu_t  EpiDef =
{
    ep_end,		// # of menu items
    &MainDef,		// previous menu
    EpisodeMenu,	// menuitem_t ->
    M_DrawEpisode,	// drawing routine ->
    48,63,              // x,y
    ep1			// lastOn
};

//
// NEW GAME
//
enum
{
    killthings,
    toorough,
    hurtme,
    violence,
    nightmare,
    newg_end
} newgame_e;

menuitem_t NewGameMenu[]=
{
    {1,"M_JKILL",	M_ChooseSkill, 'i', "I'm too young to die."},
    {1,"M_ROUGH",	M_ChooseSkill, 'h', "Hey, not too rough!."},
    {1,"M_HURT",	M_ChooseSkill, 'h', "Hurt me plenty."},
    {1,"M_ULTRA",	M_ChooseSkill, 'u', "Ultra-Violence."},
    {1,"M_NMARE",	M_ChooseSkill, 'n', "Nightmare!"}
};

menu_t  NewDef =
{
    newg_end,		// # of menu items
    &EpiDef,		// previous menu
    NewGameMenu,	// menuitem_t ->
    M_DrawNewGame,	// drawing routine ->
    48,63,              // x,y
    hurtme		// lastOn
};



//
// OPTIONS MENU
//
enum
{
    endgame,
    messages,
    detail,
    scrnsize,
    option_empty1,
    mousesens,
    soundvol,
    crispness, // [crispy] Crispness menu
    opt_end
} options_e;

menuitem_t OptionsMenu[]=
{
    {1,"M_ENDGAM",	M_EndGame,'e', "End Game"},
    {3,"M_MESSG",	M_ChangeMessages,'m', "Messages: "},
    {3,"M_DETAIL",	M_ChangeDetail,'g', "Graphic Detail: "},
    {2,"M_SCRNSZ",	M_SizeDisplay,'s', "Screen Size"},
    {-1,"",0,'\0'},
    {1,"M_MSENS",	M_Mouse,'m', "Mouse Sensitivity"}, // [crispy] mouse sensitivity menu
    {1,"M_SVOL",	M_Sound,'s', "Sound Volume"},
    {1,"M_CRISPY",	M_CrispnessCur,'c', "Crispness"} // [crispy] Crispness menu
};

menu_t  OptionsDef =
{
    opt_end,
    &MainDef,
    OptionsMenu,
    M_DrawOptions,
    60,37,
    0
};

// [crispy] mouse sensitivity menu
enum
{
    mouse_horiz,
    mouse_empty1,
    mouse_horiz2,
    mouse_empty2,
    mouse_vert,
    mouse_empty3,
    mouse_invert,
    mouse_end
} mouse_e;

static menuitem_t MouseMenu[]=
{
    {2,"",	M_ChangeSensitivity,'h'},
    {-1,"",0,'\0'},
    {2,"",	M_ChangeSensitivity_x2,'s'},
    {-1,"",0,'\0'},
    {2,"",	M_ChangeSensitivity_y,'v'},
    {-1,"",0,'\0'},
    {3,"",	M_MouseInvert,'i'},
};

static menu_t  MouseDef =
{
    mouse_end,
    &OptionsDef,
    MouseMenu,
    M_DrawMouse,
    80,64,
    0
};

// [crispy] Crispness menu
enum
{
    crispness_sep_rendering,
    crispness_hires,
    crispness_widescreen,
    crispness_uncapped,
    crispness_fpslimit,
    crispness_vsync,
    crispness_smoothscaling,
    crispness_sep_rendering_,

    crispness_sep_visual,
    crispness_coloredhud,
    crispness_translucency,
    crispness_smoothlight,
    crispness_brightmaps,
    crispness_coloredblood,
    crispness_flipcorpses,
    crispness_sep_visual_,

    crispness1_next,
    crispness1_prev,
    crispness1_end
} crispness1_e;

static menuitem_t Crispness1Menu[]=
{
    {-1,"",0,'\0'},
    {3,"",	M_CrispyToggleHires,'h'},
    {3,"",	M_CrispyToggleWidescreen,'w'},
    {3,"",	M_CrispyToggleUncapped,'u'},
    {4,"",	M_CrispyToggleFpsLimit,'f'},
    {3,"",	M_CrispyToggleVsync,'v'},
    {3,"",	M_CrispyToggleSmoothScaling,'s'},
    {-1,"",0,'\0'},
    {-1,"",0,'\0'},
    {3,"",	M_CrispyToggleColoredhud,'c'},
    {3,"",	M_CrispyToggleTranslucency,'e'},
    {3,"",	M_CrispyToggleSmoothLighting,'s'},
    {3,"",	M_CrispyToggleBrightmaps,'b'},
    {3,"",	M_CrispyToggleColoredblood,'c'},
    {3,"",	M_CrispyToggleFlipcorpses,'r'},
    {-1,"",0,'\0'},
    {1,"",	M_CrispnessNext,'n'},
    {1,"",	M_CrispnessPrev,'p'},
};

static menu_t  Crispness1Def =
{
    crispness1_end,
    &OptionsDef,
    Crispness1Menu,
    M_DrawCrispness1,
    48,18,
    1
};

enum
{
    crispness_sep_audible,
    crispness_soundfull,
    crispness_soundfix,
    crispness_sndchannels,
    crispness_soundmono,
    crispness_sep_audible_,

    crispness_sep_navigational,
    crispness_extautomap,
    crispness_smoothmap,
    crispness_automapstats,
    crispness_statsformat,
    crispness_leveltime,
    crispness_playercoords,
    crispness_secretmessage,
    crispness_sep_navigational_,

    crispness2_next,
    crispness2_prev,
    crispness2_end
} crispness2_e;

static menuitem_t Crispness2Menu[]=
{
    {-1,"",0,'\0'},
    {3,"",	M_CrispyToggleFullsounds,'p'},
    {3,"",	M_CrispyToggleSoundfixes,'m'},
    {3,"",	M_CrispyToggleSndChannels,'s'},
    {3,"",	M_CrispyToggleSoundMono,'m'},
    {-1,"",0,'\0'},
    {-1,"",0,'\0'},
    {3,"",	M_CrispyToggleExtAutomap,'e'},
    {3,"",	M_CrispyToggleSmoothMap,'m'},
    {3,"",	M_CrispyToggleAutomapstats,'s'},
    {3,"",	M_CrispyToggleStatsFormat,'f'},
    {3,"",	M_CrispyToggleLeveltime,'l'},
    {3,"",	M_CrispyTogglePlayerCoords,'p'},
    {3,"",	M_CrispyToggleSecretmessage,'s'},
    {-1,"",0,'\0'},
    {1,"",	M_CrispnessNext,'n'},
    {1,"",	M_CrispnessPrev,'p'},
};

static menu_t  Crispness2Def =
{
    crispness2_end,
    &OptionsDef,
    Crispness2Menu,
    M_DrawCrispness2,
    48,18,
    1
};

enum
{
    crispness_sep_tactical,
    crispness_freelook,
    crispness_mouselook,
    crispness_bobfactor,
    crispness_centerweapon,
    crispness_pitch,
    crispness_neghealth,
    crispness_defaultskill,
    crispness_sep_tactical_,

    crispness_sep_crosshair,
    crispness_crosshair,
    crispness_crosshairtype,
    crispness_crosshairhealth,
    crispness_crosshairtarget,
    crispness_sep_crosshair_,

    crispness3_next,
    crispness3_prev,
    crispness3_end
} crispness3_e;

static menuitem_t Crispness3Menu[]=
{
    {-1,"",0,'\0'},
    {3,"",	M_CrispyToggleFreelook,'a'},
    {3,"",	M_CrispyToggleMouseLook,'p'},
    {3,"",	M_CrispyToggleBobfactor,'p'},
    {3,"",	M_CrispyToggleCenterweapon,'c'},
    {3,"",	M_CrispyTogglePitch,'w'},
    {3,"",	M_CrispyToggleNeghealth,'n'},
    {3,"",	M_CrispyToggleDefaultSkill,'d'},
    {-1,"",0,'\0'},
    {-1,"",0,'\0'},
    {3,"",	M_CrispyToggleCrosshair,'d'},
    {3,"",	M_CrispyToggleCrosshairtype,'c'},
    {3,"",	M_CrispyToggleCrosshairHealth,'c'},
    {3,"",	M_CrispyToggleCrosshairTarget,'h'},
    {-1,"",0,'\0'},
    {1,"",	M_CrispnessNext,'n'},
    {1,"",	M_CrispnessPrev,'p'},
};

static menu_t  Crispness3Def =
{
    crispness3_end,
    &OptionsDef,
    Crispness3Menu,
    M_DrawCrispness3,
    48,18,
    1
};

enum
{
    crispness_sep_physical,
    crispness_freeaim,
    crispness_jumping,
    crispness_overunder,
    crispness_sep_physical_,

    crispness_sep_demos,
    crispness_demotimer,
    crispness_demotimerdir,
    crispness_demobar,
    crispness_demousetimer,
    crispness_sep_demos_,

    crispness4_next,
    crispness4_prev,
    crispness4_end
} crispness4_e;


static menuitem_t Crispness4Menu[]=
{
    {-1,"",0,'\0'},
    {3,"",	M_CrispyToggleFreeaim,'v'},
    {3,"",	M_CrispyToggleJumping,'a'},
    {3,"",	M_CrispyToggleOverunder,'w'},
    {-1,"",0,'\0'},
    {-1,"",0,'\0'},
    {3,"",	M_CrispyToggleDemoTimer,'v'},
    {3,"",	M_CrispyToggleDemoTimerDir,'a'},
    {3,"",	M_CrispyToggleDemoBar,'w'},
    {3,"",	M_CrispyToggleDemoUseTimer,'u'},
    {-1,"",0,'\0'},
    {1,"",	M_CrispnessNext,'n'},
    {1,"",	M_CrispnessPrev,'p'},
};

static menu_t  Crispness4Def =
{
    crispness4_end,
    &OptionsDef,
    Crispness4Menu,
    M_DrawCrispness4,
    48,18,
    1
};

static menu_t *CrispnessMenus[] =
{
	&Crispness1Def,
	&Crispness2Def,
	&Crispness3Def,
	&Crispness4Def,
};

static int crispness_cur;

//
// Read This! MENU 1 & 2
//
enum
{
    rdthsempty1,
    read1_end
} read_e;

menuitem_t ReadMenu1[] =
{
    {1,"",M_ReadThis2,0}
};

menu_t  ReadDef1 =
{
    read1_end,
    &MainDef,
    ReadMenu1,
    M_DrawReadThis1,
    280,185,
    0
};

enum
{
    rdthsempty2,
    read2_end
} read_e2;

menuitem_t ReadMenu2[]=
{
    {1,"",M_FinishReadThis,0}
};

menu_t  ReadDef2 =
{
    read2_end,
    &ReadDef1,
    ReadMenu2,
    M_DrawReadThis2,
    330,175,
    0
};

//
// SOUND VOLUME MENU
//
enum
{
    sfx_vol,
    sfx_empty1,
    music_vol,
    sfx_empty2,
    sound_end
} sound_e;

menuitem_t SoundMenu[]=
{
    {2,"M_SFXVOL",M_SfxVol,'s'},
    {-1,"",0,'\0'},
    {2,"M_MUSVOL",M_MusicVol,'m'},
    {-1,"",0,'\0'}
};

menu_t  SoundDef =
{
    sound_end,
    &OptionsDef,
    SoundMenu,
    M_DrawSound,
    80,64,
    0
};

//
// LOAD GAME MENU
//
enum
{
    load1,
    load2,
    load3,
    load4,
    load5,
    load6,
    load7, // [crispy] up to 8 savegames
    load8, // [crispy] up to 8 savegames
    load_end
} load_e;

menuitem_t LoadMenu[]=
{
    {1,"", M_LoadSelect,'1'},
    {1,"", M_LoadSelect,'2'},
    {1,"", M_LoadSelect,'3'},
    {1,"", M_LoadSelect,'4'},
    {1,"", M_LoadSelect,'5'},
    {1,"", M_LoadSelect,'6'},
    {1,"", M_LoadSelect,'7'}, // [crispy] up to 8 savegames
    {1,"", M_LoadSelect,'8'}  // [crispy] up to 8 savegames
};

menu_t  LoadDef =
{
    load_end,
    &MainDef,
    LoadMenu,
    M_DrawLoad,
    80,54,
    0
};

//
// SAVE GAME MENU
//
menuitem_t SaveMenu[]=
{
    {1,"", M_SaveSelect,'1'},
    {1,"", M_SaveSelect,'2'},
    {1,"", M_SaveSelect,'3'},
    {1,"", M_SaveSelect,'4'},
    {1,"", M_SaveSelect,'5'},
    {1,"", M_SaveSelect,'6'},
    {1,"", M_SaveSelect,'7'}, // [crispy] up to 8 savegames
    {1,"", M_SaveSelect,'8'}  // [crispy] up to 8 savegames
};

menu_t  SaveDef =
{
    load_end,
    &MainDef,
    SaveMenu,
    M_DrawSave,
    80,54,
    0
};


//
// M_ReadSaveStrings
//  read the strings from the savegame files
//
void M_ReadSaveStrings(void)
{
    FILE   *handle;
    int     i;
    char    name[256];

    for (i = 0;i < load_end;i++)
    {
        int retval;
        M_StringCopy(name, P_SaveGameFile(i), sizeof(name));

	handle = M_fopen(name, "rb");
        if (handle == NULL)
        {
            M_StringCopy(savegamestrings[i], EMPTYSTRING, SAVESTRINGSIZE);
            LoadMenu[i].status = 0;
            continue;
        }
        retval = fread(&savegamestrings[i], 1, SAVESTRINGSIZE, handle);
	fclose(handle);
        LoadMenu[i].status = retval == SAVESTRINGSIZE;
    }
}

// [FG] support up to 8 pages of savegames
static void M_DrawSaveLoadBottomLine(void)
{
  char pagestr[16];
  const int y = 152;

  dp_translation = cr[CR_GOLD];

  if (savepage > 0)
    M_WriteText(LoadDef.x, y, "< PGUP");
  if (savepage < savepage_max)
    M_WriteText(LoadDef.x+(SAVESTRINGSIZE-6)*8, y, "PGDN >");

  M_snprintf(pagestr, sizeof(pagestr), "page %d/%d", savepage + 1, savepage_max + 1);
  M_WriteText(ORIGWIDTH/2-M_StringWidth(pagestr)/2, y, pagestr);

  // [crispy] print "modified" (or created initially) time of savegame file
  if (LoadMenu[itemOn].status)
  {
    struct stat st;
    char filedate[32];

    if (M_stat(P_SaveGameFile(itemOn), &st) == 0)
    {
// [FG] suppress the most useless compiler warning ever
#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-y2k"
#endif
    strftime(filedate, sizeof(filedate), "%x %X", localtime(&st.st_mtime));
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
    M_WriteText(ORIGWIDTH/2-M_StringWidth(filedate)/2, y + 8, filedate);
    }
  }

  dp_translation = NULL;
}


//
// M_LoadGame & Cie.
//
static int LoadDef_x = 72, LoadDef_y = 28;
void M_DrawLoad(void)
{
    int             i;
	
    V_DrawPatchDirect(LoadDef_x, LoadDef_y,
                      W_CacheLumpName(DEH_String("M_LOADG"), PU_CACHE));

    for (i = 0;i < load_end; i++)
    {
	M_DrawSaveLoadBorder(LoadDef.x,LoadDef.y+LINEHEIGHT*i);
	M_WriteText(LoadDef.x,LoadDef.y+LINEHEIGHT*i,savegamestrings[i]);
    }

  M_DrawSaveLoadBottomLine();
}



//
// Draw border for the savegame description
//
void M_DrawSaveLoadBorder(int x,int y)
{
    int             i;
	
    V_DrawPatchDirect(x - 8, y + 7,
                      W_CacheLumpName(DEH_String("M_LSLEFT"), PU_CACHE));
	
    for (i = 0;i < 24;i++)
    {
	V_DrawPatchDirect(x, y + 7,
                          W_CacheLumpName(DEH_String("M_LSCNTR"), PU_CACHE));
	x += 8;
    }

    V_DrawPatchDirect(x, y + 7, 
                      W_CacheLumpName(DEH_String("M_LSRGHT"), PU_CACHE));
}



//
// User wants to load this game
//
void M_LoadSelect(int choice)
{
    char    name[256];
	
    M_StringCopy(name, P_SaveGameFile(choice), sizeof(name));

    // [crispy] save the last game you loaded
    SaveDef.lastOn = choice;
    G_LoadGame (name);
    M_ClearMenus ();

    // [crispy] allow quickload before quicksave
    if (quickSaveSlot == -2)
	quickSaveSlot = choice;
}

//
// Selected from DOOM menu
//
void M_LoadGame (int choice)
{
    // [crispy] allow loading game while multiplayer demo playback
    if (netgame && !demoplayback)
    {
	M_StartMessage(DEH_String(LOADNET),NULL,false);
	return;
    }
	
    M_SetupNextMenu(&LoadDef);
    M_ReadSaveStrings();
}


//
//  M_SaveGame & Cie.
//
static int SaveDef_x = 72, SaveDef_y = 28;
void M_DrawSave(void)
{
    int             i;
	
    V_DrawPatchDirect(SaveDef_x, SaveDef_y, W_CacheLumpName(DEH_String("M_SAVEG"), PU_CACHE));
    for (i = 0;i < load_end; i++)
    {
	M_DrawSaveLoadBorder(LoadDef.x,LoadDef.y+LINEHEIGHT*i);
	M_WriteText(LoadDef.x,LoadDef.y+LINEHEIGHT*i,savegamestrings[i]);
    }
	
    if (saveStringEnter)
    {
	i = M_StringWidth(savegamestrings[saveSlot]);
	M_WriteText(LoadDef.x + i,LoadDef.y+LINEHEIGHT*saveSlot,"_");
    }

  M_DrawSaveLoadBottomLine();
}

//
// M_Responder calls this when user is finished
//
void M_DoSave(int slot)
{
    G_SaveGame (slot,savegamestrings[slot]);
    M_ClearMenus ();

    // PICK QUICKSAVE SLOT YET?
    if (quickSaveSlot == -2)
	quickSaveSlot = slot;
}

//
// Generate a default save slot name when the user saves to
// an empty slot via the joypad.
//
static void SetDefaultSaveName(int slot)
{
    // map from IWAD or PWAD?
    if (W_IsIWADLump(maplumpinfo) && strcmp(savegamedir, ""))
    {
        M_snprintf(savegamestrings[itemOn], SAVESTRINGSIZE,
                   "%s", maplumpinfo->name);
    }
    else
    {
        char *wadname = M_StringDuplicate(W_WadNameForLump(maplumpinfo));
        char *ext = strrchr(wadname, '.');

        if (ext != NULL)
        {
            *ext = '\0';
        }

        M_snprintf(savegamestrings[itemOn], SAVESTRINGSIZE,
                   "%s (%s)", maplumpinfo->name,
                   wadname);
        free(wadname);
    }
    M_ForceUppercase(savegamestrings[itemOn]);
    joypadSave = false;
}

// [crispy] override savegame name if it already starts with a map identifier
static boolean StartsWithMapIdentifier (char *str)
{
    M_ForceUppercase(str);

    if (strlen(str) >= 4 &&
        str[0] == 'E' && isdigit(str[1]) &&
        str[2] == 'M' && isdigit(str[3]))
    {
        return true;
    }

    if (strlen(str) >= 5 &&
        str[0] == 'M' && str[1] == 'A' && str[2] == 'P' &&
        isdigit(str[3]) && isdigit(str[4]))
    {
        return true;
    }

    return false;
}

//
// User wants to save. Start string input for M_Responder
//
void M_SaveSelect(int choice)
{
    int x, y;

    // we are going to be intercepting all chars
    saveStringEnter = 1;

    // [crispy] load the last game you saved
    LoadDef.lastOn = choice;

    // We need to turn on text input:
    x = LoadDef.x - 11;
    y = LoadDef.y + choice * LINEHEIGHT - 4;
    I_StartTextInput(x, y, x + 8 + 24 * 8 + 8, y + LINEHEIGHT - 2);

    saveSlot = choice;
    M_StringCopy(saveOldString,savegamestrings[choice], SAVESTRINGSIZE);
    if (!strcmp(savegamestrings[choice], EMPTYSTRING) ||
        // [crispy] override savegame name if it already starts with a map identifier
        StartsWithMapIdentifier(savegamestrings[choice]))
    {
        savegamestrings[choice][0] = 0;

        if (joypadSave || true) // [crispy] always prefill empty savegame slot names
        {
            SetDefaultSaveName(choice);
        }
    }
    saveCharIndex = strlen(savegamestrings[choice]);
}

//
// Selected from DOOM menu
//
void M_SaveGame (int choice)
{
    if (!usergame)
    {
	M_StartMessage(DEH_String(SAVEDEAD),NULL,false);
	return;
    }
	
    if (gamestate != GS_LEVEL)
	return;
	
    M_SetupNextMenu(&SaveDef);
    M_ReadSaveStrings();
}



//
//      M_QuickSave
//

void M_QuickSaveResponse(int key)
{
    if (key == key_menu_confirm)
    {
	M_DoSave(quickSaveSlot);
	S_StartSoundOptional(NULL, sfx_mnucls, sfx_swtchx); // [NS] Optional menu sounds.
    }
}

void M_QuickSave(void)
{
    if (!usergame)
    {
	S_StartSoundOptional(NULL, sfx_mnuerr, sfx_oof); // [NS] Optional menu sounds.
	return;
    }

    if (gamestate != GS_LEVEL)
	return;
	
    if (quickSaveSlot < 0)
    {
	M_StartControlPanel();
	M_ReadSaveStrings();
	M_SetupNextMenu(&SaveDef);
	quickSaveSlot = -2;	// means to pick a slot now
	return;
    }
    M_QuickSaveResponse(key_menu_confirm);
}



//
// M_QuickLoad
//
void M_QuickLoadResponse(int key)
{
    if (key == key_menu_confirm)
    {
	M_LoadSelect(quickSaveSlot);
	S_StartSoundOptional(NULL, sfx_mnucls, sfx_swtchx); // [NS] Optional menu sounds.
    }
}


void M_QuickLoad(void)
{
    // [crispy] allow quickloading game while multiplayer demo playback
    if (netgame && !demoplayback)
    {
	M_StartMessage(DEH_String(QLOADNET),NULL,false);
	return;
    }
	
    if (quickSaveSlot < 0)
    {
	// [crispy] allow quickload before quicksave
	M_StartControlPanel();
	M_ReadSaveStrings();
	M_SetupNextMenu(&LoadDef);
	quickSaveSlot = -2;
	return;
    }
    M_QuickLoadResponse(key_menu_confirm);
}




//
// Read This Menus
// Had a "quick hack to fix romero bug"
//
void M_DrawReadThis1(void)
{
    inhelpscreens = true;

    V_DrawPatchFullScreen(W_CacheLumpName(DEH_String("HELP2"), PU_CACHE), false);
}



//
// Read This Menus - optional second page.
//
void M_DrawReadThis2(void)
{
    inhelpscreens = true;

    // We only ever draw the second page if this is 
    // gameversion == exe_doom_1_9 and gamemode == registered

    V_DrawPatchFullScreen(W_CacheLumpName(DEH_String("HELP1"), PU_CACHE), false);
}

void M_DrawReadThisCommercial(void)
{
    inhelpscreens = true;

    V_DrawPatchFullScreen(W_CacheLumpName(DEH_String("HELP"), PU_CACHE), false);
}


//
// Change Sfx & Music volumes
//
void M_DrawSound(void)
{
    V_DrawPatchDirect (60, 38, W_CacheLumpName(DEH_String("M_SVOL"), PU_CACHE));

    M_DrawThermo(SoundDef.x,SoundDef.y+LINEHEIGHT*(sfx_vol+1),
		 16,sfxVolume);

    M_DrawThermo(SoundDef.x,SoundDef.y+LINEHEIGHT*(music_vol+1),
		 16,musicVolume);
}

void M_Sound(int choice)
{
    M_SetupNextMenu(&SoundDef);
}

void M_SfxVol(int choice)
{
    switch(choice)
    {
      case 0:
	if (sfxVolume)
	    sfxVolume--;
	break;
      case 1:
	if (sfxVolume < 15)
	    sfxVolume++;
	break;
    }
	
    S_SetSfxVolume(sfxVolume * 8);
}

void M_MusicVol(int choice)
{
    switch(choice)
    {
      case 0:
	if (musicVolume)
	    musicVolume--;
	break;
      case 1:
	if (musicVolume < 15)
	    musicVolume++;
	break;
    }
	
    S_SetMusicVolume(musicVolume * 8);
}




//
// M_DrawMainMenu
//
void M_DrawMainMenu(void)
{
    // [crispy] force status bar refresh
    inhelpscreens = true;

    V_DrawPatchDirect(94, 2,
                      W_CacheLumpName(DEH_String("M_DOOM"), PU_CACHE));
}




//
// M_NewGame
//
void M_DrawNewGame(void)
{
    // [crispy] force status bar refresh
    inhelpscreens = true;

    V_DrawPatchDirect(96, 14, W_CacheLumpName(DEH_String("M_NEWG"), PU_CACHE));
    V_DrawPatchDirect(54, 38, W_CacheLumpName(DEH_String("M_SKILL"), PU_CACHE));
}

void M_NewGame(int choice)
{
    // [crispy] forbid New Game while recording a demo
    if (demorecording)
    {
	return;
    }

    if (netgame && !demoplayback)
    {
	M_StartMessage(DEH_String(NEWGAME),NULL,false);
	return;
    }
	
    // Chex Quest disabled the episode select screen, as did Doom II.

    if ((gamemode == commercial && !crispy->havenerve && !crispy->havemaster) || gameversion == exe_chex) // [crispy] NRFTL / The Master Levels
	M_SetupNextMenu(&NewDef);
    else
	M_SetupNextMenu(&EpiDef);
}


//
//      M_Episode
//
int     epi;

void M_DrawEpisode(void)
{
    // [crispy] force status bar refresh
    inhelpscreens = true;

    if (W_CheckNumForName(DEH_String("M_EPISOD")) != -1)
    V_DrawPatchDirect(54, 38, W_CacheLumpName(DEH_String("M_EPISOD"), PU_CACHE));
    else
    {
      M_WriteText(54, 38, "Which Episode?");
      EpiDef.lumps_missing = 1;
    }
}

void M_VerifyNightmare(int key)
{
    // [crispy] allow to confirm by pressing Enter key
    if (key != key_menu_confirm && key != key_menu_forward)
	return;
		
    G_DeferedInitNew(nightmare,epi+1,1);
    M_ClearMenus ();
}

void M_ChooseSkill(int choice)
{
    if (choice == nightmare)
    {
	M_StartMessage(DEH_String(NIGHTMARE),M_VerifyNightmare,true);
	return;
    }
	
    G_DeferedInitNew(choice,epi+1,1);
    M_ClearMenus ();
}

void M_Episode(int choice)
{
    if ( (gamemode == shareware)
	 && choice)
    {
	M_StartMessage(DEH_String(SWSTRING),NULL,false);
	M_SetupNextMenu(&ReadDef1);
	return;
    }

    epi = choice;
    // [crispy] have Sigil II loaded but not Sigil
    if (epi == 4 && crispy->haved1e6 && !crispy->haved1e5)
        epi = 5;
    M_SetupNextMenu(&NewDef);
}



//
// M_Options
//
static const char *detailNames[2] = {"M_GDHIGH","M_GDLOW"};
static const char *msgNames[2] = {"M_MSGOFF","M_MSGON"};

void M_DrawOptions(void)
{
    V_DrawPatchDirect(108, 15, W_CacheLumpName(DEH_String("M_OPTTTL"),
                                               PU_CACHE));
	
    if (OptionsDef.lumps_missing == -1)
    {
    V_DrawPatchDirect(OptionsDef.x + 175, OptionsDef.y + LINEHEIGHT * detail,
		      W_CacheLumpName(DEH_String(detailNames[detailLevel]),
			              PU_CACHE));
    }
    else
    if (OptionsDef.lumps_missing > 0)
    {
    M_WriteText(OptionsDef.x + M_StringWidth("Graphic Detail: "),
                OptionsDef.y + LINEHEIGHT * detail + 8 - (M_StringHeight("HighLow")/2),
                detailLevel ? "Low" : "High");
    }

    if (OptionsDef.lumps_missing == -1)
    {
    V_DrawPatchDirect(OptionsDef.x + 120, OptionsDef.y + LINEHEIGHT * messages,
                      W_CacheLumpName(DEH_String(msgNames[showMessages]),
                                      PU_CACHE));
    }
    else
    if (OptionsDef.lumps_missing > 0)
    {
    M_WriteText(OptionsDef.x + M_StringWidth("Messages: "),
                OptionsDef.y + LINEHEIGHT * messages + 8 - (M_StringHeight("OnOff")/2),
                showMessages ? "On" : "Off");
    }

// [crispy] mouse sensitivity menu
/*
    M_DrawThermo(OptionsDef.x, OptionsDef.y + LINEHEIGHT * (mousesens + 1),
		 10, mouseSensitivity);
*/

    M_DrawThermo(OptionsDef.x,OptionsDef.y+LINEHEIGHT*(scrnsize+1),
		 9 + (crispy->widescreen ? 6 : 3),screenSize); // [crispy] Crispy HUD
}

// [crispy] mouse sensitivity menu
static void M_DrawMouse(void)
{
    char mouse_menu_text[48];

    V_DrawPatchDirect (60, LoadDef_y, W_CacheLumpName(DEH_String("M_MSENS"), PU_CACHE));

    M_WriteText(MouseDef.x, MouseDef.y + LINEHEIGHT * mouse_horiz + 6,
                "HORIZONTAL: TURN");

    M_DrawThermo(MouseDef.x, MouseDef.y + LINEHEIGHT * mouse_empty1,
		 21, mouseSensitivity);

    M_WriteText(MouseDef.x, MouseDef.y + LINEHEIGHT * mouse_horiz2 + 6,
                "HORIZONTAL: STRAFE");

    M_DrawThermo(MouseDef.x, MouseDef.y + LINEHEIGHT * mouse_empty2,
		 21, mouseSensitivity_x2);

    M_WriteText(MouseDef.x, MouseDef.y + LINEHEIGHT * mouse_vert + 6,
                "VERTICAL");

    M_DrawThermo(MouseDef.x, MouseDef.y + LINEHEIGHT * mouse_empty3,
		 21, mouseSensitivity_y);

    M_snprintf(mouse_menu_text, sizeof(mouse_menu_text),
               "%sInvert Vertical Axis: %s%s", crstr[CR_NONE],
               mouse_y_invert ? crstr[CR_GREEN] : crstr[CR_DARK],
               mouse_y_invert ? "On" : "Off");
    M_WriteText(MouseDef.x, MouseDef.y + LINEHEIGHT * mouse_invert + 6,
                mouse_menu_text);

    dp_translation = NULL;
}

// [crispy] Crispness menu
#include "m_background.h"
static void M_DrawCrispnessBackground(void)
{
	const byte *src = crispness_background;
	pixel_t *dest;

	// [NS] Try to load the background from a lump.
	int lump = W_CheckNumForName("CRISPYBG");
	if (lump != -1 && W_LumpLength(lump) >= 64*64)
	{
		src = W_CacheLumpNum(lump, PU_CACHE);
	}
	dest = I_VideoBuffer;

	V_FillFlat(0, SCREENHEIGHT, 0, SCREENWIDTH, src, dest);

	inhelpscreens = true;
}

static char crispy_menu_text[48];

static void M_DrawCrispnessHeader(const char *item)
{
    M_snprintf(crispy_menu_text, sizeof(crispy_menu_text),
               "%s%s", crstr[CR_GOLD], item);
    M_WriteText(ORIGWIDTH/2 - M_StringWidth(item) / 2, 6, crispy_menu_text);
}

static void M_DrawCrispnessSeparator(int y, const char *item)
{
    M_snprintf(crispy_menu_text, sizeof(crispy_menu_text),
               "%s%s", crstr[CR_GOLD], item);
    M_WriteText(currentMenu->x - 8, currentMenu->y + CRISPY_LINEHEIGHT * y, crispy_menu_text);
}

static void M_DrawCrispnessItem(int y, const char *item, int feat, boolean cond)
{
    M_snprintf(crispy_menu_text, sizeof(crispy_menu_text),
               "%s%s: %s%s", cond ? crstr[CR_NONE] : crstr[CR_DARK], item,
               cond ? (feat ? crstr[CR_GREEN] : crstr[CR_DARK]) : crstr[CR_DARK],
               cond && feat ? "On" : "Off");
    M_WriteText(currentMenu->x, currentMenu->y + CRISPY_LINEHEIGHT * y, crispy_menu_text);
}

static void M_DrawCrispnessMultiItem(int y, const char *item, multiitem_t *multiitem, int feat, boolean cond)
{
    M_snprintf(crispy_menu_text, sizeof(crispy_menu_text),
               "%s%s: %s%s", cond ? crstr[CR_NONE] : crstr[CR_DARK], item,
               cond ? (feat ? crstr[CR_GREEN] : crstr[CR_DARK]) : crstr[CR_DARK],
               cond && feat ? multiitem[feat].name : multiitem[0].name);
    M_WriteText(currentMenu->x, currentMenu->y + CRISPY_LINEHEIGHT * y, crispy_menu_text);
}

static void M_DrawCrispnessNumericItem(int y, const char *item, int feat, const char *zero, boolean cond, const char *disabled)
{
    char number[NUMERIC_ENTRY_NUMDIGITS + 2];
    const int size = NUMERIC_ENTRY_NUMDIGITS + 2;

    if (numeric_enter)
    {
        M_snprintf(number, size, "%s_", numeric_entry_str);
    }
    else
    {
        M_snprintf(number, size, "%d", feat);
    }

    M_snprintf(crispy_menu_text, sizeof(crispy_menu_text),
               "%s%s: %s%s", cond ? crstr[CR_NONE] : crstr[CR_DARK], item,
               cond ? (feat || numeric_enter ? crstr[CR_GREEN] : crstr[CR_DARK]) : crstr[CR_DARK],
               cond ? (feat || numeric_enter ? number : zero) : disabled);
    M_WriteText(currentMenu->x, currentMenu->y + CRISPY_LINEHEIGHT * y, crispy_menu_text);
}

static void M_DrawCrispnessGoto(int y, const char *item)
{
    M_snprintf(crispy_menu_text, sizeof(crispy_menu_text),
               "%s%s", crstr[CR_GOLD], item);
    M_WriteText(currentMenu->x, currentMenu->y + CRISPY_LINEHEIGHT * y, crispy_menu_text);
}

static void M_DrawCrispness1(void)
{
    M_DrawCrispnessBackground();

    M_DrawCrispnessHeader("Crispness 1/4");

    M_DrawCrispnessSeparator(crispness_sep_rendering, "Rendering");
    M_DrawCrispnessItem(crispness_hires, "High Resolution Rendering", crispy->hires, true);
    M_DrawCrispnessMultiItem(crispness_widescreen, "Aspect Ratio", multiitem_widescreen, crispy->widescreen, aspect_ratio_correct == 1);
    M_DrawCrispnessItem(crispness_uncapped, "Uncapped Framerate", crispy->uncapped, true);
    M_DrawCrispnessNumericItem(crispness_fpslimit, "Framerate Limit", crispy->fpslimit, "None", crispy->uncapped, "35");
    M_DrawCrispnessItem(crispness_vsync, "Enable VSync", crispy->vsync, !force_software_renderer);
    M_DrawCrispnessItem(crispness_smoothscaling, "Smooth Pixel Scaling", smooth_pixel_scaling, !force_software_renderer);

    M_DrawCrispnessSeparator(crispness_sep_visual, "Visual");
    M_DrawCrispnessMultiItem(crispness_coloredhud, "Colorize HUD Elements", multiitem_coloredhud, crispy->coloredhud, true);
    M_DrawCrispnessMultiItem(crispness_translucency, "Enable Translucency", multiitem_translucency, crispy->translucency, true);
    M_DrawCrispnessItem(crispness_smoothlight, "Smooth Diminishing Lighting", crispy->smoothlight, true);
    M_DrawCrispnessMultiItem(crispness_brightmaps, "Apply Brightmaps to", multiitem_brightmaps, crispy->brightmaps, true);
    M_DrawCrispnessMultiItem(crispness_coloredblood, "Colored Blood", multiitem_coloredblood, crispy->coloredblood, gameversion != exe_chex);
    M_DrawCrispnessItem(crispness_flipcorpses, "Randomly Mirrored Corpses", crispy->flipcorpses, gameversion != exe_chex);

    M_DrawCrispnessGoto(crispness1_next, "Next Page >");
    M_DrawCrispnessGoto(crispness1_prev, "< Last Page");

    dp_translation = NULL;
}

static void M_DrawCrispness2(void)
{
    M_DrawCrispnessBackground();

    M_DrawCrispnessHeader("Crispness 2/4");

    M_DrawCrispnessSeparator(crispness_sep_audible, "Audible");
    M_DrawCrispnessItem(crispness_soundfull, "Play sounds in full length", crispy->soundfull, true);
    M_DrawCrispnessItem(crispness_soundfix, "Misc. Sound Fixes", crispy->soundfix, true);
    M_DrawCrispnessMultiItem(crispness_sndchannels, "Sound Channels", multiitem_sndchannels, snd_channels >> 4, snd_sfxdevice != SNDDEVICE_PCSPEAKER);
    M_DrawCrispnessItem(crispness_soundmono, "Mono SFX", crispy->soundmono, true);

    M_DrawCrispnessSeparator(crispness_sep_navigational, "Navigational");
    M_DrawCrispnessItem(crispness_extautomap, "Extended Automap colors", crispy->extautomap, true);
    M_DrawCrispnessItem(crispness_smoothmap, "Smooth automap lines", crispy->smoothmap, true);
    M_DrawCrispnessMultiItem(crispness_automapstats, "Show Level Stats", multiitem_widgets, crispy->automapstats, true);
    M_DrawCrispnessMultiItem(crispness_statsformat, "Level Stats Format", multiitem_statsformat, crispy->statsformat, crispy->automapstats);
    M_DrawCrispnessMultiItem(crispness_leveltime, "Show Level Time", multiitem_widgets, crispy->leveltime, true);
    M_DrawCrispnessMultiItem(crispness_playercoords, "Show Player Coords", multiitem_widgets, crispy->playercoords, true);
    M_DrawCrispnessMultiItem(crispness_secretmessage, "Report Revealed Secrets", multiitem_secretmessage, crispy->secretmessage, true);

    M_DrawCrispnessGoto(crispness2_next, "Next Page >");
    M_DrawCrispnessGoto(crispness2_prev, "< Prev Page");

    dp_translation = NULL;
}

static void M_DrawCrispness3(void)
{
    M_DrawCrispnessBackground();

    M_DrawCrispnessHeader("Crispness 3/4");

    M_DrawCrispnessSeparator(crispness_sep_tactical, "Tactical");

    M_DrawCrispnessMultiItem(crispness_freelook, "Allow Free Look", multiitem_freelook, crispy->freelook, true);
    M_DrawCrispnessItem(crispness_mouselook, "Permanent Mouse Look", crispy->mouselook, true);
    M_DrawCrispnessMultiItem(crispness_bobfactor, "Player View/Weapon Bobbing", multiitem_bobfactor, crispy->bobfactor, true);
    M_DrawCrispnessMultiItem(crispness_centerweapon, "Weapon Attack Alignment", multiitem_centerweapon, crispy->centerweapon, crispy->bobfactor != BOBFACTOR_OFF);
    M_DrawCrispnessItem(crispness_pitch, "Weapon Recoil Pitch", crispy->pitch, true);
    M_DrawCrispnessItem(crispness_neghealth, "Negative Player Health", crispy->neghealth, true);
    M_DrawCrispnessMultiItem(crispness_defaultskill, "Default Difficulty", multiitem_difficulties, crispy->defaultskill, true);

    M_DrawCrispnessSeparator(crispness_sep_crosshair, "Crosshair");

    M_DrawCrispnessMultiItem(crispness_crosshair, "Draw Crosshair", multiitem_crosshair, crispy->crosshair, true);
    M_DrawCrispnessMultiItem(crispness_crosshairtype, "Crosshair Shape", multiitem_crosshairtype, crispy->crosshairtype + 1, crispy->crosshair);
    M_DrawCrispnessItem(crispness_crosshairhealth, "Color indicates Health", crispy->crosshairhealth, crispy->crosshair);
    M_DrawCrispnessItem(crispness_crosshairtarget, "Highlight on target", crispy->crosshairtarget, crispy->crosshair);

    M_DrawCrispnessGoto(crispness3_next, "Next Page >");
    M_DrawCrispnessGoto(crispness3_prev, "< Prev Page");

    dp_translation = NULL;
}

static void M_DrawCrispness4(void)
{
    M_DrawCrispnessBackground();

    M_DrawCrispnessHeader("Crispness 4/4");

    M_DrawCrispnessSeparator(crispness_sep_physical, "Physical");

    M_DrawCrispnessMultiItem(crispness_freeaim, "Vertical Aiming", multiitem_freeaim, crispy->freeaim, crispy->singleplayer);
    M_DrawCrispnessMultiItem(crispness_jumping, "Allow Jumping", multiitem_jump, crispy->jump, crispy->singleplayer);
    M_DrawCrispnessItem(crispness_overunder, "Walk over/under Monsters", crispy->overunder, crispy->singleplayer);

    M_DrawCrispnessSeparator(crispness_sep_demos, "Demos");

    M_DrawCrispnessMultiItem(crispness_demotimer, "Show Demo Timer", multiitem_demotimer, crispy->demotimer, true);
    M_DrawCrispnessMultiItem(crispness_demotimerdir, "Playback Timer Direction", multiitem_demotimerdir, crispy->demotimerdir + 1, crispy->demotimer & DEMOTIMER_PLAYBACK);
    M_DrawCrispnessItem(crispness_demobar, "Show Demo Progress Bar", crispy->demobar, true);
    M_DrawCrispnessItem(crispness_demousetimer, "\"Use\" Button Timer", crispy->btusetimer, true);

    M_DrawCrispnessGoto(crispness4_next, "First Page >");
    M_DrawCrispnessGoto(crispness4_prev, "< Prev Page");

    dp_translation = NULL;
}

void M_Options(int choice)
{
    M_SetupNextMenu(&OptionsDef);
}

// [crispy] correctly handle inverted y-axis
static void M_Mouse(int choice)
{
    if (mouseSensitivity_y < 0)
    {
        mouseSensitivity_y = -mouseSensitivity_y;
        mouse_y_invert = 1;
    }

    if (mouse_acceleration_y < 0)
    {
        mouse_acceleration_y = -mouse_acceleration_y;
        mouse_y_invert = 1;
    }

    M_SetupNextMenu(&MouseDef);
}

static void M_CrispnessCur(int choice)
{
    M_SetupNextMenu(CrispnessMenus[crispness_cur]);
}

static void M_CrispnessNext(int choice)
{
    if (++crispness_cur > arrlen(CrispnessMenus) - 1)
    {
	crispness_cur = 0;
    }

    M_CrispnessCur(0);
}

static void M_CrispnessPrev(int choice)
{
    if (--crispness_cur < 0)
    {
	crispness_cur = arrlen(CrispnessMenus) - 1;
    }

    M_CrispnessCur(0);
}


//
//      Toggle messages on/off
//
void M_ChangeMessages(int choice)
{
    // warning: unused parameter `int choice'
    choice = 0;
    showMessages = 1 - showMessages;
	
    if (!showMessages)
	players[consoleplayer].message = DEH_String(MSGOFF);
    else
	players[consoleplayer].message = DEH_String(MSGON);

    message_dontfuckwithme = true;
}


//
// M_EndGame
//
void M_EndGameResponse(int key)
{
    // [crispy] allow to confirm by pressing Enter key
    if (key != key_menu_confirm && key != key_menu_forward)
	return;
		
    // [crispy] killough 5/26/98: make endgame quit if recording or playing back demo
    if (demorecording || singledemo)
	G_CheckDemoStatus();

    // [crispy] clear quicksave slot
    quickSaveSlot = -1;
    currentMenu->lastOn = itemOn;
    M_ClearMenus ();
    D_StartTitle ();
}

void M_EndGame(int choice)
{
    choice = 0;
    if (!usergame)
    {
	S_StartSoundOptional(NULL, sfx_mnuerr, sfx_oof); // [NS] Optional menu sounds.
	return;
    }
	
    if (netgame)
    {
	M_StartMessage(DEH_String(NETEND),NULL,false);
	return;
    }
	
    M_StartMessage(DEH_String(ENDGAME),M_EndGameResponse,true);
}




//
// M_ReadThis
//
void M_ReadThis(int choice)
{
    choice = 0;
    M_SetupNextMenu(&ReadDef1);
}

void M_ReadThis2(int choice)
{
    choice = 0;
    M_SetupNextMenu(&ReadDef2);
}

void M_FinishReadThis(int choice)
{
    choice = 0;
    M_SetupNextMenu(&MainDef);
}




//
// M_QuitDOOM
//
int     quitsounds[8] =
{
    sfx_pldeth,
    sfx_dmpain,
    sfx_popain,
    sfx_slop,
    sfx_telept,
    sfx_posit1,
    sfx_posit3,
    sfx_sgtatk
};

int     quitsounds2[8] =
{
    sfx_vilact,
    sfx_getpow,
    sfx_boscub,
    sfx_slop,
    sfx_skeswg,
    sfx_kntdth,
    sfx_bspact,
    sfx_sgtatk
};



void M_QuitResponse(int key)
{
    extern int show_endoom;

    // [crispy] allow to confirm by pressing Enter key
    if (key != key_menu_confirm && key != key_menu_forward)
	return;
    // [crispy] play quit sound only if the ENDOOM screen is also shown
    if (!netgame && show_endoom)
    {
	if (gamemode == commercial)
	    S_StartSound(NULL,quitsounds2[(gametic>>2)&7]);
	else
	    S_StartSound(NULL,quitsounds[(gametic>>2)&7]);
	I_WaitVBL(105);
    }
    I_Quit ();
}


static const char *M_SelectEndMessage(void)
{
    const char **endmsg;

    if (logical_gamemission == doom)
    {
        // Doom 1

        endmsg = doom1_endmsg;
    }
    else
    {
        // Doom 2
        
        endmsg = doom2_endmsg;
    }

    return endmsg[gametic % NUM_QUITMESSAGES];
}


void M_QuitDOOM(int choice)
{
    // [crispy] fast exit if "run" key is held down
    if (speedkeydown())
	I_Quit();

    DEH_snprintf(endstring, sizeof(endstring), "%s\n\n" DOSY,
                 DEH_String(M_SelectEndMessage()));

    M_StartMessage(endstring,M_QuitResponse,true);
}




void M_ChangeSensitivity(int choice)
{
    switch(choice)
    {
      case 0:
	if (mouseSensitivity)
	    mouseSensitivity--;
	break;
      case 1:
	if (mouseSensitivity < 255) // [crispy] extended range
	    mouseSensitivity++;
	break;
    }
}

void M_ChangeSensitivity_x2(int choice)
{
    switch(choice)
    {
      case 0:
	if (mouseSensitivity_x2)
	    mouseSensitivity_x2--;
	break;
      case 1:
	if (mouseSensitivity_x2 < 255) // [crispy] extended range
	    mouseSensitivity_x2++;
	break;
    }
}

static void M_ChangeSensitivity_y(int choice)
{
    switch(choice)
    {
      case 0:
	if (mouseSensitivity_y)
	    mouseSensitivity_y--;
	break;
      case 1:
	if (mouseSensitivity_y < 255) // [crispy] extended range
	    mouseSensitivity_y++;
	break;
    }
}

static void M_MouseInvert(int choice)
{
    choice = 0;
    mouse_y_invert = !mouse_y_invert;
}


void M_ChangeDetail(int choice)
{
    choice = 0;
    detailLevel = 1 - detailLevel;

    R_SetViewSize (screenblocks, detailLevel);

    if (!detailLevel)
	players[consoleplayer].message = DEH_String(DETAILHI);
    else
	players[consoleplayer].message = DEH_String(DETAILLO);
}




void M_SizeDisplay(int choice)
{
    switch(choice)
    {
      case 0:
	if (screenSize > 0)
	{
	    screenblocks--;
	    screenSize--;
	}
	break;
      case 1:
	if (screenSize < 8 + (crispy->widescreen ? 6 : 3)) // [crispy] Crispy HUD
	{
	    screenblocks++;
	    screenSize++;
	}
	// [crispy] reset to fullscreen HUD
	else
	{
	    screenblocks = 11;
	    screenSize = 8;
	}
	break;
    }
	

    R_SetViewSize (screenblocks, detailLevel);
}




//
//      Menu Functions
//
void
M_DrawThermo
( int	x,
  int	y,
  int	thermWidth,
  int	thermDot )
{
    int		xx;
    int		i;
    char	num[4];

    if (!thermDot)
    {
        dp_translation = cr[CR_DARK];
    }

    xx = x;
    V_DrawPatchDirect(xx, y, W_CacheLumpName(DEH_String("M_THERML"), PU_CACHE));
    xx += 8;
    for (i=0;i<thermWidth;i++)
    {
	V_DrawPatchDirect(xx, y, W_CacheLumpName(DEH_String("M_THERMM"), PU_CACHE));
	xx += 8;
    }
    V_DrawPatchDirect(xx, y, W_CacheLumpName(DEH_String("M_THERMR"), PU_CACHE));

    M_snprintf(num, 4, "%3d", thermDot);
    M_WriteText(xx + 8, y + 3, num);

    // [crispy] do not crash anymore if value exceeds thermometer range
    if (thermDot >= thermWidth)
    {
        thermDot = thermWidth - 1;
        dp_translation = cr[CR_DARK];
    }

    V_DrawPatchDirect((x + 8) + thermDot * 8, y,
		      W_CacheLumpName(DEH_String("M_THERMO"), PU_CACHE));

    dp_translation = NULL;
}


void
M_StartMessage
( const char	*string,
  void*		routine,
  boolean	input )
{
    messageLastMenuActive = menuactive;
    messageToPrint = 1;
    messageString = string;
    messageRoutine = routine;
    messageNeedsInput = input;
    menuactive = true;
    // [crispy] entering menus while recording demos pauses the game
    if (demorecording && !paused)
    {
        sendpause = true;
    }
    return;
}


//
// Find string width from hu_font chars
//
int M_StringWidth(const char *string)
{
    size_t             i;
    int             w = 0;
    int             c;
	
    for (i = 0;i < strlen(string);i++)
    {
	// [crispy] correctly center colorized strings
	if (string[i] == cr_esc)
	{
	    if (string[i+1] >= '0' && string[i+1] <= '0' + CRMAX - 1)
	    {
		i++;
		continue;
	    }
	}

	c = toupper(string[i]) - HU_FONTSTART;
	if (c < 0 || c >= HU_FONTSIZE)
	    w += 4;
	else
	    w += SHORT (hu_font[c]->width);
    }
		
    return w;
}



//
//      Find string height from hu_font chars
//
int M_StringHeight(const char* string)
{
    size_t             i;
    int             h;
    int             height = SHORT(hu_font[0]->height);
	
    h = height;
    for (i = 0;i < strlen(string);i++)
	if (string[i] == '\n')
	    h += height;
		
    return h;
}


//
//      Write a string using the hu_font
//
void
M_WriteText
( int		x,
  int		y,
  const char *string)
{
    int		w;
    const char *ch;
    int		c;
    int		cx;
    int		cy;
		

    ch = string;
    cx = x;
    cy = y;
	
    while(1)
    {
	c = *ch++;
	if (!c)
	    break;
	if (c == '\n')
	{
	    cx = x;
	    cy += 12;
	    continue;
	}
	// [crispy] support multi-colored text
	if (c == cr_esc)
	{
	    if (*ch >= '0' && *ch <= '0' + CRMAX - 1)
	    {
		c = *ch++;
		dp_translation = cr[(int) (c - '0')];
		continue;
	    }
	}
		
	c = toupper(c) - HU_FONTSTART;
	if (c < 0 || c>= HU_FONTSIZE)
	{
	    cx += 4;
	    continue;
	}
		
	w = SHORT (hu_font[c]->width);
	if (cx+w > ORIGWIDTH)
	    break;
	V_DrawPatchDirect(cx, cy, hu_font[c]);
	cx+=w;
    }
}

// These keys evaluate to a "null" key in Vanilla Doom that allows weird
// jumping in the menus. Preserve this behavior for accuracy.

static boolean IsNullKey(int key)
{
    return key == KEY_PAUSE || key == KEY_CAPSLOCK
        || key == KEY_SCRLCK || key == KEY_NUMLOCK;
}

// [crispy] reload current level / go to next level
// adapted from prboom-plus/src/e6y.c:369-449
static int G_ReloadLevel(void)
{
  int result = false;

  if (gamestate == GS_LEVEL)
  {
    // [crispy] restart demos from the map they were started
    if (demorecording)
    {
      gamemap = startmap;
    }
    G_DeferedInitNew(gameskill, gameepisode, gamemap);
    result = true;
  }

  return result;
}

static int G_GotoNextLevel(void)
{
  byte doom_next[6][9] = {
    {12, 13, 19, 15, 16, 17, 18, 21, 14},
    {22, 23, 24, 25, 29, 27, 28, 31, 26},
    {32, 33, 34, 35, 36, 39, 38, 41, 37},
    {42, 49, 44, 45, 46, 47, 48, 51, 43},
    {52, 53, 54, 55, 56, 59, 58, 61, 57},
    {62, 63, 69, 65, 66, 67, 68, 11, 64},
  };
  byte doom2_next[33] = {
     2,  3,  4,  5,  6,  7,  8,  9, 10, 11,
    12, 13, 14, 15, 31, 17, 18, 19, 20, 21,
    22, 23, 24, 25, 26, 27, 28, 29, 30, 1,
    32, 16, 3
  };
  byte nerve_next[9] = {
    2, 3, 4, 9, 6, 7, 8, 1, 5
  };

  int changed = false;

    if (gamemode == commercial)
    {
      if (crispy->havemap33)
        doom2_next[1] = 33;

      if (W_CheckNumForName("map31") < 0)
        doom2_next[14] = 16;

      if (gamemission == pack_hacx)
      {
        doom2_next[30] = 16;
        doom2_next[20] = 1;
      }

      if (gamemission == pack_master)
      {
        doom2_next[1] = 3;
        doom2_next[14] = 16;
        doom2_next[20] = 1;
      }
    }
    else
    {
      if (gamemode == shareware)
        doom_next[0][7] = 11;

      if (gamemode == registered)
        doom_next[2][7] = 11;

      // [crispy] Sigil and Sigil II
      if (!crispy->haved1e5 && !crispy->haved1e6)
        doom_next[3][7] = 11;
      else if (!crispy->haved1e5 && crispy->haved1e6)
        doom_next[3][7] = 61;
      else if (crispy->haved1e5 && !crispy->haved1e6)
        doom_next[4][7] = 11;

      if (gameversion == exe_chex)
      {
        doom_next[0][2] = 14;
        doom_next[0][4] = 11;
      }
    }

  if (gamestate == GS_LEVEL)
  {
    int epsd, map;

    if (gamemode == commercial)
    {
      epsd = gameepisode;
      if (gamemission == pack_nerve)
        map = nerve_next[gamemap-1];
      else
        map = doom2_next[gamemap-1];
    }
    else
    {
      epsd = doom_next[gameepisode-1][gamemap-1] / 10;
      map = doom_next[gameepisode-1][gamemap-1] % 10;
    }

    // [crispy] special-casing for E1M10 "Sewers" support
    if (crispy->havee1m10 && gameepisode == 1)
    {
	if (gamemap == 1)
	{
	    map = 10;
	}
	else
	if (gamemap == 10)
	{
	    epsd = 1;
	    map = 2;
	}
    }

    G_DeferedInitNew(gameskill, epsd, map);
    changed = true;
  }

  return changed;
}

//
// CONTROL PANEL
//

//
// M_Responder
//
boolean M_Responder (event_t* ev)
{
    int             ch;
    int             key;
    int             i;
    static  int     mousewait = 0;
    static  int     mousey = 0;
    static  int     lasty = 0;
    static  int     mousex = 0;
    static  int     lastx = 0;
    boolean mousextobutton = false;
    int dir;

    // In testcontrols mode, none of the function keys should do anything
    // - the only key is escape to quit.

    if (testcontrols)
    {
        if (ev->type == ev_quit
         || (ev->type == ev_keydown
          && (ev->data1 == key_menu_activate || ev->data1 == key_menu_quit)))
        {
            I_Quit();
            return true;
        }

        return false;
    }

    // "close" button pressed on window?
    if (ev->type == ev_quit)
    {
        // First click on close button = bring up quit confirm message.
        // Second click on close button = confirm quit

        if (menuactive && messageToPrint && messageRoutine == M_QuitResponse)
        {
            M_QuitResponse(key_menu_confirm);
        }
        else
        {
            S_StartSoundOptional(NULL, sfx_mnuopn, sfx_swtchn); // [NS] Optional menu sounds.
            M_QuitDOOM(0);
        }

        return true;
    }

    // key is the key pressed, ch is the actual character typed
  
    ch = 0;
    key = -1;
	
    if (ev->type == ev_joystick)
    {
        // Simulate key presses from joystick events to interact with the menu.

        if (menuactive)
        {
            if (JOY_GET_DPAD(ev->data6) != JOY_DIR_NONE)
            {
                dir = JOY_GET_DPAD(ev->data6);
            }
            else if (JOY_GET_LSTICK(ev->data6) != JOY_DIR_NONE)
            {
                dir = JOY_GET_LSTICK(ev->data6);
            }
            else
            {
                dir = JOY_GET_RSTICK(ev->data6);
            }

            if (dir & JOY_DIR_UP)
            {
                key = key_menu_up;
                joywait = I_GetTime() + 5;
            }
            else if (dir & JOY_DIR_DOWN)
            {
                key = key_menu_down;
                joywait = I_GetTime() + 5;
            }
            if (dir & JOY_DIR_LEFT)
            {
                key = key_menu_left;
                joywait = I_GetTime() + 5;
            }
            else if (dir & JOY_DIR_RIGHT)
            {
                key = key_menu_right;
                joywait = I_GetTime() + 5;
            }

#define JOY_BUTTON_MAPPED(x) ((x) >= 0)
#define JOY_BUTTON_PRESSED(x) (JOY_BUTTON_MAPPED(x) && (ev->data1 & (1 << (x))) != 0)

            if (JOY_BUTTON_PRESSED(joybfire))
            {
                // Simulate a 'Y' keypress when Doom show a Y/N dialog with Fire button.
                if (messageToPrint && messageNeedsInput)
                {
                    key = key_menu_confirm;
                }
                // Simulate pressing "Enter" when we are supplying a save slot name
                else if (saveStringEnter)
                {
                    key = KEY_ENTER;
                }
                else
                {
                    // if selecting a save slot via joypad, set a flag
                    if (currentMenu == &SaveDef)
                    {
                        joypadSave = true;
                    }
                    key = key_menu_forward;
                }
                joywait = I_GetTime() + 5;
            }
            if (JOY_BUTTON_PRESSED(joybuse))
            {
                // Simulate a 'N' keypress when Doom show a Y/N dialog with Use button.
                if (messageToPrint && messageNeedsInput)
                {
                    key = key_menu_abort;
                }
                // If user was entering a save name, back out
                else if (saveStringEnter)
                {
                    key = KEY_ESCAPE;
                }
                else
                {
                    key = key_menu_back;
                }
                joywait = I_GetTime() + 5;
            }
        }
        if (JOY_BUTTON_PRESSED(joybmenu))
        {
            key = key_menu_activate;
            joywait = I_GetTime() + 5;
        }
    }
    else
    {
	if (ev->type == ev_mouse && mousewait < I_GetTime() && menuactive)
	{
	    // [crispy] novert disables controlling the menus with the mouse
	    if (!novert)
	    {
	    mousey += ev->data3;
	    }
	    if (mousey < lasty-30)
	    {
		key = key_menu_down;
		mousewait = I_GetTime() + 5;
		mousey = lasty -= 30;
	    }
	    else if (mousey > lasty+30)
	    {
		key = key_menu_up;
		mousewait = I_GetTime() + 5;
		mousey = lasty += 30;
	    }
		
	    mousex += ev->data2;
	    if (mousex < lastx-30)
	    {
		key = key_menu_left;
		mousewait = I_GetTime() + 5;
		mousex = lastx -= 30;
		mousextobutton = true;
	    }
	    else if (mousex > lastx+30)
	    {
		key = key_menu_right;
		mousewait = I_GetTime() + 5;
		mousex = lastx += 30;
		mousextobutton = true;
	    }
		
	    if (ev->data1&1)
	    {
		key = key_menu_forward;
		mousewait = I_GetTime() + 5;
	    }
			
	    if (ev->data1&2)
	    {
		key = key_menu_back;
		mousewait = I_GetTime() + 5;
	    }

	    // [crispy] scroll menus with mouse wheel
	    if (mousebprevweapon >= 0 && ev->data1 & (1 << mousebprevweapon))
	    {
		key = key_menu_down;
		mousewait = I_GetTime() + 1;
	    }
	    else
	    if (mousebnextweapon >= 0 && ev->data1 & (1 << mousebnextweapon))
	    {
		key = key_menu_up;
		mousewait = I_GetTime() + 1;
	    }
	}
	else
	{
	    if (ev->type == ev_keydown)
	    {
		key = ev->data1;
		ch = ev->data2;
	    }
	}
    }
    
    if (key == -1)
	return false;

    // Save Game string input
    if (saveStringEnter)
    {
	switch(key)
	{
	  case KEY_BACKSPACE:
	    if (saveCharIndex > 0)
	    {
		saveCharIndex--;
		savegamestrings[saveSlot][saveCharIndex] = 0;
	    }
	    break;

          case KEY_ESCAPE:
            saveStringEnter = 0;
            I_StopTextInput();
            M_StringCopy(savegamestrings[saveSlot], saveOldString,
                         SAVESTRINGSIZE);
            break;

	  case KEY_ENTER:
	    saveStringEnter = 0;
            I_StopTextInput();
	    if (savegamestrings[saveSlot][0])
		M_DoSave(saveSlot);
	    break;

	  default:
            // Savegame name entry. This is complicated.
            // Vanilla has a bug where the shift key is ignored when entering
            // a savegame name. If vanilla_keyboard_mapping is on, we want
            // to emulate this bug by using ev->data1. But if it's turned off,
            // it implies the user doesn't care about Vanilla emulation:
            // instead, use ev->data3 which gives the fully-translated and
            // modified key input.

            if (ev->type != ev_keydown)
            {
                break;
            }
            if (vanilla_keyboard_mapping)
            {
                ch = ev->data1;
            }
            else
            {
                ch = ev->data3;
            }

            ch = toupper(ch);

            if (ch != ' '
             && (ch - HU_FONTSTART < 0 || ch - HU_FONTSTART >= HU_FONTSIZE))
            {
                break;
            }

	    if (ch >= 32 && ch <= 127 &&
		saveCharIndex < SAVESTRINGSIZE-1 &&
		M_StringWidth(savegamestrings[saveSlot]) <
		(SAVESTRINGSIZE-2)*8)
	    {
		savegamestrings[saveSlot][saveCharIndex++] = ch;
		savegamestrings[saveSlot][saveCharIndex] = 0;
	    }
	    break;
	}
	return true;
    }

    // [crispy] Enter numeric value
    if (numeric_enter)
    {
        switch(key)
        {
            case KEY_BACKSPACE:
                if (numeric_entry_index > 0)
                {
                    numeric_entry_index--;
                    numeric_entry_str[numeric_entry_index] = '\0';
                }
                break;
            case KEY_ESCAPE:
                numeric_enter = false;
                I_StopTextInput();
                break;
            case KEY_ENTER:
                if (numeric_entry_str[0] != '\0')
                {
                    numeric_entry = atoi(numeric_entry_str);
                    currentMenu->menuitems[itemOn].routine(2);
                }
                else
                {
                    numeric_enter = false;
                    I_StopTextInput();
                }
                break;
            default:
                if (ev->type != ev_keydown)
                {
                    break;
                }

                if (vanilla_keyboard_mapping)
                {
                    ch = ev->data1;
                }
                else
                {
                    ch = ev->data3;
                }

                if (ch >= '0' && ch <= '9' &&
                        numeric_entry_index < NUMERIC_ENTRY_NUMDIGITS)
                {
                    numeric_entry_str[numeric_entry_index++] = ch;
                    numeric_entry_str[numeric_entry_index] = '\0';
                }
                else
                {
                    break;
                }
        }
        return true;
    }

    // Take care of any messages that need input
    if (messageToPrint)
    {
	if (messageNeedsInput)
        {
            if (key != ' ' && key != KEY_ESCAPE
             && key != key_menu_confirm && key != key_menu_abort
             // [crispy] allow to confirm nightmare, end game and quit by pressing Enter key
             && key != key_menu_forward)
            {
                return false;
            }
	}

	menuactive = messageLastMenuActive;
	if (messageRoutine)
	    messageRoutine(key);

	// [crispy] stay in menu
	if (messageToPrint < 2)
	{
	menuactive = false;
	}
	messageToPrint = 0; // [crispy] moved here
	S_StartSoundOptional(NULL, sfx_mnucls, sfx_swtchx); // [NS] Optional menu sounds.
	return true;
    }

    // [crispy] take screen shot without weapons and HUD
    if (key != 0 && key == key_menu_cleanscreenshot)
    {
	crispy->cleanscreenshot = (screenblocks > 10) ? 2 : 1;
    }

    if ((devparm && key == key_menu_help) ||
        (key != 0 && (key == key_menu_screenshot || key == key_menu_cleanscreenshot)))
    {
	G_ScreenShot ();
	return true;
    }

    // F-Keys
    if (!menuactive)
    {
	if (key == key_menu_decscreen)      // Screen size down
        {
	    if (automapactive || chat_on)
		return false;
	    M_SizeDisplay(0);
	    S_StartSoundOptional(NULL, sfx_mnusli, sfx_stnmov); // [NS] Optional menu sounds.
	    return true;
	}
        else if (key == key_menu_incscreen) // Screen size up
        {
	    if (automapactive || chat_on)
		return false;
	    M_SizeDisplay(1);
	    S_StartSoundOptional(NULL, sfx_mnusli, sfx_stnmov); // [NS] Optional menu sounds.
	    return true;
	}
        else if (key == key_menu_help)     // Help key
        {
	    M_StartControlPanel ();

	    if (gameversion >= exe_ultimate)
	      currentMenu = &ReadDef2;
	    else
	      currentMenu = &ReadDef1;

	    itemOn = 0;
	    S_StartSoundOptional(NULL, sfx_mnuopn, sfx_swtchn); // [NS] Optional menu sounds.
	    return true;
	}
        else if (key == key_menu_save)     // Save
        {
	    M_StartControlPanel();
	    S_StartSoundOptional(NULL, sfx_mnuopn, sfx_swtchn); // [NS] Optional menu sounds.
	    M_SaveGame(0);
	    return true;
        }
        else if (key == key_menu_load)     // Load
        {
	    M_StartControlPanel();
	    S_StartSoundOptional(NULL, sfx_mnuopn, sfx_swtchn); // [NS] Optional menu sounds.
	    M_LoadGame(0);
	    return true;
        }
        else if (key == key_menu_volume)   // Sound Volume
        {
	    M_StartControlPanel ();
	    currentMenu = &SoundDef;
	    itemOn = currentMenu->lastOn; // [crispy] remember cursor position
	    S_StartSoundOptional(NULL, sfx_mnuopn, sfx_swtchn); // [NS] Optional menu sounds.
	    return true;
	}
        else if (key == key_menu_detail)   // Detail toggle
        {
	    M_ChangeDetail(0);
	    S_StartSoundOptional(NULL, sfx_mnusli, sfx_swtchn); // [NS] Optional menu sounds.
	    return true;
        }
        else if (key == key_menu_qsave)    // Quicksave
        {
	    S_StartSoundOptional(NULL, sfx_mnuopn, sfx_swtchn); // [NS] Optional menu sounds.
	    M_QuickSave();
	    return true;
        }
        else if (key == key_menu_endgame)  // End game
        {
	    S_StartSoundOptional(NULL, sfx_mnuopn, sfx_swtchn); // [NS] Optional menu sounds.
	    M_EndGame(0);
	    return true;
        }
        else if (key == key_menu_messages) // Toggle messages
        {
	    M_ChangeMessages(0);
	    S_StartSoundOptional(NULL, sfx_mnusli, sfx_swtchn); // [NS] Optional menu sounds.
	    return true;
        }
        else if (key == key_menu_qload)    // Quickload
        {
	    S_StartSoundOptional(NULL, sfx_mnuopn, sfx_swtchn); // [NS] Optional menu sounds.
	    M_QuickLoad();
	    return true;
        }
        else if (key == key_menu_quit)     // Quit DOOM
        {
	    S_StartSoundOptional(NULL, sfx_mnuopn, sfx_swtchn); // [NS] Optional menu sounds.
	    M_QuitDOOM(0);
	    return true;
        }
        else if (key == key_menu_gamma)    // gamma toggle
        {
	    crispy->gamma++;
	    if (crispy->gamma > 4+13) // [crispy] intermediate gamma levels
		crispy->gamma = 0;
	    players[consoleplayer].message = DEH_String(gammamsg[crispy->gamma]);
#ifndef CRISPY_TRUECOLOR
            I_SetPalette (W_CacheLumpName (DEH_String("PLAYPAL"),PU_CACHE));
#else
            {
		I_SetPalette (0);
		R_InitColormaps();
		inhelpscreens = true;
		R_FillBackScreen();
		viewactive = false;
            }
#endif
	    return true;
	}
        // [crispy] those two can be considered as shortcuts for the IDCLEV cheat
        // and should be treated as such, i.e. add "if (!netgame)"
        // hovewer, allow while multiplayer demos
        else if ((!netgame || netdemo) && key != 0 && key == key_menu_reloadlevel)
        {
	    if (demoplayback)         
	    {
		if (crispy->demowarp)
		{
		// [crispy] enable screen render back before replaying
		nodrawers = false;
		singletics = false;
		}
		// [crispy] replay demo lump or file
		G_DoPlayDemo();
		return true;
	    }
	    else
	    if (G_ReloadLevel())
		return true;
        }
        else if ((!netgame || netdemo) && key != 0 && key == key_menu_nextlevel)
        {
	    if (demoplayback)
	    {
		// [crispy] go to next level
		demo_gotonextlvl = true;
		G_DemoGotoNextLevel(true);
		return true;
	    }
	    else
	    if (G_GotoNextLevel())
		return true;
        }

    }

    // Pop-up menu?
    if (!menuactive)
    {
	if (key == key_menu_activate)
	{
	    M_StartControlPanel ();
	    S_StartSoundOptional(NULL, sfx_mnuopn, sfx_swtchn); // [NS] Optional menu sounds.
	    return true;
	}
	return false;
    }

    // Keys usable within menu

    if (key == key_menu_down)
    {
        // Move down to next item

        do
	{
	    if (itemOn+1 > currentMenu->numitems-1)
		itemOn = 0;
	    else itemOn++;
	    S_StartSoundOptional(NULL, sfx_mnumov, sfx_pstop); // [NS] Optional menu sounds.
	} while(currentMenu->menuitems[itemOn].status==-1);

	return true;
    }
    else if (key == key_menu_up)
    {
        // Move back up to previous item

	do
	{
	    if (!itemOn)
		itemOn = currentMenu->numitems-1;
	    else itemOn--;
	    S_StartSoundOptional(NULL, sfx_mnumov, sfx_pstop); // [NS] Optional menu sounds.
	} while(currentMenu->menuitems[itemOn].status==-1);

	return true;
    }
    else if (key == key_menu_left)
    {
        // Slide slider left

	if (currentMenu->menuitems[itemOn].routine &&
	    currentMenu->menuitems[itemOn].status)
	{
            if (currentMenu->menuitems[itemOn].status == 2)
            {
                S_StartSoundOptional(NULL, sfx_mnusli, sfx_stnmov); // [NS] Optional menu sounds.
                currentMenu->menuitems[itemOn].routine(0);
            }
            // [crispy] LR non-slider
            else if (currentMenu->menuitems[itemOn].status == 3 && !mousextobutton)
            {
                S_StartSoundOptional(NULL, sfx_mnuact, sfx_pistol); // [NS] Optional menu sounds.
                currentMenu->menuitems[itemOn].routine(0);
            }
            // [crispy] Numeric entry
            else if (currentMenu->menuitems[itemOn].status == 4 && !mousextobutton)
            {
                S_StartSoundOptional(NULL, sfx_mnusli, sfx_stnmov); // [NS] Optional menu sounds.
                currentMenu->menuitems[itemOn].routine(0);
            }
        }
	return true;
    }
    else if (key == key_menu_right)
    {
        // Slide slider right

	if (currentMenu->menuitems[itemOn].routine &&
	    currentMenu->menuitems[itemOn].status)
	{
            if (currentMenu->menuitems[itemOn].status == 2)
            {
                S_StartSoundOptional(NULL, sfx_mnusli, sfx_stnmov); // [NS] Optional menu sounds.
                currentMenu->menuitems[itemOn].routine(1);
            }
            // [crispy] LR non-slider
            else if (currentMenu->menuitems[itemOn].status == 3 && !mousextobutton)
            {
                S_StartSoundOptional(NULL, sfx_mnuact, sfx_pistol); // [NS] Optional menu sounds.
                currentMenu->menuitems[itemOn].routine(1);
            }
            // [crispy] Numeric entry
            else if (currentMenu->menuitems[itemOn].status == 4 && !mousextobutton)
            {
                S_StartSoundOptional(NULL, sfx_mnusli, sfx_stnmov); // [NS] Optional menu sounds.
                currentMenu->menuitems[itemOn].routine(1);
            }
        }
	return true;
    }
    else if (key == key_menu_forward)
    {
        // Activate menu item

	if (currentMenu->menuitems[itemOn].routine &&
	    currentMenu->menuitems[itemOn].status)
	{
	    currentMenu->lastOn = itemOn;
	    if (currentMenu->menuitems[itemOn].status == 2)
	    {
		currentMenu->menuitems[itemOn].routine(1);      // right arrow
		S_StartSoundOptional(NULL, sfx_mnusli, sfx_stnmov); // [NS] Optional menu sounds.
	    }
            else if (currentMenu->menuitems[itemOn].status == 3)
            {
                currentMenu->menuitems[itemOn].routine(1); // right arrow
                S_StartSoundOptional(NULL, sfx_mnuact, sfx_pistol); // [NS] Optional menu sounds.
            }
            else if (currentMenu->menuitems[itemOn].status == 4) // [crispy]
            {
                currentMenu->menuitems[itemOn].routine(2); // enter key
                numeric_entry_index = 0;
                numeric_entry_str[0] = '\0';
                S_StartSoundOptional(NULL, sfx_mnuact, sfx_pistol);
            }
	    else
	    {
		currentMenu->menuitems[itemOn].routine(itemOn);
		S_StartSoundOptional(NULL, sfx_mnuact, sfx_pistol); // [NS] Optional menu sounds.
	    }
	}
	return true;
    }
    else if (key == key_menu_activate)
    {
        // Deactivate menu

	currentMenu->lastOn = itemOn;
	M_ClearMenus ();
	S_StartSoundOptional(NULL, sfx_mnucls, sfx_swtchx); // [NS] Optional menu sounds.
	return true;
    }
    else if (key == key_menu_back)
    {
        // Go back to previous menu

	currentMenu->lastOn = itemOn;
	if (currentMenu->prevMenu)
	{
	    currentMenu = currentMenu->prevMenu;
	    itemOn = currentMenu->lastOn;
	    S_StartSoundOptional(NULL, sfx_mnubak, sfx_swtchn); // [NS] Optional menu sounds.
	}
	return true;
    }
    // [crispy] delete a savegame
    else if (key == key_menu_del)
    {
	if (currentMenu == &LoadDef || currentMenu == &SaveDef)
	{
	    if (LoadMenu[itemOn].status)
	    {
		currentMenu->lastOn = itemOn;
		M_ConfirmDeleteGame();
		return true;
	    }
	    else
	    {
		S_StartSoundOptional(NULL, sfx_mnuerr, sfx_oof); // [NS] Optional menu sounds.
	    }
	}
    }
    // [crispy] next/prev Crispness menu
    else if (key == KEY_PGUP)
    {
	currentMenu->lastOn = itemOn;
	if (currentMenu == CrispnessMenus[crispness_cur])
	{
	    M_CrispnessPrev(0);
	    S_StartSoundOptional(NULL, sfx_mnuact, sfx_swtchn); // [NS] Optional menu sounds.
	    return true;
	}
	else if (currentMenu == &LoadDef || currentMenu == &SaveDef)
	{
	    if (savepage > 0)
	    {
		savepage--;
		quickSaveSlot = -1;
		M_ReadSaveStrings();
		S_StartSoundOptional(NULL, sfx_mnumov, sfx_pstop);
	    }
	    return true;
	}
    }
    else if (key == KEY_PGDN)
    {
	currentMenu->lastOn = itemOn;
	if (currentMenu == CrispnessMenus[crispness_cur])
	{
	    M_CrispnessNext(0);
	    S_StartSoundOptional(NULL, sfx_mnuact, sfx_swtchn); // [NS] Optional menu sounds.
	    return true;
	}
	else if (currentMenu == &LoadDef || currentMenu == &SaveDef)
	{
	    if (savepage < savepage_max)
	    {
		savepage++;
		quickSaveSlot = -1;
		M_ReadSaveStrings();
		S_StartSoundOptional(NULL, sfx_mnumov, sfx_pstop);
	    }
	    return true;
	}
    }

    // Keyboard shortcut?
    // Vanilla Doom has a weird behavior where it jumps to the scroll bars
    // when the certain keys are pressed, so emulate this.

    else if (ch != 0 || IsNullKey(key))
    {
	for (i = itemOn+1;i < currentMenu->numitems;i++)
        {
	    if (currentMenu->menuitems[i].alphaKey == ch)
	    {
		itemOn = i;
		S_StartSoundOptional(NULL, sfx_mnumov, sfx_pstop); // [NS] Optional menu sounds.
		return true;
	    }
        }

	for (i = 0;i <= itemOn;i++)
        {
	    if (currentMenu->menuitems[i].alphaKey == ch)
	    {
		itemOn = i;
		S_StartSoundOptional(NULL, sfx_mnumov, sfx_pstop); // [NS] Optional menu sounds.
		return true;
	    }
        }
    }

    return false;
}



//
// M_StartControlPanel
//
void M_StartControlPanel (void)
{
    // intro might call this repeatedly
    if (menuactive)
	return;

    // [crispy] entering menus while recording demos pauses the game
    if (demorecording && !paused)
        sendpause = true;
    
    menuactive = 1;
    currentMenu = &MainDef;         // JDC
    itemOn = currentMenu->lastOn;   // JDC
}

// Display OPL debug messages - hack for GENMIDI development.

static void M_DrawOPLDev(void)
{
    char debug[1024];
    char *curr, *p;
    int line;

    I_OPL_DevMessages(debug, sizeof(debug));
    curr = debug;
    line = 0;

    for (;;)
    {
        p = strchr(curr, '\n');

        if (p != NULL)
        {
            *p = '\0';
        }

        M_WriteText(0, line * 8, curr);
        ++line;

        if (p == NULL)
        {
            break;
        }

        curr = p + 1;
    }
}

//
// M_Drawer
// Called after the view has been rendered,
// but before it has been blitted.
//
void M_Drawer (void)
{
    static short	x;
    static short	y;
    unsigned int	i;
    unsigned int	max;
    char		string[80];
    const char          *name;
    int			start;

    inhelpscreens = false;
    
    // Horiz. & Vertically center string and print it.
    if (messageToPrint)
    {
	// [crispy] draw a background for important questions
	if (messageToPrint == 2)
	{
	    M_DrawCrispnessBackground();
	}

	start = 0;
	y = ORIGHEIGHT/2 - M_StringHeight(messageString) / 2;
	while (messageString[start] != '\0')
	{
	    boolean foundnewline = false;

            for (i = 0; messageString[start + i] != '\0'; i++)
            {
                if (messageString[start + i] == '\n')
                {
                    M_StringCopy(string, messageString + start,
                                 sizeof(string));
                    if (i < sizeof(string))
                    {
                        string[i] = '\0';
                    }

                    foundnewline = true;
                    start += i + 1;
                    break;
                }
            }

            if (!foundnewline)
            {
                M_StringCopy(string, messageString + start, sizeof(string));
                start += strlen(string);
            }

	    x = ORIGWIDTH/2 - M_StringWidth(string) / 2;
	    M_WriteText(x > 0 ? x : 0, y, string); // [crispy] prevent negative x-coords
	    y += SHORT(hu_font[0]->height);
	}

	return;
    }

    if (opldev)
    {
        M_DrawOPLDev();
    }

    if (!menuactive)
	return;

    if (currentMenu->routine)
	currentMenu->routine();         // call Draw routine
    
    // DRAW MENU
    x = currentMenu->x;
    y = currentMenu->y;
    max = currentMenu->numitems;

    // [crispy] check current menu for missing menu graphics lumps - only once
    if (currentMenu->lumps_missing == 0)
    {
        for (i = 0; i < max; i++)
            if (currentMenu->menuitems[i].name[0])
                if (W_CheckNumForName(currentMenu->menuitems[i].name) < 0)
                    currentMenu->lumps_missing++;

        // [crispy] no lump missing, no need to check again
        if (currentMenu->lumps_missing == 0)
            currentMenu->lumps_missing = -1;
    }

    for (i=0;i<max;i++)
    {
        const char *alttext = currentMenu->menuitems[i].alttext;
        name = DEH_String(currentMenu->menuitems[i].name);

	if (name[0] && (W_CheckNumForName(name) > 0 || alttext))
	{
	    if (W_CheckNumForName(name) > 0 && currentMenu->lumps_missing == -1)
	    V_DrawPatchDirect (x, y, W_CacheLumpName(name, PU_CACHE));
	    else if (alttext)
		M_WriteText(x, y+8-(M_StringHeight(alttext)/2), alttext);
	}
	y += LINEHEIGHT;
    }

    
    // DRAW SKULL
    if (currentMenu == CrispnessMenus[crispness_cur])
    {
	char item[4];
	M_snprintf(item, sizeof(item), "%s>", whichSkull ? crstr[CR_NONE] : crstr[CR_DARK]);
	M_WriteText(currentMenu->x - 8, currentMenu->y + CRISPY_LINEHEIGHT * itemOn, item);
	dp_translation = NULL;
    }
    else
    V_DrawPatchDirect(x + SKULLXOFF, currentMenu->y - 5 + itemOn*LINEHEIGHT,
		      W_CacheLumpName(DEH_String(skullName[whichSkull]),
				      PU_CACHE));
}


//
// M_ClearMenus
//
void M_ClearMenus (void)
{
    menuactive = 0;

    // [crispy] entering menus while recording demos pauses the game
    if (demorecording && paused)
        sendpause = true;

    // if (!netgame && usergame && paused)
    //       sendpause = true;
}




//
// M_SetupNextMenu
//
void M_SetupNextMenu(menu_t *menudef)
{
    currentMenu = menudef;
    itemOn = currentMenu->lastOn;
}


//
// M_Ticker
//
void M_Ticker (void)
{
    if (--skullAnimCounter <= 0)
    {
	whichSkull ^= 1;
	skullAnimCounter = 8;
    }
}

// [crispy]
void M_SetDefaultDifficulty (void)
{
    // HMP (or skill #2) being the default, had to be placed at index 0 when drawn in the menu,
    // so all difficulties 'real' positions had to be scaled by -2, hence +2 being added
    // below in order to get the correct skill index when getting it from the skill enum.
    NewDef.lastOn = ((crispy->defaultskill) + SKILL_HMP ) % NUM_SKILLS;
}

//
// M_Init
//
void M_Init (void)
{
    currentMenu = &MainDef;
    menuactive = 0;
    itemOn = currentMenu->lastOn;
    whichSkull = 0;
    skullAnimCounter = 10;
    screenSize = screenblocks - 3;
    messageToPrint = 0;
    messageString = NULL;
    messageLastMenuActive = menuactive;
    quickSaveSlot = -1;

    M_SetDefaultDifficulty(); // [crispy] pre-select default difficulty

    // Here we could catch other version dependencies,
    //  like HELP1/2, and four episodes.

    // The same hacks were used in the original Doom EXEs.

    if (gameversion >= exe_ultimate)
    {
        MainMenu[readthis].routine = M_ReadThis2;
        ReadDef2.prevMenu = NULL;
    }

    if (gameversion >= exe_final && gameversion <= exe_final2)
    {
        ReadDef2.routine = M_DrawReadThisCommercial;
        // [crispy] rearrange Skull in Final Doom HELP screen
        ReadDef2.y -= 10;
    }

    if (gamemode == commercial)
    {
        MainMenu[readthis] = MainMenu[quitdoom];
        MainDef.numitems--;
        MainDef.y += 8;
        NewDef.prevMenu = &MainDef;
        ReadDef1.routine = M_DrawReadThisCommercial;
        ReadDef1.x = 330;
        ReadDef1.y = 165;
        ReadMenu1[rdthsempty1].routine = M_FinishReadThis;
    }

    // [crispy] Sigil
    if (!crispy->haved1e5 && !crispy->haved1e6)
    {
        EpiDef.numitems = 4;
    }
    else if (crispy->haved1e5 != crispy->haved1e6)
    {
        EpiDef.numitems = 5;
        if (crispy->haved1e6)
        {
            EpiDef.menuitems = EpisodeMenuSII;
        }
    }

    // Versions of doom.exe before the Ultimate Doom release only had
    // three episodes; if we're emulating one of those then don't try
    // to show episode four. If we are, then do show episode four
    // (should crash if missing).
    if (gameversion < exe_ultimate)
    {
        EpiDef.numitems--;
    }
    // chex.exe shows only one episode.
    else if (gameversion == exe_chex)
    {
        EpiDef.numitems = 1;
        // [crispy] never show the Episode menu
        NewDef.prevMenu = &MainDef;
    }

    // [crispy] NRFTL / The Master Levels
    if (crispy->havenerve || crispy->havemaster)
    {
        int i, j;

        NewDef.prevMenu = &EpiDef;
        EpisodeMenu[0].alphaKey = gamevariant == freedm ||
                                  gamevariant == freedoom ?
                                 'f' :
                                 'h';
        EpisodeMenu[0].alttext = gamevariant == freedm ?
                                 "FreeDM" :
                                 gamevariant == freedoom ?
                                 "Freedoom: Phase 2" :
                                 "Hell on Earth";
        EpiDef.numitems = 1;

        if (crispy->havenerve)
        {
            EpisodeMenu[EpiDef.numitems].alphaKey = 'n';
            EpisodeMenu[EpiDef.numitems].alttext = "No Rest for the Living";
            EpiDef.numitems++;

            i = W_CheckNumForName("M_EPI1");
            j = W_CheckNumForName("M_EPI2");

            // [crispy] render the episode menu with the HUD font ...
            // ... if the graphics are not available
            if (i != -1 && j != -1)
            {
                // ... or if the graphics are both from an IWAD
                if (W_IsIWADLump(lumpinfo[i]) && W_IsIWADLump(lumpinfo[j]))
                {
                    const patch_t *pi, *pj;

                    pi = W_CacheLumpNum(i, PU_CACHE);
                    pj = W_CacheLumpNum(j, PU_CACHE);

                    // ... and if the patch width for "Hell on Earth"
                    //     is longer than "No Rest for the Living"
                    if (SHORT(pi->width) > SHORT(pj->width))
                    {
                        EpiDef.lumps_missing = 1;
                    }
                }
            }
            else
            {
                EpiDef.lumps_missing = 1;
            }
        }

        if (crispy->havemaster)
        {
            EpisodeMenu[EpiDef.numitems].alphaKey = 't';
            EpisodeMenu[EpiDef.numitems].alttext = "The Master Levels";
            EpiDef.numitems++;

            i = W_CheckNumForName(EpiDef.numitems == 3 ? "M_EPI3" : "M_EPI2");

            // [crispy] render the episode menu with the HUD font
            // if the graphics are not available or not from a PWAD
            if (i == -1 || W_IsIWADLump(lumpinfo[i]))
            {
                EpiDef.lumps_missing = 1;
            }
        }
    }

    // [crispy] rearrange Load Game and Save Game menus
    {
	const patch_t *patchl, *patchs, *patchm;
	short captionheight, vstep;

	patchl = W_CacheLumpName(DEH_String("M_LOADG"), PU_CACHE);
	patchs = W_CacheLumpName(DEH_String("M_SAVEG"), PU_CACHE);
	patchm = W_CacheLumpName(DEH_String("M_LSLEFT"), PU_CACHE);

	LoadDef_x = (ORIGWIDTH - SHORT(patchl->width)) / 2 + SHORT(patchl->leftoffset);
	SaveDef_x = (ORIGWIDTH - SHORT(patchs->width)) / 2 + SHORT(patchs->leftoffset);
	LoadDef.x = SaveDef.x = (ORIGWIDTH - 24 * 8) / 2 + SHORT(patchm->leftoffset); // [crispy] see M_DrawSaveLoadBorder()

	captionheight = MAX(SHORT(patchl->height), SHORT(patchs->height));

	vstep = ORIGHEIGHT - 32; // [crispy] ST_HEIGHT
	vstep -= captionheight;
	vstep -= (load_end - 1) * LINEHEIGHT + SHORT(patchm->height);
	vstep /= 3;

	if (vstep > 0)
	{
		LoadDef_y = vstep + captionheight - SHORT(patchl->height) + SHORT(patchl->topoffset);
		SaveDef_y = vstep + captionheight - SHORT(patchs->height) + SHORT(patchs->topoffset);
		LoadDef.y = SaveDef.y = vstep + captionheight + vstep + SHORT(patchm->topoffset) - 15; // [crispy] moved up, so savegame date/time may appear above status bar
		MouseDef.y = LoadDef.y;
	}
    }

    // [crispy] remove DOS reference from the game quit confirmation dialogs
    if (!M_ParmExists("-nodeh"))
    {
	const char *string;
	char *replace;

	// [crispy] "i wouldn't leave if i were you.\ndos is much worse."
	string = doom1_endmsg[3];
	if (!DEH_HasStringReplacement(string))
	{
		replace = M_StringReplace(string, "dos", crispy->platform);
		DEH_AddStringReplacement(string, replace);
		free(replace);
	}

	// [crispy] "you're trying to say you like dos\nbetter than me, right?"
	string = doom1_endmsg[4];
	if (!DEH_HasStringReplacement(string))
	{
		replace = M_StringReplace(string, "dos", crispy->platform);
		DEH_AddStringReplacement(string, replace);
		free(replace);
	}

	// [crispy] "don't go now, there's a \ndimensional shambler waiting\nat the dos prompt!"
	string = doom2_endmsg[2];
	if (!DEH_HasStringReplacement(string))
	{
		replace = M_StringReplace(string, "dos", "command");
		DEH_AddStringReplacement(string, replace);
		free(replace);
	}
    }

    opldev = M_CheckParm("-opldev") > 0;
}

// [crispy] extended savegames
static char *savegwarning;
static void M_ForceLoadGameResponse(int key)
{
	free(savegwarning);
	free(savewadfilename);

	if (key != key_menu_confirm || !savemaplumpinfo)
	{
		// [crispy] no need to end game anymore when denied to load savegame
		//M_EndGameResponse(key_menu_confirm);
		savewadfilename = NULL;

		// [crispy] reload Load Game menu
		M_StartControlPanel();
		M_LoadGame(0);
		return;
	}

	savewadfilename = (char *)W_WadNameForLump(savemaplumpinfo);
	gameaction = ga_loadgame;
}

void M_ForceLoadGame()
{
	savegwarning =
	savemaplumpinfo ?
	M_StringJoin("This savegame requires the file\n",
	             crstr[CR_GOLD], savewadfilename, crstr[CR_NONE], "\n",
	             "to restore ", crstr[CR_GOLD], savemaplumpinfo->name, crstr[CR_NONE], " .\n\n",
	             "Continue to restore from\n",
	             crstr[CR_GOLD], W_WadNameForLump(savemaplumpinfo), crstr[CR_NONE], " ?\n\n",
	             PRESSYN, NULL) :
	M_StringJoin("This savegame requires the file\n",
	             crstr[CR_GOLD], savewadfilename, crstr[CR_NONE], "\n",
	             "to restore a map that is\n",
	             "currently not available!\n\n",
	             PRESSKEY, NULL) ;

	M_StartMessage(savegwarning, M_ForceLoadGameResponse, savemaplumpinfo != NULL);
	messageToPrint = 2;
	S_StartSoundOptional(NULL, sfx_mnuopn, sfx_swtchn); // [NS] Optional menu sounds.
}

static void M_ConfirmDeleteGameResponse (int key)
{
	free(savegwarning);

	if (key == key_menu_confirm)
	{
		char name[256];

		M_StringCopy(name, P_SaveGameFile(itemOn), sizeof(name));
		remove(name);

		if (itemOn == quickSaveSlot)
			quickSaveSlot = -1;

		M_ReadSaveStrings();
	}
}

void M_ConfirmDeleteGame ()
{
	savegwarning =
	M_StringJoin("delete savegame\n\n",
	             crstr[CR_GOLD], savegamestrings[itemOn], crstr[CR_NONE], " ?\n\n",
	             PRESSYN, NULL);

	M_StartMessage(savegwarning, M_ConfirmDeleteGameResponse, true);
	messageToPrint = 2;
	S_StartSoundOptional(NULL, sfx_mnuopn, sfx_swtchn); // [NS] Optional menu sounds.
}

// [crispy] indicate game version mismatch
void M_LoadGameVerMismatch ()
{
	M_StartMessage("Game Version Mismatch\n\n"PRESSKEY, NULL, false);
	messageToPrint = 2;
	S_StartSoundOptional(NULL, sfx_mnuopn, sfx_swtchn); // [NS] Optional menu sounds.
}
