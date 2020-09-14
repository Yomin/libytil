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
#include <ytil/def.h>
#include <mysql/mysql.h>
#include <mysql/errmsg.h>
#include <mysql/mysqld_error.h>
#include <stdlib.h>


/// MySQL statement
typedef struct db_mysql_stmt
{
    MYSQL_STMT          *stmt;      ///< statement
    db_param_bind_st    *pbind;     ///< DB parameter bindings
    db_result_bind_st   *rbind;     ///< DB result bindings
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

    case CR_UNKNOWN_HOST:
        return E_DB_UNKNOWN_HOST;

    case CR_VERSION_ERROR:
        return E_DB_VERSION_MISMATCH;

    case ER_ACCESS_DENIED_ERROR:
    case ER_ACCESS_DENIED_NO_PASSWORD_ERROR:
        return E_DB_ACCESS_DENIED;

    case ER_DBACCESS_DENIED_ERROR:
        return !strcmp(ctx, "connect") ? E_DB_UNKNOWN_DATABASE : E_DB_ACCESS_DENIED;

    case ER_NO_DB_ERROR:
        return E_DB_NO_DB;

    case ER_PARSE_ERROR:
    case ER_EMPTY_QUERY:
    case ER_UNKNOWN_TABLE:
    case ER_BAD_FIELD_ERROR:
    case ER_UNSUPPORTED_PS:
        return E_DB_MALFORMED_SQL;

    case CR_INVALID_PARAMETER_NO:
        return E_DB_OUT_OF_BOUNDS;

    /*case MYSQL_FULL:
        return E_DB_FULL;

    case MYSQL_TOOBIG:
        return E_DB_OUT_OF_RANGE;*/

    case ER_DUP_ENTRY:
    case ER_CONSTRAINT_FAILED:
        return E_DB_CONSTRAINT;

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
    const char *ptr;

    if((ptr = strchr(sql, ';')) && ptr[1])
        return error_set(E_DB_MULTI_STMT), NULL;

    if(!(mystmt = calloc(1, sizeof(db_mysql_stmt_st))))
        return error_wrap_last_errno(calloc), NULL;

    if(!(mystmt->stmt = mysql_stmt_init(db_get_ctx(db))))
        return error_map_last_mysql(mysql_stmt_init, "init", db), free(mystmt), NULL;

    if(mysql_stmt_prepare(mystmt->stmt, sql, 0))
    {
        error_map_mysql(mysql3_prepare_v3, "prepare", mysql_stmt_errno(mystmt->stmt), mysql_stmt_error(mystmt->stmt));
        mysql_stmt_close(mystmt->stmt);
        free(mystmt);

        return NULL;
    }

    if(!(stmt = db_stmt_new(db, mystmt)))
        return error_pass(), mysql_stmt_close(mystmt->stmt), free(mystmt), NULL;

    return stmt;
}

/// Free MySQL parameter.
///
/// \param bind     parameter bind infos
/// \param param    parameter to free
static void db_mysql_free_param(db_param_bind_st *bind, MYSQL_BIND *param)
{
    switch(bind->type)
    {
    case DB_TYPE_DATE:
    case DB_TYPE_TIME:
    case DB_TYPE_DATETIME:
    case DB_TYPE_TIMESTAMP:
        free(param->buffer);
        break;

    default:
        if(bind->mode == DB_PARAM_BIND_TMP)
            free(param->buffer);
    }
}

/// Free MySQL result.
///
/// \param bind     result bind infos
/// \param result   result to free
static void db_mysql_free_result(db_result_bind_st *bind, MYSQL_BIND *result)
{
    switch(bind->type)
    {
    case DB_TYPE_TEXT:
    case DB_TYPE_BLOB:

        switch(bind->mode)
        {
        case DB_RESULT_BIND_TMP:
        case DB_RESULT_BIND_DUP:

            if(result->buffer)
                free(result->buffer);

            free(result->length);

            break;

        case DB_RESULT_BIND_FIX:

            if(result->length)
                free(result->length);

            break;

        default:
            abort();
        }
        break;

    case DB_TYPE_DATE:
    case DB_TYPE_TIME:
    case DB_TYPE_DATETIME:
    case DB_TYPE_TIMESTAMP:
        free(result->buffer);
        break;

    default:
        break;
    }
}

