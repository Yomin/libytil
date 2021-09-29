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

/// \file

#ifndef YTIL_DB_PARAM_H_INCLUDED
#define YTIL_DB_PARAM_H_INCLUDED

#include <ytil/db/db.h>


/// Get number of statement parameters.
///
/// \param stmt     statement
///
/// \returns                        number of statement parameters
/// \retval -1/E_DB_UNSUPPORTED     not supported by backend
ssize_t db_param_count(db_stmt_const_ct stmt);

/// Bind null parameter.
///
/// \param stmt     statement
/// \param index    parameter index
///
/// \retval 0                           success
/// \retval -1/E_DB_ILLEGAL             statement is being executed
/// \retval -1/E_DB_OUT_OF_BOUNDS       index is out of bounds
/// \retval -1/E_DB_UNSUPPORTED         bind not supported by backend
/// \retval -1/E_DB_UNSUPPORTED_TYPE    type not supported by backend
/// \retval -1/E_GENERIC_OOM            out of memory
int db_param_bind_null(db_stmt_ct stmt, size_t index);

/// Bind boolean parameter value.
///
/// \param stmt     statement
/// \param index    parameter index
/// \param value    boolean value
///
/// \retval 0                           success
/// \retval -1/E_DB_ILLEGAL             statement is being executed
/// \retval -1/E_DB_OUT_OF_BOUNDS       index is out of bounds
/// \retval -1/E_DB_UNSUPPORTED         bind not supported by backend
/// \retval -1/E_DB_UNSUPPORTED_TYPE    type not supported by backend
/// \retval -1/E_GENERIC_OOM            out of memory
int db_param_bind_bool(db_stmt_ct stmt, size_t index, bool value);

/// Bind boolean parameter reference.
///
/// \param stmt     statement
/// \param index    parameter index
/// \param value    boolean reference
/// \param is_null  if true bind NULL value, may be NULL
///
/// \retval 0                           success
/// \retval -1/E_DB_ILLEGAL             statement is being executed
/// \retval -1/E_DB_OUT_OF_BOUNDS       index is out of bounds
/// \retval -1/E_DB_UNSUPPORTED         bind not supported by backend
/// \retval -1/E_DB_UNSUPPORTED_MODE    bind mode not supported for type
/// \retval -1/E_DB_UNSUPPORTED_TYPE    type not supported by backend
/// \retval -1/E_GENERIC_OOM            out of memory
int db_param_bind_bool_ref(db_stmt_ct stmt, size_t index, const bool *value, const bool *is_null);

/// Bind signed char parameter value.
///
/// \param stmt     statement
/// \param index    parameter index
/// \param value    signed char value
///
/// \retval 0                           success
/// \retval -1/E_DB_ILLEGAL             statement is being executed
/// \retval -1/E_DB_OUT_OF_BOUNDS       index is out of bounds
/// \retval -1/E_DB_UNSUPPORTED         bind not supported by backend
/// \retval -1/E_DB_UNSUPPORTED_MODE    bind mode not supported for type
/// \retval -1/E_DB_UNSUPPORTED_TYPE    type not supported by backend
/// \retval -1/E_GENERIC_OOM            out of memory
int db_param_bind_char(db_stmt_ct stmt, size_t index, char value);

/// Bind signed char parameter reference.
///
/// \param stmt     statement
/// \param index    parameter index
/// \param value    signed char reference
/// \param is_null  if true bind NULL value, may be NULL
///
/// \retval 0                           success
/// \retval -1/E_DB_ILLEGAL             statement is being executed
/// \retval -1/E_DB_OUT_OF_BOUNDS       index is out of bounds
/// \retval -1/E_DB_UNSUPPORTED         bind not supported by backend
/// \retval -1/E_DB_UNSUPPORTED_MODE    bind mode not supported for type
/// \retval -1/E_DB_UNSUPPORTED_TYPE    type not supported by backend
/// \retval -1/E_GENERIC_OOM            out of memory
int db_param_bind_char_ref(db_stmt_ct stmt, size_t index, const char *value, const bool *is_null);

/// Bind signed short parameter value.
///
/// \param stmt     statement
/// \param index    parameter index
/// \param value    signed short value
///
/// \retval 0                           success
/// \retval -1/E_DB_ILLEGAL             statement is being executed
/// \retval -1/E_DB_OUT_OF_BOUNDS       index is out of bounds
/// \retval -1/E_DB_UNSUPPORTED         bind not supported by backend
/// \retval -1/E_DB_UNSUPPORTED_MODE    bind mode not supported for type
/// \retval -1/E_DB_UNSUPPORTED_TYPE    type not supported by backend
/// \retval -1/E_GENERIC_OOM            out of memory
int db_param_bind_short(db_stmt_ct stmt, size_t index, short value);

/// Bind signed short parameter reference.
///
/// \param stmt     statement
/// \param index    parameter index
/// \param value    signed short reference
/// \param is_null  if true bind NULL value, may be NULL
///
/// \retval 0                           success
/// \retval -1/E_DB_ILLEGAL             statement is being executed
/// \retval -1/E_DB_OUT_OF_BOUNDS       index is out of bounds
/// \retval -1/E_DB_UNSUPPORTED         bind not supported by backend
/// \retval -1/E_DB_UNSUPPORTED_MODE    bind mode not supported for type
/// \retval -1/E_DB_UNSUPPORTED_TYPE    type not supported by backend
/// \retval -1/E_GENERIC_OOM            out of memory
int db_param_bind_short_ref(db_stmt_ct stmt, size_t index, const short *value, const bool *is_null);

/// Bind signed integer parameter value.
///
/// \param stmt     statement
/// \param index    parameter index
/// \param value    signed integer value
///
/// \retval 0                           success
/// \retval -1/E_DB_ILLEGAL             statement is being executed
/// \retval -1/E_DB_OUT_OF_BOUNDS       index is out of bounds
/// \retval -1/E_DB_UNSUPPORTED         bind not supported by backend
/// \retval -1/E_DB_UNSUPPORTED_MODE    bind mode not supported for type
/// \retval -1/E_DB_UNSUPPORTED_TYPE    type not supported by backend
/// \retval -1/E_GENERIC_OOM            out of memory
int db_param_bind_int(db_stmt_ct stmt, size_t index, int value);

