//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 1993-2008 Raven Software
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


// HEADER FILES ------------------------------------------------------------

#include <ctype.h>
#include <time.h> // [crispy] strftime, localtime
#include "h2def.h"
#include "doomkeys.h"
#include "i_input.h"
#include "i_joystick.h"
#include "i_system.h"
#include "i_timer.h"
#include "i_swap.h"
#include "i_timer.h" // [crispy] TICRATE
#include "i_video.h"
#include "m_controls.h"
#include "m_misc.h"
#include "p_local.h"
#include "r_local.h"
#include "s_sound.h"
#include "v_video.h"
#include "am_map.h"

#include "v_trans.h" // [crispy] dp_translation
#include "crispy.h"

// MACROS ------------------------------------------------------------------

#define LEFT_DIR 0
#define RIGHT_DIR 1
#define ENTER_NUMBER 2 // [crispy] numeric entry
#define ITEM_HEIGHT 20
#define SELECTOR_XOFFSET (-28)
#define SELECTOR_YOFFSET (-1)
#define SLOTTEXTLEN	16
#define ASCII_CURSOR '['

// TYPES -------------------------------------------------------------------

typedef enum
{
    ITT_EMPTY,
    ITT_EFUNC,
    ITT_LRFUNC,
    ITT_SETMENU,
    ITT_LRFUNC2, // [crispy] LR non-slider item
    ITT_NUMFUNC, // [crispy] numeric entry
    ITT_INERT
} ItemType_t;

typedef enum
{
    MENU_MAIN,
    MENU_CLASS,
    MENU_SKILL,
    MENU_OPTIONS,
    MENU_OPTIONS2,
    MENU_FILES,
    MENU_LOAD,
    MENU_SAVE,
    MENU_MOUSE,
    MENU_CRISPNESS1,
    MENU_CRISPNESS2,
    MENU_NONE
} MenuType_t;

typedef struct
{
    ItemType_t type;
    const char *text;
    void (*func) (int option);
    int option;
    MenuType_t menu;
} MenuItem_t;

typedef struct
{
    int x;
    int y;
    void (*drawFunc) (void);
    int itemCount;
    MenuItem_t *items;
    int oldItPos;
    MenuType_t prevMenu;
} Menu_t;

// [crispy]
typedef struct
{
    int value;
    const char *name;
} multiitem_t;

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------
extern void R_InitLightTables(void); // [crispy]
extern void I_ReInitGraphics(int reinit); // [crispy]
extern void R_ExecuteSetViewSize(void); // [crispy]
extern void AM_LevelInit(boolean reinit); // [crispy]
extern void AM_initVariables(void); // [crispy]

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

static void InitFonts(void);
static void SetMenu(MenuType_t menu);
static void SCQuitGame(int option);
static void SCClass(int option);
static void SCSkill(int option);
static void SCMouseSensi(int option);
static void SCMouseSensiX2(int option);
static void SCMouseSensiY(int option);
static void SCMouseInvertY(int option);
static void SCSfxVolume(int option);
static void SCMusicVolume(int option);
static void SCScreenSize(int option);
static boolean SCNetCheck(int option);
static void CrispyHires(int option);
static void CrispyToggleWidescreen(int option);
static void CrispySmoothing(int option);
static void CrispyUncapped(int option);
static void CrispyFpsLimit(int option);
static void CrispyVsync(int option);
static void CrispyBrightmaps(int option);
static void CrispySmoothLighting(int option);
static void CrispySoundMono(int option);
static void CrispySndChannels(int option);
static void CrispyPlayerCoords(int options);
static void CrispyFreelook(int option);
static void CrispyMouselook(int option);
static void CrispyBobfactor(int option);
static void CrispyCenterWeapon(int option);
static void CrispyDefaultskill(int option);
static void CrispyNextPage(int option);
static void CrispyPrevPage(int option);
static void SCNetCheck2(int option);
static void SCLoadGame(int option);
static void SCSaveGame(int option);
static void SCMessages(int option);
static void SCEndGame(int option);
static void SCInfo(int option);
static void DrawMainMenu(void);
static void DrawClassMenu(void);
static void DrawSkillMenu(void);
static void DrawOptionsMenu(void);
static void DrawOptions2Menu(void);
static void DrawFileSlots(Menu_t * menu);
static void DrawFilesMenu(void);
static void MN_DrawInfo(void);
static void DrawLoadMenu(void);
static void DrawSaveMenu(void);
static void DrawSlider(Menu_t * menu, int item, int width, int slot);
static void DrawMouseMenu(void);
static void DrawCrispnessMenu(void);
static void DrawCrispness1(void);
static void DrawCrispness2(void);
void MN_LoadSlotText(void);

// EXTERNAL DATA DECLARATIONS ----------------------------------------------


// PUBLIC DATA DEFINITIONS -------------------------------------------------

boolean MenuActive;
int InfoType;
int messageson = true;
boolean mn_SuicideConsole;

int savepage; // [crispy] support 8 pages of savegames

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static int FontABaseLump;
static int FontAYellowBaseLump;
static int FontBBaseLump;
static int MauloBaseLump;
static Menu_t *CurrentMenu;
static int CurrentItPos;
static int MenuPClass;
static int MenuTime;
static boolean soundchanged;

boolean askforquit;
static int typeofask;
static boolean FileMenuKeySteal;
static boolean slottextloaded;
static boolean joypadsave;
static char SlotText[6][SLOTTEXTLEN + 2];
static char oldSlotText[SLOTTEXTLEN + 2];
static int SlotStatus[6];
static int slotptr;
static int currentSlot;
static int quicksave;
static int quickload;

// [crispy] for entering numeric values
#define NUMERIC_ENTRY_NUMDIGITS 3
static boolean numeric_enter;
static int numeric_entry;
static char numeric_entry_str[NUMERIC_ENTRY_NUMDIGITS + 1];
static int numeric_entry_index;

static MenuItem_t MainItems[] = {
    {ITT_SETMENU, "NEW GAME", SCNetCheck2, 1, MENU_CLASS},
    {ITT_SETMENU, "OPTIONS", NULL, 0, MENU_OPTIONS},
    {ITT_SETMENU, "GAME FILES", NULL, 0, MENU_FILES},
    {ITT_EFUNC, "INFO", SCInfo, 0, MENU_NONE},
    {ITT_EFUNC, "QUIT GAME", SCQuitGame, 0, MENU_NONE}
};

static Menu_t MainMenu = {
    110, 56,
    DrawMainMenu,
    5, MainItems,
    0,
    MENU_NONE
};

static MenuItem_t ClassItems[] = {
    {ITT_EFUNC, "FIGHTER", SCClass, 0, MENU_NONE},
    {ITT_EFUNC, "CLERIC", SCClass, 1, MENU_NONE},
    {ITT_EFUNC, "MAGE", SCClass, 2, MENU_NONE}
};

static Menu_t ClassMenu = {
    66, 66,
    DrawClassMenu,
    3, ClassItems,
    0,
    MENU_MAIN
};

static MenuItem_t FilesItems[] = {
    {ITT_SETMENU, "LOAD GAME", SCNetCheck2, 2, MENU_LOAD},
    {ITT_SETMENU, "SAVE GAME", NULL, 0, MENU_SAVE}
};

static Menu_t FilesMenu = {
    110, 60,
    DrawFilesMenu,
    2, FilesItems,
    0,
    MENU_MAIN
};

static MenuItem_t LoadItems[] = {
    {ITT_EFUNC, NULL, SCLoadGame, 0, MENU_NONE},
    {ITT_EFUNC, NULL, SCLoadGame, 1, MENU_NONE},
    {ITT_EFUNC, NULL, SCLoadGame, 2, MENU_NONE},
    {ITT_EFUNC, NULL, SCLoadGame, 3, MENU_NONE},
    {ITT_EFUNC, NULL, SCLoadGame, 4, MENU_NONE},
    {ITT_EFUNC, NULL, SCLoadGame, 5, MENU_NONE}
};

static Menu_t LoadMenu = {
    70, 27-9, // [crispy] moved up, so two lines of save pages and file date will fit
    DrawLoadMenu,
    SAVES_PER_PAGE, LoadItems,
    0,
    MENU_FILES
};

static MenuItem_t SaveItems[] = {
    {ITT_EFUNC, NULL, SCSaveGame, 0, MENU_NONE},
    {ITT_EFUNC, NULL, SCSaveGame, 1, MENU_NONE},
    {ITT_EFUNC, NULL, SCSaveGame, 2, MENU_NONE},
    {ITT_EFUNC, NULL, SCSaveGame, 3, MENU_NONE},
    {ITT_EFUNC, NULL, SCSaveGame, 4, MENU_NONE},
    {ITT_EFUNC, NULL, SCSaveGame, 5, MENU_NONE}
};

static Menu_t SaveMenu = {
    70, 27-9, // [crispy] moved up, so two lines of save pages and file date will fit
    DrawSaveMenu,
    SAVES_PER_PAGE, SaveItems,
    0,
    MENU_FILES
};

static MenuItem_t SkillItems[] = {
    {ITT_EFUNC, NULL, SCSkill, sk_baby, MENU_NONE},
    {ITT_EFUNC, NULL, SCSkill, sk_easy, MENU_NONE},
    {ITT_EFUNC, NULL, SCSkill, sk_medium, MENU_NONE},
    {ITT_EFUNC, NULL, SCSkill, sk_hard, MENU_NONE},
    {ITT_EFUNC, NULL, SCSkill, sk_nightmare, MENU_NONE}
};

static Menu_t SkillMenu = {
    120, 44,
    DrawSkillMenu,
    5, SkillItems,
    2,
    MENU_CLASS
};

static MenuItem_t OptionsItems[] = {
    {ITT_EFUNC, "END GAME", SCEndGame, 0, MENU_NONE},
    {ITT_LRFUNC2, "MESSAGES : ", SCMessages, 0, MENU_NONE},
    {ITT_SETMENU, "MOUSE SENSITIVITY...", NULL, 0, MENU_MOUSE},
    {ITT_SETMENU, "MORE...", NULL, 0, MENU_OPTIONS2},
    {ITT_SETMENU, "CRISPNESS...", NULL, 0, MENU_CRISPNESS1}
};

static Menu_t OptionsMenu = {
    88, 30,
    DrawOptionsMenu,
    5, OptionsItems,
    0,
    MENU_MAIN
};

