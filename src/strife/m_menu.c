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
#include "z_zone.h"
#include "v_video.h"
#include "w_wad.h"

#include "r_local.h"


#include "hu_stuff.h"
#include "am_map.h" // [crispy]

#include "g_game.h"

#include "m_argv.h"
#include "m_controls.h"
#include "m_misc.h"
#include "m_saves.h"    // [STRIFE]
#include "p_saveg.h"

#include "s_sound.h"

#include "doomstat.h"

// Data.
#include "sounds.h"

#include "m_menu.h"
#include "p_dialog.h"

#include "m_crispy.h" // [crispy] Crispness menu
#include "v_trans.h" // [crispy] color translation and color string tables


void M_QuitStrife(int);


//
// defaulted values
//
int			mouseSensitivity = 5;
int			mouseSensitivity_x2 = 5; // [crispy] mouse sensitivity menu
int			mouseSensitivity_y = 5; // [crispy] mouse sensitivity menu

// [STRIFE]: removed this entirely
// Show messages has default, 0 = off, 1 = on
//int			showMessages = 1;
	

// Blocky mode, has default, 0 = high, 1 = normal
int			detailLevel = 0;
int			screenblocks = 10; // [STRIFE] default 10, not 9

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
// old save description before edit
char			saveOldString[SAVESTRINGSIZE];  

// [crispy] for entering numeric values
#define NUMERIC_ENTRY_NUMDIGITS 3
boolean numeric_enter;
int numeric_entry;
static char numeric_entry_str[NUMERIC_ENTRY_NUMDIGITS + 1];
static int numeric_entry_index;

// [crispy]
int show_exitscreen;

boolean                 inhelpscreens;
boolean                 menuactive;
boolean                 menupause;      // haleyjd 08/29/10: [STRIFE] New global
int                     menupausetime;  // haleyjd 09/04/10: [STRIFE] New global
boolean                 menuindialog;   // haleyjd 09/04/10: ditto

// haleyjd 08/27/10: [STRIFE] SKULLXOFF == -28, LINEHEIGHT == 19
#define CURSORXOFF		-28
#define LINEHEIGHT		19
#define CRISPY_LINEHEIGHT	10 // [crispy] Crispness menu

char			savegamestrings[10][SAVESTRINGSIZE];

char	endstring[160];

// haleyjd 09/04/10: [STRIFE] Moved menuitem / menu structures into header
// because they are needed externally by the dialog engine.

// haleyjd 08/27/10: [STRIFE] skull* stuff changed to cursor* stuff
short		itemOn;			// menu item skull is on
short		cursorAnimCounter;	// skull animation counter
short		cursorAnimCounter2;	// [crispy] Crispness menu cursor animation counter
short		whichCursor;		// which skull to draw
short		whichCursor2;		// [crispy] which Crispness menu cursor to draw

// graphic name of cursors
// haleyjd 08/27/10: [STRIFE] M_SKULL* -> M_CURS*
const char *cursorName[8] = {"M_CURS1", "M_CURS2", "M_CURS3", "M_CURS4",
                             "M_CURS5", "M_CURS6", "M_CURS7", "M_CURS8" };

// haleyjd 20110210 [STRIFE]: skill level for menus
int menuskill;

// current menudef
menu_t*	currentMenu;                          

// haleyjd 03/01/13: [STRIFE] v1.31-only:
// Keeps track of whether the save game menu is being used to name a new
// character slot, or to just save the current game. In the v1.31 disassembly
// this was the new dword_8632C variable.
boolean namingCharacter; 

//
// PROTOTYPES
//
void M_NewGame(int choice);
void M_Episode(int choice);
void M_ChooseSkill(int choice);
void M_LoadGame(int choice);
void M_SaveGame(int choice);
void M_Options(int choice);
void M_EndGame(int choice);
void M_ReadThis(int choice);
void M_ReadThis2(int choice);
void M_ReadThis3(int choice); // [STRIFE]

//void M_ChangeMessages(int choice); [STRIFE]
void M_ChangeSensitivity(int choice);
void M_ChangeSensitivity_x2(int choice); // [crispy] mouse sensitivity menu
void M_ChangeSensitivity_y(int choice); // [crispy] mouse sensitivity menu
void M_MouseInvert(int choice); // [crispy] mouse sensitivity menu
void M_SfxVol(int choice);
void M_VoiceVol(int choice); // [STRIFE]
void M_MusicVol(int choice);
void M_SizeDisplay(int choice);
void M_StartGame(int choice);
void M_Mouse(int choice); // [crispy] mouse sensitivity menu
void M_Sound(int choice);

//void M_FinishReadThis(int choice); - [STRIFE] unused
void M_SaveSelect(int choice);
void M_ReadSaveStrings(void);
void M_QuickSave(void);
void M_QuickLoad(void);

void M_DrawMainMenu(void);
void M_DrawReadThis1(void);
void M_DrawReadThis2(void);
void M_DrawReadThis3(void); // [STRIFE]
void M_DrawNewGame(void);
void M_DrawEpisode(void);
void M_DrawOptions(void);
void M_DrawMouse(void); // [crispy] mouse sensitivity menu
void M_DrawSound(void);
void M_DrawLoad(void);
void M_DrawSave(void);

void M_DrawSaveLoadBorder(int x,int y);
void M_SetupNextMenu(menu_t *menudef);
void M_DrawThermo(int x,int y,int thermWidth,int thermDot);
void M_DrawEmptyCell(menu_t *menu,int item);
void M_DrawSelCell(menu_t *menu,int item);
int  M_StringWidth(const char *string);
int  M_StringHeight(const char *string);
void M_StartMessage(const char *string,void *routine,boolean input);
void M_StopMessage(void);

// [crispy] Crispness menu
static void M_CrispnessCur(int choice);
static void M_CrispnessNext(int choice);
static void M_CrispnessPrev(int choice);
static void M_DrawCrispness1(void);
static void M_DrawCrispness2(void);
static void M_DrawCrispness3(void);



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
    {1,"M_RDTHIS",M_ReadThis,'h'}, // haleyjd 08/28/10: 'r' -> 'h'
    {1,"M_QUITG",M_QuitStrife,'q'}
};

menu_t  MainDef =
{
    main_end,
    NULL,
    MainMenu,
    M_DrawMainMenu,
    97,45, // haleyjd 08/28/10: [STRIFE] changed y coord
    0
};


//
// EPISODE SELECT
//
/*
enum
{
    ep1,
    ep2,
    ep3,
    ep4,
    ep_end
} episodes_e;

menuitem_t EpisodeMenu[]=
{
    {1,"M_EPI1", M_Episode,'k'},
    {1,"M_EPI2", M_Episode,'t'},
    {1,"M_EPI3", M_Episode,'i'},
    {1,"M_EPI4", M_Episode,'t'}
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
*/

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
    // haleyjd 08/28/10: [STRIFE] changed all shortcut letters
    {1,"M_JKILL",   M_ChooseSkill, 't'},
    {1,"M_ROUGH",   M_ChooseSkill, 'r'},
    {1,"M_HURT",    M_ChooseSkill, 'v'},
    {1,"M_ULTRA",   M_ChooseSkill, 'e'},
    {1,"M_NMARE",   M_ChooseSkill, 'b'}
};

menu_t  NewDef =
{
    newg_end,           // # of menu items
    &MainDef,           // previous menu - haleyjd [STRIFE] changed to MainDef
    NewGameMenu,        // menuitem_t ->
    M_DrawNewGame,      // drawing routine ->
    48,63,              // x,y
    toorough            // lastOn - haleyjd [STRIFE]: default to skill 1
};

//
// OPTIONS MENU
//
enum
{
    // haleyjd 08/28/10: [STRIFE] Removed messages, mouse sens., detail.
    endgame,
    scrnsize,
    option_empty1,
    mousesens, // [crispy] mouse sensitivity menu
    soundvol,
    crispness, // [crispy] Crispness menu
    opt_end
} options_e;

