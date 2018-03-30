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
// Routines for selecting files.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "doomkeys.h"

#include "txt_fileselect.h"
#include "txt_inputbox.h"
#include "txt_gui.h"
#include "txt_io.h"
#include "txt_main.h"
#include "txt_widget.h"

struct txt_fileselect_s {
    txt_widget_t widget;
    txt_inputbox_t *inputbox;
    int size;
    const char *prompt;
    const char **extensions;
};

// Dummy value to select a directory.

const char *TXT_DIRECTORY[] = { "__directory__", NULL };

#ifndef _WIN32

#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>

static char *ExecReadOutput(char **argv)
{
    char *result;
    int completed;
    int pid, status, result_len;
    int pipefd[2];

    if (pipe(pipefd) != 0)
    {
        return NULL;
    }

    pid = fork();

    if (pid == 0)
    {
        dup2(pipefd[1], fileno(stdout));
        execv(argv[0], argv);
        exit(-1);
    }

    fcntl(pipefd[0], F_SETFL, O_NONBLOCK);

    // Read program output into 'result' string.
    // Wait until the program has completed and (if it was successful)
    // a full line has been read.

    result = NULL;
    result_len = 0;
    completed = 0;

    while (!completed
        || (status == 0 && (result == NULL || strchr(result, '\n') == NULL)))
    {
        char buf[64];
        int bytes;

        if (!completed && waitpid(pid, &status, WNOHANG) != 0)
        {
            completed = 1;
        }

        bytes = read(pipefd[0], buf, sizeof(buf));

        if (bytes < 0)
        {
            if (errno != EAGAIN && errno != EWOULDBLOCK)
            {
                status = -1;
                break;
            }
        }
        else
        {
            char *new_result = realloc(result, result_len + bytes + 1);
            if (new_result == NULL)
            {
                break;
            }
            result = new_result;
            memcpy(result + result_len, buf, bytes);
            result_len += bytes;
            result[result_len] = '\0';
        }

        usleep(100 * 1000);
        TXT_Sleep(1);
        TXT_UpdateScreen();
    }

    close(pipefd[0]);
    close(pipefd[1]);

    // Must have a success exit code.

    if (WEXITSTATUS(status) != 0)
    {
        free(result);
        result = NULL;
    }

    // Strip off newline from the end.

    if (result != NULL && result[result_len - 1] == '\n')
    {
        result[result_len - 1] = '\0';
    }

    return result;
}

#endif

// This is currently disabled on Windows because it doesn't work.
// Current issues:
//   * On Windows Vista+ the mouse cursor freezes when the dialog is
//     opened. This is probably some conflict with SDL (might be
//     resolved by opening the dialog in a separate thread so that
//     TXT_UpdateScreen can be run in the background).
//   * On Windows XP the program exits/crashes when the dialog is
//     closed.
#if defined(_WIN32)

int TXT_CanSelectFiles(void)
{
    return 0;
}

char *TXT_SelectFile(const char *window_title, const char **extensions)
{
    return NULL;
}

#elif defined(xxxdisabled_WIN32)

// Windows code. Use comdlg32 to pop up a dialog box.

#include <windows.h>
#include <shlobj.h>

static BOOL (*MyGetOpenFileName)(LPOPENFILENAME) = NULL;
static LPITEMIDLIST (*MySHBrowseForFolder)(LPBROWSEINFO) = NULL;
static BOOL (*MySHGetPathFromIDList)(LPITEMIDLIST, LPTSTR) = NULL;

// Load library functions from DLL files.

static int LoadDLLs(void)
{
    HMODULE comdlg32 = LoadLibraryW(L"comdlg32.dll");
    HMODULE shell32 = LoadLibraryW(L"shell32.dll");

    if (comdlg32 == NULL || shell32 == NULL)
    {
        return 0;
    }

    MyGetOpenFileName =
        (void *) GetProcAddress(comdlg32, "GetOpenFileNameA");
    MySHBrowseForFolder =
        (void *) GetProcAddress(shell32, "SHBrowseForFolder");
    MySHGetPathFromIDList =
        (void *) GetProcAddress(shell32, "SHGetPathFromIDList");

    return MyGetOpenFileName != NULL
        && MySHBrowseForFolder != NULL
        && MySHGetPathFromIDList != NULL;
}