static MenuItem_t MouseItems[] = {
    {ITT_LRFUNC, "HORIZONTAL : TURN", SCMouseSensi, 0, MENU_NONE},
    {ITT_EMPTY, NULL, NULL, 0, MENU_NONE},
    {ITT_LRFUNC, "HORIZONTAL : STRAFE", SCMouseSensiX2, 0, MENU_NONE},
    {ITT_EMPTY, NULL, NULL, 0, MENU_NONE},
    {ITT_LRFUNC, "VERTICAL", SCMouseSensiY, 0, MENU_NONE},
    {ITT_EMPTY, NULL, NULL, 0, MENU_NONE},
    {ITT_LRFUNC2, "INVERT Y AXIS :", SCMouseInvertY, 0, MENU_NONE},
};

static Menu_t MouseMenu = {
    90, 15,
    DrawMouseMenu,
    7, MouseItems,
    0,
    MENU_OPTIONS
};
static MenuItem_t Options2Items[] = {
    {ITT_LRFUNC, "SCREEN SIZE", SCScreenSize, 0, MENU_NONE},
    {ITT_EMPTY, NULL, NULL, 0, MENU_NONE},
    {ITT_LRFUNC, "SFX VOLUME", SCSfxVolume, 0, MENU_NONE},
    {ITT_EMPTY, NULL, NULL, 0, MENU_NONE},
    {ITT_LRFUNC, "MUSIC VOLUME", SCMusicVolume, 0, MENU_NONE},
    {ITT_EMPTY, NULL, NULL, 0, MENU_NONE}
};

static Menu_t Options2Menu = {
    90, 20,
    DrawOptions2Menu,
    6, Options2Items,
    0,
    MENU_OPTIONS
};

static int crispnessmenupage;

#define NUM_CRISPNESS_MENUS 2

static MenuItem_t Crispness1Items[] = {
    {ITT_LRFUNC2, "HIGH RESOLUTION RENDERING:", CrispyHires, 0, MENU_NONE},
    {ITT_LRFUNC2, "ASPECT RATIO:", CrispyToggleWidescreen, 0, MENU_NONE},
    {ITT_LRFUNC2, "SMOOTH PIXEL SCALING:", CrispySmoothing, 0, MENU_NONE},
    {ITT_LRFUNC2, "UNCAPPED FRAMERATE:", CrispyUncapped, 0, MENU_NONE},
    {ITT_NUMFUNC, "FRAMERATE LIMIT:", CrispyFpsLimit, 0, MENU_NONE},
    {ITT_LRFUNC2, "ENABLE VSYNC:", CrispyVsync, 0, MENU_NONE},
    {ITT_EMPTY, NULL, NULL, 0, MENU_NONE},
    {ITT_EMPTY, NULL, NULL, 0, MENU_NONE},
    {ITT_LRFUNC2, "BRIGHTMAPS:", CrispyBrightmaps, 0, MENU_NONE},
    {ITT_LRFUNC2, "SMOOTH DIMINISHING LIGHTING:", CrispySmoothLighting, 0, MENU_NONE},
    {ITT_EMPTY, NULL, NULL, 0, MENU_NONE},
    {ITT_EMPTY, NULL, NULL, 0, MENU_NONE},
    {ITT_LRFUNC2, "MONO SFX:", CrispySoundMono, 0, MENU_NONE},
    {ITT_LRFUNC2, "SOUND CHANNELS:", CrispySndChannels, 0, MENU_NONE},
    {ITT_EMPTY, NULL, NULL, 0, MENU_NONE},
    {ITT_EFUNC, "NEXT PAGE", CrispyNextPage, 0, MENU_NONE},
};

static Menu_t Crispness1Menu = {
    68, 35,
    DrawCrispnessMenu,
    16, Crispness1Items,
    0,
    MENU_OPTIONS
};

static MenuItem_t Crispness2Items[] = {
    {ITT_LRFUNC2, "SHOW PLAYER COORDS:", CrispyPlayerCoords, 0, MENU_NONE},
    {ITT_EMPTY, NULL, NULL, 0, MENU_NONE},
    {ITT_EMPTY, NULL, NULL, 0, MENU_NONE},
    {ITT_LRFUNC2, "FREELOOK MODE:", CrispyFreelook, 0, MENU_NONE},
    {ITT_LRFUNC2, "PERMANENT MOUSELOOK:", CrispyMouselook, 0, MENU_NONE},
    {ITT_LRFUNC2, "PLAYER VIEW/WEAPON BOBBING:", CrispyBobfactor, 0, MENU_NONE},
    {ITT_LRFUNC2, "WEAPON ATTACK ALIGNMENT:", CrispyCenterWeapon, 0, MENU_NONE},
    {ITT_LRFUNC2, "DEFAULT DIFFICULTY:", CrispyDefaultskill, 0, MENU_NONE},
    {ITT_EMPTY, NULL, NULL, 0, MENU_NONE},
    {ITT_EFUNC, "PREV PAGE", CrispyPrevPage, 0, MENU_NONE},
};

static Menu_t Crispness2Menu = {
    68, 35,
    DrawCrispnessMenu,
    10, Crispness2Items,
    0,
    MENU_OPTIONS
};

static void (*CrispnessMenuDrawers[])(void) = {
    &DrawCrispness1,
    &DrawCrispness2,
};

static MenuType_t CrispnessMenus[] = {
    MENU_CRISPNESS1,
    MENU_CRISPNESS2,
};

static const multiitem_t multiitem_bobfactor[NUM_BOBFACTORS] =
{
    {BOBFACTOR_FULL, "FULL"},
    {BOBFACTOR_75, "75%"},
    {BOBFACTOR_OFF, "OFF"},
};

static const multiitem_t multiitem_brightmaps[NUM_BRIGHTMAPS] =
{
    {BRIGHTMAPS_OFF, "NONE"},
    {BRIGHTMAPS_TEXTURES, "WALLS"},
    {BRIGHTMAPS_SPRITES, "ITEMS"},
    {BRIGHTMAPS_BOTH, "BOTH"},
};

static const multiitem_t multiitem_centerweapon[NUM_CENTERWEAPON] =
{
    {CENTERWEAPON_OFF, "OFF"},
    {CENTERWEAPON_CENTER, "CENTERED"},
    {CENTERWEAPON_BOB, "BOBBING"},
};

static const multiitem_t multiitem_widescreen[NUM_RATIOS] =
{
    {RATIO_ORIG, "ORIGINAL"},
    {RATIO_MATCH_SCREEN, "MATCH SCREEN"},
    {RATIO_16_10, "16:10"},
    {RATIO_16_9, "16:9"},
    {RATIO_21_9, "21:9"},
};

static const multiitem_t multiitem_widgets[NUM_WIDGETS] =
{
    {WIDGETS_OFF, "NEVER"},
    {WIDGETS_AUTOMAP, "IN AUTOMAP"},
    {WIDGETS_ALWAYS, "ALWAYS"},
    {WIDGETS_STBAR, "STATUS BAR"},
};

static const multiitem_t multiitem_freelook_hh[NUM_FREELOOKS_HH] =
{
    {FREELOOK_HH_LOCK, "LOCK"},
    {FREELOOK_HH_SPRING, "SPRING"},
};

static const multiitem_t multiitem_difficulties[NUM_SKILLS] =
{
    {SKILL_HMP, "MEDIUM"},
    {SKILL_UV, "HARD"},
    {SKILL_NIGHTMARE, "VERY HARD"},
    {SKILL_ITYTD, "VERY EASY"},
    {SKILL_HNTR, "EASY"},
};

static const multiitem_t multiitem_sndchannels[3] =
{
    {8, "8"},
    {16, "16"},
    {32, "32"},
};

static Menu_t *Menus[] = {
    &MainMenu,
    &ClassMenu,
    &SkillMenu,
    &OptionsMenu,
    &Options2Menu,
    &FilesMenu,
    &LoadMenu,
    &SaveMenu,
    &MouseMenu,
    &Crispness1Menu,
    &Crispness2Menu,
};

// [crispy] intermediate gamma levels
static const char *GammaText[] = {
    TXT_GAMMA_LEVEL_050,
    TXT_GAMMA_LEVEL_055,
    TXT_GAMMA_LEVEL_060,
    TXT_GAMMA_LEVEL_065,
    TXT_GAMMA_LEVEL_070,
    TXT_GAMMA_LEVEL_075,
    TXT_GAMMA_LEVEL_080,
    TXT_GAMMA_LEVEL_085,
    TXT_GAMMA_LEVEL_090,
    TXT_GAMMA_LEVEL_OFF,
    TXT_GAMMA_LEVEL_05,
    TXT_GAMMA_LEVEL_1,
    TXT_GAMMA_LEVEL_15,
    TXT_GAMMA_LEVEL_2,
    TXT_GAMMA_LEVEL_25,
    TXT_GAMMA_LEVEL_3,
    TXT_GAMMA_LEVEL_35,
    TXT_GAMMA_LEVEL_4
};

// CODE --------------------------------------------------------------------

//---------------------------------------------------------------------------
//
// PROC MN_Init
//
//---------------------------------------------------------------------------

void MN_Init(void)
{
    InitFonts();
    MenuActive = false;
//      messageson = true;              // Set by defaults in .CFG
    MauloBaseLump = W_GetNumForName("FBULA0");  // ("M_SKL00");
    // [crispy] apply default difficulty
    SkillMenu.oldItPos = (crispy->defaultskill + SKILL_HMP) % NUM_SKILLS;
}

//---------------------------------------------------------------------------
//
// PROC InitFonts
//
//---------------------------------------------------------------------------

static void InitFonts(void)
{
    FontABaseLump = W_GetNumForName("FONTA_S") + 1;
    FontAYellowBaseLump = W_GetNumForName("FONTAY_S") + 1;
    FontBBaseLump = W_GetNumForName("FONTB_S") + 1;
}

// [crispy] Check if printable character is existing in FONTA(Y)/FONTB sets
// and do a replacement or case correction if needed.

enum {
    big_font, small_font
} fontsize_t;

static const char MN_CheckValidChar (char ascii_index, int have_cursor)
{
    if ((ascii_index > 'Z' + have_cursor && ascii_index < 'a') || ascii_index > 'z')
    {
        // Replace "\]^_`" and "{|}~" with spaces,
        // allow "[" (cursor symbol) only in small fonts.
        return ' ';
    }
    else if (ascii_index >= 'a' && ascii_index <= 'z')
    {
        // Force lowercase "a...z" characters to uppercase "A...Z".
        return ascii_index + 'A' - 'a';
    }
    else
    {
        // Valid char, do not modify it's ASCII index.
        return ascii_index;
    }
}

