/*
 * Copyright (c) 2018-2021 Martin Rödel aka Yomin
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

/// \file

#include <ytil/gen/type.h>
#include <ytil/def.h>
#include <ytil/con/vec.h>


/// type
typedef struct type
{
    type_id     id;     ///< type ID
    const char  *name;  ///< type name
} type_st;

/// types state
static vec_ct types;

/// base type names
static const char *type_names[] =
{
    [TYPE_BOOL]     = "bool",
    [TYPE_CHAR]     = "char",
    [TYPE_UCHAR]    = "uchar",
    [TYPE_SHORT]    = "short",
    [TYPE_USHORT]   = "ushort",
    [TYPE_INT]      = "int",
    [TYPE_UINT]     = "uint",
    [TYPE_LONG]     = "long",
    [TYPE_ULONG]    = "ulong",
    [TYPE_INT8]     = "int8",
    [TYPE_UINT8]    = "uint8",
    [TYPE_INT16]    = "int16",
    [TYPE_UINT16]   = "uint16",
    [TYPE_INT32]    = "int32",
    [TYPE_UINT32]   = "uint32",
    [TYPE_INT64]    = "int64",
    [TYPE_UINT64]   = "uint64",
    [TYPE_SIZE]     = "size",
    [TYPE_SSIZE]    = "ssize",
    [TYPE_FLOAT]    = "float",
    [TYPE_DOUBLE]   = "double",
    [TYPE_LDOUBLE]  = "ldouble",
    [TYPE_CSTRING]  = "cstring",
    [TYPE_POINTER]  = "pointer",
};

/// type error type definition
ERROR_DEFINE_LIST(TYPE,
    ERROR_INFO(E_TYPE_INVALID, "Invalid type.")
);

/// default error type for type module
#define ERROR_TYPE_DEFAULT ERROR_TYPE_TYPE


void types_free(void)
{
    if(types)
    {
        vec_free(types);
        types = NULL;
    }
}

type_id type_new(const char *name)
{
    type_st *type;

    assert(name);

    if(!types && !(types = vec_new_c(2, sizeof(type_st))))
        return error_wrap(), TYPE_INVALID;

    if(!(type = vec_push(types)))
        return error_wrap(), TYPE_INVALID;

    type->name  = name;
    type->id    = TYPE_BASE_TYPES + vec_size(types) - 1;

    return type->id;
}

bool type_is_valid(type_id type)
{
    return type > TYPE_INVALID && (type < TYPE_BASE_TYPES
        || (types && type - TYPE_BASE_TYPES < vec_size(types)));
}

const char *type_name(type_id id)
{
    type_st *type;

    if(id == TYPE_INVALID)
        return error_set(E_TYPE_INVALID), NULL;

    if(id < TYPE_BASE_TYPES)
        return type_names[id];

    if(!types || !(type = vec_at(types, id - TYPE_BASE_TYPES)))
        return error_set(E_TYPE_INVALID), NULL;

    return type->name;
}
