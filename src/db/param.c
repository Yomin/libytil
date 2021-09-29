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
#include <string.h>


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

ssize_t db_param_count(db_stmt_const_ct stmt)
{
    db_count_cb count = db_stmt_get_interface(stmt)->param_count;

    return_error_if_fail(count, E_DB_UNSUPPORTED, -1);

    return count(stmt);
}

int db_param_bind_null(db_stmt_ct stmt, size_t index)
{
    db_param_bind_cb bind = db_stmt_get_interface(stmt)->param_bind;
    db_param_bind_st info = { .type = DB_TYPE_NULL };

    return_error_if_fail(bind, E_DB_UNSUPPORTED, -1);
    return_error_if_pass(db_stmt_is_executing(stmt), E_DB_ILLEGAL, -1);

    return error_pass_int(bind(stmt, index, &info));
}

int db_param_bind_bool(db_stmt_ct stmt, size_t index, bool value)
{
    db_param_bind_cb bind = db_stmt_get_interface(stmt)->param_bind;
    db_param_bind_st info =
    {
        .mode   = DB_PARAM_BIND_TMP,
        .type   = DB_TYPE_BOOL,
        .data.b = &value,
        .vsize  = sizeof(bool),
    };

    return_error_if_fail(bind, E_DB_UNSUPPORTED, -1);
    return_error_if_pass(db_stmt_is_executing(stmt), E_DB_ILLEGAL, -1);

    return error_pass_int(bind(stmt, index, &info));
}

int db_param_bind_bool_ref(db_stmt_ct stmt, size_t index, const bool *value, const bool *is_null)
{
    db_param_bind_cb bind = db_stmt_get_interface(stmt)->param_bind;
    db_param_bind_st info =
    {
        .mode       = DB_PARAM_BIND_REF,
        .type       = DB_TYPE_BOOL,
        .data.b     = (bool *)value,
        .vsize      = sizeof(bool),
        .rsize      = &info.vsize,
        .is_null    = is_null,
    };

    assert(value);

    return_error_if_fail(bind, E_DB_UNSUPPORTED, -1);
    return_error_if_pass(db_stmt_is_executing(stmt), E_DB_ILLEGAL, -1);

    return error_pass_int(bind(stmt, index, &info));
}

int db_param_bind_char(db_stmt_ct stmt, size_t index, char value)
{
    db_param_bind_cb bind = db_stmt_get_interface(stmt)->param_bind;
    db_param_bind_st info =
    {
        .mode       = DB_PARAM_BIND_TMP,
        .type       = db_inttype(sizeof(char), true),
        .data.blob  = &value,
        .vsize      = sizeof(char),
    };

    return_error_if_fail(bind, E_DB_UNSUPPORTED, -1);
    return_error_if_pass(db_stmt_is_executing(stmt), E_DB_ILLEGAL, -1);

    return error_pass_int(bind(stmt, index, &info));
}

int db_param_bind_char_ref(db_stmt_ct stmt, size_t index, const char *value, const bool *is_null)
{
    db_param_bind_cb bind = db_stmt_get_interface(stmt)->param_bind;
    db_param_bind_st info =
    {
        .mode       = DB_PARAM_BIND_REF,
        .type       = db_inttype(sizeof(char), true),
        .data.blob  = (char *)value,
        .vsize      = sizeof(char),
        .rsize      = &info.vsize,
        .is_null    = is_null,
    };

    return_error_if_fail(bind, E_DB_UNSUPPORTED, -1);
    return_error_if_pass(db_stmt_is_executing(stmt), E_DB_ILLEGAL, -1);

    return error_pass_int(bind(stmt, index, &info));
}

int db_param_bind_short(db_stmt_ct stmt, size_t index, short value)
{
    db_param_bind_cb bind = db_stmt_get_interface(stmt)->param_bind;
    db_param_bind_st info =
    {
        .mode       = DB_PARAM_BIND_TMP,
        .type       = db_inttype(sizeof(short), true),
        .data.blob  = &value,
        .vsize      = sizeof(short),
    };

    return_error_if_fail(bind, E_DB_UNSUPPORTED, -1);
    return_error_if_pass(db_stmt_is_executing(stmt), E_DB_ILLEGAL, -1);

    return error_pass_int(bind(stmt, index, &info));
}

int db_param_bind_short_ref(db_stmt_ct stmt, size_t index, const short *value, const bool *is_null)
{
    db_param_bind_cb bind = db_stmt_get_interface(stmt)->param_bind;
    db_param_bind_st info =
    {
        .mode       = DB_PARAM_BIND_REF,
        .type       = db_inttype(sizeof(short), true),
        .data.blob  = (short *)value,
        .vsize      = sizeof(short),
        .rsize      = &info.vsize,
        .is_null    = is_null,
    };

    return_error_if_fail(bind, E_DB_UNSUPPORTED, -1);
    return_error_if_pass(db_stmt_is_executing(stmt), E_DB_ILLEGAL, -1);

    return error_pass_int(bind(stmt, index, &info));
}

int db_param_bind_int(db_stmt_ct stmt, size_t index, int value)
{
    db_param_bind_cb bind = db_stmt_get_interface(stmt)->param_bind;
    db_param_bind_st info =
    {
        .mode       = DB_PARAM_BIND_TMP,
        .type       = db_inttype(sizeof(int), true),
        .data.blob  = &value,
        .vsize      = sizeof(int),
    };

    return_error_if_fail(bind, E_DB_UNSUPPORTED, -1);
    return_error_if_pass(db_stmt_is_executing(stmt), E_DB_ILLEGAL, -1);

    return error_pass_int(bind(stmt, index, &info));
}

