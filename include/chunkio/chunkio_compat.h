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

#ifndef CHUNKIO_COMPAT_H
#define CHUNKIO_COMPAT_H

/* Windows compatibility utils */
#ifdef _WIN32
#  define PATH_MAX MAX_PATH
#  define ssize_t int
#  include <winsock2.h>
#  include <windows.h>
#  include <wchar.h>
#  include <io.h>
#  include <stdint.h>
#  include <stdlib.h>
#  define access _access
#  define W_OK 02 // Write permission.
#  define mode_t uint32_t
#  define mkdir(dir, mode) _mkdir(dir)
#else
#  include <unistd.h>
#endif

#endif