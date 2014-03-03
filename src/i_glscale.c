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
#include <OpenGL/glext.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glext.h>
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
static GLuint unscaled_texture = 0;
static unsigned int *unscaled_data = NULL;

// The "scaled" version of the texture:
static GLuint scaled_framebuffer = 0;
static GLuint scaled_texture = 0;
static int scaled_w, scaled_h;

// GL function pointers used for scale code:
static void (*_glBegin)(GLenum);
static void (*_glBindFramebuffer)(GLenum, GLuint);
static void (*_glBindTexture)(GLenum, GLuint);
static GLenum (*_glCheckFramebufferStatus)(GLenum);
static void (*_glClear)(GLbitfield);
static void (*_glClearColor)(GLclampf, GLclampf, GLclampf, GLclampf);
static void (*_glDrawBuffers)(GLsizei, const GLenum *);
static void (*_glEnable)(GLenum);
static void (*_glEnd)(void);
static void (*_glFramebufferTexture)(GLenum, GLenum, GLuint, GLint);
static void (*_glGenFramebuffers)(GLsizei, GLuint *);
static void (*_glGenTextures)(GLsizei, GLuint *);
static const GLubyte *(*_glGetString)(GLenum);
static void (*_glLoadIdentity)(void);
static void (*_glMatrixMode)(GLenum);
static void (*_glShadeModel)(GLenum);
static void (*_glTexCoord2f)(GLfloat, GLfloat);
static void (*_glTexImage2D)(GLenum, GLint, GLint, GLsizei, GLsizei, GLint,
                             GLenum, GLenum, const GLvoid *);
static void (*_glTexParameteri)(GLenum, GLenum, GLint);
static void (*_glVertex2f)(GLfloat, GLfloat);
static void (*_glViewport)(GLint, GLint, GLsizei, GLsizei);

static void *GetGLFunction(char *name)
{
    void *ptr;

    ptr = SDL_GL_GetProcAddress(name);

    if (ptr == NULL)
    {
        fprintf(stderr, "Failed to find GL function: %s\n", name);
    }

    return ptr;
}

static int SetGLFunctions(void)
{
    return
        (_glBegin = GetGLFunction("glBegin"))
     && (_glBindFramebuffer = GetGLFunction("glBindFramebuffer"))
     && (_glBindTexture = GetGLFunction("glBindTexture"))
     && (_glCheckFramebufferStatus = GetGLFunction("glCheckFramebufferStatus"))
     && (_glClear = GetGLFunction("glClear"))
     && (_glClearColor = GetGLFunction("glClearColor"))
     && (_glDrawBuffers = GetGLFunction("glDrawBuffers"))
     && (_glEnable = GetGLFunction("glEnable"))
     && (_glEnd = GetGLFunction("glEnd"))
     && (_glFramebufferTexture = GetGLFunction("glFramebufferTexture"))
     && (_glGenFramebuffers = GetGLFunction("glGenFramebuffers"))
     && (_glGenTextures = GetGLFunction("glGenTextures"))
     && (_glGetString = GetGLFunction("glGetString"))
     && (_glLoadIdentity = GetGLFunction("glLoadIdentity"))
     && (_glMatrixMode = GetGLFunction("glMatrixMode"))
     && (_glShadeModel = GetGLFunction("glShadeModel"))
     && (_glTexCoord2f = GetGLFunction("glTexCoord2f"))
     && (_glTexImage2D = GetGLFunction("glTexImage2D"))
     && (_glTexParameteri = GetGLFunction("glTexParameteri"))
     && (_glVertex2f = GetGLFunction("glVertex2f"))
     && (_glViewport = GetGLFunction("glViewport"));
}

// Returns true if the specified GL extension is available.
static boolean HaveExtension(char *extname)
{
    const GLubyte *last_ext_start;
    const GLubyte *extensions;
    const GLubyte *p;

    extensions = _glGetString(GL_EXTENSIONS);

    if (extensions == NULL)
    {
        fprintf(stderr, "Failed to read GL extensions\n");
        return false;
    }

    // Extensions are listed in a string and separated by spaces.
    p = extensions;
    last_ext_start = extensions;

    while (*p != '\0')
    {
        ++p;

        // Every time we reach an end-of-string (space or NUL), check
        // if the extension we just passed over matched the one we're
        // looking for.
        if (*p == ' ' || *p == '\0')
        {
            if (p - last_ext_start == strlen(extname)
             && !strncmp((char *) last_ext_start, extname, strlen(extname)))
            {
                return true;
            }

            last_ext_start = p + 1;
        }
    }

    fprintf(stderr, "Missing GL extension: %s\n", extname);
    return false;
}

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
static boolean CreateTextures(void)
{
    int factor;

    // We need to have all the required extensions, otherwise this
    // isn't going to work.
    if (!HaveExtension("GL_ARB_texture_non_power_of_two")
     || !HaveExtension("GL_EXT_framebuffer_object"))
    {
        return false;
    }

    // Unscaled texture for input:
    if (unscaled_data == NULL)
    {
        unscaled_data = malloc(SCREENWIDTH * SCREENHEIGHT * sizeof(int));
    }
    if (unscaled_texture == 0)
    {
        _glGenTextures(1, &unscaled_texture);
    }
    _glBindTexture(GL_TEXTURE_2D, unscaled_texture);
    _glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, SCREENWIDTH, SCREENHEIGHT, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, unscaled_data);

    // Framebuffer for scaled texture:
    if (scaled_framebuffer == 0)
    {
        _glGenFramebuffers(1, &scaled_framebuffer);
    }
    _glBindFramebuffer(GL_FRAMEBUFFER, scaled_framebuffer);

    // TODO: Check GL_MAX_TEXTURE_SIZE and set a tighter maximum scale
    // size if necessary.

    // How big is the scaled texture?
    // It must be an integer multiple of the original (unscaled) screen
    // size, but no more than 2x the screen size we are rendering -
    // GL_LINEAR uses a 2x2 matrix to calculate which pixel to use.
    // Clamp the scale factor because we don't want to get too large.
    factor = window_w * 2 / SCREENWIDTH;
    if (window_h <= SCREENWIDTH)
    {
        factor = 1;
    }
    else if (factor > gl_max_scale)
    {
        factor = gl_max_scale;
    }
    scaled_w = SCREENWIDTH * factor;

    factor = window_h * 2 / SCREENHEIGHT;

    if (window_h <= SCREENHEIGHT_4_3)
    {
        factor = 1;
    }
    else if (factor > gl_max_scale)
    {
        factor = gl_max_scale;
    }
    scaled_h = SCREENHEIGHT * factor;

    // Scaled texture:
    _glGenTextures(1, &scaled_texture);
    _glBindTexture(GL_TEXTURE_2D, scaled_texture);
    _glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, scaled_w, scaled_h, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, NULL);

    return true;
}

