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

#include <ytil/db/interface.h>
#include <ytil/def.h>
#include <stdlib.h>


/// default error type for db result module
#define ERROR_TYPE_DEFAULT ERROR_TYPE_DB


/// Get suitable integer type for size and signedness.
///
/// \param size         bytes of type
/// \param is_signed    signedness
///
/// \returns DB integer type
static inline db_type_id db_inttype(size_t size, bool is_signed)
{
    switch(size)
    {
    case 1:
        return is_signed ? DB_TYPE_INT8 : DB_TYPE_UINT8;

    case 2:
        return is_signed ? DB_TYPE_INT16 : DB_TYPE_UINT16;

    case 4:
        return is_signed ? DB_TYPE_INT32 : DB_TYPE_UINT32;

    case 8:
        return is_signed ? DB_TYPE_INT64 : DB_TYPE_UINT64;

    default:
        abort();
    }
}

ssize_t db_result_count(db_stmt_const_ct stmt)
{
    db_count_cb count = db_stmt_get_interface(stmt)->result_count;

    return_error_if_fail(count, E_DB_UNSUPPORTED, -1);

    return count(stmt);
}

int db_result_bind_bool(db_stmt_ct stmt, size_t index, bool *value, bool *is_null)
{
    db_result_bind_cb bind = db_stmt_get_interface(stmt)->result_bind;
    db_result_bind_st info =
    {
        .mode       = DB_RESULT_BIND_FIX,
        .type       = DB_TYPE_BOOL,
        .data.b     = value,
        .cap        = sizeof(bool),
        .is_null    = is_null,
    };

    assert(value);

    return_error_if_fail(bind, E_DB_UNSUPPORTED, -1);

    return error_pass_int(bind(stmt, index, &info));
}

int db_result_bind_char(db_stmt_ct stmt, size_t index, char *value, bool *is_null)
{
    db_result_bind_cb bind = db_stmt_get_interface(stmt)->result_bind;
    db_result_bind_st info =
    {
        .mode       = DB_RESULT_BIND_FIX,
        .type       = db_inttype(sizeof(char), true),
        .data.blob  = value,
        .cap        = sizeof(char),
        .is_null    = is_null,
    };

    assert(value);

    return_error_if_fail(bind, E_DB_UNSUPPORTED, -1);

    return error_pass_int(bind(stmt, index, &info));
}

int db_result_bind_short(db_stmt_ct stmt, size_t index, short *value, bool *is_null)
{
    db_result_bind_cb bind = db_stmt_get_interface(stmt)->result_bind;
    db_result_bind_st info =
    {
        .mode       = DB_RESULT_BIND_FIX,
        .type       = db_inttype(sizeof(short), true),
        .data.blob  = value,
        .cap        = sizeof(short),
        .is_null    = is_null,
    };

    assert(value);

    return_error_if_fail(bind, E_DB_UNSUPPORTED, -1);

    return error_pass_int(bind(stmt, index, &info));
}

int db_result_bind_int(db_stmt_ct stmt, size_t index, int *value, bool *is_null)
{
    db_result_bind_cb bind = db_stmt_get_interface(stmt)->result_bind;
    db_result_bind_st info =
    {
        .mode       = DB_RESULT_BIND_FIX,
        .type       = db_inttype(sizeof(int), true),
        .data.blob  = value,
        .cap        = sizeof(int),
        .is_null    = is_null,
    };

    assert(value);

    return_error_if_fail(bind, E_DB_UNSUPPORTED, -1);

    return error_pass_int(bind(stmt, index, &info));
}

int db_result_bind_long(db_stmt_ct stmt, size_t index, long *value, bool *is_null)
{
    db_result_bind_cb bind = db_stmt_get_interface(stmt)->result_bind;
    db_result_bind_st info =
    {
        .mode       = DB_RESULT_BIND_FIX,
        .type       = db_inttype(sizeof(long), true),
        .data.blob  = value,
        .cap        = sizeof(long),
        .is_null    = is_null,
    };

    assert(value);

    return_error_if_fail(bind, E_DB_UNSUPPORTED, -1);

    return error_pass_int(bind(stmt, index, &info));
}

int db_result_bind_longlong(db_stmt_ct stmt, size_t index, long long *value, bool *is_null)
{
    db_result_bind_cb bind = db_stmt_get_interface(stmt)->result_bind;
    db_result_bind_st info =
    {
        .mode       = DB_RESULT_BIND_FIX,
        .type       = db_inttype(sizeof(long long), true),
        .data.blob  = value,
        .cap        = sizeof(long long),
        .is_null    = is_null,
    };

    assert(value);

    return_error_if_fail(bind, E_DB_UNSUPPORTED, -1);

    return error_pass_int(bind(stmt, index, &info));
}