/// Bind signed integer parameter reference.
///
/// \param stmt     statement
/// \param index    parameter index
/// \param value    signed integer reference
/// \param is_null  if true bind NULL value, may be NULL
///
/// \retval 0                           success
/// \retval -1/E_DB_ILLEGAL             statement is being executed
/// \retval -1/E_DB_OUT_OF_BOUNDS       index is out of bounds
/// \retval -1/E_DB_UNSUPPORTED         bind not supported by backend
/// \retval -1/E_DB_UNSUPPORTED_MODE    bind mode not supported for type
/// \retval -1/E_DB_UNSUPPORTED_TYPE    type not supported by backend
/// \retval -1/E_GENERIC_OOM            out of memory
int db_param_bind_int_ref(db_stmt_ct stmt, size_t index, const int *value, const bool *is_null);

/// Bind signed long parameter value.
///
/// \param stmt     statement
/// \param index    parameter index
/// \param value    signed long value
///
/// \retval 0                           success
/// \retval -1/E_DB_ILLEGAL             statement is being executed
/// \retval -1/E_DB_OUT_OF_BOUNDS       index is out of bounds
/// \retval -1/E_DB_UNSUPPORTED         bind not supported by backend
/// \retval -1/E_DB_UNSUPPORTED_MODE    bind mode not supported for type
/// \retval -1/E_DB_UNSUPPORTED_TYPE    type not supported by backend
/// \retval -1/E_GENERIC_OOM            out of memory
int db_param_bind_long(db_stmt_ct stmt, size_t index, long value);

/// Bind signed long parameter reference.
///
/// \param stmt     statement
/// \param index    parameter index
/// \param value    signed long reference
/// \param is_null  if true bind NULL value, may be NULL
///
/// \retval 0                           success
/// \retval -1/E_DB_ILLEGAL             statement is being executed
/// \retval -1/E_DB_OUT_OF_BOUNDS       index is out of bounds
/// \retval -1/E_DB_UNSUPPORTED         bind not supported by backend
/// \retval -1/E_DB_UNSUPPORTED_MODE    bind mode not supported for type
/// \retval -1/E_DB_UNSUPPORTED_TYPE    type not supported by backend
/// \retval -1/E_GENERIC_OOM            out of memory
int db_param_bind_long_ref(db_stmt_ct stmt, size_t index, const long *value, const bool *is_null);

/// Bind signed longlong parameter value.
///
/// \param stmt     statement
/// \param index    parameter index
/// \param value    signed longlong value
///
/// \retval 0                           success
/// \retval -1/E_DB_ILLEGAL             statement is being executed
/// \retval -1/E_DB_OUT_OF_BOUNDS       index is out of bounds
/// \retval -1/E_DB_UNSUPPORTED         bind not supported by backend
/// \retval -1/E_DB_UNSUPPORTED_MODE    bind mode not supported for type
/// \retval -1/E_DB_UNSUPPORTED_TYPE    type not supported by backend
/// \retval -1/E_GENERIC_OOM            out of memory
int db_param_bind_longlong(db_stmt_ct stmt, size_t index, long long value);

/// Bind signed longlong parameter reference.
///
/// \param stmt     statement
/// \param index    parameter index
/// \param value    signed longlong reference
/// \param is_null  if true bind NULL value, may be NULL
///
/// \retval 0                           success
/// \retval -1/E_DB_ILLEGAL             statement is being executed
/// \retval -1/E_DB_OUT_OF_BOUNDS       index is out of bounds
/// \retval -1/E_DB_UNSUPPORTED         bind not supported by backend
/// \retval -1/E_DB_UNSUPPORTED_MODE    bind mode not supported for type
/// \retval -1/E_DB_UNSUPPORTED_TYPE    type not supported by backend
/// \retval -1/E_GENERIC_OOM            out of memory
int db_param_bind_longlong_ref(db_stmt_ct stmt, size_t index, const long long *value, const bool *is_null);

/// Bind 8 bit signed integer parameter value.
///
/// \param stmt     statement
/// \param index    parameter index
/// \param value    8 bit signed integer value
///
/// \retval 0                           success
/// \retval -1/E_DB_ILLEGAL             statement is being executed
/// \retval -1/E_DB_OUT_OF_BOUNDS       index is out of bounds
/// \retval -1/E_DB_UNSUPPORTED         bind not supported by backend
/// \retval -1/E_DB_UNSUPPORTED_MODE    bind mode not supported for type
/// \retval -1/E_DB_UNSUPPORTED_TYPE    type not supported by backend
/// \retval -1/E_GENERIC_OOM            out of memory
int db_param_bind_int8(db_stmt_ct stmt, size_t index, int8_t value);

/// Bind 8 bit signed integer parameter reference.
///
/// \param stmt     statement
/// \param index    parameter index
/// \param value    8 bit signed integer reference
/// \param is_null  if true bind NULL value, may be NULL
///
/// \retval 0                           success
/// \retval -1/E_DB_ILLEGAL             statement is being executed
/// \retval -1/E_DB_OUT_OF_BOUNDS       index is out of bounds
/// \retval -1/E_DB_UNSUPPORTED         bind not supported by backend
/// \retval -1/E_DB_UNSUPPORTED_MODE    bind mode not supported for type
/// \retval -1/E_DB_UNSUPPORTED_TYPE    type not supported by backend
/// \retval -1/E_GENERIC_OOM            out of memory
int db_param_bind_int8_ref(db_stmt_ct stmt, size_t index, const int8_t *value, const bool *is_null);

/// Bind 16 bit signed integer parameter value.
///
/// \param stmt     statement
/// \param index    parameter index
/// \param value    16 bit signed integer value
///
/// \retval 0                           success
/// \retval -1/E_DB_ILLEGAL             statement is being executed
/// \retval -1/E_DB_OUT_OF_BOUNDS       index is out of bounds
/// \retval -1/E_DB_UNSUPPORTED         bind not supported by backend
/// \retval -1/E_DB_UNSUPPORTED_MODE    bind mode not supported for type
/// \retval -1/E_DB_UNSUPPORTED_TYPE    type not supported by backend
/// \retval -1/E_GENERIC_OOM            out of memory
int db_param_bind_int16(db_stmt_ct stmt, size_t index, int16_t value);

/// Bind 16 bit signed integer parameter reference.
///
/// \param stmt     statement
/// \param index    parameter index
/// \param value    16 bit signed integer reference
/// \param is_null  if true bind NULL value, may be NULL
///
/// \retval 0                           success
/// \retval -1/E_DB_ILLEGAL             statement is being executed
/// \retval -1/E_DB_OUT_OF_BOUNDS       index is out of bounds
/// \retval -1/E_DB_UNSUPPORTED         bind not supported by backend
/// \retval -1/E_DB_UNSUPPORTED_MODE    bind mode not supported for type
/// \retval -1/E_DB_UNSUPPORTED_TYPE    type not supported by backend
/// \retval -1/E_GENERIC_OOM            out of memory
int db_param_bind_int16_ref(db_stmt_ct stmt, size_t index, const int16_t *value, const bool *is_null);

