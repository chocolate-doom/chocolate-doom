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
// DESCRIPTION:  Heads-up displays
//


#include <ctype.h>

#include "doomdef.h"
#include "doomkeys.h"

#include "z_zone.h"

#include "deh_main.h"
#include "i_input.h"
#include "i_swap.h"
#include "i_video.h"

#include "hu_stuff.h"
#include "hu_lib.h"
#include "m_controls.h"
#include "m_misc.h"
#include "w_wad.h"

#include "s_sound.h"

#include "doomstat.h"

// Data.
#include "dstrings.h"
#include "sounds.h"

//
// Locally used constants, shortcuts.
//
#define HU_TITLE        (mapnames[gamemap-1])
#define HU_TITLEHEIGHT  1
#define HU_TITLEX       0

// haleyjd 09/01/10: [STRIFE] 167 -> 160 to move up level name
#define HU_TITLEY       (160 - SHORT(hu_font[0]->height))

#define HU_INPUTTOGGLE  't'
#define HU_INPUTX       HU_MSGX
#define HU_INPUTY       (HU_MSGY + HU_MSGHEIGHT*(SHORT(hu_font[0]->height) +1))
#define HU_INPUTWIDTH   64
#define HU_INPUTHEIGHT  1

char *chat_macros[10] =
{
    HUSTR_CHATMACRO0,
    HUSTR_CHATMACRO1,
    HUSTR_CHATMACRO2,
    HUSTR_CHATMACRO3,
    HUSTR_CHATMACRO4,
    HUSTR_CHATMACRO5,
    HUSTR_CHATMACRO6,
    HUSTR_CHATMACRO7,
    HUSTR_CHATMACRO8,
    HUSTR_CHATMACRO9
};

// villsa [STRIFE]
char player_names[8][16] =
{
    "1: ",
    "2: ",
    "3: ",
    "4: ",
    "5: ",
    "6: ",
    "7: ",
    "8: "
};

char                    chat_char; // remove later.
static player_t*        plr;
patch_t*                hu_font[HU_FONTSIZE];
patch_t*                yfont[HU_FONTSIZE];   // haleyjd 09/18/10: [STRIFE]
static hu_textline_t    w_title;
boolean                 chat_on;
static hu_itext_t       w_chat;
static boolean          always_off = false;
static char             chat_dest[MAXPLAYERS];
static hu_itext_t       w_inputbuffer[MAXPLAYERS];

static boolean          message_on;
boolean                 message_dontfuckwithme;
static boolean          message_nottobefuckedwith;

static hu_stext_t       w_message;
static int              message_counter;

//extern int              showMessages; [STRIFE] no such variable

static boolean          headsupactive = false;

// haleyjd 20130915 [STRIFE]: need nickname
extern char *nickname;

// haleyjd 20130915 [STRIFE]: true if setting nickname
static boolean hu_setting_name = false;

//
// Builtin map names.
// The actual names can be found in DStrings.h.
//

// haleyjd 08/31/10: [STRIFE] Changed for Strife level names.
// List of names for levels.

char *mapnames[] =
{
    // Strife map names

    // First "episode" - Quest to destroy the Order's Castle
    HUSTR_1,
    HUSTR_2,
    HUSTR_3,
    HUSTR_4,
    HUSTR_5,
    HUSTR_6,
    HUSTR_7,
    HUSTR_8,
    HUSTR_9,

    // Second "episode" - Kill the Bishop and Make a Choice
    HUSTR_10,
    HUSTR_11,
    HUSTR_12,
    HUSTR_13,
    HUSTR_14,
    HUSTR_15,
    HUSTR_16,
    HUSTR_17,
    HUSTR_18,
    HUSTR_19,

    // Third "episode" - Shut down Factory, kill Loremaster and Entity
    HUSTR_20,
    HUSTR_21,
    HUSTR_22,
    HUSTR_23,
    HUSTR_24,
    HUSTR_25,
    HUSTR_26,
    HUSTR_27,
    HUSTR_28,
    HUSTR_29,

    // "Secret" levels - Abandoned Base and Training Facility
    HUSTR_30,
    HUSTR_31,

    // Demo version maps
    HUSTR_32,
    HUSTR_33,
    HUSTR_34
};

//
// HU_Init
//
// haleyjd 09/18/10: [STRIFE]
// * Modified to load yfont along with hu_font.
//
void HU_Init(void)
{
    int		i;
    int		j;
    char	buffer[9];

    // load the heads-up font
    j = HU_FONTSTART;
    for (i=0;i<HU_FONTSIZE;i++)
    {
        DEH_snprintf(buffer, 9, "STCFN%.3d", j++);
        hu_font[i] = (patch_t *) W_CacheLumpName(buffer, PU_STATIC);

        // haleyjd 09/18/10: load yfont as well; and yes, this is exactly
        // how Rogue did it :P
        buffer[2] = 'B';
        yfont[i] = (patch_t *) W_CacheLumpName(buffer, PU_STATIC);
    }
}