static int InitLibraries(void)
{
    static int initted = 0, success = 0;

    if (!initted)
    {
        success = LoadDLLs();
        initted = 1;
    }

    return success;
}

// Generate the "filter" string from the list of extensions.

static char *GenerateFilterString(const char **extensions)
{
    unsigned int result_len = 1;
    unsigned int i;
    char *result, *out;
    size_t out_len, offset;

    if (extensions == NULL)
    {
        return NULL;
    }

    for (i = 0; extensions[i] != NULL; ++i)
    {
        result_len += 16 + strlen(extensions[i]) * 3;
    }

    result = malloc(result_len);
    out = result; out_len = result_len;

    for (i = 0; extensions[i] != NULL; ++i)
    {
        // .wad files (*.wad)\0
        offset = TXT_snprintf(out, out_len, "%s files (*.%s)",
                              extensions[i], extensions[i]);
        out += offset + 1; out_len -= offset + 1;

        // *.wad\0
        offset = TXT_snprintf(out, out_len, "*.%s", extensions[i]);
        out_len += offset + 1; out_len -= offset + 1;
    }

    *out = '\0';

    return result;
}

int TXT_CanSelectFiles(void)
{
    return InitLibraries();
}

static char *SelectDirectory(char *window_title)
{
    LPITEMIDLIST pidl;
    BROWSEINFO bi;
    char selected[MAX_PATH] = "";
    char *result;

    ZeroMemory(&bi, sizeof(bi));
    bi.hwndOwner = NULL;
    bi.lpszTitle = window_title;
    bi.pszDisplayName = selected;
    bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;

    pidl = MySHBrowseForFolder(&bi);

    result = NULL;

    if (pidl != NULL)
    {
        if (MySHGetPathFromIDList(pidl, selected))
        {
            result = strdup(selected);
        }

        // TODO: Free pidl
    }

    return result;
}

char *TXT_SelectFile(const char *window_title, const char **extensions)
{
    OPENFILENAME fm;
    char selected[MAX_PATH] = "";
    char *filter_string, *result;

    if (!InitLibraries())
    {
        return NULL;
    }

    if (extensions == TXT_DIRECTORY)
    {
        return SelectDirectory(window_title);
    }

    filter_string = GenerateFilterString(extensions);

    ZeroMemory(&fm, sizeof(fm));
    fm.lStructSize = sizeof(OPENFILENAME);
    fm.hwndOwner = NULL;
    fm.lpstrTitle = window_title;
    fm.lpstrFilter = filter_string;
    fm.lpstrFile = selected;
    fm.nMaxFile = MAX_PATH;
    fm.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
    fm.lpstrDefExt = "";

    if (!MyGetOpenFileName(&fm))
    {
        result = NULL;
    }
    else
    {
        result = strdup(selected);
    }

    free(filter_string);

    return result;
}

#elif defined(__MACOSX__)

// Mac OS X code. Popping up a dialog requires Objective C/Cocoa
// but we can get away with using AppleScript which avoids adding
// an Objective C dependency. This is rather silly.

// Printf format string for the "wrapper" portion of the AppleScript:
#define APPLESCRIPT_WRAPPER "copy POSIX path of (%s) to stdout"