/// Finalize MySQL statement.
///
/// \implements db_finalize_cb
static void db_mysql_finalize(db_stmt_ct stmt)
{
    db_mysql_stmt_st *mystmt = db_stmt_get_ctx(stmt);
    size_t c, count;

    if(mystmt->pbind && mystmt->param)
    {
        count = mysql_stmt_param_count(mystmt->stmt);

        for(c = 0; c < count; c++)
            db_mysql_free_param(&mystmt->pbind[c], &mystmt->param[c]);
    }

    if(mystmt->rbind && mystmt->result)
    {
        count = mysql_stmt_field_count(mystmt->stmt);

        for(c = 0; c < count; c++)
            db_mysql_free_result(&mystmt->rbind[c], &mystmt->result[c]);
    }

    if(mystmt->pbind)
        free(mystmt->pbind);

    if(mystmt->rbind)
        free(mystmt->rbind);

    if(mystmt->param)
        free(mystmt->param);

    if(mystmt->result)
        free(mystmt->result);

    if(mystmt->meta)
        mysql_free_result(mystmt->meta);

    mysql_stmt_close(mystmt->stmt);

    free(mystmt);
}

/// Copy DB date/time/datetime/timestamp into MySQL time.
///
/// \param dst      MySQL time object
/// \param type     type of \p value
/// \param value    date, time, datetime or timestamp
static void db_mysql_copy_into_time(MYSQL_TIME *dst, db_type_id type, const void *value)
{
    const db_datetime_st *datetime = value;
    const db_date_st *date = NULL;
    const db_time_st *time = NULL;

    switch(type)
    {
    case DB_TYPE_DATE:
        date = value;
        break;

    case DB_TYPE_TIME:
        time = value;
        break;

    case DB_TYPE_DATETIME:
        date = &datetime->date;
        time = &datetime->time;
        break;

    case DB_TYPE_TIMESTAMP:
        abort(); /// \todo timestamp

    default:
        abort();
    }

    if(date)
    {
        dst->year   = date->year;
        dst->month  = date->month;
        dst->day    = date->day;
    }

    if(time)
    {
        dst->hour   = time->hour;
        dst->minute = time->minute;
        dst->second = time->second;
    }
}

/// Copy MySQL time into DB date/time/datetime/timestamp.
///
/// \param value    date, time, datetime or timestamp
/// \param type     type of \p value
/// \param src      MySQL time object
static void db_mysql_copy_from_time(void *value, db_type_id type, const MYSQL_TIME *src)
{
    db_datetime_st *datetime = value;
    db_date_st *date = NULL;
    db_time_st *time = NULL;

    switch(type)
    {
    case DB_TYPE_DATE:
        date = value;
        break;

    case DB_TYPE_TIME:
        time = value;
        break;

    case DB_TYPE_DATETIME:
        date = &datetime->date;
        time = &datetime->time;
        break;

    case DB_TYPE_TIMESTAMP:
        abort(); /// \todo timestamp

    default:
        abort();
    }

    if(date)
    {
        date->year      = src->year;
        date->month     = src->month;
        date->day       = src->day;
    }

    if(time)
    {
        time->hour      = src->hour;
        time->minute    = src->minute;
        time->second    = src->second;
    }
}