menuitem_t OptionsMenu[] =
{
    // haleyjd 08/28/10: [STRIFE] Removed messages, mouse sens., detail.
    {1, " ", M_EndGame, 'e', "End Game"},
    {2, " ", M_SizeDisplay, 's', "Screen Size"},
    {-1, "", 0, '\0'},
    {1, " ", M_Mouse, 'm', "Mouse Sensitivity"}, // [crispy] mouse sensitivity menu
    {1, " ", M_Sound, 's', "Sound Volume"}, // [crispy] no longer "Settings" menu
    {1, " ", M_CrispnessCur, 'c', "Crispness"} // [crispy] Crispness menu
};

menu_t  OptionsDef =
{
    opt_end,
    &MainDef,
    OptionsMenu,
    M_DrawOptions,
    80, 37,
    0,
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

static menuitem_t MouseMenu[] =
{
    {2, "", M_ChangeSensitivity, 'h'},
    {-1, "", 0, '\0'},
    {2, "", M_ChangeSensitivity_x2, 's'},
    {-1, "", 0, '\0'},
    {2, "", M_ChangeSensitivity_y, 'v'},
    {-1, "", 0, '\0'},
    {1, "", M_MouseInvert, 'i'},
};

static menu_t MouseDef =
{
    mouse_end,
    &OptionsDef,
    MouseMenu,
    M_DrawMouse,
    80, 18,
    0
};

// [crispy] Crispness menu
enum
{
    crispness_sep_rendering,
    crispness_hires,
    crispness_widescreen,
    crispness_smoothscaling,
    crispness_uncapped,
    crispness_fpslimit,
    crispness_vsync,
    crispness_sep_rendering_,

    crispness_sep_visual,
    crispness_smoothlight,
    crispness_sep_visual_,

    crispness1_next,
    crispness1_prev,
    crispness1_end
} crispness1_e;

static menuitem_t Crispness1Menu[] =
{
    {-1, "", 0, '\0'},
    {2, "", M_CrispyToggleHires, 'h'},
    {2, "", M_CrispyToggleWidescreen, 'w'},
    {2, "", M_CrispyToggleSmoothScaling, 's'},
    {2, "", M_CrispyToggleUncapped, 'u'},
    {3, "", M_CrispyToggleFpsLimit, 'f'},
    {2, "", M_CrispyToggleVsync, 'v'},
    {-1, "", 0, '\0'},
    {-1, "", 0, '\0'},
    {2, "", M_CrispyToggleSmoothLighting, 's'},
    {-1, "", 0, '\0'},
    {1, "", M_CrispnessNext, 'n'},
    {1, "", M_CrispnessPrev, 'p'},
};

static menu_t Crispness1Def =
{
    crispness1_end,
    &OptionsDef,
    Crispness1Menu,
    M_DrawCrispness1,
    48, 30,
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
    crispness_smoothmap,
    crispness_leveltime,
    crispness_playercoords,
    crispness_sep_navigational_,

    crispness2_next,
    crispness2_prev,
    crispness2_end
} crispness2_e;

static menuitem_t Crispness2Menu[] =
{
    {-1, "", 0, '\0'},
    {2, "", M_CrispyToggleFullsounds, 'p'},
    {2, "", M_CrispyToggleSoundfixes, 'm'},
    {2, "", M_CrispyToggleSndChannels, 's'},
    {2, "", M_CrispyToggleSoundMono, 'm'},
    {-1, "", 0, '\0'},
    {-1, "", 0, '\0'},
    {2, "", M_CrispyToggleSmoothMap, 'm'},
    {2, "", M_CrispyToggleLeveltime, 'l'},
    {2, "", M_CrispyTogglePlayerCoords, 'p'},
    {-1, "", 0, '\0'},
    {1, "", M_CrispnessNext, 'n'},
    {1, "", M_CrispnessPrev, 'p'},
};

static menu_t Crispness2Def =
{
    crispness2_end,
    &OptionsDef,
    Crispness2Menu,
    M_DrawCrispness2,
    48, 30,
    1
};

enum
{
    crispness_sep_tactical,
    crispness_runcentering,
    crispness_freelook,
    crispness_mouselook,
    crispness_bobfactor,
    crispness_centerweapon,
    crispness_defaultskill,
    crispness_sep_tactical_,

    crispness_sep_crosshair,
    crispness_crosshair,
    crispness_crosshairhealth,
    crispness_sep_crosshair_,

    crispness3_next,
    crispness3_prev,
    crispness3_end
} crispness3_e;

static menuitem_t Crispness3Menu[] =
{
    {-1, "", 0, '\0'},
    {2, "", M_CrispyToggleRunCentering, 'r'},
    {2, "", M_CrispyToggleFreelook, 'f'},
    {2, "", M_CrispyToggleMouseLook, 'p'},
    {2, "", M_CrispyToggleBobfactor, 'p'},
    {2, "", M_CrispyToggleCenterweapon, 'c'},
    {2, "", M_CrispyToggleDefaultSkill, 'd'},
    {-1, "", 0, '\0'},
    {-1, "", 0, '\0'},
    {2, "", M_CrispyToggleCrosshair, 'd'},
    {2, "", M_CrispyToggleCrosshairHealth, 'c'},
    {-1, "", 0, '\0'},
    {1, "", M_CrispnessNext, 'n'},
    {1, "", M_CrispnessPrev, 'p'},
};

static menu_t Crispness3Def =
{
    crispness3_end,
    &OptionsDef,
    Crispness3Menu,
    M_DrawCrispness3,
    48, 30,
    1
};

static menu_t *CrispnessMenus[] =
{
    &Crispness1Def,
    &Crispness2Def,
    &Crispness3Def,
};

static int crispness_cur;

//
// Read This! MENU 1 & 2 & [STRIFE] 3
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
    {1,"",M_ReadThis3,0} // haleyjd 08/28/10: [STRIFE] Go to ReadThis3
};

menu_t  ReadDef2 =
{
    read2_end,
    &ReadDef1,
    ReadMenu2,
    M_DrawReadThis2,
    250,185, // haleyjd 08/28/10: [STRIFE] changed coords
    0
};

// haleyjd 08/28/10: Added Read This! menu 3
enum
{
    rdthsempty3,
    read3_end
} read_e3;

menuitem_t ReadMenu3[]=
{
    {1,"",M_ClearMenus,0}
};

menu_t  ReadDef3 =
{
    read3_end,
    &ReadDef2,
    ReadMenu3,
    M_DrawReadThis3,
    250, 185,
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
    voice_vol,
    sfx_empty3,
    sound_end
} sound_e;

// haleyjd 08/29/10:
// [STRIFE] 
// * Added voice volume
// * Moved mouse sensitivity here (who knows why...)
menuitem_t SoundMenu[]=
{
    {2,"M_SFXVOL",M_SfxVol,'s'},
    {-1,"",0,'\0'},
    {2,"M_MUSVOL",M_MusicVol,'m'},
    {-1,"",0,'\0'},
    {2,"M_VOIVOL",M_VoiceVol,'v'}, 
    {-1,"",0,'\0'},
};

menu_t  SoundDef =
{
    sound_end,
    &OptionsDef,
    SoundMenu,
    M_DrawSound,
    80,35,       // [STRIFE] changed y coord 64 -> 35
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
    load_end
} load_e;

