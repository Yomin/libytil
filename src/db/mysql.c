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

#include <ytil/db/mysql.h>
#include <ytil/db/interface.h>
#include <ytil/ext/string.h>
#include <ytil/ext/time.h>
#include <ytil/def.h>
#include <mysql/mysql.h>
#include <mysql/errmsg.h>
#include <mysql/mysqld_error.h>
#include <stdlib.h>


/// MySQL statement
typedef struct db_mysql_stmt
{
    MYSQL_STMT          *stmt;      ///< statement
    MYSQL_BIND          *param;     ///< MySQL parameter bindings
    MYSQL_BIND          *result;    ///< MySQL result bindings
    MYSQL_RES           *meta;      ///< result meta infos
} db_mysql_stmt_st;


static_assert(sizeof(bool) == sizeof(my_bool), "bool type mismatch");


/// Error out-of-memory check callback to check MySQL codes.
///
/// \implements error_oom_cb
static bool error_mysql_is_oom(const error_type_st *type, int code)
{
    return code == CR_OUT_OF_MEMORY;
}

/// Error retrieval callback to retrieve last MySQL code from a DB or STMT object.
///
/// \implements error_last_cb
static int error_mysql_last(const error_type_st *type, const char **desc, const char *ctx_type, void *ctx)
{
    db_mysql_stmt_st *mystmt;
    MYSQL *mydb;

    if(!strcmp(ctx_type, "db"))
    {
        mydb = db_get_ctx(ctx);

        if(desc)
            *desc = mysql_error(mydb);

        return mysql_errno(mydb);
    }

    if(!strcmp(ctx_type, "stmt"))
    {
        mystmt = db_stmt_get_ctx(ctx);

        if(desc)
            *desc = mysql_stmt_error(mystmt->stmt);

        return mysql_stmt_errno(mystmt->stmt);
    }

    abort();
}

/// MySQL error type definition
ERROR_DEFINE_CALLBACK(MYSQL, NULL, NULL, error_mysql_is_oom, error_mysql_last);

/// Error map callback to map MySQL error codes to DB error codes.
///
/// \implements error_map_cb
static int error_mysql_map(const error_type_st *type, int code, void *ctx)
{
    switch(code)
    {
    case CR_CONN_HOST_ERROR:
    case CR_CONNECTION_ERROR:
    case CR_IPSOCK_ERROR:
    case CR_SERVER_GONE_ERROR:
    case CR_SERVER_LOST:
        return E_DB_CONNECTION;

    case ER_DUP_ENTRY:
    case ER_CONSTRAINT_FAILED:
        return E_DB_CONSTRAINT;

    case CR_VERSION_ERROR:
        return E_DB_INCOMPATIBLE;

    case ER_ACCESS_DENIED_ERROR:
    case ER_ACCESS_DENIED_NO_PASSWORD_ERROR:
        return E_DB_LOGIN_FAILED;

    case ER_PARSE_ERROR:
    case ER_EMPTY_QUERY:
    case ER_UNKNOWN_TABLE:
    case ER_NO_SUCH_TABLE:
    case ER_BAD_FIELD_ERROR:
    case ER_UNSUPPORTED_PS:
        return E_DB_MALFORMED_SQL;

    case ER_NO_DB_ERROR:
        return E_DB_NO_DB;

    case CR_INVALID_PARAMETER_NO:
        return E_DB_OUT_OF_BOUNDS;

    case ER_DBACCESS_DENIED_ERROR:
        return !strcmp(ctx, "connect") ? E_DB_UNKNOWN_DATABASE : E_DB_ACCESS_DENIED;

    case CR_UNKNOWN_HOST:
        return E_DB_UNKNOWN_HOST;

    default:
        return E_DB_EXTENDED;
    }
}

/// Map sub function MySQL error with DB error.
///
/// \param sub      name of sub function
/// \param type     type of sub function
/// \param code     error code
/// \param desc     error description
#define error_map_mysql(sub, type, code, desc) \
    error_map_sub_d(error_mysql_map, (type), sub, MYSQL, code, desc)

/// Map last sub function MySQL error from DB object to DB error.
///
/// \param sub      name of sub function
/// \param type     type of sub function
/// \param db       database
#define error_map_last_mysql(sub, type, db) \
    error_map_last_sub_x(error_mysql_map, (type), sub, MYSQL, "db", db)

/// Map last sub function MySQL error from STMT object to DB error.
///
/// \param sub      name of sub function
/// \param type     type of sub function
/// \param stmt     statement
#define error_map_last_mysql_stmt(sub, type, stmt) \
    error_map_last_sub_x(error_mysql_map, (type), sub, MYSQL, "stmt", stmt)