int db_result_bind_int8(db_stmt_ct stmt, size_t index, int8_t *value, bool *is_null)
{
    db_result_bind_cb bind = db_stmt_get_interface(stmt)->result_bind;
    db_result_bind_st info =
    {
        .mode       = DB_RESULT_BIND_FIX,
        .type       = DB_TYPE_INT8,
        .data.i8    = value,
        .cap        = sizeof(int8_t),
        .is_null    = is_null,
    };

    assert(value);

    return_error_if_fail(bind, E_DB_UNSUPPORTED, -1);

    return error_pass_int(bind(stmt, index, &info));
}

int db_result_bind_int16(db_stmt_ct stmt, size_t index, int16_t *value, bool *is_null)
{
    db_result_bind_cb bind = db_stmt_get_interface(stmt)->result_bind;
    db_result_bind_st info =
    {
        .mode       = DB_RESULT_BIND_FIX,
        .type       = DB_TYPE_INT16,
        .data.i16   = value,
        .cap        = sizeof(int16_t),
        .is_null    = is_null,
    };

    assert(value);

    return_error_if_fail(bind, E_DB_UNSUPPORTED, -1);

    return error_pass_int(bind(stmt, index, &info));
}

int db_result_bind_int32(db_stmt_ct stmt, size_t index, int32_t *value, bool *is_null)
{
    db_result_bind_cb bind = db_stmt_get_interface(stmt)->result_bind;
    db_result_bind_st info =
    {
        .mode       = DB_RESULT_BIND_FIX,
        .type       = DB_TYPE_INT32,
        .data.i32   = value,
        .cap        = sizeof(int32_t),
        .is_null    = is_null,
    };

    assert(value);

    return_error_if_fail(bind, E_DB_UNSUPPORTED, -1);

    return error_pass_int(bind(stmt, index, &info));
}

int db_result_bind_int64(db_stmt_ct stmt, size_t index, int64_t *value, bool *is_null)
{
    db_result_bind_cb bind = db_stmt_get_interface(stmt)->result_bind;
    db_result_bind_st info =
    {
        .mode       = DB_RESULT_BIND_FIX,
        .type       = DB_TYPE_INT64,
        .data.i64   = value,
        .cap        = sizeof(int64_t),
        .is_null    = is_null,
    };

    assert(value);

    return_error_if_fail(bind, E_DB_UNSUPPORTED, -1);

    return error_pass_int(bind(stmt, index, &info));
}

int db_result_bind_uchar(db_stmt_ct stmt, size_t index, unsigned char *value, bool *is_null)
{
    db_result_bind_cb bind = db_stmt_get_interface(stmt)->result_bind;
    db_result_bind_st info =
    {
        .mode       = DB_RESULT_BIND_FIX,
        .type       = db_inttype(sizeof(unsigned char), false),
        .data.blob  = value,
        .cap        = sizeof(unsigned char),
        .is_null    = is_null,
    };

    assert(value);

    return_error_if_fail(bind, E_DB_UNSUPPORTED, -1);

    return error_pass_int(bind(stmt, index, &info));
}

int db_result_bind_ushort(db_stmt_ct stmt, size_t index, unsigned short *value, bool *is_null)
{
    db_result_bind_cb bind = db_stmt_get_interface(stmt)->result_bind;
    db_result_bind_st info =
    {
        .mode       = DB_RESULT_BIND_FIX,
        .type       = db_inttype(sizeof(unsigned short), false),
        .data.blob  = value,
        .cap        = sizeof(unsigned short),
        .is_null    = is_null,
    };

    assert(value);

    return_error_if_fail(bind, E_DB_UNSUPPORTED, -1);

    return error_pass_int(bind(stmt, index, &info));
}

int db_result_bind_uint(db_stmt_ct stmt, size_t index, unsigned int *value, bool *is_null)
{
    db_result_bind_cb bind = db_stmt_get_interface(stmt)->result_bind;
    db_result_bind_st info =
    {
        .mode       = DB_RESULT_BIND_FIX,
        .type       = db_inttype(sizeof(unsigned int), false),
        .data.blob  = value,
        .cap        = sizeof(unsigned int),
        .is_null    = is_null,
    };

    assert(value);

    return_error_if_fail(bind, E_DB_UNSUPPORTED, -1);

    return error_pass_int(bind(stmt, index, &info));
}