/// Update MySQL parameters.
///
/// \param stmt     statement
static void db_mysql_update_params(db_stmt_ct stmt)
{
    db_mysql_stmt_st *mystmt    = db_stmt_get_ctx(stmt);
    db_param_bind_st *bind      = mystmt->pbind;
    MYSQL_BIND *param           = mystmt->param;
    size_t p, params            = mysql_stmt_param_count(mystmt->stmt);

    for(p = 0; p < params; p++, bind++, param++)
    {
        if(bind->mode != DB_PARAM_BIND_REF)
            continue;

        switch(bind->type)
        {
        case DB_TYPE_TEXT:
        case DB_TYPE_BLOB:
            param->buffer           = *(void **)bind->data;
            param->buffer_length    = bind->rsize ? *bind->rsize : strlen(param->buffer);
            break;

        case DB_TYPE_DATE:
        case DB_TYPE_TIME:
        case DB_TYPE_DATETIME:
        case DB_TYPE_TIMESTAMP:
            db_mysql_copy_into_time(param->buffer, bind->type, bind->data);
            break;

        default:
            break;
        }
    }
}

/// Update MySQL result text or BLOB.
///
/// \param stmt     statement
/// \param index    field index
/// \param bind     bind infos
/// \param result   result to update
///
/// \retval 0                   success
/// \retval -1/E_GENERIC_OOM    out of memory
static int db_mysql_update_result_blob(db_stmt_const_ct stmt, size_t index, const db_result_bind_st *bind, MYSQL_BIND *result)
{
    db_mysql_stmt_st *mystmt    = db_stmt_get_ctx(stmt);
    unsigned long offset;
    void *tmp;

    if(!result->buffer || *result->length > result->buffer_length)
    {
        if(!(tmp = realloc(result->buffer, *result->length + 1)))
            return error_wrap_last_errno(realloc), -1;

        offset                  = result->buffer_length;
        result->buffer          = tmp;
        result->buffer_length   = *result->length;

        if(mysql_stmt_fetch_column(mystmt->stmt, result, index, offset))
            return error_map_last_mysql_stmt(mysql_stmt_fetch_column, "fetch", stmt), -1;
    }

    ((char *)result->buffer)[*result->length] = '\0';

    *(void **)bind->data = result->buffer;

    if(bind->mode == DB_RESULT_BIND_DUP)
    {
        result->buffer          = NULL;
        result->buffer_length   = 0;
    }

    if(bind->size)
        *bind->size = *result->length;

    return 0;
}

