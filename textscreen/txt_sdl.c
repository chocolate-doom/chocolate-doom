// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// Copyright(C) 2005,2006 Simon Howard
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
//-----------------------------------------------------------------------------
//
// Text mode emulation in SDL
//
//-----------------------------------------------------------------------------

#include "SDL.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "doomkeys.h"

#include "txt_main.h"
#include "txt_sdl.h"

#if defined(_MSC_VER) && !defined(__cplusplus)
#define inline __inline
#endif

typedef struct
{
    unsigned char *data;
    unsigned int w;
    unsigned int h;
} txt_font_t;

// Fonts:

#include "txt_font.h"
#include "txt_smallfont.h"

// Time between character blinks in ms

#define BLINK_PERIOD 250

static SDL_Surface *screen;
static unsigned char *screendata;
static int key_mapping = 1;

static TxtSDLEventCallbackFunc event_callback;
static void *event_callback_data;

static int modifier_state[TXT_NUM_MODIFIERS];

// Font we are using:

static txt_font_t *font;

//#define TANGO

#ifndef TANGO

static SDL_Color ega_colors[] = 
{
    {0x00, 0x00, 0x00, 0x00},          // 0: Black
    {0x00, 0x00, 0xa8, 0x00},          // 1: Blue
    {0x00, 0xa8, 0x00, 0x00},          // 2: Green
    {0x00, 0xa8, 0xa8, 0x00},          // 3: Cyan
    {0xa8, 0x00, 0x00, 0x00},          // 4: Red
    {0xa8, 0x00, 0xa8, 0x00},          // 5: Magenta
    {0xa8, 0x54, 0x00, 0x00},          // 6: Brown
    {0xa8, 0xa8, 0xa8, 0x00},          // 7: Grey
    {0x54, 0x54, 0x54, 0x00},          // 8: Dark grey
    {0x54, 0x54, 0xfe, 0x00},          // 9: Bright blue
    {0x54, 0xfe, 0x54, 0x00},          // 10: Bright green
    {0x54, 0xfe, 0xfe, 0x00},          // 11: Bright cyan
    {0xfe, 0x54, 0x54, 0x00},          // 12: Bright red
    {0xfe, 0x54, 0xfe, 0x00},          // 13: Bright magenta
    {0xfe, 0xfe, 0x54, 0x00},          // 14: Yellow
    {0xfe, 0xfe, 0xfe, 0x00},          // 15: Bright white
};

#else

// Colors that fit the Tango desktop guidelines: see
// http://tango.freedesktop.org/ also
// http://uwstopia.nl/blog/2006/07/tango-terminal

static SDL_Color ega_colors[] = 
{
    {0x2e, 0x34, 0x36, 0x00},          // 0: Black
    {0x34, 0x65, 0xa4, 0x00},          // 1: Blue
    {0x4e, 0x9a, 0x06, 0x00},          // 2: Green
    {0x06, 0x98, 0x9a, 0x00},          // 3: Cyan
    {0xcc, 0x00, 0x00, 0x00},          // 4: Red
    {0x75, 0x50, 0x7b, 0x00},          // 5: Magenta
    {0xc4, 0xa0, 0x00, 0x00},          // 6: Brown
    {0xd3, 0xd7, 0xcf, 0x00},          // 7: Grey
    {0x55, 0x57, 0x53, 0x00},          // 8: Dark grey
    {0x72, 0x9f, 0xcf, 0x00},          // 9: Bright blue
    {0x8a, 0xe2, 0x34, 0x00},          // 10: Bright green
    {0x34, 0xe2, 0xe2, 0x00},          // 11: Bright cyan
    {0xef, 0x29, 0x29, 0x00},          // 12: Bright red
    {0x34, 0xe2, 0xe2, 0x00},          // 13: Bright magenta
    {0xfc, 0xe9, 0x4f, 0x00},          // 14: Yellow
    {0xee, 0xee, 0xec, 0x00},          // 15: Bright white
};

#endif

static txt_font_t *FontForName(char *name)
{
    if (!strcmp(name, "small"))
    {
        return &small_font;
    }
    else if (!strcmp(name, "normal"))
    {
        return &main_font;
    }
    else
    {
        return NULL;
    }
}

