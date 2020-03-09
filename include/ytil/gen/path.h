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

#ifndef __YTIL_GEN_PATH_H__
#define __YTIL_GEN_PATH_H__

#include <ytil/gen/str.h>
#include <stdbool.h>
#include <stddef.h>

typedef enum path_error
{
      E_PATH_INVALID_PROPERTIES
    , E_PATH_UNSUPPORTED
} path_error_id;

typedef struct path_prop
{
    const char *sep;        // path separators
    const char *current;    // current dir representation
    const char *parent;     // parent dir representation
    bool case_sensitive;    // case senitivity for path comparision
} path_prop_st;

struct path;
typedef struct path       *path_ct;
typedef const struct path *path_const_ct;


// create new path from str with path properties prop
path_ct path_new(str_const_ct str, const path_prop_st *prop);
// create new path from cstr with path properties prop
path_ct path_new_c(const char *str, const path_prop_st *prop);
// create new path from cstr of len with path properties prop
path_ct path_new_cn(const char *str, size_t len, const path_prop_st *prop);
// create new empty path
path_ct path_new_empty(void);
// create new 'current' path
path_ct path_new_current(void);
// create new 'parent' path
path_ct path_new_parent(void);
// duplicate path
path_ct path_dup(path_const_ct path);
// reset path to empty dir
void    path_reset(path_ct path);
// free path
void    path_free(path_ct path);

// check whether path is empty
bool path_is_empty(path_const_ct path);

// check if path1 equals path2
bool path_is_equal(path_const_ct path1, path_const_ct path2, const path_prop_st *prop);

// get count of path components
size_t  path_depth(path_const_ct path);
// get length of path
ssize_t path_len(path_const_ct path, const path_prop_st *prop);

// set str with path properties prop as path
path_ct path_set(path_ct path, str_const_ct str, const path_prop_st *prop);
// set cstr with path properties prop as path
path_ct path_set_c(path_ct path, const char *str, const path_prop_st *prop);
// set cstr of len with path properties prop as path
path_ct path_set_cn(path_ct path, const char *str, size_t len, const path_prop_st *prop);

// append str with path properties prop as new path components
path_ct path_append(path_ct path, str_const_ct str, const path_prop_st *prop);
// append cstr with path properties prop as new path components
path_ct path_append_c(path_ct path, const char *str, const path_prop_st *prop);
// append cstr of len with path properties prop as new path components
path_ct path_append_cn(path_ct path, const char *str, size_t len, const path_prop_st *prop);

// drop n path components from tail
path_ct path_drop(path_ct path, size_t n);

// get path as string with path properties prop
str_ct path_get(path_const_ct path, const path_prop_st *prop);

#endif
