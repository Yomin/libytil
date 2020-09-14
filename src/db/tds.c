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

#include <ytil/db/tds.h>
#include <ytil/db/interface.h>
#include <ytil/ext/string.h>
#include <ytil/ext/time.h>
#include <ytil/con/vec.h>
#include <ytil/gen/str.h>
#include <ytil/def.h>
#include <sybfront.h>
#include <sybdb.h>
#include <stdlib.h>


/// TDS result
typedef struct db_tds_result
{
    db_result_bind_st   bind;       /// bind infos
    void                *data;      /// result data
    size_t              cap;        /// result capacity
    int                 is_null;    /// result is NULL
} db_tds_result_st;

/// TDS statement
typedef struct db_tds_stmt
{
    char                *sql;       ///< SQL
    str_ct              esql;       ///< expanded SQL

    size_t              params;     ///< number of parameters
    db_param_bind_st    *param;     ///< parameter bindings
    vec_ct              result;     ///< result bindings
} db_tds_stmt_st;


static_assert(sizeof(bool) == sizeof(DBBIT), "bool type mismatch");


/// Error out-of-memory check callback to check TDS codes.
///
/// \implements error_oom_cb
static bool error_tds_is_oom(const error_type_st *type, int code)
{
    return code == SYBEMEM;
}

/// TDS error type definition
ERROR_DEFINE_CALLBACK(TDS, NULL, NULL, error_tds_is_oom, NULL);

/// Error map callback to map TDS error codes to DB error codes.
///
/// \implements error_map_cb
static int error_tds_map(const error_type_st *type, int code, void *ctx)
{
    switch(code)
    {
    case SYBECONN:
    case SYBEFCON:
    case SYBESOCK:
        return E_DB_CONNECTION;

    case SYBEINTF:
    case SYBEUHST:
        return E_DB_UNKNOWN_HOST;

    case 102:   // incorrect syntax
    case 105:   // unclosed quotation mark
    case 207:   // invalid column name
    case 1038:  // empty object or column name
        return E_DB_MALFORMED_SQL;

    case 911:   // database does not exist
    case 916:   // principal is not able to access database
        return E_DB_UNKNOWN_DATABASE;

    case 4019:  // requested language not available
        return E_DB_UNKNOWN_LANGUAGE;

    case 17053: // OS_ERROR
    case 17803: // SRV_NOMEMORY
        return E_DB_BUSY;

    case 17809: // SRV_NOFREESRVPROC
        return E_DB_MAX_CONNECTIONS;

    case 229:   // permission denied on object
    case 230:   // permission denied on column
    case 18452: // LOGON_INVALID_CONNECT
    case 18456: // LOGON_FAILED
    case 18470: // account is disabled
        return E_DB_ACCESS_DENIED;

    default:
        return E_DB_EXTENDED;
    }
}

/// Map sub function TDS error with DB error.
///
/// \param sub      name of sub function
/// \param code     error code
#define error_map_tds(sub, code) \
    error_map_sub(error_tds_map, NULL, sub, TDS, code)

/// Map last sub function TDS error to DB error.
///
/// \param sub      name of sub function
#define error_map_pre_tds(sub) \
    error_map_pre_sub(error_tds_map, NULL, sub)

/// default error type for TDS module
#define ERROR_TYPE_DEFAULT ERROR_TYPE_DB


/// Close TDS database.
///
/// \implements db_close_cb
static void db_tds_close(db_ct db)
{
    DBPROCESS *tdsdb = db_get_ctx(db);

    dbclose(tdsdb);
}

