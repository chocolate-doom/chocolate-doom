// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// Copyright(C) 2006 Simon Howard
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
//     Search for and locate an IWAD file, and initialize according
//     to the IWAD type.
//
//-----------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "deh_main.h"
#include "doomdef.h"
#include "doomstat.h"
#include "i_system.h"
#include "m_argv.h"
#include "m_config.h"
#include "m_misc.h"
#include "w_wad.h"
#include "z_zone.h"

// Array of locations to search for IWAD files
//
// "128 IWAD search directories should be enough for anybody".

#define MAX_IWAD_DIRS 128

static boolean iwad_dirs_built = false;
static char *iwad_dirs[MAX_IWAD_DIRS];
static int num_iwad_dirs = 0;

static void AddIWADDir(char *dir)
{
    if (num_iwad_dirs < MAX_IWAD_DIRS)
    {
        iwad_dirs[num_iwad_dirs] = dir;
        ++num_iwad_dirs;
    }
}

// This is Windows-specific code that automatically finds the location
// of installed IWAD files.  The registry is inspected to find special
// keys installed by the Windows installers for various CD versions
// of Doom.  From these keys we can deduce where to find an IWAD.

#if defined(_WIN32) && !defined(_WIN32_WCE)

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

typedef struct 
{
    HKEY root;
    char *path;
    char *value;
} registry_value_t;

#define UNINSTALLER_STRING "\\uninstl.exe /S "

// Keys installed by the various CD editions.  These are actually the 
// commands to invoke the uninstaller and look like this:
//
// C:\Program Files\Path\uninstl.exe /S C:\Program Files\Path
//
// With some munging we can find where Doom was installed.

static registry_value_t uninstall_values[] = 
{
    // Ultimate Doom, CD version (Depths of Doom trilogy)

    {
        HKEY_LOCAL_MACHINE, 
        "Software\\Microsoft\\Windows\\CurrentVersion\\"
            "Uninstall\\Ultimate Doom for Windows 95",
        "UninstallString",
    },

    // Doom II, CD version (Depths of Doom trilogy)

    {
        HKEY_LOCAL_MACHINE, 
        "Software\\Microsoft\\Windows\\CurrentVersion\\"
            "Uninstall\\Doom II for Windows 95",
        "UninstallString",
    },

    // Final Doom

    {
        HKEY_LOCAL_MACHINE, 
        "Software\\Microsoft\\Windows\\CurrentVersion\\"
            "Uninstall\\Final Doom for Windows 95",
        "UninstallString",
    },

    // Shareware version

    {
        HKEY_LOCAL_MACHINE, 
        "Software\\Microsoft\\Windows\\CurrentVersion\\"
            "Uninstall\\Doom Shareware for Windows 95",
        "UninstallString",
    },
};

// Value installed by the Collector's Edition when it is installed

static registry_value_t collectors_edition_value =
{
    HKEY_LOCAL_MACHINE,
    "Software\\Activision\\DOOM Collector's Edition\\v1.0",
    "INSTALLPATH",
};

// Subdirectories of the above install path, where IWADs are installed.

static char *collectors_edition_subdirs[] = 
{
    "Doom2",
    "Final Doom",
    "Ultimate Doom",
};

// Location where Steam is installed

static registry_value_t steam_install_location =
{
    HKEY_LOCAL_MACHINE,
    "Software\\Valve\\Steam",
    "InstallPath",
};

// Subdirs of the steam install directory where IWADs are found

static char *steam_install_subdirs[] =
{
    "steamapps\\common\\doom 2\\base",
    "steamapps\\common\\final doom\\base",
    "steamapps\\common\\ultimate doom\\base",
};

static char *GetRegistryString(registry_value_t *reg_val)
{
    HKEY key;
    DWORD len;
    DWORD valtype;
    char *result;

    // Open the key (directory where the value is stored)

    if (RegOpenKeyEx(reg_val->root, reg_val->path, 0, KEY_READ, &key) 
          != ERROR_SUCCESS)
    {
        return NULL;
    }

    // Find the type and length of the string

    if (RegQueryValueEx(key, reg_val->value, NULL, &valtype, NULL, &len) 
          != ERROR_SUCCESS)
    {
        return NULL;
    }

    // Only accept strings

    if (valtype != REG_SZ)
    {
        return NULL;
    }

    // Allocate a buffer for the value and read the value

    result = malloc(len);

    if (RegQueryValueEx(key, reg_val->value, NULL, &valtype, (unsigned char *) result, &len) 
          != ERROR_SUCCESS)
    {
        free(result);
        return NULL;
    }

    // Close the key
        
    RegCloseKey(key);

    return result;
}

