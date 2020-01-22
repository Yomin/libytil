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

#ifndef __YTIL_GEN_TYPE_H__
#define __YTIL_GEN_TYPE_H__

#include <ytil/gen/str.h>

typedef enum type_error
{
      E_TYPE_INVALID_NAME
    , E_TYPE_INVALID_TYPE
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
    
    , TYPE_SIZE
    , TYPE_SSIZE
    
    , TYPE_FLOAT
    , TYPE_DOUBLE
    
    , TYPE_CSTRING
    , TYPE_POINTER
    
    , TYPE_BASE_TYPES
} type_id;


// free all registered types
void types_free(void);

// register new type
type_id type_register(str_const_ct name);

// check if type exists
bool type_is_valid(type_id type);

// get type name
const char *type_name(type_id type);

#endif