/// Bind 32 bit signed integer parameter value.
///
/// \param stmt     statement
/// \param index    parameter index
/// \param value    32 bit signed integer value
///
/// \retval 0                           success
/// \retval -1/E_DB_ILLEGAL             statement is being executed
/// \retval -1/E_DB_OUT_OF_BOUNDS       index is out of bounds
/// \retval -1/E_DB_UNSUPPORTED         bind not supported by backend
/// \retval -1/E_DB_UNSUPPORTED_MODE    bind mode not supported for type
/// \retval -1/E_DB_UNSUPPORTED_TYPE    type not supported by backend
/// \retval -1/E_GENERIC_OOM            out of memory
int db_param_bind_int32(db_stmt_ct stmt, size_t index, int32_t value);

/// Bind 32 bit signed integer parameter reference.
///
/// \param stmt     statement
/// \param index    parameter index
/// \param value    32 bit signed integer reference
/// \param is_null  if true bind NULL value, may be NULL
///
/// \retval 0                           success
/// \retval -1/E_DB_ILLEGAL             statement is being executed
/// \retval -1/E_DB_OUT_OF_BOUNDS       index is out of bounds
/// \retval -1/E_DB_UNSUPPORTED         bind not supported by backend
/// \retval -1/E_DB_UNSUPPORTED_MODE    bind mode not supported for type
/// \retval -1/E_DB_UNSUPPORTED_TYPE    type not supported by backend
/// \retval -1/E_GENERIC_OOM            out of memory
int db_param_bind_int32_ref(db_stmt_ct stmt, size_t index, const int32_t *value, const bool *is_null);

/// Bind 64 bit signed integer parameter value.
///
/// \param stmt     statement
/// \param index    parameter index
/// \param value    64 bit signed integer value
///
/// \retval 0                           success
/// \retval -1/E_DB_ILLEGAL             statement is being executed
/// \retval -1/E_DB_OUT_OF_BOUNDS       index is out of bounds
/// \retval -1/E_DB_UNSUPPORTED         bind not supported by backend
/// \retval -1/E_DB_UNSUPPORTED_MODE    bind mode not supported for type
/// \retval -1/E_DB_UNSUPPORTED_TYPE    type not supported by backend
/// \retval -1/E_GENERIC_OOM            out of memory
int db_param_bind_int64(db_stmt_ct stmt, size_t index, int64_t value);

/// Bind 64 bit signed integer parameter reference.
///
/// \param stmt     statement
/// \param index    parameter index
/// \param value    64 bit signed integer reference
/// \param is_null  if true bind NULL value, may be NULL
///
/// \retval 0                           success
/// \retval -1/E_DB_ILLEGAL             statement is being executed
/// \retval -1/E_DB_OUT_OF_BOUNDS       index is out of bounds
/// \retval -1/E_DB_UNSUPPORTED         bind not supported by backend
/// \retval -1/E_DB_UNSUPPORTED_MODE    bind mode not supported for type
/// \retval -1/E_DB_UNSUPPORTED_TYPE    type not supported by backend
/// \retval -1/E_GENERIC_OOM            out of memory
int db_param_bind_int64_ref(db_stmt_ct stmt, size_t index, const int64_t *value, const bool *is_null);

/// Bind unsigned char parameter value.
///
/// \param stmt     statement
/// \param index    parameter index
/// \param value    unsigned char value
///
/// \retval 0                           success
/// \retval -1/E_DB_ILLEGAL             statement is being executed
/// \retval -1/E_DB_OUT_OF_BOUNDS       index is out of bounds
/// \retval -1/E_DB_UNSUPPORTED         bind not supported by backend
/// \retval -1/E_DB_UNSUPPORTED_MODE    bind mode not supported for type
/// \retval -1/E_DB_UNSUPPORTED_TYPE    type not supported by backend
/// \retval -1/E_GENERIC_OOM            out of memory
int db_param_bind_uchar(db_stmt_ct stmt, size_t index, unsigned char value);

/// Bind unsigned char parameter reference.
///
/// \param stmt     statement
/// \param index    parameter index
/// \param value    unsigned char reference
/// \param is_null  if true bind NULL value, may be NULL
///
/// \retval 0                           success
/// \retval -1/E_DB_ILLEGAL             statement is being executed
/// \retval -1/E_DB_OUT_OF_BOUNDS       index is out of bounds
/// \retval -1/E_DB_UNSUPPORTED         bind not supported by backend
/// \retval -1/E_DB_UNSUPPORTED_MODE    bind mode not supported for type
/// \retval -1/E_DB_UNSUPPORTED_TYPE    type not supported by backend
/// \retval -1/E_GENERIC_OOM            out of memory
int db_param_bind_uchar_ref(db_stmt_ct stmt, size_t index, const unsigned char *value, const bool *is_null);

/// Bind unsigned short parameter value.
///
/// \param stmt     statement
/// \param index    parameter index
/// \param value    unsigned short value
///
/// \retval 0                           success
/// \retval -1/E_DB_ILLEGAL             statement is being executed
/// \retval -1/E_DB_OUT_OF_BOUNDS       index is out of bounds
/// \retval -1/E_DB_UNSUPPORTED         bind not supported by backend
/// \retval -1/E_DB_UNSUPPORTED_MODE    bind mode not supported for type
/// \retval -1/E_DB_UNSUPPORTED_TYPE    type not supported by backend
/// \retval -1/E_GENERIC_OOM            out of memory
int db_param_bind_ushort(db_stmt_ct stmt, size_t index, unsigned short value);

/// Bind unsigned short parameter reference.
///
/// \param stmt     statement
/// \param index    parameter index
/// \param value    unsigned short reference
/// \param is_null  if true bind NULL value, may be NULL
///
/// \retval 0                           success
/// \retval -1/E_DB_ILLEGAL             statement is being executed
/// \retval -1/E_DB_OUT_OF_BOUNDS       index is out of bounds
/// \retval -1/E_DB_UNSUPPORTED         bind not supported by backend
/// \retval -1/E_DB_UNSUPPORTED_MODE    bind mode not supported for type
/// \retval -1/E_DB_UNSUPPORTED_TYPE    type not supported by backend
/// \retval -1/E_GENERIC_OOM            out of memory
int db_param_bind_ushort_ref(db_stmt_ct stmt, size_t index, const unsigned short *value, const bool *is_null);