static char *CreateEscapedString(const char *original)
{
    char *result;
    const char *in;
    char *out;

    // We need to take care not to overflow the buffer, so count exactly.
#define ESCAPED_CHARS "\"\\"
    size_t count_extras = 2;    // start counting the two quotes
    for (in = original; *in; ++in)
    {
        if (strchr(ESCAPED_CHARS, *in))
        {
            ++count_extras;
        }
    }

    result = malloc(strlen(original) + count_extras + 1);
    if (!result)
    {
        return NULL;
    }
    out = result;
    *out++ = '"';
    for (in = original; *in; ++in)
    {
        if (strchr(ESCAPED_CHARS, *in))
        {
            *out++ = '\\';
        }
        *out++ = *in;
    }
    *out++ = '"';
    *out = 0;

    return result;
#undef ESCAPED_CHARS
}

// Build list of extensions, like: {"wad","lmp","txt"}

static char *CreateExtensionsList(const char **extensions)
{
    char *result, *escaped;
    unsigned int result_len;
    unsigned int i;

    if (extensions == NULL)
    {
        return NULL;
    }

    result_len = 3;
    for (i = 0; extensions[i] != NULL; ++i)
    {
        result_len += 5 + strlen(extensions[i]) * 2;
    }

    result = malloc(result_len);
    if (!result)
    {
        return NULL;
    }
    TXT_StringCopy(result, "{", result_len);

    for (i = 0; extensions[i] != NULL; ++i)
    {
        escaped = CreateEscapedString(extensions[i]);
        if (!escaped)
        {
            free(result);
            return NULL;
        }
        TXT_StringConcat(result, escaped, result_len);
        free(escaped);

        if (extensions[i + 1] != NULL)
        {
            TXT_StringConcat(result, ",", result_len);
        }
    }

    TXT_StringConcat(result, "}", result_len);

    return result;
}

static char *GenerateSelector(const char *const window_title, const char **extensions)
{
    const char *chooser;
    char *ext_list = NULL;
    char *window_title_escaped = NULL;
    char *result = NULL;
    unsigned int result_len = 64;

    if (extensions == TXT_DIRECTORY)
    {
        chooser = "choose folder";
    }
    else
    {
        chooser = "choose file";
        ext_list = CreateExtensionsList(extensions);
        if (!ext_list)
        {
            return NULL;
        }
    }

    // Calculate size.

    if (window_title != NULL)
    {
        window_title_escaped = CreateEscapedString(window_title);
        if (!window_title_escaped)
        {
            free(ext_list);
            return NULL;
        }
        result_len += strlen(window_title_escaped);
    }
    if (ext_list != NULL)
    {
        result_len += strlen(ext_list);
    }

    result = malloc(result_len);
    if (!result)
    {
        free(window_title_escaped);
        free(ext_list);
        return NULL;
    }

    TXT_StringCopy(result, chooser, result_len);

    if (window_title_escaped != NULL)
    {
        TXT_StringConcat(result, " with prompt ", result_len);
        TXT_StringConcat(result, window_title_escaped, result_len);
    }

    if (ext_list != NULL)
    {
        TXT_StringConcat(result, " of type ", result_len);
        TXT_StringConcat(result, ext_list, result_len);
    }

    free(window_title_escaped);
    free(ext_list);
    return result;
}

static char *GenerateAppleScript(const char *window_title, const char **extensions)
{
    char *selector, *result;
    size_t result_len;

    selector = GenerateSelector(window_title, extensions);
    if (!selector)
    {
        return NULL;
    }

    result_len = strlen(APPLESCRIPT_WRAPPER) + strlen(selector);
    result = malloc(result_len);
    if (!result)
    {
        free(selector);
        return NULL;
    }

    TXT_snprintf(result, result_len, APPLESCRIPT_WRAPPER, selector);
    free(selector);

    return result;
}

int TXT_CanSelectFiles(void)
{
    return 1;
}

char *TXT_SelectFile(const char *window_title, const char **extensions)
{
    char *argv[4];
    char *result, *applescript;

    applescript = GenerateAppleScript(window_title, extensions);
    if (!applescript)
    {
        return NULL;
    }

    argv[0] = "/usr/bin/osascript";
    argv[1] = "-e";
    argv[2] = applescript;
    argv[3] = NULL;

    result = ExecReadOutput(argv);

    free(applescript);

    return result;
}