// Check for the uninstall strings from the CD versions

static void CheckUninstallStrings(void)
{
    unsigned int i;

    for (i=0; i<arrlen(uninstall_values); ++i)
    {
        char *val;
        char *path;
        char *unstr;

        val = GetRegistryString(&uninstall_values[i]);

        if (val == NULL)
        {
            continue;
        }

        unstr = strstr(val, UNINSTALLER_STRING);

        if (unstr == NULL)
        {
            free(val);
        }
        else
        {
            path = unstr + strlen(UNINSTALLER_STRING);

            AddIWADDir(path);
        }
    }
}

// Check for Doom: Collector's Edition

static void CheckCollectorsEdition(void)
{
    char *install_path;
    char *subpath;
    unsigned int i;

    install_path = GetRegistryString(&collectors_edition_value);

    if (install_path == NULL)
    {
        return;
    }

    for (i=0; i<arrlen(collectors_edition_subdirs); ++i)
    {
        subpath = malloc(strlen(install_path)
                         + strlen(collectors_edition_subdirs[i])
                         + 5);

        sprintf(subpath, "%s\\%s", install_path, collectors_edition_subdirs[i]);

        AddIWADDir(subpath);
    }

    free(install_path);
}


// Check for Doom downloaded via Steam

static void CheckSteamEdition(void)
{
    char *install_path;
    char *subpath;
    size_t i;

    install_path = GetRegistryString(&steam_install_location);

    if (install_path == NULL)
    {
        return;
    }

    for (i=0; i<arrlen(steam_install_subdirs); ++i)
    {
        subpath = malloc(strlen(install_path) 
                         + strlen(steam_install_subdirs[i]) + 5);

        sprintf(subpath, "%s\\%s", install_path, steam_install_subdirs[i]);

        AddIWADDir(subpath);
    }
}

// Default install directories for DOS Doom

static void CheckDOSDefaults(void)
{
    // These are the default install directories used by the deice
    // installer program:

    AddIWADDir("\\doom2");              // Doom II
    AddIWADDir("\\plutonia");           // Final Doom
    AddIWADDir("\\tnt");
    AddIWADDir("\\doom_se");            // Ultimate Doom
    AddIWADDir("\\doom");               // Shareware / Registered Doom
    AddIWADDir("\\dooms");              // Shareware versions
    AddIWADDir("\\doomsw");
}

#endif

static struct 
{
    char *name;
    GameMission_t mission;
} iwads[] = {
    {"doom2.wad",    doom2},
    {"plutonia.wad", pack_plut},
    {"tnt.wad",      pack_tnt},
    {"doom.wad",     doom},
    {"doom1.wad",    doom},
    {"chex.wad",     doom},
    {"hacx.wad",     doom2},
};

// Hack for chex quest mode

static void CheckSpecialIWADs(char *iwad_name)
{
    if (!strcasecmp(iwad_name, "chex.wad"))
    {
        gameversion = exe_chex;
    }

    if (!strcasecmp(iwad_name, "hacx.wad"))
    {
        gameversion = exe_hacx;
    }
}

// Returns true if the specified path is a path to a file
// of the specified name.

static boolean DirIsFile(char *path, char *filename)
{
    size_t path_len;
    size_t filename_len;

    path_len = strlen(path);
    filename_len = strlen(filename);

    return path_len >= filename_len + 1
        && path[path_len - filename_len - 1] == DIR_SEPARATOR
        && !strcasecmp(&path[path_len - filename_len], filename);
}

// Check if the specified directory contains the specified IWAD
// file, returning the full path to the IWAD if found, or NULL
// if not found.

