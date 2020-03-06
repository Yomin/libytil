/*
 * Copyright (c) 2018-2020 Martin Rödel a.k.a. Yomin Nimoy
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

#ifndef __YTIL_GEN_FSPATH_H__
#define __YTIL_GEN_FSPATH_H__

#include <ytil/gen/path.h>

typedef enum fspath_error
{
      E_FSPATH_INVALID_DEVICE_NAME
    , E_FSPATH_INVALID_DRIVE_LETTER
    , E_FSPATH_INVALID_PATH
    , E_FSPATH_INVALID_SUFFIX
    , E_FSPATH_INVALID_TYPE
    , E_FSPATH_INVALID_UNC_HOST
    , E_FSPATH_INVALID_UNC_SHARE
    , E_FSPATH_MALFORMED
    , E_FSPATH_UNSUPPORTED
} fspath_error_id;

typedef enum fspath_type
{
      FSPATH_TYPE_STANDARD
    , FSPATH_TYPE_DRIVE
    , FSPATH_TYPE_UNC
    , FSPATH_TYPE_DEVICE
    , FSPATH_TYPES
} fspath_type_id;

typedef enum fspath_style
{
      FSPATH_STYLE_POSIX
    , FSPATH_STYLE_WINDOWS
    , FSPATH_STYLES
} fspath_style_id;

#ifdef _WIN32
#   define FSPATH_STYLE_NATIVE FSPATH_STYLE_WINDOWS
#else
#   define FSPATH_STYLE_NATIVE FSPATH_STYLE_POSIX
#endif

struct fspath;
typedef struct fspath       *fspath_ct;
typedef const struct fspath *fspath_const_ct;

/*
// create new path from str
fspath_ct fspath_new(str_const_ct str, fspath_style_id style);
// create new path from cstr
fspath_ct fspath_new_c(const char *str, fspath_style_id style);
// create new path from cstr of len
fspath_ct fspath_new_cn(const char *str, size_t len, fspath_style_id style);
// create new 'current' path
fspath_ct fspath_new_current(void);
// create new 'parent' path
fspath_ct fspath_new_parent(void);
// duplicate path
fspath_ct fspath_dup(fspath_const_ct path);
// reset path to 'current' dir
void    fspath_reset(fspath_ct path);
// free fspath
void    fspath_free(fspath_ct path);

// check whether path is absolute
bool path_is_absolute(path_const_ct path);
// check whether path is relative
bool path_is_relative(path_const_ct path);
// check whether path is directory (has trailing path separator)
bool path_is_directory(path_const_ct path);

// check if path1 equals path2
bool path_is_equal(path_const_ct path1, path_const_ct path2, path_style_id style);

// get path type
path_type_id path_type(path_const_ct path);

// get count of path components
size_t path_depth(path_const_ct path);
// get length of path
size_t path_len(path_const_ct path, path_style_id style);

// get 'current' directory in respective style
const char *path_current(path_style_id style);
// get 'parent' directory in respective style
const char *path_parent(path_style_id style);
// get path separators in respective style
const char *path_separator(path_style_id style);

// set str as path
path_ct path_set(path_ct path, str_const_ct str, path_style_id style);
// set cstr as path
path_ct path_set_c(path_ct path, const char *str, path_style_id style);
// set cstr of len as path
path_ct path_set_cn(path_ct path, const char *str, size_t len, path_style_id style);

// set drive letter for drive paths
path_ct path_set_drive(path_ct path, char letter);
// set host+share for UNC paths
path_ct path_set_unc(path_ct path, str_const_ct host, str_const_ct share);
// set share for UNC paths
path_ct path_set_unc_share(path_ct path, str_const_ct share);
// set name+id for device paths
path_ct path_set_device(path_ct path, str_const_ct name, size_t id);
// set id for device paths
path_ct path_set_device_ident(path_ct path, size_t id);

// replace suffix of last path component or add if not yet existing
path_ct path_set_suffix(path_ct path, str_const_ct suffix);
// add suffix to last path component, if a suffix already exists append suffix
path_ct path_add_suffix(path_ct path, str_const_ct suffix);

// append str as new path components
path_ct path_append(path_ct path, str_const_ct str, path_style_id style);
// append cstr as new path components
path_ct path_append_c(path_ct path, const char *str, path_style_id style);
// append cstr of len as new path components
path_ct path_append_cn(path_ct path, const char *str, size_t len, path_style_id style);

// get path as string
str_ct path_get(path_const_ct path, path_style_id style);

// get drive letter for drive paths
char path_get_drive_letter(path_const_ct path);
// get host for UNC paths
str_const_ct path_get_unc_host(path_const_ct path);
// get share for UNC paths
str_const_ct path_get_unc_share(path_const_ct path);
// get name for device paths
str_const_ct path_get_device_name(path_const_ct path);
// get id for device paths
ssize_t path_get_device_ident(path_const_ct path);

// get all path components but last
str_ct path_dirname(path_const_ct path, path_style_id style);
// get last path component
str_ct path_basename(path_const_ct path, path_style_id style);
*/
#endif