int db_param_bind_int_ref(db_stmt_ct stmt, size_t index, const int *value, const bool *is_null)
{
    db_param_bind_cb bind = db_stmt_get_interface(stmt)->param_bind;
    db_param_bind_st info =
    {
        .mode       = DB_PARAM_BIND_REF,
        .type       = db_inttype(sizeof(int), true),
        .data.blob  = (int *)value,
        .vsize      = sizeof(int),
        .rsize      = &info.vsize,
        .is_null    = is_null,
    };

    return_error_if_fail(bind, E_DB_UNSUPPORTED, -1);
    return_error_if_pass(db_stmt_is_executing(stmt), E_DB_ILLEGAL, -1);

    return error_pass_int(bind(stmt, index, &info));
}

int db_param_bind_long(db_stmt_ct stmt, size_t index, long value)
{
    db_param_bind_cb bind = db_stmt_get_interface(stmt)->param_bind;
    db_param_bind_st info =
    {
        .mode       = DB_PARAM_BIND_TMP,
        .type       = db_inttype(sizeof(long), true),
        .data.blob  = &value,
        .vsize      = sizeof(long),
    };

    return_error_if_fail(bind, E_DB_UNSUPPORTED, -1);
    return_error_if_pass(db_stmt_is_executing(stmt), E_DB_ILLEGAL, -1);

    return error_pass_int(bind(stmt, index, &info));
}

int db_param_bind_long_ref(db_stmt_ct stmt, size_t index, const long *value, const bool *is_null)
{
    db_param_bind_cb bind = db_stmt_get_interface(stmt)->param_bind;
    db_param_bind_st info =
    {
        .mode       = DB_PARAM_BIND_REF,
        .type       = db_inttype(sizeof(long), true),
        .data.blob  = (long *)value,
        .vsize      = sizeof(long),
        .rsize      = &info.vsize,
        .is_null    = is_null,
    };

    return_error_if_fail(bind, E_DB_UNSUPPORTED, -1);
    return_error_if_pass(db_stmt_is_executing(stmt), E_DB_ILLEGAL, -1);

    return error_pass_int(bind(stmt, index, &info));
}

int db_param_bind_longlong(db_stmt_ct stmt, size_t index, long long value)
{
    db_param_bind_cb bind = db_stmt_get_interface(stmt)->param_bind;
    db_param_bind_st info =
    {
        .mode       = DB_PARAM_BIND_TMP,
        .type       = db_inttype(sizeof(long long), true),
        .data.blob  = &value,
        .vsize      = sizeof(long long),
    };

    return_error_if_fail(bind, E_DB_UNSUPPORTED, -1);
    return_error_if_pass(db_stmt_is_executing(stmt), E_DB_ILLEGAL, -1);

    return error_pass_int(bind(stmt, index, &info));
}

int db_param_bind_longlong_ref(db_stmt_ct stmt, size_t index, const long long *value, const bool *is_null)
{
    db_param_bind_cb bind = db_stmt_get_interface(stmt)->param_bind;
    db_param_bind_st info =
    {
        .mode       = DB_PARAM_BIND_REF,
        .type       = db_inttype(sizeof(long long), true),
        .data.blob  = (long long *)value,
        .vsize      = sizeof(long long),
        .rsize      = &info.vsize,
        .is_null    = is_null,
    };

    return_error_if_fail(bind, E_DB_UNSUPPORTED, -1);
    return_error_if_pass(db_stmt_is_executing(stmt), E_DB_ILLEGAL, -1);

    return error_pass_int(bind(stmt, index, &info));
}

int db_param_bind_int8(db_stmt_ct stmt, size_t index, int8_t value)
{
    db_param_bind_cb bind = db_stmt_get_interface(stmt)->param_bind;
    db_param_bind_st info =
    {
        .mode       = DB_PARAM_BIND_TMP,
        .type       = DB_TYPE_INT8,
        .data.i8    = &value,
        .vsize      = sizeof(int8_t),
    };

    return_error_if_fail(bind, E_DB_UNSUPPORTED, -1);
    return_error_if_pass(db_stmt_is_executing(stmt), E_DB_ILLEGAL, -1);

    return error_pass_int(bind(stmt, index, &info));
}

int db_param_bind_int8_ref(db_stmt_ct stmt, size_t index, const int8_t *value, const bool *is_null)
{
    db_param_bind_cb bind = db_stmt_get_interface(stmt)->param_bind;
    db_param_bind_st info =
    {
        .mode       = DB_PARAM_BIND_REF,
        .type       = DB_TYPE_INT8,
        .data.i8    = (int8_t *)value,
        .vsize      = sizeof(int8_t),
        .rsize      = &info.vsize,
        .is_null    = is_null,
    };

    return_error_if_fail(bind, E_DB_UNSUPPORTED, -1);
    return_error_if_pass(db_stmt_is_executing(stmt), E_DB_ILLEGAL, -1);

    return error_pass_int(bind(stmt, index, &info));
}

int db_param_bind_int16(db_stmt_ct stmt, size_t index, int16_t value)
{
    db_param_bind_cb bind = db_stmt_get_interface(stmt)->param_bind;
    db_param_bind_st info =
    {
        .mode       = DB_PARAM_BIND_TMP,
        .type       = DB_TYPE_INT16,
        .data.i16   = &value,
        .vsize      = sizeof(int16_t),
    };

    return_error_if_fail(bind, E_DB_UNSUPPORTED, -1);
    return_error_if_pass(db_stmt_is_executing(stmt), E_DB_ILLEGAL, -1);

    return error_pass_int(bind(stmt, index, &info));
}

int db_param_bind_int16_ref(db_stmt_ct stmt, size_t index, const int16_t *value, const bool *is_null)
{
    db_param_bind_cb bind = db_stmt_get_interface(stmt)->param_bind;
    db_param_bind_st info =
    {
        .mode       = DB_PARAM_BIND_REF,
        .type       = DB_TYPE_INT16,
        .data.i16   = (int16_t *)value,
        .vsize      = sizeof(int16_t),
        .rsize      = &info.vsize,
        .is_null    = is_null,
    };

    return_error_if_fail(bind, E_DB_UNSUPPORTED, -1);
    return_error_if_pass(db_stmt_is_executing(stmt), E_DB_ILLEGAL, -1);

    return error_pass_int(bind(stmt, index, &info));
}