int db_result_bind_ulong(db_stmt_ct stmt, size_t index, unsigned long *value, bool *is_null)
{
    db_result_bind_cb bind = db_stmt_get_interface(stmt)->result_bind;
    db_result_bind_st info =
    {
        .mode       = DB_RESULT_BIND_FIX,
        .type       = db_inttype(sizeof(unsigned long), false),
        .data.blob  = value,
        .cap        = sizeof(unsigned long),
        .is_null    = is_null,
    };

    assert(value);

    return_error_if_fail(bind, E_DB_UNSUPPORTED, -1);

    return error_pass_int(bind(stmt, index, &info));
}

int db_result_bind_ulonglong(db_stmt_ct stmt, size_t index, unsigned long long *value, bool *is_null)
{
    db_result_bind_cb bind = db_stmt_get_interface(stmt)->result_bind;
    db_result_bind_st info =
    {
        .mode       = DB_RESULT_BIND_FIX,
        .type       = db_inttype(sizeof(unsigned long long), false),
        .data.blob  = value,
        .cap        = sizeof(unsigned long long),
        .is_null    = is_null,
    };

    assert(value);

    return_error_if_fail(bind, E_DB_UNSUPPORTED, -1);

    return error_pass_int(bind(stmt, index, &info));
}

int db_result_bind_uint8(db_stmt_ct stmt, size_t index, uint8_t *value, bool *is_null)
{
    db_result_bind_cb bind = db_stmt_get_interface(stmt)->result_bind;
    db_result_bind_st info =
    {
        .mode       = DB_RESULT_BIND_FIX,
        .type       = DB_TYPE_UINT8,
        .data.u8    = value,
        .cap        = sizeof(uint8_t),
        .is_null    = is_null,
    };

    assert(value);

    return_error_if_fail(bind, E_DB_UNSUPPORTED, -1);

    return error_pass_int(bind(stmt, index, &info));
}

int db_result_bind_uint16(db_stmt_ct stmt, size_t index, uint16_t *value, bool *is_null)
{
    db_result_bind_cb bind = db_stmt_get_interface(stmt)->result_bind;
    db_result_bind_st info =
    {
        .mode       = DB_RESULT_BIND_FIX,
        .type       = DB_TYPE_UINT16,
        .data.u16   = value,
        .cap        = sizeof(uint16_t),
        .is_null    = is_null,
    };

    assert(value);

    return_error_if_fail(bind, E_DB_UNSUPPORTED, -1);

    return error_pass_int(bind(stmt, index, &info));
}

int db_result_bind_uint32(db_stmt_ct stmt, size_t index, uint32_t *value, bool *is_null)
{
    db_result_bind_cb bind = db_stmt_get_interface(stmt)->result_bind;
    db_result_bind_st info =
    {
        .mode       = DB_RESULT_BIND_FIX,
        .type       = DB_TYPE_UINT32,
        .data.u32   = value,
        .cap        = sizeof(uint32_t),
        .is_null    = is_null,
    };

    assert(value);

    return_error_if_fail(bind, E_DB_UNSUPPORTED, -1);

    return error_pass_int(bind(stmt, index, &info));
}

int db_result_bind_uint64(db_stmt_ct stmt, size_t index, uint64_t *value, bool *is_null)
{
    db_result_bind_cb bind = db_stmt_get_interface(stmt)->result_bind;
    db_result_bind_st info =
    {
        .mode       = DB_RESULT_BIND_FIX,
        .type       = DB_TYPE_UINT64,
        .data.u64   = value,
        .cap        = sizeof(uint64_t),
        .is_null    = is_null,
    };

    assert(value);

    return_error_if_fail(bind, E_DB_UNSUPPORTED, -1);

    return error_pass_int(bind(stmt, index, &info));
}

int db_result_bind_float(db_stmt_ct stmt, size_t index, float *value, bool *is_null)
{
    db_result_bind_cb bind = db_stmt_get_interface(stmt)->result_bind;
    db_result_bind_st info =
    {
        .mode       = DB_RESULT_BIND_FIX,
        .type       = DB_TYPE_FLOAT,
        .data.f     = value,
        .cap        = sizeof(float),
        .is_null    = is_null,
    };

    assert(value);

    return_error_if_fail(bind, E_DB_UNSUPPORTED, -1);

    return error_pass_int(bind(stmt, index, &info));
}

int db_result_bind_double(db_stmt_ct stmt, size_t index, double *value, bool *is_null)
{
    db_result_bind_cb bind = db_stmt_get_interface(stmt)->result_bind;
    db_result_bind_st info =
    {
        .mode       = DB_RESULT_BIND_FIX,
        .type       = DB_TYPE_DOUBLE,
        .data.d     = value,
        .cap        = sizeof(double),
        .is_null    = is_null,
    };

    assert(value);

    return_error_if_fail(bind, E_DB_UNSUPPORTED, -1);

    return error_pass_int(bind(stmt, index, &info));
}