/// Bind unsigned integer parameter value.
///
/// \param stmt     statement
/// \param index    parameter index
/// \param value    unsigned integer value
///
/// \retval 0                           success
/// \retval -1/E_DB_ILLEGAL             statement is being executed
/// \retval -1/E_DB_OUT_OF_BOUNDS       index is out of bounds
/// \retval -1/E_DB_UNSUPPORTED         bind not supported by backend
/// \retval -1/E_DB_UNSUPPORTED_MODE    bind mode not supported for type
/// \retval -1/E_DB_UNSUPPORTED_TYPE    type not supported by backend
/// \retval -1/E_GENERIC_OOM            out of memory
int db_param_bind_uint(db_stmt_ct stmt, size_t index, unsigned int value);

/// Bind unsigned integer parameter reference.
///
/// \param stmt     statement
/// \param index    parameter index
/// \param value    unsigned integer reference
/// \param is_null  if true bind NULL value, may be NULL
///
/// \retval 0                           success
/// \retval -1/E_DB_ILLEGAL             statement is being executed
/// \retval -1/E_DB_OUT_OF_BOUNDS       index is out of bounds
/// \retval -1/E_DB_UNSUPPORTED         bind not supported by backend
/// \retval -1/E_DB_UNSUPPORTED_MODE    bind mode not supported for type
/// \retval -1/E_DB_UNSUPPORTED_TYPE    type not supported by backend
/// \retval -1/E_GENERIC_OOM            out of memory
int db_param_bind_uint_ref(db_stmt_ct stmt, size_t index, const unsigned int *value, const bool *is_null);

/// Bind unsigned long parameter value.
///
/// \param stmt     statement
/// \param index    parameter index
/// \param value    unsigned long value
///
/// \retval 0                           success
/// \retval -1/E_DB_ILLEGAL             statement is being executed
/// \retval -1/E_DB_OUT_OF_BOUNDS       index is out of bounds
/// \retval -1/E_DB_UNSUPPORTED         bind not supported by backend
/// \retval -1/E_DB_UNSUPPORTED_MODE    bind mode not supported for type
/// \retval -1/E_DB_UNSUPPORTED_TYPE    type not supported by backend
/// \retval -1/E_GENERIC_OOM            out of memory
int db_param_bind_ulong(db_stmt_ct stmt, size_t index, unsigned long value);

/// Bind unsigned long parameter reference.
///
/// \param stmt     statement
/// \param index    parameter index
/// \param value    unsigned long reference
/// \param is_null  if true bind NULL value, may be NULL
///
/// \retval 0                           success
/// \retval -1/E_DB_ILLEGAL             statement is being executed
/// \retval -1/E_DB_OUT_OF_BOUNDS       index is out of bounds
/// \retval -1/E_DB_UNSUPPORTED         bind not supported by backend
/// \retval -1/E_DB_UNSUPPORTED_MODE    bind mode not supported for type
/// \retval -1/E_DB_UNSUPPORTED_TYPE    type not supported by backend
/// \retval -1/E_GENERIC_OOM            out of memory
int db_param_bind_ulong_ref(db_stmt_ct stmt, size_t index, const unsigned long *value, const bool *is_null);

/// Bind unsigned longlong parameter value.
///
/// \param stmt     statement
/// \param index    parameter index
/// \param value    signed longlong value
///
/// \retval 0                           success
/// \retval -1/E_DB_ILLEGAL             statement is being executed
/// \retval -1/E_DB_OUT_OF_BOUNDS       index is out of bounds
/// \retval -1/E_DB_UNSUPPORTED         bind not supported by backend
/// \retval -1/E_DB_UNSUPPORTED_MODE    bind mode not supported for type
/// \retval -1/E_DB_UNSUPPORTED_TYPE    type not supported by backend
/// \retval -1/E_GENERIC_OOM            out of memory
int db_param_bind_ulonglong(db_stmt_ct stmt, size_t index, unsigned long long value);

/// Bind unsigned longlong parameter reference.
///
/// \param stmt     statement
/// \param index    parameter index
/// \param value    signed longlong reference
/// \param is_null  if true bind NULL value, may be NULL
///
/// \retval 0                           success
/// \retval -1/E_DB_ILLEGAL             statement is being executed
/// \retval -1/E_DB_OUT_OF_BOUNDS       index is out of bounds
/// \retval -1/E_DB_UNSUPPORTED         bind not supported by backend
/// \retval -1/E_DB_UNSUPPORTED_MODE    bind mode not supported for type
/// \retval -1/E_DB_UNSUPPORTED_TYPE    type not supported by backend
/// \retval -1/E_GENERIC_OOM            out of memory
int db_param_bind_ulonglong_ref(db_stmt_ct stmt, size_t index, const unsigned long long *value, const bool *is_null);

/// Bind 8 bit unsigned integer parameter value.
///
/// \param stmt     statement
/// \param index    parameter index
/// \param value    8 bit unsigned integer value
///
/// \retval 0                           success
/// \retval -1/E_DB_ILLEGAL             statement is being executed
/// \retval -1/E_DB_OUT_OF_BOUNDS       index is out of bounds
/// \retval -1/E_DB_UNSUPPORTED         bind not supported by backend
/// \retval -1/E_DB_UNSUPPORTED_MODE    bind mode not supported for type
/// \retval -1/E_DB_UNSUPPORTED_TYPE    type not supported by backend
/// \retval -1/E_GENERIC_OOM            out of memory
int db_param_bind_uint8(db_stmt_ct stmt, size_t index, uint8_t value);

/// Bind 8 bit unsigned integer parameter reference.
///
/// \param stmt     statement
/// \param index    parameter index
/// \param value    8 bit unsigned integer reference
/// \param is_null  if true bind NULL value, may be NULL
///
/// \retval 0                           success
/// \retval -1/E_DB_ILLEGAL             statement is being executed
/// \retval -1/E_DB_OUT_OF_BOUNDS       index is out of bounds
/// \retval -1/E_DB_UNSUPPORTED         bind not supported by backend
/// \retval -1/E_DB_UNSUPPORTED_MODE    bind mode not supported for type
/// \retval -1/E_DB_UNSUPPORTED_TYPE    type not supported by backend
/// \retval -1/E_GENERIC_OOM            out of memory
int db_param_bind_uint8_ref(db_stmt_ct stmt, size_t index, const uint8_t *value, const bool *is_null);