menuitem_t LoadMenu[]=
{
    {1,"", M_LoadSelect,'1'},
    {1,"", M_LoadSelect,'2'},
    {1,"", M_LoadSelect,'3'},
    {1,"", M_LoadSelect,'4'},
    {1,"", M_LoadSelect,'5'},
    {1,"", M_LoadSelect,'6'}
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
    {1,"", M_SaveSelect,'6'}
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

void M_DrawNameChar(void);

//
// NAME CHARACTER MENU
//
// [STRIFE]
// haleyjd 20110210: New "Name Your Character" Menu
//
menu_t NameCharDef =
{
    load_end,
    &NewDef,
    SaveMenu,
    M_DrawNameChar,
    80,54,
    0
};


//
// M_ReadSaveStrings
//  read the strings from the savegame files
//
// [STRIFE]
// haleyjd 20110210: Rewritten to read "name" file in each slot directory
//
void M_ReadSaveStrings(void)
{
    FILE *handle;
    int   i;
    char *fname = NULL;

    for(i = 0; i < load_end; i++)
    {
        int retval;
        if(fname)
            Z_Free(fname);
        fname = M_SafeFilePath(savegamedir, M_MakeStrifeSaveDir(i, "\\name"));

        handle = M_fopen(fname, "rb");
        if(handle == NULL)
        {
            M_StringCopy(savegamestrings[i], DEH_String(EMPTYSTRING),
                         sizeof(savegamestrings[i]));
            LoadMenu[i].status = 0;
            continue;
        }
        retval = fread(savegamestrings[i], 1, SAVESTRINGSIZE, handle);
        fclose(handle);
        LoadMenu[i].status = retval == SAVESTRINGSIZE;
    }

    if(fname)
        Z_Free(fname);
}

//
// M_DrawNameChar
//
// haleyjd 09/22/10: [STRIFE] New function
// Handler for drawing the "Name Your Character" menu.
//
void M_DrawNameChar(void)
{
    int i;
    const char *Name_Char = DEH_String("Name Your Character"); // [crispy]

    // [crispy] print "Name Your Character" centered and slightly higher
    M_WriteText(ORIGWIDTH/2-M_StringWidth(Name_Char)/2, NameCharDef.y/2, Name_Char);

    for (i = 0;i < load_end; i++)
    {
        M_DrawSaveLoadBorder(LoadDef.x,LoadDef.y+LINEHEIGHT*i);
        M_WriteText(LoadDef.x,LoadDef.y+LINEHEIGHT*i,savegamestrings[i]);
    }

    if (saveStringEnter)
    {
        i = M_StringWidth(savegamestrings[quickSaveSlot]);
        M_WriteText(LoadDef.x + i,LoadDef.y+LINEHEIGHT*quickSaveSlot,"_");
    }
}

//
// M_DoNameChar
//
// haleyjd 09/22/10: [STRIFE] New function
// Handler for items in the "Name Your Character" menu.
//
void M_DoNameChar(int choice)
{
    int map;

    // 20130301: clear naming character flag for 1.31 save logic
    if(gameversion == exe_strife_1_31)
        namingCharacter = false;
    sendsave = 1;
    ClearTmp();
    G_WriteSaveName(choice, savegamestrings[choice]);
    quickSaveSlot = choice;  
    SaveDef.lastOn = choice;
    ClearSlot();
    FromCurr();
    
    if(isdemoversion)
        map = 33;
    else
        map = 2;

    G_DeferedInitNew(menuskill, map);
    M_ClearMenus(0);
}

// [crispy] print "modified" (or created initially) time of savegame file
static void M_DrawSaveLoadBottomLine(void)
{
    const int y = 145;

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
        M_WriteText(ORIGWIDTH/2-M_StringWidth(filedate)/2, y, filedate);
        }
    }
}

//
// M_LoadGame & Cie.
//
static int LoadDef_x = 80, LoadDef_y = 54;
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
    // [STRIFE]: completely rewritten
    char *name = NULL;

    G_WriteSaveName(choice, savegamestrings[choice]);
    ToCurr();

    // use safe & portable filepath concatenation for Choco
    name = M_SafeFilePath(savegamedir, M_MakeStrifeSaveDir(choice, ""));

    G_ReadCurrent(name);
    quickSaveSlot = choice;
    M_ClearMenus(0);

    Z_Free(name);
}

//
// Selected from DOOM menu
//
// [STRIFE] Verified unmodified
//
void M_LoadGame (int choice)
{
    if (netgame)
    {
        M_StartMessage(DEH_String(LOADNET), NULL, false);
        return;
    }

    M_SetupNextMenu(&LoadDef);
    M_ReadSaveStrings();
}


//
//  M_SaveGame & Cie.
//
static int SaveDef_x = 80, SaveDef_y = 54;
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
        i = M_StringWidth(savegamestrings[quickSaveSlot]);
        M_WriteText(LoadDef.x + i,LoadDef.y+LINEHEIGHT*quickSaveSlot,"_");
    }

    M_DrawSaveLoadBottomLine();
}

//
// M_Responder calls this when user is finished
//
void M_DoSave(int slot)
{
    // [STRIFE]: completely rewritten
    if(slot >= 0)
    {
        sendsave = 1;
        G_WriteSaveName(slot, savegamestrings[slot]);
        M_ClearMenus(0);
        quickSaveSlot = slot;        
        // haleyjd 20130922: slight divergence. We clear the destination slot 
        // of files here, which vanilla did not do. As a result, 1.31 had 
        // broken save behavior to the point of unusability. fraggle agrees 
        // this is detrimental enough to be fixed - unconditionally, for now.
        ClearSlot();        
        FromCurr();
    }
    else
        M_StartMessage(DEH_String(QSAVESPOT), NULL, false);
}

//
// User wants to save. Start string input for M_Responder
//
void M_SaveSelect(int choice)
{
    int x, y;

    // we are going to be intercepting all chars
    saveStringEnter = 1;

    // We need to turn on text input:
    x = LoadDef.x - 11;
    y = LoadDef.y + choice * LINEHEIGHT - 4;
    I_StartTextInput(x, y, x + 8 + 24 * 8 + 8, y + LINEHEIGHT - 2);

    // [STRIFE]
    quickSaveSlot = choice;
    //saveSlot = choice;

    M_StringCopy(saveOldString, savegamestrings[choice], sizeof(saveOldString));
    if (!strcmp(savegamestrings[choice], DEH_String(EMPTYSTRING)))
        savegamestrings[choice][0] = 0;
    saveCharIndex = strlen(savegamestrings[choice]);
}

//
// Selected from DOOM menu
//
void M_SaveGame (int choice)
{
    // [STRIFE]
    if (netgame)
    {
        // haleyjd 20110211: Hooray for Rogue's awesome multiplayer support...
        M_StartMessage(DEH_String("You can't save a netgame"), NULL, false);
        return;
    }
    if (!usergame)
    {
        M_StartMessage(DEH_String(SAVEDEAD),NULL,false);
        return;
    }

    if (gamestate != GS_LEVEL)
        return;

    // [STRIFE]
    if(gameversion == exe_strife_1_31)
    {
        // haleyjd 20130301: in 1.31, we can choose a slot again.
        M_SetupNextMenu(&SaveDef);
        M_ReadSaveStrings();
    }
    else
    {
        // In 1.2 and lower, you save over your character slot exclusively
        M_ReadSaveStrings();
        M_DoSave(quickSaveSlot);
    }
}



//
//      M_QuickSave
//
char    tempstring[80];

void M_QuickSaveResponse(int key)
{
    if (key == key_menu_confirm)
    {
        M_DoSave(quickSaveSlot);
        S_StartSound(NULL, sfx_mtalht); // [STRIFE] sound
    }
}

