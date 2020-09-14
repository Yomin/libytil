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

#include <ytil/db/sqlite.h>
#include <ytil/db/interface.h>
#include <ytil/ext/string.h>
#include <ytil/def.h>
#include <sqlite3.h>
#include <stdlib.h>
#include <math.h>


/// Error description callback to retrieve SQLite error description.
///
/// \implements error_desc_cb
static const char *error_sqlite_desc(const error_type_st *type, int code, char *buf, size_t size)
{
    return sqlite3_errstr(code);
}

/// Error out-of-memory check callback to check SQLite codes.
///
/// \implements error_oom_cb
static bool error_sqlite_is_oom(const error_type_st *type, int code)
{
    return code == SQLITE_NOMEM;
}

/// Error retrieval callback to retrieve last SQLITE code from a DB or STMT object.
///
/// \implements error_last_cb
static int error_sqlite_last(const error_type_st *type, const char **desc, const char *ctx_type, void *ctx)
{
    if(!strcmp(ctx_type, "db"))
        return sqlite3_errcode(db_get_ctx(ctx));

    if(!strcmp(ctx_type, "stmt"))
        return sqlite3_errcode(db_get_ctx(db_stmt_get_db(ctx)));

    abort();
}

/// SQLite error type definition
ERROR_DEFINE_CALLBACK(SQLITE,
    NULL, error_sqlite_desc, error_sqlite_is_oom, error_sqlite_last);

/// Error map callback to map SQLite error codes to DB error codes.
///
/// \implements error_map_cb
static int error_sqlite_map(const error_type_st *type, int code, void *ctx)
{
    switch(code)
    {
    case SQLITE_CONSTRAINT:
        return E_DB_CONSTRAINT;

    case SQLITE_ERROR:
        return E_DB_MALFORMED_SQL;

    case SQLITE_FULL:
        return E_DB_FULL;

    case SQLITE_RANGE:
        return E_DB_OUT_OF_BOUNDS;

    case SQLITE_TOOBIG:
        return E_DB_OUT_OF_RANGE;

    default:
        return E_DB_EXTENDED;
    }
}

/// Map sub function SQLite error to DB error.
///
/// \param sub      name of sub function
/// \param rc       sub rc
#define error_map_sqlite(sub, rc) \
    error_map_sub(error_sqlite_map, NULL, sub, SQLITE, rc)

/// Map last sub function SQLite error from DB object to DB error.
///
/// \param sub      name of sub function
/// \param db       database
#define error_map_last_sqlite(sub, db) \
    error_map_last_sub_x(error_sqlite_map, NULL, sub, SQLITE, "db", db)

/// Map last sub function SQLite error from STMT object to DB error.
///
/// \param sub      name of sub function
/// \param stmt     statement
#define error_map_last_sqlite_stmt(sub, stmt) \
    error_map_last_sub_x(error_sqlite_map, NULL, sub, SQLITE, "stmt", stmt)

/// default error type for sqlite module
#define ERROR_TYPE_DEFAULT ERROR_TYPE_DB


/// Close SQLite database.
///
/// \implements db_close_cb
static void db_sqlite_close(db_ct db)
{
    sqlite3_close(db_get_ctx(db));
}

/// Prepare SQLite statement.
///
/// \implements db_prepare_cb
static db_stmt_ct db_sqlite_prepare(db_ct db, const char *sql)
{
    sqlite3_stmt *stmt3 = NULL;
    const char *tail;
    db_stmt_ct stmt;
    int rc;

    if((rc = sqlite3_prepare_v3(db_get_ctx(db), sql, -1, 0, &stmt3, &tail)))
        return error_map_sqlite(sqlite3_prepare_v3, rc), NULL;

    if(!stmt3) // sql is empty or comment
        return error_set(E_DB_MALFORMED_SQL), NULL;

    if(tail[0])
        return error_set(E_DB_MULTI_STMT), sqlite3_finalize(stmt3), NULL;

    if(!(stmt = db_stmt_new(db, stmt3)))
        return error_pass(), sqlite3_finalize(stmt3), NULL;

    return stmt;
}