int db_param_bind_int32(db_stmt_ct stmt, size_t index, int32_t value)
{
    db_param_bind_cb bind = db_stmt_get_interface(stmt)->param_bind;
    db_param_bind_st info =
    {
        .mode       = DB_PARAM_BIND_TMP,
        .type       = DB_TYPE_INT32,
        .data.i32   = &value,
        .vsize      = sizeof(int32_t),
    };

    return_error_if_fail(bind, E_DB_UNSUPPORTED, -1);
    return_error_if_pass(db_stmt_is_executing(stmt), E_DB_ILLEGAL, -1);

    return error_pass_int(bind(stmt, index, &info));
}

int db_param_bind_int32_ref(db_stmt_ct stmt, size_t index, const int32_t *value, const bool *is_null)
{
    db_param_bind_cb bind = db_stmt_get_interface(stmt)->param_bind;
    db_param_bind_st info =
    {
        .mode       = DB_PARAM_BIND_REF,
        .type       = DB_TYPE_INT32,
        .data.i32   = (int32_t *)value,
        .vsize      = sizeof(int32_t),
        .rsize      = &info.vsize,
        .is_null    = is_null,
    };

    return_error_if_fail(bind, E_DB_UNSUPPORTED, -1);
    return_error_if_pass(db_stmt_is_executing(stmt), E_DB_ILLEGAL, -1);

    return error_pass_int(bind(stmt, index, &info));
}

int db_param_bind_int64(db_stmt_ct stmt, size_t index, int64_t value)
{
    db_param_bind_cb bind = db_stmt_get_interface(stmt)->param_bind;
    db_param_bind_st info =
    {
        .mode       = DB_PARAM_BIND_TMP,
        .type       = DB_TYPE_INT64,
        .data.i64   = &value,
        .vsize      = sizeof(int64_t),
    };

    return_error_if_fail(bind, E_DB_UNSUPPORTED, -1);
    return_error_if_pass(db_stmt_is_executing(stmt), E_DB_ILLEGAL, -1);

    return error_pass_int(bind(stmt, index, &info));
}

int db_param_bind_int64_ref(db_stmt_ct stmt, size_t index, const int64_t *value, const bool *is_null)
{
    db_param_bind_cb bind = db_stmt_get_interface(stmt)->param_bind;
    db_param_bind_st info =
    {
        .mode       = DB_PARAM_BIND_REF,
        .type       = DB_TYPE_INT64,
        .data.i64   = (int64_t *)value,
        .vsize      = sizeof(int64_t),
        .rsize      = &info.vsize,
        .is_null    = is_null,
    };

    return_error_if_fail(bind, E_DB_UNSUPPORTED, -1);
    return_error_if_pass(db_stmt_is_executing(stmt), E_DB_ILLEGAL, -1);

    return error_pass_int(bind(stmt, index, &info));
}

int db_param_bind_uchar(db_stmt_ct stmt, size_t index, unsigned char value)
{
    db_param_bind_cb bind = db_stmt_get_interface(stmt)->param_bind;
    db_param_bind_st info =
    {
        .mode       = DB_PARAM_BIND_TMP,
        .type       = db_inttype(sizeof(unsigned char), false),
        .data.blob  = &value,
        .vsize      = sizeof(unsigned char),
    };

    return_error_if_fail(bind, E_DB_UNSUPPORTED, -1);
    return_error_if_pass(db_stmt_is_executing(stmt), E_DB_ILLEGAL, -1);

    return error_pass_int(bind(stmt, index, &info));
}

int db_param_bind_uchar_ref(db_stmt_ct stmt, size_t index, const unsigned char *value, const bool *is_null)
{
    db_param_bind_cb bind = db_stmt_get_interface(stmt)->param_bind;
    db_param_bind_st info =
    {
        .mode       = DB_PARAM_BIND_REF,
        .type       = db_inttype(sizeof(unsigned char), false),
        .data.blob  = (unsigned char *)value,
        .vsize      = sizeof(unsigned char),
        .rsize      = &info.vsize,
        .is_null    = is_null,
    };

    return_error_if_fail(bind, E_DB_UNSUPPORTED, -1);
    return_error_if_pass(db_stmt_is_executing(stmt), E_DB_ILLEGAL, -1);

    return error_pass_int(bind(stmt, index, &info));
}

int db_param_bind_ushort(db_stmt_ct stmt, size_t index, unsigned short value)
{
    db_param_bind_cb bind = db_stmt_get_interface(stmt)->param_bind;
    db_param_bind_st info =
    {
        .mode       = DB_PARAM_BIND_TMP,
        .type       = db_inttype(sizeof(unsigned short), false),
        .data.blob  = &value,
        .vsize      = sizeof(unsigned short),
    };

    return_error_if_fail(bind, E_DB_UNSUPPORTED, -1);
    return_error_if_pass(db_stmt_is_executing(stmt), E_DB_ILLEGAL, -1);

    return error_pass_int(bind(stmt, index, &info));
}

int db_param_bind_ushort_ref(db_stmt_ct stmt, size_t index, const unsigned short *value, const bool *is_null)
{
    db_param_bind_cb bind = db_stmt_get_interface(stmt)->param_bind;
    db_param_bind_st info =
    {
        .mode       = DB_PARAM_BIND_REF,
        .type       = db_inttype(sizeof(unsigned short), false),
        .data.blob  = (unsigned short *)value,
        .vsize      = sizeof(unsigned short),
        .rsize      = &info.vsize,
        .is_null    = is_null,
    };

    return_error_if_fail(bind, E_DB_UNSUPPORTED, -1);
    return_error_if_pass(db_stmt_is_executing(stmt), E_DB_ILLEGAL, -1);

    return error_pass_int(bind(stmt, index, &info));
}