int db_result_bind_ldouble(db_stmt_ct stmt, size_t index, long double *value, bool *is_null)
{
    db_result_bind_cb bind = db_stmt_get_interface(stmt)->result_bind;
    db_result_bind_st info =
    {
        .mode       = DB_RESULT_BIND_FIX,
        .type       = DB_TYPE_LDOUBLE,
        .data.ld    = value,
        .cap        = sizeof(long double),
        .is_null    = is_null,
    };

    assert(value);

    return_error_if_fail(bind, E_DB_UNSUPPORTED, -1);

    return error_pass_int(bind(stmt, index, &info));
}

int db_result_bind_text(db_stmt_ct stmt, size_t index, char **text, size_t *size)
{
    db_result_bind_cb bind = db_stmt_get_interface(stmt)->result_bind;
    db_result_bind_st info =
    {
        .mode       = DB_RESULT_BIND_TMP,
        .type       = DB_TYPE_TEXT,
        .data.ptext = text,
        .cap        = sizeof(char *),
        .size       = size,
    };

    assert(text);

    return_error_if_fail(bind, E_DB_UNSUPPORTED, -1);

    return error_pass_int(bind(stmt, index, &info));
}

int db_result_bind_text_dup(db_stmt_ct stmt, size_t index, char **text, size_t *size)
{
    db_result_bind_cb bind = db_stmt_get_interface(stmt)->result_bind;
    db_result_bind_st info =
    {
        .mode       = DB_RESULT_BIND_DUP,
        .type       = DB_TYPE_TEXT,
        .data.ptext = text,
        .cap        = sizeof(char *),
        .size       = size,
    };

    assert(text);

    return_error_if_fail(bind, E_DB_UNSUPPORTED, -1);

    return error_pass_int(bind(stmt, index, &info));
}

int db_result_bind_text_fix(db_stmt_ct stmt, size_t index, char *text, size_t capacity, size_t *size, bool *is_null)
{
    db_result_bind_cb bind = db_stmt_get_interface(stmt)->result_bind;
    db_result_bind_st info =
    {
        .mode       = DB_RESULT_BIND_FIX,
        .type       = DB_TYPE_TEXT,
        .data.text  = text,
        .cap        = capacity,
        .size       = size,
        .is_null    = is_null,
    };


    return_error_if_fail(bind, E_DB_UNSUPPORTED, -1);

    return error_pass_int(bind(stmt, index, &info));
}

int db_result_bind_blob(db_stmt_ct stmt, size_t index, void **blob, size_t *size)
{
    db_result_bind_cb bind = db_stmt_get_interface(stmt)->result_bind;
    db_result_bind_st info =
    {
        .mode       = DB_RESULT_BIND_TMP,
        .type       = DB_TYPE_BLOB,
        .data.pblob = blob,
        .cap        = sizeof(void *),
        .size       = size,
    };

    assert(blob);

    return_error_if_fail(bind, E_DB_UNSUPPORTED, -1);

    return error_pass_int(bind(stmt, index, &info));
}

int db_result_bind_blob_dup(db_stmt_ct stmt, size_t index, void **blob, size_t *size)
{
    db_result_bind_cb bind = db_stmt_get_interface(stmt)->result_bind;
    db_result_bind_st info =
    {
        .mode       = DB_RESULT_BIND_DUP,
        .type       = DB_TYPE_BLOB,
        .data.pblob = blob,
        .cap        = sizeof(void *),
        .size       = size,
    };

    assert(blob);

    return_error_if_fail(bind, E_DB_UNSUPPORTED, -1);

    return error_pass_int(bind(stmt, index, &info));
}

int db_result_bind_blob_fix(db_stmt_ct stmt, size_t index, void *blob, size_t capacity, size_t *size, bool *is_null)
{
    db_result_bind_cb bind = db_stmt_get_interface(stmt)->result_bind;
    db_result_bind_st info =
    {
        .mode       = DB_RESULT_BIND_FIX,
        .type       = DB_TYPE_BLOB,
        .data.blob  = blob,
        .cap        = capacity,
        .size       = size,
        .is_null    = is_null,
    };

    return_error_if_fail(bind, E_DB_UNSUPPORTED, -1);

    return error_pass_int(bind(stmt, index, &info));
}