/// default error type for mysql module
#define ERROR_TYPE_DEFAULT ERROR_TYPE_DB


/// Close MySQL database.
///
/// \implements db_close_cb
static void db_mysql_close(db_ct db)
{
    mysql_close(db_get_ctx(db));
}

/// Prepare MySQL statement.
///
/// \implements db_prepare_cb
static db_stmt_ct db_mysql_prepare(db_ct db, const char *sql)
{
    db_mysql_stmt_st *mystmt;
    db_stmt_ct stmt;

    if(!(mystmt = calloc(1, sizeof(db_mysql_stmt_st))))
        return error_wrap_last_errno(calloc), NULL;

    if(!(stmt = db_stmt_prepare(db, sql, mystmt)))
        return error_pass(), free(mystmt), NULL;

    if(!(mystmt->stmt = mysql_stmt_init(db_get_ctx(db))))
        return error_map_last_mysql(mysql_stmt_init, "init", db), db_stmt_finalize(stmt), NULL;

    if(mysql_stmt_prepare(mystmt->stmt, sql, 0))
    {
        error_map_mysql(mysql3_prepare_v3, "prepare", mysql_stmt_errno(mystmt->stmt), mysql_stmt_error(mystmt->stmt));
        db_stmt_finalize(stmt);

        return NULL;
    }

    assert(db_stmt_param_count(stmt) == mysql_stmt_param_count(mystmt->stmt));

    return stmt;
}

/// Free MySQL parameter bind.
///
/// \param type     data type
/// \param mode     bind mode
/// \param bind     bind to free
static void db_mysql_free_param_bind(db_type_id type, db_param_bind_id mode, MYSQL_BIND *bind)
{
    switch(type)
    {
    case DB_TYPE_DATE:
    case DB_TYPE_TIME:
    case DB_TYPE_DATETIME:
    case DB_TYPE_TIMESTAMP:
        free(bind->buffer);
        break;

    default:
        break;
    }
}

/// Free MySQL parameter.
///
/// \implements db_param_map_cb
static int db_mysql_free_param(db_stmt_const_ct stmt, size_t index, const db_param_bind_st *param, void *ctx)
{
    MYSQL_BIND *bind = ctx;

    db_mysql_free_param_bind(param->type, param->mode, &bind[index]);

    return 0;
}

/// Free MySQL result bind.
///
/// \param type     data type
/// \param mode     bind mode
/// \param bind     bind to free
static void db_mysql_free_result_bind(db_type_id type, db_result_bind_id mode, MYSQL_BIND *bind)
{
    switch(type)
    {
    case DB_TYPE_TEXT:
    case DB_TYPE_BLOB:

        switch(mode)
        {
        case DB_RESULT_BIND_TMP:
        case DB_RESULT_BIND_DUP:
            free(bind->buffer);
            free(bind->length);
            free(bind->is_null);
            break;

        case DB_RESULT_BIND_FIX:
            free(bind->length);
            break;

        default:
            abort();
        }
        break;

    case DB_TYPE_DATE:
    case DB_TYPE_TIME:
    case DB_TYPE_DATETIME:
    case DB_TYPE_TIMESTAMP:
        free(bind->buffer);
        break;

    default:
        break;
    }
}

/// Free MySQL result.
///
/// \implements db_result_map_cb
static int db_mysql_free_result(db_stmt_const_ct stmt, size_t index, const db_result_bind_st *result, void *state, void *ctx)
{
    MYSQL_BIND *bind = ctx;

    db_mysql_free_result_bind(result->type, result->mode, &bind[index]);

    return 0;
}

/// Finalize MySQL statement.
///
/// \implements db_finalize_cb
static void db_mysql_finalize(db_stmt_ct stmt)
{
    db_mysql_stmt_st *mystmt = db_stmt_get_ctx(stmt);

    if(mystmt->param)
        db_stmt_param_map(stmt, db_mysql_free_param, mystmt->param);

    if(mystmt->result)
        db_stmt_result_map(stmt, db_mysql_free_result, mystmt->result);

    free(mystmt->param);
    free(mystmt->result);

    if(mystmt->meta)
        mysql_free_result(mystmt->meta);

    if(mystmt->stmt)
        mysql_stmt_close(mystmt->stmt);

    free(mystmt);
}

