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

#ifndef YTIL_DB_RESULT_H_INCLUDED
#define YTIL_DB_RESULT_H_INCLUDED

#include <ytil/db/db.h>


/// Get number of fields in result set.
///
/// \param stmt     statement
///
/// \returns                        number of fields in result set
/// \retval -1/E_DB_UNSUPPORTED     not supported by backend
ssize_t db_result_count(db_stmt_const_ct stmt);

/// Bind boolean field.
///
/// \param stmt     statement
/// \param index    field index
/// \param value    variable to store field into
/// \param is_null  variable to set on whether field is NULL, may be NULL
///
/// \retval 0                           success
/// \retval -1/E_DB_OUT_OF_BOUNDS       index is out of bounds
/// \retval -1/E_DB_UNSUPPORTED         bind not supported by backend
/// \retval -1/E_DB_UNSUPPORTED_MODE    bind mode not supported for type
/// \retval -1/E_DB_UNSUPPORTED_TYPE    type not supported by backend
/// \retval -1/E_GENERIC_OOM            out of memory
int db_result_bind_bool(db_stmt_ct stmt, size_t index, bool *value, bool *is_null);

/// Bind signed char field.
///
/// \param stmt     statement
/// \param index    field index
/// \param value    variable to store field into
/// \param is_null  variable to set on whether field is NULL, may be NULL
///
/// \retval 0                           success
/// \retval -1/E_DB_OUT_OF_BOUNDS       index is out of bounds
/// \retval -1/E_DB_UNSUPPORTED         bind not supported by backend
/// \retval -1/E_DB_UNSUPPORTED_MODE    bind mode not supported for type
/// \retval -1/E_DB_UNSUPPORTED_TYPE    type not supported by backend
/// \retval -1/E_GENERIC_OOM            out of memory
int db_result_bind_char(db_stmt_ct stmt, size_t index, char *value, bool *is_null);

/// Bind signed short field.
///
/// \param stmt     statement
/// \param index    field index
/// \param value    variable to store field into
/// \param is_null  variable to set on whether field is NULL, may be NULL
///
/// \retval 0                           success
/// \retval -1/E_DB_OUT_OF_BOUNDS       index is out of bounds
/// \retval -1/E_DB_UNSUPPORTED         bind not supported by backend
/// \retval -1/E_DB_UNSUPPORTED_MODE    bind mode not supported for type
/// \retval -1/E_DB_UNSUPPORTED_TYPE    type not supported by backend
/// \retval -1/E_GENERIC_OOM            out of memory
int db_result_bind_short(db_stmt_ct stmt, size_t index, short *value, bool *is_null);

/// Bind signed integer field.
///
/// \param stmt     statement
/// \param index    field index
/// \param value    variable to store field into
/// \param is_null  variable to set on whether field is NULL, may be NULL
///
/// \retval 0                           success
/// \retval -1/E_DB_OUT_OF_BOUNDS       index is out of bounds
/// \retval -1/E_DB_UNSUPPORTED         bind not supported by backend
/// \retval -1/E_DB_UNSUPPORTED_MODE    bind mode not supported for type
/// \retval -1/E_DB_UNSUPPORTED_TYPE    type not supported by backend
/// \retval -1/E_GENERIC_OOM            out of memory
int db_result_bind_int(db_stmt_ct stmt, size_t index, int *value, bool *is_null);

/// Bind signed long field.
///
/// \param stmt     statement
/// \param index    field index
/// \param value    variable to store field into
/// \param is_null  variable to set on whether field is NULL, may be NULL
///
/// \retval 0                           success
/// \retval -1/E_DB_OUT_OF_BOUNDS       index is out of bounds
/// \retval -1/E_DB_UNSUPPORTED         bind not supported by backend
/// \retval -1/E_DB_UNSUPPORTED_MODE    bind mode not supported for type
/// \retval -1/E_DB_UNSUPPORTED_TYPE    type not supported by backend
/// \retval -1/E_GENERIC_OOM            out of memory
int db_result_bind_long(db_stmt_ct stmt, size_t index, long *value, bool *is_null);

