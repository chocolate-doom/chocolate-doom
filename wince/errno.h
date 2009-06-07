//
// "Extension" implementation of errno.h for Windows CE.
//
// I (Simon Howard) release this file to the public domain.
//

#ifndef WINCE_ERRNO_H
#define WINCE_ERRNO_H

extern int *_GetErrno();

#define errno (*_GetErrno())

#endif /* #ifndef WINCE_ERROR_H */