/// Finalize SQLite statement.
///
/// \implements db_finalize_cb
static void db_sqlite_finalize(db_stmt_ct stmt)
{
    sqlite3_finalize(db_stmt_get_ctx(stmt));
}

/// Load SQLite statement parameter.
///
/// \param stmt     statement
/// \param index    parameter index
/// \param param    parameter bind infos
///
/// \retval 0                   success
/// \retval -1/E_GENERIC_OOM    out of memory
static int db_sqlite_load_param(db_stmt_const_ct stmt, size_t index, const db_param_bind_st *param)
{
    sqlite3_stmt *stmt3 = db_stmt_get_ctx(stmt);
    int rc;

    if(param->type == DB_TYPE_NULL || (param->is_null && *param->is_null))
    {
        if((rc = sqlite3_bind_null(stmt3, index + 1)))
            return error_map_sqlite(sqlite3_bind_null, rc), -1;

        return 0;
    }

    switch(param->type)
    {
    case DB_TYPE_INT8:
    case DB_TYPE_INT16:
    case DB_TYPE_INT32:

        if((rc = sqlite3_bind_int(stmt3, index + 1, *(int *)param->data)))
            return error_map_sqlite(sqlite3_bind_int, rc), -1;

        break;

    case DB_TYPE_INT64:

        if((rc = sqlite3_bind_int64(stmt3, index + 1, *(int64_t *)param->data)))
            return error_map_sqlite(sqlite3_bind_int64, rc), -1;

        break;

    case DB_TYPE_DOUBLE:

        if((rc = sqlite3_bind_double(stmt3, index + 1, *(double *)param->data)))
            return error_map_sqlite(sqlite3_bind_double, rc), -1;

        break;

    case DB_TYPE_TEXT:

        switch(param->mode)
        {
        case DB_PARAM_BIND_FIX:
            rc = sqlite3_bind_text(stmt3, index + 1, param->data, param->vsize, NULL);
            break;

        case DB_PARAM_BIND_TMP:
            rc = sqlite3_bind_text(stmt3,
                index + 1, param->data, param->vsize, SQLITE_TRANSIENT);
            break;

        case DB_PARAM_BIND_REF:
            rc = sqlite3_bind_text(stmt3,
                index + 1, *(char **)param->data, param->rsize ? (int)*param->rsize : -1, NULL);
            break;

        default:
            abort();
        }

        if(rc)
            return error_map_sqlite(sqlite3_bind_text, rc), -1;

        break;

    case DB_TYPE_BLOB:

        switch(param->mode)
        {
        case DB_PARAM_BIND_FIX:
            rc = sqlite3_bind_blob(stmt3,
                index + 1, param->data, param->vsize, NULL);
            break;

        case DB_PARAM_BIND_TMP:
            rc = sqlite3_bind_blob(stmt3,
                index + 1, param->data, param->vsize, SQLITE_TRANSIENT);
            break;

        case DB_PARAM_BIND_REF:
            rc = sqlite3_bind_blob(stmt3,
                index + 1, *(void **)param->data, *param->rsize, NULL);
            break;

        default:
            abort();
        }

        if(rc)
            return error_map_sqlite(sqlite3_bind_blob, rc), -1;

        break;

    default:
        abort();
    }

    return 0;
}

/// Update SQLite referenced statement parameter.
///
/// \implements db_param_map_cb
static int db_sqlite_update_param(db_stmt_const_ct stmt, size_t index, const db_param_bind_st *param, void *ctx)
{
    if(param->mode != DB_PARAM_BIND_REF)
        return 0;

    return error_pass_int(db_sqlite_load_param(stmt, index, param));
}