int db_param_bind_uint(db_stmt_ct stmt, size_t index, unsigned int value)
{
    db_param_bind_cb bind = db_stmt_get_interface(stmt)->param_bind;
    db_param_bind_st info =
    {
        .mode       = DB_PARAM_BIND_TMP,
        .type       = db_inttype(sizeof(unsigned int), false),
        .data.blob  = &value,
        .vsize      = sizeof(unsigned int),
    };

    return_error_if_fail(bind, E_DB_UNSUPPORTED, -1);
    return_error_if_pass(db_stmt_is_executing(stmt), E_DB_ILLEGAL, -1);

    return error_pass_int(bind(stmt, index, &info));
}

int db_param_bind_uint_ref(db_stmt_ct stmt, size_t index, const unsigned int *value, const bool *is_null)
{
    db_param_bind_cb bind = db_stmt_get_interface(stmt)->param_bind;
    db_param_bind_st info =
    {
        .mode       = DB_PARAM_BIND_REF,
        .type       = db_inttype(sizeof(unsigned int), false),
        .data.blob  = (unsigned int *)value,
        .vsize      = sizeof(unsigned int),
        .rsize      = &info.vsize,
        .is_null    = is_null,
    };

    return_error_if_fail(bind, E_DB_UNSUPPORTED, -1);
    return_error_if_pass(db_stmt_is_executing(stmt), E_DB_ILLEGAL, -1);

    return error_pass_int(bind(stmt, index, &info));
}

int db_param_bind_ulong(db_stmt_ct stmt, size_t index, unsigned long value)
{
    db_param_bind_cb bind = db_stmt_get_interface(stmt)->param_bind;
    db_param_bind_st info =
    {
        .mode       = DB_PARAM_BIND_TMP,
        .type       = db_inttype(sizeof(unsigned long), false),
        .data.blob  = &value,
        .vsize      = sizeof(unsigned long),
    };

    return_error_if_fail(bind, E_DB_UNSUPPORTED, -1);
    return_error_if_pass(db_stmt_is_executing(stmt), E_DB_ILLEGAL, -1);

    return error_pass_int(bind(stmt, index, &info));
}

int db_param_bind_ulong_ref(db_stmt_ct stmt, size_t index, const unsigned long *value, const bool *is_null)
{
    db_param_bind_cb bind = db_stmt_get_interface(stmt)->param_bind;
    db_param_bind_st info =
    {
        .mode       = DB_PARAM_BIND_REF,
        .type       = db_inttype(sizeof(unsigned long), false),
        .data.blob  = (unsigned long *)value,
        .vsize      = sizeof(unsigned long),
        .rsize      = &info.vsize,
        .is_null    = is_null,
    };

    return_error_if_fail(bind, E_DB_UNSUPPORTED, -1);
    return_error_if_pass(db_stmt_is_executing(stmt), E_DB_ILLEGAL, -1);

    return error_pass_int(bind(stmt, index, &info));
}

int db_param_bind_ulonglong(db_stmt_ct stmt, size_t index, unsigned long long value)
{
    db_param_bind_cb bind = db_stmt_get_interface(stmt)->param_bind;
    db_param_bind_st info =
    {
        .mode       = DB_PARAM_BIND_TMP,
        .type       = db_inttype(sizeof(unsigned long long), false),
        .data.blob  = &value,
        .vsize      = sizeof(unsigned long long),
    };

    return_error_if_fail(bind, E_DB_UNSUPPORTED, -1);
    return_error_if_pass(db_stmt_is_executing(stmt), E_DB_ILLEGAL, -1);

    return error_pass_int(bind(stmt, index, &info));
}

int db_param_bind_ulonglong_ref(db_stmt_ct stmt, size_t index, const unsigned long long *value, const bool *is_null)
{
    db_param_bind_cb bind = db_stmt_get_interface(stmt)->param_bind;
    db_param_bind_st info =
    {
        .mode       = DB_PARAM_BIND_REF,
        .type       = db_inttype(sizeof(unsigned long long), false),
        .data.blob  = (unsigned long long *)value,
        .vsize      = sizeof(unsigned long long),
        .rsize      = &info.vsize,
        .is_null    = is_null,
    };

    return_error_if_fail(bind, E_DB_UNSUPPORTED, -1);
    return_error_if_pass(db_stmt_is_executing(stmt), E_DB_ILLEGAL, -1);

    return error_pass_int(bind(stmt, index, &info));
}

int db_param_bind_uint8(db_stmt_ct stmt, size_t index, uint8_t value)
{
    db_param_bind_cb bind = db_stmt_get_interface(stmt)->param_bind;
    db_param_bind_st info =
    {
        .mode       = DB_PARAM_BIND_TMP,
        .type       = DB_TYPE_UINT8,
        .data.u8    = &value,
        .vsize      = sizeof(uint8_t),
    };

    return_error_if_fail(bind, E_DB_UNSUPPORTED, -1);
    return_error_if_pass(db_stmt_is_executing(stmt), E_DB_ILLEGAL, -1);

    return error_pass_int(bind(stmt, index, &info));
}

int db_param_bind_uint8_ref(db_stmt_ct stmt, size_t index, const uint8_t *value, const bool *is_null)
{
    db_param_bind_cb bind = db_stmt_get_interface(stmt)->param_bind;
    db_param_bind_st info =
    {
        .mode       = DB_PARAM_BIND_REF,
        .type       = DB_TYPE_UINT8,
        .data.u8    = (uint8_t *)value,
        .vsize      = sizeof(uint8_t),
        .rsize      = &info.vsize,
        .is_null    = is_null,
    };

    return_error_if_fail(bind, E_DB_UNSUPPORTED, -1);
    return_error_if_pass(db_stmt_is_executing(stmt), E_DB_ILLEGAL, -1);

    return error_pass_int(bind(stmt, index, &info));
}

