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
#include <stdbool.h>
#include <stddef.h>

typedef enum regpath_error
{
      E_REGPATH_INVALID_BASE
    , E_REGPATH_INVALID_PATH
} regpath_error_id;

typedef enum regpath_base
{
      REGPATH_CLASSES_ROOT
    , REGPATH_CURRENT_CONFIG
    , REGPATH_CURRENT_USER
    , REGPATH_DYN_DATA
    , REGPATH_LOCAL_MACHINE
    , REGPATH_PERFORMANCE_DATA
    , REGPATH_USERS
    , REGPATH_BASES
} regpath_base_id;

struct regpath;
typedef struct regpath       *regpath_ct;
typedef const struct regpath *regpath_const_ct;


// create new regpath from str
regpath_ct regpath_new(str_const_ct str);
// create new regpath from cstr
regpath_ct regpath_new_c(const char *str);
// create new regpath from cstr of len
regpath_ct regpath_new_cn(const char *str, size_t len);
// create new regpath from base and str
regpath_ct regpath_new_with_base(regpath_base_id base, str_const_ct str);
// create new regpath from base and cstr
regpath_ct regpath_new_with_base_c(regpath_base_id base, const char *str);
// create new regpath from base and cstr of len
regpath_ct regpath_new_with_base_cn(regpath_base_id base, const char *str, size_t len);
// duplicate regpath
regpath_ct regpath_dup(regpath_const_ct path);
// free regpath
void       regpath_free(regpath_ct path);

// check if regpath1 equals regpath2
bool regpath_is_equal(regpath_const_ct path1, regpath_const_ct path2);

// get regpath base
regpath_base_id regpath_base(regpath_const_ct path);
// get count of regpath components
size_t          regpath_depth(regpath_const_ct path);
// get length of regpath
size_t          regpath_len(regpath_const_ct path);

// set str as regpath
regpath_ct regpath_set(regpath_ct path, str_const_ct str);
// set cstr as regpath
regpath_ct regpath_set_c(regpath_ct path, const char *str);
// set cstr of len as regpath
regpath_ct regpath_set_cn(regpath_ct path, const char *str, size_t len);
// set str with base as regpath
regpath_ct regpath_set_with_base(regpath_ct path, regpath_base_id base, str_const_ct str);
// set cstr with base as regpath
regpath_ct regpath_set_with_base_c(regpath_ct path, regpath_base_id base, const char *str);
// set cstr of len with base as regpath
regpath_ct regpath_set_with_base_cn(regpath_ct path, regpath_base_id base, const char *str, size_t len);
/*
// set base for registry paths
path_ct path_set_registry_base(path_ct path, path_reg_base_id base);

// append str as new path components
path_ct path_append(path_ct path, str_const_ct str, path_style_id style);
// append cstr as new path components
path_ct path_append_c(path_ct path, const char *str, path_style_id style);
// append cstr of len as new path components
path_ct path_append_cn(path_ct path, const char *str, size_t len, path_style_id style);

// drop n path components from end
path_ct path_drop(path_ct path, size_t n);

// get path as string
str_ct path_get(path_const_ct path, path_style_id style);

// get base for registry paths
path_reg_base_id path_get_registry_base(path_const_ct path);

// get all path components but last
str_ct path_dirname(path_const_ct path, path_style_id style);
// get last path component
str_ct path_basename(path_const_ct path, path_style_id style);
*/
#endif
