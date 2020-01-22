/*
 * Copyright (c) 2020 Martin Rödel aka Yomin
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

#ifndef __YTIL_ITF_ITF_H__
#define __YTIL_ITF_ITF_H__

#include <ytil/gen/type.h>
#include <ytil/gen/str.h>

typedef enum itf_error
{
      E_ITF_INVALID_INTERFACE
    , E_ITF_INVALID_NAME
    , E_ITF_INVALID_TYPE
    , E_ITF_REGISTERED
    , E_ITF_UNSUPPORTED
} itf_error_id;

typedef enum itfid
{
      ITF_INVALID
} itf_id;

typedef void (*itf_dtor_cb)(void *instance);


// free all interfaces
void itfs_free(void);

// create new interface
itf_id itf_new(str_const_ct name, itf_dtor_cb dtor);

// register new type for interface
int itf_register(itf_id itf, type_id type, void *instance);

// get interface name
str_const_ct itf_name(itf_id itf);

// get type specific interface
void *itf_get(itf_id itf, type_id type);

#endif