//---------------------------------------------------------------------------
//
// PROC MN_DrTextA
//
// Draw text using font A.
//
//---------------------------------------------------------------------------

void MN_DrTextA(const char *text, int x, int y)
{
    char c;
    patch_t *p;

    while ((c = *text++) != 0)
    {
        c = MN_CheckValidChar(c, small_font); // [crispy] check for valid characters

        if (c < 33)
        {
            x += 5;
        }
        else
        {
            p = W_CacheLumpNum(FontABaseLump + c - 33, PU_CACHE);
            V_DrawSBPatch(x, y, p);
            x += SHORT(p->width) - 1;
        }
    }
}

//==========================================================================
//
// MN_DrTextAYellow
//
//==========================================================================

void MN_DrTextAYellow(const char *text, int x, int y)
{
    char c;
    patch_t *p;

    while ((c = *text++) != 0)
    {
        c = MN_CheckValidChar(c, small_font); // [crispy] check for valid characters

        if (c < 33)
        {
            x += 5;
        }
        else
        {
            p = W_CacheLumpNum(FontAYellowBaseLump + c - 33, PU_CACHE);
            V_DrawSBPatch(x, y, p);
            x += SHORT(p->width) - 1;
        }
    }
}

//---------------------------------------------------------------------------
//
// FUNC MN_TextAWidth
//
// Returns the pixel width of a string using font A.
//
//---------------------------------------------------------------------------

int MN_TextAWidth(const char *text)
{
    char c;
    int width;
    patch_t *p;

    width = 0;
    while ((c = *text++) != 0)
    {
        c = MN_CheckValidChar(c, small_font); // [crispy] check for valid characters

        if (c < 33)
        {
            width += 5;
        }
        else
        {
            p = W_CacheLumpNum(FontABaseLump + c - 33, PU_CACHE);
            width += SHORT(p->width) - 1;
        }
    }
    return (width);
}

//---------------------------------------------------------------------------
//
// PROC MN_DrTextB
//
// Draw text using font B.
//
//---------------------------------------------------------------------------

void MN_DrTextB(const char *text, int x, int y)
{
    char c;
    patch_t *p;

    while ((c = *text++) != 0)
    {
        c = MN_CheckValidChar(c, big_font); // [crispy] check for valid characters

        if (c < 33)
        {
            x += 8;
        }
        else
        {
            p = W_CacheLumpNum(FontBBaseLump + c - 33, PU_CACHE);
            V_DrawPatch(x, y, p);
            x += SHORT(p->width) - 1;
        }
    }
}

//---------------------------------------------------------------------------
//
// FUNC MN_TextBWidth
//
// Returns the pixel width of a string using font B.
//
//---------------------------------------------------------------------------

int MN_TextBWidth(const char *text)
{
    char c;
    int width;
    patch_t *p;

    width = 0;
    while ((c = *text++) != 0)
    {
        c = MN_CheckValidChar(c, big_font); // [crispy] check for valid characters

        if (c < 33)
        {
            width += 5;
        }
        else
        {
            p = W_CacheLumpNum(FontBBaseLump + c - 33, PU_CACHE);
            width += SHORT(p->width) - 1;
        }
    }
    return (width);
}

//---------------------------------------------------------------------------
//
// PROC MN_Ticker
//
//---------------------------------------------------------------------------

void MN_Ticker(void)
{
    if (MenuActive == false)
    {
        return;
    }
    MenuTime++;
}

//---------------------------------------------------------------------------
//
// PROC MN_Drawer
//
//---------------------------------------------------------------------------

const char *QuitEndMsg[] = {
    "ARE YOU SURE YOU WANT TO QUIT?",
    "ARE YOU SURE YOU WANT TO END THE GAME?",
    "DO YOU WANT TO QUICKSAVE THE GAME NAMED",
    "DO YOU WANT TO QUICKLOAD THE GAME NAMED",
    "ARE YOU SURE YOU WANT TO SUICIDE?",
    "DO YOU WANT TO DELETE THE GAME NAMED",
};

void MN_Drawer(void)
{
    int i;
    int x;
    int y;
    MenuItem_t *item;
    const char *selName;

    if (MenuActive == false)
    {
        if (askforquit)
        {
            MN_DrTextA(QuitEndMsg[typeofask - 1], 160 -
                       MN_TextAWidth(QuitEndMsg[typeofask - 1]) / 2, 80);
            if (typeofask == 3)
            {
                MN_DrTextA(SlotText[quicksave - 1], 160 -
                           MN_TextAWidth(SlotText[quicksave - 1]) / 2, 90);
                MN_DrTextA("?", 160 +
                           MN_TextAWidth(SlotText[quicksave - 1]) / 2, 90);
            }
            if (typeofask == 4)
            {
                MN_DrTextA(SlotText[quickload - 1], 160 -
                           MN_TextAWidth(SlotText[quickload - 1]) / 2, 90);
                MN_DrTextA("?", 160 +
                           MN_TextAWidth(SlotText[quicksave - 1]) / 2, 90);
            }
            if (typeofask == 6)
            {
                MN_DrTextA(SlotText[CurrentItPos], 160 -
                           MN_TextAWidth(SlotText[CurrentItPos]) / 2, 90);
                MN_DrTextA("?", 160 +
                           MN_TextAWidth(SlotText[CurrentItPos]) / 2, 90);
            }
            UpdateState |= I_FULLSCRN;
        }
        return;
    }
    else
    {
        UpdateState |= I_FULLSCRN;
        if (InfoType)
        {
            MN_DrawInfo();
            return;
        }
        if (screenblocks < 10)
        {
            BorderNeedRefresh = true;
        }
        if (CurrentMenu->drawFunc != NULL)
        {
            CurrentMenu->drawFunc();
        }
        x = CurrentMenu->x;
        y = CurrentMenu->y;
        item = CurrentMenu->items;
        for (i = 0; i < CurrentMenu->itemCount; i++)
        {
            if (item->type != ITT_EMPTY && item->text)
            {
                if (CurrentMenu->drawFunc == DrawCrispnessMenu)
                {
                    // [crispy] use smaller font
                    MN_DrTextA(item->text, x, y);
                }
                else
                {
                    MN_DrTextB(item->text, x, y);
                }
            }
            if (CurrentMenu->drawFunc == DrawCrispnessMenu)
            {
                // [crispy] use 10px vertical spacing for small font
                y += ITEM_HEIGHT/2;
            }
            else
            {
                y += ITEM_HEIGHT;
            }
            item++;
        }
        if (CurrentMenu->drawFunc == DrawCrispnessMenu)
        {
            // [crispy] use small blue gem instead of big red arrow
            y = CurrentMenu->y + (CurrentItPos * ITEM_HEIGHT/2) + SELECTOR_YOFFSET;
            selName = MenuTime & 8 ? "INVGEMR1" : "INVGEMR2";
            V_DrawPatch(x + (SELECTOR_XOFFSET/2), y,
                        W_CacheLumpName(selName, PU_CACHE));
        }
        else
        {
            y = CurrentMenu->y + (CurrentItPos * ITEM_HEIGHT) + SELECTOR_YOFFSET;
            selName = MenuTime & 16 ? "M_SLCTR1" : "M_SLCTR2";
            V_DrawPatch(x + SELECTOR_XOFFSET, y,
                        W_CacheLumpName(selName, PU_CACHE));
        }
    }
}

//---------------------------------------------------------------------------
//
// PROC DrawMainMenu
//
//---------------------------------------------------------------------------

static void DrawMainMenu(void)
{
    int frame;

    frame = (MenuTime / 5) % 7;
    V_DrawPatch(88, 0, W_CacheLumpName("M_HTIC", PU_CACHE));
// Old Gold skull positions: (40, 10) and (232, 10)
    V_DrawPatch(37, 80, W_CacheLumpNum(MauloBaseLump + (frame + 2) % 7,
                                       PU_CACHE));
    V_DrawPatch(278, 80, W_CacheLumpNum(MauloBaseLump + frame, PU_CACHE));
}

//==========================================================================
//
// DrawClassMenu
//
//==========================================================================

static void DrawClassMenu(void)
{
    pclass_t class;
    static const char *boxLumpName[3] = {
        "m_fbox",
        "m_cbox",
        "m_mbox"
    };
    static const char *walkLumpName[3] = {
        "m_fwalk1",
        "m_cwalk1",
        "m_mwalk1"
    };

    MN_DrTextB("CHOOSE CLASS:", 34, 24);
    class = (pclass_t) CurrentMenu->items[CurrentItPos].option;
    V_DrawPatch(174, 8, W_CacheLumpName(boxLumpName[class], PU_CACHE));
    V_DrawPatch(174 + 24, 8 + 12,
                W_CacheLumpNum(W_GetNumForName(walkLumpName[class])
                               + ((MenuTime >> 3) & 3), PU_CACHE));
}

//---------------------------------------------------------------------------
//
// PROC DrawSkillMenu
//
//---------------------------------------------------------------------------

static void DrawSkillMenu(void)
{
    MN_DrTextB("CHOOSE SKILL LEVEL:", 74, 16);
}

//---------------------------------------------------------------------------
//
// PROC DrawFilesMenu
//
//---------------------------------------------------------------------------

static void DrawFilesMenu(void)
{
// clear out the quicksave/quickload stuff
    quicksave = 0;
    quickload = 0;
    P_ClearMessage(&players[consoleplayer]);
}

// [crispy] support additional pages of savegames
static void DrawSaveLoadBottomLine(const Menu_t *menu)
{
    char pagestr[16];
    static short width;
    const int y = menu->y + ITEM_HEIGHT * SAVES_PER_PAGE;

    if (!width)
    {
        const patch_t *const p = W_CacheLumpName("M_FSLOT", PU_CACHE);
        width = SHORT(p->width);
    }
    dp_translation = cr[CR_GOLD];
    if (savepage > 0)
        MN_DrTextA("- PGUP", menu->x + 1, y);
    if (savepage < SAVEPAGE_MAX)
        MN_DrTextA("PGDN +", menu->x + width - MN_TextAWidth("PGDN +"), y);

    M_snprintf(pagestr, sizeof(pagestr), "PAGE %d/%d", savepage + 1, SAVEPAGE_MAX + 1);
    MN_DrTextA(pagestr, ORIGWIDTH / 2 - MN_TextAWidth(pagestr) / 2, y);

    // [crispy] print "modified" (or created initially) time of savegame file
    if (SlotStatus[CurrentItPos] && !FileMenuKeySteal)
    {
        struct stat st;
        char filedate[32];
        char filename[100];

        M_snprintf(filename, sizeof(filename), "%shex%d.hxs", SavePath, CurrentItPos + (savepage * 10));
        if (M_stat(filename, &st) == 0)
        {
// [FG] suppress the most useless compiler warning ever
#if defined(__GNUC__)
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wformat-y2k"
#endif
        strftime(filedate, sizeof(filedate), "%x %X", localtime(&st.st_mtime));
#if defined(__GNUC__)
#  pragma GCC diagnostic pop
#endif
        MN_DrTextA(filedate, ORIGWIDTH / 2 - MN_TextAWidth(filedate) / 2, y + 10);
        }
    }

    dp_translation = NULL;
}