/// Prepare TDS statement.
///
/// \implements db_prepare_cb
static db_stmt_ct db_tds_prepare(db_ct db, const char *sql)
{
    db_tds_stmt_st *tdsstmt;
    db_stmt_ct stmt;

    return_error_if_fail(sql[0], E_DB_MALFORMED_SQL, NULL);

    if(!(tdsstmt = calloc(1, sizeof(db_tds_stmt_st))))
        return error_wrap_last_errno(calloc), NULL;

    if(!(tdsstmt->sql = strdup(sql)))
        return error_wrap_last_errno(strdup), free(tdsstmt), NULL;

    if(!(stmt = db_stmt_new(db, tdsstmt)))
        return error_pass(), free(tdsstmt->sql), free(tdsstmt), NULL;

    /// \todo TDS multi STMT support
    for(; (sql = strchr(sql, '?')); sql++)
    {
        if(sql[1] == '?') // ?? = literal ?
            sql++;
        else
            tdsstmt->params++;
    }

    return stmt;
}

/// Free TDS parameter.
///
/// \param param    parameter to free
static void db_tds_free_param(db_param_bind_st *param)
{
    if(param->mode == DB_PARAM_BIND_TMP)
        free((void *)param->data);
}

/// Free TDS result field.
///
/// \implements vec_dtor_cb
static void db_tds_vec_free_result_field(vec_const_ct vec, void *elem, void *ctx)
{
    db_tds_result_st *result = elem;

    if(result->data)
        free(result->data);
}

/// Free TDS result.
///
/// \implements vec_dtor_cb
static void db_tds_vec_free_result(vec_const_ct vec, void *elem, void *ctx)
{
    vec_ct *result = elem;

    if(*result)
        vec_free_f(*result, db_tds_vec_free_result_field, NULL);
}

/// Finalize TDS statement.
///
/// \implements db_finalize_cb
static void db_tds_finalize(db_stmt_ct stmt)
{
    db_tds_stmt_st *tdsstmt = db_stmt_get_ctx(stmt);
    size_t c;

    free(tdsstmt->sql);

    if(tdsstmt->esql)
        str_unref(tdsstmt->esql);

    if(tdsstmt->param)
    {
        for(c = 0; c < tdsstmt->params; c++)
            db_tds_free_param(&tdsstmt->param[c]);

        free(tdsstmt->param);
    }

    if(tdsstmt->result)
        vec_free_f(tdsstmt->result, db_tds_vec_free_result, NULL);

    free(tdsstmt);
}

static str_ct db_tds_esql_append_text(str_ct esql, const db_param_bind_st *param)
{
    const char *text, *next;
    char open, close;
    size_t size;

    switch(param->mode)
    {
    case DB_PARAM_BIND_FIX:
    case DB_PARAM_BIND_TMP:
        text    = param->data;
        size    = param->vsize;
        break;

    case DB_PARAM_BIND_REF:
        text    = *(void **)param->data;
        size    = param->rsize ? *param->rsize : strlen(text);
        break;

    default:
        abort();
    }

    if(!text)
        return error_wrap_ptr(str_append_c(esql, "NULL"));

    switch(param->type)
    {
    case DB_TYPE_ID:
        open    = '[';
        close   = ']';
        break;

    case DB_TYPE_TEXT:
        open    = '\'';
        close   = '\'';
        break;

    default:
        abort();
    }

    if(!str_append_cn(esql, &open, 1))
        return error_wrap(), NULL;

    for(; (next = memchr(text, close, size)); size -= next - text + 1, text = next + 1)
    {
        if(!str_append_cn(esql, text, next - text))
            return error_wrap(), NULL;

        if(!str_append_f(esql, "%c%c", close, close))
            return error_wrap(), NULL;
    }

    if(!str_append_cn(esql, text, size) || !str_append_cn(esql, &close, 1))
        return error_wrap(), NULL;

    return esql;
}

static str_ct db_tds_esql_append_blob(str_ct esql, const db_param_bind_st *param)
{
    const unsigned char *data;
    size_t b, size;

    switch(param->mode)
    {
    case DB_PARAM_BIND_FIX:
    case DB_PARAM_BIND_TMP:
        data    = param->data;
        size    = param->vsize;
        break;

    case DB_PARAM_BIND_REF:
        data    = *(void **)param->data;
        size    = *param->rsize;
        break;

    default:
        abort();
    }

    if(!data)
        return error_wrap_ptr(str_append_c(esql, "NULL"));

    if(!str_append_c(esql, "0x"))
        return error_wrap(), NULL;

    for(b = 0; b < size; b++)
        if(!str_append_f(esql, "%02hhx", data[b]))
            return error_wrap(), NULL;

    return esql;
}