/// Get SQL from MySQL statement.
///
/// \implements db_sql_cb
static const char *db_mysql_sql(db_stmt_ct stmt, db_sql_id type)
{
    return error_pass_ptr(db_stmt_sql(stmt, type));
}

/// Bind MySQL type.
///
/// \param bind     bind to set
/// \param type     type to bind
static void db_mysql_bind_type(MYSQL_BIND *bind, db_type_id type)
{
    switch(type)
    {
    case DB_TYPE_INVALID:   // nothing bound
    case DB_TYPE_NULL:
        bind->buffer_type   = MYSQL_TYPE_NULL;
        break;

    case DB_TYPE_INT8:
    case DB_TYPE_UINT8:
        bind->buffer_type   = MYSQL_TYPE_TINY;
        bind->is_unsigned   = type == DB_TYPE_UINT8;
        break;

    case DB_TYPE_INT16:
    case DB_TYPE_UINT16:
        bind->buffer_type   = MYSQL_TYPE_SHORT;
        bind->is_unsigned   = type == DB_TYPE_UINT16;
        break;

    case DB_TYPE_INT32:
    case DB_TYPE_UINT32:
        bind->buffer_type   = MYSQL_TYPE_LONG;
        bind->is_unsigned   = type == DB_TYPE_UINT32;
        break;

    case DB_TYPE_INT64:
    case DB_TYPE_UINT64:
        bind->buffer_type   = MYSQL_TYPE_LONGLONG;
        bind->is_unsigned   = type == DB_TYPE_UINT64;
        break;

    case DB_TYPE_FLOAT:
        bind->buffer_type   = MYSQL_TYPE_FLOAT;
        break;

    case DB_TYPE_DOUBLE:
        bind->buffer_type   = MYSQL_TYPE_DOUBLE;
        break;

    case DB_TYPE_TEXT:
        bind->buffer_type   = MYSQL_TYPE_STRING;
        break;

    case DB_TYPE_BLOB:
        bind->buffer_type   = MYSQL_TYPE_BLOB;
        break;

    case DB_TYPE_DATE:
        bind->buffer_type   = MYSQL_TYPE_DATE;
        break;

    case DB_TYPE_TIME:
        bind->buffer_type   = MYSQL_TYPE_TIME;
        break;

    case DB_TYPE_DATETIME:
    case DB_TYPE_TIMESTAMP:
        bind->buffer_type   = MYSQL_TYPE_DATETIME;
        break;

    default:
        abort();
    }
}

/// Convert DB date/time/datetime/timestamp to MySQL time.
///
/// \param dst      MySQL time object
/// \param type     type of \p src
/// \param src      date, time, datetime or timestamp
static void db_mysql_convert_to_mysql_time(MYSQL_TIME *dst, db_type_id type, const db_data_un *src)
{
    tm_st tm = { 0 };

    memset(dst, 0, sizeof(MYSQL_TIME));

    switch(type)
    {
    case DB_TYPE_DATE:
        dst->year   = src->date->year;
        dst->month  = src->date->month;
        dst->day    = src->date->day;
        break;

    case DB_TYPE_TIME:
        dst->hour   = src->time->hour;
        dst->minute = src->time->minute;
        dst->second = src->time->second;
        break;

    case DB_TYPE_DATETIME:
        dst->year   = src->dt->date.year;
        dst->month  = src->dt->date.month;
        dst->day    = src->dt->date.day;
        dst->hour   = src->dt->time.hour;
        dst->minute = src->dt->time.minute;
        dst->second = src->dt->time.second;
        break;

    case DB_TYPE_TIMESTAMP:
        gmtime_r(src->ts, &tm);

        dst->year   = tm.tm_year + 1900;
        dst->month  = tm.tm_mon + 1;
        dst->day    = tm.tm_mday;
        dst->hour   = tm.tm_hour;
        dst->minute = tm.tm_min;
        dst->second = tm.tm_sec;
        break;

    default:
        abort();
    }
}

