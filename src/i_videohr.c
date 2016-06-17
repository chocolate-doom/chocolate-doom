//
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
//     SDL emulation of VGA 640x480x4 planar video mode,
//     for Hexen startup loading screen.
//

#include "SDL.h"
#include "string.h"

#include "doomtype.h"
#include "i_timer.h"

// Palette fade-in takes two seconds

#define FADE_TIME 2000

#define HR_SCREENWIDTH 640
#define HR_SCREENHEIGHT 480

static SDL_Surface *hr_screen = NULL;
static SDL_Surface *hr_surface = NULL;
static char *window_title = "";

boolean I_SetVideoModeHR(void)
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        return false;
    }

    SDL_WM_SetCaption(window_title,  NULL);

    // Create screen surface at the native desktop pixel depth (bpp=0),
    // as we cannot trust true 8-bit to reliably work nowadays.
    hr_screen = SDL_SetVideoMode(HR_SCREENWIDTH, HR_SCREENHEIGHT, 0, 0);

    if (hr_screen == NULL)
    {
        SDL_QuitSubSystem(SDL_INIT_VIDEO);
        return false;
    }

    // We do all actual drawing into an intermediate surface.
    hr_surface = SDL_CreateRGBSurface(0, HR_SCREENWIDTH, HR_SCREENHEIGHT,
                                      8, 0, 0, 0, 0);

    return true;
}

void I_SetWindowTitleHR(char *title)
{
    window_title = title;
}

void I_UnsetVideoModeHR(void)
{
    if (hr_screen != NULL)
    {
        SDL_QuitSubSystem(SDL_INIT_VIDEO);
        hr_screen = NULL;
        SDL_FreeSurface(hr_surface);
        hr_surface = NULL;
    }
}

void I_ClearScreenHR(void)
{
    SDL_Rect area = { 0, 0, HR_SCREENWIDTH, HR_SCREENHEIGHT };

    SDL_FillRect(hr_surface, &area, 0);
}

void I_SlamBlockHR(int x, int y, int w, int h, const byte *src)
{
    SDL_Rect blit_rect;
    const byte *srcptrs[4];
    byte srcbits[4];
    byte *dest;
    int x1, y1;
    int i;
    int bit;

    // Set up source pointers to read from source buffer - each 4-bit
    // pixel has its bits split into four sub-buffers

    for (i=0; i<4; ++i)
    {
        srcptrs[i] = src + (i * w * h / 8);
    }

    if (SDL_LockSurface(hr_surface) < 0)
    {
        return;
    }

    // Draw each pixel

    bit = 0;

    for (y1=y; y1<y+h; ++y1)
    {
        dest = ((byte *) hr_surface->pixels) + y1 * hr_surface->pitch + x;

        for (x1=x; x1<x+w; ++x1)
        {
            // Get the bits for this pixel
            // For each bit, find the byte containing it, shift down
            // and mask out the specific bit wanted.

            for (i=0; i<4; ++i)
            {
                srcbits[i] = (srcptrs[i][bit / 8] >> (7 - (bit % 8))) & 0x1;
            }

            // Reassemble the pixel value

            *dest = (srcbits[0] << 0)
                  | (srcbits[1] << 1)
                  | (srcbits[2] << 2)
                  | (srcbits[3] << 3);

            // Next pixel!

            ++dest;
            ++bit;
        }
    }

    SDL_UnlockSurface(hr_surface);

    // Update the region we drew.
    blit_rect.x = x;
    blit_rect.y = y;
    blit_rect.w = w;
    blit_rect.h = h;
    SDL_BlitSurface(hr_surface, &blit_rect, hr_screen, &blit_rect);
    SDL_UpdateRects(hr_screen, 1, &blit_rect);
}

void I_SlamHR(const byte *buffer)
{
    I_SlamBlockHR(0, 0, HR_SCREENWIDTH, HR_SCREENHEIGHT, buffer);
}

void I_InitPaletteHR(void)
{
    // ...
}

void I_SetPaletteHR(const byte *palette)
{
    SDL_Rect screen_rect = {0, 0, HR_SCREENWIDTH, HR_SCREENHEIGHT};
    SDL_Color sdlpal[16];
    int i;

    for (i=0; i<16; ++i)
    {
        sdlpal[i].r = palette[i * 3 + 0] * 4;
        sdlpal[i].g = palette[i * 3 + 1] * 4;
        sdlpal[i].b = palette[i * 3 + 2] * 4;
    }

    // After setting colors, update the screen.
    SDL_SetColors(hr_surface, sdlpal, 0, 16);
    SDL_BlitSurface(hr_surface, &screen_rect, hr_screen, &screen_rect);
    SDL_UpdateRects(hr_screen, 1, &screen_rect);
}

void I_FadeToPaletteHR(const byte *palette)
{
    byte tmppal[16 * 3];
    int starttime;
    int elapsed;
    int i;

    starttime = I_GetTimeMS();

    for (;;)
    {
        elapsed = I_GetTimeMS() - starttime;

        if (elapsed >= FADE_TIME)
        {
            break;
        }

        // Generate the fake palette

        for (i=0; i<16 * 3; ++i)
        {
            tmppal[i] = (palette[i] * elapsed) / FADE_TIME;
        }

        I_SetPaletteHR(tmppal);
        SDL_Flip(hr_surface);

        // Sleep a bit

        I_Sleep(10);
    }

    // Set the final palette

    I_SetPaletteHR(palette);
}

void I_BlackPaletteHR(void)
{
    byte blackpal[16 * 3];

    memset(blackpal, 0, sizeof(blackpal));

    I_SetPaletteHR(blackpal);
}

// Check if the user has hit the escape key to abort startup.
boolean I_CheckAbortHR(void)
{
    SDL_Event ev;
    boolean result = false;

    // Not initialized?
    if (hr_surface == NULL)
    {
        return false;
    }

    while (SDL_PollEvent(&ev))
    {
        if (ev.type == SDL_KEYDOWN && ev.key.keysym.sym == SDLK_ESCAPE)
        {
            result = true;
        }
    }

    return result;
}