#else

// Linux version: invoke the Zenity command line program to pop up a
// dialog box. This avoids adding Gtk+ as a compile dependency.

#define ZENITY_BINARY "/usr/bin/zenity"

static unsigned int NumExtensions(const char **extensions)
{
    unsigned int result = 0;

    if (extensions != NULL)
    {
        for (result = 0; extensions[result] != NULL; ++result);
    }

    return result;
}

static int ZenityAvailable(void)
{
    return system(ZENITY_BINARY " --help >/dev/null 2>&1") == 0;
}

int TXT_CanSelectFiles(void)
{
    return ZenityAvailable();
}

//
// ExpandExtension
// given an extension (like wad)
// return a pointer to a string that is a case-insensitive
// pattern representation (like [Ww][Aa][Dd])
//
static char *ExpandExtension(const char *orig)
{
    int oldlen, newlen, i;
    char *c, *newext = NULL;

    oldlen = strlen(orig);
    newlen = oldlen * 4; // pathological case: 'w' => '[Ww]'
    newext = malloc(newlen+1);

    if (newext == NULL)
    {
        return NULL;
    }

    c = newext;
    for (i = 0; i < oldlen; ++i)
    {
        if (isalpha(orig[i]))
        {
            *c++ = '[';
            *c++ = tolower(orig[i]);
            *c++ = toupper(orig[i]);
            *c++ = ']';
        }
        else
        {
            *c++ = orig[i];
        }
    }
    *c = '\0';
    return newext;
}

char *TXT_SelectFile(const char *window_title, const char **extensions)
{
    unsigned int i;
    size_t len;
    char *result;
    char **argv;
    int argc;

    if (!ZenityAvailable())
    {
        return NULL;
    }

    argv = calloc(5 + NumExtensions(extensions), sizeof(char *));
    argv[0] = strdup(ZENITY_BINARY);
    argv[1] = strdup("--file-selection");
    argc = 2;

    if (window_title != NULL)
    {
        len = 10 + strlen(window_title);
        argv[argc] = malloc(len);
        TXT_snprintf(argv[argc], len, "--title=%s", window_title);
        ++argc;
    }

    if (extensions == TXT_DIRECTORY)
    {
        argv[argc] = strdup("--directory");
        ++argc;
    }
    else if (extensions != NULL)
    {
        for (i = 0; extensions[i] != NULL; ++i)
        {
            char * newext = ExpandExtension(extensions[i]);
            if (newext)
            {
                len = 30 + strlen(extensions[i]) + strlen(newext);
                argv[argc] = malloc(len);
                TXT_snprintf(argv[argc], len, "--file-filter=.%s | *.%s",
                             extensions[i], newext);
                ++argc;
                free(newext);
            }
        }

        argv[argc] = strdup("--file-filter=*.* | *.*");
        ++argc;
    }

    argv[argc] = NULL;

    result = ExecReadOutput(argv);

    for (i = 0; i < argc; ++i)
    {
        free(argv[i]);
    }

    free(argv);

    return result;
}

#endif

static void TXT_FileSelectSizeCalc(TXT_UNCAST_ARG(fileselect))
{
    TXT_CAST_ARG(txt_fileselect_t, fileselect);

    // Calculate widget size, but override the width to always
    // be the configured size.

    TXT_CalcWidgetSize(fileselect->inputbox);
    fileselect->widget.w = fileselect->size;
    fileselect->widget.h = fileselect->inputbox->widget.h;
}

static void TXT_FileSelectDrawer(TXT_UNCAST_ARG(fileselect))
{
    TXT_CAST_ARG(txt_fileselect_t, fileselect);

    // Input box widget inherits all the properties of the
    // file selector.

    fileselect->inputbox->widget.x = fileselect->widget.x + 2;
    fileselect->inputbox->widget.y = fileselect->widget.y;
    fileselect->inputbox->widget.w = fileselect->widget.w - 2;
    fileselect->inputbox->widget.h = fileselect->widget.h;

    // Triple bar symbol gives a distinguishing look to the file selector.
    TXT_DrawCodePageString("\xf0 ");
    TXT_BGColor(TXT_COLOR_BLACK, 0);
    TXT_DrawWidget(fileselect->inputbox);
}

