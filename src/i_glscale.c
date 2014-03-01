// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
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
//     OpenGL-based scaling.
//
// The idea here is to do screen scaling and aspect ratio correction similar
// to that done in software in i_scale.c. However, Chocolate Doom is pretty
// unique and unorthodox in the way it does scaling.
//
// OpenGL has two basic scaling modes:
//   GL_NEAREST: Blocky scale up which works fine for integer multiple
//       scaling, but for non-integer scale factors causes distortion
//       of shapes, as some pixels will be doubled up.
//   GL_LINEAR:  Scale based on linear interpolation between the pixels
//       of the original texture. This causes a blurry appearance which
//       is unpleasant and unauthentic.
//
// What we want instead is a "blocky" scale-up with linear interpolation
// at the edges of the pixels to give a consistent appearance. In OpenGL
// we achieve this through a two-stage process:
//
// 1. We draw the software screen buffer into an OpenGL texture (named
//    unscaled_texture).
// 2. We draw the unscaled texture into a larger texture, scaling up
//    using GL_NEAREST to an integer multiple of the original screen
//    size. For example, 320x200 -> 960x600. GL framebuffers are used
//    to draw into the second texture (scaled_texture).
// 3. We draw the scaled texture to the screen, using GL_LINEAR to scale
//    back down to fit the actual screen resolution.
//
// Notes:
//  * The scaled texture should be a maximum of 2x the actual screen
//    size. This is because GL_LINEAR uses a 2x2 matrix of pixels to
//    interpolate between. If we go over 2x the actual screen size,
//    when we scale down we end up losing quality.
//  * The larger the scaled texture, the more accurately "blocky" the
//    screen appears; however, it's diminishing returns. After about
//    4x there's no real perceptible difference.
//
//-----------------------------------------------------------------------------

// TODO: Entire file should be surrounded with a #ifdef ENABLE_OPENGL
// so we can build without OpenGL.

#include <SDL.h>
#ifdef __MACOSX__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#endif

#include <stdio.h>
#include <stdlib.h>

#include "i_video.h"

// Maximum scale factor for the intermediate scaled texture. A value
// of 4 is pretty much perfect; you can try larger values but it's
// a case of diminishing returns.
int gl_max_scale = 4;

// Screen dimensions:
static int screen_w, screen_h;

// Size of the calculated "window" of the screen that shows content:
// at a 4:3 mode these are equal to screen_w, screen_h.
static int window_w, window_h;

// The texture that "receives" the original 320x200 screen contents:
static GLuint unscaled_texture;
static unsigned int *unscaled_data;

// The "scaled" version of the texture:
static GLuint scaled_framebuffer;
static GLuint scaled_texture;
static int scaled_w, scaled_h;

// The converted GL_RGBA format palette.
static unsigned int gl_palette[256];

// Called on startup or on window resize so that we calculate the
// size of the actual "window" where we show the game screen.
static void CalculateWindowSize(void)
{
    int base_width, base_height;

    // Aspect ratio we want depends on whether we have correction
    // turned on.
    if (aspect_ratio_correct)
    {
        base_width = SCREENWIDTH;
        base_height = SCREENHEIGHT_4_3;
    }
    else
    {
        base_width = SCREENWIDTH;
        base_height = SCREENHEIGHT;
    }

    // Either we will have borders to the left and right sides,
    // or at the top and bottom. Which is it?
    if (screen_w * base_height > screen_h * base_width)
    {
        window_w = (screen_h * base_width) / base_height;
        window_h = screen_h;
    }
    else
    {
        window_w = screen_w;
        window_h = (screen_w * base_height) / base_width;
    }
}