static str_ct db_tds_esql_append_param(str_ct esql, const db_param_bind_st *param)
{
    const db_datetime_st *datetime  = param->data;
    const db_date_st *date          = param->data;
    const db_time_st *time          = param->data;
    tm_st tm                        = { 0 };

    if(param->type == DB_TYPE_NULL || (param->is_null && *param->is_null))
        return error_wrap_ptr(str_append_c(esql, "NULL"));

    switch(param->type)
    {
    case DB_TYPE_BOOL:
        return error_wrap_ptr(
            str_append_c(esql, *(bool *)param->data ? "'TRUE'" : "'FALSE'"));

    case DB_TYPE_INT8:
        return error_wrap_ptr(
            str_append_f(esql, "%"PRId8, *(int8_t *)param->data));

    case DB_TYPE_INT16:
        return error_wrap_ptr(
            str_append_f(esql, "%"PRId16, *(int16_t *)param->data));

    case DB_TYPE_INT32:
        return error_wrap_ptr(
            str_append_f(esql, "%"PRId32, *(int32_t *)param->data));

    case DB_TYPE_INT64:
        return error_wrap_ptr(
            str_append_f(esql, "%"PRId64, *(int64_t *)param->data));

    case DB_TYPE_FLOAT:
        return error_wrap_ptr(
            str_append_f(esql, "%.*g", FLT_DECIMAL_DIG, *(float *)param->data));

    case DB_TYPE_DOUBLE:
        return error_wrap_ptr(
            str_append_f(esql, "%.*g", DBL_DECIMAL_DIG, *(double *)param->data));

    case DB_TYPE_ID:
    case DB_TYPE_TEXT:
        return error_pass_ptr(db_tds_esql_append_text(esql, param));

    case DB_TYPE_BLOB:
        return error_pass_ptr(db_tds_esql_append_blob(esql, param));

    case DB_TYPE_DATE:
        return error_wrap_ptr(str_append_f(esql, "'%04u-%02u-%02u'",
            date->year, date->month, date->day));

    case DB_TYPE_TIME:
        return error_wrap_ptr(str_append_f(esql, "'%02u:%02u:%02u'",
            time->hour, time->minute, time->second));

    case DB_TYPE_DATETIME:
        date = &datetime->date;
        time = &datetime->time;

        return error_wrap_ptr(str_append_f(esql, "'%04u-%02u-%02uT%02u:%02u:%02u'",
            date->year, date->month, date->day, time->hour, time->minute, time->second));

    case DB_TYPE_TIMESTAMP:
        gmtime_r(param->data, &tm);

        return error_wrap_ptr(str_append_f(esql, "'%04u-%02u-%02uT%02u:%02u:%02u'",
            tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
            tm.tm_hour, tm.tm_min, tm.tm_sec));

    default:
        abort();
    }
}

/// Expand TDS SQL.
///
/// \param stmt     statement
///
/// \returns                    expanded SQL
/// \retval NULL/E_GENERIC_OOM  out of memory
static str_ct db_tds_expand_sql(db_stmt_ct stmt)
{
    db_tds_stmt_st *tdsstmt = db_stmt_get_ctx(stmt);
    db_param_bind_st *param = tdsstmt->param;
    const char *sql         = tdsstmt->sql;
    str_ct esql             = tdsstmt->esql;
    const char *next;

    if(esql)
        str_clear(esql);
    else if(!(esql = tdsstmt->esql = str_prepare_c(0, 100)))
        return error_wrap(), NULL;

    for(; (next = strchr(sql, '?')); sql = next + 1)
    {
        if(!str_append_cn(esql, sql, next - sql))
            return error_wrap(), NULL;

        if(next[1] == '?') // ?? = literal ?
        {
            if(!str_append_c(esql, "?"))
                return error_wrap(), NULL;

            next++;
        }
        else
        {
            if(!db_tds_esql_append_param(esql, param))
                return error_pass(), NULL;

            param++;
        }
    }

    return error_wrap_ptr(str_append_c(esql, sql));
}

