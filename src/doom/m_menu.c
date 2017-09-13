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


#include "doomdef.h"
#include "doomkeys.h"
#include "dstrings.h"

#include "d_main.h"
#include "deh_main.h"

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
#include "p_extsaveg.h" // [crispy] savewadfilename
#include "p_local.h" // [crispy] struct maplumpinfo

#include "s_sound.h"

#include "doomstat.h"

// Data.
#include "sounds.h"

#include "m_menu.h"

#include "v_trans.h" // [crispy] colored "invert mouse" message
#include "r_sky.h" // [crispy] R_InitSkyMap()

extern patch_t*		hu_font[HU_FONTSIZE];
extern boolean		message_dontfuckwithme;

extern boolean		chat_on;		// in heads-up code

//
// defaulted values
//
int			mouseSensitivity = 5;
int			mouseSensitivity_y = 5;

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
char*			messageString;

// message x & y
int			messx;
int			messy;
int			messageLastMenuActive;

// timed message = no input from user
boolean			messageNeedsInput;

void    (*messageRoutine)(int response);

char gammamsg[5][26] =
{
    GAMMALVL0,
    GAMMALVL1,
    GAMMALVL2,
    GAMMALVL3,
    GAMMALVL4
};

// we are going to be entering a savegame string
int			saveStringEnter;              
int             	saveSlot;	// which slot to save in
int			saveCharIndex;	// which char we're editing
// old save description before edit
char			saveOldString[SAVESTRINGSIZE];  

boolean			inhelpscreens;
boolean			menuactive;

#define SKULLXOFF		-32
#define LINEHEIGHT		16
#define CRISPY_LINEHEIGHT	10

extern boolean		sendpause;
char			savegamestrings[10][SAVESTRINGSIZE];

char	endstring[160];

static boolean opldev;

int crispy_screenshotmsg = 0;
int crispy_cleanscreenshot = 0;
extern boolean speedkeydown (void);

//
// MENU TYPEDEFS
//
typedef struct
{
    // 0 = no cursor here, 1 = ok, 2 = arrows ok
    short	status;
    
    char	name[10];
    
    // choice = menu item #.
    // if status = 2,
    //   choice=0:leftarrow,1:rightarrow
    void	(*routine)(int choice);
    
    // hotkey in menu
    char	alphaKey;			
    char	*alttext; // [crispy] alternative text for the Options menu
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
} menu_t;

short		itemOn;			// menu item skull is on
short		skullAnimCounter;	// skull animation counter
short		whichSkull;		// which skull to draw

// graphic name of skulls
// warning: initializer-string for array of chars is too long
char    *skullName[2] = {"M_SKULL1","M_SKULL2"};

// current menudef
menu_t*	currentMenu;                          

//
// PROTOTYPES
//
void M_NewGame(int choice);
void M_Episode(int choice);
static void M_Expansion(int choice);
void M_ChooseSkill(int choice);
void M_LoadGame(int choice);
void M_SaveGame(int choice);
void M_Options(int choice);
void M_EndGame(int choice);
void M_ReadThis(int choice);
void M_ReadThis2(int choice);
void M_QuitDOOM(int choice);

void M_ChangeMessages(int choice);
void M_ChangeSensitivity(int choice);
static void M_ChangeSensitivity_y(int choice);
static void M_MouseInvert(int choice);
static void M_MouseLook(int choice);
void M_SfxVol(int choice);
void M_MusicVol(int choice);
void M_ChangeDetail(int choice);
void M_SizeDisplay(int choice);
void M_StartGame(int choice);
static void M_Mouse(int choice);
void M_Sound(int choice);

void M_FinishReadThis(int choice);
void M_LoadSelect(int choice);
void M_SaveSelect(int choice);
void M_ReadSaveStrings(void);
void M_QuickSave(void);
void M_QuickLoad(void);

void M_DrawMainMenu(void);
void M_DrawReadThis1(void);
void M_DrawReadThis2(void);
void M_DrawNewGame(void);
void M_DrawEpisode(void);
void M_DrawOptions(void);
static void M_DrawMouse(void);
void M_DrawSound(void);
void M_DrawLoad(void);
void M_DrawSave(void);

void M_DrawSaveLoadBorder(int x,int y);
void M_SetupNextMenu(menu_t *menudef);
void M_DrawThermo(int x,int y,int thermWidth,int thermDot);
void M_DrawEmptyCell(menu_t *menu,int item);
void M_DrawSelCell(menu_t *menu,int item);
void M_WriteText(int x, int y, char *string);
int  M_StringWidth(char *string);
int  M_StringHeight(char *string);
void M_StartMessage(char *string,void *routine,boolean input);
void M_StopMessage(void);
void M_ClearMenus (void);

static void M_CrispyToggleAutomapstats(int choice);
static void M_CrispyToggleCenterweapon(int choice);
static void M_CrispyToggleColoredblood(int choice);
static void M_CrispyToggleColoredblood2(int choice);
static void M_CrispyToggleColoredhud(int choice);
static void M_CrispyToggleCrosshair(int choice);
static void M_CrispyToggleCrosshairtype(int choice);
static void M_CrispyToggleExtsaveg(int choice);
static void M_CrispyToggleFlipcorpses(int choice);
static void M_CrispyToggleFreeaim(int choice);
static void M_CrispyToggleFreelook(int choice);
static void M_CrispyToggleFullsounds(int choice);
static void M_CrispyToggleJumping(int choice);
static void M_CrispyToggleNeghealth(int choice);
static void M_CrispyToggleOverunder(int choice);
static void M_CrispyTogglePitch(int choice);
static void M_CrispyToggleRecoil(int choice);
static void M_CrispyToggleSecretmessage(int choice);
static void M_CrispyToggleTranslucency(int choice);
static void M_CrispyToggleUncapped(int choice);
static void M_Crispness(int choice);
static void M_Crispness1(int choice);
static void M_Crispness2(int choice);
static void M_Crispness3(int choice);
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

//
// EXPANSION SELECT
//
enum
{
    ex1,
    ex2,
    ex_end
} expansions_e;

static menuitem_t ExpansionMenu[]=
{
    {1,"M_EPI1", M_Expansion,'h'},
    {1,"M_EPI2", M_Expansion,'n'},
};

