//
// 03/10/2006 James Haley
//
// For this module only: 
// This code is public domain. No change sufficient enough to constitute a
// significant or original work has been made, and thus it remains as such.
//
//
// DESCRIPTION:
//
// Implementation of POSIX opendir for Visual C++.
// Derived from the MinGW C Library Extensions Source (released to the
//  public domain).
//

#ifndef I_OPNDIR_H__
#define I_OPNDIR_H__

#include <io.h>

#ifndef FILENAME_MAX
#define FILENAME_MAX 260
#endif

struct dirent
{
   long		  d_ino;    /* Always zero. */
   unsigned short d_reclen; /* Always zero. */
   unsigned short d_namlen; /* Length of name in d_name. */
   char           d_name[FILENAME_MAX]; /* File name. */
};

/*
 * This is an internal data structure. Good programmers will not use it
 * except as an argument to one of the functions below.
 * dd_stat field is now int (was short in older versions).
 */
typedef struct
{
   /* disk transfer area for this dir */
   struct _finddata_t dd_dta;

   /* dirent struct to return from dir (NOTE: this makes this thread
    * safe as long as only one thread uses a particular DIR struct at
    * a time) */
   struct dirent dd_dir;

   /* _findnext handle */
   long	dd_handle;

   /*
    * Status of search:
    *   0 = not started yet (next entry to read is first entry)
    *  -1 = off the end
    *   positive = 0 based index of next entry
    */
   int dd_stat;

   /* given path for dir with search pattern (struct is extended) */
   char dd_name[1];
} DIR;

DIR *opendir(const char *);
struct dirent *readdir(DIR *);
int closedir(DIR *);
void rewinddir(DIR *);
long telldir(DIR *);
void seekdir(DIR *, long);

#endif

// EOF