/// Bind signed long long field.
///
/// \param stmt     statement
/// \param index    field index
/// \param value    variable to store field into
/// \param is_null  variable to set on whether field is NULL, may be NULL
///
/// \retval 0                           success
/// \retval -1/E_DB_OUT_OF_BOUNDS       index is out of bounds
/// \retval -1/E_DB_UNSUPPORTED         bind not supported by backend
/// \retval -1/E_DB_UNSUPPORTED_MODE    bind mode not supported for type
/// \retval -1/E_DB_UNSUPPORTED_TYPE    type not supported by backend
/// \retval -1/E_GENERIC_OOM            out of memory
int db_result_bind_longlong(db_stmt_ct stmt, size_t index, long long *value, bool *is_null);

/// Bind 8 bit signed integer field.
///
/// \param stmt     statement
/// \param index    field index
/// \param value    variable to store field into
/// \param is_null  variable to set on whether field is NULL, may be NULL
///
/// \retval 0                           success
/// \retval -1/E_DB_OUT_OF_BOUNDS       index is out of bounds
/// \retval -1/E_DB_UNSUPPORTED         bind not supported by backend
/// \retval -1/E_DB_UNSUPPORTED_MODE    bind mode not supported for type
/// \retval -1/E_DB_UNSUPPORTED_TYPE    type not supported by backend
/// \retval -1/E_GENERIC_OOM            out of memory
int db_result_bind_int8(db_stmt_ct stmt, size_t index, int8_t *value, bool *is_null);

/// Bind 16 bit signed integer field.
///
/// \param stmt     statement
/// \param index    field index
/// \param value    variable to store field into
/// \param is_null  variable to set on whether field is NULL, may be NULL
///
/// \retval 0                           success
/// \retval -1/E_DB_OUT_OF_BOUNDS       index is out of bounds
/// \retval -1/E_DB_UNSUPPORTED         bind not supported by backend
/// \retval -1/E_DB_UNSUPPORTED_MODE    bind mode not supported for type
/// \retval -1/E_DB_UNSUPPORTED_TYPE    type not supported by backend
/// \retval -1/E_GENERIC_OOM            out of memory
int db_result_bind_int16(db_stmt_ct stmt, size_t index, int16_t *value, bool *is_null);

/// Bind 32 bit signed integer field.
///
/// \param stmt     statement
/// \param index    field index
/// \param value    variable to store field into
/// \param is_null  variable to set on whether field is NULL, may be NULL
///
/// \retval 0                           success
/// \retval -1/E_DB_OUT_OF_BOUNDS       index is out of bounds
/// \retval -1/E_DB_UNSUPPORTED         bind not supported by backend
/// \retval -1/E_DB_UNSUPPORTED_MODE    bind mode not supported for type
/// \retval -1/E_DB_UNSUPPORTED_TYPE    type not supported by backend
/// \retval -1/E_GENERIC_OOM            out of memory
int db_result_bind_int32(db_stmt_ct stmt, size_t index, int32_t *value, bool *is_null);

/// Bind 64 bit signed integer field.
///
/// \param stmt     statement
/// \param index    field index
/// \param value    variable to store field into
/// \param is_null  variable to set on whether field is NULL, may be NULL
///
/// \retval 0                           success
/// \retval -1/E_DB_OUT_OF_BOUNDS       index is out of bounds
/// \retval -1/E_DB_UNSUPPORTED         bind not supported by backend
/// \retval -1/E_DB_UNSUPPORTED_MODE    bind mode not supported for type
/// \retval -1/E_DB_UNSUPPORTED_TYPE    type not supported by backend
/// \retval -1/E_GENERIC_OOM            out of memory
int db_result_bind_int64(db_stmt_ct stmt, size_t index, int64_t *value, bool *is_null);

/// Bind unsigned char field.
///
/// \param stmt     statement
/// \param index    field index
/// \param value    variable to store field into
/// \param is_null  variable to set on whether field is NULL, may be NULL
///
/// \retval 0                           success
/// \retval -1/E_DB_OUT_OF_BOUNDS       index is out of bounds
/// \retval -1/E_DB_UNSUPPORTED         bind not supported by backend
/// \retval -1/E_DB_UNSUPPORTED_MODE    bind mode not supported for type
/// \retval -1/E_DB_UNSUPPORTED_TYPE    type not supported by backend
/// \retval -1/E_GENERIC_OOM            out of memory
int db_result_bind_uchar(db_stmt_ct stmt, size_t index, unsigned char *value, bool *is_null);

