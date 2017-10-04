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
//
// Chat mode
//

#include <string.h>
#include <ctype.h>

#include "doomdef.h"
#include "doomkeys.h"

#include "deh_str.h"
#include "i_input.h"
#include "m_controls.h"
#include "m_misc.h"
#include "p_local.h"
#include "s_sound.h"
#include "v_video.h"

#define QUEUESIZE		128
#define MESSAGESIZE	128
#define MESSAGELEN 	265

#define CT_PLR_GREEN		1
#define CT_PLR_YELLOW	2
#define CT_PLR_RED		3
#define CT_PLR_BLUE		4
#define CT_PLR_ALL		5

#define CT_ESCAPE 6

// Public data


boolean chatmodeon;

// Private data

void CT_queueChatChar(char ch);
void CT_ClearChatMessage(int player);
void CT_AddChar(int player, char c);
void CT_BackSpace(int player);

int head;
int tail;
byte ChatQueue[QUEUESIZE];
int chat_dest[MAXPLAYERS];
char chat_msg[MAXPLAYERS][MESSAGESIZE];
char plr_lastmsg[MAXPLAYERS][MESSAGESIZE + 9];  // add in the length of the pre-string
int msgptr[MAXPLAYERS];
int msglen[MAXPLAYERS];

boolean cheated;

static int FontABaseLump;

char *CT_FromPlrText[MAXPLAYERS] = {
    "GREEN:  ",
    "YELLOW:  ",
    "RED:  ",
    "BLUE:  "
};

char *chat_macros[10];

boolean altdown;
boolean shiftdown;


//===========================================================================
//
// CT_Init
//
//      Initialize chat mode data
//===========================================================================

void CT_Init(void)
{
    int i;

    head = 0;                   //initialize the queue index
    tail = 0;
    chatmodeon = false;
    memset(ChatQueue, 0, QUEUESIZE);
    for (i = 0; i < MAXPLAYERS; i++)
    {
        chat_dest[i] = 0;
        msgptr[i] = 0;
        memset(plr_lastmsg[i], 0, MESSAGESIZE);
        memset(chat_msg[i], 0, MESSAGESIZE);
    }
    FontABaseLump = W_GetNumForName(DEH_String("FONTA_S")) + 1;
    return;
}

//===========================================================================
//
// CT_Stop
//
//===========================================================================

void CT_Stop(void)
{
    chatmodeon = false;
    I_StopTextInput();
    return;
}

// These keys are allowed by Vanilla Heretic:

static boolean ValidChatChar(char c)
{
    return (c >= 'a' && c <= 'z')
        || (c >= 'A' && c <= 'Z')
        || (c >= '0' && c <= '9')
        || c == '!' || c == '?'
        || c == ' ' || c == '\''
        || c == ',' || c == '.'
        || c == '-' || c == '=';
}

//===========================================================================
//
// CT_Responder
//
//===========================================================================

boolean CT_Responder(event_t * ev)
{
    char *macro;

    int sendto;

    if (!netgame)
    {
        return false;
    }
    if (ev->data1 == KEY_LALT || ev->data2 == KEY_RALT)
    {
        altdown = (ev->type == ev_keydown);
        return false;
    }
    if (ev->data1 == KEY_RSHIFT)
    {
        shiftdown = (ev->type == ev_keydown);
        return false;
    }
    if (ev->type != ev_keydown)
    {
        return false;
    }
    if (!chatmodeon)
    {
        sendto = 0;
        if (ev->data1 == key_multi_msg)
        {
            sendto = CT_PLR_ALL;
        }
        else if (ev->data1 == key_multi_msgplayer[0])
        {
            sendto = CT_PLR_GREEN;
        }
        else if (ev->data1 == key_multi_msgplayer[1])
        {
            sendto = CT_PLR_YELLOW;
        }
        else if (ev->data1 == key_multi_msgplayer[2])
        {
            sendto = CT_PLR_RED;
        }
        else if (ev->data1 == key_multi_msgplayer[3])
        {
            sendto = CT_PLR_BLUE;
        }
        if (sendto == 0 || (sendto != CT_PLR_ALL && !playeringame[sendto - 1])
            || sendto == consoleplayer + 1)
        {
            return false;
        }
        CT_queueChatChar(sendto);
        chatmodeon = true;
        I_StartTextInput(25, 10, SCREENWIDTH, 18);
        return true;
    }
    else
    {
        if (altdown)
        {
            if (ev->data1 >= '0' && ev->data1 <= '9')
            {
                if (ev->data1 == '0')
                {               // macro 0 comes after macro 9
                    ev->data1 = '9' + 1;
                }
                macro = chat_macros[ev->data1 - '1'];
                CT_queueChatChar(KEY_ENTER);    //send old message
                CT_queueChatChar(chat_dest[consoleplayer]);     // chose the dest.
                while (*macro)
                {
                    CT_queueChatChar(toupper(*macro++));
                }
                CT_queueChatChar(KEY_ENTER);    //send it off...
                CT_Stop();
                return true;
            }
        }
        if (ev->data1 == KEY_ENTER)
        {
            CT_queueChatChar(KEY_ENTER);
            CT_Stop();
            return true;
        }
        else if (ev->data1 == KEY_ESCAPE)
        {
            CT_queueChatChar(CT_ESCAPE);
            CT_Stop();
            return true;
        }
        else if (ev->data1 == KEY_BACKSPACE)
        {
            CT_queueChatChar(KEY_BACKSPACE);
            return true;
        }
        else if (ValidChatChar(ev->data3))
        {
            CT_queueChatChar(toupper(ev->data3));
            return true;
        }
    }
    return false;
}