static menu_t  ExpDef =
{
    ex_end,		// # of menu items
    &MainDef,		// previous menu
    ExpansionMenu,	// menuitem_t ->
    M_DrawEpisode,	// drawing routine ->
    48,63,              // x,y
    ex1			// lastOn
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
    {1,"M_JKILL",	M_ChooseSkill, 'i'},
    {1,"M_ROUGH",	M_ChooseSkill, 'h'},
    {1,"M_HURT",	M_ChooseSkill, 'h'},
    {1,"M_ULTRA",	M_ChooseSkill, 'u'},
    {1,"M_NMARE",	M_ChooseSkill, 'n'}
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
    crispness, // [crispy] crispness menu
    opt_end
} options_e;

menuitem_t OptionsMenu[]=
{
    {1,"M_ENDGAM",	M_EndGame,'e', "End Game"},
    {1,"M_MESSG",	M_ChangeMessages,'m', "Messages: "},
    {1,"M_DETAIL",	M_ChangeDetail,'g', "Graphic Detail: "},
    {2,"M_SCRNSZ",	M_SizeDisplay,'s', "Screen Size"},
    {-1,"",0,'\0'},
    {1,"M_MSENS",	M_Mouse,'m', "Mouse Sensitivity"}, // [crispy] mouse sensitivity menu
    {1,"M_SVOL",	M_Sound,'s', "Sound Volume"},
    {1,"M_CRISPY",	M_Crispness,'c', "Crispness"} // [crispy] crispness menu
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
    mouse_vert,
    mouse_empty2,
    mouse_invert,
    mouse_look,
    mouse_end
} mouse_e;

static menuitem_t MouseMenu[]=
{
    {2,"",	M_ChangeSensitivity,'h'},
    {-1,"",0,'\0'},
    {2,"",	M_ChangeSensitivity_y,'v'},
    {-1,"",0,'\0'},
    {1,"",	M_MouseInvert,'i'},
    {1,"",	M_MouseLook,'l'},
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

// [crispy] crispness menu
enum
{
    crispness_sep_visual,
    crispness_uncapped,
    crispness_coloredhud,
    crispness_translucency,
    crispness_coloredblood,
    crispness_coloredblood2,
    crispness_flipcorpses,
    crispness1_sep_audible,
    crispness_sep_audible,
    crispness_fullsounds,
    crispness1_sep_goto2,
    crispness1_goto2,
    crispness1_end
} crispness_e;

static menuitem_t CrispnessMenu[]=
{
    {-1,"",0,'\0'},
    {1,"",	M_CrispyToggleUncapped,'u'},
    {1,"",	M_CrispyToggleColoredhud,'c'},
    {1,"",	M_CrispyToggleTranslucency,'e'},
    {1,"",	M_CrispyToggleColoredblood,'e'},
    {1,"",	M_CrispyToggleColoredblood2,'f'},
    {1,"",	M_CrispyToggleFlipcorpses,'r'},
    {-1,"",0,'\0'},
    {-1,"",0,'\0'},
    {1,"",	M_CrispyToggleFullsounds,'p'},
    {-1,"",0,'\0'},
    {1,"",	M_Crispness2,'n'},
};

static menu_t  Crispness1Def =
{
    crispness1_end,
    &OptionsDef,
    CrispnessMenu,
    M_DrawCrispness1,
    48,36,
    1
};

static menu_t *CrispnessXDef = &Crispness1Def;

enum
{
    crispness_sep_tactical,
    crispness_crosshair,
    crispness_crosshairtype,
    crispness_freelook,
    crispness_neghealth,
    crispness_centerweapon,
    crispness_pitch,
    crispness_secretmessage,
    crispness_automapstats,
    crispness_extsaveg,
    crispness2_sep_goto2,
    crispness2_goto1,
    crispness2_goto3,
    crispness2_end
} crispness2_e;

static menuitem_t Crispness2Menu[]=
{
    {-1,"",0,'\0'},
    {1,"",	M_CrispyToggleCrosshair,'d'},
    {1,"",	M_CrispyToggleCrosshairtype,'c'},
    {1,"",	M_CrispyToggleFreelook,'a'},
    {1,"",	M_CrispyToggleNeghealth,'n'},
    {1,"",	M_CrispyToggleCenterweapon,'c'},
    {1,"",	M_CrispyTogglePitch,'w'},
    {1,"",	M_CrispyToggleSecretmessage,'s'},
    {1,"",	M_CrispyToggleAutomapstats,'s'},
    {1,"",	M_CrispyToggleExtsaveg,'e'},
    {-1,"",0,'\0'},
    {1,"",	M_Crispness1,'p'},
    {1,"",	M_Crispness3,'n'},
};

static menu_t  Crispness2Def =
{
    crispness2_end,
    &Crispness1Def,
    Crispness2Menu,
    M_DrawCrispness2,
    48,36,
    1
};

enum
{
    crispness_sep_physical,
    crispness_freeaim,
    crispness_jumping,
    crispness_overunder,
    crispness_recoil,
    crispness3_sep_goto1,
    crispness3_goto2,
    crispness3_end
} crispness3_e;

static menuitem_t Crispness3Menu[]=
{
    {-1,"",0,'\0'},
    {1,"",	M_CrispyToggleFreeaim,'v'},
    {1,"",	M_CrispyToggleJumping,'a'},
    {1,"",	M_CrispyToggleOverunder,'w'},
    {1,"",	M_CrispyToggleRecoil,'w'},
    {-1,"",0,'\0'},
    {1,"",	M_Crispness2,'p'},
};

static menu_t  Crispness3Def =
{
    crispness3_end,
    &Crispness2Def,
    Crispness3Menu,
    M_DrawCrispness3,
    48,36,
    1
};

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
        M_StringCopy(name, P_SaveGameFile(i), sizeof(name));

	handle = fopen(name, "rb");
        if (handle == NULL)
        {
            M_StringCopy(savegamestrings[i], EMPTYSTRING, SAVESTRINGSIZE);
            LoadMenu[i].status = 0;
            continue;
        }
	fread(&savegamestrings[i], 1, SAVESTRINGSIZE, handle);
	fclose(handle);
	LoadMenu[i].status = 1;
    }
}


