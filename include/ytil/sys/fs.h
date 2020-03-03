/*
 * Copyright (c) 2019-2020 Martin Rödel a.k.a. Yomin Nimoy
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef __YTIL_SYS_FS_H__
#define __YTIL_SYS_FS_H__

#include <ytil/gen/path.h>

typedef enum fs_error
{
      E_FS_ACCESS_DENIED
    , E_FS_BUSY
    , E_FS_CALLBACK
    , E_FS_ERRNO
    , E_FS_INVALID_PATH
    , E_FS_NO_SPACE
    , E_FS_NOT_DIRECTORY
    , E_FS_NOT_FOUND
} fs_error_id;

typedef enum fs_stat_flag
{
      FS_STAT_DEFAULT       = 0
    , FS_STAT_LINK_NOFOLLOW = 1 // do not follow symlinks
} fs_stat_fs;

typedef enum fs_move_flag
{
      FS_MOVE_DEFAULT   = 0
    , FS_MOVE_REPLACE   = 1
} fs_move_fs;

typedef enum fs_copy_flag
{
      FS_COPY_DEFAULT   = 0
    , FS_COPY_MERGE     = 1
} fs_copy_fs;

typedef enum fs_type
{
      FS_TYPE_UNKNOWN
    , FS_TYPE_BLOCK
    , FS_TYPE_CHARACTER
    , FS_TYPE_PIPE
    , FS_TYPE_SOCKET
    , FS_TYPE_LINK
    , FS_TYPE_REGULAR
    , FS_TYPE_DIRECTORY
} fs_type_id;

typedef enum fs_walk_type
{
      FS_WALK_FILE
    , FS_WALK_DIR_PRE
    , FS_WALK_DIR_POST
    , FS_WALK_DIR_ERROR
    , FS_WALK_STAT_ERROR
    , FS_WALK_TYPES
} fs_walk_type_id;

typedef struct fs_stat
{
    fs_type_id type;
    size_t size;
    short uid, gid;
    time_t atime, mtime, ctime;
} fs_stat_st;

typedef int (*fs_walk_cb)(fs_walk_type_id type, path_const_ct file, size_t depth, fs_stat_st *info, void *ctx);

// get file status
fs_stat_st *fs_stat(path_const_ct file, fs_stat_fs flags, fs_stat_st *fst);

// iterate over all files in directory
// negative depth recurses indefinitely
int fs_walk(path_const_ct dir, ssize_t depth, fs_stat_fs flags, fs_walk_cb walk, void *ctx);

// move file from src to dst
int fs_move(path_const_ct src, path_const_ct dst, fs_move_fs flags);
// copy file from src to dst
int fs_copy(path_const_ct src, path_const_ct dst, fs_copy_fs flags);
// remove file
int fs_remove(path_const_ct file);
// remove file, apply error cb if not removeable file is encountered
int fs_remove_f(path_const_ct file, fs_walk_cb error, void *ctx);

#endif
