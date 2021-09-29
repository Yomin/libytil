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

#include <ytil/db/stmt.h>
#include <ytil/def.h>
#include <ytil/def/magic.h>
#include <ytil/ext/string.h>
#include <ytil/ext/time.h>
#include <ytil/gen/str.h>
#include <stdlib.h>
#include <float.h>


#define MAGIC define_magic("DBS")   ///< statement magic

/// database statement
typedef struct db_stmt
{
    DEBUG_MAGIC

    db_ct   db;                     ///< database
    void    *ctx;                   ///< statement context

    char *name;                     ///< statement name

    char    *sql;                   ///< SQL
    str_ct  sql_exp;                ///< expanded SQL
    str_ct  sql_esc;                ///< escaped SQL

    db_param_bind_st    *param;     ///< parameters
    size_t              params;     ///< number of parameters
    db_result_bind_st   *result;    ///< result fields
    size_t              results;    ///< number of result fields
    char                *state;     ///< result states
    size_t              stsize;     ///< result state size

    bool executing;                 ///< statement is being executed
} db_stmt_st;

/// default error type for stmt module
#define ERROR_TYPE_DEFAULT ERROR_TYPE_DB


db_stmt_ct db_stmt_new(db_ct db, const void *ctx)
{
    db_stmt_ct stmt;

    if(!(stmt = calloc(1, sizeof(db_stmt_st))))
        return error_wrap_last_errno(calloc), NULL;

    init_magic(stmt);

    stmt->db    = db;
    stmt->ctx   = (void *)ctx;

    db_ref(db);

    return stmt;
}

static void db_stmt_free_param(db_param_bind_st *param)
{
    if(param->mode == DB_PARAM_BIND_TMP)
        free(param->data.blob);
}

static void db_stmt_free_params(db_stmt_ct stmt)
{
    size_t p;

    for(p = 0; p < stmt->params; p++)
        db_stmt_free_param(&stmt->param[p]);

    free(stmt->param);
}

int db_stmt_finalize(db_stmt_ct stmt)
{
    db_finalize_cb finalize = db_stmt_get_interface(stmt)->finalize;

    return_error_if_pass(stmt->executing, E_DB_ILLEGAL, -1);

    if(finalize)
        finalize(stmt);

    db_unref(stmt->db);

    free(stmt->name);
    free(stmt->sql);

    if(stmt->sql_exp)
        str_unref(stmt->sql_exp);

    if(stmt->sql_esc)
        str_unref(stmt->sql_esc);

    db_stmt_free_params(stmt);
    free(stmt->result);
    free(stmt->state);

    free(stmt);

    return 0;
}

void *db_stmt_get_ctx(db_stmt_const_ct stmt)
{
    assert_magic(stmt);

    return stmt->ctx;
}

db_ct db_stmt_get_db(db_stmt_const_ct stmt)
{
    assert_magic(stmt);

    return stmt->db;
}

const db_interface_st *db_stmt_get_interface(db_stmt_const_ct stmt)
{
    assert_magic(stmt);

    return db_get_interface(stmt->db);
}

int db_stmt_set_name(db_stmt_ct stmt, const char *name)
{
    char *dname = NULL;

    assert_magic(stmt);

    if(name && !(dname = strdup(name)))
        return error_wrap_last_errno(strdup), -1;

    free(stmt->name);
    stmt->name = dname;

    return 0;
}

const char *db_stmt_get_name(db_stmt_const_ct stmt)
{
    assert_magic(stmt);

    return stmt->name;
}

int db_stmt_set_executing(db_stmt_ct stmt, bool executing)
{
    assert_magic(stmt);

    return_error_if_pass(stmt->executing == executing, E_DB_ILLEGAL, -1);

    stmt->executing = executing;

    return 0;
}

bool db_stmt_is_executing(db_stmt_const_ct stmt)
{
    assert_magic(stmt);

    return stmt->executing;
}