int db_param_bind_uint16(db_stmt_ct stmt, size_t index, uint16_t value)
{
    db_param_bind_cb bind = db_stmt_get_interface(stmt)->param_bind;
    db_param_bind_st info =
    {
        .mode       = DB_PARAM_BIND_TMP,
        .type       = DB_TYPE_UINT16,
        .data.u16   = &value,
        .vsize      = sizeof(uint16_t),
    };

    return_error_if_fail(bind, E_DB_UNSUPPORTED, -1);
    return_error_if_pass(db_stmt_is_executing(stmt), E_DB_ILLEGAL, -1);

    return error_pass_int(bind(stmt, index, &info));
}

int db_param_bind_uint16_ref(db_stmt_ct stmt, size_t index, const uint16_t *value, const bool *is_null)
{
    db_param_bind_cb bind = db_stmt_get_interface(stmt)->param_bind;
    db_param_bind_st info =
    {
        .mode       = DB_PARAM_BIND_REF,
        .type       = DB_TYPE_UINT16,
        .data.u16   = (uint16_t *)value,
        .vsize      = sizeof(uint16_t),
        .rsize      = &info.vsize,
        .is_null    = is_null,
    };

    return_error_if_fail(bind, E_DB_UNSUPPORTED, -1);
    return_error_if_pass(db_stmt_is_executing(stmt), E_DB_ILLEGAL, -1);

    return error_pass_int(bind(stmt, index, &info));
}

int db_param_bind_uint32(db_stmt_ct stmt, size_t index, uint32_t value)
{
    db_param_bind_cb bind = db_stmt_get_interface(stmt)->param_bind;
    db_param_bind_st info =
    {
        .mode       = DB_PARAM_BIND_TMP,
        .type       = DB_TYPE_UINT32,
        .data.u32   = &value,
        .vsize      = sizeof(uint32_t),
    };

    return_error_if_fail(bind, E_DB_UNSUPPORTED, -1);
    return_error_if_pass(db_stmt_is_executing(stmt), E_DB_ILLEGAL, -1);

    return error_pass_int(bind(stmt, index, &info));
}

int db_param_bind_uint32_ref(db_stmt_ct stmt, size_t index, const uint32_t *value, const bool *is_null)
{
    db_param_bind_cb bind = db_stmt_get_interface(stmt)->param_bind;
    db_param_bind_st info =
    {
        .mode       = DB_PARAM_BIND_REF,
        .type       = DB_TYPE_UINT32,
        .data.u32   = (uint32_t *)value,
        .vsize      = sizeof(uint32_t),
        .rsize      = &info.vsize,
        .is_null    = is_null,
    };

    return_error_if_fail(bind, E_DB_UNSUPPORTED, -1);
    return_error_if_pass(db_stmt_is_executing(stmt), E_DB_ILLEGAL, -1);

    return error_pass_int(bind(stmt, index, &info));
}

int db_param_bind_uint64(db_stmt_ct stmt, size_t index, uint64_t value)
{
    db_param_bind_cb bind = db_stmt_get_interface(stmt)->param_bind;
    db_param_bind_st info =
    {
        .mode       = DB_PARAM_BIND_TMP,
        .type       = DB_TYPE_UINT64,
        .data.u64   = &value,
        .vsize      = sizeof(uint64_t),
    };

    return_error_if_fail(bind, E_DB_UNSUPPORTED, -1);
    return_error_if_pass(db_stmt_is_executing(stmt), E_DB_ILLEGAL, -1);

    return error_pass_int(bind(stmt, index, &info));
}

int db_param_bind_uint64_ref(db_stmt_ct stmt, size_t index, const uint64_t *value, const bool *is_null)
{
    db_param_bind_cb bind = db_stmt_get_interface(stmt)->param_bind;
    db_param_bind_st info =
    {
        .mode       = DB_PARAM_BIND_REF,
        .type       = DB_TYPE_UINT64,
        .data.u64   = (uint64_t *)value,
        .vsize      = sizeof(uint64_t),
        .rsize      = &info.vsize,
        .is_null    = is_null,
    };

    return_error_if_fail(bind, E_DB_UNSUPPORTED, -1);
    return_error_if_pass(db_stmt_is_executing(stmt), E_DB_ILLEGAL, -1);

    return error_pass_int(bind(stmt, index, &info));
}

int db_param_bind_float(db_stmt_ct stmt, size_t index, float value)
{
    db_param_bind_cb bind = db_stmt_get_interface(stmt)->param_bind;
    db_param_bind_st info =
    {
        .mode   = DB_PARAM_BIND_TMP,
        .type   = DB_TYPE_FLOAT,
        .data.f = &value,
        .vsize  = sizeof(float),
    };

    return_error_if_fail(bind, E_DB_UNSUPPORTED, -1);
    return_error_if_pass(db_stmt_is_executing(stmt), E_DB_ILLEGAL, -1);

    return error_pass_int(bind(stmt, index, &info));
}

int db_param_bind_float_ref(db_stmt_ct stmt, size_t index, const float *value, const bool *is_null)
{
    db_param_bind_cb bind = db_stmt_get_interface(stmt)->param_bind;
    db_param_bind_st info =
    {
        .mode       = DB_PARAM_BIND_REF,
        .type       = DB_TYPE_FLOAT,
        .data.f     = (float *)value,
        .vsize      = sizeof(float),
        .rsize      = &info.vsize,
        .is_null    = is_null,
    };

    return_error_if_fail(bind, E_DB_UNSUPPORTED, -1);
    return_error_if_pass(db_stmt_is_executing(stmt), E_DB_ILLEGAL, -1);

    return error_pass_int(bind(stmt, index, &info));
}