/// Update MySQL result field.
///
/// \param stmt     statement
/// \param index    field index
///
/// \retval 0                   success
/// \retval -1/E_GENERIC_OOM    out of memory
static int db_mysql_update_result(db_stmt_const_ct stmt, size_t index)
{
    db_mysql_stmt_st *mystmt    = db_stmt_get_ctx(stmt);
    db_result_bind_st *bind     = &mystmt->rbind[index];
    MYSQL_BIND *result          = &mystmt->result[index];

    switch(bind->type)
    {
    case DB_TYPE_TEXT:
    case DB_TYPE_BLOB:

        switch(bind->mode)
        {
        case DB_RESULT_BIND_TMP:
        case DB_RESULT_BIND_DUP:

            if(db_mysql_update_result_blob(stmt, index, bind, result))
                return error_pass(), -1;

            break;

        case DB_RESULT_BIND_FIX:

            if(bind->size)
                *bind->size = *result->length;

            break;

        default:
            abort();
        }
        break;

    case DB_TYPE_DATE:
    case DB_TYPE_TIME:
    case DB_TYPE_DATETIME:
    case DB_TYPE_TIMESTAMP:
        db_mysql_copy_from_time(bind->data, bind->type, result->buffer);
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
    db_mysql_stmt_st *mystmt    = db_stmt_get_ctx(stmt);
    size_t f, fields            = mysql_stmt_field_count(mystmt->stmt);

    for(f = 0; f < fields; f++)
        if(db_mysql_update_result(stmt, f))
            return error_pass(), -1;

    return 0;
}

/// Execute MySQL statement.
///
/// \implements db_exec_cb
static int db_mysql_exec(db_stmt_ct stmt, db_record_cb record, void *ctx)
{
    db_mysql_stmt_st *mystmt = db_stmt_get_ctx(stmt);
    size_t row;
    int rc;

    if(mystmt->pbind && mystmt->param)
    {
        db_mysql_update_params(stmt);

        if(mysql_stmt_bind_param(mystmt->stmt, mystmt->param))
            return error_map_last_mysql_stmt(mysql_stmt_bind_param, "bind", stmt), -1;
    }

    if(mysql_stmt_execute(mystmt->stmt))
        return error_map_last_mysql_stmt(mysql_stmt_execute, "exec", stmt), -1;

    if(mystmt->result && mysql_stmt_bind_result(mystmt->stmt, mystmt->result))
    {
        error_map_last_mysql_stmt(mysql_stmt_bind_result, "bind", stmt);
        mysql_stmt_reset(mystmt->stmt);

        return -1;
    }

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

        if(mystmt->rbind && mystmt->result && db_mysql_update_results(stmt))
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

/// Bind MySQL type.
///
/// \param type     type to bind
/// \param bind     bind to set
///
/// \retval 0                           success
/// \retval -1/E_DB_UNSUPPORTED_TYPE    type not supported
static int db_mysql_bind_type(db_type_id type, MYSQL_BIND *bind)
{
    switch(type)
    {
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
        bind->buffer_type   = MYSQL_TYPE_DATETIME;
        break;

    default:
        return_error_if_reached(E_DB_UNSUPPORTED_TYPE, -1);
    }

    return 0;
}

/// Bind MySQL scalar statement parameter.
///
/// \param bind     bind infos
/// \param param    parameter bindings to fill
///
/// \retval 0                           success
/// \retval -1/E_DB_UNSUPPORTED_MODE    unsupported bind mode
/// \retval -1/E_GENERIC_OOM            out of memory
static int db_mysql_param_bind_scalar(const db_param_bind_st *bind, MYSQL_BIND *param)
{
    switch(bind->mode)
    {
    case DB_PARAM_BIND_FIX:
        param->buffer           = (void *)bind->data;
        param->buffer_length    = bind->vsize;
        param->length           = &param->buffer_length;
        break;

    case DB_PARAM_BIND_TMP:

        if(!(param->buffer = memdup(bind->data, bind->vsize)))
            return error_wrap_last_errno(memdup), -1;

        param->buffer_length    = bind->vsize;
        param->length           = &param->buffer_length;

        break;

    case DB_PARAM_BIND_REF:
        param->buffer           = (void *)bind->data;
        param->buffer_length    = *bind->rsize; // does not change for scalars
        param->length           = &param->buffer_length;
        break;

    default:
        return_error_if_reached(E_DB_UNSUPPORTED_MODE, -1);
    }

    return 0;
}

/// Bind MySQL text or BLOB statement parameter.
///
/// \param bind     bind infos
/// \param param    parameter bindings to fill
///
/// \retval 0                           success
/// \retval -1/E_DB_UNSUPPORTED_MODE    unsupported bind mode
/// \retval -1/E_GENERIC_OOM            out of memory
static int db_mysql_param_bind_blob(const db_param_bind_st *bind, MYSQL_BIND *param)
{
    switch(bind->mode)
    {
    case DB_PARAM_BIND_FIX:
        param->buffer           = (void *)bind->data;
        param->buffer_length    = bind->vsize;
        param->length           = &param->buffer_length;
        break;

    case DB_PARAM_BIND_TMP:

        if(!(param->buffer = memdup(bind->data, bind->vsize)))
            return error_wrap_last_errno(memdup), -1;

        param->buffer_length    = bind->vsize;
        param->length           = &param->buffer_length;

        break;

    case DB_PARAM_BIND_REF:
        param->buffer           = NULL; // to be updated
        param->buffer_length    = 0; // to be updated
        param->length           = &param->buffer_length;
        break;

    default:
        return_error_if_reached(E_DB_UNSUPPORTED_MODE, -1);
    }

    return 0;
}

/// Bind MySQL date/time/datetime/timestamp statement parameter.
///
/// \param bind     bind infos
/// \param param    parameter bindings to fill
///
/// \retval 0                           success
/// \retval -1/E_DB_UNSUPPORTED_MODE    unsupported bind mode
/// \retval -1/E_GENERIC_OOM            out of memory
static int db_mysql_param_bind_date(const db_param_bind_st *bind, MYSQL_BIND *param)
{
    if(!(param->buffer = calloc(1, sizeof(MYSQL_TIME))))
        return error_wrap_last_errno(calloc), -1;

    param->buffer_length    = sizeof(MYSQL_TIME);
    param->length           = &param->buffer_length;

    switch(bind->mode)
    {
    case DB_PARAM_BIND_FIX:
    case DB_PARAM_BIND_TMP:
        db_mysql_copy_into_time(param->buffer, bind->type, bind->data);
        break;

    case DB_PARAM_BIND_REF:
        break;

    default:
        free(param->buffer);
        return_error_if_reached(E_DB_UNSUPPORTED_MODE, -1);
    }

    return 0;
}

/// Bind MySQL statement parameter.
///
/// \implements db_param_bind_cb
static int db_mysql_param_bind(db_stmt_ct stmt, size_t index, const db_param_bind_st *bind)
{
    db_mysql_stmt_st *mystmt    = db_stmt_get_ctx(stmt);
    size_t params               = mysql_stmt_param_count(mystmt->stmt);
    MYSQL_BIND *param, backup;
    int rc;

    return_error_if_fail(index < params, E_DB_OUT_OF_BOUNDS, -1);

    if(!mystmt->pbind && !(mystmt->pbind = calloc(params, sizeof(db_param_bind_st))))
        return error_wrap_last_errno(calloc), -1;

    if(!mystmt->param && !(mystmt->param = calloc(params, sizeof(MYSQL_BIND))))
        return error_wrap_last_errno(calloc), -1;

    param   = &mystmt->param[index];
    backup  = *param;

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
        rc = db_mysql_param_bind_scalar(bind, param);
        break;

    case DB_TYPE_TEXT:
    case DB_TYPE_BLOB:
        rc = db_mysql_param_bind_blob(bind, param);
        break;

    case DB_TYPE_DATE:
    case DB_TYPE_TIME:
    case DB_TYPE_DATETIME:
    case DB_TYPE_TIMESTAMP:
        rc = db_mysql_param_bind_date(bind, param);
        break;

    default:
        return_error_if_reached(E_DB_UNSUPPORTED_TYPE, -1);
    }

    if(rc)
        return *param = backup, error_pass(), -1;

    db_mysql_bind_type(bind->type, param);
    param->is_null = (my_bool *)bind->is_null;  // type size is asserted

    db_mysql_free_param(&mystmt->pbind[index], &backup);

    mystmt->pbind[index] = *bind;

    return 0;
}

/// Get number of MySQL result fields.
///
/// \implements db_result_count_cb
static ssize_t db_mysql_result_count(db_stmt_const_ct stmt)
{
    db_mysql_stmt_st *mystmt = db_stmt_get_ctx(stmt);

    return mysql_stmt_field_count(mystmt->stmt);
}

/// Bind MySQL scalar result field.
///
/// \param bind     bind infos
/// \param result   result bindings to fill
///
/// \retval 0                           success
/// \retval -1/E_DB_UNSUPPORTED_MODE    unsupported bind mode
/// \retval -1/E_GENERIC_OOM            out of memory
static int db_mysql_result_bind_scalar(const db_result_bind_st *bind, MYSQL_BIND *result)
{
    if(bind->mode != DB_RESULT_BIND_FIX)
        return error_set(E_DB_UNSUPPORTED_MODE), -1;

    result->buffer          = bind->data;
    result->buffer_length   = bind->cap;
    result->length          = NULL;

    if(bind->size)
        *bind->size = bind->cap;

    return 0;
}

/// Bind MySQL text or BLOB result field.
///
/// \param bind     bind infos
/// \param result   result bindings to fill
///
/// \retval 0                           success
/// \retval -1/E_DB_UNSUPPORTED_MODE    unsupported bind mode
/// \retval -1/E_GENERIC_OOM            out of memory
static int db_mysql_result_bind_blob(const db_result_bind_st *bind, MYSQL_BIND *result)
{
    switch(bind->mode)
    {
    case DB_RESULT_BIND_TMP:
    case DB_RESULT_BIND_DUP:

        result->buffer          = NULL;
        result->buffer_length   = 0;

        if(!(result->length = calloc(1, sizeof(unsigned long))))
            return error_wrap_last_errno(calloc), -1;

        break;

    case DB_RESULT_BIND_FIX:

        result->buffer          = bind->data;
        result->buffer_length   = bind->cap;
        result->length          = NULL;

        if(bind->size && !(result->length = calloc(1, sizeof(unsigned long))))
            return error_wrap_last_errno(calloc), -1;

        break;

    default:
        return_error_if_reached(E_DB_UNSUPPORTED_MODE, -1);
    }

    return 0;
}

/// Bind MySQL date/time/datetime/timestamp result field.
///
/// \param bind     bind infos
/// \param result   result bindings to fill
///
/// \retval 0                           success
/// \retval -1/E_DB_UNSUPPORTED_MODE    unsupported bind mode
/// \retval -1/E_GENERIC_OOM            out of memory
static int db_mysql_result_bind_date(const db_result_bind_st *bind, MYSQL_BIND *result)
{
    if(bind->mode != DB_RESULT_BIND_FIX)
        return error_set(E_DB_UNSUPPORTED_MODE), -1;

    if(!(result->buffer = calloc(1, sizeof(MYSQL_TIME))))
        return error_wrap_last_errno(calloc), -1;

    result->buffer_length   = sizeof(MYSQL_TIME);
    result->length          = NULL;

    if(bind->size)
        *bind->size = bind->cap;

    return 0;
}

/// Bind MySQL result field.
///
/// \implements db_result_bind_cb
static int db_mysql_result_bind(db_stmt_ct stmt, size_t index, const db_result_bind_st *bind)
{
    db_mysql_stmt_st *mystmt    = db_stmt_get_ctx(stmt);
    size_t fields               = mysql_stmt_field_count(mystmt->stmt);
    MYSQL_BIND *result, backup;
    int rc;

    return_error_if_fail(index < fields, E_DB_OUT_OF_BOUNDS, -1);

    if(!mystmt->rbind && !(mystmt->rbind = calloc(fields, sizeof(db_result_bind_st))))
        return error_wrap_last_errno(calloc), -1;

    if(!mystmt->result && !(mystmt->result = calloc(fields, sizeof(MYSQL_BIND))))
        return error_wrap_last_errno(calloc), -1;

    result  = &mystmt->result[index];
    backup  = *result;

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
        rc = db_mysql_result_bind_scalar(bind, result);
        break;

    case DB_TYPE_TEXT:
    case DB_TYPE_BLOB:
        rc = db_mysql_result_bind_blob(bind, result);
        break;

    case DB_TYPE_DATE:
    case DB_TYPE_TIME:
    case DB_TYPE_DATETIME:
    case DB_TYPE_TIMESTAMP:
        rc = db_mysql_result_bind_date(bind, result);
        break;

    default:
        return_error_if_reached(E_DB_UNSUPPORTED_TYPE, -1);
    }

    if(rc)
        return *result = backup, error_pass(), -1;

    db_mysql_bind_type(bind->type, result);
    result->is_null = (my_bool *)bind->is_null; // type size is asserted

    db_mysql_free_result(&mystmt->rbind[index], &backup);

    mystmt->rbind[index] = *bind;

    return 0;
}