db_stmt_ct db_stmt_prepare(db_ct db, const char *sql, const void *ctx)
{
    db_stmt_ct stmt;
    const char *ptr;
    size_t params;
    char *dsql;

    return_error_if_fail(sql[0], E_DB_MALFORMED_SQL, NULL);
    return_error_if_pass((ptr = strchr(sql, ';')) && ptr[1], E_DB_MULTI_STMT, NULL);

    if(!(dsql = strdup(sql)))
        return error_wrap_last_errno(strdup), NULL;

    for(params = 0; (sql = strchr(sql, '?')); sql++)
    {
        if(sql[1] == '?') // ?? = literal ?
            sql++;
        else
            params++;
    }

    if(!(stmt = db_stmt_new(db, ctx)))
        return error_pass(), free(dsql), NULL;

    stmt->sql = dsql;

    if(db_stmt_param_init(stmt, params))
        return error_pass(), db_stmt_finalize(stmt), NULL;

    return stmt;
}

static str_ct db_stmt_expand_data(str_ct sql, const db_param_bind_st *param)
{
    const char *data, *next;
    size_t b, size;

    switch(param->mode)
    {
    case DB_PARAM_BIND_FIX:
    case DB_PARAM_BIND_TMP:
        data    = param->data.blob;
        size    = param->vsize;
        break;

    case DB_PARAM_BIND_REF:
        assert(param->rsize || param->type != DB_TYPE_BLOB);

        data    = *param->data.pblob;
        size    = param->rsize ? *param->rsize : data ? strlen(data) : 0;
        break;

    default:
        abort();
    }

    if(!data)
        return error_wrap_ptr(str_append_c(sql, "NULL"));

    if(param->type == DB_TYPE_TEXT)
    {
        if(!str_append_c(sql, "'"))
            return error_wrap(), NULL;

        for(; (next = memchr(data, '\'', size)); size -= next - data + 1, data = next + 1)
        {
            if(!str_append_cn(sql, data, next - data))
                return error_wrap(), NULL;

            if(!str_append_c(sql, "''"))
                return error_wrap(), NULL;
        }

        if(!str_append_cn(sql, data, size))
            return error_wrap(), NULL;
    }
    else
    {
        if(!str_append_c(sql, "x'"))
            return error_wrap(), NULL;

        for(b = 0; b < size; b++)
        {
            if(!str_append_f(sql, "%02hhx", (unsigned char)data[b]))
                return error_wrap(), NULL;
        }
    }

    if(!str_append_c(sql, "'"))
        return error_wrap(), NULL;

    return sql;
}

