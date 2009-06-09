//
// "Extension" implementation of ANSI C file functions for Windows CE.
//
// I (Simon Howard) release this file to the public domain.
//

#ifndef WINCE_FILEOPS_H
#define WINCE_FILEOPS_H

int remove(const char *pathname);
int rename(const char *oldpath, const char *newpath);

#endif /* #ifndef WINCE_FILEOPS_H */

