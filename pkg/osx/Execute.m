/* ... */
//-----------------------------------------------------------------------------
//
// Copyright(C) 2009 Simon Howard
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define RESPONSE_FILE "/tmp/launcher.rsp"

static char *executable_path;

// Called on startup to save the location of the launcher program
// (within a package, other executables should be in the same directory)

void SetProgramLocation(const char *path)
{
    char *p;

    executable_path = strdup(path);

    p = strrchr(executable_path, '/');
    *p = '\0';
}

// Write out the response file containing command line arguments.

static void WriteResponseFile(const char *iwad, const char *args)
{
    FILE *fstream;

    fstream = fopen(RESPONSE_FILE, "w");

    if (iwad != NULL)
    {
        fprintf(fstream, "-iwad \"%s\"", iwad);
    }

    if (args != NULL)
    {
        fprintf(fstream, "%s", args);
    }

    fclose(fstream);
}

static void DoExec(const char *executable, const char *iwad, const char *args)
{
    char *argv[3];

    argv[0] = malloc(strlen(executable_path) + strlen(executable) + 3);
    sprintf(argv[0], "%s/%s", executable_path, executable);

    if (iwad != NULL || args != NULL)
    {
        WriteResponseFile(iwad, args);

        argv[1] = "@" RESPONSE_FILE;
        argv[2] = NULL;
    }
    else
    {
        argv[1] = NULL;
    }

    execv(argv[0], argv);
    exit(-1);
}

// Execute the specified executable contained in the same directory
// as the launcher, with the specified arguments.

void ExecuteProgram(const char *executable, const char *iwad, const char *args)
{
    pid_t childpid;

    childpid = fork();

    if (childpid == 0)
    {
        signal(SIGCHLD, SIG_DFL);

        DoExec(executable, iwad, args);
    }
    else
    {
        signal(SIGCHLD, SIG_IGN);
    }
}