/// Bind unsigned short field.
///
/// \param stmt     statement
/// \param index    field index
/// \param value    variable to store field into
/// \param is_null  variable to set on whether field is NULL, may be NULL
///
/// \retval 0                           success
/// \retval -1/E_DB_OUT_OF_BOUNDS       index is out of bounds
/// \retval -1/E_DB_UNSUPPORTED         bind not supported by backend
/// \retval -1/E_DB_UNSUPPORTED_MODE    bind mode not supported for type
/// \retval -1/E_DB_UNSUPPORTED_TYPE    type not supported by backend
/// \retval -1/E_GENERIC_OOM            out of memory
int db_result_bind_ushort(db_stmt_ct stmt, size_t index, unsigned short *value, bool *is_null);

/// Bind unsigned integer field.
///
/// \param stmt     statement
/// \param index    field index
/// \param value    variable to store field into
/// \param is_null  variable to set on whether field is NULL, may be NULL
///
/// \retval 0                           success
/// \retval -1/E_DB_OUT_OF_BOUNDS       index is out of bounds
/// \retval -1/E_DB_UNSUPPORTED         bind not supported by backend
/// \retval -1/E_DB_UNSUPPORTED_MODE    bind mode not supported for type
/// \retval -1/E_DB_UNSUPPORTED_TYPE    type not supported by backend
/// \retval -1/E_GENERIC_OOM            out of memory
int db_result_bind_uint(db_stmt_ct stmt, size_t index, unsigned int *value, bool *is_null);

/// Bind unsigned long field.
///
/// \param stmt     statement
/// \param index    field index
/// \param value    variable to store field into
/// \param is_null  variable to set on whether field is NULL, may be NULL
///
/// \retval 0                           success
/// \retval -1/E_DB_OUT_OF_BOUNDS       index is out of bounds
/// \retval -1/E_DB_UNSUPPORTED         bind not supported by backend
/// \retval -1/E_DB_UNSUPPORTED_MODE    bind mode not supported for type
/// \retval -1/E_DB_UNSUPPORTED_TYPE    type not supported by backend
/// \retval -1/E_GENERIC_OOM            out of memory
int db_result_bind_ulong(db_stmt_ct stmt, size_t index, unsigned long *value, bool *is_null);

/// Bind unsigned long long field.
///
/// \param stmt     statement
/// \param index    field index
/// \param value    variable to store field into
/// \param is_null  variable to set on whether field is NULL, may be NULL
///
/// \retval 0                           success
/// \retval -1/E_DB_OUT_OF_BOUNDS       index is out of bounds
/// \retval -1/E_DB_UNSUPPORTED         bind not supported by backend
/// \retval -1/E_DB_UNSUPPORTED_MODE    bind mode not supported for type
/// \retval -1/E_DB_UNSUPPORTED_TYPE    type not supported by backend
/// \retval -1/E_GENERIC_OOM            out of memory
int db_result_bind_ulonglong(db_stmt_ct stmt, size_t index, unsigned long long *value, bool *is_null);

/// Bind 8 bit unsigned integer field.
///
/// \param stmt     statement
/// \param index    field index
/// \param value    variable to store field into
/// \param is_null  variable to set on whether field is NULL, may be NULL
///
/// \retval 0                           success
/// \retval -1/E_DB_OUT_OF_BOUNDS       index is out of bounds
/// \retval -1/E_DB_UNSUPPORTED         bind not supported by backend
/// \retval -1/E_DB_UNSUPPORTED_MODE    bind mode not supported for type
/// \retval -1/E_DB_UNSUPPORTED_TYPE    type not supported by backend
/// \retval -1/E_GENERIC_OOM            out of memory
int db_result_bind_uint8(db_stmt_ct stmt, size_t index, uint8_t *value, bool *is_null);