//===========================================================================
//
// CT_Ticker
//
//===========================================================================

void CT_Ticker(void)
{
    int i;
    int j;
    char c;
    int numplayers;

    for (i = 0; i < MAXPLAYERS; i++)
    {
        if (!playeringame[i])
        {
            continue;
        }
        if ((c = players[i].cmd.chatchar) != 0)
        {
            if (c <= 5)
            {
                chat_dest[i] = c;
                continue;
            }
            else if (c == CT_ESCAPE)
            {
                CT_ClearChatMessage(i);
            }
            else if (c == KEY_ENTER)
            {
                numplayers = 0;
                for (j = 0; j < MAXPLAYERS; j++)
                {
                    numplayers += playeringame[j];
                }
                CT_AddChar(i, 0);       // set the end of message character
                if (numplayers > 2)
                {
                    M_StringCopy(plr_lastmsg[i], DEH_String(CT_FromPlrText[i]),
                                 sizeof(plr_lastmsg[i]));
                    M_StringConcat(plr_lastmsg[i], chat_msg[i],
                                   sizeof(plr_lastmsg[i]));
                }
                else
                {
                    M_StringCopy(plr_lastmsg[i], chat_msg[i],
                                 sizeof(plr_lastmsg[i]));
                }
                if (i != consoleplayer && (chat_dest[i] == consoleplayer + 1
                                           || chat_dest[i] == CT_PLR_ALL)
                    && *chat_msg[i])
                {
                    P_SetMessage(&players[consoleplayer], plr_lastmsg[i],
                                 true);
                    S_StartSound(NULL, sfx_chat);
                }
                else if (i == consoleplayer && (*chat_msg[i]))
                {
                    if (numplayers > 1)
                    {
                        P_SetMessage(&players[consoleplayer],
                                     DEH_String("-MESSAGE SENT-"), true);
                        S_StartSound(NULL, sfx_chat);
                    }
                    else
                    {
                        P_SetMessage(&players[consoleplayer],
                                     DEH_String("THERE ARE NO OTHER PLAYERS IN THE GAME!"),
                                     true);
                        S_StartSound(NULL, sfx_chat);
                    }
                }
                CT_ClearChatMessage(i);
            }
            else if (c == KEY_BACKSPACE)
            {
                CT_BackSpace(i);
            }
            else
            {
                CT_AddChar(i, c);
            }
        }
    }
    return;
}

//===========================================================================
//
// CT_Drawer
//
//===========================================================================

void CT_Drawer(void)
{
    int i;
    int x;
    patch_t *patch;

    if (chatmodeon)
    {
        x = 25;
        for (i = 0; i < msgptr[consoleplayer]; i++)
        {
            if (chat_msg[consoleplayer][i] < 33)
            {
                x += 6;
            }
            else
            {
                patch = W_CacheLumpNum(FontABaseLump +
                                       chat_msg[consoleplayer][i] - 33,
                                       PU_CACHE);
                V_DrawPatch(x, 10, patch);
                x += patch->width;
            }
        }
        V_DrawPatch(x, 10, W_CacheLumpName(DEH_String("FONTA59"), PU_CACHE));
        BorderTopRefresh = true;
        UpdateState |= I_MESSAGES;
    }
}

//===========================================================================
//
// CT_queueChatChar
//
//===========================================================================

void CT_queueChatChar(char ch)
{
    if (((tail + 1) & (QUEUESIZE - 1)) == head)
    {                           // the queue is full
        return;
    }
    ChatQueue[tail] = ch;
    tail = (tail + 1) & (QUEUESIZE - 1);
}

//===========================================================================
//
// CT_dequeueChatChar
//
//===========================================================================

char CT_dequeueChatChar(void)
{
    byte temp;

    if (head == tail)
    {                           // queue is empty
        return 0;
    }
    temp = ChatQueue[head];
    head = (head + 1) & (QUEUESIZE - 1);
    return temp;
}

//===========================================================================
//
// CT_AddChar
//
//===========================================================================

void CT_AddChar(int player, char c)
{
    patch_t *patch;

    if (msgptr[player] + 1 >= MESSAGESIZE || msglen[player] >= MESSAGELEN)
    {                           // full.
        return;
    }
    chat_msg[player][msgptr[player]] = c;
    msgptr[player]++;
    if (c < 33)
    {
        msglen[player] += 6;
    }
    else
    {
        patch = W_CacheLumpNum(FontABaseLump + c - 33, PU_CACHE);
        msglen[player] += patch->width;
    }
}

//===========================================================================
//
// CT_BackSpace
//
//      Backs up a space, when the user hits (obviously) backspace
//===========================================================================

void CT_BackSpace(int player)
{
    patch_t *patch;
    char c;

    if (msgptr[player] == 0)
    {                           // message is already blank
        return;
    }
    msgptr[player]--;
    c = chat_msg[player][msgptr[player]];
    if (c < 33)
    {
        msglen[player] -= 6;
    }
    else
    {
        patch = W_CacheLumpNum(FontABaseLump + c - 33, PU_CACHE);
        msglen[player] -= patch->width;
    }
    chat_msg[player][msgptr[player]] = 0;
}

//===========================================================================
//
// CT_ClearChatMessage
//
//      Clears out the data for the chat message, but the player's message
//              is still saved in plrmsg.
//===========================================================================

void CT_ClearChatMessage(int player)
{
    memset(chat_msg[player], 0, MESSAGESIZE);
    msgptr[player] = 0;
    msglen[player] = 0;
}