// Create the OpenGL textures used for scaling.
static void CreateTextures(void)
{
    int factor;

    // TODO: Check for GL_ARB_texture_non_power_of_two in the string
    // returned by glGetString(GL_EXTENSIONS); if not present then
    // we need to do something here as a workaround.
    // OES_texture_npot also works.

    // Unscaled texture for input:
    unscaled_data = malloc(SCREENWIDTH * SCREENHEIGHT * sizeof(int));
    glGenTextures(1, &unscaled_texture);
    glBindTexture(GL_TEXTURE_2D, unscaled_texture);
    memset(unscaled_data, 0x77, SCREENWIDTH * SCREENHEIGHT * sizeof(int)/2);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, SCREENWIDTH, SCREENHEIGHT, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, unscaled_data);

    // Framebuffer for scaled texture:
    glGenFramebuffers(1, &scaled_framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, scaled_framebuffer);

    // TODO: Check GL_MAX_TEXTURE_SIZE and set a tighter maximum scale
    // size if necessary.

    // How big is the scaled texture?
    // It must be an integer multiple of the original (unscaled) screen
    // size, but no more than 2x the screen size we are rendering -
    // GL_LINEAR uses a 2x2 matrix to calculate which pixel to use.
    // Clamp the scale factor because we don't want to get too large.
    factor = (window_w + SCREENWIDTH - 1) / SCREENWIDTH;
    if (factor > gl_max_scale)
    {
        factor = gl_max_scale;
    }
    scaled_w = SCREENWIDTH * factor;

    factor = (window_h + SCREENHEIGHT - 1) / SCREENHEIGHT;
    if (factor > gl_max_scale)
    {
        factor = gl_max_scale;
    }
    scaled_h = SCREENHEIGHT * factor;

    // Scaled texture:
    glGenTextures(1, &scaled_texture);
    glBindTexture(GL_TEXTURE_2D, scaled_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, scaled_w, scaled_h, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, NULL);
}

// Convert the given Doom-format palette into a GL_RGBA palette
// that we can use to convert the screen.
static void ConvertPalette(byte *palette, unsigned int *out)
{
    unsigned int i;
    byte *o;

    o = (byte *) out;

    for (i = 0; i < 256; ++i)
    {
        // Zero out the bottom two bits of each channel - the PC VGA
        // controller only supports 6 bits of accuracy.

	*o++ = palette[i * 3] & ~3;
	*o++ = palette[i * 3 + 1] & ~3;
	*o++ = palette[i * 3 + 2] & ~3;
	*o++ = 0xff;
    }
}

// Import screen data from the given pointer and palette and update
// the unscaled_texture texture.
static void SetInputData(byte *screen, unsigned int *palette)
{
    unsigned int i;

    // TODO: Maybe support GL_RGB as well as GL_RGBA?
    for (i = 0; i < SCREENWIDTH * SCREENHEIGHT; ++i)
    {
	unscaled_data[i] = palette[screen[i]];
    }

    glBindTexture(GL_TEXTURE_2D, unscaled_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, SCREENWIDTH, SCREENHEIGHT, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, unscaled_data);
}

// Draw unscaled_texture (containing the screen buffer) into the
// second scaled_texture texture.
static void DrawUnscaledToScaled(void)
{
    GLenum DrawBuffers[1] = {GL_COLOR_ATTACHMENT0};

    // Render unscaled texture into scaled texture:
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                         scaled_texture, 0);
    glDrawBuffers(1, DrawBuffers);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
	fprintf(stderr, "Failed to set up framebuffer\n");
        return;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, scaled_framebuffer);
    glLoadIdentity();
    glViewport(0, 0, scaled_w, scaled_h);
    glBindTexture(GL_TEXTURE_2D, unscaled_texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    glBegin(GL_QUADS);
    glTexCoord2f(0, 1); glVertex2f(-1, 1);
    glTexCoord2f(1, 1); glVertex2f(1, 1);
    glTexCoord2f(1, 0); glVertex2f(1, -1);
    glTexCoord2f(0, 0); glVertex2f(-1, -1);
    glEnd();

    // Finished with framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

// Render the scaled_texture to the screen.
static void DrawScreen(void)
{
    GLfloat w, h;

    glClear(GL_COLOR_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glViewport(0, 0, screen_w, screen_h);
    glBindTexture(GL_TEXTURE_2D, scaled_texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    w = (float) window_w / screen_w;
    h = (float) window_h / screen_h;

    glBegin(GL_QUADS);
    glTexCoord2f(0, 0); glVertex2f(-w, h);
    glTexCoord2f(1, 0); glVertex2f(w, h);
    glTexCoord2f(1, 1); glVertex2f(w, -h);
    glTexCoord2f(0, 1); glVertex2f(-w, -h);
    glEnd();
}

void I_GL_PreInit(void)
{
    SDL_GL_SetAttribute( SDL_GL_RED_SIZE, 5 );
    SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, 5 );
    SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, 5 );
    SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );
}

void I_GL_InitScale(int w, int h)
{
    glEnable(GL_TEXTURE_2D);
    glShadeModel(GL_SMOOTH);
    glClearColor(0, 0, 0, 0);

    screen_w = w;
    screen_h = h;
    CalculateWindowSize();
    CreateTextures();
}

void I_GL_SetPalette(byte *palette)
{
    ConvertPalette(palette, gl_palette);
}

void I_GL_UpdateScreen(byte *screendata)
{
    SetInputData(screendata, gl_palette);
    DrawUnscaledToScaled();
    DrawScreen();
}