/// Bind 16 bit unsigned integer field.
///
/// \param stmt     statement
/// \param index    field index
/// \param value    variable to store field into
/// \param is_null  variable to set on whether field is NULL, may be NULL
///
/// \retval 0                           success
/// \retval -1/E_DB_OUT_OF_BOUNDS       index is out of bounds
/// \retval -1/E_DB_UNSUPPORTED         bind not supported by backend
/// \retval -1/E_DB_UNSUPPORTED_MODE    bind mode not supported for type
/// \retval -1/E_DB_UNSUPPORTED_TYPE    type not supported by backend
/// \retval -1/E_GENERIC_OOM            out of memory
int db_result_bind_uint16(db_stmt_ct stmt, size_t index, uint16_t *value, bool *is_null);

/// Bind 32 bit unsigned integer field.
///
/// \param stmt     statement
/// \param index    field index
/// \param value    variable to store field into
/// \param is_null  variable to set on whether field is NULL, may be NULL
///
/// \retval 0                           success
/// \retval -1/E_DB_OUT_OF_BOUNDS       index is out of bounds
/// \retval -1/E_DB_UNSUPPORTED         bind not supported by backend
/// \retval -1/E_DB_UNSUPPORTED_MODE    bind mode not supported for type
/// \retval -1/E_DB_UNSUPPORTED_TYPE    type not supported by backend
/// \retval -1/E_GENERIC_OOM            out of memory
int db_result_bind_uint32(db_stmt_ct stmt, size_t index, uint32_t *value, bool *is_null);

/// Bind 64 bit unsigned integer field.
///
/// \param stmt     statement
/// \param index    field index
/// \param value    variable to store field into
/// \param is_null  variable to set on whether field is NULL, may be NULL
///
/// \retval 0                           success
/// \retval -1/E_DB_OUT_OF_BOUNDS       index is out of bounds
/// \retval -1/E_DB_UNSUPPORTED         bind not supported by backend
/// \retval -1/E_DB_UNSUPPORTED_MODE    bind mode not supported for type
/// \retval -1/E_DB_UNSUPPORTED_TYPE    type not supported by backend
/// \retval -1/E_GENERIC_OOM            out of memory
int db_result_bind_uint64(db_stmt_ct stmt, size_t index, uint64_t *value, bool *is_null);

/// Bind float field.
///
/// \param stmt     statement
/// \param index    field index
/// \param value    variable to store field into
/// \param is_null  variable to set on whether field is NULL, may be NULL
///
/// \retval 0                           success
/// \retval -1/E_DB_OUT_OF_BOUNDS       index is out of bounds
/// \retval -1/E_DB_UNSUPPORTED         bind not supported by backend
/// \retval -1/E_DB_UNSUPPORTED_MODE    bind mode not supported for type
/// \retval -1/E_DB_UNSUPPORTED_TYPE    type not supported by backend
/// \retval -1/E_GENERIC_OOM            out of memory
int db_result_bind_float(db_stmt_ct stmt, size_t index, float *value, bool *is_null);

/// Bind double field.
///
/// \param stmt     statement
/// \param index    field index
/// \param value    variable to store field into
/// \param is_null  variable to set on whether field is NULL, may be NULL
///
/// \retval 0                           success
/// \retval -1/E_DB_OUT_OF_BOUNDS       index is out of bounds
/// \retval -1/E_DB_UNSUPPORTED         bind not supported by backend
/// \retval -1/E_DB_UNSUPPORTED_MODE    bind mode not supported for type
/// \retval -1/E_DB_UNSUPPORTED_TYPE    type not supported by backend
/// \retval -1/E_GENERIC_OOM            out of memory
int db_result_bind_double(db_stmt_ct stmt, size_t index, double *value, bool *is_null);

/// Bind temporary text field.
///
/// After fetching the result, \p text points to the fetched text.
/// The pointer is only valid until the next fetch.
/// If the field is NULL, \p text is set to NULL.
///
/// \param stmt     statement
/// \param index    field index
/// \param text     variable to store temporary text pointer into
/// \param size     variable to store text size into, may be NULL
///
/// \retval 0                           success
/// \retval -1/E_DB_OUT_OF_BOUNDS       index is out of bounds
/// \retval -1/E_DB_UNSUPPORTED         bind not supported by backend
/// \retval -1/E_DB_UNSUPPORTED_MODE    bind mode not supported for type
/// \retval -1/E_DB_UNSUPPORTED_TYPE    type not supported by backend
/// \retval -1/E_GENERIC_OOM            out of memory
int db_result_bind_text(db_stmt_ct stmt, size_t index, char **text, size_t *size);