/// Bind MySQL parameter.
///
/// \implements db_param_map_cb
static int db_mysql_bind_param(db_stmt_const_ct stmt, size_t index, const db_param_bind_st *param, void *ctx)
{
    MYSQL_BIND *binds = ctx, *bind = &binds[index];

    db_mysql_bind_type(bind, param->type);

    bind->is_null   = (my_bool *)param->is_null; // type size is asserted
    bind->length    = &bind->buffer_length;

    switch(param->type)
    {
    case DB_TYPE_TEXT:
    case DB_TYPE_BLOB:

        if(param->mode == DB_PARAM_BIND_REF)
        {
            bind->buffer        = *param->data.pblob;
            bind->buffer_length = param->rsize ? *param->rsize : strlen(bind->buffer);

            return 0;
        }

        break;

    case DB_TYPE_DATE:
    case DB_TYPE_TIME:
    case DB_TYPE_DATETIME:
    case DB_TYPE_TIMESTAMP:

        if(!bind->buffer && !(bind->buffer = calloc(1, sizeof(MYSQL_TIME))))
            return error_wrap_last_errno(calloc), -1;

        bind->buffer_length = sizeof(MYSQL_TIME);

        db_mysql_convert_to_mysql_time(bind->buffer, param->type, &param->data);

        return 0;

    default:
        break;
    }

    switch(param->mode)
    {
    case DB_PARAM_BIND_FIX:
    case DB_PARAM_BIND_TMP:
        bind->buffer        = param->data.blob;
        bind->buffer_length = param->vsize;
        break;

    case DB_PARAM_BIND_REF:
        bind->buffer        = param->data.blob;
        bind->buffer_length = *param->rsize;
        break;

    default:
        abort();
    }

    return 0;
}

/// Bind MySQL parameters.
///
/// \param stmt     statement
///
/// \retval 0                   success
/// \retval -1/E_GENERIC_OOM    out of memory
static int db_mysql_bind_params(db_stmt_ct stmt)
{
    db_mysql_stmt_st *mystmt    = db_stmt_get_ctx(stmt);
    size_t params               = mysql_stmt_param_count(mystmt->stmt);

    if(!params)
        return 0;

    if(!mystmt->param && !(mystmt->param = calloc(params, sizeof(MYSQL_BIND))))
        return error_wrap_last_errno(calloc), -1;

    if(db_stmt_param_map(stmt, db_mysql_bind_param, mystmt->param))
        return error_lift(E_DB_CALLBACK), -1;

    if(mysql_stmt_bind_param(mystmt->stmt, mystmt->param))
        return error_map_last_mysql_stmt(mysql_stmt_bind_param, "bind", stmt), -1;

    return 0;
}

/// Bind MySQL result field.
///
/// \implements db_result_map_cb
static int db_mysql_bind_result(db_stmt_const_ct stmt, size_t index, const db_result_bind_st *result, void *state, void *ctx)
{
    MYSQL_BIND *binds = ctx, *bind = &binds[index];

    db_mysql_bind_type(bind, result->type);

    bind->is_null = (my_bool *)result->is_null; // type size is asserted

    switch(result->type)
    {
    case DB_TYPE_TEXT:
    case DB_TYPE_BLOB:

        switch(result->mode)
        {
        case DB_RESULT_BIND_TMP:
        case DB_RESULT_BIND_DUP:

            if(!bind->length && !(bind->length = calloc(1, sizeof(unsigned long))))
                return error_wrap_last_errno(calloc), -1;

            if(!bind->is_null && !(bind->is_null = calloc(1, sizeof(my_bool))))
                return error_wrap_last_errno(calloc), -1;

            break;

        case DB_RESULT_BIND_FIX:
            bind->buffer        = result->data.blob;
            bind->buffer_length = result->cap;

                if(!bind->length && !(bind->length = calloc(1, sizeof(unsigned long))))
                    return error_wrap_last_errno(calloc), -1;

            break;

        default:
            abort();
        }

        break;

    case DB_TYPE_DATE:
    case DB_TYPE_TIME:
    case DB_TYPE_DATETIME:
    case DB_TYPE_TIMESTAMP:

        if(!bind->buffer && !(bind->buffer = calloc(1, sizeof(MYSQL_TIME))))
            return error_wrap_last_errno(calloc), -1;

        bind->buffer_length   = sizeof(MYSQL_TIME);
        bind->length          = NULL;

        break;

    default:
        bind->buffer        = result->data.blob;
        bind->buffer_length = result->cap;
        bind->length        = &bind->buffer_length;
    }

    return 0;
}

/// Bind MySQL result fields.
///
/// \param stmt     statement
///
/// \retval 0                   success
/// \retval -1/E_GENERIC_OOM    out of memory
static int db_mysql_bind_results(db_stmt_ct stmt)
{
    db_mysql_stmt_st *mystmt    = db_stmt_get_ctx(stmt);
    size_t fields               = mysql_stmt_field_count(mystmt->stmt);

    if(!fields)
        return 0;

    if(!mystmt->result && !(mystmt->result = calloc(fields, sizeof(MYSQL_BIND))))
        return error_wrap_last_errno(calloc), -1;

    if(db_stmt_result_map(stmt, db_mysql_bind_result, mystmt->result))
        return error_lift(E_DB_CALLBACK), -1;

    if(mysql_stmt_bind_result(mystmt->stmt, mystmt->result))
        return error_map_last_mysql_stmt(mysql_stmt_bind_result, "bind", stmt), -1;

    return 0;
}