int db_param_bind_double(db_stmt_ct stmt, size_t index, double value)
{
    db_param_bind_cb bind = db_stmt_get_interface(stmt)->param_bind;
    db_param_bind_st info =
    {
        .mode   = DB_PARAM_BIND_TMP,
        .type   = DB_TYPE_DOUBLE,
        .data.d = &value,
        .vsize  = sizeof(double),
    };

    return_error_if_fail(bind, E_DB_UNSUPPORTED, -1);
    return_error_if_pass(db_stmt_is_executing(stmt), E_DB_ILLEGAL, -1);

    return error_pass_int(bind(stmt, index, &info));
}

int db_param_bind_double_ref(db_stmt_ct stmt, size_t index, const double *value, const bool *is_null)
{
    db_param_bind_cb bind = db_stmt_get_interface(stmt)->param_bind;
    db_param_bind_st info =
    {
        .mode       = DB_PARAM_BIND_REF,
        .type       = DB_TYPE_DOUBLE,
        .data.d     = (double *)value,
        .vsize      = sizeof(double),
        .rsize      = &info.vsize,
        .is_null    = is_null,
    };

    return_error_if_fail(bind, E_DB_UNSUPPORTED, -1);
    return_error_if_pass(db_stmt_is_executing(stmt), E_DB_ILLEGAL, -1);

    return error_pass_int(bind(stmt, index, &info));
}

int db_param_bind_ldouble(db_stmt_ct stmt, size_t index, long double value)
{
    db_param_bind_cb bind = db_stmt_get_interface(stmt)->param_bind;
    db_param_bind_st info =
    {
        .mode       = DB_PARAM_BIND_TMP,
        .type       = DB_TYPE_LDOUBLE,
        .data.ld    = &value,
        .vsize      = sizeof(long double),
    };

    return_error_if_fail(bind, E_DB_UNSUPPORTED, -1);
    return_error_if_pass(db_stmt_is_executing(stmt), E_DB_ILLEGAL, -1);

    return error_pass_int(bind(stmt, index, &info));
}

int db_param_bind_ldouble_ref(db_stmt_ct stmt, size_t index, const long double *value, const bool *is_null)
{
    db_param_bind_cb bind = db_stmt_get_interface(stmt)->param_bind;
    db_param_bind_st info =
    {
        .mode       = DB_PARAM_BIND_REF,
        .type       = DB_TYPE_LDOUBLE,
        .data.ld    = (long double *)value,
        .vsize      = sizeof(long double),
        .rsize      = &info.vsize,
        .is_null    = is_null,
    };

    return_error_if_fail(bind, E_DB_UNSUPPORTED, -1);
    return_error_if_pass(db_stmt_is_executing(stmt), E_DB_ILLEGAL, -1);

    return error_pass_int(bind(stmt, index, &info));
}

int db_param_bind_text(db_stmt_ct stmt, size_t index, const char *text, ssize_t size)
{
    db_param_bind_cb bind = db_stmt_get_interface(stmt)->param_bind;
    db_param_bind_st info =
    {
        .mode       = DB_PARAM_BIND_FIX,
        .type       = DB_TYPE_TEXT,
        .data.text  = (char *)text,
        .vsize      = !text ? 0 : size < 0 ? strlen(text) : (size_t)size,
    };

    return_error_if_fail(bind, E_DB_UNSUPPORTED, -1);
    return_error_if_pass(db_stmt_is_executing(stmt), E_DB_ILLEGAL, -1);

    return error_pass_int(bind(stmt, index, &info));
}

int db_param_bind_text_tmp(db_stmt_ct stmt, size_t index, const char *text, ssize_t size)
{
    db_param_bind_cb bind = db_stmt_get_interface(stmt)->param_bind;
    db_param_bind_st info =
    {
        .mode       = DB_PARAM_BIND_TMP,
        .type       = DB_TYPE_TEXT,
        .data.text  = (char *)text,
        .vsize      = !text ? 0 : size < 0 ? strlen(text) : (size_t)size,
    };

    return_error_if_fail(bind, E_DB_UNSUPPORTED, -1);
    return_error_if_pass(db_stmt_is_executing(stmt), E_DB_ILLEGAL, -1);

    return error_pass_int(bind(stmt, index, &info));
}

int db_param_bind_text_ref(db_stmt_ct stmt, size_t index, const char *const *text, const size_t *size)
{
    db_param_bind_cb bind = db_stmt_get_interface(stmt)->param_bind;
    db_param_bind_st info =
    {
        .mode       = DB_PARAM_BIND_REF,
        .type       = DB_TYPE_TEXT,
        .data.ptext = (char **)text,
        .rsize      = size,
    };

    assert(text);

    return_error_if_fail(bind, E_DB_UNSUPPORTED, -1);
    return_error_if_pass(db_stmt_is_executing(stmt), E_DB_ILLEGAL, -1);

    return error_pass_int(bind(stmt, index, &info));
}

int db_param_bind_blob(db_stmt_ct stmt, size_t index, const void *blob, size_t size)
{
    db_param_bind_cb bind = db_stmt_get_interface(stmt)->param_bind;
    db_param_bind_st info =
    {
        .mode       = DB_PARAM_BIND_FIX,
        .type       = DB_TYPE_BLOB,
        .data.blob  = (void *)blob,
        .vsize      = size,
    };

    return_error_if_fail(bind, E_DB_UNSUPPORTED, -1);
    return_error_if_pass(db_stmt_is_executing(stmt), E_DB_ILLEGAL, -1);

    return error_pass_int(bind(stmt, index, &info));
}

int db_param_bind_blob_tmp(db_stmt_ct stmt, size_t index, const void *blob, size_t size)
{
    db_param_bind_cb bind = db_stmt_get_interface(stmt)->param_bind;
    db_param_bind_st info =
    {
        .mode       = DB_PARAM_BIND_TMP,
        .type       = DB_TYPE_BLOB,
        .data.blob  = (void *)blob,
        .vsize      = size,
    };

    return_error_if_fail(bind, E_DB_UNSUPPORTED, -1);
    return_error_if_pass(db_stmt_is_executing(stmt), E_DB_ILLEGAL, -1);

    return error_pass_int(bind(stmt, index, &info));
}

