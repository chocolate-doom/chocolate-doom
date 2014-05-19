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
//
// Text mode emulation in SDL
//

#ifndef TXT_SDL_H
#define TXT_SDL_H

// The textscreen API itself doesn't need SDL; however, SDL needs its
// headers included where main() is defined.

#include "SDL.h"

// Event callback function type: a function of this type can be used
// to intercept events in the textscreen event processing loop.  
// Returning 1 will cause the event to be eaten; the textscreen code
// will not see it.

typedef int (*TxtSDLEventCallbackFunc)(SDL_Event *event, void *user_data);

// Set a callback function to call in the SDL event loop.  Useful for
// intercepting events.  Pass callback=NULL to clear an existing
// callback function.
// user_data is a void pointer to be passed to the callback function.

void TXT_SDL_SetEventCallback(TxtSDLEventCallbackFunc callback, void *user_data);

#endif /* #ifndef TXT_SDL_H */