//
// HU_Stop
//
// [STRIFE] Verified unmodified.
//
void HU_Stop(void)
{
    headsupactive = false;
}

//
// HU_Start
//
// haleyjd 09/18/10: [STRIFE] Added a hack for nickname at the end.
//
void HU_Start(void)
{
    int         i;
    char*       s;

    // haleyjd 20120211: [STRIFE] not called here.
    //if (headsupactive)
    //    HU_Stop();
    
    // haleyjd 20120211: [STRIFE] moved up
    // create the map title widget
    HUlib_initTextLine(&w_title,
                       HU_TITLEX, HU_TITLEY,
                       hu_font,
                       HU_FONTSTART);

    // haleyjd 08/31/10: [STRIFE] Get proper map name.
    s = HU_TITLE;

    // [STRIFE] Removed Chex Quest stuff.

    // dehacked substitution to get modified level name
    s = DEH_String(s);
    
    while (*s)
        HUlib_addCharToTextLine(&w_title, *(s++));

    // haleyjd 20120211: [STRIFE] check for headsupactive
    if(!headsupactive)
    {
        plr = &players[consoleplayer];
        message_on = false;
        message_dontfuckwithme = false;
        message_nottobefuckedwith = false;
        chat_on = false;

        // create the message widget
        HUlib_initSText(&w_message,
                        HU_MSGX, HU_MSGY, HU_MSGHEIGHT,
                        hu_font,
                        HU_FONTSTART, &message_on);

        // create the chat widget
        HUlib_initIText(&w_chat,
                        HU_INPUTX, HU_INPUTY,
                        hu_font,
                        HU_FONTSTART, &chat_on);

        // create the inputbuffer widgets
        for (i=0 ; i<MAXPLAYERS ; i++)
            HUlib_initIText(&w_inputbuffer[i], 0, 0, 0, 0, &always_off);

        headsupactive = true;

        // haleyjd 09/18/10: [STRIFE] nickname weirdness. 
        if(nickname != player_names[consoleplayer])
        {
            if(nickname != NULL && *nickname)
            {
                DEH_printf("have one\n");
                nickname = player_names[consoleplayer];
            }
        }
    }
}

//
// HU_Drawer
//
// [STRIFE] Verified unmodified.
//
void HU_Drawer(void)
{
    HUlib_drawSText(&w_message);
    HUlib_drawIText(&w_chat);
    if (automapactive)
        HUlib_drawTextLine(&w_title, false);
}

//
// HU_Erase
//
// [STRIFE] Verified unmodified.
//
void HU_Erase(void)
{
    HUlib_eraseSText(&w_message);
    HUlib_eraseIText(&w_chat);
    HUlib_eraseTextLine(&w_title);
}

//
// HU_addMessage
//
// haleyjd 09/18/10: [STRIFE] New function
// See if you can tell whether or not I had trouble with this :P
// Looks to be extremely buggy, hackish, and error-prone.
//
// <Markov> This is definitely not the best that Rogue had to offer. Markov.
//
//  Fastcall Registers:   edx          ebx
//      Temp Registers:   esi          edi
void HU_addMessage(char *prefix, char *message)
{
    char  c;         // eax
    int   width = 0; // edx
    char *rover1;    // ebx (in first loop)
    char *rover2;    // ecx (in second loop)
    char *bufptr;    // ebx (in second loop)
    char buffer[HU_MAXLINELENGTH+2];  // esp+52h

    // Loop 1: Total up width of prefix.
    rover1 = prefix;
    if(rover1)
    {
        while((c = *rover1))
        {
            c = toupper(c) - HU_FONTSTART;
            ++rover1;

            if(c < 0 || c >= HU_FONTSIZE)
                width += 4;
            else
                width += SHORT(hu_font[(int) c]->width);
        }
    }

    // Loop 2: Copy as much of message into buffer as will fit on screen
    bufptr = buffer;
    rover2 = message;
    while((c = *rover2))
    {
        if((c == ' ' || c == '-') && width > 285)
            break;

        *bufptr = c;
        ++bufptr;       // BUG: No check for overflow.
        ++rover2;
        c = toupper(c);

        if(c == ' ' || c < '!' || c >= '_')
            width += 4;
        else
        {
            c -= HU_FONTSTART;
            width += SHORT(hu_font[(int) c]->width);
        }
    }

    // Too big to fit?
    // BUG: doesn't consider by how much it's over.
    if(width > 320) 
    {
        // backup a char... hell if I know why.
        --bufptr;
        --rover2;
    }

    // rover2 is not at the end?
    if((c = *rover2))
    {
        // if not ON a space...
        if(c != ' ')
        {
            // back up both pointers til one is found.
            // BUG: no check against LHS of buffer. Hurr!
            while(*bufptr != ' ')
            {
                --bufptr;
                --rover2;
            }
        }
    }

    *bufptr = '\0';

    // Add two message lines.
    HUlib_addMessageToSText(&w_message, prefix, buffer);
    HUlib_addMessageToSText(&w_message, NULL,   rover2);
}