//---------------------------------------------------------------------------
//
// PROC DrawLoadMenu
//
//---------------------------------------------------------------------------

static void DrawLoadMenu(void)
{
    if (!slottextloaded)
    {
        MN_LoadSlotText();
    }
    DrawFileSlots(&LoadMenu);
    // [crispy] moved here, draw title on top of file slots
    MN_DrTextB("LOAD GAME", 160 - MN_TextBWidth("LOAD GAME") / 2, 1);
    DrawSaveLoadBottomLine(&LoadMenu);
}

//---------------------------------------------------------------------------
//
// PROC DrawSaveMenu
//
//---------------------------------------------------------------------------

static void DrawSaveMenu(void)
{
    if (!slottextloaded)
    {
        MN_LoadSlotText();
    }
    DrawFileSlots(&SaveMenu);
    // [crispy] moved here, draw title on top of file slots
    MN_DrTextB("SAVE GAME", 160 - MN_TextBWidth("SAVE GAME") / 2, 1);
    DrawSaveLoadBottomLine(&SaveMenu);
}

static boolean ReadDescriptionForSlot(int slot, char *description)
{
    FILE *fp;
    boolean found;
    char name[100];
    char versionText[HXS_VERSION_TEXT_LENGTH];

    slot += savepage * 10; // [crispy]

    M_snprintf(name, sizeof(name), "%shex%d.hxs", SavePath, slot);

    fp = M_fopen(name, "rb");

    if (fp == NULL)
    {
        return false;
    }

    found = fread(description, HXS_DESCRIPTION_LENGTH, 1, fp) == 1
         && fread(versionText, HXS_VERSION_TEXT_LENGTH, 1, fp) == 1;

    found = found && strcmp(versionText, HXS_VERSION_TEXT) == 0;

    fclose(fp);

    return found;
}

//===========================================================================
//
// MN_LoadSlotText
//
// For each slot, looks for save games and reads the description field.
//
//===========================================================================

void MN_LoadSlotText(void)
{
    char description[HXS_DESCRIPTION_LENGTH];
    int slot;

    for (slot = 0; slot < 6; slot++)
    {
        if (ReadDescriptionForSlot(slot, description))
        {
            memcpy(SlotText[slot], description, SLOTTEXTLEN);
            SlotStatus[slot] = 1;
        }
        else
        {
            memset(SlotText[slot], 0, SLOTTEXTLEN);
            SlotStatus[slot] = 0;
        }
    }
    slottextloaded = true;
}

//---------------------------------------------------------------------------
//
// PROC DrawFileSlots
//
//---------------------------------------------------------------------------

static void DrawFileSlots(Menu_t * menu)
{
    int i;
    int x;
    int y;

    x = menu->x;
    y = menu->y;
    for (i = 0; i < SAVES_PER_PAGE; i++)
    {
        V_DrawPatch(x, y, W_CacheLumpName("M_FSLOT", PU_CACHE));
        if (SlotStatus[i])
        {
            MN_DrTextA(SlotText[i], x + 5, y + 5);
        }
        y += ITEM_HEIGHT;
    }
}

//---------------------------------------------------------------------------
//
// PROC DrawOptionsMenu
//
//---------------------------------------------------------------------------

static void DrawOptionsMenu(void)
{
    if (messageson)
    {
        MN_DrTextB("ON", 196, 50);
    }
    else
    {
        MN_DrTextB("OFF", 196, 50);
    }
}

//---------------------------------------------------------------------------
//
// PROC DrawOptions2Menu
//
//---------------------------------------------------------------------------

static void DrawOptions2Menu(void)
{
    DrawSlider(&Options2Menu, 1, 14, screenblocks - 3);
    DrawSlider(&Options2Menu, 3, 16, snd_MaxVolume);
    DrawSlider(&Options2Menu, 5, 16, snd_MusicVolume);
}

//---------------------------------------------------------------------------
//
// PROC SCQuitGame
//
//---------------------------------------------------------------------------

static void SCQuitGame(int option)
{
    MenuActive = false;
    askforquit = true;
    typeofask = 1;              //quit game
    if (!netgame && !demoplayback)
    {
        paused = true;
    }
}

//---------------------------------------------------------------------------
//
// PROC SCEndGame
//
//---------------------------------------------------------------------------

static void SCEndGame(int option)
{
    if (demoplayback)
    {
        return;
    }
    if (SCNetCheck(3))
    {
        MenuActive = false;
        askforquit = true;
        typeofask = 2;          //endgame
        if (!netgame && !demoplayback)
        {
            paused = true;
        }
    }
}

//---------------------------------------------------------------------------
//
// PROC SCMessages
//
//---------------------------------------------------------------------------

static void SCMessages(int option)
{
    messageson ^= 1;
    if (messageson)
    {
        P_SetMessage(&players[consoleplayer], "MESSAGES ON", true);
    }
    else
    {
        P_SetMessage(&players[consoleplayer], "MESSAGES OFF", true);
    }
    S_StartSound(NULL, SFX_CHAT);
}

//===========================================================================
//
// SCNetCheck
//
//===========================================================================

static boolean SCNetCheck(int option)
{
    if (!netgame)
    {
        return true;
    }
    switch (option)
    {
        case 1:                // new game
            P_SetMessage(&players[consoleplayer],
                         "YOU CAN'T START A NEW GAME IN NETPLAY!", true);
            break;
        case 2:                // load game
            P_SetMessage(&players[consoleplayer],
                         "YOU CAN'T LOAD A GAME IN NETPLAY!", true);
            break;
        case 3:                // end game
            P_SetMessage(&players[consoleplayer],
                         "YOU CAN'T END A GAME IN NETPLAY!", true);
            break;
    }
    MenuActive = false;
    S_StartSound(NULL, SFX_CHAT);
    return false;
}

//===========================================================================
//
// SCNetCheck2
//
//===========================================================================

static void SCNetCheck2(int option)
{
    SCNetCheck(option);
    return;
}

//---------------------------------------------------------------------------
//
// PROC SCLoadGame
//
//---------------------------------------------------------------------------

static void SCLoadGame(int option)
{
    if (demoplayback)
    {
        // deactivate playback, return control to player
        demoextend = false;
    }
    if (!SlotStatus[option])
    {                           // Don't try to load from an empty slot
        return;
    }
    G_LoadGame(option);
    MN_DeactivateMenu();
    BorderNeedRefresh = true;
    if (quickload == -1)
    {
        quickload = option + 1;
        P_ClearMessage(&players[consoleplayer]);
    }
}

// [crispy]
static void SCDeleteGame(int option)
{
    if (!SlotStatus[option])
    {
        return;
    }

    SV_ClearSaveSlot(option + savepage * 10);
    CurrentMenu->oldItPos = CurrentItPos;
    MN_LoadSlotText();
    BorderNeedRefresh = true;
}

