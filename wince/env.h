//
// "Extension" implementation of getenv for Windows CE.
//
// I (Simon Howard) release this file to the public domain.
//

#ifndef WINCE_ENV_H
#define WINCE_ENV_H

// SDL provides an implementation of getenv/putenv:

#include "SDL_getenv.h"

extern void PopulateEnvironment(void);

#endif /* #ifndef WINCE_ENV_H */