/// Get SQL from TDS statement.
///
/// \implements db_sql_cb
const char *db_tds_sql(db_stmt_ct stmt, db_sql_id type)
{
    db_tds_stmt_st *tdsstmt = db_stmt_get_ctx(stmt);

    switch(type)
    {
    case DB_SQL_PLAIN:
        //if(!escape)
            return tdsstmt->sql;

        /*if(tdsstmt->esql)
            str_set_s(tdsstmt->esql, tdsstmt->sql);
        else if(!(tdsstmt->esql = str_new_s(tdsstmt->sql)))
            return error_wrap(), NULL;

        break;*/

    case DB_SQL_EXPANDED:
        if(!db_tds_expand_sql(stmt))
            return error_pass(), NULL;

        break;

    default:
        return_error_if_reached(E_DB_UNSUPPORTED, NULL);
    }

    //if(escape && !str_escape(tdsstmt->esql))
      //  return error_wrap(), NULL;

    return str_c(tdsstmt->esql);
}

static int db_tds_get_bind_type(db_type_id type)
{
    switch(type)
    {
    case DB_TYPE_BOOL:
        return BITBIND;

    case DB_TYPE_INT8:
        return TINYBIND;

    case DB_TYPE_INT16:
        return SMALLBIND;

    case DB_TYPE_INT32:
        return INTBIND;

    case DB_TYPE_INT64:
        return BIGINTBIND;

    case DB_TYPE_FLOAT:
        return REALBIND;

    case DB_TYPE_DOUBLE:
        return FLT8BIND;

    case DB_TYPE_TEXT:
        return NTBSTRINGBIND;

    case DB_TYPE_BLOB:
        return BINARYBIND;

    case DB_TYPE_DATE:
    case DB_TYPE_TIME:
    case DB_TYPE_DATETIME:
    case DB_TYPE_TIMESTAMP:
        return DATETIMEBIND;

    default:
        abort();
    }
}

/// Bind TDS result field.
///
/// \implements vec_fold_cb
static int db_tds_vec_bind_result_field(vec_const_ct vec, size_t index, void *elem, void *ctx)
{
    db_tds_result_st *result    = elem;
    DBPROCESS *tdsdb            = db_get_ctx(db_stmt_get_db(ctx));
    int bindtype                = db_tds_get_bind_type(result->bind.type);
    void *data                  = result->bind.data;
    size_t cap                  = result->bind.cap;

    if(!dbnullbind(tdsdb, index + 1, &result->is_null))
        return error_map_pre_tds(dbnullbind), -1;

    switch(result->bind.type)
    {
    case DB_TYPE_TEXT:
    case DB_TYPE_BLOB:

        if(result->bind.mode != DB_RESULT_BIND_FIX)
            return 0; // manual fetch

        break;

    case DB_TYPE_DATE:
    case DB_TYPE_TIME:
    case DB_TYPE_DATETIME:
    case DB_TYPE_TIMESTAMP:

        data    = result->data;
        cap     = sizeof(DBDATETIME);

        break;

    default:
        break;
    }

    if(!dbbind(tdsdb, index + 1, bindtype, cap, data))
        return error_map_pre_tds(dbbind), -1;

    return 0;
}

/// Bind TDS result fields.
///
/// \param stmt     statement
/// \param index    result index
///
/// \retval 0       success
static int db_tds_bind_result(db_stmt_const_ct stmt, size_t index)
{
    db_tds_stmt_st *tdsstmt = db_stmt_get_ctx(stmt);
    vec_ct result;

    if(!tdsstmt->result || !(result = vec_at_p(tdsstmt->result, index)))
        return 0;

    return error_pick_int(E_VEC_CALLBACK,
        vec_fold(result, db_tds_vec_bind_result_field, stmt));
}