/// Update SQLite referenced statement parameters.
///
/// \param stmt     statement
///
/// \retval 0                   success
/// \retval -1/E_GENERIC_OOM    out of memory
static int db_sqlite_update_params(db_stmt_const_ct stmt)
{
    return error_lift_int(E_DB_CALLBACK,
        db_stmt_param_map(stmt, db_sqlite_update_param, NULL));
}

/// Fetch SQLite text result field.
///
/// \param stmt     statement
/// \param index    field index
/// \param result   result bind infos
///
/// \returns                    text size
/// \retval -1/E_GENERIC_OOM    out of memory
static ssize_t db_sqlite_fetch_text(db_stmt_const_ct stmt, size_t index, const db_result_bind_st *result)
{
    sqlite3_stmt *stmt3 = db_stmt_get_ctx(stmt);
    size_t size, copysize;
    char *text;
    int rc;

    if(!(text = (char *)sqlite3_column_text(stmt3, index)))
    {
        rc = sqlite3_errcode(db_get_ctx(db_stmt_get_db(stmt)));

        if(rc != SQLITE_OK && rc != SQLITE_DONE && rc != SQLITE_ROW)
            return error_map_sqlite(sqlite3_column_text, rc), -1;
    }

    size = sqlite3_column_bytes(stmt3, index);

    switch(result->mode)
    {
    case DB_RESULT_BIND_TMP:
        *(char **)result->data = text;
        break;

    case DB_RESULT_BIND_DUP:
        if(!text)
            *(char **)result->data = NULL;
        else if(!(*(char **)result->data = memdup(text, size + 1)))
            return error_wrap_last_errno(memdup), -1;
        break;

    case DB_RESULT_BIND_FIX:
        if(result->data && result->cap)
        {
            copysize = MIN(result->cap - 1, size);
            memcpy(result->data, text, copysize);
            ((char *)result->data)[copysize] = '\0';
        }
        break;

    default:
        abort();
    }

    return size;
}

/// Fetch SQLite BLOB result field.
///
/// \param stmt     statement
/// \param index    field index
/// \param result   result bind infos
///
/// \returns                    BLOB size
/// \retval -1/E_GENERIC_OOM    out of memory
static ssize_t db_sqlite_fetch_blob(db_stmt_const_ct stmt, size_t index, const db_result_bind_st *result)
{
    sqlite3_stmt *stmt3 = db_stmt_get_ctx(stmt);
    size_t size;
    void *blob;
    int rc;

    if(!(blob = (void *)sqlite3_column_blob(stmt3, index)))
    {
        rc = sqlite3_errcode(db_get_ctx(db_stmt_get_db(stmt)));

        if(rc != SQLITE_OK && rc != SQLITE_DONE && rc != SQLITE_ROW)
            return error_map_sqlite(sqlite3_column_blob, rc), -1;
    }

    size = sqlite3_column_bytes(stmt3, index);

    switch(result->mode)
    {
    case DB_RESULT_BIND_TMP:
        *(void **)result->data = blob;
        break;

    case DB_RESULT_BIND_DUP:
        if(!blob)
            *(void **)result->data = NULL;
        else if(!(*(void **)result->data = memdup(blob, size)))
            return error_wrap_last_errno(memdup), -1;
        break;

    case DB_RESULT_BIND_FIX:
        if(result->data && result->cap)
            memcpy(result->data, blob, MIN(result->cap, size));
        break;

    default:
        abort();
    }

    return size;
}