// Import screen data from the given pointer and palette and update
// the unscaled_texture texture.
static void SetInputData(byte *screen, SDL_Color *palette)
{
    SDL_Color *c;
    byte *s;
    unsigned int i;

    // TODO: Maybe support GL_RGB as well as GL_RGBA?
    s = (byte *) unscaled_data;
    for (i = 0; i < SCREENWIDTH * SCREENHEIGHT; ++i)
    {
        c = &palette[screen[i]];
        *s++ = c->r;
        *s++ = c->g;
        *s++ = c->b;
        *s++ = 0xff;
    }

    _glBindTexture(GL_TEXTURE_2D, unscaled_texture);
    _glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, SCREENWIDTH, SCREENHEIGHT, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, unscaled_data);
}

// Draw unscaled_texture (containing the screen buffer) into the
// second scaled_texture texture.
static void DrawUnscaledToScaled(void)
{
    GLenum DrawBuffers[1] = {GL_COLOR_ATTACHMENT0};

    // Render unscaled texture into scaled texture:
    _glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                          scaled_texture, 0);
    _glDrawBuffers(1, DrawBuffers);

    if (_glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
	fprintf(stderr, "Failed to set up framebuffer\n");
        return;
    }

    _glBindFramebuffer(GL_FRAMEBUFFER, scaled_framebuffer);
    _glLoadIdentity();
    _glViewport(0, 0, scaled_w, scaled_h);
    _glBindTexture(GL_TEXTURE_2D, unscaled_texture);
    _glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    _glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    _glBegin(GL_QUADS);
    _glTexCoord2f(0, 1); _glVertex2f(-1, 1);
    _glTexCoord2f(1, 1); _glVertex2f(1, 1);
    _glTexCoord2f(1, 0); _glVertex2f(1, -1);
    _glTexCoord2f(0, 0); _glVertex2f(-1, -1);
    _glEnd();

    // Finished with framebuffer
    _glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

// Render the scaled_texture to the screen.
static void DrawScreen(void)
{
    GLfloat w, h;

    _glClear(GL_COLOR_BUFFER_BIT);

    _glMatrixMode(GL_MODELVIEW);
    _glLoadIdentity();
    _glViewport(0, 0, screen_w, screen_h);
    _glBindTexture(GL_TEXTURE_2D, scaled_texture);

    // Translate scaled-up texture to the screen with linear filtering.
    _glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    _glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    // Don't wrap/repeat the texture; this stops the linear filtering
    // from blurring the edges of the screen with each other.
    _glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    _glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

    w = (float) window_w / screen_w;
    h = (float) window_h / screen_h;

    _glBegin(GL_QUADS);
    _glTexCoord2f(0, 0); _glVertex2f(-w, h);
    _glTexCoord2f(1, 0); _glVertex2f(w, h);
    _glTexCoord2f(1, 1); _glVertex2f(w, -h);
    _glTexCoord2f(0, 1); _glVertex2f(-w, -h);
    _glEnd();
}

boolean I_GL_PreInit(void)
{
    if (SDL_GL_LoadLibrary(NULL) < 0)
    {
        return false;
    }

    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 5);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    return true;
}

boolean I_GL_InitScale(int w, int h)
{
    if (!SetGLFunctions())
    {
        return false;
    }

    _glEnable(GL_TEXTURE_2D);
    _glShadeModel(GL_SMOOTH);
    _glClearColor(0, 0, 0, 0);

    screen_w = w;
    screen_h = h;
    CalculateWindowSize();
    if (!CreateTextures())
    {
        return false;
    }

    return true;
}

void I_GL_UpdateScreen(byte *screendata, SDL_Color *palette)
{
    SetInputData(screendata, palette);
    DrawUnscaledToScaled();
    DrawScreen();
}