/// Convert MySQL time to DB date/time/datetime/timestamp.
///
/// \param type     type of \p dst
/// \param dst      date, time, datetime or timestamp
/// \param src      MySQL time object
static void db_mysql_convert_from_mysql_time(db_type_id type, const db_data_un *dst, const MYSQL_TIME *src)
{
    tm_st tm = { 0 };

    switch(type)
    {
    case DB_TYPE_DATE:
        dst->date->year     = src->year;
        dst->date->month    = src->month;
        dst->date->day      = src->day;
        break;

    case DB_TYPE_TIME:
        dst->time->hour     = src->hour;
        dst->time->minute   = src->minute;
        dst->time->second   = src->second;
        break;

    case DB_TYPE_DATETIME:
        dst->dt->date.year      = src->year;
        dst->dt->date.month     = src->month;
        dst->dt->date.day       = src->day;
        dst->dt->time.hour      = src->hour;
        dst->dt->time.minute    = src->minute;
        dst->dt->time.second    = src->second;
        break;

    case DB_TYPE_TIMESTAMP:
        tm.tm_year  = src->year - 1900;
        tm.tm_mon   = src->month - 1;
        tm.tm_mday  = src->day;
        tm.tm_hour  = src->hour;
        tm.tm_min   = src->minute;
        tm.tm_sec   = src->second;

        *dst->ts = timegm(&tm);
        break;

    default:
        abort();
    }
}

/// Update MySQL result text or BLOB data.
///
/// \param stmt     statement
/// \param index    field index
/// \param result   result to update
/// \param bind     bind to get data from
///
/// \retval 0                   success
/// \retval -1/E_GENERIC_OOM    out of memory
static int db_mysql_update_result_data(db_stmt_const_ct stmt, size_t index, const db_result_bind_st *result, MYSQL_BIND *bind)
{
    db_mysql_stmt_st *mystmt = db_stmt_get_ctx(stmt);
    unsigned long offset;
    void *tmp;

    if(result->mode == DB_RESULT_BIND_FIX)
    {
        if(result->type == DB_TYPE_TEXT && result->data.text)
            result->data.text[MIN(result->cap - 1, *bind->length)] = '\0';

        if(result->size)
            *result->size = *bind->length;

        return 0;
    }

    if(*bind->is_null)
    {
        *result->data.pblob = NULL;

        if(result->size)
            *result->size = 0;

        return 0;
    }

    if(!bind->buffer || *bind->length > bind->buffer_length)
    {
        if(!(tmp = realloc(bind->buffer, *bind->length + 1)))
            return error_wrap_last_errno(realloc), -1;

        offset              = bind->buffer_length;
        bind->buffer        = tmp;
        bind->buffer_length = *bind->length;

        if(mysql_stmt_fetch_column(mystmt->stmt, bind, index, offset))
            return error_map_last_mysql_stmt(mysql_stmt_fetch_column, "fetch", stmt), -1;
    }

    ((char *)bind->buffer)[*bind->length] = '\0';

    *result->data.pblob = bind->buffer;

    if(result->mode == DB_RESULT_BIND_DUP)
    {
        bind->buffer        = NULL;
        bind->buffer_length = 0;
    }

    if(result->size)
        *result->size = *bind->length;

    return 0;
}

/// Update MySQL result field.
///
/// \implements db_result_map_cb
static int db_mysql_update_result(db_stmt_const_ct stmt, size_t index, const db_result_bind_st *result, void *state, void *ctx)
{
    db_mysql_stmt_st *mystmt    = db_stmt_get_ctx(stmt);
    MYSQL_BIND *bind            = &mystmt->result[index];

    switch(result->type)
    {
    case DB_TYPE_TEXT:
    case DB_TYPE_BLOB:
        return error_pass_int(db_mysql_update_result_data(stmt, index, result, bind));

    case DB_TYPE_DATE:
    case DB_TYPE_TIME:
    case DB_TYPE_DATETIME:
    case DB_TYPE_TIMESTAMP:
        db_mysql_convert_from_mysql_time(result->type, &result->data, bind->buffer);
        break;

    default:
        break;
    }

    return 0;
}