/// Update TDS result text or BLOB.
///
/// \param stmt     statement
/// \param index    field index
/// \param result   result to update
///
/// \retval 0                       success
/// \retval -1/E_DB_TYPE_MISMATCH   type conversion not possible
/// \retval -1/E_GENERIC_OOM        out of memory
static int db_tds_update_result_blob(db_stmt_const_ct stmt, size_t index, db_tds_result_st *result)
{
    DBPROCESS *tdsdb = db_get_ctx(db_stmt_get_db(stmt));
    size_t size;
    void *tmp;

    switch(dbcoltype(tdsdb, index + 1))
    {
    case SYBCHAR:
    case SYBVARCHAR:
    case SYBTEXT:
    case SYBNTEXT:
    case SYBBINARY:
    case SYBVARBINARY:
    case SYBIMAGE:
        break;

    default:
        return_error_if_reached(E_DB_TYPE_MISMATCH, -1); /// \todo dbconvert
    }

    size = dbdatlen(tdsdb, index + 1);

    if(result->bind.size)
        *result->bind.size = size;

    if(result->bind.mode != DB_RESULT_BIND_TMP && result->bind.mode != DB_RESULT_BIND_DUP)
        return 0;

    if(result->is_null)
    {
        *(void **)result->bind.data = NULL;

        return 0;
    }

    if(!result->data || size > result->cap)
    {
        if(!(tmp = realloc(result->data, size + 1)))
            return error_wrap_last_errno(realloc), -1;

        result->data    = tmp;
        result->cap     = size;
    }

    memcpy(result->data, dbdata(tdsdb, index + 1), size);

    ((char *)result->data)[size] = '\0';

    *(void **)result->bind.data = result->data;

    if(result->bind.mode == DB_RESULT_BIND_DUP)
    {
        result->data    = NULL;
        result->cap     = 0;
    }

    return 0;
}

/// Convert TDS datetime to DB date/time/datetime/timestamp.
///
/// \param stmt     statement
/// \param type     type of \p dst
/// \param dst      date, time, datetime or timestamp
/// \param src      TDS datetime object
static void db_tds_convert_datetime(db_stmt_const_ct stmt, db_type_id type, void *dst, DBDATETIME *src)
{
    DBPROCESS *tdsdb            = db_get_ctx(db_stmt_get_db(stmt));
    db_datetime_st *datetime    = dst;
    db_date_st *date            = NULL;
    db_time_st *time            = NULL;
    DBDATEREC rec               = { 0 };
    time_t *timestamp           = dst;
    db_datetime_st dt           = { 0 };
    tm_st tm                    = { 0 };

    switch(type)
    {
    case DB_TYPE_DATE:
        date = dst;
        break;

    case DB_TYPE_TIME:
        time = dst;
        break;

    case DB_TYPE_DATETIME:
        date = &datetime->date;
        time = &datetime->time;
        break;

    case DB_TYPE_TIMESTAMP:
        date = &dt.date;
        time = &dt.time;
        break;

    default:
        abort();
    }

    dbdatecrack(tdsdb, &rec, src);

    if(date)
    {
#ifdef MSDBLIB
        date->year      = rec.year;
        date->month     = rec.month;
        date->day       = rec.day;
#else
        date->year      = rec.dateyear;
        date->month     = rec.datemonth + 1;
        date->day       = rec.datedmonth;
#endif
    }

    if(time)
    {
#ifdef MSDBLIB
        time->hour      = rec.hour;
        time->minute    = rec.minute;
        time->second    = rec.second;
#else
        time->hour      = rec.datehour;
        time->minute    = rec.dateminute;
        time->second    = rec.datesecond;
#endif
    }

    if(type == DB_TYPE_TIMESTAMP)
    {
        tm.tm_year  = dt.date.year - 1900;
        tm.tm_mon   = dt.date.month - 1;
        tm.tm_mday  = dt.date.day;
        tm.tm_hour  = dt.time.hour;
        tm.tm_min   = dt.time.minute;
        tm.tm_sec   = dt.time.second;

        *timestamp  = timegm(&tm);
    }
}

