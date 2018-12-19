/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*  Chunk I/O
 *  =========
 *  Copyright 2018 Eduardo Silva <eduardo@monkey.io>
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */


#include <stdio.h>
#include <stdlib.h>
#include <chunkio/chunkio_compat.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#ifdef _WIN32
#include <share.h>
#include <strsafe.h>
#else
#include <fts.h>
#include <sys/mman.h>
#endif
#include <errno.h>

#include <chunkio/chunkio_compat.h>
#include <chunkio/cio_log.h>

#ifdef _WIN32
int cio_utils_recursive_delete(const char *dir)
{
    int ret;
    WCHAR szDir[PATH_MAX + 1];
    SHFILEOPSTRUCTW fileOperation = { 0 };
    ret = StringCchCopyW(szDir, PATH_MAX, dir);
    if (ret != S_OK) {
        return -1;
    }

    fileOperation.wFunc = FO_DELETE;
    fileOperation.pFrom = szDir;
    fileOperation.fFlags = FOF_NO_UI | FOF_NOCONFIRMATION;
    return SHFileOperation(&fileOperation);
}
#else
/*
 * Taken from StackOverflow:
 *
 * https://stackoverflow.com/questions/2256945/removing-a-non-empty-directory-programmatically-in-c-or-c
 */
int cio_utils_recursive_delete(const char *dir)
{
    int ret = 0;
    FTS *ftsp = NULL;
    FTSENT *curr;
    char *files[] = { (char *) dir, NULL };
    struct stat st;

    ret = stat(dir, &st);
    if (ret == -1) {
        return -1;
    }

    ftsp = fts_open(files, FTS_NOCHDIR | FTS_PHYSICAL | FTS_XDEV, NULL);
    if (!ftsp) {
        fprintf(stderr, "%s: fts_open failed: %s\n", dir, strerror(errno));
        ret = -1;
        goto finish;
    }

    while ((curr = fts_read(ftsp))) {
        switch (curr->fts_info) {
        case FTS_NS:
        case FTS_DNR:
        case FTS_ERR:
            fprintf(stderr, "%s: fts_read error: %s\n",
                    curr->fts_accpath, strerror(curr->fts_errno));
            break;
        case FTS_DC:
        case FTS_DOT:
        case FTS_NSOK:
            break;
        case FTS_D:
            break;
        case FTS_DP:
        case FTS_F:
        case FTS_SL:
        case FTS_SLNONE:
        case FTS_DEFAULT:
            if (remove(curr->fts_accpath) < 0) {
                fprintf(stderr, "%s: Failed to remove: %s\n",
                        curr->fts_path, strerror(errno));
                ret = -1;
            }
            break;
        }
    }

 finish:
    if (ftsp) {
        fts_close(ftsp);
    }

    return ret;
}
#endif

#ifdef _WIN32
int cio_utils_read_file(const char *path, char **buf, size_t *size, HANDLE *handle)
#else
int cio_utils_read_file(const char *path, char **buf, size_t *size)
#endif
{
    int fd;
    int ret;
    char *data;
    struct stat st;
#ifdef _WIN32
    HANDLE fmo;
#endif

    fd = open(path, O_RDONLY);
    if (fd == -1) {
        perror("open");
        return -1;
    }

    ret = fstat(fd, &st);
    if (ret == -1) {
        perror("fstat");
        close(fd);
        return -1;
    }
    if (!S_ISREG(st.st_mode)) {
        close(fd);
        return -1;
    }
#ifdef _WIN32
    fmo = CreateFileMapping(fd, NULL, PAGE_READONLY, 0, st.st_size, NULL);
    if (!fmo) {
        perror("CreateFileMapping");
        return -1;
    }
    data = MapViewOfFile(fmo, FILE_MAP_READ, 0, (DWORD)0, (SIZE_T)st.st_size);
    if (!data) {
        perror("MapViewOfFile");
        close(fd);
        return -1;
    }
#else
    data = mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (data == MAP_FAILED) {
        perror("mmap");
        close(fd);
        return -1;
    }
#endif
    close(fd);

    *buf = data;
    *size = st.st_size;
#ifdef _WIN32
    *handle = fmo;
#endif

    return 0;
}