static void TXT_FileSelectDestructor(TXT_UNCAST_ARG(fileselect))
{
    TXT_CAST_ARG(txt_fileselect_t, fileselect);

    TXT_DestroyWidget(fileselect->inputbox);
}

static int DoSelectFile(txt_fileselect_t *fileselect)
{
    char *path;
    char **var;

    if (TXT_CanSelectFiles())
    {
        path = TXT_SelectFile(fileselect->prompt,
                              fileselect->extensions);

        // Update inputbox variable.
        // If cancel was pressed (ie. NULL was returned by TXT_SelectFile)
        // then reset to empty string, not NULL).

        if (path == NULL)
        {
            path = strdup("");
        }

        var = fileselect->inputbox->value;
        free(*var);
        *var = path;
        return 1;
    }

    return 0;
}

static int TXT_FileSelectKeyPress(TXT_UNCAST_ARG(fileselect), int key)
{
    TXT_CAST_ARG(txt_fileselect_t, fileselect);

    // When the enter key is pressed, pop up a file selection dialog,
    // if file selectors work. Allow holding down 'alt' to override
    // use of the native file selector, so the user can just type a path.

    if (!fileselect->inputbox->editing
     && !TXT_GetModifierState(TXT_MOD_ALT)
     && key == KEY_ENTER)
    {
        if (DoSelectFile(fileselect))
        {
            return 1;
        }
    }

    return TXT_WidgetKeyPress(fileselect->inputbox, key);
}

static void TXT_FileSelectMousePress(TXT_UNCAST_ARG(fileselect),
                                     int x, int y, int b)
{
    TXT_CAST_ARG(txt_fileselect_t, fileselect);

    if (!fileselect->inputbox->editing
     && !TXT_GetModifierState(TXT_MOD_ALT)
     && b == TXT_MOUSE_LEFT)
    {
        if (DoSelectFile(fileselect))
        {
            return;
        }
    }

    TXT_WidgetMousePress(fileselect->inputbox, x, y, b);
}

static void TXT_FileSelectFocused(TXT_UNCAST_ARG(fileselect), int focused)
{
    TXT_CAST_ARG(txt_fileselect_t, fileselect);

    TXT_SetWidgetFocus(fileselect->inputbox, focused);
}

txt_widget_class_t txt_fileselect_class =
{
    TXT_AlwaysSelectable,
    TXT_FileSelectSizeCalc,
    TXT_FileSelectDrawer,
    TXT_FileSelectKeyPress,
    TXT_FileSelectDestructor,
    TXT_FileSelectMousePress,
    NULL,
    TXT_FileSelectFocused,
};

// If the (inner) inputbox widget is changed, emit a change to the
// outer (fileselect) widget.

static void InputBoxChanged(TXT_UNCAST_ARG(widget), TXT_UNCAST_ARG(fileselect))
{
    TXT_CAST_ARG(txt_fileselect_t, fileselect);

    TXT_EmitSignal(&fileselect->widget, "changed");
}

txt_fileselect_t *TXT_NewFileSelector(char **variable, int size,
                                      const char *prompt, const char **extensions)
{
    txt_fileselect_t *fileselect;

    fileselect = malloc(sizeof(txt_fileselect_t));
    TXT_InitWidget(fileselect, &txt_fileselect_class);
    fileselect->inputbox = TXT_NewInputBox(variable, 1024);
    fileselect->inputbox->widget.parent = &fileselect->widget;
    fileselect->size = size;
    fileselect->prompt = prompt;
    fileselect->extensions = extensions;

    TXT_SignalConnect(fileselect->inputbox, "changed",
                      InputBoxChanged, fileselect);

    return fileselect;
}

