/*
 * Copyright (c) 2020 Martin Rödel a.k.a. Yomin Nimoy
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

#ifndef __YTIL_GEN_REGPATH_H__
#define __YTIL_GEN_REGPATH_H__

#include <ytil/gen/str.h>

typedef enum regpath_error
{
      E_REGPATH_INVALID_BASE
} regpath_error_id;

typedef enum regpath_base
{
      REGPATH_INVALID
    , REGPATH_CLASSES_ROOT
    , REGPATH_CURRENT_CONFIG
    , REGPATH_CURRENT_USER
    , REGPATH_LOCAL_MACHINE
    , REGPATH_USERS
    , REGPATH_BASES
} regpath_base_id;

struct regpath;
typedef struct regpath       *regpath_ct;
typedef const struct regpath *regpath_const_ct;


// create new regpath from str
regpath_ct regpath_new(regpath_base_id base, str_const_ct str);
// create new regpath from cstr
regpath_ct regpath_new_c(regpath_base_id base, const char *str);
// create new regpath from cstr of len
regpath_ct regpath_new_cn(regpath_base_id base, const char *str, size_t len);
// duplicate regpath
regpath_ct regpath_dup(regpath_const_ct path);
// free regpath
void       regpath_free(regpath_ct path);

// check if regpath1 equals regpath2
bool regpath_is_equal(regpath_const_ct path1, regpath_const_ct path2);

// get regpath base
regpath_base_id regpath_base(regpath_const_ct path);
/*
// get count of path components
size_t path_depth(path_const_ct path);
// get length of path
size_t path_len(path_const_ct path, path_style_id style);

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
// set base for registry paths
path_ct path_set_registry_base(path_ct path, path_reg_base_id base);

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

// drop n path components from end
path_ct path_drop(path_ct path, size_t n);

// remove suffix from last path component if existing
path_ct path_drop_suffix(path_ct path);

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
// get base for registry paths
path_reg_base_id path_get_registry_base(path_const_ct path);

// get suffix of last path component, empty str if none existing
str_ct path_get_suffix(path_const_ct path);

// get all path components but last
str_ct path_dirname(path_const_ct path, path_style_id style);
// get last path component
str_ct path_basename(path_const_ct path, path_style_id style);
*/
#endif