//
// HU_Ticker
//
// haleyjd 09/18/10: [STRIFE] Changes to split up message into two lines,
// and support for player names (STRIFE-TODO: unfinished!)
//
void HU_Ticker(void)
{
    int i, rc;
    char c;
    //char *prefix;  STRIFE-TODO

    // tick down message counter if message is up
    if (message_counter && !--message_counter)
    {
        message_on = false;
        message_nottobefuckedwith = false;
    }

    // haleyjd 20110219: [STRIFE] this condition was removed
    //if (showMessages || message_dontfuckwithme)
    //{
        // display message if necessary
        if ((plr->message && !message_nottobefuckedwith)
            || (plr->message && message_dontfuckwithme))
        {
            //HUlib_addMessageToSText(&w_message, 0, plr->message);
            HU_addMessage(NULL, plr->message); // haleyjd [STRIFE]
            plr->message = 0;
            message_on = true;
            message_counter = HU_MSGTIMEOUT;
            message_nottobefuckedwith = message_dontfuckwithme;
            message_dontfuckwithme = 0;
        }
    //} // else message_on = false;

    // check for incoming chat characters
    if (netgame)
    {
        for (i=0 ; i<MAXPLAYERS; i++)
        {
            if (!playeringame[i])
                continue;
            if (i != consoleplayer
                && (c = players[i].cmd.chatchar))
            {
                if (c <= HU_CHANGENAME) // [STRIFE]: allow HU_CHANGENAME here
                    chat_dest[i] = c;
                else
                {
                    rc = HUlib_keyInIText(&w_inputbuffer[i], c);
                    if (rc && c == KEY_ENTER)
                    {
                        if (w_inputbuffer[i].l.len
                            && (chat_dest[i] == consoleplayer+1
                             || chat_dest[i] == HU_BROADCAST))
                        {
                            HU_addMessage(player_names[i],
                                          w_inputbuffer[i].l.l);

                            message_nottobefuckedwith = true;
                            message_on = true;
                            message_counter = HU_MSGTIMEOUT;
                            S_StartSound(0, sfx_radio);
                        }
                        else if(chat_dest[i] == HU_CHANGENAME)
                        {
                            // haleyjd 20130915 [STRIFE]: set player name
                            DEH_snprintf(player_names[i], sizeof(player_names[i]),
                                         "%.13s: ", w_inputbuffer[i].l.l);
                        }
                        HUlib_resetIText(&w_inputbuffer[i]);
                    }
                }
                players[i].cmd.chatchar = 0;
            }
        }
    }
}

#define QUEUESIZE		128

static char	chatchars[QUEUESIZE];
static int	head = 0;
static int	tail = 0;

//
// HU_queueChatChar
//
// haleyjd 09/18/10: [STRIFE]
// * No message is given if a chat queue overflow occurs.
//
void HU_queueChatChar(char c)
{
    chatchars[head] = c;
    if (((head + 1) & (QUEUESIZE-1)) != tail)
    {
        head = (head + 1) & (QUEUESIZE-1);
    }
}

//
// HU_dequeueChatChar
//
// [STRIFE] Verified unmodified.
//
char HU_dequeueChatChar(void)
{
    char c;

    if (head != tail)
    {
        c = chatchars[tail];
        tail = (tail + 1) & (QUEUESIZE-1);
    }
    else
    {
        c = 0;
    }

    return c;
}

// fraggle 01/05/15: New functions to support the Chocolate input interface.
static void StartChatInput(void)
{
    chat_on = true;
    I_StartTextInput(HU_INPUTX, HU_INPUTY, SCREENWIDTH, HU_INPUTY + 8);
}

static void StopChatInput(void)
{
    chat_on = false;
    I_StopTextInput();
}