static char *CheckDirectoryHasIWAD(char *dir, char *iwadname)
{
    char *filename; 

    // As a special case, the "directory" may refer directly to an
    // IWAD file if the path comes from DOOMWADDIR or DOOMWADPATH.
    
    if (DirIsFile(dir, iwadname) && M_FileExists(dir))
    {
        return strdup(dir);
    }

    // Construct the full path to the IWAD if it is located in
    // this directory, and check if it exists.

    filename = malloc(strlen(dir) + strlen(iwadname) + 3);

    if (!strcmp(dir, "."))
    {
        strcpy(filename, iwadname);
    }
    else
    {
        sprintf(filename, "%s%c%s", dir, DIR_SEPARATOR, iwadname);
    }

    if (M_FileExists(filename))
    {
        return filename;
    }

    free(filename);

    return NULL;
}

// Search a directory to try to find an IWAD
// Returns the location of the IWAD if found, otherwise NULL.

static char *SearchDirectoryForIWAD(char *dir)
{
    char *filename;
    size_t i;

    for (i=0; i<arrlen(iwads); ++i) 
    {
        filename = CheckDirectoryHasIWAD(dir, DEH_String(iwads[i].name));

        if (filename != NULL)
        {
            CheckSpecialIWADs(iwads[i].name);
            gamemission = iwads[i].mission;

            return filename;
        }
    }

    return NULL;
}

// When given an IWAD with the '-iwad' parameter,
// attempt to identify it by its name.

static void IdentifyIWADByName(char *name)
{
    size_t i;

    gamemission = none;
    
    for (i=0; i<arrlen(iwads); ++i)
    {
        char *iwadname;

        iwadname = DEH_String(iwads[i].name);

        if (strlen(name) < strlen(iwadname))
            continue;

        // Check if it ends in this IWAD name.

        if (!strcasecmp(name + strlen(name) - strlen(iwadname), 
                        iwadname))
        {
            CheckSpecialIWADs(iwads[i].name);
            gamemission = iwads[i].mission;
            break;
        }
    }
}

//
// Add directories from the list in the DOOMWADPATH environment variable.
// 

static void AddDoomWadPath(void)
{
    char *doomwadpath;
    char *p;

    // Check the DOOMWADPATH environment variable.

    doomwadpath = getenv("DOOMWADPATH");

    if (doomwadpath == NULL)
    {
        return;
    }

    doomwadpath = strdup(doomwadpath);

    // Add the initial directory

    AddIWADDir(doomwadpath);

    // Split into individual dirs within the list.

    p = doomwadpath;

    for (;;)
    {
        p = strchr(p, PATH_SEPARATOR);

        if (p != NULL)
        {
            // Break at the separator and store the right hand side
            // as another iwad dir
  
            *p = '\0';
            p += 1;

            AddIWADDir(p);
        }
        else
        {
            break;
        }
    }
}


//
// Build a list of IWAD files
//

static void BuildIWADDirList(void)
{
    char *doomwaddir;

    if (iwad_dirs_built)
    {
        return;
    }

    // Look in the current directory.  Doom always does this.

    AddIWADDir(".");

    // Add DOOMWADDIR if it is in the environment

    doomwaddir = getenv("DOOMWADDIR");

    if (doomwaddir != NULL)
    {
        AddIWADDir(doomwaddir);
    }        

    // Add dirs from DOOMWADPATH

    AddDoomWadPath();

#if defined(_WIN32_WCE)

    // Windows CE locations:

    AddIWADDir("\\Storage Card");
    AddIWADDir(getenv("HOME"));

#elif defined(_WIN32) 

    // Search the registry and find where IWADs have been installed.

    CheckUninstallStrings();
    CheckCollectorsEdition();
    CheckSteamEdition();
    CheckDOSDefaults();

#else

    // Standard places where IWAD files are installed under Unix.

    AddIWADDir("/usr/share/games/doom");
    AddIWADDir("/usr/local/share/games/doom");

#endif

    // Don't run this function again.

    iwad_dirs_built = true;
}

//
// Searches WAD search paths for an WAD with a specific filename.
// 

char *D_FindWADByName(char *name)
{
    char *buf;
    int i;
    
    // Absolute path?

    if (M_FileExists(name))
    {
        return name;
    }

    BuildIWADDirList();
    
    // Search through all IWAD paths for a file with the given name.

    for (i=0; i<num_iwad_dirs; ++i)
    {
        // As a special case, if this is in DOOMWADDIR or DOOMWADPATH,
        // the "directory" may actually refer directly to an IWAD
        // file.

        if (DirIsFile(iwad_dirs[i], name) && M_FileExists(iwad_dirs[i]))
        {
            return strdup(iwad_dirs[i]);
        }

        // Construct a string for the full path

        buf = malloc(strlen(iwad_dirs[i]) + strlen(name) + 5);
        sprintf(buf, "%s%c%s", iwad_dirs[i], DIR_SEPARATOR, name);

        if (M_FileExists(buf))
        {
            return buf;
        }

        free(buf);
    }

    // File not found

    return NULL;
}

