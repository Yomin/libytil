/*
 * Copyright (c) 2018-2020 Martin Rödel aka Yomin
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

#ifndef __YTIL_TYPE_TYPE_H__
#define __YTIL_TYPE_TYPE_H__

#include <stddef.h>
#include <stdbool.h>
#include <ytil/gen/str.h>

typedef enum type_error
{
      E_TYPE_INVALID_CAPABILITY
    , E_TYPE_INVALID_NAME
    , E_TYPE_INVALID_SIZE
    , E_TYPE_INVALID_TYPE
    , E_TYPE_UNSUPPORTED
} type_error_id;

typedef enum typeid
{
      TYPE_INVALID
    
    , TYPE_BOOL
    
    , TYPE_CHAR
    , TYPE_UCHAR
    , TYPE_INT
    , TYPE_UINT
    , TYPE_LONG
    , TYPE_ULONG
    
    , TYPE_INT8
    , TYPE_UINT8
    , TYPE_INT16
    , TYPE_UINT16
    , TYPE_INT32
    , TYPE_UINT32
    , TYPE_INT64
    , TYPE_UINT64
    
    , TYPE_ENUM
    , TYPE_SIZE
    , TYPE_SSIZE
    
    , TYPE_FLOAT
    , TYPE_DOUBLE
    
    , TYPE_CSTRING
    , TYPE_POINTER
    
    , TYPE_BASE_TYPES
} type_id;

typedef enum type_cap
{
      TYPE_CAP_NONE
    , TYPE_CAP_CHECK
    , TYPE_CAP_SHOW
    , TYPE_CAP_CMP
    , TYPE_CAP_EQUAL
    , TYPE_CAP_FREE
    , TYPE_CAPS
} type_cap_id;


typedef bool   (*type_check_cb)(type_id type, type_id *params, type_cap_id cap, void *ctx);
typedef size_t (*type_size_cb)(type_id type, type_id *params, void *ctx);
typedef str_ct (*type_show_cb)(str_ct str, type_id type, type_id *params, void *value, void *ctx);
typedef int    (*type_cmp_cb)(type_id type, type_id *params, void *v1, void *v2, void *ctx);
typedef bool   (*type_equal_cb)(type_id type, type_id *params, void *v1, void *v2, void *ctx);
typedef void   (*type_free_cb)(type_id type, type_id *params, void *value, void *ctx);
typedef void   (*type_dtor_cb)(type_id type, void *ctx);


void types_free(void);

type_id _type_register(str_const_ct name, size_t size, ...);
#define  type_register(name, size, ...) \
        _type_register(name, size, __VA_ARGS__ __VA_OPT__(,) TYPE_CAP_NONE)
type_id _type_register_x(str_const_ct name, size_t size, void *ctx, type_dtor_cb dtor, ...);
#define  type_register_x(name, size, ctx, dtor, ...) \
        _type_register_x(name, size, ctx, dtor, __VA_ARGS__ __VA_OPT__(,) TYPE_CAP_NONE)
/*
type_id type_register_p(str_const_ct name, size_t size, size_t params, size_t caps, ...);
type_id type_register_px(str_const_ct name, size_t size, void *ctx, type_dtor_cb dtor, size_t params, size_t caps, ...);
type_id type_register_pf(str_const_ct name, type_size_cb size, size_t params, size_t caps, ...);
type_id type_register_pfx(str_const_ct name, type_size_cb size, void *ctx, type_dtor_cb dtor, size_t params, size_t caps, ...);
*/

bool type_is_valid(type_id type);

const char *type_name(type_id type);
size_t      type_size(type_id type);
bool        type_check(type_id type, type_cap_id cap);
str_ct      type_show(type_id type, void *value);
int         type_cmp(type_id type, void *v1, void *v2);
bool        type_equal(type_id type, void *v1, void *v2);
void        type_free(type_id type, void *value);

#endif