/// Fetch SQLite result field.
///
/// \param stmt     statement
/// \param index    field index
/// \param result   result bind infos
/// \param ctx      unused
///
/// \retval 0                   success
/// \retval -1/E_GENERIC_OOM    out of memory
static int db_sqlite_fetch_field(db_stmt_const_ct stmt, size_t index, const db_result_bind_st *result, void *ctx)
{
    sqlite3_stmt *stmt3 = db_stmt_get_ctx(stmt);
    bool is_null;
    ssize_t size;

    is_null = sqlite3_column_type(stmt3, index) == SQLITE_NULL;

    if(result->is_null)
        *result->is_null = is_null;

    switch(result->type)
    {
    case DB_TYPE_INVALID:   // nothing bound
    case DB_TYPE_NULL:
        size = 0;
        break;

    case DB_TYPE_INT8:
    case DB_TYPE_INT16:
    case DB_TYPE_INT32:
        if(result->data)
            *(int *)result->data = sqlite3_column_int(stmt3, index);

        size = sizeof(int);
        break;

    case DB_TYPE_INT64:
        if(result->data)
            *(int64_t *)result->data = sqlite3_column_int64(stmt3, index);

        size = sizeof(int64_t);
        break;

    case DB_TYPE_DOUBLE:
        if(result->data)
        {
            if(is_null)
                *(double *)result->data = NAN;
            else
                *(double *)result->data = sqlite3_column_double(stmt3, index);
        }

        size = sizeof(double);
        break;

    case DB_TYPE_TEXT:
        if((size = db_sqlite_fetch_text(stmt, index, result)) < 0)
            return error_pass(), -1;
        break;

    case DB_TYPE_BLOB:
        if((size = db_sqlite_fetch_blob(stmt, index, result)) < 0)
            return error_pass(), -1;
        break;

    default:
        abort();
    }

    if(result->size)
        *result->size = size;

    return 0;
}

/// Fetch SQLite result fields.
///
/// \param stmt     statement
///
/// \retval 0                   success
/// \retval -1/E_GENERIC_OOM    out of memory
static int db_sqlite_fetch_fields(db_stmt_const_ct stmt)
{
    return error_lift_int(E_DB_CALLBACK,
        db_stmt_result_map(stmt, db_sqlite_fetch_field, NULL));
}

/// Get SQL from SQLite statement.
///
/// \implements db_sql_cb
static const char *db_sqlite_sql(db_stmt_ct stmt, db_sql_id type)
{
    sqlite3_stmt *stmt3 = db_stmt_get_ctx(stmt);
    const char *sql;

    switch(type)
    {
    case DB_SQL_PLAIN:
        return sqlite3_sql(stmt3);

    case DB_SQL_EXPANDED:

        if(db_sqlite_update_params(stmt))
            return error_pass(), NULL;

        if(!(sql = sqlite3_expanded_sql(stmt3)))
            return error_map_last_sqlite_stmt(sqlite3_expanded_sql, stmt), NULL;

        return sql;

    default:
        return_error_if_reached(E_DB_UNSUPPORTED, NULL);
    }
}

/// Execute SQLite statement.
///
/// \implements db_exec_cb
static int db_sqlite_exec(db_stmt_ct stmt, db_record_cb record, void *ctx)
{
    sqlite3_stmt *stmt3 = db_stmt_get_ctx(stmt);
    size_t row;
    int rc;

    if(db_sqlite_update_params(stmt))
        return error_pass(), -1;

    for(row = 0; (rc = sqlite3_step(stmt3)) == SQLITE_ROW; row++)
    {
        if(db_sqlite_fetch_fields(stmt))
            return error_pass(), sqlite3_reset(stmt3), -1;

        if(!record)
            return sqlite3_reset(stmt3), 0;

        if((rc = record(stmt, row, ctx)))
            return sqlite3_reset(stmt3), error_pack_int(E_DB_CALLBACK, rc);
    }

    sqlite3_reset(stmt3);

    if(rc != SQLITE_DONE)
        return error_map_sqlite(sqlite3_step, rc), -1;

    return 0;
}

/// Get number of SQLite statement parameters.
///
/// \implements db_param_count_cb
static ssize_t db_sqlite_param_count(db_stmt_const_ct stmt)
{
    return sqlite3_bind_parameter_count(db_stmt_get_ctx(stmt));
}

