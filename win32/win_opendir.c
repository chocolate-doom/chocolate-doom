/*
MIT License
Copyright (c) 2019 win32ports
Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#ifdef _WIN32

#include "win_opendir.h"

#include <stdint.h>
#include <errno.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shlwapi.h>

#ifdef _MSC_VER
#pragma comment(lib, "Shlwapi.lib")
#endif

#ifndef NTFS_MAX_PATH
#define NTFS_MAX_PATH 32768
#endif /* NTFS_MAX_PATH */

#ifndef FSCTL_GET_REPARSE_POINT
#define FSCTL_GET_REPARSE_POINT 0x900a8
#endif /* FSCTL_GET_REPARSE_POINT */

#ifndef FILE_NAME_NORMALIZED
#define FILE_NAME_NORMALIZED 0
#endif /* FILE_NAME_NORMALIZED */

struct __dir
{
    struct dirent *entries;
    intptr_t fd;
    long int count;
    long int index;
};

int closedir(DIR *dirp)
{
    struct __dir *data = NULL;
    if (!dirp)
    {
        errno = EBADF;
        return -1;
    }
    data = (struct __dir *) dirp;
    CloseHandle((HANDLE) data->fd);
    free(data->entries);
    free(data);
    return 0;
}

static void __seterrno(int value)
{
#ifdef _MSC_VER
    _set_errno(value);
#else  /* _MSC_VER */
    errno = value;
#endif /* _MSC_VER */
}

static int __islink(const wchar_t *name, char *buffer)
{
    DWORD io_result = 0;
    DWORD bytes_returned = 0;
    HANDLE hFile = CreateFileW(
        name, 0, 0, NULL, OPEN_EXISTING,
        FILE_FLAG_OPEN_REPARSE_POINT | FILE_FLAG_BACKUP_SEMANTICS, 0);
    if (hFile == INVALID_HANDLE_VALUE)
    {
        return 0;
    }

    io_result = DeviceIoControl(hFile, FSCTL_GET_REPARSE_POINT, NULL, 0, buffer,
                                MAXIMUM_REPARSE_DATA_BUFFER_SIZE,
                                &bytes_returned, NULL);

    CloseHandle(hFile);

    if (io_result == 0)
    {
        return 0;
    }

    return ((REPARSE_GUID_DATA_BUFFER *) buffer)->ReparseTag ==
           IO_REPARSE_TAG_SYMLINK;
}

#pragma pack(push, 1)

typedef struct dirent_FILE_ID_128
{
    BYTE Identifier[16];
} dirent_FILE_ID_128;

typedef struct _dirent_FILE_ID_INFO
{
    ULONGLONG VolumeSerialNumber;
    dirent_FILE_ID_128 FileId;
} dirent_FILE_ID_INFO;

#pragma pack(pop)

typedef enum dirent_FILE_INFO_BY_HANDLE_CLASS
{
    dirent_FileIdInfo = 18
} dirent_FILE_INFO_BY_HANDLE_CLASS;

static __ino_t __inode(const wchar_t *name)
{
    __ino_t value = {0};
    BOOL result;
    dirent_FILE_ID_INFO fileid;
    BY_HANDLE_FILE_INFORMATION info;
    typedef BOOL(__stdcall * pfnGetFileInformationByHandleEx)(
        HANDLE hFile, dirent_FILE_INFO_BY_HANDLE_CLASS FileInformationClass,
        LPVOID lpFileInformation, DWORD dwBufferSize);
    pfnGetFileInformationByHandleEx fnGetFileInformationByHandleEx;
    HANDLE hFile;

    HANDLE hKernel32 = GetModuleHandleW(L"kernel32.dll");
    if (!hKernel32)
    {
        return value;
    }

    fnGetFileInformationByHandleEx =
        (pfnGetFileInformationByHandleEx) GetProcAddress(
            hKernel32, "GetFileInformationByHandleEx");
    if (!fnGetFileInformationByHandleEx)
    {
        return value;
    }

    hFile = CreateFileW(name, GENERIC_READ, FILE_SHARE_READ, NULL,
                        OPEN_EXISTING, 0, 0);
    if (hFile == INVALID_HANDLE_VALUE)
    {
        return value;
    }

    result = fnGetFileInformationByHandleEx(hFile, dirent_FileIdInfo, &fileid,
                                            sizeof(fileid));
    if (result)
    {
        value.serial = fileid.VolumeSerialNumber;
        memcpy(value.fileid, fileid.FileId.Identifier, 16);
    }
    else
    {
        result = GetFileInformationByHandle(hFile, &info);
        if (result)
        {
            value.serial = info.dwVolumeSerialNumber;
            memcpy(value.fileid + 8, &info.nFileIndexHigh, 4);
            memcpy(value.fileid + 12, &info.nFileIndexLow, 4);
        }
    }
    CloseHandle(hFile);
    return value;
}

