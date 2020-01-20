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

#ifndef YTIL_GEN_TYPE_H_INCLUDED
#define YTIL_GEN_TYPE_H_INCLUDED

#include <ytil/gen/error.h>
#include <stdbool.h>


/// type error
typedef enum type_error
{
    E_TYPE_INVALID      ///< invalid type ID
} type_error_id;

/// type error type declaration
ERROR_DECLARE(TYPE);

/// type ID
typedef enum typeid
{
    TYPE_INVALID,       ///< invalid type
    TYPE_BOOL,          ///< boolean type
    TYPE_CHAR,          ///< signed char type
    TYPE_UCHAR,         ///< unsigned char type
    TYPE_SHORT,         ///< signed short type
    TYPE_USHORT,        ///< unsigned short type
    TYPE_INT,           ///< signed int type
    TYPE_UINT,          ///< unsigned int type
    TYPE_LONG,          ///< signed long type
    TYPE_ULONG,         ///< unsigned long type
    TYPE_INT8,          ///< signed 8bit int type
    TYPE_UINT8,         ///< unsigned 8bit int type
    TYPE_INT16,         ///< signed 16bit int type
    TYPE_UINT16,        ///< unsigned 16bit int type
    TYPE_INT32,         ///< signed 32bit int type
    TYPE_UINT32,        ///< unsigned 32bit int type
    TYPE_INT64,         ///< signed 64bit int type
    TYPE_UINT64,        ///< unsigned 64bit int type
    TYPE_SIZE,          ///< size_t type
    TYPE_SSIZE,         ///< ssize_t type
    TYPE_FLOAT,         ///< float type
    TYPE_DOUBLE,        ///< double type
    TYPE_LDOUBLE,       ///< long double type
    TYPE_CSTRING,       ///< Cstring type
    TYPE_POINTER,       ///< pointer type
    TYPE_BASE_TYPES,    ///< number of base types
} type_id;


/// Free all registered types.
///
///
void types_free(void);

/// Register new type.
///
/// \param name     type name
///
/// \returns                            type ID
/// \retval TYPE_INVALID/E_GENERIC_OOM  out of memory
type_id type_new(const char *name);

/// Check if type is valid.
///
/// \param type     type ID
///
/// \retval true    type is valid
/// \retval false   type is not valid
bool type_is_valid(type_id type);

/// Get type name.
///
/// \param type     type ID
///
/// \returns                        type name
/// \retval NULL/E_TYPE_INVALID     invalid type
const char *type_name(type_id type);


#endif