static str_ct db_stmt_expand_param(str_ct sql, const db_param_bind_st *param)
{
    tm_st tm;

    if(param->type == DB_TYPE_NULL || (param->is_null && *param->is_null))
        return error_wrap_ptr(str_append_c(sql, "NULL"));

    switch(param->type)
    {
    case DB_TYPE_BOOL:
        return error_wrap_ptr(
            str_append_c(sql, *param->data.b ? "true" : "false"));

    case DB_TYPE_INT8:
        return error_wrap_ptr(
            str_append_f(sql, "%"PRId8, *param->data.i8));

    case DB_TYPE_UINT8:
        return error_wrap_ptr(
            str_append_f(sql, "%"PRIu8, *param->data.u8));

    case DB_TYPE_INT16:
        return error_wrap_ptr(
            str_append_f(sql, "%"PRId16, *param->data.i16));

    case DB_TYPE_UINT16:
        return error_wrap_ptr(
            str_append_f(sql, "%"PRIu16, *param->data.u16));

    case DB_TYPE_INT32:
        return error_wrap_ptr(
            str_append_f(sql, "%"PRId32, *param->data.i32));

    case DB_TYPE_UINT32:
        return error_wrap_ptr(
            str_append_f(sql, "%"PRIu32, *param->data.u32));

    case DB_TYPE_INT64:
        return error_wrap_ptr(
            str_append_f(sql, "%"PRId64, *param->data.i64));

    case DB_TYPE_UINT64:
        return error_wrap_ptr(
            str_append_f(sql, "%"PRIu64, *param->data.u64));

    case DB_TYPE_FLOAT:
        return error_wrap_ptr(
            str_append_f(sql, "%.*g", FLT_DECIMAL_DIG, *param->data.f));

    case DB_TYPE_DOUBLE:
        return error_wrap_ptr(
            str_append_f(sql, "%.*g", DBL_DECIMAL_DIG, *param->data.d));

    case DB_TYPE_LDOUBLE:
        return error_wrap_ptr(
            str_append_f(sql, "%.*Lg", LDBL_DECIMAL_DIG, *param->data.ld));

    case DB_TYPE_TEXT:
    case DB_TYPE_BLOB:
        return error_pass_ptr(db_stmt_expand_data(sql, param));

    case DB_TYPE_DATE:
        return error_wrap_ptr(str_append_f(sql, "'%04u-%02u-%02u'",
            param->data.date->year, param->data.date->month, param->data.date->day));

    case DB_TYPE_TIME:
        return error_wrap_ptr(str_append_f(sql, "'%02u:%02u:%02u'",
            param->data.time->hour, param->data.time->minute, param->data.time->second));

    case DB_TYPE_DATETIME:
        return error_wrap_ptr(str_append_f(sql, "'%04u-%02u-%02u %02u:%02u:%02u'",
            param->data.dt->date.year, param->data.dt->date.month, param->data.dt->date.day,
            param->data.dt->time.hour, param->data.dt->time.minute, param->data.dt->time.second));

    case DB_TYPE_TIMESTAMP:
        gmtime_r(param->data.ts, &tm);

        return error_wrap_ptr(str_append_f(sql, "'%04u-%02u-%02u %02u:%02u:%02u'",
            tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
            tm.tm_hour, tm.tm_min, tm.tm_sec));

    default:
        abort();
    }
}

/// Expand SQL.
///
/// \param stmt     statement
///
/// \returns                    expanded SQL
/// \retval NULL/E_GENERIC_OOM  out of memory
static str_ct db_stmt_expand_sql(db_stmt_ct stmt)
{
    db_param_bind_st *param = stmt->param;
    const char *sql, *next;

    if(stmt->sql_exp)
        str_clear(stmt->sql_exp);
    else if(!(stmt->sql_exp = str_prepare_c(0, 100)))
        return error_wrap(), NULL;

    for(sql = stmt->sql; (next = strchr(sql, '?')); sql = next + 1)
    {
        if(!str_append_cn(stmt->sql_exp, sql, next - sql))
            return error_wrap(), NULL;

        if(next[1] == '?') // ?? = literal ?
        {
            if(!str_append_c(stmt->sql_exp, "?"))
                return error_wrap(), NULL;

            next++;
        }
        else
        {
            if(!db_stmt_expand_param(stmt->sql_exp, param))
                return error_pass(), NULL;

            param++;
        }
    }

    return error_wrap_ptr(str_append_c(stmt->sql_exp, sql));
}

const char *db_stmt_sql(db_stmt_ct stmt, db_sql_id type)
{
    assert_magic(stmt);
    assert(stmt->sql);

    switch(type)
    {
    case DB_SQL_PLAIN:
        return stmt->sql;

    case DB_SQL_EXPANDED:

        if(!db_stmt_expand_sql(stmt))
            return error_pass(), NULL;

        return str_c(stmt->sql_exp);

    default:
        abort();
    }
}

const char *db_stmt_escape_sql(db_stmt_ct stmt, const char *sql)
{
    assert_magic(stmt);
    assert(sql);

    if(stmt->sql_esc)
        str_set_s(stmt->sql_esc, sql);
    else if(!(stmt->sql_esc = str_new_s(sql)))
        return error_wrap(), NULL;

    if(!str_escape(stmt->sql_esc))
        return error_wrap(), NULL;

    return str_c(stmt->sql_esc);
}