/// Bind 16 bit unsigned integer parameter value.
///
/// \param stmt     statement
/// \param index    parameter index
/// \param value    16 bit unsigned integer value
///
/// \retval 0                           success
/// \retval -1/E_DB_ILLEGAL             statement is being executed
/// \retval -1/E_DB_OUT_OF_BOUNDS       index is out of bounds
/// \retval -1/E_DB_UNSUPPORTED         bind not supported by backend
/// \retval -1/E_DB_UNSUPPORTED_MODE    bind mode not supported for type
/// \retval -1/E_DB_UNSUPPORTED_TYPE    type not supported by backend
/// \retval -1/E_GENERIC_OOM            out of memory
int db_param_bind_uint16(db_stmt_ct stmt, size_t index, uint16_t value);

/// Bind 16 bit unsigned integer parameter reference.
///
/// \param stmt     statement
/// \param index    parameter index
/// \param value    16 bit unsigned integer reference
/// \param is_null  if true bind NULL value, may be NULL
///
/// \retval 0                           success
/// \retval -1/E_DB_ILLEGAL             statement is being executed
/// \retval -1/E_DB_OUT_OF_BOUNDS       index is out of bounds
/// \retval -1/E_DB_UNSUPPORTED         bind not supported by backend
/// \retval -1/E_DB_UNSUPPORTED_MODE    bind mode not supported for type
/// \retval -1/E_DB_UNSUPPORTED_TYPE    type not supported by backend
/// \retval -1/E_GENERIC_OOM            out of memory
int db_param_bind_uint16_ref(db_stmt_ct stmt, size_t index, const uint16_t *value, const bool *is_null);

/// Bind 32 bit unsigned integer parameter value.
///
/// \param stmt     statement
/// \param index    parameter index
/// \param value    32 bit unsigned integer value
///
/// \retval 0                           success
/// \retval -1/E_DB_ILLEGAL             statement is being executed
/// \retval -1/E_DB_OUT_OF_BOUNDS       index is out of bounds
/// \retval -1/E_DB_UNSUPPORTED         bind not supported by backend
/// \retval -1/E_DB_UNSUPPORTED_MODE    bind mode not supported for type
/// \retval -1/E_DB_UNSUPPORTED_TYPE    type not supported by backend
/// \retval -1/E_GENERIC_OOM            out of memory
int db_param_bind_uint32(db_stmt_ct stmt, size_t index, uint32_t value);

/// Bind 32 bit unsigned integer parameter reference.
///
/// \param stmt     statement
/// \param index    parameter index
/// \param value    32 bit unsigned integer reference
/// \param is_null  if true bind NULL value, may be NULL
///
/// \retval 0                           success
/// \retval -1/E_DB_ILLEGAL             statement is being executed
/// \retval -1/E_DB_OUT_OF_BOUNDS       index is out of bounds
/// \retval -1/E_DB_UNSUPPORTED         bind not supported by backend
/// \retval -1/E_DB_UNSUPPORTED_MODE    bind mode not supported for type
/// \retval -1/E_DB_UNSUPPORTED_TYPE    type not supported by backend
/// \retval -1/E_GENERIC_OOM            out of memory
int db_param_bind_uint32_ref(db_stmt_ct stmt, size_t index, const uint32_t *value, const bool *is_null);

/// Bind 64 bit unsigned integer parameter value.
///
/// \param stmt     statement
/// \param index    parameter index
/// \param value    64 bit unsigned integer value
///
/// \retval 0                           success
/// \retval -1/E_DB_ILLEGAL             statement is being executed
/// \retval -1/E_DB_OUT_OF_BOUNDS       index is out of bounds
/// \retval -1/E_DB_UNSUPPORTED         bind not supported by backend
/// \retval -1/E_DB_UNSUPPORTED_MODE    bind mode not supported for type
/// \retval -1/E_DB_UNSUPPORTED_TYPE    type not supported by backend
/// \retval -1/E_GENERIC_OOM            out of memory
int db_param_bind_uint64(db_stmt_ct stmt, size_t index, uint64_t value);

/// Bind 64 bit unsigned integer parameter reference.
///
/// \param stmt     statement
/// \param index    parameter index
/// \param value    64 bit unsigned integer reference
/// \param is_null  if true bind NULL value, may be NULL
///
/// \retval 0                           success
/// \retval -1/E_DB_ILLEGAL             statement is being executed
/// \retval -1/E_DB_OUT_OF_BOUNDS       index is out of bounds
/// \retval -1/E_DB_UNSUPPORTED         bind not supported by backend
/// \retval -1/E_DB_UNSUPPORTED_MODE    bind mode not supported for type
/// \retval -1/E_DB_UNSUPPORTED_TYPE    type not supported by backend
/// \retval -1/E_GENERIC_OOM            out of memory
int db_param_bind_uint64_ref(db_stmt_ct stmt, size_t index, const uint64_t *value, const bool *is_null);

/// Bind float parameter value.
///
/// \param stmt     statement
/// \param index    parameter index
/// \param value    float value
///
/// \retval 0                           success
/// \retval -1/E_DB_ILLEGAL             statement is being executed
/// \retval -1/E_DB_OUT_OF_BOUNDS       index is out of bounds
/// \retval -1/E_DB_UNSUPPORTED         bind not supported by backend
/// \retval -1/E_DB_UNSUPPORTED_MODE    bind mode not supported for type
/// \retval -1/E_DB_UNSUPPORTED_TYPE    type not supported by backend
/// \retval -1/E_GENERIC_OOM            out of memory
int db_param_bind_float(db_stmt_ct stmt, size_t index, float value);

/// Bind float parameter reference.
///
/// \param stmt     statement
/// \param index    parameter index
/// \param value    float reference
/// \param is_null  if true bind NULL value, may be NULL
///
/// \retval 0                           success
/// \retval -1/E_DB_ILLEGAL             statement is being executed
/// \retval -1/E_DB_OUT_OF_BOUNDS       index is out of bounds
/// \retval -1/E_DB_UNSUPPORTED         bind not supported by backend
/// \retval -1/E_DB_UNSUPPORTED_MODE    bind mode not supported for type
/// \retval -1/E_DB_UNSUPPORTED_TYPE    type not supported by backend
/// \retval -1/E_GENERIC_OOM            out of memory
int db_param_bind_float_ref(db_stmt_ct stmt, size_t index, const float *value, const bool *is_null);

/// Bind double parameter value.
///
/// \param stmt     statement
/// \param index    parameter index
/// \param value    double value
///
/// \retval 0                           success
/// \retval -1/E_DB_ILLEGAL             statement is being executed
/// \retval -1/E_DB_OUT_OF_BOUNDS       index is out of bounds
/// \retval -1/E_DB_UNSUPPORTED         bind not supported by backend
/// \retval -1/E_DB_UNSUPPORTED_MODE    bind mode not supported for type
/// \retval -1/E_DB_UNSUPPORTED_TYPE    type not supported by backend
/// \retval -1/E_GENERIC_OOM            out of memory
int db_param_bind_double(db_stmt_ct stmt, size_t index, double value);