/// Update MySQL result fields.
///
/// \param stmt     statement
///
/// \retval 0                   success
/// \retval -1/E_GENERIC_OOM    out of memory
static int db_mysql_update_results(db_stmt_const_ct stmt)
{
    return error_lift_int(E_DB_CALLBACK,
        db_stmt_result_map(stmt, db_mysql_update_result, NULL));
}

/// Execute MySQL statement.
///
/// \implements db_exec_cb
static int db_mysql_exec(db_stmt_ct stmt, db_record_cb record, void *ctx)
{
    db_mysql_stmt_st *mystmt = db_stmt_get_ctx(stmt);
    size_t row;
    int rc;

    if(db_mysql_bind_params(stmt))
        return error_pass(), -1;

    if(mysql_stmt_execute(mystmt->stmt))
        return error_map_last_mysql_stmt(mysql_stmt_execute, "exec", stmt), -1;

    if(db_mysql_bind_results(stmt))
        return error_pass(), mysql_stmt_reset(mystmt->stmt), -1;

    if(!mysql_stmt_field_count(mystmt->stmt))
        return mysql_stmt_reset(mystmt->stmt), 0;

    for(row = 0;; row++)
    {
        switch(mysql_stmt_fetch(mystmt->stmt))
        {
        case 0:
        case MYSQL_DATA_TRUNCATED:
            break;

        case MYSQL_NO_DATA:
            mysql_stmt_reset(mystmt->stmt);

            return 0;

        default:
            error_map_last_mysql_stmt(mysql_stmt_fetch, "fetch", stmt);
            mysql_stmt_reset(mystmt->stmt);

            return -1;
        }

        if(db_mysql_update_results(stmt))
            return error_pass(), mysql_stmt_reset(mystmt->stmt), -1;

        if(!record)
            return mysql_stmt_reset(mystmt->stmt), 0;

        if((rc = record(stmt, row, ctx)))
            return mysql_stmt_reset(mystmt->stmt), error_pack_int(E_DB_CALLBACK, rc);
    }
}

/// Get number of MySQL statement parameters.
///
/// \implements db_param_count_cb
static ssize_t db_mysql_param_count(db_stmt_const_ct stmt)
{
    db_mysql_stmt_st *mystmt = db_stmt_get_ctx(stmt);

    return mysql_stmt_param_count(mystmt->stmt);
}

/// Bind MySQL statement parameter.
///
/// \implements db_param_bind_cb
static int db_mysql_param_bind(db_stmt_ct stmt, size_t index, const db_param_bind_st *bind)
{
    db_mysql_stmt_st *mystmt    = db_stmt_get_ctx(stmt);
    size_t params               = mysql_stmt_param_count(mystmt->stmt);

    return_error_if_fail(index < params, E_DB_OUT_OF_BOUNDS, -1);

    switch(bind->type)
    {
    case DB_TYPE_NULL:
    case DB_TYPE_INT8:
    case DB_TYPE_UINT8:
    case DB_TYPE_INT16:
    case DB_TYPE_UINT16:
    case DB_TYPE_INT32:
    case DB_TYPE_UINT32:
    case DB_TYPE_INT64:
    case DB_TYPE_UINT64:
    case DB_TYPE_FLOAT:
    case DB_TYPE_DOUBLE:
    case DB_TYPE_TEXT:
    case DB_TYPE_BLOB:
    case DB_TYPE_DATE:
    case DB_TYPE_TIME:
    case DB_TYPE_DATETIME:
    case DB_TYPE_TIMESTAMP:
        break;

    default:
        return_error_if_reached(E_DB_UNSUPPORTED_TYPE, -1);
    }

    switch(bind->mode)
    {
    case DB_PARAM_BIND_FIX:
    case DB_PARAM_BIND_TMP:
    case DB_PARAM_BIND_REF:
        break;

    default:
        return_error_if_reached(E_DB_UNSUPPORTED_MODE, -1);
    }

    return error_pass_int(db_stmt_param_bind(stmt, index, bind));
}

/// Get number of MySQL result fields.
///
/// \implements db_result_count_cb
static ssize_t db_mysql_result_count(db_stmt_const_ct stmt)
{
    db_mysql_stmt_st *mystmt = db_stmt_get_ctx(stmt);

    return mysql_stmt_field_count(mystmt->stmt);
}