int db_stmt_param_init(db_stmt_ct stmt, size_t n)
{
    assert_magic(stmt);
    assert(!stmt->param || stmt->params == n);

    return_value_if_pass(!n || stmt->params == n, 0);

    if(!(stmt->param = calloc(n, sizeof(db_param_bind_st))))
        return error_wrap_last_errno(calloc), -1;

    stmt->params = n;

    return 0;
}

size_t db_stmt_param_count(db_stmt_const_ct stmt)
{
    assert_magic(stmt);

    return stmt->params;
}

int db_stmt_param_bind(db_stmt_ct stmt, size_t index, const db_param_bind_st *param)
{
    void *data;

    assert_magic(stmt);
    assert(param);

    return_error_if_fail(index < stmt->params, E_DB_OUT_OF_BOUNDS, -1);

    data = param->data.blob;

    if(param->mode == DB_PARAM_BIND_TMP && !(data = memdup(data, param->vsize)))
        return error_wrap_last_errno(memdup), -1;

    db_stmt_free_param(&stmt->param[index]);

    stmt->param[index]              = *param;
    stmt->param[index].data.blob    = data;

    return 0;
}

const db_param_bind_st *db_stmt_param_get(db_stmt_const_ct stmt, size_t index)
{
    assert_magic(stmt);

    return_error_if_fail(index < stmt->params, E_DB_OUT_OF_BOUNDS, NULL);

    return &stmt->param[index];
}

int db_stmt_param_map(db_stmt_const_ct stmt, db_param_map_cb map, const void *ctx)
{
    size_t p;
    int rc;

    assert_magic(stmt);
    assert(map);

    for(p = 0; p < stmt->params; p++)
    {
        if((rc = map(stmt, p, &stmt->param[p], (void *)ctx)))
            return error_pack_int(E_DB_CALLBACK, rc);
    }

    return 0;
}

int db_stmt_result_init(db_stmt_ct stmt, size_t n, size_t stsize)
{
    assert_magic(stmt);
    assert(!stmt->result || stmt->results == n);

    return_value_if_pass(!n || stmt->results == n, 0);

    if(!(stmt->result = calloc(n, sizeof(db_result_bind_st))))
        return error_wrap_last_errno(calloc), -1;

    if(stsize && !(stmt->state = calloc(n, stsize)))
        return error_wrap_last_errno(calloc), free(stmt->result), -1;

    stmt->results   = n;
    stmt->stsize    = stsize;

    return 0;
}

size_t db_stmt_result_count(db_stmt_const_ct stmt)
{
    assert_magic(stmt);

    return stmt->results;
}

int db_stmt_result_bind(db_stmt_ct stmt, size_t index, const db_result_bind_st *result)
{
    assert_magic(stmt);
    assert(result);

    return_error_if_fail(index < stmt->results, E_DB_OUT_OF_BOUNDS, -1);

    stmt->result[index] = *result;

    return 0;
}

const db_result_bind_st *db_stmt_result_get(db_stmt_const_ct stmt, size_t index)
{
    assert_magic(stmt);

    return_error_if_fail(index < stmt->results, E_DB_OUT_OF_BOUNDS, NULL);

    return &stmt->result[index];
}

int db_stmt_result_map(db_stmt_const_ct stmt, db_result_map_cb map, const void *ctx)
{
    db_result_bind_st *result;
    void *state;
    size_t r;
    int rc;

    assert_magic(stmt);
    assert(map);

    for(r = 0; r < stmt->results; r++)
    {
        result  = &stmt->result[r];
        state   = stmt->state ? &stmt->state[r * stmt->stsize] : NULL;

        if((rc = map(stmt, r, result, state, (void *)ctx)))
            return error_pack_int(E_DB_CALLBACK, rc);
    }

    return 0;
}