//
// Generate a default save slot name when the user saves to
// an empty slot via the joypad.
//
static void SetDefaultSaveName(int slot)
{
    // map from IWAD or PWAD?
    if (W_IsIWADLump(maplumpinfo) && strcmp(SavePath, ""))
    {
        M_snprintf(SlotText[slot], SLOTTEXTLEN,
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

        M_snprintf(SlotText[slot], SLOTTEXTLEN,
                   "%s (%s)", maplumpinfo->name,
                   wadname);
        free(wadname);
    }
    M_ForceUppercase(SlotText[slot]);
    joypadsave = false;
}

//---------------------------------------------------------------------------
//
// PROC SCSaveGame
//
//---------------------------------------------------------------------------

static const char *const class_str[NUMCLASSES] =
{
    "FIGHTR",
    "CLERIC",
    "MAGE",
    "PIG",
};

static void SCSaveGame(int option)
{
    char *ptr;

    // [crispy] check if saving is allowed
    if (!usergame)
    {
        return;
    }

    if (!FileMenuKeySteal)
    {
        int x, y;

        FileMenuKeySteal = true;
        // We need to activate the text input interface to type the save
        // game name:
        x = SaveMenu.x + 1;
        y = SaveMenu.y + 1 + option * ITEM_HEIGHT;
        I_StartTextInput(x, y, x + 190, y + ITEM_HEIGHT - 2);

        M_StringCopy(oldSlotText, SlotText[option], sizeof(oldSlotText));
        ptr = SlotText[option];
        // [crispy] generate a default save slot name when saving to an empty slot
        if (!strcmp(ptr, "") /* && joypadsave */ || (strlen(oldSlotText) >= 3 && !strncmp(oldSlotText, "HUB", 3)))
        {
            SetDefaultSaveName(option);
          M_snprintf(ptr, sizeof(oldSlotText), "HUB %d.%d, %s",
                     P_GetMapCluster(gamemap), gamemap,
                     class_str[PlayerClass[consoleplayer]]);
        }

        while (*ptr)
        {
            ptr++;
        }
        *ptr = '[';
        *(ptr + 1) = 0;
        SlotStatus[option]++;
        currentSlot = option;
        slotptr = ptr - SlotText[option];
        return;
    }
    else
    {
        G_SaveGame(option, SlotText[option]);
        FileMenuKeySteal = false;
        I_StopTextInput();
        MN_DeactivateMenu();
    }
    BorderNeedRefresh = true;
    if (quicksave == -1)
    {
        quicksave = option + 1;
        P_ClearMessage(&players[consoleplayer]);
    }
}

//==========================================================================
//
// SCClass
//
//==========================================================================

static void SCClass(int option)
{
    if (netgame)
    {
        P_SetMessage(&players[consoleplayer],
                     "YOU CAN'T START A NEW GAME FROM WITHIN A NETGAME!",
                     true);
        return;
    }
    MenuPClass = option;
    switch (MenuPClass)
    {
        case PCLASS_FIGHTER:
            SkillMenu.x = 120;
            SkillItems[0].text = "SQUIRE";
            SkillItems[1].text = "KNIGHT";
            SkillItems[2].text = "WARRIOR";
            SkillItems[3].text = "BERSERKER";
            SkillItems[4].text = "TITAN";
            break;
        case PCLASS_CLERIC:
            SkillMenu.x = 116;
            SkillItems[0].text = "ALTAR BOY";
            SkillItems[1].text = "ACOLYTE";
            SkillItems[2].text = "PRIEST";
            SkillItems[3].text = "CARDINAL";
            SkillItems[4].text = "POPE";
            break;
        case PCLASS_MAGE:
            SkillMenu.x = 112;
            SkillItems[0].text = "APPRENTICE";
            SkillItems[1].text = "ENCHANTER";
            SkillItems[2].text = "SORCERER";
            SkillItems[3].text = "WARLOCK";
            SkillItems[4].text = "ARCHIMAGE";
            break;
    }
    SetMenu(MENU_SKILL);
}

//---------------------------------------------------------------------------
//
// PROC SCSkill
//
//---------------------------------------------------------------------------

static void SCSkill(int option)
{
    if (demoplayback)
    {
        // deactivate playback, return control to player
        demoextend = false;
    }

    PlayerClass[consoleplayer] = MenuPClass;
    G_DeferredNewGame(option);
    SB_SetClassData();
    SB_state = -1;
    MN_DeactivateMenu();
}

//---------------------------------------------------------------------------
//
// PROC SCMouseSensi
//
//---------------------------------------------------------------------------

static void SCMouseSensi(int option)
{
    if (option == RIGHT_DIR)
    {
        if (mouseSensitivity < 255)
        {
            mouseSensitivity++;
        }
    }
    else if (mouseSensitivity)
    {
        mouseSensitivity--;
    }
}

//---------------------------------------------------------------------------
//
// PROC SCMouseSensiX2
//
//---------------------------------------------------------------------------

static void SCMouseSensiX2(int option)
{
    if (option == RIGHT_DIR)
    {
        if (mouseSensitivity_x2 < 255)
        {
            mouseSensitivity_x2++;
        }
    }
    else if (mouseSensitivity_x2)
    {
        mouseSensitivity_x2--;
    }
}

//---------------------------------------------------------------------------
//
// PROC SCMouseSensiY
//
//---------------------------------------------------------------------------

static void SCMouseSensiY(int option)
{
    if (option == RIGHT_DIR)
    {
        if (mouseSensitivity_y < 255)
        {
            mouseSensitivity_y++;
        }
    }
    else if (mouseSensitivity_y)
    {
        mouseSensitivity_y--;
    }
}

//---------------------------------------------------------------------------
//
// PROC SCMouseInvertY
//
//---------------------------------------------------------------------------

static void SCMouseInvertY(int option)
{
    mouse_y_invert = !mouse_y_invert;
}

//---------------------------------------------------------------------------
//
// PROC SCSfxVolume
//
//---------------------------------------------------------------------------

static void SCSfxVolume(int option)
{
    if (option == RIGHT_DIR)
    {
        if (snd_MaxVolume < 15)
        {
            snd_MaxVolume++;
        }
    }
    else if (snd_MaxVolume)
    {
        snd_MaxVolume--;
    }
    soundchanged = true;        // we'll set it when we leave the menu
}

//---------------------------------------------------------------------------
//
// PROC SCMusicVolume
//
//---------------------------------------------------------------------------

static void SCMusicVolume(int option)
{
    if (option == RIGHT_DIR)
    {
        if (snd_MusicVolume < 15)
        {
            snd_MusicVolume++;
        }
    }
    else if (snd_MusicVolume)
    {
        snd_MusicVolume--;
    }
    S_SetMusicVolume();
}

//---------------------------------------------------------------------------
//
// PROC SCScreenSize
//
//---------------------------------------------------------------------------

static void SCScreenSize(int option)
{
    if (option == RIGHT_DIR)
    {
        if (screenblocks < 16)
        {
            screenblocks++;
        }
    }
    else if (screenblocks > 3)
    {
        screenblocks--;
    }
    R_SetViewSize(BETWEEN(3, 11, screenblocks), detailLevel);
}

//---------------------------------------------------------------------------
//
// PROC SCInfo
//
//---------------------------------------------------------------------------

static void SCInfo(int option)
{
    InfoType = 1;
    S_StartSound(NULL, SFX_DOOR_LIGHT_CLOSE);
    if (!netgame && !demoplayback)
    {
        paused = true;
    }
}

//---------------------------------------------------------------------------
//
// Crispness menu: toggable features
//
//---------------------------------------------------------------------------

static void ChangeSettingEnum(int *setting, int option, int num_values)
{
    if (option == RIGHT_DIR)
    {
        *setting += 1;
    }
    else
    {
        *setting += num_values - 1;
    }

    *setting %= num_values;
}

static void CrispyHiresHook(void)
{
    crispy->hires = !crispy->hires;
    // [crispy] re-initialize framebuffers, textures and renderer
    I_ReInitGraphics(REINIT_FRAMEBUFFERS | REINIT_TEXTURES | REINIT_ASPECTRATIO);
    // [crispy] re-calculate framebuffer coordinates
    R_ExecuteSetViewSize();
    // [crispy] scale the sky for new resolution
    R_InitSkyMap();
    // [crispy] re-calculate automap coordinates
    AM_LevelInit(true);
    if (automapactive) {
        AM_initVariables();
    }
    // [crispy] refresh the status bar
    SB_state = -1;
}

static void CrispyHires(int option)
{
    crispy->post_rendering_hook = CrispyHiresHook;
}

static int hookoption;
static void CrispyToggleWidescreenHook (void)
{
    ChangeSettingEnum(&crispy->widescreen, hookoption, NUM_RATIOS);
    // [crispy] re-initialize framebuffers, textures and renderer
    I_ReInitGraphics(REINIT_FRAMEBUFFERS | REINIT_TEXTURES | REINIT_ASPECTRATIO);
    // [crispy] re-calculate framebuffer coordinates
    R_ExecuteSetViewSize();
    // [crispy] re-calculate automap coordinates
    AM_LevelInit(true);
    if (automapactive) {
        AM_initVariables();
    }
}
static void CrispyToggleWidescreen(int option)
{
    hookoption = option;
    crispy->post_rendering_hook = CrispyToggleWidescreenHook;
}

static void CrispySmoothing(int option)
{
    crispy->smoothscaling = !crispy->smoothscaling;
}

static void CrispyUncapped(int option)
{
    crispy->uncapped = !crispy->uncapped;
}

static void CrispyFpsLimit(int option)
{
    if (!crispy->uncapped)
    {
        return;
    }

    if (option == LEFT_DIR)
    {
        crispy->fpslimit--;
    }
    else if (option == RIGHT_DIR)
    {
        if (crispy->fpslimit < TICRATE)
        {
            crispy->fpslimit = TICRATE;
        }
        else
        {
            crispy->fpslimit++;
        }
    }
    else if (option == ENTER_NUMBER)
    {
        if (numeric_enter)
        {
            crispy->fpslimit = numeric_entry;
            numeric_enter = false;
            I_StopTextInput();
        }
        else
        {
            numeric_enter = true;
            I_StartTextInput(0, 0, 0, 0);
            return;
        }
    }

    if (crispy->fpslimit < TICRATE)
    {
        crispy->fpslimit = 0;
    }
    else if (crispy->fpslimit > CRISPY_FPSLIMIT_MAX)
    {
        crispy->fpslimit = CRISPY_FPSLIMIT_MAX;
    }
}

static void CrispyVsyncHook(void)
{
    crispy->vsync = !crispy->vsync;
    I_ToggleVsync();
}

static void CrispyVsync(int option)
{
    crispy->post_rendering_hook = CrispyVsyncHook;
}

static void CrispyBrightmaps(int option)
{
    ChangeSettingEnum(&crispy->brightmaps, option, NUM_BRIGHTMAPS);
}

static void CrispySmoothLightingHook (void)
{
    crispy->smoothlight = !crispy->smoothlight;
#ifdef CRISPY_TRUECOLOR
    // [crispy] re-calculate amount of colormaps and light tables
    R_InitTrueColormaps(LevelUseFullBright ? "COLORMAP" : "FOGMAP");
#endif
    // [crispy] re-calculate the zlight[][] array
    R_InitLightTables();
    // [crispy] re-calculate the scalelight[][] array
    R_ExecuteSetViewSize();
}

static void CrispySmoothLighting(int option)
{
    crispy->post_rendering_hook = CrispySmoothLightingHook;
}

static void CrispySoundMono(int option)
{
    crispy->soundmono = !crispy->soundmono;
}

static void CrispySndChannels(int option)
{
    S_UpdateSndChannels(option);
}

static void CrispyPlayerCoords(int option)
{
    // disable "always" and "status bar" setting
    ChangeSettingEnum(&crispy->playercoords, option, NUM_WIDGETS - 2);
}

static void CrispyFreelook(int option)
{
    ChangeSettingEnum(&crispy->freelook_hh, option, NUM_FREELOOKS_HH);
}

static void CrispyMouselook(int option)
{
    crispy->mouselook = !crispy->mouselook;
}

static void CrispyBobfactor(int option)
{
    ChangeSettingEnum(&crispy->bobfactor, option, NUM_BOBFACTORS);
}

static void CrispyCenterWeapon(int option)
{
    if (crispy->bobfactor == BOBFACTOR_OFF)
    {
        return;
    }

    ChangeSettingEnum(&crispy->centerweapon, option, NUM_CENTERWEAPON);
}

static void CrispyDefaultskill(int option)
{
    ChangeSettingEnum(&crispy->defaultskill, option, NUM_SKILLS);
    SkillMenu.oldItPos = (crispy->defaultskill + SKILL_HMP) % NUM_SKILLS;
}

static void CrispyNextPage(int option)
{
    crispnessmenupage++;
    crispnessmenupage %= NUM_CRISPNESS_MENUS;
}

static void CrispyPrevPage(int option)
{
    crispnessmenupage += NUM_CRISPNESS_MENUS - 1;
    crispnessmenupage %= NUM_CRISPNESS_MENUS;
}

static void CrispyReturnToMenu()
{
	Menu_t *cur = CurrentMenu;
	MN_ActivateMenu();
	CurrentMenu = cur;
	CurrentItPos = CurrentMenu->oldItPos;
}

//---------------------------------------------------------------------------
//
// FUNC MN_Responder
//
//---------------------------------------------------------------------------

boolean MN_Responder(event_t * event)
{
    int key;
    int charTyped;
    int i;
    MenuItem_t *item;
    char *textBuffer;
    int dir;

    // In testcontrols mode, none of the function keys should do anything
    // - the only key is escape to quit.

    if (testcontrols)
    {
        if (event->type == ev_quit
         || (event->type == ev_keydown
          && (event->data1 == key_menu_activate
           || event->data1 == key_menu_quit)))
        {
            I_Quit();
            return true;
        }

        return false;
    }

    // "close" button pressed on window?
    if (event->type == ev_quit)
    {
        // First click on close = bring up quit confirm message.
        // Second click = confirm quit.

        if (!MenuActive && askforquit && typeofask == 1)
        {
            G_CheckDemoStatus();
            I_Quit();
        }
        else
        {
            SCQuitGame(0);
            S_StartSound(NULL, SFX_CHAT);
        }

        return true;
    }

    charTyped = 0;
    key = -1;

    if (event->type == ev_joystick)
    {
        // Simulate key presses from joystick events to interact with the menu.

        if (MenuActive)
        {
            if (JOY_GET_DPAD(event->data6) != JOY_DIR_NONE)
            {
                dir = JOY_GET_DPAD(event->data6);
            }
            else if (JOY_GET_LSTICK(event->data6) != JOY_DIR_NONE)
            {
                dir = JOY_GET_LSTICK(event->data6);
            }
            else
            {
                dir = JOY_GET_RSTICK(event->data6);
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
#define JOY_BUTTON_PRESSED(x) (JOY_BUTTON_MAPPED(x) && (event->data1 & (1 << (x))) != 0)

            if (JOY_BUTTON_PRESSED(joybfire))
            {
                // Simulate pressing "Enter" when we are supplying a save slot name
                if (FileMenuKeySteal)
                {
                    key = KEY_ENTER;
                }
                else
                {
                    // if selecting a save slot via joypad, set a flag
                    if (CurrentMenu == &SaveMenu)
                    {
                        joypadsave = true;
                    }
                    key = key_menu_forward;
                }
                joywait = I_GetTime() + 5;
            }
            if (JOY_BUTTON_PRESSED(joybuse))
            {
                // If user was entering a save name, back out
                if (FileMenuKeySteal)
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
        else if (askforquit)
        {
            if (JOY_BUTTON_PRESSED(joybfire))
            {
                // Simulate a 'Y' keypress
                key = key_menu_confirm;
                joywait = I_GetTime() + 5;
            }
            if (JOY_BUTTON_PRESSED(joybuse))
            {
                // Simulate a 'N' keypress
                key = key_menu_abort;
                joywait = I_GetTime() + 5;
            }
        }
        if (JOY_BUTTON_PRESSED(joybmenu))
        {
            MN_ActivateMenu();
            joywait = I_GetTime() + 5;
            return true;
        }
    }
    else // [crispy] allow menu control with the mouse
    {
        static int mousewait = 0;
        static int mousey = 0;
        static int lasty = 0;

        if (event->type == ev_mouse && mousewait < I_GetTime())
        {
            // [crispy] novert disables up/down cursor movement with the mouse
            if (!novert)
            {
                mousey += event->data3;
            }

            if (mousey < lasty - 30)
            {
                key = key_menu_down;
                mousewait = I_GetTime() + 5;
                mousey = lasty -= 30;
            }
            else if (mousey > lasty + 30)
            {
                key = key_menu_up;
                mousewait = I_GetTime() + 5;
                mousey = lasty += 30;
            }

            if (event->data1 & 1)
            {
                key = key_menu_forward;
                mousewait = I_GetTime() + 5;
            }

            if (event->data1 & 2)
            {
                if (FileMenuKeySteal)
                {
                    key = KEY_ESCAPE;
                    FileMenuKeySteal = false;
                }
                else
                {
                    key = key_menu_back;
                }
                mousewait = I_GetTime() + 5;
            }

            // [crispy] scroll menus with mouse wheel
            if (event->data1 & (1 << 4))
            {
                key = key_menu_down;
                mousewait = I_GetTime() + 1;
            }
            else
            if (event->data1 & (1 << 3))
            {
                key = key_menu_up;
                mousewait = I_GetTime() + 1;
            }
        }
        else
        {
            if (event->type == ev_keydown)
            {
                key = event->data1;
                charTyped = event->data2;
            }
        }
    }

    if (event->type != ev_keydown && key == -1)
    {
        return false;
    }

    if (event->type == ev_keydown)
    {
        key = event->data1;
        charTyped = event->data2;
    }

    if (InfoType)
    {
        /* The 4-Level Demo Version also has 3 Info pages
        if (gamemode == shareware)
        {
            InfoType = (InfoType + 1) % 5;
        }
        else
        */
        {
            InfoType = (InfoType + 1) % 4;
        }
        if (key == KEY_ESCAPE)
        {
            InfoType = 0;
        }
        if (!InfoType)
        {
            if (!netgame && !demoplayback)
            {
                paused = false;
            }
            MN_DeactivateMenu();
            SB_state = -1;      //refresh the statbar
            BorderNeedRefresh = true;
        }
        S_StartSound(NULL, SFX_DOOR_LIGHT_CLOSE);
        return (true);          //make the info screen eat the keypress
    }

    if ((ravpic && key == KEY_F1) ||
        (key != 0 && key == key_menu_screenshot))
    {
        G_ScreenShot();
        return (true);
    }

    if (askforquit)
    {
        if (key == key_menu_confirm
        // [crispy] allow to confirm quit (1) and end game (2) by pressing Enter key
        || (key == key_menu_forward && (typeofask == 1 || typeofask == 2)))
        {
            switch (typeofask)
            {
                case 1:
                    G_CheckDemoStatus();
                    I_Quit();
                    return false;
                case 2:
                    P_ClearMessage(&players[consoleplayer]);
                    askforquit = false;
                    typeofask = 0;
                    paused = false;
#ifndef CRISPY_TRUECOLOR
                    I_SetPalette(W_CacheLumpName("PLAYPAL", PU_CACHE));
#else
                    I_SetPalette(0);
#endif
                    H2_StartTitle();    // go to intro/demo mode.
                    return false;
                case 3:
                    P_SetMessage(&players[consoleplayer],
                                 "QUICKSAVING....", false);
                    FileMenuKeySteal = true;
                    SCSaveGame(quicksave - 1);
                    BorderNeedRefresh = true;
                    break;
                case 4:
                    P_SetMessage(&players[consoleplayer],
                                 "QUICKLOADING....", false);
                    SCLoadGame(quickload - 1);
                    BorderNeedRefresh = true;
                    break;
                case 5:
                    BorderNeedRefresh = true;
                    mn_SuicideConsole = true;
                    break;
                case 6:
                    SCDeleteGame(CurrentItPos);
                    BorderNeedRefresh = true;
                    CrispyReturnToMenu();
                    break;
                default:
                    break;
            }

            askforquit = false;
            typeofask = 0;

            return true;
        }
        else if (key == key_menu_abort || key == KEY_ESCAPE)
        {
            players[consoleplayer].messageTics = 0;
            askforquit = false;
            typeofask = 0;
            paused = false;
            UpdateState |= I_FULLSCRN;
            BorderNeedRefresh = true;
            return true;
        }

        return false;           // don't let the keys filter thru
    }
    if (!MenuActive && !chatmodeon)
    {
        if (key == key_menu_decscreen)
        {
            if (automapactive)
            {               // Don't screen size in automap
                return (false);
            }
            SCScreenSize(LEFT_DIR);
            S_StartSound(NULL, SFX_PICKUP_KEY);
            BorderNeedRefresh = true;
            UpdateState |= I_FULLSCRN;
            return (true);
        }
        else if (key == key_menu_incscreen)
        {
            if (automapactive)
            {               // Don't screen size in automap
                return (false);
            }
            SCScreenSize(RIGHT_DIR);
            S_StartSound(NULL, SFX_PICKUP_KEY);
            BorderNeedRefresh = true;
            UpdateState |= I_FULLSCRN;
            return (true);
        }
        else if (key == key_menu_help)           // F1 (help screen)
        {
            SCInfo(0);      // start up info screens
            MenuActive = true;
            return (true);
        }
        else if (key == key_menu_save)           // F2 (save game)
        {
            if (gamestate == GS_LEVEL && !demoplayback)
            {
                MenuActive = true;
                FileMenuKeySteal = false;
                MenuTime = 0;
                CurrentMenu = &SaveMenu;
                CurrentItPos = CurrentMenu->oldItPos;
                if (!netgame && !demoplayback)
                {
                    paused = true;
                }
                S_StartSound(NULL, SFX_DOOR_LIGHT_CLOSE);
                slottextloaded = false;     //reload the slot text, when needed
            }
            return true;
        }
        else if (key == key_menu_load)           // F3 (load game)
        {
            if (SCNetCheck(2))
            {
                MenuActive = true;
                FileMenuKeySteal = false;
                MenuTime = 0;
                CurrentMenu = &LoadMenu;
                CurrentItPos = CurrentMenu->oldItPos;
                if (!netgame && !demoplayback)
                {
                    paused = true;
                }
                S_StartSound(NULL, SFX_DOOR_LIGHT_CLOSE);
                slottextloaded = false;     //reload the slot text, when needed
            }
            return true;
        }
        else if (key == key_menu_volume)         // F4 (volume)
        {
            MenuActive = true;
            FileMenuKeySteal = false;
            MenuTime = 0;
            CurrentMenu = &Options2Menu;
            CurrentItPos = CurrentMenu->oldItPos;
            if (!netgame && !demoplayback)
            {
                paused = true;
            }
            S_StartSound(NULL, SFX_DOOR_LIGHT_CLOSE);
            slottextloaded = false; //reload the slot text, when needed
            return true;
        }
        else if (key == key_menu_detail)         // F5 (suicide)
        {
            MenuActive = false;
            askforquit = true;
            typeofask = 5;  // suicide
            return true;
        }
        else if (key == key_menu_qsave)          // F6 (quicksave)
        {
            if (gamestate == GS_LEVEL && !demoplayback)
            {
                if (!quicksave || quicksave == -1)
                {
                    MenuActive = true;
                    FileMenuKeySteal = false;
                    MenuTime = 0;
                    CurrentMenu = &SaveMenu;
                    CurrentItPos = CurrentMenu->oldItPos;
                    if (!netgame && !demoplayback)
                    {
                        paused = true;
                    }
                    S_StartSound(NULL, SFX_DOOR_LIGHT_CLOSE);
                    slottextloaded = false; //reload the slot text
                    quicksave = -1;
                    P_SetMessage(&players[consoleplayer],
                                 "CHOOSE A QUICKSAVE SLOT", true);
                }
                else
                {
                    askforquit = true;
                    typeofask = 3;
                    if (!netgame && !demoplayback)
                    {
                        paused = true;
                    }
                    S_StartSound(NULL, SFX_CHAT);
                }
            }
            return true;
        }
        else if (key == key_menu_endgame)        // F7 (end game)
        {
            if (SCNetCheck(3))
            {
                if (gamestate == GS_LEVEL && !demoplayback)
                {
                    S_StartSound(NULL, SFX_CHAT);
                    SCEndGame(0);
                }
            }
            return true;
        }
        else if (key == key_menu_messages)       // F8 (toggle messages)
        {
            SCMessages(0);
            return true;
        }
        else if (key == key_menu_qload)          // F9 (quickload)
        {
            if (SCNetCheck(2))
            {
                if (!quickload || quickload == -1)
                {
                    MenuActive = true;
                    FileMenuKeySteal = false;
                    MenuTime = 0;
                    CurrentMenu = &LoadMenu;
                    CurrentItPos = CurrentMenu->oldItPos;
                    if (!netgame && !demoplayback)
                    {
                        paused = true;
                    }
                    S_StartSound(NULL, SFX_DOOR_LIGHT_CLOSE);
                    slottextloaded = false; // reload the slot text
                    quickload = -1;
                    P_SetMessage(&players[consoleplayer],
                                 "CHOOSE A QUICKLOAD SLOT", true);
                }
                else
                {
                    askforquit = true;
                    if (!netgame && !demoplayback)
                    {
                        paused = true;
                    }
                    typeofask = 4;
                    S_StartSound(NULL, SFX_CHAT);
                }
            }
            return true;
        }
        else if (key == key_menu_quit)           // F10 (quit)
        {
            // [crispy] allow to invoke quit in any game state
            // if (gamestate == GS_LEVEL || gamestate == GS_FINALE)
            {
                SCQuitGame(0);
                S_StartSound(NULL, SFX_CHAT);
            }
            return true;
        }
        else if (key == key_menu_gamma)          // F11 (gamma correction)
        {
            crispy->gamma++;
            if (crispy->gamma > 4+13) // [crispy] intermediate gamma levels
            {
                crispy->gamma = 0;
            }
            SB_PaletteFlash(true);  // force change
#ifdef CRISPY_TRUECOLOR
            R_InitTrueColormaps(LevelUseFullBright ? "COLORMAP" : "FOGMAP");
            BorderNeedRefresh = true;
            SB_state = -1;
#endif
            P_SetMessage(&players[consoleplayer], GammaText[crispy->gamma],
                         false);
            return true;
        }
        else if (key == KEY_F12)                 // F12 (???)
        {
            // F12 - reload current map (devmaps mode)

            if (netgame)
            {
                return false;
            }
            if (gamekeydown[key_speed])
            {               // Monsters ON
                nomonsters = false;
            }
            if (gamekeydown[key_strafe])
            {               // Monsters OFF
                nomonsters = true;
            }
            G_DeferedInitNew(gameskill, gameepisode, gamemap);
            P_SetMessage(&players[consoleplayer], TXT_CHEATWARP, false);
            return true;
        }
    }

    if (!MenuActive)
    {
        // [crispy] don't pop up the menu on other keys during a demo
        if (key == key_menu_activate || gamestate == GS_DEMOSCREEN || (demoplayback && !singledemo))
        {
            MN_ActivateMenu();
            return (true);
        }
        return (false);
    }
    if (!FileMenuKeySteal && !numeric_enter)
    {
        item = &CurrentMenu->items[CurrentItPos];

        if (key == key_menu_down)                // Next menu item
        {
            do
            {
                if (CurrentItPos + 1 > CurrentMenu->itemCount - 1)
                {
                    CurrentItPos = 0;
                }
                else
                {
                    CurrentItPos++;
                }
            }
            while (CurrentMenu->items[CurrentItPos].type == ITT_EMPTY);
            S_StartSound(NULL, SFX_FIGHTER_HAMMER_HITWALL);
            return (true);
        }
        else if (key == key_menu_up)             // Previous menu item
        {
            do
            {
                if (CurrentItPos == 0)
                {
                    CurrentItPos = CurrentMenu->itemCount - 1;
                }
                else
                {
                    CurrentItPos--;
                }
            }
            while (CurrentMenu->items[CurrentItPos].type == ITT_EMPTY);
            S_StartSound(NULL, SFX_FIGHTER_HAMMER_HITWALL);
            return (true);
        }
        else if (key == key_menu_left)           // Slider left
        {
            if ((item->type == ITT_LRFUNC || item->type == ITT_LRFUNC2 ||
                 item->type == ITT_NUMFUNC) && item->func != NULL)
            {
                item->func(LEFT_DIR);
                if (item->type == ITT_LRFUNC2)
                {
                    S_StartSound(NULL, SFX_DOOR_LIGHT_CLOSE);
                }
                else
                {
                    S_StartSound(NULL, SFX_PICKUP_KEY);
                }
            }
            return (true);
        }
        else if (key == key_menu_right)          // Slider right
        {
            if ((item->type == ITT_LRFUNC || item->type == ITT_LRFUNC2 ||
                 item->type == ITT_NUMFUNC) && item->func != NULL)
            {
                item->func(RIGHT_DIR);
                if (item->type == ITT_LRFUNC2)
                {
                    S_StartSound(NULL, SFX_DOOR_LIGHT_CLOSE);
                }
                else
                {
                    S_StartSound(NULL, SFX_PICKUP_KEY);
                }
            }
            return (true);
        }
        else if (key == key_menu_forward)        // Activate item (enter)
        {
            if (item->type == ITT_SETMENU)
            {
                if (item->func != NULL)
                {
                    item->func(item->option);
                }
                SetMenu(item->menu);
            }
            else if (item->func != NULL)
            {
                CurrentMenu->oldItPos = CurrentItPos;
                if (item->type == ITT_LRFUNC || item->type == ITT_LRFUNC2)
                {
                    item->func(RIGHT_DIR);
                }
                else if (item->type == ITT_EFUNC)
                {
                    item->func(item->option);
                }
                // [crispy] numeric entry
                else if (item->type == ITT_NUMFUNC)
                {
                    item->func(ENTER_NUMBER);
                    numeric_entry_index = 0;
                    numeric_entry_str[0] = '\0';
                }
            }
            S_StartSound(NULL, SFX_DOOR_LIGHT_CLOSE);
            return (true);
        }
        else if (key == key_menu_activate)
        {
            MN_DeactivateMenu();
            return (true);
        }
        else if (key == key_menu_back)
        {
            S_StartSound(NULL, SFX_PICKUP_KEY);

            if (CurrentMenu->prevMenu == MENU_NONE)
            {
                MN_DeactivateMenu();
            }
            else
            {
                SetMenu(CurrentMenu->prevMenu);
            }
            return (true);
        }
        // [crispy] delete a savegame
        else if (key == key_menu_del)
        {
            if (CurrentMenu == &LoadMenu || CurrentMenu == &SaveMenu)
            {
                if (SlotStatus[CurrentItPos])
                {
                    MenuActive = false;
                    askforquit = true;
                    if (!netgame && !demoplayback)
                    {
                        paused = true;
                    }
                    typeofask = 6;
                    S_StartSound(NULL, SFX_CHAT);
                }
            }
            return (true);
        }
        // [crispy] next/prev savegame page
        else if (key == KEY_PGUP)
        {
            if (CurrentMenu->drawFunc == DrawCrispnessMenu)
            {
                CrispyPrevPage(0);
                S_StartSound(NULL, SFX_FIGHTER_HAMMER_HITWALL);
            }
            else if (CurrentMenu == &LoadMenu || CurrentMenu == &SaveMenu)
            {
                if (savepage > 0)
                {
                    savepage--;
                    quicksave = -1;
                    MN_LoadSlotText();
                    S_StartSound(NULL, SFX_FIGHTER_HAMMER_HITWALL);
                }
                return true;
            }
        }
        else if (key == KEY_PGDN)
        {
            if (CurrentMenu->drawFunc == DrawCrispnessMenu)
            {
                CrispyNextPage(0);
                S_StartSound(NULL, SFX_FIGHTER_HAMMER_HITWALL);
            }
            else if (CurrentMenu == &LoadMenu || CurrentMenu == &SaveMenu)
            {
                if (savepage < SAVEPAGE_MAX)
                {
                    savepage++;
                    quicksave = -1;
                    MN_LoadSlotText();
                    S_StartSound(NULL, SFX_FIGHTER_HAMMER_HITWALL);
                }
                return true;
            }
        }
        else if (charTyped != 0)
        {
            // [crispy] allow multiple jumps over menu items with same first letters.
            for (i = CurrentItPos + 1; i < CurrentMenu->itemCount; i++)
            {
                if (CurrentMenu->items[i].text)
                {
                    if (toupper(charTyped)
                        == toupper(CurrentMenu->items[i].text[0]))
                    {
                        CurrentItPos = i;
                        return (true);
                    }
                }
            }
            for (i = 0; i <= CurrentItPos; i++)
            {
                if (CurrentMenu->items[i].text)
                {
                    if (toupper(charTyped)
                        == toupper(CurrentMenu->items[i].text[0]))
                    {
                        CurrentItPos = i;
                        return (true);
                    }
                }
            }
        }
        return (false);
    }
    else if (FileMenuKeySteal)
    {
        // Editing file names
        // When typing a savegame name, we use the fully shifted and
        // translated input value from event->data3.
        charTyped = event->data3;
        textBuffer = &SlotText[currentSlot][slotptr];
        if (key == KEY_BACKSPACE)
        {
            if (slotptr)
            {
                *textBuffer = 0;
                slotptr--;
                textBuffer = &SlotText[currentSlot][slotptr];
                *textBuffer = ASCII_CURSOR;
            }
            return (true);
        }
        if (key == KEY_ESCAPE)
        {
            M_StringCopy(SlotText[currentSlot], oldSlotText,
                         sizeof(SlotText[currentSlot]));
            SlotStatus[currentSlot]--;
            MN_DeactivateMenu();
            return (true);
        }
        if (key == KEY_ENTER)
        {
            SlotText[currentSlot][slotptr] = 0; // clear the cursor
            item = &CurrentMenu->items[CurrentItPos];
            CurrentMenu->oldItPos = CurrentItPos;
            if (item->type == ITT_EFUNC)
            {
                item->func(item->option);
                if (item->menu != MENU_NONE)
                {
                    SetMenu(item->menu);
                }
            }
            return (true);
        }
        if (slotptr < SLOTTEXTLEN && key != KEY_BACKSPACE)
        {
            if (isalpha(charTyped))
            {
                *textBuffer++ = toupper(charTyped);
                *textBuffer = ASCII_CURSOR;
                slotptr++;
                return (true);
            }
            if (isdigit(charTyped) || charTyped == ' '
             || charTyped == ',' || charTyped == '.' || charTyped == '-'
             || charTyped == '!')
            {
                *textBuffer++ = charTyped;
                *textBuffer = ASCII_CURSOR;
                slotptr++;
                return (true);
            }
        }
        return (true);
    }
    else if (numeric_enter) // [crispy] numeric entry
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
                    item = &CurrentMenu->items[CurrentItPos];
                    item->func(ENTER_NUMBER);
                }
                else
                {
                    numeric_enter = false;
                    I_StopTextInput();
                }
                break;
            default:
                charTyped = event->data3;

                if (charTyped >= '0' && charTyped <= '9' &&
                        numeric_entry_index < NUMERIC_ENTRY_NUMDIGITS)
                {
                    numeric_entry_str[numeric_entry_index++] = charTyped;
                    numeric_entry_str[numeric_entry_index] = '\0';
                }
                else
                {
                    break;
                }
        }
        return (true);
    }
    return (false);
}

//---------------------------------------------------------------------------
//
// PROC MN_ActivateMenu
//
//---------------------------------------------------------------------------

void MN_ActivateMenu(void)
{
    if (MenuActive)
    {
        return;
    }
    if (paused)
    {
        S_ResumeSound();
    }
    MenuActive = true;
    FileMenuKeySteal = false;
    MenuTime = 0;
    CurrentMenu = &MainMenu;
    CurrentItPos = CurrentMenu->oldItPos;
    if (!netgame && !demoplayback)
    {
        paused = true;
    }
    S_StartSound(NULL, SFX_PLATFORM_STOP);
    slottextloaded = false;     //reload the slot text, when needed
}

//---------------------------------------------------------------------------
//
// PROC MN_DeactivateMenu
//
//---------------------------------------------------------------------------

void MN_DeactivateMenu(void)
{
    if (CurrentMenu != NULL)
    {
        CurrentMenu->oldItPos = CurrentItPos;
    }
    MenuActive = false;
    if (FileMenuKeySteal)
    {
        I_StopTextInput();
    }
    if (!netgame)
    {
        paused = false;
    }
    S_StartSound(NULL, SFX_PLATFORM_STOP);
    P_ClearMessage(&players[consoleplayer]);
}

//---------------------------------------------------------------------------
//
// PROC MN_DrawInfo
//
//---------------------------------------------------------------------------

void MN_DrawInfo(void)
{
    lumpindex_t lumpindex; // [crispy]

#ifndef CRISPY_TRUECOLOR
    I_SetPalette(W_CacheLumpName("PLAYPAL", PU_CACHE));
#else
    I_SetPalette(0);
#endif

    // [crispy] Refactor to allow for use of V_DrawFullscreenRawOrPatch

    switch (InfoType)
    {
        case 1:
            lumpindex = W_GetNumForName("HELP1");
            break;

        case 2:
            lumpindex = W_GetNumForName("HELP2");
            break;

        case 3:
            lumpindex = W_GetNumForName("CREDIT");
            break;

        default:
            lumpindex = W_GetNumForName("TITLE");
            break;
    }

    V_DrawFullscreenRawOrPatch(lumpindex);

//      V_DrawPatch(0, 0, W_CacheLumpNum(W_GetNumForName("TITLE")+InfoType,
//              PU_CACHE));
}


//---------------------------------------------------------------------------
//
// PROC SetMenu
//
//---------------------------------------------------------------------------

static void SetMenu(MenuType_t menu)
{
    CurrentMenu->oldItPos = CurrentItPos;
    CurrentMenu = Menus[menu];
    CurrentItPos = CurrentMenu->oldItPos;
}

//---------------------------------------------------------------------------
//
// PROC DrawSlider
//
//---------------------------------------------------------------------------

static void DrawSlider(Menu_t * menu, int item, int width, int slot)
{
    int x;
    int y;
    int x2;
    int count;
    char num[4];

    x = menu->x + 24;
    y = menu->y + 2 + (item * ITEM_HEIGHT);
    V_DrawPatch(x - 32, y, W_CacheLumpName("M_SLDLT", PU_CACHE));
    for (x2 = x, count = width; count--; x2 += 8)
    {
        V_DrawPatch(x2, y, W_CacheLumpName(count & 1 ? "M_SLDMD1"
                                           : "M_SLDMD2", PU_CACHE));
    }
    V_DrawPatch(x2, y, W_CacheLumpName("M_SLDRT", PU_CACHE));

    // [crispy] print the value
    M_snprintf(num, 4, "%3d", slot);
    MN_DrTextA(num, x2 + 32, y + 3);

    // [crispy] do not crash anymore if the value is out of bounds
    if (slot >= width)
    {
        slot = width - 1;
    }

    V_DrawPatch(x + 4 + slot * 8, y + 7,
                W_CacheLumpName("M_SLDKB", PU_CACHE));
}

//---------------------------------------------------------------------------
//
// PROC DrawMouseMenu
//
//---------------------------------------------------------------------------

static void DrawMouseMenu(void)
{

    DrawSlider(&MouseMenu, 1, 16, mouseSensitivity);
    DrawSlider(&MouseMenu, 3, 16, mouseSensitivity_x2);
    DrawSlider(&MouseMenu, 5, 16, mouseSensitivity_y);

    // Invert mouse y
    MN_DrTextB(mouse_y_invert ? "ON" : "OFF", 226, 135);
}

//---------------------------------------------------------------------------
//
// PROC DrawCrispnessMenu
//
//---------------------------------------------------------------------------

static void M_DrawCrispnessBackground(void)
{
    byte *src;
    pixel_t *dest;

    src = W_CacheLumpName("F_022", PU_CACHE);
    dest = I_VideoBuffer;

    V_FillFlat(0, SCREENHEIGHT, 0, SCREENWIDTH, src, dest);

    SB_state = -1;
}

static void DrawCrispnessMenu(void)
{
    SetMenu(CrispnessMenus[crispnessmenupage]);

    // Background
    M_DrawCrispnessBackground();

    (*CrispnessMenuDrawers[crispnessmenupage])();

    dp_translation = NULL;
}

static void DrawCrispnessHeader(const char *item)
{
    dp_translation = cr[CR_GOLD];
    MN_DrTextA(item, 160 - MN_TextAWidth(item) / 2, 6);
}

static void DrawCrispnessSubheader(const char *name, int y)
{
    dp_translation = cr[CR_GOLD];
    MN_DrTextA(name, 63, y);
}

static void DrawCrispnessItem(boolean item, int x, int y)
{
    dp_translation = item ? cr[CR_GREEN] : cr[CR_DARK];
    MN_DrTextA(item ? "ON" : "OFF", x, y);
}

static void DrawCrispnessMultiItem(int item, int x, int y, const multiitem_t *multi,
        boolean cond)
{
    dp_translation = cond ? NULL :
                     item ? cr[CR_GREEN] : cr[CR_DARK];
    MN_DrTextA(cond ? multi[0].name : multi[item].name, x, y);
}

static void DrawCrispnessNumericItem(int item, int x, int y, const char *zero,
        boolean cond, const char *disabled)
{
    char number[NUMERIC_ENTRY_NUMDIGITS + 2];
    const int size = NUMERIC_ENTRY_NUMDIGITS + 2;

    if (numeric_enter)
    {
        M_snprintf(number, size, "%s%c", numeric_entry_str, ASCII_CURSOR);
    }
    else
    {
        M_snprintf(number, size, "%d", item);
    }

    dp_translation = cond ? NULL :
                    (item || numeric_enter) ? cr[CR_GREEN] : cr[CR_DARK];

    if (cond)
    {
        MN_DrTextA(disabled, x, y);
    }
    else if (item || numeric_enter)
    {
        MN_DrTextA(number, x, y);
    }
    else
    {
        MN_DrTextA(zero, x, y);
    }
}

static void DrawCrispness1(void)
{
    DrawCrispnessHeader("CRISPNESS 1/2");

    DrawCrispnessSubheader("RENDERING", 25);

    // Hires rendering
    DrawCrispnessItem(crispy->hires, 254, 35);

    // Widescreen
    DrawCrispnessMultiItem(crispy->widescreen, 164, 45, multiitem_widescreen, false);

    // Smooth pixel scaling
    DrawCrispnessItem(crispy->smoothscaling, 216, 55);

    // Uncapped framerate
    DrawCrispnessItem(crispy->uncapped, 217, 65);

    // Framerate limit
    DrawCrispnessNumericItem(crispy->fpslimit, 181, 75, "NONE", !crispy->uncapped, "35");

    // Vsync
    DrawCrispnessItem(crispy->vsync, 167, 85);

    DrawCrispnessSubheader("VISUAL", 105);

    // Brightmaps
    DrawCrispnessMultiItem(crispy->brightmaps, 150, 115, multiitem_brightmaps, false);

    // Smooth Diminishing Lighting
    DrawCrispnessItem(crispy->smoothlight, 257, 125);

    DrawCrispnessSubheader("AUDIBLE", 145);

    // Mono SFX
    DrawCrispnessItem(crispy->soundmono, 137, 155);

    // Sound Channels
    DrawCrispnessMultiItem(snd_Channels >> 4, 181, 165, multiitem_sndchannels, false);
}

static void DrawCrispness2(void)
{
    DrawCrispnessHeader("CRISPNESS 2/2");

    DrawCrispnessSubheader("NAVIGATIONAL", 25);

    // Player coordinates
    DrawCrispnessMultiItem(crispy->playercoords, 211, 35, multiitem_widgets, false);

    DrawCrispnessSubheader("TACTICAL", 55);

    // Freelook
    DrawCrispnessMultiItem(crispy->freelook_hh, 175, 65, multiitem_freelook_hh, false);

    // Mouselook
    DrawCrispnessItem(crispy->mouselook, 220, 75);

    // Bobfactor
    DrawCrispnessMultiItem(crispy->bobfactor, 265, 85, multiitem_bobfactor, false);

    // Weapon attack alignment
    DrawCrispnessMultiItem(crispy->centerweapon, 245, 95, multiitem_centerweapon,
            crispy->bobfactor == BOBFACTOR_OFF);

    // Default difficulty
    DrawCrispnessMultiItem(crispy->defaultskill, 200, 105, multiitem_difficulties, false);
}