/// Bind SQLite statement parameter.
///
/// \implements db_param_bind_cb
static int db_sqlite_param_bind(db_stmt_ct stmt, size_t index, const db_param_bind_st *bind)
{
    size_t params = sqlite3_bind_parameter_count(db_stmt_get_ctx(stmt));

    return_error_if_fail(index < params, E_DB_OUT_OF_BOUNDS, -1);

    switch(bind->type)
    {
    case DB_TYPE_NULL:
    case DB_TYPE_INT64:
    case DB_TYPE_DOUBLE:
    case DB_TYPE_TEXT:
    case DB_TYPE_BLOB:
        break;

    case DB_TYPE_INT8:
        return_error_if_fail(sizeof(int) == sizeof(int8_t), E_DB_UNSUPPORTED_TYPE, -1);
        break;

    case DB_TYPE_INT16:
        return_error_if_fail(sizeof(int) == sizeof(int16_t), E_DB_UNSUPPORTED_TYPE, -1);
        break;

    case DB_TYPE_INT32:
        return_error_if_fail(sizeof(int) == sizeof(int32_t), E_DB_UNSUPPORTED_TYPE, -1);
        break;

    default:
        return_error_if_reached(E_DB_UNSUPPORTED_TYPE, -1);
    }

    switch(bind->mode)
    {
    case DB_PARAM_BIND_FIX:
    case DB_PARAM_BIND_TMP:
        return error_pass_int(db_sqlite_load_param(stmt, index, bind));

    case DB_PARAM_BIND_REF:

        if(db_stmt_param_init(stmt, params) || db_stmt_param_bind(stmt, index, bind))
            return error_pass(), -1;

        return 0;

    default:
        return_error_if_reached(E_DB_UNSUPPORTED_MODE, -1);
    }
}

/// Get number of SQLite result fields.
///
/// \implements db_result_count_cb
static ssize_t db_sqlite_result_count(db_stmt_const_ct stmt)
{
    return sqlite3_column_count(db_stmt_get_ctx(stmt));
}