/// Update TDS result field.
///
/// \implements vec_fold_cb
static int db_tds_vec_update_result_field(vec_const_ct vec, size_t index, void *elem, void *ctx)
{
    db_tds_result_st *result    = elem;
    db_stmt_ct stmt             = ctx;

    switch(result->bind.type)
    {
    case DB_TYPE_TEXT:
    case DB_TYPE_BLOB:

        if(db_tds_update_result_blob(stmt, index, result))
            return error_pass(), -1;

        break;

    case DB_TYPE_DATE:
    case DB_TYPE_TIME:
    case DB_TYPE_DATETIME:
    case DB_TYPE_TIMESTAMP:
        db_tds_convert_datetime(stmt, result->bind.type, result->bind.data, result->data);
        break;

    default:
        break;
    }

    if(result->bind.is_null)
        *result->bind.is_null = !!result->is_null;

    return 0;
}

/// Update TDS result fields.
///
/// \param stmt     statement
/// \param index    result index
///
/// \retval 0                   success
/// \retval -1/E_GENERIC_OOM    out of memory
static int db_tds_update_result(db_stmt_const_ct stmt, size_t index)
{
    db_tds_stmt_st *tdsstmt = db_stmt_get_ctx(stmt);
    vec_ct result;

    if(!tdsstmt->result || !(result = vec_at_p(tdsstmt->result, index)))
        return 0;

    return error_pick_int(E_VEC_CALLBACK,
        vec_fold(result, db_tds_vec_update_result_field, stmt));
}

/// Execute TDS statement.
///
/// \implements db_exec_cb
static int db_tds_exec(db_stmt_ct stmt, db_record_cb record, void *ctx)
{
    db_tds_stmt_st *tdsstmt = db_stmt_get_ctx(stmt);
    DBPROCESS *tdsdb        = db_get_ctx(db_stmt_get_db(stmt));
    size_t result, row;
    int rc;

    if(!db_tds_expand_sql(stmt))
        return error_pass(), -1;

    if(!dbcmd(tdsdb, str_c(tdsstmt->esql)))
        return error_map_pre_tds(dbcmd), -1;

    if(!dbsqlexec(tdsdb))
        return error_map_pre_tds(dbsqlexec), -1;

    for(result = 0; (rc = dbresults(tdsdb)) != NO_MORE_RESULTS; result++)
    {
        if(!rc)
            return error_map_pre_tds(dbresults), -1;

        if(db_tds_bind_result(stmt, result))
            return error_pass(), -1;

        for(row = 0; (rc = dbnextrow(tdsdb)) != NO_MORE_ROWS; row++)
        {
            switch(rc)
            {
            case REG_ROW:
                break;

            case FAIL:
                return error_map_pre_tds(dbnextrow), -1;

            case BUF_FULL:
                abort();

            default:        // compute ID
                continue;   // ignore compute rows
            }

            if(db_tds_update_result(stmt, result))
                return error_pass(), -1;

            if(!record)
            {
                if(!dbcanquery(tdsdb))
                    return error_map_pre_tds(dbcanquery), -1;

                break;
            }

            if((rc = record(stmt, row, ctx)))
                return error_pack_int(E_DB_CALLBACK, rc);
        }

    }

    return 0;
}

/// Get number of TDS statement parameters.
///
/// \implements db_param_count_cb
static ssize_t db_tds_param_count(db_stmt_const_ct stmt)
{
    db_tds_stmt_st *tdsstmt = db_stmt_get_ctx(stmt);

    return tdsstmt->params;
}