/// Bind allocated text field.
///
/// After fetching the result, \p text points to an allocated copy of the fetched text.
/// The copy has to be freed after use.
/// If the field is NULL, \p text is set to NULL.
///
/// \param stmt     statement
/// \param index    field index
/// \param text     variable to store allocated text pointer into
/// \param size     variable to store text size into, may be NULL
///
/// \retval 0                           success
/// \retval -1/E_DB_OUT_OF_BOUNDS       index is out of bounds
/// \retval -1/E_DB_UNSUPPORTED         bind not supported by backend
/// \retval -1/E_DB_UNSUPPORTED_MODE    bind mode not supported for type
/// \retval -1/E_DB_UNSUPPORTED_TYPE    type not supported by backend
/// \retval -1/E_GENERIC_OOM            out of memory
int db_result_bind_text_dup(db_stmt_ct stmt, size_t index, char **text, size_t *size);

/// Bind fix text field.
///
/// After fetching the result, the \p text buffer contains the fetched text.
/// Truncation may occour if the fetched text does not fit into the buffer.
/// \p size contains the real text size, even if \p text is NULL or truncation occured.
/// To check for truncation, check whether \p size is >= \p capacity.
/// The \p text buffer is always null terminated, except when capacity is 0.
///
/// \param stmt         statement
/// \param index        field index
/// \param text         buffer to store text into, may be NULL
/// \param capacity     capacity of \p text
/// \param size         variable to store text size into, may be NULL
/// \param is_null      variable to set on whether field is NULL, may be NULL
///
/// \retval 0                           success
/// \retval -1/E_DB_OUT_OF_BOUNDS       index is out of bounds
/// \retval -1/E_DB_UNSUPPORTED         bind not supported by backend
/// \retval -1/E_DB_UNSUPPORTED_MODE    bind mode not supported for type
/// \retval -1/E_DB_UNSUPPORTED_TYPE    type not supported by backend
/// \retval -1/E_GENERIC_OOM            out of memory
int db_result_bind_text_fix(db_stmt_ct stmt, size_t index, char *text, size_t capacity, size_t *size, bool *is_null);

/// Bind temporary BLOB field.
///
/// After fetching the result, \p blob points to the fetched BLOB.
/// The pointer is only valid until the next fetch.
/// If the field is NULL, \p blob is set to NULL.
///
/// \param stmt     statement
/// \param index    field index
/// \param blob     variable to store temporary BLOB pointer into
/// \param size     variable to store BLOB size into, may be NULL
///
/// \retval 0                           success
/// \retval -1/E_DB_OUT_OF_BOUNDS       index is out of bounds
/// \retval -1/E_DB_UNSUPPORTED         bind not supported by backend
/// \retval -1/E_DB_UNSUPPORTED_MODE    bind mode not supported for type
/// \retval -1/E_DB_UNSUPPORTED_TYPE    type not supported by backend
/// \retval -1/E_GENERIC_OOM            out of memory
int db_result_bind_blob(db_stmt_ct stmt, size_t index, void **blob, size_t *size);

/// Bind allocated BLOB field.
///
/// After fetching the result, \p blob points to an allocated copy of the fetched BLOB.
/// The copy has to be freed after use.
/// If the field is NULL, \p blob is set to NULL.
///
/// \param stmt     statement
/// \param index    field index
/// \param blob     variable to store allocated BLOB pointer into
/// \param size     variable to store BLOB size into, may be NULL
///
/// \retval 0                           success
/// \retval -1/E_DB_OUT_OF_BOUNDS       index is out of bounds
/// \retval -1/E_DB_UNSUPPORTED         bind not supported by backend
/// \retval -1/E_DB_UNSUPPORTED_MODE    bind mode not supported for type
/// \retval -1/E_DB_UNSUPPORTED_TYPE    type not supported by backend
/// \retval -1/E_GENERIC_OOM            out of memory
int db_result_bind_blob_dup(db_stmt_ct stmt, size_t index, void **blob, size_t *size);