/// Fetch MySQL field.
///
/// \implements db_result_fetch_cb
static int db_mysql_result_fetch(db_stmt_const_ct stmt, size_t index, size_t offset)
{
    db_mysql_stmt_st *mystmt    = db_stmt_get_ctx(stmt);
    size_t fields               = mysql_stmt_field_count(mystmt->stmt);

    return_error_if_fail(index < fields, E_DB_OUT_OF_BOUNDS, -1);
    return_value_if_fail(mystmt->result, 0);

    if(mysql_stmt_fetch_column(mystmt->stmt, &mystmt->result[index], index, offset))
        return error_map_last_mysql_stmt(mysql_stmt_fetch_column, "fetch", stmt), -1;

    return error_pass_int(db_mysql_update_result(stmt, index));
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

/// Get MySQL field type.
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
        return DB_TYPE_TEXT; /// \todo TEXT vs CHAR

    case MYSQL_TYPE_BLOB: /// \todo TEXT
        return DB_TYPE_BLOB;

    case MYSQL_TYPE_DATE:
        return DB_TYPE_DATE;

    case MYSQL_TYPE_TIME:
        return DB_TYPE_TIME;

    case MYSQL_TYPE_DATETIME:
        return DB_TYPE_DATETIME;

    case MYSQL_TYPE_TIMESTAMP:
        return DB_TYPE_TIMESTAMP;

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

    return_error_if_fail(name, E_DB_NO_NAME, NULL);

    return name;
}

