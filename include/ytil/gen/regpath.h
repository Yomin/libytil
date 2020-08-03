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
    , E_REGPATH_MALFORMED
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
// create new regpath from base and str
regpath_ct regpath_new_base(regpath_base_id base, str_const_ct str);
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
// set str with base as regpath
regpath_ct regpath_set_base(regpath_ct path, regpath_base_id base, str_const_ct str);

// append str as new regpath components
regpath_ct regpath_append(regpath_ct path, str_const_ct str);

// drop n regpath components from end
regpath_ct regpath_drop(regpath_ct path, size_t n);

// get regpath as string
str_ct regpath_get(regpath_const_ct path);

#endif