/// Bind double parameter reference.
///
/// \param stmt     statement
/// \param index    parameter index
/// \param value    double reference
/// \param is_null  if true bind NULL value, may be NULL
///
/// \retval 0                           success
/// \retval -1/E_DB_ILLEGAL             statement is being executed
/// \retval -1/E_DB_OUT_OF_BOUNDS       index is out of bounds
/// \retval -1/E_DB_UNSUPPORTED         bind not supported by backend
/// \retval -1/E_DB_UNSUPPORTED_MODE    bind mode not supported for type
/// \retval -1/E_DB_UNSUPPORTED_TYPE    type not supported by backend
/// \retval -1/E_GENERIC_OOM            out of memory
int db_param_bind_double_ref(db_stmt_ct stmt, size_t index, const double *value, const bool *is_null);

/// Bind long double parameter value.
///
/// \param stmt     statement
/// \param index    parameter index
/// \param value    long double value
///
/// \retval 0                           success
/// \retval -1/E_DB_ILLEGAL             statement is being executed
/// \retval -1/E_DB_OUT_OF_BOUNDS       index is out of bounds
/// \retval -1/E_DB_UNSUPPORTED         bind not supported by backend
/// \retval -1/E_DB_UNSUPPORTED_MODE    bind mode not supported for type
/// \retval -1/E_DB_UNSUPPORTED_TYPE    type not supported by backend
/// \retval -1/E_GENERIC_OOM            out of memory
int db_param_bind_ldouble(db_stmt_ct stmt, size_t index, long double value);

/// Bind long double parameter reference.
///
/// \param stmt     statement
/// \param index    parameter index
/// \param value    long double reference
/// \param is_null  if true bind NULL value, may be NULL
///
/// \retval 0                           success
/// \retval -1/E_DB_ILLEGAL             statement is being executed
/// \retval -1/E_DB_OUT_OF_BOUNDS       index is out of bounds
/// \retval -1/E_DB_UNSUPPORTED         bind not supported by backend
/// \retval -1/E_DB_UNSUPPORTED_MODE    bind mode not supported for type
/// \retval -1/E_DB_UNSUPPORTED_TYPE    type not supported by backend
/// \retval -1/E_GENERIC_OOM            out of memory
int db_param_bind_ldouble_ref(db_stmt_ct stmt, size_t index, const long double *value, const bool *is_null);

/// Bind static text parameter value.
///
/// \param stmt     statement
/// \param index    parameter index
/// \param text     static text, may be NULL to bind NULL value
/// \param size     text size in bytes, -1 for null terminated strings
///
/// \retval 0                           success
/// \retval -1/E_DB_ILLEGAL             statement is being executed
/// \retval -1/E_DB_OUT_OF_BOUNDS       index is out of bounds
/// \retval -1/E_DB_OUT_OF_RANGE        text too big
/// \retval -1/E_DB_UNSUPPORTED         bind not supported by backend
/// \retval -1/E_DB_UNSUPPORTED_MODE    bind mode not supported for type
/// \retval -1/E_DB_UNSUPPORTED_TYPE    type not supported by backend
/// \retval -1/E_GENERIC_OOM            out of memory
int db_param_bind_text(db_stmt_ct stmt, size_t index, const char *text, ssize_t size);

/// Bind transient text parameter value.
///
/// \param stmt     statement
/// \param index    parameter index
/// \param text     transient text, may be NULL to bind NULL value
/// \param size     text size in bytes, -1 for null terminated strings
///
/// \retval 0                           success
/// \retval -1/E_DB_ILLEGAL             statement is being executed
/// \retval -1/E_DB_OUT_OF_BOUNDS       index is out of bounds
/// \retval -1/E_DB_OUT_OF_RANGE        text too big
/// \retval -1/E_DB_UNSUPPORTED         bind not supported by backend
/// \retval -1/E_DB_UNSUPPORTED_MODE    bind mode not supported for type
/// \retval -1/E_DB_UNSUPPORTED_TYPE    type not supported by backend
/// \retval -1/E_GENERIC_OOM            out of memory
int db_param_bind_text_tmp(db_stmt_ct stmt, size_t index, const char *text, ssize_t size);

/// Bind text parameter reference.
///
/// \param stmt     statement
/// \param index    parameter index
/// \param text     text reference, set *text = NULL for NULL value
/// \param size     text size in bytes, may be NULL for null terminated texts
///
/// \retval 0                           success
/// \retval -1/E_DB_ILLEGAL             statement is being executed
/// \retval -1/E_DB_OUT_OF_BOUNDS       index is out of bounds
/// \retval -1/E_DB_UNSUPPORTED         bind not supported by backend
/// \retval -1/E_DB_UNSUPPORTED_MODE    bind mode not supported for type
/// \retval -1/E_DB_UNSUPPORTED_TYPE    type not supported by backend
/// \retval -1/E_GENERIC_OOM            out of memory
int db_param_bind_text_ref(db_stmt_ct stmt, size_t index, const char *const *text, const size_t *size);

/// Bind static BLOB parameter value.
///
/// \param stmt     statement
/// \param index    parameter index
/// \param blob     static BLOB, may be NULL to bind NULL value
/// \param size     \p blob size
///
/// \retval 0                           success
/// \retval -1/E_DB_ILLEGAL             statement is being executed
/// \retval -1/E_DB_OUT_OF_BOUNDS       index is out of bounds
/// \retval -1/E_DB_OUT_OF_RANGE        BLOB too big
/// \retval -1/E_DB_UNSUPPORTED         bind not supported by backend
/// \retval -1/E_DB_UNSUPPORTED_MODE    bind mode not supported for type
/// \retval -1/E_DB_UNSUPPORTED_TYPE    type not supported by backend
/// \retval -1/E_GENERIC_OOM            out of memory
int db_param_bind_blob(db_stmt_ct stmt, size_t index, const void *blob, size_t size);

/// Bind transient BLOB parameter value.
///
/// \param stmt     statement
/// \param index    parameter index
/// \param blob     transient BLOB, may be NULL to bind NULL value
/// \param size     \p blob size
///
/// \retval 0                           success
/// \retval -1/E_DB_ILLEGAL             statement is being executed
/// \retval -1/E_DB_OUT_OF_BOUNDS       index is out of bounds
/// \retval -1/E_DB_OUT_OF_RANGE        BLOB too big
/// \retval -1/E_DB_UNSUPPORTED         bind not supported by backend
/// \retval -1/E_DB_UNSUPPORTED_MODE    bind mode not supported for type
/// \retval -1/E_DB_UNSUPPORTED_TYPE    type not supported by backend
/// \retval -1/E_GENERIC_OOM            out of memory
int db_param_bind_blob_tmp(db_stmt_ct stmt, size_t index, const void *blob, size_t size);