//
// D_TryWADByName
//
// Searches for a WAD by its filename, or passes through the filename
// if not found.
//

char *D_TryFindWADByName(char *filename)
{
    char *result;

    result = D_FindWADByName(filename);

    if (result != NULL)
    {
        return result;
    }
    else
    {
        return filename;
    }
}

//
// FindIWAD
// Checks availability of IWAD files by name,
// to determine whether registered/commercial features
// should be executed (notably loading PWADs).
//

char *D_FindIWAD(void)
{
    char *result;
    char *iwadfile;
    int iwadparm;
    int i;

    // Check for the -iwad parameter

    //!
    // Specify an IWAD file to use.
    //
    // @arg <file>
    //

    iwadparm = M_CheckParmWithArgs("-iwad", 1);

    if (iwadparm)
    {
        // Search through IWAD dirs for an IWAD with the given name.

        iwadfile = myargv[iwadparm + 1];

        result = D_FindWADByName(iwadfile);

        if (result == NULL)
        {
            I_Error("IWAD file '%s' not found!", iwadfile);
        }
        
        IdentifyIWADByName(result);
    }
    else
    {
        // Search through the list and look for an IWAD

        result = NULL;

        BuildIWADDirList();
    
        for (i=0; result == NULL && i<num_iwad_dirs; ++i)
        {
            result = SearchDirectoryForIWAD(iwad_dirs[i]);
        }
    }

    return result;
}

// 
// Get the IWAD name used for savegames.
//

static char *SaveGameIWADName(void)
{
    size_t i;

    // Chex quest hack

    if (gameversion == exe_chex)
    {
        return "chex.wad";
    }

    // Hacx hack

    if (gameversion == exe_hacx)
    {
        return "hacx.wad";
    }

    // Find what subdirectory to use for savegames
    //
    // They should be stored in something like
    //    ~/.chocolate-doom/savegames/doom.wad/
    //
    // The directory depends on the IWAD, so that savegames for
    // different IWADs are kept separate.
    //
    // Note that we match on gamemission rather than on IWAD name.
    // This ensures that doom1.wad and doom.wad saves are stored
    // in the same place.

    for (i=0; i<arrlen(iwads); ++i)
    {
        if (gamemission == iwads[i].mission)
        {
            return iwads[i].name;
        }
    }
    
    return NULL;
}
// 
// SetSaveGameDir
//
// Chooses the directory used to store saved games.
//

void D_SetSaveGameDir(void)
{
    char *iwad_name;

    if (!strcmp(configdir, ""))
    {
        // Use the current directory, just like configdir.

        savegamedir = strdup("");
    }
    else
    {
        // Directory for savegames

        iwad_name = SaveGameIWADName();

        if (iwad_name == NULL) 
        {
            iwad_name = "unknown.wad";
        }

        savegamedir = Z_Malloc(strlen(configdir) + 30, PU_STATIC, 0);
        sprintf(savegamedir, "%ssavegames%c", configdir,
                             DIR_SEPARATOR);

        M_MakeDirectory(savegamedir);

        sprintf(savegamedir + strlen(savegamedir), "%s%c",
                iwad_name, DIR_SEPARATOR);

        M_MakeDirectory(savegamedir);
    }
}

// Strings for dehacked replacements of the startup banner
//
// These are from the original source: some of them are perhaps
// not used in any dehacked patches

static char *banners[] = 
{
    // doom2.wad
    "                         "
    "DOOM 2: Hell on Earth v%i.%i"
    "                           ",
    // doom1.wad
    "                            "
    "DOOM Shareware Startup v%i.%i"
    "                           ",
    // doom.wad
    "                            "
    "DOOM Registered Startup v%i.%i"
    "                           ",
    // Registered DOOM uses this
    "                          "
    "DOOM System Startup v%i.%i"
    "                          ",
    // doom.wad (Ultimate DOOM)
    "                         "
    "The Ultimate DOOM Startup v%i.%i"
    "                        ",
    // tnt.wad
    "                     "
    "DOOM 2: TNT - Evilution v%i.%i"
    "                           ",
    // plutonia.wad
    "                   "
    "DOOM 2: Plutonia Experiment v%i.%i"
    "                           ",
};