/// Bind fix BLOB field.
///
/// After fetching the result, the \p blob buffer contains the fetched BLOB.
/// Truncation may occour if the fetched BLOB does not fit into the buffer.
/// \p size contains the real BLOB size, even if \p blob is NULL or truncation occured.
/// To check for truncation, check whether \p size is > \p capacity.
///
/// \param stmt         statement
/// \param index        field index
/// \param blob         buffer to store BLOB into, may be NULL
/// \param capacity     capacity of \p blob
/// \param size         variable to store BLOB size into, may be NULL
/// \param is_null      variable to set on whether field is NULL, may be NULL
///
/// \retval 0                           success
/// \retval -1/E_DB_OUT_OF_BOUNDS       index is out of bounds
/// \retval -1/E_DB_UNSUPPORTED         bind not supported by backend
/// \retval -1/E_DB_UNSUPPORTED_MODE    bind mode not supported for type
/// \retval -1/E_DB_UNSUPPORTED_TYPE    type not supported by backend
/// \retval -1/E_GENERIC_OOM            out of memory
int db_result_bind_blob_fix(db_stmt_ct stmt, size_t index, void *blob, size_t capacity, size_t *size, bool *is_null);

/// Bind date field.
///
/// \param stmt     statement
/// \param index    field index
/// \param date     variable to store field into
/// \param is_null  variable to set on whether field is NULL, may be NULL
///
/// \retval 0                           success
/// \retval -1/E_DB_OUT_OF_BOUNDS       index is out of bounds
/// \retval -1/E_DB_UNSUPPORTED         bind not supported by backend
/// \retval -1/E_DB_UNSUPPORTED_MODE    bind mode not supported for type
/// \retval -1/E_DB_UNSUPPORTED_TYPE    type not supported by backend
/// \retval -1/E_GENERIC_OOM            out of memory
int db_result_bind_date(db_stmt_ct stmt, size_t index, db_date_st *date, bool *is_null);

/// Bind time field.
///
/// \param stmt     statement
/// \param index    field index
/// \param time     variable to store field into
/// \param is_null  variable to set on whether field is NULL, may be NULL
///
/// \retval 0                           success
/// \retval -1/E_DB_OUT_OF_BOUNDS       index is out of bounds
/// \retval -1/E_DB_UNSUPPORTED         bind not supported by backend
/// \retval -1/E_DB_UNSUPPORTED_MODE    bind mode not supported for type
/// \retval -1/E_DB_UNSUPPORTED_TYPE    type not supported by backend
/// \retval -1/E_GENERIC_OOM            out of memory
int db_result_bind_time(db_stmt_ct stmt, size_t index, db_time_st *time, bool *is_null);

/// Bind datetime field.
///
/// \param stmt         statement
/// \param index        field index
/// \param datetime     variable to store field into
/// \param is_null      variable to set on whether field is NULL, may be NULL
///
/// \retval 0                           success
/// \retval -1/E_DB_OUT_OF_BOUNDS       index is out of bounds
/// \retval -1/E_DB_UNSUPPORTED         bind not supported by backend
/// \retval -1/E_DB_UNSUPPORTED_MODE    bind mode not supported for type
/// \retval -1/E_DB_UNSUPPORTED_TYPE    type not supported by backend
/// \retval -1/E_GENERIC_OOM            out of memory
int db_result_bind_datetime(db_stmt_ct stmt, size_t index, db_datetime_st *datetime, bool *is_null);

/// Bind timestamp field.
///
/// \param stmt         statement
/// \param index        field index
/// \param timestamp    variable to store field into
/// \param is_null      variable to set on whether field is NULL, may be NULL
///
/// \retval 0                           success
/// \retval -1/E_DB_OUT_OF_BOUNDS       index is out of bounds
/// \retval -1/E_DB_UNSUPPORTED         bind not supported by backend
/// \retval -1/E_DB_UNSUPPORTED_MODE    bind mode not supported for type
/// \retval -1/E_DB_UNSUPPORTED_TYPE    type not supported by backend
/// \retval -1/E_GENERIC_OOM            out of memory
int db_result_bind_timestamp(db_stmt_ct stmt, size_t index, time_t *timestamp, bool *is_null);