//
// M_LoadGame & Cie.
//
static int LoadDef_x = 72, LoadDef_y = 28;
void M_DrawLoad(void)
{
    int             i;
	
    V_DrawPatchShadow2(LoadDef_x, LoadDef_y,
                      W_CacheLumpName(DEH_String("M_LOADG"), PU_CACHE));

    for (i = 0;i < load_end; i++)
    {
	M_DrawSaveLoadBorder(LoadDef.x,LoadDef.y+LINEHEIGHT*i);

	// [crispy] shade empty savegame slots
	if (!LoadMenu[i].status)
	    dp_translation = cr[CR_DARK];

	M_WriteText(LoadDef.x,LoadDef.y+LINEHEIGHT*i,savegamestrings[i]);

	V_ClearDPTranslation();
    }
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
    // [crispy] forbid New Game and (Quick) Load while recording a demo
    if (demorecording)
    {
	return;
    }

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
	
    V_DrawPatchShadow2(SaveDef_x, SaveDef_y, W_CacheLumpName(DEH_String("M_SAVEG"), PU_CACHE));
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
// User wants to save. Start string input for M_Responder
//
void M_SaveSelect(int choice)
{
    // we are going to be intercepting all chars
    saveStringEnter = 1;
    
    // [crispy] load the last game you saved
    LoadDef.lastOn = choice;
    saveSlot = choice;
    M_StringCopy(saveOldString,savegamestrings[choice], SAVESTRINGSIZE);
    if (!strcmp(savegamestrings[choice], EMPTYSTRING))
	savegamestrings[choice][0] = 0;
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
char    tempstring[80];

void M_QuickSaveResponse(int key)
{
    if (key == key_menu_confirm)
    {
	M_DoSave(quickSaveSlot);
	S_StartSound(NULL,sfx_swtchx);
    }
}

void M_QuickSave(void)
{
    char *savegamestring;

    if (!usergame)
    {
	S_StartSound(NULL,sfx_oof);
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
    // [crispy] print savegame name in golden letters
    savegamestring = M_StringJoin(crstr[CR_GOLD],
                                  savegamestrings[quickSaveSlot],
                                  crstr[CR_NONE],
                                  NULL);
    DEH_snprintf(tempstring, 80, QSPROMPT, savegamestring);
    free(savegamestring);
    M_StartMessage(tempstring,M_QuickSaveResponse,true);
}



//
// M_QuickLoad
//
void M_QuickLoadResponse(int key)
{
    if (key == key_menu_confirm)
    {
	M_LoadSelect(quickSaveSlot);
	S_StartSound(NULL,sfx_swtchx);
    }
}


void M_QuickLoad(void)
{
    char *savegamestring;

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
    // [crispy] print savegame name in golden letters
    savegamestring = M_StringJoin(crstr[CR_GOLD],
                                  savegamestrings[quickSaveSlot],
                                  crstr[CR_NONE],
                                  NULL);
    DEH_snprintf(tempstring, 80, QLPROMPT, savegamestring);
    free(savegamestring);
    M_StartMessage(tempstring,M_QuickLoadResponse,true);
}




//
// Read This Menus
// Had a "quick hack to fix romero bug"
//
void M_DrawReadThis1(void)
{
    inhelpscreens = true;

    V_DrawPatchDirect(0, 0, W_CacheLumpName(DEH_String("HELP2"), PU_CACHE));
}



//
// Read This Menus - optional second page.
//
void M_DrawReadThis2(void)
{
    inhelpscreens = true;

    // We only ever draw the second page if this is 
    // gameversion == exe_doom_1_9 and gamemode == registered

    V_DrawPatchDirect(0, 0, W_CacheLumpName(DEH_String("HELP1"), PU_CACHE));
}

void M_DrawReadThisCommercial(void)
{
    inhelpscreens = true;

    V_DrawPatchDirect(0, 0, W_CacheLumpName(DEH_String("HELP"), PU_CACHE));
}


//
// Change Sfx & Music volumes
//
void M_DrawSound(void)
{
    V_DrawPatchShadow2 (60, 38, W_CacheLumpName(DEH_String("M_SVOL"), PU_CACHE));

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
    V_DrawPatchDirect(94, 2,
                      W_CacheLumpName(DEH_String("M_DOOM"), PU_CACHE));
}




//
// M_NewGame
//
void M_DrawNewGame(void)
{
    V_DrawPatchShadow2(96, 14, W_CacheLumpName(DEH_String("M_NEWG"), PU_CACHE));
    V_DrawPatchShadow2(54, 38, W_CacheLumpName(DEH_String("M_SKILL"), PU_CACHE));
}

void M_NewGame(int choice)
{
    // [crispy] forbid New Game and (Quick) Load while recording a demo
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

    if (nervewadfile)
	M_SetupNextMenu(&ExpDef);
    else
    if (gamemode == commercial || gameversion == exe_chex)
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
    V_DrawPatchShadow2(54, 38, W_CacheLumpName(DEH_String("M_EPISOD"), PU_CACHE));
}

void M_VerifyNightmare(int key)
{
    if (key != key_menu_confirm)
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
    M_SetupNextMenu(&NewDef);
}

static void M_Expansion(int choice)
{
    epi = choice;
    M_SetupNextMenu(&NewDef);
}


//
// M_Options
//
// [crispy] no patches are drawn in the Options menu anymore
/*
static char *detailNames[2] = {"M_GDHIGH","M_GDLOW"};
static char *msgNames[2] = {"M_MSGOFF","M_MSGON"};
*/

void M_DrawOptions(void)
{
    V_DrawPatchShadow2(108, 15, W_CacheLumpName(DEH_String("M_OPTTTL"),
                                               PU_CACHE));
	
// [crispy] no patches are drawn in the Options menu anymore
/*
    V_DrawPatchDirect(OptionsDef.x + 175, OptionsDef.y + LINEHEIGHT * detail,
		      W_CacheLumpName(DEH_String(detailNames[detailLevel]),
			              PU_CACHE));
*/

    M_WriteText(OptionsDef.x + M_StringWidth("Graphic Detail: "),
                OptionsDef.y + LINEHEIGHT * detail + 8 - (M_StringHeight("HighLow")/2),
                detailLevel ? "Low" : "High");

// [crispy] no patches are drawn in the Options menu anymore
/*
    V_DrawPatchDirect(OptionsDef.x + 120, OptionsDef.y + LINEHEIGHT * messages,
                      W_CacheLumpName(DEH_String(msgNames[showMessages]),
                                      PU_CACHE));
*/
    M_WriteText(OptionsDef.x + M_StringWidth("Messages: "),
                OptionsDef.y + LINEHEIGHT * messages + 8 - (M_StringHeight("OnOff")/2),
                showMessages ? "On" : "Off");

    M_DrawThermo(OptionsDef.x,OptionsDef.y+LINEHEIGHT*(scrnsize+1),
		 9 + 2,screenSize); // [crispy] Crispy HUD
}

// [crispy] mouse sensitivity menu
static void M_DrawMouse(void)
{
    char mouse_menu_text[48];

    V_DrawPatchShadow2 (60, 38, W_CacheLumpName(DEH_String("M_MSENS"), PU_CACHE));

    M_WriteText(MouseDef.x, MouseDef.y + LINEHEIGHT * mouse_horiz + 6,
                "HORIZONTAL");

    M_DrawThermo(MouseDef.x, MouseDef.y + LINEHEIGHT * mouse_empty1,
		 21, mouseSensitivity);

    M_WriteText(MouseDef.x, MouseDef.y + LINEHEIGHT * mouse_vert + 6,
                "VERTICAL");

    M_DrawThermo(MouseDef.x, MouseDef.y + LINEHEIGHT * mouse_empty2,
		 21, mouseSensitivity_y);

    M_snprintf(mouse_menu_text, sizeof(mouse_menu_text),
               "%sInvert Vertical Axis: %s%s", crstr[CR_NONE],
               mouse_y_invert ? crstr[CR_GREEN] : crstr[CR_DARK],
               mouse_y_invert ? "On" : "Off");
    M_WriteText(MouseDef.x, MouseDef.y + LINEHEIGHT * mouse_invert + 6,
                mouse_menu_text);

    M_snprintf(mouse_menu_text, sizeof(mouse_menu_text),
               "%sPermanent Mouse Look: %s%s", crstr[CR_NONE],
               crispy_mouselook ? crstr[CR_GREEN] : crstr[CR_DARK],
               crispy_mouselook ? "On" : "Off");
    M_WriteText(MouseDef.x, MouseDef.y + LINEHEIGHT * mouse_look + 6,
                mouse_menu_text);

    V_ClearDPTranslation();
}

// [crispy] crispness menu
static void M_DrawCrispnessBackground(void)
{
    static byte *sdest;

    inhelpscreens = true;

    if (!sdest)
    {
	byte *src, *dest;
	int x, y;

	src = W_CacheLumpName("FLOOR4_6" , PU_CACHE);
	dest = (unsigned char *) Z_Malloc (SCREENWIDTH * SCREENHEIGHT * sizeof(*dest), PU_STATIC, NULL);
	sdest = dest;

	for (y = 0; y < SCREENHEIGHT; y++)
	{
	    for (x = 0; x < SCREENWIDTH; x++)
	    {
		*dest++ = src[(y & 63) * 64 + (x & 63)];
	    }
	}
    }

    memcpy(I_VideoBuffer, sdest, SCREENWIDTH * SCREENHEIGHT * sizeof(*I_VideoBuffer));
}

static void M_DrawCrispnessHeader(char *item)
{
    char crispy_menu_text[48];

    M_snprintf(crispy_menu_text, sizeof(crispy_menu_text),
               "%s%s", crstr[CR_GOLD], item);
    M_WriteText(ORIGWIDTH/2 - M_StringWidth(item) / 2, 20, crispy_menu_text);
}

static void M_DrawCrispnessSeparator(int y, char *item)
{
    char crispy_menu_text[48];

    M_snprintf(crispy_menu_text, sizeof(crispy_menu_text),
               "%s%s", crstr[CR_GOLD], item);
    M_WriteText(currentMenu->x - 8, currentMenu->y + CRISPY_LINEHEIGHT * y, crispy_menu_text);
}

static void M_DrawCrispnessItem(int y, char *item, int feat, boolean cond)
{
    char crispy_menu_text[48];

    M_snprintf(crispy_menu_text, sizeof(crispy_menu_text),
               "%s%s: %s%s", cond ? crstr[CR_NONE] : crstr[CR_DARK], item,
               cond ? (feat ? crstr[CR_GREEN] : crstr[CR_DARK]) : crstr[CR_DARK],
               cond && feat ? "On" : "Off");
    M_WriteText(currentMenu->x, currentMenu->y + CRISPY_LINEHEIGHT * y, crispy_menu_text);
}

typedef struct
{
    int value;
    char *name;
} multiitem_t;

static multiitem_t multiitem_centerweapon[NUM_CENTERWEAPON] =
{
    {CENTERWEAPON_OFF, "off"},
    {CENTERWEAPON_HOR, "horizontal"},
    {CENTERWEAPON_HORVER, "centered"},
    {CENTERWEAPON_BOB, "bobbing"},
};

static multiitem_t multiitem_coloredblood[NUM_COLOREDBLOOD] =
{
    {COLOREDBLOOD_OFF, "off"},
    {COLOREDBLOOD_BLOOD, "blood"},
    {COLOREDBLOOD_CORPSE, "corpses"},
    {COLOREDBLOOD_BOTH, "both"},
};

static multiitem_t multiitem_coloredhud[NUM_COLOREDHUD] =
{
    {COLOREDHUD_OFF, "off"},
    {COLOREDHUD_BAR, "status bar"},
    {COLOREDHUD_TEXT, "hud texts"},
    {COLOREDHUD_BOTH, "both"},
};

static multiitem_t multiitem_crosshair[NUM_CROSSHAIRS] =
{
    {CROSSHAIR_OFF, "off"},
    {CROSSHAIR_STATIC, "static"},
    {CROSSHAIR_PROJECTED, "projected"},
};

static multiitem_t multiitem_crosshairtype[] =
{
    {-1, ""},
    {0, "cross"},
    {1, "chevron"},
    {2, "dot"},
};

static multiitem_t multiitem_freeaim[NUM_FREEAIMS] =
{
    {FREEAIM_AUTO, "autoaim"},
    {FREEAIM_DIRECT, "direct"},
    {FREEAIM_BOTH, "both"},
};

static multiitem_t multiitem_freelook[NUM_FREELOOKS] =
{
    {FREELOOK_OFF, "off"},
    {FREELOOK_SPRING, "spring"},
    {FREELOOK_LOCK, "lock"},
};

static multiitem_t multiitem_jump[NUM_JUMPS] =
{
    {JUMP_OFF, "off"},
    {JUMP_LOW, "low"},
    {JUMP_HIGH, "high"},
};

static multiitem_t multiitem_neghealth[NUM_NEGHEALTHS] =
{
    {NEGHEALTH_OFF, "off"},
    {NEGHEALTH_DM, "deathmatch"},
    {NEGHEALTH_ON, "always"},
};

static multiitem_t multiitem_translucency[NUM_TRANSLUCENCY] =
{
    {TRANSLUCENCY_OFF, "off"},
    {TRANSLUCENCY_MISSILE, "projectiles"},
    {TRANSLUCENCY_ITEM, "items"},
    {TRANSLUCENCY_BOTH, "both"},
};

static multiitem_t multiitem_uncapped[NUM_UNCAPPED] =
{
    {UNCAPPED_OFF, "35 fps"},
    {UNCAPPED_ON, "uncapped"},
    {UNCAPPED_60FPS, "60 fps"},
    {UNCAPPED_70FPS, "70 fps"},
};

static void M_DrawCrispnessMultiItem(int y, char *item, multiitem_t *multiitem, int feat, boolean cond)
{
    char crispy_menu_text[48];

    M_snprintf(crispy_menu_text, sizeof(crispy_menu_text),
               "%s%s: %s%s", cond ? crstr[CR_NONE] : crstr[CR_DARK], item,
               cond ? (feat ? crstr[CR_GREEN] : crstr[CR_DARK]) : crstr[CR_DARK],
               multiitem[feat].name);
    M_WriteText(currentMenu->x, currentMenu->y + CRISPY_LINEHEIGHT * y, crispy_menu_text);
}

static void M_DrawCrispnessGoto(int y, char *item)
{
    char crispy_menu_text[48];

    M_snprintf(crispy_menu_text, sizeof(crispy_menu_text),
               "%s%s", crstr[CR_GOLD], item);
    M_WriteText(currentMenu->x, currentMenu->y + CRISPY_LINEHEIGHT * y, crispy_menu_text);
}

static void M_DrawCrispness1(void)
{
    CrispnessXDef = &Crispness1Def;

    M_DrawCrispnessBackground();

    M_DrawCrispnessHeader("Crispness 1/3");

    M_DrawCrispnessSeparator(crispness_sep_visual, "Visual");

    M_DrawCrispnessMultiItem(crispness_uncapped, "Rendering Framerate", multiitem_uncapped, crispy_uncapped, true);
    M_DrawCrispnessMultiItem(crispness_coloredhud, "Colorize HUD Elements", multiitem_coloredhud, crispy_coloredhud, true);
    M_DrawCrispnessMultiItem(crispness_translucency, "Enable Translucency", multiitem_translucency, crispy_translucency, true);
    M_DrawCrispnessMultiItem(crispness_coloredblood, "Colored Blood and Corpses", multiitem_coloredblood, crispy_coloredblood & COLOREDBLOOD_BOTH, true);
    M_DrawCrispnessItem(crispness_coloredblood2, "Fix Spectre and Lost Soul Blood", crispy_coloredblood & COLOREDBLOOD_FIX, true);
    M_DrawCrispnessItem(crispness_flipcorpses, "Randomly Mirrored Corpses", crispy_flipcorpses, true);

    M_DrawCrispnessSeparator(crispness_sep_audible, "Audible");
    M_DrawCrispnessItem(crispness_fullsounds, "Play sounds in full length", crispy_fullsounds, true);

    M_DrawCrispnessGoto(crispness1_goto2, "Next Page >");

    V_ClearDPTranslation();
}

static void M_DrawCrispness2(void)
{
    CrispnessXDef = &Crispness2Def;

    M_DrawCrispnessBackground();

    M_DrawCrispnessHeader("Crispness 2/3");

    M_DrawCrispnessSeparator(crispness_sep_tactical, "Tactical");

    M_DrawCrispnessMultiItem(crispness_crosshair, "Draw Crosshair", multiitem_crosshair, crispy_crosshair, true);
    M_DrawCrispnessMultiItem(crispness_crosshairtype, "Crosshair Type", multiitem_crosshairtype, crispy_crosshairtype + 1, crispy_crosshair);
    M_DrawCrispnessMultiItem(crispness_freelook, "Allow Free Look", multiitem_freelook, crispy_freelook, true);
    M_DrawCrispnessMultiItem(crispness_neghealth, "Negative Player Health", multiitem_neghealth, crispy_neghealth, true);
    M_DrawCrispnessMultiItem(crispness_centerweapon, "Weapon Attack Alignment", multiitem_centerweapon, crispy_centerweapon, true);
    M_DrawCrispnessItem(crispness_pitch, "Weapon Recoil Pitch", crispy_pitch, true);
    M_DrawCrispnessItem(crispness_secretmessage, "Show Revealed Secrets", crispy_secretmessage, true);
    M_DrawCrispnessItem(crispness_automapstats, "Show Level Stats in Automap", crispy_automapstats, true);
    M_DrawCrispnessItem(crispness_extsaveg, "Extended Savegames", crispy_extsaveg, true);

    M_DrawCrispnessGoto(crispness2_goto3, "Next Page >");
    M_DrawCrispnessGoto(crispness2_goto1, "< Prev Page");

    V_ClearDPTranslation();
}

static void M_DrawCrispness3(void)
{
    CrispnessXDef = &Crispness3Def;

    M_DrawCrispnessBackground();

    M_DrawCrispnessHeader("Crispness 3/3");

    M_DrawCrispnessSeparator(crispness_sep_physical, "Physical");

    M_DrawCrispnessMultiItem(crispness_freeaim, "Vertical Aiming", multiitem_freeaim, crispy_freeaim, singleplayer);
    M_DrawCrispnessMultiItem(crispness_jumping, "Allow Jumping", multiitem_jump, crispy_jump, singleplayer);
    M_DrawCrispnessItem(crispness_overunder, "Walk over/under Monsters", crispy_overunder, singleplayer);
    M_DrawCrispnessItem(crispness_recoil, "Weapon Recoil Thrust", crispy_recoil, singleplayer);

    M_DrawCrispnessGoto(crispness3_goto2, "< Prev Page");

    V_ClearDPTranslation();
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

static void M_Crispness(int choice)
{
    M_SetupNextMenu(CrispnessXDef);
}

static void M_Crispness1(int choice)
{
    M_SetupNextMenu(&Crispness1Def);
}

static void M_Crispness2(int choice)
{
    M_SetupNextMenu(&Crispness2Def);
}

static void M_Crispness3(int choice)
{
    M_SetupNextMenu(&Crispness3Def);
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
    if (key != key_menu_confirm)
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

    if (key != key_menu_confirm)
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


static char *M_SelectEndMessage(void)
{
    char **endmsg;

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

static void M_MouseLook(int choice)
{
    choice = 0;
    crispy_mouselook = !crispy_mouselook;

    players[consoleplayer].lookdir = 0;
    R_InitSkyMap();
}

static void M_CrispyToggleAutomapstats(int choice)
{
    choice = 0;
    crispy_automapstats = !crispy_automapstats;
}

static void M_CrispyToggleExtsaveg(int choice)
{
    choice = 0;
    crispy_extsaveg = !crispy_extsaveg;
}

static void M_CrispyToggleCenterweapon(int choice)
{
    choice = 0;
    crispy_centerweapon = (crispy_centerweapon + 1) % NUM_CENTERWEAPON;
}

static void M_CrispyToggleColoredblood(int choice)
{
    // [crispy] preserve coloredblood_fix value when switching colored blood and corpses
    const int coloredblood_fix = crispy_coloredblood & COLOREDBLOOD_FIX;
    choice = 0;
    crispy_coloredblood = (crispy_coloredblood + 1) % NUM_COLOREDBLOOD;
    crispy_coloredblood |= coloredblood_fix;
}

static void M_CrispyToggleColoredblood2(int choice)
{
    choice = 0;
    crispy_coloredblood ^= COLOREDBLOOD_FIX;
}

static void M_CrispyToggleColoredhud(int choice)
{
    choice = 0;
    crispy_coloredhud = (crispy_coloredhud + 1) % NUM_COLOREDHUD;
}

static void M_CrispyToggleCrosshair(int choice)
{
    choice = 0;
    crispy_crosshair = (crispy_crosshair + 1) % NUM_CROSSHAIRS;
}

static void M_CrispyToggleCrosshairtype(int choice)
{
    if (!crispy_crosshair)
    {
	S_StartSound(NULL,sfx_oof);
	return;
    }

    choice = 0;
    crispy_crosshairtype = crispy_crosshairtype + 1;

    if (!laserpatch[crispy_crosshairtype].c)
    {
	crispy_crosshairtype = 0;
    }
}

static void M_CrispyToggleFlipcorpses(int choice)
{
    choice = 0;
    crispy_flipcorpses = !crispy_flipcorpses;
}

static void M_CrispyToggleFreeaim(int choice)
{
    if (!singleplayer)
    {
	S_StartSound(NULL,sfx_oof);
	return;
    }

    choice = 0;
    crispy_freeaim = (crispy_freeaim + 1) % NUM_FREEAIMS;
}

static void M_CrispyToggleFreelook(int choice)
{
    choice = 0;
    crispy_freelook = (crispy_freelook + 1) % NUM_FREELOOKS;

    players[consoleplayer].lookdir = 0;
    R_InitSkyMap();
}

static void M_CrispyToggleNeghealth(int choice)
{
    choice = 0;
    crispy_neghealth = (crispy_neghealth + 1) % NUM_NEGHEALTHS;
}

static void M_CrispyToggleJumping(int choice)
{
    if (!singleplayer)
    {
	S_StartSound(NULL,sfx_oof);
	return;
    }

    choice = 0;
    crispy_jump = (crispy_jump + 1) % NUM_JUMPS;
}

static void M_CrispyToggleOverunder(int choice)
{
    if (!singleplayer)
    {
	S_StartSound(NULL,sfx_oof);
	return;
    }

    choice = 0;
    crispy_overunder = !crispy_overunder;
}

static void M_CrispyTogglePitch(int choice)
{
    choice = 0;
    crispy_pitch = !crispy_pitch;
    R_InitSkyMap();
}

static void M_CrispyToggleRecoil(int choice)
{
    if (!singleplayer)
    {
	S_StartSound(NULL,sfx_oof);
	return;
    }

    choice = 0;
    crispy_recoil = !crispy_recoil;
}

static void M_CrispyToggleSecretmessage(int choice)
{
    choice = 0;
    crispy_secretmessage = !crispy_secretmessage;
}

static void M_CrispyToggleTranslucency(int choice)
{
    choice = 0;
    crispy_translucency = (crispy_translucency + 1) % NUM_TRANSLUCENCY;
}

static void M_CrispyToggleUncapped(int choice)
{
    choice = 0;
    crispy_uncapped = (crispy_uncapped + 1) % NUM_UNCAPPED;
}

static void M_CrispyToggleFullsounds(int choice)
{
    choice = 0;
    crispy_fullsounds = !crispy_fullsounds;
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
	if (screenSize < 8 + 2) // [crispy] Crispy HUD
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

    V_ClearDPTranslation();
}



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
( char*		string,
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


void M_StopMessage(void)
{
    menuactive = messageLastMenuActive;
    messageToPrint = 0;
}



//
// Find string width from hu_font chars
//
int M_StringWidth(char* string)
{
    size_t             i;
    int             w = 0;
    int             c;
	
    for (i = 0;i < strlen(string);i++)
    {
	// [crispy] correctly center colorized strings
	if (string[i] == '\x1b')
	{
	    i++;
	    continue;
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
int M_StringHeight(char* string)
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
  char*		string)
{
    int		w;
    char*	ch;
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
	if (c == '\x1b')
	{
	    c = *ch++;
	    dp_translation = cr[(int) (c - '0')];
	    continue;
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
	if (!messageToPrint && (currentMenu == &LoadDef || currentMenu == &SaveDef))
	{
	V_DrawPatchDirect(cx, cy, hu_font[c]);
	}
	else
	{
	    V_DrawPatchShadow1(cx, cy, hu_font[c]);
	}
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
    G_DeferedInitNew(gameskill, gameepisode, gamemap);
    result = true;
  }

  return result;
}

static int G_GotoNextLevel(void)
{
  static byte doom_next[4][9] = {
    {12, 13, 19, 15, 16, 17, 18, 21, 14},
    {22, 23, 24, 25, 29, 27, 28, 31, 26},
    {32, 33, 34, 35, 36, 39, 38, 41, 37},
    {42, 49, 44, 45, 46, 47, 48, 11, 43}
  };
  static byte doom2_next[33] = {
    0, 3, 4, 5, 6, 7, 8, 9, 10, 11,
    12, 13, 14, 15, 31, 17, 18, 19, 20, 21,
    22, 23, 24, 25, 26, 27, 28, 29, 30, 1,
    32, 16, 3
  };
  static byte nerve_next[9] = {
    2, 3, 4, 9, 6, 7, 8, 1, 5
  };

  int changed = false;

  // [crispy] process only once
  if (!doom2_next[0])
  {
    doom2_next[0] = 2;

    if (gamemode == commercial)
    {
      if (crispy_havemap33)
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

      if (gamemission == pack_chex)
      {
        doom_next[0][2] = 14;
        doom_next[0][4] = 11;
      }
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
    if (crispy_havee1m10 && gameepisode == 1)
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
    static  int     joywait = 0;
    static  int     mousewait = 0;
    static  int     mousey = 0;
    static  int     lasty = 0;
    static  int     mousex = 0;
    static  int     lastx = 0;

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
            S_StartSound(NULL,sfx_swtchn);
            M_QuitDOOM(0);
        }

        return true;
    }

    // key is the key pressed, ch is the actual character typed
  
    ch = 0;
    key = -1;
	
    if (ev->type == ev_joystick && joywait < I_GetTime())
    {
	if (ev->data3 < 0)
	{
	    key = key_menu_up;
	    joywait = I_GetTime() + 5;
	}
	else if (ev->data3 > 0)
	{
	    key = key_menu_down;
	    joywait = I_GetTime() + 5;
	}
		
	if (ev->data2 < 0)
	{
	    key = key_menu_left;
	    joywait = I_GetTime() + 2;
	}
	else if (ev->data2 > 0)
	{
	    key = key_menu_right;
	    joywait = I_GetTime() + 2;
	}
		
	if (ev->data1&1)
	{
	    key = key_menu_forward;
	    joywait = I_GetTime() + 5;
	}
	if (ev->data1&2)
	{
	    key = key_menu_back;
	    joywait = I_GetTime() + 5;
	}
        if (joybmenu >= 0 && (ev->data1 & (1 << joybmenu)) != 0)
        {
            key = key_menu_activate;
	    joywait = I_GetTime() + 5;
        }
    }
    else
    {
	if (ev->type == ev_mouse && mousewait < I_GetTime())
	{
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
		mousewait = I_GetTime() + 15;
	    }
			
	    if (ev->data1&2)
	    {
		key = key_menu_back;
		mousewait = I_GetTime() + 15;
	    }

	    // [crispy] scroll menus with mouse wheel
	    if (mousebprevweapon >= 0 && ev->data1 & (1 << mousebprevweapon))
	    {
		key = key_menu_down;
		mousewait = I_GetTime() + 5;
	    }
	    else
	    if (mousebnextweapon >= 0 && ev->data1 & (1 << mousebnextweapon))
	    {
		key = key_menu_up;
		mousewait = I_GetTime() + 5;
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
            M_StringCopy(savegamestrings[saveSlot], saveOldString,
                         SAVESTRINGSIZE);
            break;

	  case KEY_ENTER:
	    saveStringEnter = 0;
	    if (savegamestrings[saveSlot][0])
		M_DoSave(saveSlot);
	    break;

	  default:
            // This is complicated.
            // Vanilla has a bug where the shift key is ignored when entering
            // a savegame name. If vanilla_keyboard_mapping is on, we want
            // to emulate this bug by using 'data1'. But if it's turned off,
            // it implies the user doesn't care about Vanilla emulation: just
            // use the correct 'data2'.

            if (vanilla_keyboard_mapping)
            {
                ch = key;
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
    
    // Take care of any messages that need input
    if (messageToPrint)
    {
	if (messageNeedsInput)
        {
            if (key != ' ' && key != KEY_ESCAPE
             && key != key_menu_confirm && key != key_menu_abort)
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
	S_StartSound(NULL,sfx_swtchx);
	return true;
    }

    // [crispy] take screen shot without weapons and HUD
    if (key != 0 && key == key_menu_cleanscreenshot)
    {
	crispy_cleanscreenshot = (screenblocks > 10) ? 2 : 1;
	key = key_menu_screenshot;
    }

    if ((devparm && key == key_menu_help) ||
        (key != 0 && key == key_menu_screenshot))
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
	    S_StartSound(NULL,sfx_stnmov);
	    return true;
	}
        else if (key == key_menu_incscreen) // Screen size up
        {
	    if (automapactive || chat_on)
		return false;
	    M_SizeDisplay(1);
	    S_StartSound(NULL,sfx_stnmov);
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
	    S_StartSound(NULL,sfx_swtchn);
	    return true;
	}
        else if (key == key_menu_save)     // Save
        {
	    M_StartControlPanel();
	    S_StartSound(NULL,sfx_swtchn);
	    M_SaveGame(0);
	    return true;
        }
        else if (key == key_menu_load)     // Load
        {
	    // [crispy] forbid New Game and (Quick) Load while recording a demo
	    if (demorecording)
	    {
		S_StartSound(NULL,sfx_oof);
	    }
	    else
	    {
	    M_StartControlPanel();
	    S_StartSound(NULL,sfx_swtchn);
	    M_LoadGame(0);
	    }
	    return true;
        }
        else if (key == key_menu_volume)   // Sound Volume
        {
	    M_StartControlPanel ();
	    currentMenu = &SoundDef;
	    itemOn = sfx_vol;
	    S_StartSound(NULL,sfx_swtchn);
	    return true;
	}
        else if (key == key_menu_detail)   // Detail toggle
        {
	    M_ChangeDetail(0);
	    S_StartSound(NULL,sfx_swtchn);
	    return true;
        }
        else if (key == key_menu_qsave)    // Quicksave
        {
	    S_StartSound(NULL,sfx_swtchn);
	    M_QuickSave();
	    return true;
        }
        else if (key == key_menu_endgame)  // End game
        {
	    S_StartSound(NULL,sfx_swtchn);
	    M_EndGame(0);
	    return true;
        }
        else if (key == key_menu_messages) // Toggle messages
        {
	    M_ChangeMessages(0);
	    S_StartSound(NULL,sfx_swtchn);
	    return true;
        }
        else if (key == key_menu_qload)    // Quickload
        {
	    // [crispy] forbid New Game and (Quick) Load while recording a demo
	    if (demorecording)
	    {
		S_StartSound(NULL,sfx_oof);
	    }
	    else
	    {
	    S_StartSound(NULL,sfx_swtchn);
	    M_QuickLoad();
	    }
	    return true;
        }
        else if (key == key_menu_quit)     // Quit DOOM
        {
	    S_StartSound(NULL,sfx_swtchn);
	    M_QuitDOOM(0);
	    return true;
        }
        else if (key == key_menu_gamma)    // gamma toggle
        {
	    usegamma++;
	    if (usegamma > 4)
		usegamma = 0;
	    players[consoleplayer].message = DEH_String(gammamsg[usegamma]);
            I_SetPalette (W_CacheLumpName (DEH_String("PLAYPAL"),PU_CACHE));
	    return true;
	}
        // [crispy] those two can be considered as shortcuts for the IDCLEV cheat
        // and should be treated as such, i.e. add "if (!netgame)"
        else if (!netgame && key == key_menu_reloadlevel)
        {
	    if (G_ReloadLevel())
		return true;
        }
        else if (!netgame && key == key_menu_nextlevel)
        {
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
	    S_StartSound(NULL,sfx_swtchn);
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
	    S_StartSound(NULL,sfx_pstop);
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
	    S_StartSound(NULL,sfx_pstop);
	} while(currentMenu->menuitems[itemOn].status==-1);

	return true;
    }
    else if (key == key_menu_left)
    {
        // Slide slider left

	if (currentMenu->menuitems[itemOn].routine &&
	    currentMenu->menuitems[itemOn].status == 2)
	{
	    S_StartSound(NULL,sfx_stnmov);
	    currentMenu->menuitems[itemOn].routine(0);
	}
	return true;
    }
    else if (key == key_menu_right)
    {
        // Slide slider right

	if (currentMenu->menuitems[itemOn].routine &&
	    currentMenu->menuitems[itemOn].status == 2)
	{
	    S_StartSound(NULL,sfx_stnmov);
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
		S_StartSound(NULL,sfx_stnmov);
	    }
	    else
	    {
		currentMenu->menuitems[itemOn].routine(itemOn);
		S_StartSound(NULL,sfx_pistol);
	    }
	}
	return true;
    }
    else if (key == key_menu_activate)
    {
        // Deactivate menu

	currentMenu->lastOn = itemOn;
	M_ClearMenus ();
	S_StartSound(NULL,sfx_swtchx);
	return true;
    }
    else if (key == key_menu_back)
    {
        // Go back to previous menu

	currentMenu->lastOn = itemOn;
	if (currentMenu->prevMenu)
	{
	    if (nervewadfile && currentMenu == &NewDef)
	        currentMenu->prevMenu = &ExpDef;

	    currentMenu = currentMenu->prevMenu;
	    itemOn = currentMenu->lastOn;
	    S_StartSound(NULL,sfx_swtchn);
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
		S_StartSound(NULL,sfx_oof);
	    }
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
		S_StartSound(NULL,sfx_pstop);
		return true;
	    }
        }

	for (i = 0;i <= itemOn;i++)
        {
	    if (currentMenu->menuitems[i].alphaKey == ch)
	    {
		itemOn = i;
		S_StartSound(NULL,sfx_pstop);
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
    extern void I_OPL_DevMessages(char *, size_t);
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
    char               *name;
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

    for (i=0;i<max;i++)
    {
        name = DEH_String(currentMenu->menuitems[i].name);

	if (name[0])
	{
	    // [crispy] shade unavailable menu items
	    if ((currentMenu == &MainDef && i == savegame && (!usergame || gamestate != GS_LEVEL)) ||
	        (currentMenu == &OptionsDef && i == endgame && (!usergame || netgame)) ||
	        (currentMenu == &MainDef && i == loadgame && ((netgame && !demoplayback) || demorecording)) ||
	        (currentMenu == &MainDef && i == newgame && (demorecording || (netgame && !demoplayback))))
	        dp_translation = cr[CR_DARK];

	    if (currentMenu == &OptionsDef)
	    {
		char *alttext = currentMenu->menuitems[i].alttext;

		if (alttext)
		    M_WriteText(x, y+8-(M_StringHeight(alttext)/2), alttext);
	    }
	    else
	    V_DrawPatchShadow2 (x, y, W_CacheLumpName(name, PU_CACHE));

	    V_ClearDPTranslation();
	}
	y += LINEHEIGHT;
    }

    
    // DRAW SKULL
    if (currentMenu == CrispnessXDef)
    {
	char item[4];
	M_snprintf(item, sizeof(item), "%s>", whichSkull ? crstr[CR_NONE] : crstr[CR_DARK]);
	M_WriteText(currentMenu->x - 8, currentMenu->y + CRISPY_LINEHEIGHT * itemOn, item);
	V_ClearDPTranslation();
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
		LoadDef.y = SaveDef.y = vstep + captionheight + vstep + SHORT(patchm->topoffset) - 7; // [crispy] see M_DrawSaveLoadBorder()
	}
    }

    // [crispy] remove DOS reference from the game quit confirmation dialogs
    if (!M_ParmExists("-nodeh"))
    {
	char *string, *replace;

	// [crispy] "i wouldn't leave if i were you.\ndos is much worse."
	string = doom1_endmsg[3];
	if (!strcmp(string, DEH_String(string)))
	{
		replace = M_StringReplace(string, "dos", "your desktop");
		DEH_AddStringReplacement(string, replace);
		free(replace);
	}

	// [crispy] "you're trying to say you like dos\nbetter than me, right?"
	string = doom1_endmsg[4];
	if (!strcmp(string, DEH_String(string)))
	{
		replace = M_StringReplace(string, "dos\n", "your\ndesktop ");
		DEH_AddStringReplacement(string, replace);
		free(replace);
	}

	// [crispy] "don't go now, there's a \ndimensional shambler waiting\nat the dos prompt!"
	string = doom2_endmsg[2];
	if (!strcmp(string, DEH_String(string)))
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

	if (key != key_menu_confirm)
	{
		M_EndGameResponse(key_menu_confirm);
		savewadfilename = NULL;

		// [crispy] reload Load Game menu
		M_StartControlPanel();
		M_LoadGame(0);
		return;
	}

	savewadfilename = maplumpinfo->wad_file->basename;
	gameaction = ga_loadgame;
}

void M_ForceLoadGame()
{
	savegwarning =
	M_StringJoin("This savegame requires the file\n",
	             crstr[CR_GOLD], savewadfilename, crstr[CR_NONE], "\n",
	             "to restore ", crstr[CR_GOLD], maplumpinfo->name, crstr[CR_NONE], " .\n\n",
	             "Continue to restore from\n",
	             crstr[CR_GOLD], maplumpinfo->wad_file->basename, crstr[CR_NONE], " ?\n\n",
	             PRESSYN, NULL);

	M_StartMessage(savegwarning, M_ForceLoadGameResponse, true);
	messageToPrint = 2;
	S_StartSound(NULL,sfx_swtchn);
}

static void M_ConfirmDeleteGameResponse (int key)
{
	free(savegwarning);

	if (key == key_menu_confirm)
	{
		char name[256];

		M_StringCopy(name, P_SaveGameFile(itemOn), sizeof(name));
		remove(name);

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
	S_StartSound(NULL,sfx_swtchn);
}