/// Bind MySQL result field.
///
/// \implements db_result_bind_cb
static int db_mysql_result_bind(db_stmt_ct stmt, size_t index, const db_result_bind_st *bind)
{
    db_mysql_stmt_st *mystmt    = db_stmt_get_ctx(stmt);
    size_t fields               = mysql_stmt_field_count(mystmt->stmt);
    db_result_bind_st old;

    return_error_if_fail(index < fields, E_DB_OUT_OF_BOUNDS, -1);

    switch(bind->type)
    {
    case DB_TYPE_NULL:
        break;

    case DB_TYPE_INT8:
    case DB_TYPE_UINT8:
    case DB_TYPE_INT16:
    case DB_TYPE_UINT16:
    case DB_TYPE_INT32:
    case DB_TYPE_UINT32:
    case DB_TYPE_INT64:
    case DB_TYPE_UINT64:
    case DB_TYPE_FLOAT:
    case DB_TYPE_DOUBLE:
    case DB_TYPE_DATE:
    case DB_TYPE_TIME:
    case DB_TYPE_DATETIME:
    case DB_TYPE_TIMESTAMP:
        return_error_if_fail(bind->mode == DB_RESULT_BIND_FIX, E_DB_UNSUPPORTED_MODE, -1);
        break;

    case DB_TYPE_TEXT:
    case DB_TYPE_BLOB:
        switch(bind->mode)
        {
        case DB_RESULT_BIND_FIX:
        case DB_RESULT_BIND_TMP:
        case DB_RESULT_BIND_DUP:
            break;

        default:
            return_error_if_reached(E_DB_UNSUPPORTED_MODE, -1);
        }
        break;

    default:
        return_error_if_reached(E_DB_UNSUPPORTED_TYPE, -1);
    }

    if(db_stmt_result_init(stmt, fields, 0))
        return error_pass(), -1;

    old = *db_stmt_result_get(stmt, index);

    if(db_stmt_result_bind(stmt, index, bind))
        return error_pass(), -1;

    if(mystmt->result)
    {
        db_mysql_free_result_bind(old.type, old.mode, &mystmt->result[index]);
        memset(&mystmt->result[index], 0, sizeof(MYSQL_BIND));
    }

    return 0;
}

/// Get field infos from result set.
///
/// \param stmt     statement
/// \param index    field index
///
/// \returns                            field info object
/// \retval NULL/E_DB_OUT_OF_BOUNDS     index is out of bounds
/// \retval NULL/E_GENERIC_OOM          out of memory
static MYSQL_FIELD *db_mysql_get_field(db_stmt_const_ct stmt, size_t index)
{
    db_mysql_stmt_st *mystmt = db_stmt_get_ctx(stmt);

    if(!mystmt->meta && !(mystmt->meta = mysql_stmt_result_metadata(mystmt->stmt)))
        return error_map_last_mysql_stmt(mysql_stmt_result_metadata, "meta", stmt), NULL;

    return_error_if_fail(index < mysql_num_fields(mystmt->meta), E_DB_OUT_OF_BOUNDS, NULL);

    return mysql_fetch_field_direct(mystmt->meta, index);
}

/// Get MySQL result field type.
///
/// \implements db_type_cb
static db_type_id db_mysql_result_type(db_stmt_const_ct stmt, size_t index)
{
    MYSQL_FIELD *field;

    if(!(field = db_mysql_get_field(stmt, index)))
        return error_pass(), DB_TYPE_INVALID;

    switch(field->type)
    {
    case MYSQL_TYPE_NULL:
        return DB_TYPE_NULL;

    case MYSQL_TYPE_TINY:
        return field->flags & UNSIGNED_FLAG ? DB_TYPE_UINT8 : DB_TYPE_INT8;

    case MYSQL_TYPE_SHORT:
        return field->flags & UNSIGNED_FLAG ? DB_TYPE_UINT16 : DB_TYPE_INT16;

    case MYSQL_TYPE_LONG:
        return field->flags & UNSIGNED_FLAG ? DB_TYPE_UINT32 : DB_TYPE_INT32;

    case MYSQL_TYPE_LONGLONG:
        return field->flags & UNSIGNED_FLAG ? DB_TYPE_UINT64 : DB_TYPE_INT64;

    case MYSQL_TYPE_FLOAT:
        return DB_TYPE_FLOAT;

    case MYSQL_TYPE_DOUBLE:
        return DB_TYPE_DOUBLE;

    case MYSQL_TYPE_STRING:
    case MYSQL_TYPE_VAR_STRING:
        return DB_TYPE_TEXT;

    case MYSQL_TYPE_BLOB:
        return DB_TYPE_BLOB;

    case MYSQL_TYPE_DATE:
        return DB_TYPE_DATE;

    case MYSQL_TYPE_TIME:
        return DB_TYPE_TIME;

    case MYSQL_TYPE_DATETIME:
    case MYSQL_TYPE_TIMESTAMP:
        return DB_TYPE_DATETIME;

    default:
        return_error_if_reached(E_DB_UNKNOWN_TYPE, DB_TYPE_INVALID);
    }
}