/// Bind TDS statement parameter.
///
/// \implements db_param_bind_cb
static int db_tds_param_bind(db_stmt_ct stmt, size_t index, const db_param_bind_st *bind)
{
    db_tds_stmt_st *tdsstmt = db_stmt_get_ctx(stmt);
    void *data              = (void *)bind->data;

    return_error_if_fail(index < tdsstmt->params, E_DB_OUT_OF_BOUNDS, -1);

    if(!tdsstmt->param && !(tdsstmt->param = calloc(tdsstmt->params, sizeof(db_param_bind_st))))
        return error_wrap_last_errno(calloc), -1;

    switch(bind->type)
    {
    case DB_TYPE_NULL:
    case DB_TYPE_BOOL:
    case DB_TYPE_INT8:
    case DB_TYPE_INT16:
    case DB_TYPE_INT32:
    case DB_TYPE_INT64:
    case DB_TYPE_FLOAT:
    case DB_TYPE_DOUBLE:
    case DB_TYPE_ID:
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
    case DB_PARAM_BIND_REF:
        break;

    case DB_PARAM_BIND_TMP:
        if(!(data = memdup(bind->data, bind->vsize)))
            return error_wrap_last_errno(memdup), -1;
        break;

    default:
        return_error_if_reached(E_DB_UNSUPPORTED_MODE, -1);
    }

    db_tds_free_param(&tdsstmt->param[index]);

    tdsstmt->param[index]       = *bind;
    tdsstmt->param[index].data  = data;

    return 0;
}

/// Get number of TDS result fields.
///
/// \implements db_result_count_cb
static ssize_t db_tds_result_count(db_stmt_const_ct stmt)
{
    return error_set(E_DB_UNSUPPORTED), -1; /// \todo TDS result count
}

/// Bind TDS result field.
///
/// \implements db_result_bind_cb
static int db_tds_result_bind(db_stmt_ct stmt, size_t index, const db_result_bind_st *bind)
{
/*    db_tds_stmt_st *tdsstmt = db_stmt_get_ctx(stmt);
    db_tds_result_st *field;
    vec_ct *result;
    size_t size;
    void *data = NULL;

    switch(bind->type)
    {
    case DB_TYPE_BOOL:
    case DB_TYPE_INT8:
    case DB_TYPE_INT16:
    case DB_TYPE_INT32:
    case DB_TYPE_INT64:
    case DB_TYPE_FLOAT:
    case DB_TYPE_DOUBLE:
    case DB_TYPE_DATE:
    case DB_TYPE_TIME:
    case DB_TYPE_DATETIME:
    case DB_TYPE_TIMESTAMP:
        if(bind->mode != DB_RESULT_BIND_FIX)
            return error_set(E_DB_UNSUPPORTED_MODE), -1;
        break;

    case DB_TYPE_TEXT:
    case DB_TYPE_BLOB:
        break;

    default:
        return_error_if_reached(E_DB_UNSUPPORTED_TYPE, -1);
    }

    if(!tdsstmt->result && !(tdsstmt->result = vec_new(2, sizeof(vec_ct))))
        return error_wrap(), -1;

    if(result_index >= (size = vec_size(tdsstmt->result)))
    {
        if(!vec_push_n(tdsstmt->result, result_index - size + 1))
            return error_wrap(), -1;
    }

    result = vec_at(tdsstmt->result, result_index);

    if(!*result && !(*result = vec_new(5, sizeof(db_tds_result_st))))
        return error_wrap(), -1;

    if(field_index >= (size = vec_size(*result)))
    {
        if(!vec_push_n(*result, field_index - size + 1))
            return error_wrap(), -1;
    }

    switch(bind->type)
    {
    case DB_TYPE_DATE:
    case DB_TYPE_TIME:
    case DB_TYPE_DATETIME:
    case DB_TYPE_TIMESTAMP:
        if(!(data = calloc(1, sizeof(DBDATETIME))))
            return error_wrap_last_errno(calloc), -1;
        break;

    default:
        break;
    }

    field = vec_at(*result, field_index);

    if(field->data)
        free(field->data);

    field->bind = *bind;
    field->data = data;
*/
    return 0;
}

/// Fetch TDS field.
///
/// \implements db_result_fetch_cb
static int db_tds_result_fetch(db_stmt_const_ct stmt, size_t index, size_t offset)
{
    return 0; /// \todo
}