/// Fetch result field value.
///
/// Fields are fetched into the prebound buffers while executing a statement.
/// In case a field needs to be rebound to a different type,
/// or in case of truncation to a larger buffer, this function
/// will fetch the field again into the newly bound buffer.
///
/// \param stmt     statement
/// \param index    field index
/// \param offset   value offset in bytes, value is fetched starting at offset
///
/// \retval 0                       success
/// \retval -1/E_DB_ILLEGAL         statement is not beeing executed
/// \retval -1/E_DB_OUT_OF_BOUNDS   index is out of bounds
/// \retval -1/E_DB_OUT_OF_RANGE    offset is out of range
/// \retval -1/E_DB_UNSUPPORTED     not supported by backend
/// \retval -1/E_GENERIC_OOM        out of memory
int db_result_fetch(db_stmt_const_ct stmt, size_t index, size_t offset);

/// Get type of result field.
///
/// \param stmt     statement
/// \param index    field index
///
/// \returns                                    field type
/// \retval DB_TYPE_INVALID/E_DB_OUT_OF_BOUNDS  index is out of bounds
/// \retval DB_TYPE_INVALID/E_DB_UNKNOWN_TYPE   unknown field type
/// \retval DB_TYPE_INVALID/E_DB_UNSUPPORTED    not supported by backend
db_type_id db_result_get_type(db_stmt_const_ct stmt, size_t index);

/// Get database name of result field.
///
/// \param stmt     statement
/// \param index    field index
///
/// \returns                        database name
/// \retval NULL/E_DB_NO_NAME       field is expression or sub query
/// \retval NULL/E_DB_OUT_OF_BOUNDS index is out of bounds
/// \retval NULL/E_DB_UNSUPPORTED   not supported by backend
/// \retval NULL/E_GENERIC_OOM      out of memory
const char *db_result_get_database_name(db_stmt_const_ct stmt, size_t index);

/// Get table name of result field.
///
/// \param stmt     statement
/// \param index    field index
///
/// \returns                        table name
/// \retval NULL/E_DB_NO_NAME       field is expression or sub query
/// \retval NULL/E_DB_OUT_OF_BOUNDS index is out of bounds
/// \retval NULL/E_DB_UNSUPPORTED   not supported by backend
/// \retval NULL/E_GENERIC_OOM      out of memory
const char *db_result_get_table_name(db_stmt_const_ct stmt, size_t index);

/// Get original table name of result field.
///
/// \param stmt     statement
/// \param index    field index
///
/// \returns                        original table name
/// \retval NULL/E_DB_NO_NAME       field is expression or sub query
/// \retval NULL/E_DB_OUT_OF_BOUNDS index is out of bounds
/// \retval NULL/E_DB_UNSUPPORTED   not supported by backend
/// \retval NULL/E_GENERIC_OOM      out of memory
const char *db_result_get_original_table_name(db_stmt_const_ct stmt, size_t index);

/// Get name of result field.
///
/// \param stmt     statement
/// \param index    field index
///
/// \returns                        field name
/// \retval NULL/E_DB_OUT_OF_BOUNDS index is out of bounds
/// \retval NULL/E_DB_UNSUPPORTED   not supported by backend
/// \retval NULL/E_GENERIC_OOM      out of memory
const char *db_result_get_field_name(db_stmt_const_ct stmt, size_t index);

/// Get original name of result field.
///
/// \param stmt     statement
/// \param index    field index
///
/// \returns                        origin field name
/// \retval NULL/E_DB_NO_NAME       field is expression or sub query
/// \retval NULL/E_DB_OUT_OF_BOUNDS index is out of bounds
/// \retval NULL/E_DB_UNSUPPORTED   not supported by backend
/// \retval NULL/E_GENERIC_OOM      out of memory
const char *db_result_get_original_field_name(db_stmt_const_ct stmt, size_t index);


#endif // ifndef YTIL_DB_RESULT_H_INCLUDED