int db_result_bind_date(db_stmt_ct stmt, size_t index, db_date_st *date, bool *is_null)
{
    db_result_bind_cb bind = db_stmt_get_interface(stmt)->result_bind;
    db_result_bind_st info =
    {
        .mode       = DB_RESULT_BIND_FIX,
        .type       = DB_TYPE_DATE,
        .data.date  = date,
        .cap        = sizeof(db_date_st),
        .is_null    = is_null,
    };

    assert(date);

    return_error_if_fail(bind, E_DB_UNSUPPORTED, -1);

    return error_pass_int(bind(stmt, index, &info));
}

int db_result_bind_time(db_stmt_ct stmt, size_t index, db_time_st *time, bool *is_null)
{
    db_result_bind_cb bind = db_stmt_get_interface(stmt)->result_bind;
    db_result_bind_st info =
    {
        .mode       = DB_RESULT_BIND_FIX,
        .type       = DB_TYPE_TIME,
        .data.time  = time,
        .cap        = sizeof(db_time_st),
        .is_null    = is_null,
    };

    assert(time);

    return_error_if_fail(bind, E_DB_UNSUPPORTED, -1);

    return error_pass_int(bind(stmt, index, &info));
}

int db_result_bind_datetime(db_stmt_ct stmt, size_t index, db_datetime_st *datetime, bool *is_null)
{
    db_result_bind_cb bind = db_stmt_get_interface(stmt)->result_bind;
    db_result_bind_st info =
    {
        .mode       = DB_RESULT_BIND_FIX,
        .type       = DB_TYPE_DATETIME,
        .data.dt    = datetime,
        .cap        = sizeof(db_datetime_st),
        .is_null    = is_null,
    };

    assert(datetime);

    return_error_if_fail(bind, E_DB_UNSUPPORTED, -1);

    return error_pass_int(bind(stmt, index, &info));
}

int db_result_bind_timestamp(db_stmt_ct stmt, size_t index, time_t *timestamp, bool *is_null)
{
    db_result_bind_cb bind = db_stmt_get_interface(stmt)->result_bind;
    db_result_bind_st info =
    {
        .mode       = DB_RESULT_BIND_FIX,
        .type       = DB_TYPE_TIMESTAMP,
        .data.ts    = timestamp,
        .cap        = sizeof(time_t),
        .is_null    = is_null,
    };

    assert(timestamp);

    return_error_if_fail(bind, E_DB_UNSUPPORTED, -1);

    return error_pass_int(bind(stmt, index, &info));
}

db_type_id db_result_get_type(db_stmt_const_ct stmt, size_t index)
{
    db_type_cb get_type = db_stmt_get_interface(stmt)->result_type;
    db_type_id type;

    return_error_if_fail(get_type, E_DB_UNSUPPORTED, DB_TYPE_INVALID);

    if((type = get_type(stmt, index)) == DB_TYPE_INVALID)
        error_pass();

    return type;
}

const char *db_result_get_database_name(db_stmt_const_ct stmt, size_t index)
{
    db_name_cb get_name = db_stmt_get_interface(stmt)->result_name;

    return_error_if_fail(get_name, E_DB_UNSUPPORTED, NULL);

    return error_pass_ptr(get_name(stmt, index, DB_NAME_DATABASE));
}

const char *db_result_get_table_name(db_stmt_const_ct stmt, size_t index)
{
    db_name_cb get_name = db_stmt_get_interface(stmt)->result_name;

    return_error_if_fail(get_name, E_DB_UNSUPPORTED, NULL);

    return error_pass_ptr(get_name(stmt, index, DB_NAME_TABLE));
}

const char *db_result_get_original_table_name(db_stmt_const_ct stmt, size_t index)
{
    db_name_cb get_name = db_stmt_get_interface(stmt)->result_name;

    return_error_if_fail(get_name, E_DB_UNSUPPORTED, NULL);

    return error_pass_ptr(get_name(stmt, index, DB_NAME_TABLE_ORG));
}

const char *db_result_get_field_name(db_stmt_const_ct stmt, size_t index)
{
    db_name_cb get_name = db_stmt_get_interface(stmt)->result_name;

    return_error_if_fail(get_name, E_DB_UNSUPPORTED, NULL);

    return error_pass_ptr(get_name(stmt, index, DB_NAME_FIELD));
}

const char *db_result_get_original_field_name(db_stmt_const_ct stmt, size_t index)
{
    db_name_cb get_name = db_stmt_get_interface(stmt)->result_name;

    return_error_if_fail(get_name, E_DB_UNSUPPORTED, NULL);

    return error_pass_ptr(get_name(stmt, index, DB_NAME_FIELD_ORG));
}