//
// Get game name: if the startup banner has been replaced, use that.
// Otherwise, use the name given
// 

static char *GetGameName(char *gamename)
{
    size_t i;
    char *deh_sub;

    for (i=0; i<arrlen(banners); ++i)
    {
        // Has the banner been replaced?

        deh_sub = DEH_String(banners[i]);

        if (deh_sub != banners[i])
        {
            // Has been replaced
            // We need to expand via printf to include the Doom version 
            // number
            // We also need to cut off spaces to get the basic name

            gamename = Z_Malloc(strlen(deh_sub) + 10, PU_STATIC, 0);
            sprintf(gamename, deh_sub, DOOM_VERSION / 100, DOOM_VERSION % 100);

            while (gamename[0] != '\0' && isspace(gamename[0]))
                strcpy(gamename, gamename+1);

            while (gamename[0] != '\0' && isspace(gamename[strlen(gamename)-1]))
                gamename[strlen(gamename) - 1] = '\0';
            
            return gamename;
        }
    }

    return gamename;
}


//
// Find out what version of Doom is playing.
//

void D_IdentifyVersion(void)
{
    // gamemission is set up by the D_FindIWAD function.  But if 
    // we specify '-iwad', we have to identify using 
    // IdentifyIWADByName.  However, if the iwad does not match
    // any known IWAD name, we may have a dilemma.  Try to 
    // identify by its contents.

    if (gamemission == none)
    {
        unsigned int i;

        for (i=0; i<numlumps; ++i)
        {
            if (!strncasecmp(lumpinfo[i].name, "MAP01", 8))
            {
                gamemission = doom2;
                break;
            } 
            else if (!strncasecmp(lumpinfo[i].name, "E1M1", 8))
            {
                gamemission = doom;
                break;
            }
        }

        if (gamemission == none)
        {
            // Still no idea.  I don't think this is going to work.

            I_Error("Unknown or invalid IWAD file.");
        }
    }

    // Make sure gamemode is set up correctly

    if (gamemission == doom)
    {
        // Doom 1.  But which version?

        if (W_CheckNumForName("E4M1") > 0)
        {
            // Ultimate Doom

            gamemode = retail;
        } 
        else if (W_CheckNumForName("E3M1") > 0)
        {
            gamemode = registered;
        }
        else
        {
            gamemode = shareware;
        }
    }
    else
    {
        // Doom 2 of some kind.

        gamemode = commercial;
    }
}

// Set the gamedescription string

void D_SetGameDescription(void)
{
    gamedescription = "Unknown";

    if (gamemission == doom)
    {
        // Doom 1.  But which version?

        if (gamemode == retail)
        {
            // Ultimate Doom

            gamedescription = GetGameName("The Ultimate DOOM");
        } 
        else if (gamemode == registered)
        {
            gamedescription = GetGameName("DOOM Registered");
        }
        else if (gamemode == shareware)
        {
            gamedescription = GetGameName("DOOM Shareware");
        }
    }
    else
    {
        // Doom 2 of some kind.  But which mission?

        if (gamemission == doom2)
            gamedescription = GetGameName("DOOM 2: Hell on Earth");
        else if (gamemission == pack_plut)
            gamedescription = GetGameName("DOOM 2: Plutonia Experiment"); 
        else if (gamemission == pack_tnt)
            gamedescription = GetGameName("DOOM 2: TNT - Evilution");
    }
}

// Clever hack: Setup can invoke Doom to determine which IWADs are installed.
// Doom searches install paths and exits with the return code being a 
// bitmask of the installed IWAD files.

void D_FindInstalledIWADs(void)
{
    unsigned int i;
    int result;

    BuildIWADDirList();

    result = 0;

    for (i=0; i<arrlen(iwads); ++i)
    {
        if (D_FindWADByName(iwads[i].name) != NULL)
        {
            result |= 1 << i;
        }
    }

    exit(result);
}

