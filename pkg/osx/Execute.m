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
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <AppKit/AppKit.h>

#include "config.h"

#define RESPONSE_FILE "/tmp/launcher.rsp"
#define TEMP_SCRIPT "/tmp/tempscript.sh"

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
    char *homedir;

    childpid = fork();

    if (childpid == 0)
    {
        signal(SIGCHLD, SIG_DFL);

        // Change directory to home dir before launch, so that any demos
        // are saved somewhere sensible.

        homedir = getenv("HOME");

        if (homedir != NULL)
        {
            chdir(homedir);
        }

        DoExec(executable, iwad, args);
    }
    else
    {
        signal(SIGCHLD, SIG_IGN);
    }
}

// Write a sequence of commands that will display the specified message
// via shell commands.

static void WriteMessage(FILE *script, char *msg)
{
    char *p;

    fprintf(script, "echo \"");

    for (p=msg; *p != '\0'; ++p)
    {
        // Start new line?

        if (*p == '\n')
        {
            fprintf(script, "\"\necho \"");
            continue;
        }

        // Escaped character?

        if (*p == '\\' || *p == '\"')
        {
            fprintf(script, "\\");
        }

        fprintf(script, "%c", *p);
    }

    fprintf(script, "\"\n");
}

// Open a terminal window with the PATH set appropriately, and DOOMWADPATH
// set to the specified value.

void OpenTerminalWindow(const char *doomwadpath)
{
    FILE *stream;

    // Generate a shell script that sets the PATH to include the location
    // where the Doom binaries are, and DOOMWADPATH to include the
    // IWAD files that have been configured in the launcher interface.
    // The script then deletes itself and starts a shell.

    stream = fopen(TEMP_SCRIPT, "w");

    fprintf(stream, "#!/bin/sh\n");
    //fprintf(stream, "set -x\n");
    fprintf(stream, "PATH=\"%s:$PATH\"\n", executable_path);

    // MANPATH is set to point to the directory within the bundle that
    // contains the Unix manpages.  However, the bundle name or path to
    // it can contain a space, and OS X doesn't like this!  As a
    // workaround, create a symlink in /tmp to point to the real directory,
    // and put *this* in MANPATH.

    fprintf(stream, "rm -f \"/tmp/%s.man\"\n", PACKAGE_TARNAME);
    fprintf(stream, "ln -s \"%s/man\" \"/tmp/%s.man\"\n",
                    executable_path, PACKAGE_TARNAME);
    fprintf(stream, "MANPATH=\"/tmp/%s.man:$(manpath)\"\n", PACKAGE_TARNAME);
    fprintf(stream, "export MANPATH\n");

    fprintf(stream, "DOOMWADPATH=\"%s\"\n", doomwadpath);
    fprintf(stream, "export DOOMWADPATH\n");
    fprintf(stream, "rm -f \"%s\"\n", TEMP_SCRIPT);

    // Display a useful message:

    fprintf(stream, "clear\n");
    WriteMessage(stream,
        "\n"
        "This command line has the PATH variable configured so that you may\n"
        "launch the game with whatever parameters you desire.\n"
        "\n"
        "For example:\n"
        "\n"
        "   " PACKAGE_TARNAME " -iwad doom2.wad -file sid.wad -warp 1\n"
        "\n"
        "Type 'exit' to exit.\n");

    fprintf(stream, "exec $SHELL\n");
    fprintf(stream, "\n");

    fclose(stream);

    chmod(TEMP_SCRIPT, 0755);

    // Tell the terminal to open a window to run the script.

    [[NSWorkspace sharedWorkspace] openFile: @TEMP_SCRIPT
                                   withApplication: @"Terminal"];
}

void OpenDocumentation(const char *filename)
{
    NSString *path;

    path = [NSString stringWithFormat: @"%s/Documentation/%s",
                     executable_path, filename];

    [[NSWorkspace sharedWorkspace] openFile: path];
}