void M_QuickSave(void)
{
    if (netgame)
    {
        // haleyjd 20110211 [STRIFE]: More fun...
        M_StartMessage(DEH_String("You can't save a netgame"), NULL, false);
        return;
    }

    if (!usergame)
    {
        S_StartSound(NULL, sfx_oof);
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
    DEH_snprintf(tempstring, 80, QSPROMPT, savegamestrings[quickSaveSlot]);
    M_StartMessage(tempstring,M_QuickSaveResponse,true);
}



//
// M_QuickLoadResponse
//
void M_QuickLoadResponse(int key)
{
    if (key == key_menu_confirm)
    {
        M_LoadSelect(quickSaveSlot);
        S_StartSound(NULL, sfx_mtalht); // [STRIFE] sound
    }
}

//
// M_QuickLoad
//
// [STRIFE] Verified unmodified
//
void M_QuickLoad(void)
{
    if (netgame)
    {
        M_StartMessage(DEH_String(QLOADNET),NULL,false);
        return;
    }

    if (quickSaveSlot < 0)
    {
        M_StartMessage(DEH_String(QSAVESPOT),NULL,false);
        return;
    }
    DEH_snprintf(tempstring, 80, QLPROMPT, savegamestrings[quickSaveSlot]);
    M_StartMessage(tempstring,M_QuickLoadResponse,true);
}




//
// Read This Menus
// Had a "quick hack to fix romero bug"
// haleyjd 08/28/10: [STRIFE] Draw HELP1, unconditionally.
//
void M_DrawReadThis1(void)
{
    inhelpscreens = true;

    V_DrawPatchFullScreen(W_CacheLumpName(DEH_String("HELP1"), PU_CACHE), false);
}



//
// Read This Menus
// haleyjd 08/28/10: [STRIFE] Not optional, draws HELP2
//
void M_DrawReadThis2(void)
{
    inhelpscreens = true;

    V_DrawPatchFullScreen(W_CacheLumpName(DEH_String("HELP2"), PU_CACHE), false);
}


//
// Read This Menus
// haleyjd 08/28/10: [STRIFE] New function to draw HELP3.
//
void M_DrawReadThis3(void)
{
    inhelpscreens = true;
    
    V_DrawPatchFullScreen(W_CacheLumpName(DEH_String("HELP3"), PU_CACHE), false);
}

//
// Change Sfx & Music volumes
//
// haleyjd 08/29/10: [STRIFE]
// * Changed title graphic coordinates
// * Added voice volume and sensitivity sliders
//
void M_DrawSound(void)
{
    M_DrawThermo(SoundDef.x,SoundDef.y+LINEHEIGHT*(sfx_vol+1),
                 16,sfxVolume);

    M_DrawThermo(SoundDef.x,SoundDef.y+LINEHEIGHT*(music_vol+1),
                 16,musicVolume);

    M_DrawThermo(SoundDef.x,SoundDef.y+LINEHEIGHT*(voice_vol+1),
                 16,voiceVolume);
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

//
// M_VoiceVol
//
// haleyjd 08/29/10: [STRIFE] New function
// Sets voice volume level.
//
void M_VoiceVol(int choice)
{
    switch(choice)
    {
    case 0:
        if (voiceVolume)
            voiceVolume--;
        break;
    case 1:
        if (voiceVolume < 15)
            voiceVolume++;
        break;
    }

    S_SetVoiceVolume(voiceVolume * 8);
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
// haleyjd 08/27/10: [STRIFE] Changed x coordinate; M_DOOM -> M_STRIFE
//
void M_DrawMainMenu(void)
{
    V_DrawPatchDirect(84, 2,
                      W_CacheLumpName(DEH_String("M_STRIFE"), PU_CACHE));
}




//
// M_NewGame
//
// haleyjd 08/31/10: [STRIFE] Changed M_NEWG -> M_NGAME
//
void M_DrawNewGame(void)
{
    V_DrawPatchDirect(96, 14, W_CacheLumpName(DEH_String("M_NGAME"), PU_CACHE));
    V_DrawPatchDirect(54, 38, W_CacheLumpName(DEH_String("M_SKILL"), PU_CACHE));
}

void M_NewGame(int choice)
{
    if (netgame && !demoplayback)
    {
        M_StartMessage(DEH_String(NEWGAME),NULL,false);
        return;
    }
    // haleyjd 09/07/10: [STRIFE] Removed Chex Quest and DOOM gamemodes
    if(gameversion == exe_strife_1_31)
       namingCharacter = true; // for 1.31 save logic
    M_SetupNextMenu(&NewDef);
}


//
//      M_Episode
//

// haleyjd: [STRIFE] Unused
/*
int     epi;

void M_DrawEpisode(void)
{
    V_DrawPatchDirect(54, 38, W_CacheLumpName(DEH_String("M_EPISOD"), PU_CACHE));
}

void M_VerifyNightmare(int key)
{
    if (key != key_menu_confirm)
        return;

    G_DeferedInitNew(nightmare, 1);
    M_ClearMenus (0);
}
*/

void M_ChooseSkill(int choice)
{
    // haleyjd 09/07/10: Removed nightmare confirmation
    // [STRIFE]: start "Name Your Character" menu
    menuskill = choice;
    currentMenu = &NameCharDef;
    itemOn = NameCharDef.lastOn;
    M_ReadSaveStrings();
}

/*
// haleyjd [STRIFE] Unused
void M_Episode(int choice)
{
    if ( (gamemode == shareware)
	 && choice)
    {
	M_StartMessage(DEH_String(SWSTRING),NULL,false);
	M_SetupNextMenu(&ReadDef1);
	return;
    }

    // Yet another hack...
    if ( (gamemode == registered)
	 && (choice > 2))
    {
      fprintf( stderr,
	       "M_Episode: 4th episode requires UltimateDOOM\n");
      choice = 0;
    }
	 
    epi = choice;
    M_SetupNextMenu(&NewDef);
}
*/


//
// M_Options
//
char    detailNames[2][9]	= {"M_GDHIGH","M_GDLOW"};
char	msgNames[2][9]		= {"M_MSGOFF","M_MSGON"};


void M_DrawOptions(void)
{
    // haleyjd 08/27/10: [STRIFE] M_OPTTTL -> M_OPTION
    V_DrawPatchDirect(108, 15, 
                      W_CacheLumpName(DEH_String("M_OPTION"), PU_CACHE));

    // haleyjd 08/26/10: [STRIFE] Removed messages, sensitivity, detail.

    M_DrawThermo(OptionsDef.x,OptionsDef.y+LINEHEIGHT*(scrnsize+1),
                 11,screenSize); // [crispy] Crispy HUD
}

// [crispy] mouse sensitivity menu
void M_DrawMouse(void)
{
    char mouse_menu_text[48];

    M_WriteText(MouseDef.x, MouseDef.y + LINEHEIGHT * mouse_horiz + 9,
                "Horizontal: Turn");

    M_DrawThermo(MouseDef.x, MouseDef.y + LINEHEIGHT * mouse_empty1,
                 16, mouseSensitivity);

    M_WriteText(MouseDef.x, MouseDef.y + LINEHEIGHT * mouse_horiz2 + 9,
                "Horizontal: Strafe");

    M_DrawThermo(MouseDef.x, MouseDef.y + LINEHEIGHT * mouse_empty2,
                 16, mouseSensitivity_x2);

    M_WriteText(MouseDef.x, MouseDef.y + LINEHEIGHT * mouse_vert + 9,
                "Vertical");

    M_DrawThermo(MouseDef.x, MouseDef.y + LINEHEIGHT * mouse_empty3,
                 16, mouseSensitivity_y);

    M_snprintf(mouse_menu_text, sizeof(mouse_menu_text),
               "Invert Vertical Axis: %s", mouse_y_invert ? "On" : "Off");
    M_WriteText(MouseDef.x, MouseDef.y + LINEHEIGHT * mouse_invert + 9,
                mouse_menu_text);

    dp_translation = NULL;
}

// [crispy] Crispness menu
static void M_DrawCrispnessBackground(void)
{
    byte *src;
    pixel_t *dest;

    src = W_CacheLumpName(back_flat, PU_CACHE);
    dest = I_VideoBuffer;

    V_FillFlat(0, SCREENHEIGHT, 0, SCREENWIDTH, src, dest);

    inhelpscreens = true;
}

static char crispy_menu_text[48];

static void M_DrawCrispnessHeader(const char *item)
{
    M_snprintf(crispy_menu_text, sizeof(crispy_menu_text),
               "%s%s", crstr[CR_RED], item);
    M_WriteText(ORIGWIDTH/2 - M_StringWidth(item) / 2, CRISPY_LINEHEIGHT, crispy_menu_text);
}

static void M_DrawCrispnessSeparator(int y, const char *item)
{
    M_snprintf(crispy_menu_text, sizeof(crispy_menu_text),
               "%s%s", crstr[CR_RED], item);
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
               "%s%s", crstr[CR_NONE], item);
    M_WriteText(currentMenu->x, currentMenu->y + CRISPY_LINEHEIGHT * y, crispy_menu_text);
}

static void M_DrawCrispness1(void)
{
    M_DrawCrispnessBackground();

    M_DrawCrispnessHeader("Crispness 1/3");

    M_DrawCrispnessSeparator(crispness_sep_rendering, "Rendering");
    M_DrawCrispnessItem(crispness_hires, "High Resolution Rendering", crispy->hires, true);
    M_DrawCrispnessMultiItem(crispness_widescreen, "Aspect Ratio", multiitem_widescreen, crispy->widescreen, aspect_ratio_correct == 1);
    M_DrawCrispnessItem(crispness_smoothscaling, "Smooth Pixel Scaling", smooth_pixel_scaling, !force_software_renderer);
    M_DrawCrispnessItem(crispness_uncapped, "Uncapped Framerate", crispy->uncapped, true);
    M_DrawCrispnessNumericItem(crispness_fpslimit, "Framerate Limit", crispy->fpslimit, "None", crispy->uncapped, "35");
    M_DrawCrispnessItem(crispness_vsync, "Enable VSync", crispy->vsync, !force_software_renderer);

    M_DrawCrispnessSeparator(crispness_sep_visual, "Visual");
    M_DrawCrispnessItem(crispness_smoothlight, "Smooth Diminishing Lighting", crispy->smoothlight, true);

    M_DrawCrispnessGoto(crispness1_next, "Next Page >");
    M_DrawCrispnessGoto(crispness1_prev, "< Last Page");

    dp_translation = NULL;
}

static void M_DrawCrispness2(void)
{
    M_DrawCrispnessBackground();

    M_DrawCrispnessHeader("Crispness 2/3");

    M_DrawCrispnessSeparator(crispness_sep_audible, "Audible");
    M_DrawCrispnessItem(crispness_soundfull, "Play Sounds in Full Length", crispy->soundfull, true);
    M_DrawCrispnessItem(crispness_soundfix, "Misc. Sound Fixes", crispy->soundfix, true);
    M_DrawCrispnessMultiItem(crispness_sndchannels, "Sound Channels", multiitem_sndchannels, snd_channels >> 4, snd_sfxdevice != SNDDEVICE_PCSPEAKER);
    M_DrawCrispnessItem(crispness_soundmono, "Mono SFX", crispy->soundmono, true);

    M_DrawCrispnessSeparator(crispness_sep_navigational, "Navigational");
    M_DrawCrispnessItem(crispness_smoothmap, "Smooth Automap Lines", crispy->smoothmap, true);
    M_DrawCrispnessMultiItem(crispness_leveltime, "Show Level Time", multiitem_widgets, crispy->leveltime, true);
    M_DrawCrispnessMultiItem(crispness_playercoords, "Show Player Coords", multiitem_widgets, crispy->playercoords, true);

    M_DrawCrispnessGoto(crispness2_next, "Next Page >");
    M_DrawCrispnessGoto(crispness2_prev, "< Prev Page");

    dp_translation = NULL;
}

static void M_DrawCrispness3(void)
{
    M_DrawCrispnessBackground();

    M_DrawCrispnessHeader("Crispness 3/3");

    M_DrawCrispnessSeparator(crispness_sep_tactical, "Tactical");
    M_DrawCrispnessItem(crispness_runcentering, "\"Run\" Centers View", runcentering, true);
    M_DrawCrispnessMultiItem(crispness_freelook, "Freelook Mode", multiitem_freelook, crispy->freelook_hh, true);
    M_DrawCrispnessItem(crispness_mouselook, "Permanent Mouse Look", crispy->mouselook, !demorecording && !netgame);
    M_DrawCrispnessMultiItem(crispness_bobfactor, "View/Weapon Bobbing", multiitem_bobfactor, crispy->bobfactor, true);
    M_DrawCrispnessMultiItem(crispness_centerweapon, "Attack Alignment", multiitem_centerweapon, crispy->centerweapon, crispy->bobfactor != BOBFACTOR_OFF);
    M_DrawCrispnessMultiItem(crispness_defaultskill, "Default Difficulty", multiitem_difficulties, crispy->defaultskill, true);

    M_DrawCrispnessSeparator(crispness_sep_crosshair, "Crosshair");
    M_DrawCrispnessItem(crispness_crosshair, "Draw Crosshair", crispy->crosshair, true);
    M_DrawCrispnessItem(crispness_crosshairhealth, "Color Indicates Health", crispy->crosshairhealth, crispy->crosshair);

    M_DrawCrispnessGoto(crispness3_next, "First Page >");
    M_DrawCrispnessGoto(crispness3_prev, "< Prev Page");

    dp_translation = NULL;
}

void M_Options(int choice)
{
    M_SetupNextMenu(&OptionsDef);
}

// [crispy] correctly handle inverted y-axis
void M_Mouse(int choice)
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
// M_AutoUseHealth
//
// [STRIFE] New function
// haleyjd 20110211: toggle autouse health state
//
void M_AutoUseHealth(void)
{
    if(!netgame && usergame)
    {
        players[consoleplayer].cheats ^= CF_AUTOHEALTH;

        if(players[consoleplayer].cheats & CF_AUTOHEALTH)
            players[consoleplayer].message = DEH_String("Auto use health ON");
        else
            players[consoleplayer].message = DEH_String("Auto use health OFF");
    }
}

//
// M_ChangeShowText
//
// [STRIFE] New function
//
void M_ChangeShowText(void)
{
    dialogshowtext ^= true;

    if(dialogshowtext)
        players[consoleplayer].message = DEH_String("Conversation Text On");
    else
        players[consoleplayer].message = DEH_String("Conversation Text Off");
}

//
//      Toggle messages on/off
//
// [STRIFE] Messages cannot be disabled in Strife
/*
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
*/


//
// M_EndGame
//
void M_EndGameResponse(int key)
{
    // [crispy] allow to confirm by pressing Enter key
    if (key != key_menu_confirm && key != key_menu_forward)
        return;

    currentMenu->lastOn = itemOn;
    M_ClearMenus (0);
    D_StartTitle ();
}

void M_EndGame(int choice)
{
    choice = 0;
    if (!usergame)
    {
        S_StartSound(NULL,sfx_oof);
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

//
// M_ReadThis2
//
// haleyjd 08/28/10: [STRIFE] Eliminated DOOM stuff.
//
void M_ReadThis2(int choice)
{
    choice = 0;
    M_SetupNextMenu(&ReadDef2);
}

//
// M_ReadThis3
//
// haleyjd 08/28/10: [STRIFE] New function.
//
void M_ReadThis3(int choice)
{
    choice = 0;
    M_SetupNextMenu(&ReadDef3);
}

/*
// haleyjd 08/28/10: [STRIFE] Not used.
void M_FinishReadThis(int choice)
{
    choice = 0;
    M_SetupNextMenu(&MainDef);
}
*/

#if 0

//
// M_CheckStartCast
//
// [STRIFE] New but unused function. Was going to start a cast
//   call from within the menu system... not functional even in
//   the earliest demo version.
//
void M_CheckStartCast()
{
    if(usergame)
    {
        M_StartMessage(DEH_String("You have to end your game first."), NULL, false);
        return;
    }

    F_StartCast();
    M_ClearMenus(0);
}
#endif

//
// M_QuitResponse
//
// haleyjd 09/11/10: [STRIFE] Modifications to start up endgame
// demosequence.
//
void M_QuitResponse(int key)
{
    char buffer[20];

    // [crispy] allow to confirm by pressing Enter key
    if (key != key_menu_confirm && key != key_menu_forward)
        return;

    // [crispy] quit immediately if not showing exit screen
    if(!show_exitscreen || netgame)
        I_Quit();
    else
    {
        DEH_snprintf(buffer, sizeof(buffer), "qfmrm%i", gametic % 8 + 1);
        I_StartVoice(buffer);
        D_QuitGame();
    }
}

/*
// haleyjd 09/11/10: [STRIFE] Unused
static char *M_SelectEndMessage(void)
{
}
*/

//
// M_QuitStrife
//
// [STRIFE] Renamed from M_QuitDOOM
// haleyjd 09/11/10: No randomized text message; that's taken care of
// by the randomized voice message after confirmation.
//
void M_QuitStrife(int choice)
{
    // [crispy] fast exit if "run" key is held down
    if (speedkeydown())
        I_Quit();

    DEH_snprintf(endstring, sizeof(endstring),
                 "Do you really want to leave?\n\n" DOSY);
  
    M_StartMessage(endstring, M_QuitResponse, true);
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

void M_ChangeSensitivity_y(int choice)
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

void M_MouseInvert(int choice)
{
    choice = 0;
    mouse_y_invert = !mouse_y_invert;
}

/*
// haleyjd [STRIFE] Unused
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
*/

// [STRIFE] Verified unmodified
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
        if (screenSize < 10) // [crispy] Crispy HUD
        {
            screenblocks++;
            screenSize++;
        }
        break;
    }

    R_SetViewSize (screenblocks, detailLevel);
}




//
//      Menu Functions
//

//
// M_DrawThermo
//
// haleyjd 08/28/10: [STRIFE] Changes to some patch coordinates.
//
void
M_DrawThermo
( int	x,
  int	y,
  int	thermWidth,
  int	thermDot )
{
    int         xx;
    int         yy; // [STRIFE] Needs a temp y coordinate variable
    int         i;
    char        num[4]; // [crispy]

    // [crispy] Darken the slider when the value is zero.
    if (!thermDot)
        dp_translation = cr[CR_DARK];

    xx = x;
    yy = y + 6; // [STRIFE] +6 to y coordinate
    V_DrawPatchDirect(xx, yy, W_CacheLumpName(DEH_String("M_THERML"), PU_CACHE));
    xx += 8;
    for (i=0;i<thermWidth;i++)
    {
        V_DrawPatchDirect(xx, yy, W_CacheLumpName(DEH_String("M_THERMM"), PU_CACHE));
        xx += 8;
    }
    V_DrawPatchDirect(xx, yy, W_CacheLumpName(DEH_String("M_THERMR"), PU_CACHE));

    // [crispy] Draw value to the right of the slider.
    M_snprintf(num, 4, "%3d", thermDot);
    M_WriteText(xx + 18, yy, num);

    // [crispy] Don't crash if value exceeds thermometer range.
    if (thermDot >= thermWidth)
    {
        thermDot = thermWidth - 1;
        dp_translation = cr[CR_DARK];
    }

    // [STRIFE] +2 to initial y coordinate
    V_DrawPatchDirect((x + 8) + thermDot * 8, y + 2,
                      W_CacheLumpName(DEH_String("M_THERMO"), PU_CACHE));

    // [crispy]
    dp_translation = NULL;
}


// haleyjd: These are from DOOM v0.5 and the prebeta! They drew those ugly red &
// blue checkboxes... preserved for historical interest, as not in Strife.
void
M_DrawEmptyCell
( menu_t*	menu,
  int		item )
{
    V_DrawPatchDirect(menu->x - 10, menu->y + item * LINEHEIGHT - 1, 
                      W_CacheLumpName(DEH_String("M_CELL1"), PU_CACHE));
}

void
M_DrawSelCell
( menu_t*	menu,
  int		item )
{
    V_DrawPatchDirect(menu->x - 10, menu->y + item * LINEHEIGHT - 1,
                      W_CacheLumpName(DEH_String("M_CELL2"), PU_CACHE));
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
    return;
}



void M_StopMessage(void)
{
    menuactive = messageLastMenuActive;
    messageToPrint = 0;
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
int M_StringHeight(const char *string)
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
// M_WriteText
//
// Write a string using the hu_font
// haleyjd 09/04/10: [STRIFE]
// * Rogue made a lot of changes to this for the dialog system.
//
int
M_WriteText
( int           x,
  int           y,
  const char*   string) // haleyjd: made const for safety w/dialog engine
{
    int	        w;
    const char* ch;
    int         c;
    int         cx;
    int         cy;

    ch = string;
    cx = x;
    cy = y;

    while(1)
    {
        c = *ch++;
        if (!c)
            break;

        // haleyjd 09/04/10: [STRIFE] Don't draw spaces at the start of lines.
        if(c == ' ' && cx == x)
            continue;

        if (c == '\n')
        {
            cx = x;
            cy += 11; // haleyjd 09/04/10: [STRIFE]: Changed 12 -> 11
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

        // haleyjd 09/04/10: [STRIFE] Different linebreak handling
        if (cx + w > ORIGWIDTH - 20)
        {
            cx = x;
            cy += 11;
            --ch;
        }
        else
        {
            V_DrawPatchDirect(cx, cy, hu_font[c]);
            cx += w;
        }
    }

    // [STRIFE] Return final y coordinate.
    return cy + 12;
}

//
// M_DialogDimMsg
//
// [STRIFE] New function
// haleyjd 09/04/10: Painstakingly transformed from the assembly code, as the
// decompiler could not touch it. Redimensions a string to fit on screen, leaving
// at least a 20 pixel margin on the right side. The string passed in must be
// writable.
//
void M_DialogDimMsg(int x, int y, char *str, boolean useyfont)
{
    int rightbound = (ORIGWIDTH - 20) - x;
    patch_t **fontarray;  // ebp
    int linewidth = 0;    // esi
    int i = 0;            // edx
    char *message = str;  // edi
    char  bl;             // bl

    if(useyfont)
       fontarray = yfont;
    else
       fontarray = hu_font;

    bl = toupper(*message);

    if(!bl)
        return;

    // outer loop - run to end of string
    do
    {
        if(bl != '\n')
        {
            int charwidth; // eax
            int tempwidth; // ecx

            if(bl < HU_FONTSTART || bl > HU_FONTEND)
                charwidth = 4;
            else
                charwidth = SHORT(fontarray[bl - HU_FONTSTART]->width);

            tempwidth = linewidth + charwidth;

            // Test if the line still fits within the boundary...
            if(tempwidth >= rightbound)
            {
                // Doesn't fit...
                char *tempptr = &message[i]; // ebx
                char  al;                    // al

                // inner loop - run backward til a space (or the start of the
                // string) is found, subtracting width off the current line.
                // BUG: shouldn't we stop at a previous '\n' too?
                while(*tempptr != ' ' && i > 0)
                {
                    tempptr--;
                    // BUG: they didn't add the first char to linewidth yet...
                    linewidth -= charwidth; 
                    i--;
                    al = toupper(*tempptr);
                    if(al < HU_FONTSTART || al > HU_FONTEND)
                        charwidth = 4;
                    else
                        charwidth = SHORT(fontarray[al - HU_FONTSTART]->width);
                }
                // Replace the space with a linebreak.
                // BUG: what if i is zero? ... infinite loop time!
                message[i] = '\n';
                linewidth = 0;
            }
            else
            {
                // The line does fit.
                // Spaces at the start of a line don't count though.
                if(!(bl == ' ' && linewidth == 0))
                    linewidth += charwidth;
            }
        }
        else
            linewidth = 0; // '\n' seen, so reset the line width
    }
    while((bl = toupper(message[++i])) != 0); // step to the next character
}

// These keys evaluate to a "null" key in Vanilla Doom that allows weird
// jumping in the menus. Preserve this behavior for accuracy.

static boolean IsNullKey(int key)
{
    return key == KEY_PAUSE || key == KEY_CAPSLOCK
        || key == KEY_SCRLCK || key == KEY_NUMLOCK;
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
    static  int     joywait = 0;
    static  int     mousewait = 0;
    static  int     mousey = 0;
    static  int     lasty = 0;
    static  int     mousex = 0;
    static  int     lastx = 0;
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
            S_StartSound(NULL, sfx_swtchn);
            M_QuitStrife(0);
        }

        return true;
    }

    // key is the key pressed, ch is the actual character typed
  
    ch = 0;
    key = -1;

    if (ev->type == ev_joystick && joywait < I_GetTime())
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
            else
            {
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
            else
            {
                key = key_menu_back;
            }
            joywait = I_GetTime() + 5;
        }
        if (JOY_BUTTON_PRESSED(joybmenu))
        {
            key = key_menu_activate;
            joywait = I_GetTime() + 5;
        }
    }
    else
    {
        if (ev->type == ev_mouse && mousewait < I_GetTime())
        {
            // [crispy] Don't control Crispness menu with y-axis mouse movement.
            // "novert" disables up/down cursor movement with the mouse.
            if (!inhelpscreens && !novert)
                mousey += ev->data3;

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

            // [crispy] Don't control Crispness menu with x-axis mouse movement.
            if (!inhelpscreens)
                mousex += ev->data2;

            if (mousex < lastx-30)
            {
                key = key_menu_left;
                mousewait = I_GetTime() + 5;
                mousex = lastx -= 30;
            }
            else if (mousex > lastx+30)
            {
                key = key_menu_right;
                mousewait = I_GetTime() + 5;
                mousex = lastx += 30;
            }

            if (ev->data1&1)
            {
                key = key_menu_forward;
                mousewait = I_GetTime() + 5;
                if (menuindialog) // [crispy] fix mouse fire delay
                {
                mouse_fire_countdown = 5;   // villsa [STRIFE]
                }
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
                savegamestrings[quickSaveSlot][saveCharIndex] = 0;
            }
            break;

        case KEY_ESCAPE:
            saveStringEnter = 0;
            I_StopTextInput();
            M_StringCopy(savegamestrings[quickSaveSlot], saveOldString,
                         sizeof(savegamestrings[quickSaveSlot]));
            break;

        case KEY_ENTER:
            // [STRIFE]
            saveStringEnter = 0;
            I_StopTextInput();
            if(gameversion == exe_strife_1_31 && !namingCharacter)
            {
               // In 1.31, we can be here as a result of normal saving again,
               // whereas in 1.2 this only ever happens when naming your
               // character to begin a new game.
               M_DoSave(quickSaveSlot);
               return true;
            }
            if (savegamestrings[quickSaveSlot][0])
                M_DoNameChar(quickSaveSlot);
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
                M_StringWidth(savegamestrings[quickSaveSlot]) <
                (SAVESTRINGSIZE-2)*8)
            {
                savegamestrings[quickSaveSlot][saveCharIndex++] = ch;
                savegamestrings[quickSaveSlot][saveCharIndex] = 0;
            }
            break;
        }
        return true;
    }

    // [crispy] Enter numeric value
    if (numeric_enter)
    {
        switch (key)
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
                // [crispy] allow to confirm end game and quit by pressing Enter key
                && key != key_menu_forward)
            {
                return false;
            }
        }

        menuactive = messageLastMenuActive;
        messageToPrint = 0;
        if (messageRoutine)
            messageRoutine(key);

        menupause = false;                // [STRIFE] unpause
        menuactive = false;
        S_StartSound(NULL, sfx_mtalht);   // [STRIFE] sound
        return true;
    }

    // [STRIFE]:
    // * In v1.2 this is moved to F9 (quickload)
    // * In v1.31 it is moved to F12 with DM spy, and quicksave
    //   functionality is restored separate from normal saving
    /*
    if (devparm && key == key_menu_help)
    {
        G_ScreenShot ();
        return true;
    }
    */

    // [crispy] clean screenshot
    if (key != 0 && key == key_menu_cleanscreenshot)
    {
        crispy->cleanscreenshot = (screenblocks > 10) ? 2 : 1;
        G_ScreenShot();
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
            S_StartSound(NULL, sfx_stnmov);
            return true;
        }
        else if (key == key_menu_incscreen) // Screen size up
        {
            if (automapactive || chat_on)
                return false;
            M_SizeDisplay(1);
            S_StartSound(NULL, sfx_stnmov);
            return true;
        }
        else if (key == key_menu_help)     // Help key
        {
            M_StartControlPanel ();
            // haleyjd 08/29/10: [STRIFE] always ReadDef1
            currentMenu = &ReadDef1; 

            itemOn = 0;
            S_StartSound(NULL, sfx_swtchn);
            return true;
        }
        else if (key == key_menu_save)     // Save
        {
            // [STRIFE]: Hub saves
            if(gameversion == exe_strife_1_31)
                namingCharacter = false; // just saving normally, in 1.31

            if(netgame || players[consoleplayer].health <= 0 ||
                players[consoleplayer].cheats & CF_ONFIRE)
            {
                S_StartSound(NULL, sfx_oof);
            }
            else
            {
                M_StartControlPanel();
                S_StartSound(NULL, sfx_swtchn);
                M_SaveGame(0);
            }
            return true;
        }
        else if (key == key_menu_load)     // Load
        {
            // [STRIFE]: Hub saves
            if(gameversion == exe_strife_1_31)
            {
                // 1.31: normal save loading
                namingCharacter = false;
                M_StartControlPanel();
                M_LoadGame(0);
                S_StartSound(NULL, sfx_swtchn);
            }
            else
            {
                // Pre 1.31: quickload only
                S_StartSound(NULL, sfx_swtchn);
                M_QuickLoad();
            }
            return true;
        }
        else if (key == key_menu_volume)   // Sound Volume
        {
            M_StartControlPanel ();
            currentMenu = &SoundDef;
            itemOn = currentMenu->lastOn; // [crispy] remember cursor position
            S_StartSound(NULL, sfx_swtchn);
            return true;
        }
        else if (key == key_menu_detail)   // Detail toggle
        {
            //M_ChangeDetail(0);
            M_AutoUseHealth(); // [STRIFE]
            S_StartSound(NULL, sfx_swtchn);
            return true;
        }
        else if (key == key_menu_qsave)    // Quicksave
        {
            // [STRIFE]: Hub saves
            if(gameversion == exe_strife_1_31)
                namingCharacter = false; // for 1.31 save changes

            if(netgame || players[consoleplayer].health <= 0 ||
               players[consoleplayer].cheats & CF_ONFIRE)
            {
                S_StartSound(NULL, sfx_oof);
            }
            else
            {
                S_StartSound(NULL, sfx_swtchn);
                M_QuickSave();
            }
            return true;
        }
        else if (key == key_menu_endgame)  // End game
        {
            S_StartSound(NULL, sfx_swtchn);
            M_EndGame(0);
            return true;
        }
        else if (key == key_menu_messages) // Toggle messages
        {
            //M_ChangeMessages(0);
            M_ChangeShowText(); // [STRIFE]
            S_StartSound(NULL, sfx_swtchn);
            return true;
        }
        else if (key == key_menu_qload)    // Quickload
        {
            // [STRIFE]
            // * v1.2: takes a screenshot
            // * v1.31: does quickload again
            if(gameversion == exe_strife_1_31)
            {
                namingCharacter = false;
                S_StartSound(NULL, sfx_swtchn);
                M_QuickLoad();
            }
            else
                G_ScreenShot();
            return true;
        }
        else if (key == key_menu_quit)     // Quit DOOM
        {
            S_StartSound(NULL, sfx_swtchn);
            M_QuitStrife(0);
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
            I_SetPalette(0);
            R_InitColormaps();
            inhelpscreens = true;
            R_FillBackScreen();
            viewactive = false;
#endif
            return true;
        }
        else if(gameversion == exe_strife_1_31 && key == key_spy)
        {
            // haleyjd 20130301: 1.31 moved screenshots to F12.
            G_ScreenShot();
            return true;
        }
        else if (key != 0 && key == key_menu_screenshot)
        {
            G_ScreenShot();
            return true;
        }
    }

    // Pop-up menu?
    if (!menuactive)
    {
        if (key == key_menu_activate)
        {
            M_StartControlPanel ();
            S_StartSound(NULL, sfx_swtchn);
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
            S_StartSound(NULL, sfx_pstop);
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
            S_StartSound(NULL, sfx_pstop);
        } while(currentMenu->menuitems[itemOn].status==-1);

        return true;
    }
    else if (key == key_menu_left)
    {
        // Slide slider left

        if (currentMenu->menuitems[itemOn].routine &&
            currentMenu->menuitems[itemOn].status >= 2)
        {
            S_StartSound(NULL, sfx_stnmov);
            currentMenu->menuitems[itemOn].routine(0);
        }
        return true;
    }
    else if (key == key_menu_right)
    {
        // Slide slider right

        if (currentMenu->menuitems[itemOn].routine &&
            currentMenu->menuitems[itemOn].status >= 2)
        {
            S_StartSound(NULL, sfx_stnmov);
            currentMenu->menuitems[itemOn].routine(1);
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
                S_StartSound(NULL, sfx_stnmov);
            }
            else if (currentMenu->menuitems[itemOn].status == 3) // [crispy]
            {
                currentMenu->menuitems[itemOn].routine(2); // enter key
                numeric_entry_index = 0;
                numeric_entry_str[0] = '\0';
            }
            else
            {
                currentMenu->menuitems[itemOn].routine(itemOn);
                //S_StartSound(NULL, sfx_swish); [STRIFE] No sound is played here.
            }
        }
        return true;
    }
    else if (key == key_menu_activate)
    {
        // Deactivate menu
        if(gameversion == exe_strife_1_31) // [STRIFE]: 1.31 saving
            namingCharacter = false;

        if(menuindialog) // [STRIFE] - Get out of dialog engine semi-gracefully
            P_DialogDoChoice(-1);

        currentMenu->lastOn = itemOn;
        M_ClearMenus (0);
        S_StartSound(NULL, sfx_mtalht); // villsa [STRIFE]: sounds
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
            S_StartSound(NULL, sfx_swtchn);
        }
        return true;
    }
    // [crispy] next/prev Crispness menu
    else if (key == KEY_PGUP)
    {
        currentMenu->lastOn = itemOn;
        if (currentMenu == CrispnessMenus[crispness_cur])
        {
            M_CrispnessPrev(0);
            S_StartSound(NULL, sfx_swtchn);
            return true;
        }
    }
    else if (key == KEY_PGDN)
    {
        currentMenu->lastOn = itemOn;
        if (currentMenu == CrispnessMenus[crispness_cur])
        {
            M_CrispnessNext(0);
            S_StartSound(NULL, sfx_swtchn);
            return true;
        }
    }

    // Keyboard shortcut?
    // Vanilla Strife has a weird behavior where it jumps to the scroll bars
    // when certain keys are pressed, so emulate this.

    else if (ch != 0 || IsNullKey(key))
    {
        // Keyboard shortcut?

        for (i = itemOn+1;i < currentMenu->numitems;i++)
        {
            if (currentMenu->menuitems[i].alphaKey == ch)
            {
                itemOn = i;
                S_StartSound(NULL, sfx_pstop);
                return true;
            }
        }

        for (i = 0;i <= itemOn;i++)
        {
            if (currentMenu->menuitems[i].alphaKey == ch)
            {
                itemOn = i;
                S_StartSound(NULL, sfx_pstop);
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
    
    menuactive = 1;
    menupause = true;
    currentMenu = &MainDef;         // JDC
    itemOn = currentMenu->lastOn;   // JDC
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
        start = 0;
        y = 100 - M_StringHeight(messageString) / 2;
        while (messageString[start] != '\0')
        {
            int foundnewline = 0;

            for (i = 0; i < strlen(messageString + start); i++)
            {
                if (messageString[start + i] == '\n')
                {
                    M_StringCopy(string, messageString + start,
                                 sizeof(string));
                    if (i < sizeof(string))
                    {
                        string[i] = '\0';
                    }

                    foundnewline = 1;
                    start += i + 1;
                    break;
                }
            }

            if (!foundnewline)
            {
                M_StringCopy(string, messageString + start,
                             sizeof(string));
                start += strlen(string);
            }

            x = 160 - M_StringWidth(string) / 2;
            M_WriteText(x, y, string);
            y += SHORT(hu_font[0]->height);
        }

        return;
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
                V_DrawPatchDirect(x, y, W_CacheLumpName(name, PU_CACHE));
            else if (alttext)
                M_WriteText(x, y + 12 - (M_StringHeight(alttext) / 2), alttext);
        }
        y += LINEHEIGHT;
    }

    
    // haleyjd 08/27/10: [STRIFE] Adjust to draw spinning Sigil
    // DRAW SIGIL
    // [crispy] Show ">" cursor in Crispness menu; don't show Sigil cursor on
    // help screens
    if (currentMenu == CrispnessMenus[crispness_cur])
    {
        char item[4];
        M_snprintf(item, sizeof(item), "%s>", whichCursor2 ? crstr[CR_NONE] : crstr[CR_DARK]);
        M_WriteText(currentMenu->x - 8, currentMenu->y + CRISPY_LINEHEIGHT * itemOn, item);
        dp_translation = NULL;
    }
    else if (currentMenu != &ReadDef1 && currentMenu != &ReadDef2 &&
             currentMenu != &ReadDef3)
    {
    V_DrawPatchDirect(x + CURSORXOFF, currentMenu->y - 5 + itemOn*LINEHEIGHT,
                      W_CacheLumpName(DEH_String(cursorName[whichCursor]),
                                      PU_CACHE));
    }
}