//
// Select the font to use, based on screen resolution
//
// If the highest screen resolution available is less than
// 640x480, use the small font.
//

static void ChooseFont(void)
{
    SDL_Rect **modes;
    char *env;
    int i;

    // Allow normal selection to be overridden from an environment variable:

    env = getenv("TEXTSCREEN_FONT");

    if (env != NULL)
    {
        font = FontForName(env);

        if (font != NULL)
        {
            return;
        }
    }

    // Check all modes

    modes = SDL_ListModes(NULL, SDL_FULLSCREEN);

    // If in doubt and we can't get a list, always prefer to
    // fall back to the normal font:

    font = &main_font;

    if (modes == NULL || modes == (SDL_Rect **) -1 || *modes == NULL)
    {
#ifdef _WIN32_WCE
        font = &small_font;
#endif
        return;
    }

    for (i=0; modes[i] != NULL; ++i)
    {
        if (modes[i]->w >= 640 && modes[i]->h >= 480)
        {
            return;
        }
    }

    // No large mode found.

    font = &small_font;
}

//
// Initialize text mode screen
//
// Returns 1 if successful, 0 if an error occurred
//

int TXT_Init(void)
{
    if (SDL_InitSubSystem(SDL_INIT_VIDEO) < 0)
    {
        return 0;
    }

    ChooseFont();

    screen = SDL_SetVideoMode(TXT_SCREEN_W * font->w,
                              TXT_SCREEN_H * font->h, 8, 0);

    if (screen == NULL)
        return 0;

    SDL_SetColors(screen, ega_colors, 0, 16);
    SDL_EnableUNICODE(1);

    screendata = malloc(TXT_SCREEN_W * TXT_SCREEN_H * 2);
    memset(screendata, 0, TXT_SCREEN_W * TXT_SCREEN_H * 2);

    // Ignore all mouse motion events

//    SDL_EventState(SDL_MOUSEMOTION, SDL_IGNORE);

    // Repeat key presses so we can hold down arrows to scroll down the
    // menu, for example. This is what setup.exe does.

    SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);

    return 1;
}

void TXT_Shutdown(void)
{
    free(screendata);
    SDL_QuitSubSystem(SDL_INIT_VIDEO);
}

unsigned char *TXT_GetScreenData(void)
{
    return screendata;
}

static inline void UpdateCharacter(int x, int y)
{
    unsigned char character;
    unsigned char *p;
    unsigned char *s, *s1;
    int bg, fg;
    unsigned int x1, y1;

    p = &screendata[(y * TXT_SCREEN_W + x) * 2];
    character = p[0];

    fg = p[1] & 0xf;
    bg = (p[1] >> 4) & 0xf;

    if (bg & 0x8)
    {
        // blinking

        bg &= ~0x8;

        if (((SDL_GetTicks() / BLINK_PERIOD) % 2) == 0)
        {
            fg = bg;
        }
    }

    p = &font->data[character * font->h];

    s = ((unsigned char *) screen->pixels) 
          + (y * font->h * screen->pitch) + (x * font->w);

    for (y1=0; y1<font->h; ++y1)
    {
        s1 = s;

        for (x1=0; x1<font->w; ++x1)
        {
            if (*p & (1 << (7-x1)))
            {
                *s1++ = fg;
            }
            else
            {
                *s1++ = bg;
            }
        }

        ++p;
        s += screen->pitch;
    }
}

static int LimitToRange(int val, int min, int max)
{
    if (val < min)
    {
        return min;
    }
    else if (val > max)
    {
        return max;
    }
    else
    {
        return val;
    }
}

void TXT_UpdateScreenArea(int x, int y, int w, int h)
{
    int x1, y1;
    int x_end;
    int y_end;

    x_end = LimitToRange(x + w, 0, TXT_SCREEN_W);
    y_end = LimitToRange(y + h, 0, TXT_SCREEN_H);
    x = LimitToRange(x, 0, TXT_SCREEN_W);
    y = LimitToRange(y, 0, TXT_SCREEN_H);

    for (y1=y; y1<y_end; ++y1)
    {
        for (x1=x; x1<x_end; ++x1)
        {
            UpdateCharacter(x1, y1);
        }
    }

    SDL_UpdateRect(screen,
                   x * font->w, y * font->h,
                   (x_end - x) * font->w, (y_end - y) * font->h);
}