/// Get MySQL database/table/field name.
///
/// \implements db_name_cb
static const char *db_mysql_result_name(db_stmt_const_ct stmt, size_t index, db_name_id type)
{
    MYSQL_FIELD *field;
    const char *name;

    if(!(field = db_mysql_get_field(stmt, index)))
        return error_pass(), NULL;

    switch(type)
    {
    case DB_NAME_DATABASE:
        name = field->db;
        break;

    case DB_NAME_TABLE:
        name = field->table;
        break;

    case DB_NAME_TABLE_ORG:
        name = field->org_table;
        break;

    case DB_NAME_FIELD:
        name = field->name;
        break;

    case DB_NAME_FIELD_ORG:
        name = field->org_name;
        break;

    default:
        return_error_if_reached(E_DB_UNSUPPORTED, NULL);
    }

    return_error_if_fail(name && name[0], E_DB_NO_NAME, NULL);

    return name;
}

/// MySQL interface definition
static const db_interface_st mysql =
{
    .close          = db_mysql_close,
    .prepare        = db_mysql_prepare,
    .finalize       = db_mysql_finalize,
    .sql            = db_mysql_sql,
    .exec           = db_mysql_exec,
    .param_count    = db_mysql_param_count,
    .param_bind     = db_mysql_param_bind,
    .result_count   = db_mysql_result_count,
    .result_bind    = db_mysql_result_bind,
    .result_type    = db_mysql_result_type,
    .result_name    = db_mysql_result_name,
};

/// Connect to MySQL database.
///
/// \param host         hostname, may be NULL for localhost
/// \param port         port, if <= 0 default port is used
/// \param socket       unix socket path, may be NULL
/// \param user         user name, may be NULL for current user
/// \param password     user password, may be NULL
/// \param timeout      connection timeout in seconds, 0 = default
/// \param default_db   default database, may be NULL
///
/// \returns                            db handle
/// \retval NULL/E_DB_ACCESS_DENIED     access denied
/// \retval NULL/E_DB_CONNECTION        could not reach server
/// \retval NULL/E_DB_INCOMPATIBLE      connector not compatible with server
/// \retval NULL/E_DB_UNKNOWN_DATABASE  default DB unknown
/// \retval NULL/E_DB_UNKNOWN_HOST      host not found
/// \retval NULL/E_GENERIC_OOM          out of memory
static db_ct db_mysql_connect_f(const char *host, int port, const char *socket, const char *user, const char *password, unsigned int timeout, const char *default_db)
{
    MYSQL *mydb;
    db_ct db;

    if(!(mydb = mysql_init(NULL)))
        return error_map_mysql(mysql_init, "init", CR_OUT_OF_MEMORY, "Out of memory."), NULL;

    port = port < 0 ? 0 : port;

    mysql_options(mydb, MYSQL_INIT_COMMAND, "SET lc_messages = 'en_US';");
    mysql_options(mydb, MYSQL_SET_CHARSET_NAME, "utf8mb4");

    if(timeout)
        mysql_options(mydb, MYSQL_OPT_CONNECT_TIMEOUT, &timeout);

    if(!mysql_real_connect(mydb, host, user, password, default_db, port, socket, 0))
    {
        error_map_mysql(mysql_real_connect, "connect", mysql_errno(mydb), mysql_error(mydb));
        mysql_close(mydb);

        return NULL;
    }

    if(!(db = db_new(&mysql, mydb)))
        return error_pass(), mysql_close(mydb), NULL;

    return db;
}

db_ct db_mysql_connect(const char *host, int port, const char *user, const char *password, unsigned int timeout, const char *default_db)
{
    return error_pass_ptr(db_mysql_connect_f(host, port, NULL, user, password, timeout, default_db));
}

db_ct db_mysql_connect_local(const char *user, const char *password, const char *default_db)
{
    return error_pass_ptr(db_mysql_connect_f(NULL, 0, NULL, user, password, 0, default_db));
}

db_ct db_mysql_connect_unix(const char *socket, const char *user, const char *password, const char *default_db)
{
    return error_pass_ptr(db_mysql_connect_f(NULL, 0, socket, user, password, 0, default_db));
}

__attribute__((destructor))
void db_mysql_free(void)
{
    mysql_library_end();
}
