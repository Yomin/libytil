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
#include <ytil/gen/str.h>
#include <stdlib.h>


#define MAGIC define_magic("DBS")   ///< statement magic

/// database statement
typedef struct db_stmt
{
    DEBUG_MAGIC

    db_ct   db;                     ///< database
    void    *ctx;                   ///< statement context

    char    *sql;                   ///< SQL
    str_ct  esql;                   ///< escaped SQL

    db_param_bind_st    *param;     ///< parameters
    size_t              params;     ///< number of parameters
    db_result_bind_st   *result;    ///< result fields
    size_t              results;    ///< number of result fields

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

static void db_stmt_free_params(db_stmt_ct stmt)
{
    free(stmt->param); /// \todo

    stmt->param     = NULL;
    stmt->params    = 0;
}

static void db_stmt_free_results(db_stmt_ct stmt)
{
    free(stmt->result);

    stmt->result    = NULL;
    stmt->results   = 0;
}

int db_stmt_finalize(db_stmt_ct stmt)
{
    db_finalize_cb finalize = db_stmt_get_interface(stmt)->finalize;

    return_error_if_pass(stmt->executing, E_DB_ILLEGAL, -1);

    if(finalize)
        finalize(stmt);

    db_unref(stmt->db);

    free(stmt->sql);

    if(stmt->esql)
        str_unref(stmt->esql);

    db_stmt_free_params(stmt);
    db_stmt_free_results(stmt);

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

int db_stmt_prepare(db_stmt_ct stmt, const char *sql)
{
    const char *ptr;
    size_t params;
    char *dsql;

    assert_magic(stmt);

    return_error_if_fail(sql[0], E_DB_MALFORMED_SQL, -1);
    return_error_if_pass((ptr = strchr(sql, ';')) && ptr[1], E_DB_MULTI_STMT, -1);

    for(params = 0; (sql = strchr(sql, '?')); sql++)
    {
        if(sql[1] == '?') // ?? = literal ?
            sql++;
        else
            params++;
    }

    if(!(dsql = strdup(sql)))
        return error_wrap_last_errno(strdup), -1;

    if(db_stmt_param_init(stmt, params))
        return error_pass(), free(dsql), -1;

    free(stmt->sql);
    stmt->sql = dsql;

    return 0;
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
        return stmt->sql; /// \todo

    default:
        abort();
    }
}

const char *db_stmt_escape_sql(db_stmt_ct stmt, const char *sql)
{
    assert_magic(stmt);
    assert(sql);

    if(stmt->esql)
        str_set_s(stmt->esql, sql);
    else if(!(stmt->esql = str_new_s(sql)))
        return error_wrap(), NULL;

    if(!str_escape(stmt->esql))
        return error_wrap(), NULL;

    return str_c(stmt->esql);
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

int db_stmt_param_bind(db_stmt_ct stmt, size_t index, const db_param_bind_st *param)
{
    assert_magic(stmt);

    return_error_if_fail(index < stmt->params, E_DB_OUT_OF_BOUNDS, -1);

    stmt->param[index] = *param;

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

int db_stmt_result_init(db_stmt_ct stmt, size_t n)
{
    assert_magic(stmt);
    assert(!stmt->result || stmt->results == n);

    return_value_if_pass(!n || stmt->results == n, 0);

    if(!(stmt->result = calloc(n, sizeof(db_result_bind_st))))
        return error_wrap_last_errno(calloc), -1;

    stmt->results = n;

    return 0;
}

int db_stmt_result_bind(db_stmt_ct stmt, size_t index, const db_result_bind_st *result)
{
    assert_magic(stmt);

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
    size_t r;
    int rc;

    assert_magic(stmt);
    assert(map);

    for(r = 0; r < stmt->results; r++)
    {
        if((rc = map(stmt, r, &stmt->result[r], (void *)ctx)))
            return error_pack_int(E_DB_CALLBACK, rc);
    }

    return 0;
}