//
// HU_Responder
//
// haleyjd 09/18/10: [STRIFE]
// * Mostly unmodified, except:
//   - The default value of key_message_refresh is changed. That is handled
//     elsewhere in Choco, however.
//   - There is support for setting the player name through the chat
//     mechanism.
//
boolean HU_Responder(event_t *ev)
{
    static char         lastmessage[HU_MAXLINELENGTH+1];
    char*               macromessage;
    boolean             eatkey = false;
    static boolean      altdown = false;
    unsigned char       c;
    int                 i;
    int                 numplayers;
    
    static int          num_nobrainers = 0;

    numplayers = 0;
    for (i=0 ; i<MAXPLAYERS ; i++)
        numplayers += playeringame[i];

    if (ev->data1 == KEY_RSHIFT)
    {
        return false;
    }
    else if (ev->data1 == KEY_RALT || ev->data1 == KEY_LALT)
    {
        altdown = ev->type == ev_keydown;
        return false;
    }

    if (ev->type != ev_keydown)
        return false;

    if (!chat_on)
    {
        if (ev->data1 == key_message_refresh)
        {
            message_on = true;
            message_counter = HU_MSGTIMEOUT;
            eatkey = true;
        }
        else if (netgame && ev->data2 == key_multi_msg)
        {
            StartChatInput();
            eatkey = true;
            HUlib_resetIText(&w_chat);
            HU_queueChatChar(HU_BROADCAST);
        }
        // [STRIFE]: You cannot go straight to chatting with a particular
        // player from here... you must press 't' first. See below.
    }
    else
    {
        c = ev->data3;
        // send a macro
        if (altdown)
        {
            c = c - '0';
            if (c > 9)
                return false;
            // fprintf(stderr, "got here\n");
            macromessage = chat_macros[c];

            // kill last message with a '\n'
            HU_queueChatChar(KEY_ENTER); // DEBUG!!!

            // send the macro message
            while (*macromessage)
                HU_queueChatChar(*macromessage++);
            HU_queueChatChar(KEY_ENTER);

            // leave chat mode and notify that it was sent
            StopChatInput();
            M_StringCopy(lastmessage, chat_macros[c], sizeof(lastmessage));
            plr->message = lastmessage;
            eatkey = true;
        }
        else
        {
            if(w_chat.l.len) // [STRIFE]: past first char of chat?
            {
                eatkey = HUlib_keyInIText(&w_chat, c);
                if (eatkey)
                    HU_queueChatChar(c);
            }
            else
            {
                // [STRIFE]: check for player-specific message;
                // slightly different than vanilla, to allow keys to be customized
                for(i = 0; i < MAXPLAYERS; i++)
                {
                    if (ev->data1 == key_multi_msgplayer[i])
                        break;
                }
                if(i < MAXPLAYERS)
                {
                    // talking to self?
                    if(i == consoleplayer)
                    {
                        num_nobrainers++;
                        if (num_nobrainers < 3)
                            plr->message = DEH_String(HUSTR_TALKTOSELF1);
                        else if (num_nobrainers < 6)
                            plr->message = DEH_String(HUSTR_TALKTOSELF2);
                        else if (num_nobrainers < 9)
                            plr->message = DEH_String(HUSTR_TALKTOSELF3);
                        else if (num_nobrainers < 32)
                            plr->message = DEH_String(HUSTR_TALKTOSELF4);
                        else
                            plr->message = DEH_String(HUSTR_TALKTOSELF5);
                    }
                    else
                    {
                        eatkey = true;
                        HU_queueChatChar(i+1);
                        DEH_snprintf(lastmessage, sizeof(lastmessage),
                            "Talking to: %c", '1' + i);
                        plr->message = lastmessage;
                    }
                }
                else if(c == '$') // [STRIFE]: name changing
                {
                    eatkey = true;
                    HU_queueChatChar(HU_CHANGENAME);
                    M_StringCopy(lastmessage, DEH_String("Changing Name:"),
                                 sizeof(lastmessage));
                    plr->message = lastmessage;
                    hu_setting_name = true;
                }
                else
                {
                    eatkey = HUlib_keyInIText(&w_chat, c);
                    if (eatkey)
                        HU_queueChatChar(c);
                }
            }

            if (c == KEY_ENTER)
            {
                StopChatInput();
                if (w_chat.l.len)
                {
                    // [STRIFE]: name setting
                    if(hu_setting_name)
                    {
                        DEH_snprintf(lastmessage, sizeof(lastmessage),
                            "%s now %.13s", player_names[consoleplayer],
                            w_chat.l.l);
                        // haleyjd 20141024: missing name set for local client
                        DEH_snprintf(player_names[consoleplayer],
                            sizeof(player_names[consoleplayer]),
                            "%.13s: ", w_chat.l.l);
                        hu_setting_name = false;
                    }
                    else
                    {
                        M_StringCopy(lastmessage, w_chat.l.l,
                                     sizeof(lastmessage));
                    }
                    plr->message = lastmessage;
                }
            }
            else if (c == KEY_ESCAPE)
            {
                StopChatInput();
            }
        }
    }

    return eatkey;
}