/// MySQL interface definition
static const db_interface_st mysql =
{
    .close          = db_mysql_close,
    .prepare        = db_mysql_prepare,
    .finalize       = db_mysql_finalize,
    .exec           = db_mysql_exec,
    .param_count    = db_mysql_param_count,
    .param_bind     = db_mysql_param_bind,
    .result_count   = db_mysql_result_count,
    .result_bind    = db_mysql_result_bind,
    .result_fetch   = db_mysql_result_fetch,
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
/// \param default_db   default database, may be NULL
///
/// \returns                            db handle
/// \retval NULL/E_DB_ACCESS_DENIED     access denied
/// \retval NULL/E_DB_CONNECTION        could not reach server
/// \retval NULL/E_DB_UNKNOWN_DATABASE  default DB unknown or access denied
/// \retval NULL/E_DB_UNKNOWN_HOST      host not found
/// \retval NULL/E_DB_VERSION_MISMATCH  client - server version mismatch
/// \retval NULL/E_GENERIC_OOM          out of memory
static db_ct db_mysql_connect_f(const char *host, int port, const char *socket, const char *user, const char *password, const char *default_db)
{
    MYSQL *mydb;
    db_ct db;

    if(!(mydb = mysql_init(NULL)))
        return error_map_mysql(mysql_init, "init", CR_OUT_OF_MEMORY, "Out of memory."), NULL;

    port = port < 0 ? 0 : port;

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

db_ct db_mysql_connect(const char *host, int port, const char *user, const char *password, const char *default_db)
{
    return error_pass_ptr(db_mysql_connect_f(host, port, NULL, user, password, default_db));
}

db_ct db_mysql_connect_local(const char *user, const char *password, const char *default_db)
{
    return error_pass_ptr(db_mysql_connect_f(NULL, 0, NULL, user, password, default_db));
}

db_ct db_mysql_connect_unix(const char *socket, const char *user, const char *password, const char *default_db)
{
    return error_pass_ptr(db_mysql_connect_f(NULL, 0, socket, user, password, default_db));
}

__attribute__((destructor))
void db_mysql_free(void)
{
    mysql_library_end();
}