void TXT_UpdateScreen(void)
{
    TXT_UpdateScreenArea(0, 0, TXT_SCREEN_W, TXT_SCREEN_H);
}

void TXT_GetMousePosition(int *x, int *y)
{
#if SDL_VERSION_ATLEAST(1, 3, 0)
    SDL_GetMouseState(0, x, y);
#else
    SDL_GetMouseState(x, y);
#endif

    *x /= font->w;
    *y /= font->h;
}

//
// Translates the SDL key
//

static int TranslateKey(SDL_keysym *sym)
{
    switch(sym->sym)
    {
        case SDLK_LEFT:        return KEY_LEFTARROW;
        case SDLK_RIGHT:       return KEY_RIGHTARROW;
        case SDLK_DOWN:        return KEY_DOWNARROW;
        case SDLK_UP:          return KEY_UPARROW;
        case SDLK_ESCAPE:      return KEY_ESCAPE;
        case SDLK_RETURN:      return KEY_ENTER;
        case SDLK_TAB:         return KEY_TAB;
        case SDLK_F1:          return KEY_F1;
        case SDLK_F2:          return KEY_F2;
        case SDLK_F3:          return KEY_F3;
        case SDLK_F4:          return KEY_F4;
        case SDLK_F5:          return KEY_F5;
        case SDLK_F6:          return KEY_F6;
        case SDLK_F7:          return KEY_F7;
        case SDLK_F8:          return KEY_F8;
        case SDLK_F9:          return KEY_F9;
        case SDLK_F10:         return KEY_F10;
        case SDLK_F11:         return KEY_F11;
        case SDLK_F12:         return KEY_F12;

        case SDLK_BACKSPACE:   return KEY_BACKSPACE;
        case SDLK_DELETE:      return KEY_DEL;

        case SDLK_PAUSE:       return KEY_PAUSE;

        case SDLK_LSHIFT:
        case SDLK_RSHIFT:
                               return KEY_RSHIFT;

        case SDLK_LCTRL:
        case SDLK_RCTRL:
                               return KEY_RCTRL;

        case SDLK_LALT:
        case SDLK_RALT:
#if !SDL_VERSION_ATLEAST(1, 3, 0)
        case SDLK_LMETA:
        case SDLK_RMETA:
#endif
                               return KEY_RALT;

        case SDLK_CAPSLOCK:    return KEY_CAPSLOCK;
        case SDLK_SCROLLOCK:   return KEY_SCRLCK;

        case SDLK_KP0:         return KEYP_0;
        case SDLK_KP1:         return KEYP_1;
        case SDLK_KP2:         return KEYP_2;
        case SDLK_KP3:         return KEYP_3;
        case SDLK_KP4:         return KEYP_4;
        case SDLK_KP5:         return KEYP_5;
        case SDLK_KP6:         return KEYP_6;
        case SDLK_KP7:         return KEYP_7;
        case SDLK_KP8:         return KEYP_8;
        case SDLK_KP9:         return KEYP_9;

        case SDLK_KP_PERIOD:   return KEYP_PERIOD;
        case SDLK_KP_MULTIPLY: return KEYP_MULTIPLY;
        case SDLK_KP_PLUS:     return KEYP_PLUS;
        case SDLK_KP_MINUS:    return KEYP_MINUS;
        case SDLK_KP_DIVIDE:   return KEYP_DIVIDE;
        case SDLK_KP_EQUALS:   return KEYP_EQUALS;
        case SDLK_KP_ENTER:    return KEYP_ENTER;

        case SDLK_HOME:        return KEY_HOME;
        case SDLK_INSERT:      return KEY_INS;
        case SDLK_END:         return KEY_END;
        case SDLK_PAGEUP:      return KEY_PGUP;
        case SDLK_PAGEDOWN:    return KEY_PGDN;

#ifdef SDL_HAVE_APP_KEYS
        case SDLK_APP1:        return KEY_F1;
        case SDLK_APP2:        return KEY_F2;
        case SDLK_APP3:        return KEY_F3;
        case SDLK_APP4:        return KEY_F4;
        case SDLK_APP5:        return KEY_F5;
        case SDLK_APP6:        return KEY_F6;
#endif

        default:               break;
    }

    // Returned value is different, depending on whether key mapping is
    // enabled.  Key mapping is preferable most of the time, for typing
    // in text, etc.  However, when we want to read raw keyboard codes
    // for the setup keyboard configuration dialog, we want the raw
    // key code.

    if (key_mapping)
    {
        return sym->unicode;
    }
    else
    {
        return tolower(sym->sym);
    }
}