int db_param_bind_blob_ref(db_stmt_ct stmt, size_t index, const void *const *blob, const size_t *size)
{
    db_param_bind_cb bind = db_stmt_get_interface(stmt)->param_bind;
    db_param_bind_st info =
    {
        .mode       = DB_PARAM_BIND_REF,
        .type       = DB_TYPE_BLOB,
        .data.pblob = (void **)blob,
        .rsize      = size,
    };

    assert(blob);
    assert(size);

    return_error_if_fail(bind, E_DB_UNSUPPORTED, -1);
    return_error_if_pass(db_stmt_is_executing(stmt), E_DB_ILLEGAL, -1);

    return error_pass_int(bind(stmt, index, &info));
}

/// Check if date and/or time is valid.
///
/// \param date     date to check, may be NULL
/// \param time     time to check, may be NULL
///
/// \retval true    date and/or time is valid
/// \retval false   date and/or time is not valid
static bool db_dt_is_valid(const db_date_st *date, const db_time_st *time)
{
    if(date)
    {
        return_value_if_fail(RANGE(date->month, 1, 12), false);
        return_value_if_fail(RANGE(date->day, 1, 31), false);
    }

    if(time)
    {
        return_value_if_fail(RANGE(time->hour, 0, 23), false);
        return_value_if_fail(RANGE(time->minute, 0, 59), false);
        return_value_if_fail(RANGE(time->second, 0, 59), false);
    }

    return true;
}

int db_param_bind_date(db_stmt_ct stmt, size_t index, const db_date_st *date)
{
    db_param_bind_cb bind = db_stmt_get_interface(stmt)->param_bind;
    db_param_bind_st info =
    {
        .mode       = DB_PARAM_BIND_FIX,
        .type       = DB_TYPE_DATE,
        .data.date  = (db_date_st *)date,
        .vsize      = sizeof(db_date_st),
    };

    return_error_if_fail(bind, E_DB_UNSUPPORTED, -1);
    return_error_if_pass(db_stmt_is_executing(stmt), E_DB_ILLEGAL, -1);
    return_error_if_fail(db_dt_is_valid(date, NULL), E_DB_OUT_OF_RANGE, -1);

    return error_pass_int(bind(stmt, index, &info));
}

int db_param_bind_date_tmp(db_stmt_ct stmt, size_t index, const db_date_st *date)
{
    db_param_bind_cb bind = db_stmt_get_interface(stmt)->param_bind;
    db_param_bind_st info =
    {
        .mode       = DB_PARAM_BIND_TMP,
        .type       = DB_TYPE_DATE,
        .data.date  = (db_date_st *)date,
        .vsize      = sizeof(db_date_st),
    };

    return_error_if_fail(bind, E_DB_UNSUPPORTED, -1);
    return_error_if_pass(db_stmt_is_executing(stmt), E_DB_ILLEGAL, -1);
    return_error_if_fail(db_dt_is_valid(date, NULL), E_DB_OUT_OF_RANGE, -1);

    return error_pass_int(bind(stmt, index, &info));
}

int db_param_bind_date_full(db_stmt_ct stmt, size_t index, int16_t year, uint8_t month, uint8_t day)
{
    db_date_st date = { .year = year, .month = month, .day = day };

    return error_pass_int(db_param_bind_date_tmp(stmt, index, &date));
}

int db_param_bind_date_ref(db_stmt_ct stmt, size_t index, const db_date_st *date, const bool *is_null)
{
    db_param_bind_cb bind = db_stmt_get_interface(stmt)->param_bind;
    db_param_bind_st info =
    {
        .mode       = DB_PARAM_BIND_REF,
        .type       = DB_TYPE_DATE,
        .data.date  = (db_date_st *)date,
        .vsize      = sizeof(db_date_st),
        .rsize      = &info.vsize,
        .is_null    = is_null,
    };

    assert(date);

    return_error_if_fail(bind, E_DB_UNSUPPORTED, -1);
    return_error_if_pass(db_stmt_is_executing(stmt), E_DB_ILLEGAL, -1);
    return_error_if_fail(db_dt_is_valid(date, NULL), E_DB_OUT_OF_RANGE, -1);

    return error_pass_int(bind(stmt, index, &info));
}

int db_param_bind_time(db_stmt_ct stmt, size_t index, const db_time_st *time)
{
    db_param_bind_cb bind = db_stmt_get_interface(stmt)->param_bind;
    db_param_bind_st info =
    {
        .mode       = DB_PARAM_BIND_FIX,
        .type       = DB_TYPE_TIME,
        .data.time  = (db_time_st *)time,
        .vsize      = sizeof(db_time_st),
    };

    return_error_if_fail(bind, E_DB_UNSUPPORTED, -1);
    return_error_if_pass(db_stmt_is_executing(stmt), E_DB_ILLEGAL, -1);
    return_error_if_fail(db_dt_is_valid(NULL, time), E_DB_OUT_OF_RANGE, -1);

    return error_pass_int(bind(stmt, index, &info));
}

int db_param_bind_time_tmp(db_stmt_ct stmt, size_t index, const db_time_st *time)
{
    db_param_bind_cb bind = db_stmt_get_interface(stmt)->param_bind;
    db_param_bind_st info =
    {
        .mode       = DB_PARAM_BIND_TMP,
        .type       = DB_TYPE_TIME,
        .data.time  = (db_time_st *)time,
        .vsize      = sizeof(db_time_st),
    };

    return_error_if_fail(bind, E_DB_UNSUPPORTED, -1);
    return_error_if_pass(db_stmt_is_executing(stmt), E_DB_ILLEGAL, -1);
    return_error_if_fail(db_dt_is_valid(NULL, time), E_DB_OUT_OF_RANGE, -1);

    return error_pass_int(bind(stmt, index, &info));
}