static DIR *__internal_opendir(wchar_t *wname, int size)
{
    struct __dir *data = NULL;
    struct dirent *tmp_entries = NULL;
    static wchar_t *suffix = L"\\*.*";
    static int extra_prefix = 4; /* use prefix "\\?\" to handle long file names */
    static int extra_suffix = 4; /* use suffix "\*.*" to find everything */
    WIN32_FIND_DATAW w32fd = {0};
    HANDLE hFindFile = INVALID_HANDLE_VALUE;
    static int grow_factor = 2;
    char *buffer = NULL;

    BOOL relative = PathIsRelativeW(wname + extra_prefix);

    memcpy(wname + size - 1, suffix, sizeof(wchar_t) * extra_suffix);
    wname[size + extra_suffix - 1] = 0;

    if (relative)
    {
        wname += extra_prefix;
        size -= extra_prefix;
    }
    hFindFile = FindFirstFileW(wname, &w32fd);
    if (INVALID_HANDLE_VALUE == hFindFile)
    {
        __seterrno(ENOENT);
        return NULL;
    }

    data = (struct __dir *) malloc(sizeof(struct __dir));
    if (!data)
    {
        goto out_of_memory;
    }
    wname[size - 1] = 0;
    data->fd = (intptr_t) CreateFileW(wname, 0, 0, NULL, OPEN_EXISTING,
                                      FILE_FLAG_BACKUP_SEMANTICS, 0);
    wname[size - 1] = L'\\';
    data->count = 16;
    data->index = 0;
    data->entries = (struct dirent *) malloc(sizeof(struct dirent) * data->count);
    if (!data->entries)
    {
        goto out_of_memory;
    }
    buffer = malloc(MAXIMUM_REPARSE_DATA_BUFFER_SIZE);
    if (!buffer)
    {
        goto out_of_memory;
    }

    do
    {
        WideCharToMultiByte(CP_UTF8, 0, w32fd.cFileName, -1,
                            data->entries[data->index].d_name, NAME_MAX,
                            NULL, NULL);

        memcpy(wname + size, w32fd.cFileName, sizeof(wchar_t) * NAME_MAX);

        if (((w32fd.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) ==
             FILE_ATTRIBUTE_REPARSE_POINT)
            && __islink(wname, buffer))
        {
            data->entries[data->index].d_type = DT_LNK;
        }
        else if ((w32fd.dwFileAttributes & FILE_ATTRIBUTE_DEVICE) ==
                 FILE_ATTRIBUTE_DEVICE)
        {
            data->entries[data->index].d_type = DT_CHR;
        }
        else if ((w32fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ==
                 FILE_ATTRIBUTE_DIRECTORY)
        {
            data->entries[data->index].d_type = DT_DIR;
        }
        else
        {
            data->entries[data->index].d_type = DT_REG;
        }

        data->entries[data->index].d_ino = __inode(wname);
        data->entries[data->index].d_reclen = sizeof(struct dirent);
        data->entries[data->index].d_namelen =
            (unsigned char) wcslen(w32fd.cFileName);
        data->entries[data->index].d_off = 0;

        if (++data->index == data->count)
        {
            tmp_entries = (struct dirent *) realloc(
                data->entries,
                sizeof(struct dirent) * data->count * grow_factor);
            if (!tmp_entries)
            {
                goto out_of_memory;
            }
            data->entries = tmp_entries;
            data->count *= grow_factor;
        }
    } while (FindNextFileW(hFindFile, &w32fd) != 0);

    free(buffer);
    FindClose(hFindFile);

    data->count = data->index;
    data->index = 0;
    return (DIR *) data;

out_of_memory:
    if (data)
    {
        if (INVALID_HANDLE_VALUE != (HANDLE) data->fd)
        {
            CloseHandle((HANDLE) data->fd);
        }
        free(data->entries);
    }
    free(buffer);
    free(data);
    if (INVALID_HANDLE_VALUE != hFindFile)
    {
        FindClose(hFindFile);
    }
    __seterrno(ENOMEM);
    return NULL;
}

static wchar_t *__get_buffer()
{
    wchar_t *name = malloc(sizeof(wchar_t) * (NTFS_MAX_PATH + NAME_MAX + 8));
    if (name)
    {
        memcpy(name, L"\\\\?\\", sizeof(wchar_t) * 4);
    }
    return name;
}

DIR *opendir(const char *name)
{
    DIR *dirp = NULL;
    wchar_t *wname = __get_buffer();
    int size = 0;
    if (!wname)
    {
        errno = ENOMEM;
        return NULL;
    }
    size = MultiByteToWideChar(CP_UTF8, 0, name, -1, wname + 4, NTFS_MAX_PATH);
    if (0 == size)
    {
        free(wname);
        return NULL;
    }
    /* Ensure path has no trailing backslash before __internal_opendir. */
    /* Mind that size includes NULL, thus -2 for last character. */
    if (wname[size+4-2] == L'\\')
    {
        wname[size+4-2] = 0;
        size--;
    }
    dirp = __internal_opendir(wname, size + 4);
    free(wname);
    return dirp;
}

struct dirent *readdir(DIR *dirp)
{
    struct __dir *data = (struct __dir *) dirp;
    if (!data)
    {
        errno = EBADF;
        return NULL;
    }
    if (data->index < data->count)
    {
        return &data->entries[data->index++];
    }
    return NULL;
}

#endif /* _WIN32 */