// Convert an SDL button index to textscreen button index.
//
// Note special cases because 2 == mid in SDL, 3 == mid in textscreen/setup

static int SDLButtonToTXTButton(int button)
{
    switch (button)
    {
        case SDL_BUTTON_LEFT:
            return TXT_MOUSE_LEFT;
        case SDL_BUTTON_RIGHT:
            return TXT_MOUSE_RIGHT;
        case SDL_BUTTON_MIDDLE:
            return TXT_MOUSE_MIDDLE;
        default:
            return TXT_MOUSE_BASE + button - 1;
    }
}

static int MouseHasMoved(void)
{
    static int last_x = 0, last_y = 0;
    int x, y;

    TXT_GetMousePosition(&x, &y);

    if (x != last_x || y != last_y)
    {
        last_x = x; last_y = y;
        return 1;
    }
    else
    {
        return 0;
    }
}

// Examine a key press/release and update the modifier key state
// if necessary.

static void UpdateModifierState(SDL_keysym *sym, int pressed)
{
    txt_modifier_t mod;

    switch (sym->sym)
    {
        case SDLK_LSHIFT:
        case SDLK_RSHIFT:
            mod = TXT_MOD_SHIFT;
            break;

        case SDLK_LCTRL:
        case SDLK_RCTRL:
            mod = TXT_MOD_CTRL;
            break;

        case SDLK_LALT:
        case SDLK_RALT:
#if !SDL_VERSION_ATLEAST(1, 3, 0)
        case SDLK_LMETA:
        case SDLK_RMETA:
#endif
            mod = TXT_MOD_ALT;
            break;

        default:
            return;
    }

    if (pressed)
    {
        ++modifier_state[mod];
    }
    else
    {
        --modifier_state[mod];
    }
}

signed int TXT_GetChar(void)
{
    SDL_Event ev;

    while (SDL_PollEvent(&ev))
    {
        // If there is an event callback, allow it to intercept this
        // event.

        if (event_callback != NULL)
        {
            if (event_callback(&ev, event_callback_data))
            {
                continue;
            }
        }

        // Process the event.

        switch (ev.type)
        {
            case SDL_MOUSEBUTTONDOWN:
                if (ev.button.button < TXT_MAX_MOUSE_BUTTONS)
                {
                    return SDLButtonToTXTButton(ev.button.button);
                }
                break;

            case SDL_KEYDOWN:
                UpdateModifierState(&ev.key.keysym, 1);

                return TranslateKey(&ev.key.keysym);

            case SDL_KEYUP:
                UpdateModifierState(&ev.key.keysym, 0);
                break;

            case SDL_QUIT:
                // Quit = escape
                return 27;

            case SDL_MOUSEMOTION:
                if (MouseHasMoved())
                {
                    return 0;
                }

            default:
                break;
        }
    }

    return -1;
}

int TXT_GetModifierState(txt_modifier_t mod)
{
    if (mod < TXT_NUM_MODIFIERS)
    {
        return modifier_state[mod] > 0;
    }

    return 0;
}