int db_param_bind_time_full(db_stmt_ct stmt, size_t index, uint8_t hour, uint8_t minute, uint8_t second)
{
    db_time_st time = { .hour = hour, .minute = minute, .second = second };

    return error_pass_int(db_param_bind_time_tmp(stmt, index, &time));
}

int db_param_bind_time_ref(db_stmt_ct stmt, size_t index, const db_time_st *time, const bool *is_null)
{
    db_param_bind_cb bind = db_stmt_get_interface(stmt)->param_bind;
    db_param_bind_st info =
    {
        .mode       = DB_PARAM_BIND_REF,
        .type       = DB_TYPE_TIME,
        .data.time  = (db_time_st *)time,
        .vsize      = sizeof(db_time_st),
        .rsize      = &info.vsize,
        .is_null    = is_null,
    };

    assert(time);

    return_error_if_fail(bind, E_DB_UNSUPPORTED, -1);
    return_error_if_pass(db_stmt_is_executing(stmt), E_DB_ILLEGAL, -1);
    return_error_if_fail(db_dt_is_valid(NULL, time), E_DB_OUT_OF_RANGE, -1);

    return error_pass_int(bind(stmt, index, &info));
}

int db_param_bind_datetime(db_stmt_ct stmt, size_t index, const db_datetime_st *datetime)
{
    db_param_bind_cb bind = db_stmt_get_interface(stmt)->param_bind;
    db_param_bind_st info =
    {
        .mode       = DB_PARAM_BIND_FIX,
        .type       = DB_TYPE_DATETIME,
        .data.dt    = (db_datetime_st *)datetime,
        .vsize      = sizeof(db_datetime_st),
    };

    return_error_if_fail(bind, E_DB_UNSUPPORTED, -1);
    return_error_if_pass(db_stmt_is_executing(stmt), E_DB_ILLEGAL, -1);
    return_error_if_fail(db_dt_is_valid(&datetime->date, &datetime->time), E_DB_OUT_OF_RANGE, -1);

    return error_pass_int(bind(stmt, index, &info));
}

int db_param_bind_datetime_tmp(db_stmt_ct stmt, size_t index, const db_datetime_st *datetime)
{
    db_param_bind_cb bind = db_stmt_get_interface(stmt)->param_bind;
    db_param_bind_st info =
    {
        .mode       = DB_PARAM_BIND_TMP,
        .type       = DB_TYPE_DATETIME,
        .data.dt    = (db_datetime_st *)datetime,
        .vsize      = sizeof(db_datetime_st),
    };

    return_error_if_fail(bind, E_DB_UNSUPPORTED, -1);
    return_error_if_pass(db_stmt_is_executing(stmt), E_DB_ILLEGAL, -1);
    return_error_if_fail(db_dt_is_valid(&datetime->date, &datetime->time), E_DB_OUT_OF_RANGE, -1);

    return error_pass_int(bind(stmt, index, &info));
}

int db_param_bind_datetime_full(db_stmt_ct stmt, size_t index, int16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second)
{
    db_datetime_st datetime =
    {
        .date.year  = year, .date.month  = month,  .date.day    = day,
        .time.hour  = hour, .time.minute = minute, .time.second = second
    };

    return error_pass_int(db_param_bind_datetime_tmp(stmt, index, &datetime));
}

int db_param_bind_datetime_ref(db_stmt_ct stmt, size_t index, const db_datetime_st *datetime, const bool *is_null)
{
    db_param_bind_cb bind = db_stmt_get_interface(stmt)->param_bind;
    db_param_bind_st info =
    {
        .mode       = DB_PARAM_BIND_REF,
        .type       = DB_TYPE_DATETIME,
        .data.dt    = (db_datetime_st *)datetime,
        .vsize      = sizeof(db_datetime_st),
        .rsize      = &info.vsize,
        .is_null    = is_null,
    };

    assert(datetime);

    return_error_if_fail(bind, E_DB_UNSUPPORTED, -1);
    return_error_if_pass(db_stmt_is_executing(stmt), E_DB_ILLEGAL, -1);
    return_error_if_fail(db_dt_is_valid(&datetime->date, &datetime->time), E_DB_OUT_OF_RANGE, -1);

    return error_pass_int(bind(stmt, index, &info));
}

int db_param_bind_timestamp(db_stmt_ct stmt, size_t index, time_t timestamp)
{
    db_param_bind_cb bind = db_stmt_get_interface(stmt)->param_bind;
    db_param_bind_st info =
    {
        .mode       = DB_PARAM_BIND_TMP,
        .type       = DB_TYPE_TIMESTAMP,
        .data.ts    = &timestamp,
        .vsize      = sizeof(time_t),
    };

    return_error_if_fail(bind, E_DB_UNSUPPORTED, -1);
    return_error_if_pass(db_stmt_is_executing(stmt), E_DB_ILLEGAL, -1);

    return error_pass_int(bind(stmt, index, &info));
}

int db_param_bind_timestamp_ref(db_stmt_ct stmt, size_t index, const time_t *timestamp, const bool *is_null)
{
    db_param_bind_cb bind = db_stmt_get_interface(stmt)->param_bind;
    db_param_bind_st info =
    {
        .mode       = DB_PARAM_BIND_REF,
        .type       = DB_TYPE_TIMESTAMP,
        .data.ts    = (time_t *)timestamp,
        .vsize      = sizeof(time_t),
        .rsize      = &info.vsize,
        .is_null    = is_null,
    };

    return_error_if_fail(bind, E_DB_UNSUPPORTED, -1);
    return_error_if_pass(db_stmt_is_executing(stmt), E_DB_ILLEGAL, -1);

    return error_pass_int(bind(stmt, index, &info));
}