//
// M_ClearMenus
//
// haleyjd 08/28/10: [STRIFE] Added an int param so this can be called by menus.
//         09/08/10: Added menupause.
//
void M_ClearMenus (int choice)
{
    choice = 0;     // haleyjd: for no warning; not from decompilation.
    menuactive = 0;
    menupause = 0;
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
// haleyjd 08/27/10: [STRIFE] Rewritten for Sigil cursor
//
void M_Ticker (void)
{
    if (--cursorAnimCounter <= 0)
    {
        whichCursor = (whichCursor + 1) % 8;
        cursorAnimCounter = 5;
    }

    // [crispy] Crispness menu cursor
    if (--cursorAnimCounter2 <= 0)
    {
        whichCursor2 ^= 1;
        cursorAnimCounter2 = 8;
    }
}

// [crispy]
void M_SetDefaultDifficulty (void)
{
    // Strife default is SKILL_HNTR ("Rookie").
    NewDef.lastOn = (crispy->defaultskill + SKILL_HNTR) % NUM_SKILLS;
}

//
// M_Init
//
// haleyjd 08/27/10: [STRIFE] Removed DOOM gamemode stuff
//
void M_Init (void)
{
    currentMenu = &MainDef;
    menuactive = 0;
    itemOn = currentMenu->lastOn;
    whichCursor = 0;
    whichCursor2 = 0; // [crispy] Crispness menu cursor
    cursorAnimCounter = 10;
    cursorAnimCounter2 = 10; // [crispy] Crispness menu cursor
    screenSize = screenblocks - 3;
    messageToPrint = 0;
    messageString = NULL;
    messageLastMenuActive = menuactive; // STRIFE-FIXME: assigns 0 here...
    quickSaveSlot = -1;

    M_SetDefaultDifficulty(); // [crispy] pre-select default difficulty

    // [STRIFE]: Initialize savegame paths and clear temporary directory
    G_WriteSaveName(5, "ME");
    ClearTmp();

    // [crispy] rearrange Load Game and Save Game menus
    {
        const patch_t *patchl, *patchs, *patchm;
        short captionheight, vstep;

        patchl = W_CacheLumpName(DEH_String("M_LOADG"), PU_CACHE);
        patchs = W_CacheLumpName(DEH_String("M_SAVEG"), PU_CACHE);
        patchm = W_CacheLumpName(DEH_String("M_LSLEFT"), PU_CACHE);

        LoadDef_x = (ORIGWIDTH - SHORT(patchl->width)) / 2 + SHORT(patchl->leftoffset);
        SaveDef_x = (ORIGWIDTH - SHORT(patchs->width)) / 2 + SHORT(patchs->leftoffset);
        NameCharDef.x = LoadDef.x = SaveDef.x = (ORIGWIDTH - 24 * 8) / 2 + SHORT(patchm->leftoffset);

        captionheight = MAX(SHORT(patchl->height), SHORT(patchs->height));

        vstep = ORIGHEIGHT - 32; // [crispy] ST_HEIGHT
        vstep -= captionheight;
        vstep -= (load_end - 1) * LINEHEIGHT + SHORT(patchm->height);
        vstep /= 3;

        if (vstep > 0)
        {
            LoadDef_y = vstep + captionheight - SHORT(patchl->height) + SHORT(patchl->topoffset);
            SaveDef_y = vstep + captionheight - SHORT(patchs->height) + SHORT(patchs->topoffset);
            LoadDef.y = SaveDef.y = vstep + captionheight + vstep + SHORT(patchm->topoffset) - 19; // [crispy] moved up, so savegame date/time may appear above status bar
            NameCharDef.y = MouseDef.y = LoadDef.y;
        }
    }

    // Here we could catch other version dependencies,
    //  like HELP1/2, and four episodes.
}