/// Bind BLOB parameter reference.
///
/// \param stmt     statement
/// \param index    parameter index
/// \param blob     BLOB reference, set *blob = NULL for NULL value
/// \param size     \p blob size
///
/// \retval 0                           success
/// \retval -1/E_DB_ILLEGAL             statement is being executed
/// \retval -1/E_DB_OUT_OF_BOUNDS       index is out of bounds
/// \retval -1/E_DB_UNSUPPORTED         bind not supported by backend
/// \retval -1/E_DB_UNSUPPORTED_MODE    bind mode not supported for type
/// \retval -1/E_DB_UNSUPPORTED_TYPE    type not supported by backend
/// \retval -1/E_GENERIC_OOM            out of memory
int db_param_bind_blob_ref(db_stmt_ct stmt, size_t index, const void *const *blob, const size_t *size);

/// Bind static date parameter value.
///
/// \param stmt     statement
/// \param index    parameter index
/// \param date     static date
///
/// \retval 0                           success
/// \retval -1/E_DB_ILLEGAL             statement is being executed
/// \retval -1/E_DB_OUT_OF_BOUNDS       index is out of bounds
/// \retval -1/E_DB_OUT_OF_RANGE        month or day is out of range
/// \retval -1/E_DB_UNSUPPORTED         bind not supported by backend
/// \retval -1/E_DB_UNSUPPORTED_MODE    bind mode not supported for type
/// \retval -1/E_DB_UNSUPPORTED_TYPE    type not supported by backend
/// \retval -1/E_GENERIC_OOM            out of memory
int db_param_bind_date(db_stmt_ct stmt, size_t index, const db_date_st *date);

/// Bind transient date parameter value.
///
/// \param stmt     statement
/// \param index    parameter index
/// \param date     transient date
///
/// \retval 0                           success
/// \retval -1/E_DB_ILLEGAL             statement is being executed
/// \retval -1/E_DB_OUT_OF_BOUNDS       index is out of bounds
/// \retval -1/E_DB_OUT_OF_RANGE        month or day is out of range
/// \retval -1/E_DB_UNSUPPORTED         bind not supported by backend
/// \retval -1/E_DB_UNSUPPORTED_MODE    bind mode not supported for type
/// \retval -1/E_DB_UNSUPPORTED_TYPE    type not supported by backend
/// \retval -1/E_GENERIC_OOM            out of memory
int db_param_bind_date_tmp(db_stmt_ct stmt, size_t index, const db_date_st *date);

/// Bind date parameter with separate values.
///
/// \param stmt     statement
/// \param index    parameter index
/// \param year     year
/// \param month    month
/// \param day      day
///
/// \retval 0                           success
/// \retval -1/E_DB_ILLEGAL             statement is being executed
/// \retval -1/E_DB_OUT_OF_BOUNDS       index is out of bounds
/// \retval -1/E_DB_OUT_OF_RANGE        month or day is out of range
/// \retval -1/E_DB_UNSUPPORTED         bind not supported by backend
/// \retval -1/E_DB_UNSUPPORTED_MODE    bind mode not supported for type
/// \retval -1/E_DB_UNSUPPORTED_TYPE    type not supported by backend
/// \retval -1/E_GENERIC_OOM            out of memory
int db_param_bind_date_full(db_stmt_ct stmt, size_t index, int16_t year, uint8_t month, uint8_t day);

/// Bind date parameter reference.
///
/// \param stmt     statement
/// \param index    parameter index
/// \param date     date reference
/// \param is_null  if true bind NULL date, may be NULL
///
/// \retval 0                           success
/// \retval -1/E_DB_ILLEGAL             statement is being executed
/// \retval -1/E_DB_OUT_OF_BOUNDS       index is out of bounds
/// \retval -1/E_DB_OUT_OF_RANGE        month or day is out of range
/// \retval -1/E_DB_UNSUPPORTED         bind not supported by backend
/// \retval -1/E_DB_UNSUPPORTED_MODE    bind mode not supported for type
/// \retval -1/E_DB_UNSUPPORTED_TYPE    type not supported by backend
/// \retval -1/E_GENERIC_OOM            out of memory
int db_param_bind_date_ref(db_stmt_ct stmt, size_t index, const db_date_st *date, const bool *is_null);

/// Bind static time parameter value.
///
/// \param stmt     statement
/// \param index    parameter index
/// \param time     static time
///
/// \retval 0                           success
/// \retval -1/E_DB_ILLEGAL             statement is being executed
/// \retval -1/E_DB_OUT_OF_BOUNDS       index is out of bounds
/// \retval -1/E_DB_OUT_OF_RANGE        hour, minute or second is out of range
/// \retval -1/E_DB_UNSUPPORTED         bind not supported by backend
/// \retval -1/E_DB_UNSUPPORTED_MODE    bind mode not supported for type
/// \retval -1/E_DB_UNSUPPORTED_TYPE    type not supported by backend
/// \retval -1/E_GENERIC_OOM            out of memory
int db_param_bind_time(db_stmt_ct stmt, size_t index, const db_time_st *time);

/// Bind transient time parameter value.
///
/// \param stmt     statement
/// \param index    parameter index
/// \param time     transient time
///
/// \retval 0                           success
/// \retval -1/E_DB_ILLEGAL             statement is being executed
/// \retval -1/E_DB_OUT_OF_BOUNDS       index is out of bounds
/// \retval -1/E_DB_OUT_OF_RANGE        hour, minute or second is out of range
/// \retval -1/E_DB_UNSUPPORTED         bind not supported by backend
/// \retval -1/E_DB_UNSUPPORTED_MODE    bind mode not supported for type
/// \retval -1/E_DB_UNSUPPORTED_TYPE    type not supported by backend
/// \retval -1/E_GENERIC_OOM            out of memory
int db_param_bind_time_tmp(db_stmt_ct stmt, size_t index, const db_time_st *time);

/// Bind time parameter with separate values.
///
/// \param stmt     statement
/// \param index    parameter index
/// \param hour     hour
/// \param minute   minute
/// \param second   second
///
/// \retval 0                           success
/// \retval -1/E_DB_ILLEGAL             statement is being executed
/// \retval -1/E_DB_OUT_OF_BOUNDS       index is out of bounds
/// \retval -1/E_DB_OUT_OF_RANGE        hour, minute or second is out of range
/// \retval -1/E_DB_UNSUPPORTED         bind not supported by backend
/// \retval -1/E_DB_UNSUPPORTED_MODE    bind mode not supported for type
/// \retval -1/E_DB_UNSUPPORTED_TYPE    type not supported by backend
/// \retval -1/E_GENERIC_OOM            out of memory
int db_param_bind_time_full(db_stmt_ct stmt, size_t index, uint8_t hour, uint8_t minute, uint8_t second);