static const char *SpecialKeyName(int key)
{
    switch (key)
    {
        case ' ':             return "SPACE";
        case KEY_RIGHTARROW:  return "RIGHT";
        case KEY_LEFTARROW:   return "LEFT";
        case KEY_UPARROW:     return "UP";
        case KEY_DOWNARROW:   return "DOWN";
        case KEY_ESCAPE:      return "ESC";
        case KEY_ENTER:       return "ENTER";
        case KEY_TAB:         return "TAB";
        case KEY_F1:          return "F1";
        case KEY_F2:          return "F2";
        case KEY_F3:          return "F3";
        case KEY_F4:          return "F4";
        case KEY_F5:          return "F5";
        case KEY_F6:          return "F6";
        case KEY_F7:          return "F7";
        case KEY_F8:          return "F8";
        case KEY_F9:          return "F9";
        case KEY_F10:         return "F10";
        case KEY_F11:         return "F11";
        case KEY_F12:         return "F12";
        case KEY_BACKSPACE:   return "BKSP";
        case KEY_PAUSE:       return "PAUSE";
        case KEY_EQUALS:      return "EQUALS";
        case KEY_MINUS:       return "MINUS";
        case KEY_RSHIFT:      return "SHIFT";
        case KEY_RCTRL:       return "CTRL";
        case KEY_RALT:        return "ALT";
        case KEY_CAPSLOCK:    return "CAPS";
        case KEY_SCRLCK:      return "SCRLCK";
        case KEY_HOME:        return "HOME";
        case KEY_END:         return "END";
        case KEY_PGUP:        return "PGUP";
        case KEY_PGDN:        return "PGDN";
        case KEY_INS:         return "INS";
        case KEY_DEL:         return "DEL";
                 /*
        case KEYP_0:          return "PAD0";
        case KEYP_1:          return "PAD1";
        case KEYP_2:          return "PAD2";
        case KEYP_3:          return "PAD3";
        case KEYP_4:          return "PAD4";
        case KEYP_5:          return "PAD5";
        case KEYP_6:          return "PAD6";
        case KEYP_7:          return "PAD7";
        case KEYP_8:          return "PAD8";
        case KEYP_9:          return "PAD9";
        case KEYP_UPARROW:    return "PAD_U";
        case KEYP_DOWNARROW:  return "PAD_D";
        case KEYP_LEFTARROW:  return "PAD_L";
        case KEYP_RIGHTARROW: return "PAD_R";
        case KEYP_MULTIPLY:   return "PAD*";
        case KEYP_PLUS:       return "PAD+";
        case KEYP_MINUS:      return "PAD-";
        case KEYP_DIVIDE:     return "PAD/";
                   */

        default:              return NULL;
    }
}

void TXT_GetKeyDescription(int key, char *buf)
{
    const char *keyname;

    keyname = SpecialKeyName(key);

    if (keyname != NULL)
    {
        strcpy(buf, keyname);
    }
    else if (isprint(key))
    {
        sprintf(buf, "%c", toupper(key));
    }
    else
    {
        sprintf(buf, "??%i", key);
    }
}

// Searches the desktop screen buffer to determine whether there are any
// blinking characters.

int TXT_ScreenHasBlinkingChars(void)
{
    int x, y;
    unsigned char *p;

    // Check all characters in screen buffer

    for (y=0; y<TXT_SCREEN_H; ++y)
    {
        for (x=0; x<TXT_SCREEN_W; ++x) 
        {
            p = &screendata[(y * TXT_SCREEN_W + x) * 2];

            if (p[1] & 0x80)
            {
                // This character is blinking

                return 1;
            }
        }
    }

    // None found

    return 0;
}

// Sleeps until an event is received, the screen needs to be redrawn, 
// or until timeout expires (if timeout != 0)

void TXT_Sleep(int timeout)
{
    unsigned int start_time;

    if (TXT_ScreenHasBlinkingChars())
    {
        int time_to_next_blink;

        time_to_next_blink = BLINK_PERIOD - (SDL_GetTicks() % BLINK_PERIOD);

        // There are blinking characters on the screen, so we 
        // must time out after a while
       
        if (timeout == 0 || timeout > time_to_next_blink)
        {
            // Add one so it is always positive

            timeout = time_to_next_blink + 1;
        }
    }

    if (timeout == 0)
    {
        // We can just wait forever until an event occurs

        SDL_WaitEvent(NULL);
    }
    else
    {
        // Sit in a busy loop until the timeout expires or we have to
        // redraw the blinking screen

        start_time = SDL_GetTicks();

        while (SDL_GetTicks() < start_time + timeout)
        {
            if (SDL_PollEvent(NULL) != 0)
            {
                // Received an event, so stop waiting

                break;
            }

            // Don't hog the CPU

            SDL_Delay(1);
        }
    }
}

void TXT_EnableKeyMapping(int enable)
{
    key_mapping = enable;
}

void TXT_SetWindowTitle(char *title)
{
    SDL_WM_SetCaption(title, NULL);
}

void TXT_SDL_SetEventCallback(TxtSDLEventCallbackFunc callback, void *user_data)
{
    event_callback = callback;
    event_callback_data = user_data;
}