/// Get TDS field type.
///
/// \implements db_type_cb
static db_type_id db_tds_result_type(db_stmt_const_ct stmt, size_t index)
{
    return_error_if_reached(E_DB_UNKNOWN_TYPE, DB_TYPE_INVALID); /// \todo
}

/// Get TDS field name.
///
/// \implements db_name_cb
static const char *db_tds_result_name(db_stmt_const_ct stmt, size_t index, db_name_id type)
{
    DBPROCESS *tdsdb = db_get_ctx(db_stmt_get_db(stmt));
    const char *name;

    switch(type)
    {
    case DB_NAME_FIELD:
        name = dbcolname(tdsdb, index);
        break;

    case DB_NAME_FIELD_ORG:
        name = dbcolsource(tdsdb, index);
        break;

    default:
        return_error_if_reached(E_DB_UNSUPPORTED, NULL);
    }

    return_error_if_fail(name, E_DB_NO_NAME, NULL);

    return name;
}

/// TDS interface definition
static const db_interface_st tds =
{
    .close          = db_tds_close,
    .prepare        = db_tds_prepare,
    .finalize       = db_tds_finalize,
    .sql            = db_tds_sql,
    .exec           = db_tds_exec,
    .param_count    = db_tds_param_count,
    .param_bind     = db_tds_param_bind,
    .result_count   = db_tds_result_count,
    .result_bind    = db_tds_result_bind,
    .result_fetch   = db_tds_result_fetch,
    .result_type    = db_tds_result_type,
    .result_name    = db_tds_result_name,
};

#include <stdio.h>
static int db_tds_error_handler(DBPROCESS *proc, int severity, int error_db, int error_os, char *msg_db, char *msg_os)
{
    printf("severity: %i\n", severity);
    printf("DB error: %i %s\n", error_db, msg_db);
    printf("OS error: %i %s\n", error_os, msg_os);
    printf("------------------\n");

    if(error_depth() && error_type(0) == &ERROR_TYPE_TDS)
        return INT_CANCEL;

    if(error_db == SYBESMSG) // server message
        return INT_CANCEL;

    error_set_sd(TDS, error_db, msg_db);

    return INT_CANCEL;
}

static int db_tds_msg_handler(DBPROCESS *proc, int msg_no, int msg_state, int severity, char *msg_text, char *srv_name, char *proc_name, int line)
{
    printf("state: %i\n", msg_state);
    printf("severity: %i\n", severity);
    printf("msg: %i %s\n", msg_no, msg_text);
    printf("------------------\n");

    if(error_depth() && error_type(0) == &ERROR_TYPE_TDS)
        return 0;

    if(msg_no == 5701 || msg_no == 5703) // language/database changed
        return 0;

    error_set_sd(TDS, msg_no, msg_text);

    return 0;
}

db_ct db_tds_connect(const char *app, const char *host, const char *user, const char *password, const char *default_db)
{
    DBPROCESS *tdsdb;
    LOGINREC *login;
    db_ct db;

    if(!dbinit())
        return error_map_tds(dbinit, SYBEMEM), NULL;

    dberrhandle(db_tds_error_handler);
    dbmsghandle(db_tds_msg_handler);

    if(!(login = dblogin()))
        return error_map_pre_tds(dblogin), NULL;

    DBSETLAPP(login, app);
    DBSETLUSER(login, user);
    DBSETLPWD(login, password);
    DBSETLNATLANG(login, "english");
    DBSETLCHARSET(login, "UTF-8");

    if(!(tdsdb = dbopen(login, host ? host : "localhost")))
        return error_map_pre_tds(dbopen), dbloginfree(login), NULL;

    dbloginfree(login);

    if(default_db && !dbuse(tdsdb, default_db))
        return error_map_pre_tds(dbuse), dbclose(tdsdb), NULL;

    if(!(db = db_new(&tds, tdsdb)))
        return error_pass(), dbclose(tdsdb), NULL;

    return db;
}

__attribute__((destructor))
void db_tds_free(void)
{
    dbexit();
}