/// Bind time parameter reference.
///
/// \param stmt     statement
/// \param index    parameter index
/// \param time     time reference
/// \param is_null  if true bind NULL time, may be NULL
///
/// \retval 0                           success
/// \retval -1/E_DB_ILLEGAL             statement is being executed
/// \retval -1/E_DB_OUT_OF_BOUNDS       index is out of bounds
/// \retval -1/E_DB_OUT_OF_RANGE        hour, minute or second is out of range
/// \retval -1/E_DB_UNSUPPORTED         bind not supported by backend
/// \retval -1/E_DB_UNSUPPORTED_MODE    bind mode not supported for type
/// \retval -1/E_DB_UNSUPPORTED_TYPE    type not supported by backend
/// \retval -1/E_GENERIC_OOM            out of memory
int db_param_bind_time_ref(db_stmt_ct stmt, size_t index, const db_time_st *time, const bool *is_null);

/// Bind static datetime parameter value.
///
/// \param stmt         statement
/// \param index        parameter index
/// \param datetime     static datetime
///
/// \retval 0                           success
/// \retval -1/E_DB_ILLEGAL             statement is being executed
/// \retval -1/E_DB_OUT_OF_BOUNDS       index is out of bounds
/// \retval -1/E_DB_OUT_OF_RANGE        month, day, hour, minute or second is out of range
/// \retval -1/E_DB_UNSUPPORTED         bind not supported by backend
/// \retval -1/E_DB_UNSUPPORTED_MODE    bind mode not supported for type
/// \retval -1/E_DB_UNSUPPORTED_TYPE    type not supported by backend
/// \retval -1/E_GENERIC_OOM            out of memory
int db_param_bind_datetime(db_stmt_ct stmt, size_t index, const db_datetime_st *datetime);

/// Bind transient datetime parameter value.
///
/// \param stmt         statement
/// \param index        parameter index
/// \param datetime     transient datetime
///
/// \retval 0                           success
/// \retval -1/E_DB_ILLEGAL             statement is being executed
/// \retval -1/E_DB_OUT_OF_BOUNDS       index is out of bounds
/// \retval -1/E_DB_OUT_OF_RANGE        month, day, hour, minute or second is out of range
/// \retval -1/E_DB_UNSUPPORTED         bind not supported by backend
/// \retval -1/E_DB_UNSUPPORTED_MODE    bind mode not supported for type
/// \retval -1/E_DB_UNSUPPORTED_TYPE    type not supported by backend
/// \retval -1/E_GENERIC_OOM            out of memory
int db_param_bind_datetime_tmp(db_stmt_ct stmt, size_t index, const db_datetime_st *datetime);

/// Bind datetime parameter with separate values.
///
/// \param stmt     statement
/// \param index    parameter index
/// \param year     year
/// \param month    month
/// \param day      day
/// \param hour     hour
/// \param minute   minute
/// \param second   second
///
/// \retval 0                           success
/// \retval -1/E_DB_ILLEGAL             statement is being executed
/// \retval -1/E_DB_OUT_OF_BOUNDS       index is out of bounds
/// \retval -1/E_DB_OUT_OF_RANGE        month, day, hour, minute or second is out of range
/// \retval -1/E_DB_UNSUPPORTED         bind not supported by backend
/// \retval -1/E_DB_UNSUPPORTED_MODE    bind mode not supported for type
/// \retval -1/E_DB_UNSUPPORTED_TYPE    type not supported by backend
/// \retval -1/E_GENERIC_OOM            out of memory
int db_param_bind_datetime_full(db_stmt_ct stmt, size_t index, int16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second);

/// Bind datetime parameter reference.
///
/// \param stmt         statement
/// \param index        parameter index
/// \param datetime     datetime reference
/// \param is_null      if true bind NULL datetime, may be NULL
///
/// \retval 0                           success
/// \retval -1/E_DB_ILLEGAL             statement is being executed
/// \retval -1/E_DB_OUT_OF_BOUNDS       index is out of bounds
/// \retval -1/E_DB_OUT_OF_RANGE        month, day, hour, minute or second is out of range
/// \retval -1/E_DB_UNSUPPORTED         bind not supported by backend
/// \retval -1/E_DB_UNSUPPORTED_MODE    bind mode not supported for type
/// \retval -1/E_DB_UNSUPPORTED_TYPE    type not supported by backend
/// \retval -1/E_GENERIC_OOM            out of memory
int db_param_bind_datetime_ref(db_stmt_ct stmt, size_t index, const db_datetime_st *datetime, const bool *is_null);

/// Bind timestamp parameter value.
///
/// \param stmt         statement
/// \param index        parameter index
/// \param timestamp    unix timestamp
///
/// \retval 0                           success
/// \retval -1/E_DB_ILLEGAL             statement is being executed
/// \retval -1/E_DB_OUT_OF_BOUNDS       index is out of bounds
/// \retval -1/E_DB_UNSUPPORTED         bind not supported by backend
/// \retval -1/E_DB_UNSUPPORTED_MODE    bind mode not supported for type
/// \retval -1/E_DB_UNSUPPORTED_TYPE    type not supported by backend
/// \retval -1/E_GENERIC_OOM            out of memory
int db_param_bind_timestamp(db_stmt_ct stmt, size_t index, time_t timestamp);

/// Bind timestamp parameter reference.
///
/// \param stmt         statement
/// \param index        parameter index
/// \param timestamp    unix timestamp reference
/// \param is_null      if true bind NULL timestamp, may be NULL
///
/// \retval 0                           success
/// \retval -1/E_DB_ILLEGAL             statement is being executed
/// \retval -1/E_DB_OUT_OF_BOUNDS       index is out of bounds
/// \retval -1/E_DB_UNSUPPORTED         bind not supported by backend
/// \retval -1/E_DB_UNSUPPORTED_MODE    bind mode not supported for type
/// \retval -1/E_DB_UNSUPPORTED_TYPE    type not supported by backend
/// \retval -1/E_GENERIC_OOM            out of memory
int db_param_bind_timestamp_ref(db_stmt_ct stmt, size_t index, const time_t *timestamp, const bool *is_null);


#endif // ifndef YTIL_DB_PARAM_H_INCLUDED