/// Bind SQLite result field.
///
/// \implements db_result_bind_cb
static int db_sqlite_result_bind(db_stmt_ct stmt, size_t index, const db_result_bind_st *bind)
{
    size_t fields = sqlite3_column_count(db_stmt_get_ctx(stmt));

    return_error_if_fail(index < fields, E_DB_OUT_OF_BOUNDS, -1);

    switch(bind->type)
    {
    case DB_TYPE_NULL:
        break;

    case DB_TYPE_INT8:
        if(sizeof(int) != sizeof(int8_t) || bind->mode != DB_RESULT_BIND_FIX)
            return error_set(E_DB_UNSUPPORTED_TYPE), -1;
        break;

    case DB_TYPE_INT16:
        if(sizeof(int) != sizeof(int16_t) || bind->mode != DB_RESULT_BIND_FIX)
            return error_set(E_DB_UNSUPPORTED_TYPE), -1;
        break;

    case DB_TYPE_INT32:
        if(sizeof(int) != sizeof(int32_t) || bind->mode != DB_RESULT_BIND_FIX)
            return error_set(E_DB_UNSUPPORTED_TYPE), -1;
        break;

    case DB_TYPE_INT64:
    case DB_TYPE_DOUBLE:
        if(bind->mode != DB_RESULT_BIND_FIX)
            return error_set(E_DB_UNSUPPORTED_TYPE), -1;
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

    if(db_stmt_result_init(stmt, fields) || db_stmt_result_bind(stmt, index, bind))
        return error_pass(), -1;

    return 0;
}

/// Fetch SQLite field.
///
/// \implements db_result_fetch_cb
static int db_sqlite_result_fetch(db_stmt_const_ct stmt, size_t index, size_t offset)
{
    const db_result_bind_st *result;

    return_error_if_pass(offset, E_DB_OUT_OF_RANGE, -1);

    if(!(result = db_stmt_result_get(stmt, index)))
        return error_pass(), -1;

    return error_pass_int(db_sqlite_fetch_field(stmt, index, result, NULL));
}

/// Get SQLite database/table/field name.
///
/// \implements db_name_cb
static const char *db_sqlite_result_name(db_stmt_const_ct stmt, size_t index, db_name_id type)
{
    sqlite3_stmt *stmt3 = db_stmt_get_ctx(stmt);
    const char *name;

    if(index >= (unsigned)sqlite3_column_count(stmt3))
        return error_set(E_DB_OUT_OF_BOUNDS), NULL;

    switch(type)
    {
    case DB_NAME_DATABASE:
        name = sqlite3_column_database_name(stmt3, index);
        break;

    case DB_NAME_TABLE_ORG:
        name = sqlite3_column_table_name(stmt3, index);
        break;

    case DB_NAME_FIELD:
        name = sqlite3_column_name(stmt3, index);
        break;

    case DB_NAME_FIELD_ORG:
        name = sqlite3_column_origin_name(stmt3, index);
        break;

    default:
        return_error_if_reached(E_DB_UNSUPPORTED, NULL);
    }

    return_error_if_fail(name, E_DB_NO_NAME, NULL);

    return name;
}

/// SQLite event handler which invokes DB event handler.
///
/// \implements sqlite3_xCallback
static int db_sqlite_event(unsigned int type, void *ctx, void *vstmt, void *vsql)
{
    db_ct db = ctx;
    sqlite3_stmt *stmt = vstmt;
    char *sql = vsql, *esql;

    if((esql = sqlite3_expanded_sql(stmt)))
    {
        db_event(db, esql);
        sqlite3_free(esql);
    }
    else
    {
        db_event(db, sql);
    }

    return 0;
}

/// Enable/disable SQLite tracing.
///
/// \implements db_trace_cb
static int db_sqlite_trace(db_ct db, bool enable)
{
    int rc;

    if((rc = sqlite3_trace_v2(db_get_ctx(db), SQLITE_TRACE_STMT, enable ? db_sqlite_event : NULL, db)))
        return error_map_sqlite(sqlite_trace_v2, rc), -1;

    return 0;
}

/// SQLite interface definition
static const db_interface_st sqlite =
{
    .close          = db_sqlite_close,
    .prepare        = db_sqlite_prepare,
    .finalize       = db_sqlite_finalize,
    .sql            = db_sqlite_sql,
    .exec           = db_sqlite_exec,
    .param_count    = db_sqlite_param_count,
    .param_bind     = db_sqlite_param_bind,
    .result_count   = db_sqlite_result_count,
    .result_bind    = db_sqlite_result_bind,
    .result_fetch   = db_sqlite_result_fetch,
    .result_name    = db_sqlite_result_name,
    .trace          = db_sqlite_trace,
};

/// Open SQLite database.
///
/// \param name     database name
///
/// \returns                    db handle
/// \retval NULL/E_GENERIC_OOM  out of memory
static db_ct db_sqlite_open_f(const char *name)
{
    sqlite3 *db3 = NULL;
    db_ct db;
    int rc;

    if((rc = sqlite3_open_v2(name, &db3, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL)))
        return error_map_sqlite(sqlite3_open_v2, rc), sqlite3_close(db3), NULL;

    if(!(db = db_new(&sqlite, db3)))
        return error_pass(), sqlite3_close(db3), NULL;

    return db;
}

db_ct db_sqlite_open(path_const_ct file)
{
    str_ct path;
    db_ct db;

    assert(file);

    if(!(path = path_get(file, PATH_STYLE_NATIVE)))
        return error_wrap(), NULL;

    db = db_sqlite_open_f(str_c(path));

    str_unref(path);

    return error_pass_ptr(db);
}

db_ct db_sqlite_open_memory(void)
{
    return error_pass_ptr(db_sqlite_open_f(":memory:"));
}

db_ct db_sqlite_open_private(void)
{
    return error_pass_ptr(db_sqlite_open_f(""));
}
