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
#include <ytil/def.h>
#include <ytil/ext/stdio.h>
#include <ctpublic.h>
#include <sybdb.h>
#include <stdlib.h>
#include <errno.h>


/// TDS DB
typedef struct db_tds
{
    CS_CONTEXT      *ctx;   ///< context
    CS_CONNECTION   *con;   ///< connection
    size_t          id;     ///< next statement ID
} db_tds_st;

typedef struct db_tds_result
{
    CS_SMALLINT     indicator;
} db_tds_result_st;


/// TDS error type definition
ERROR_DEFINE_CALLBACK(TDS, NULL, NULL, NULL, NULL);

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

    case 229:   // permission denied on object
    case 230:   // permission denied on column
    case 916:   // principal is not able to access database
        return E_DB_ACCESS_DENIED;

    case 17053: // OS_ERROR
    case 17803: // SRV_NOMEMORY
        return E_DB_BUSY;

    //case ?:
      //  return E_DB_INCOMPATIBLE;

    case 18452: // LOGON_INVALID_CONNECT
    case 18456: // LOGON_FAILED
    case 18470: // account is disabled
        return E_DB_LOGIN_FAILED;

    case 102:   // incorrect syntax
    case 105:   // unclosed quotation mark
    case 207:   // invalid column name
    case 343    // unknown object type
    case 1038:  // empty object or column name
        return E_DB_MALFORMED_SQL;

    case 17809: // SRV_NOFREESRVPROC
        return E_DB_MAX_CONNECTIONS;

    case 911:   // database does not exist
        return E_DB_UNKNOWN_DATABASE;

    case 4019:  // requested language not available
        return E_DB_UNKNOWN_LANGUAGE;

    default:
        return E_DB_EXTENDED;
    }
}

/// Clear error and execute CTLib expression.
///
/// \param expr     CTLib expression
///
/// \retval rc      expression rc
#define EXEC(expr) __extension__ ({ \
    error_clear(); \
    errno = 0; \
    \
    (expr); \
})

/// Clear error, execute CTLib expression and swap CS_SUCCEED/CS_FAIL rc.
///
/// \param expr     CTLib expression
///
/// \retval 0       CS_SUCCEED
/// \retval 1       CS_FAIL
/// \retval rc      expression rc
#define EXEC0(expr) __extension__ ({ \
    error_clear(); \
    errno = 0; \
    \
    int rc = (expr); \
    \
    rc == CS_FAIL ? 1 : rc == CS_SUCCEED ? 0 : rc; \
})

/// Map last sub function TDS error to DB error.
///
/// \param sub      name of sub function
/// \param desc     error description in case neither error nor errno set
#define error_map_last_tds(sub, desc, ...) __extension__ ({ \
    if(error_depth()) \
        error_map_pre_sub(error_tds_map, NULL, sub); \
    else if(errno) \
        error_wrap_errno(sub, errno); \
    else \
        error_wrap_sub_d(sub, TDS, 0, __VA_OPT__(FMT)((desc) __VA_OPT__(,) __VA_ARGS__)); \
})

#define goto_error(sub, desc, ...) \
    do { error_map_last_tds(sub, (desc) __VA_OPT__(,) __VA_ARGS__); goto error; } while(0)

/// default error type for TDS module
#define ERROR_TYPE_DEFAULT ERROR_TYPE_DB


/// Close TDS database.
///
/// \implements db_close_cb
static void db_tds_close(db_ct db)
{
    db_tds_st *tdb = db_get_ctx(db);

    ct_close(tdb->con, CS_UNUSED);
    ct_con_drop(tdb->con);

    ct_exit(tdb->ctx, CS_UNUSED);
    cs_ctx_drop(tdb->ctx);

    free(tdb);
}

typedef int (*db_tds_result_cb)(db_stmt_ct stmt, CS_COMMAND *cmd, void *ctx);

static int db_tds_cmd(db_stmt_ct stmt, int cmd_type, const char *arg, int result_type, db_tds_result_cb result, void *ctx)
{
    db_tds_st *tdb  = db_get_ctx(db_stmt_get_db(stmt));
    char *id        = (char *)db_stmt_get_name(stmt);
    CS_COMMAND *cmd = NULL;
    bool had_result = false;
    int rc, type;

    if(EXEC0(ct_cmd_alloc(tdb->con, &cmd)))
        return error_map_last_tds(ct_cmd_alloc, "Failed to alloc command."), -1;

    if(EXEC0(ct_dynamic(cmd, cmd_type, id, CS_NULLTERM, (char *)arg, arg ? CS_NULLTERM : CS_UNUSED)))
        goto_error(ct_dynamic, "Failed to prepare cmd.");

    if(EXEC0(ct_send(cmd)))
        goto_error(ct_send, "Failed to send cmd.");

    while(1)
    {
        switch(rc = EXEC(ct_results(cmd, &type)))
        {
        case CS_SUCCEED:
            break;

        case CS_END_RESULTS:

            if(result && !had_result)
                goto_error(ct_results, "Missing result.");

            ct_cmd_drop(cmd);

            return 0;

        case CS_FAIL:
            goto_error(ct_results, "Failed to get result.");

        default:
            goto_error(ct_results, "Unexpected result rc %i.", rc);
        }

        switch(type)
        {
        case CS_CMD_DONE:
        case CS_CMD_SUCCEED:
            continue;

        case CS_CMD_FAIL:
            goto_error(ct_results, "Failed to get result.");

        default:

            if(!result || type != result_type)
                goto_error(ct_results, "Unexpected result type %i.", type);

            if(had_result)
                goto_error(ct_results, "Multi result.");

            if(result(stmt, cmd, ctx))
            {
                error_pass();
                goto error;
            }

            had_result = true;
        }
    }

error:
    ct_cmd_drop(cmd);

    return -1;
}

static int db_tds_prepare_result(db_stmt_ct stmt, CS_COMMAND *cmd, void *ctx)
{
    int fields;

    if(EXEC0(ct_res_info(cmd, CS_NUMDATA, &fields, CS_UNUSED, NULL)))
        return error_map_last_tds(ct_res_info, "Failed to get number of result fields."), -1;

    if(db_stmt_result_init(stmt, fields, sizeof(db_tds_result_st)))
        return error_pass(), -1;

    printf("result fields: %i\n", fields);

    return 0;
}

/// Prepare TDS statement.
///
/// \implements db_prepare_cb
static db_stmt_ct db_tds_prepare(db_ct db, const char *sql)
{
    db_tds_st *tdb = db_get_ctx(db);
    db_stmt_ct stmt;
    char id[100];

    printf("SQL: %s\n", sql);

    if(!(stmt = db_stmt_prepare(db, sql, NULL)))
        return error_pass(), NULL;

    snprintf(id, sizeof(id), "%zu", tdb->id++);

    if(db_stmt_set_name(stmt, id))
        return error_pass(), db_finalize(stmt), NULL;

    if(db_tds_cmd(stmt, CS_PREPARE, sql, CS_CMD_SUCCEED, NULL, NULL))
        return error_pass(), db_finalize(stmt), NULL;

    if(db_tds_cmd(stmt, CS_DESCRIBE_OUTPUT, NULL, CS_DESCRIBE_RESULT, db_tds_prepare_result, NULL))
        return error_pass(), db_finalize(stmt), NULL;

    return stmt;
}

/// Get SQL from TDS statement.
///
/// \implements db_sql_cb
const char *db_tds_sql(db_stmt_ct stmt, db_sql_id type)
{
    return error_pass_ptr(db_stmt_sql(stmt, type));
}

static int db_tds_get_bind_type(db_type_id type)
{
    switch(type)
    {
    case DB_TYPE_UINT8:
        return CS_TINYINT_TYPE;

    case DB_TYPE_INT16:
        return CS_SMALLINT_TYPE;

    case DB_TYPE_UINT16:
        return CS_USMALLINT_TYPE;

    case DB_TYPE_INT32:
        return CS_INT_TYPE;

    case DB_TYPE_UINT32:
        return CS_UINT_TYPE;

    case DB_TYPE_INT64:
        return CS_BIGINT_TYPE;

    case DB_TYPE_UINT64:
        return CS_UBIGINT_TYPE;

    case DB_TYPE_FLOAT:
        return CS_REAL_TYPE;

    case DB_TYPE_DOUBLE:
        return CS_FLOAT_TYPE;

    case DB_TYPE_TEXT:
        return CS_CHAR_TYPE;

    case DB_TYPE_BLOB:
        return CS_BINARY_TYPE;

    case DB_TYPE_DATE:
        return CS_DATE_TYPE;

    case DB_TYPE_TIME:
        return CS_TIME_TYPE;

    case DB_TYPE_DATETIME:
    case DB_TYPE_TIMESTAMP:
        return CS_DATETIME_TYPE;

    default:
        abort();
    }
}

/// Bind TDS result field.
///
/// \implements db_result_map_cb
static int db_tds_bind_result(db_stmt_const_ct stmt, size_t index, const db_result_bind_st *result, void *state, void *ctx)
{
    db_tds_result_st *rstate    = state;
    CS_COMMAND *cmd             = ctx;
    CS_DATAFMT fmt;
    void *data;

    if(EXEC0(ct_describe(cmd, index + 1, &fmt)))
        return error_map_last_tds(ct_describe, "Failed to describe result field."), -1;

    data            = result->data.blob;
    fmt.datatype    = db_tds_get_bind_type(result->type);

    if(EXEC0(ct_bind(cmd, index + 1, &fmt, data, NULL, &rstate->indicator)))
        return error_map_last_tds(ct_bind, "Failed to bind result field."), -1;

    return 0;
}

/// Update TDS result field.
///
/// \implements db_result_map_cb
static int db_tds_update_result(db_stmt_const_ct stmt, size_t index, const db_result_bind_st *result, void *state, void *ctx)
{
    db_tds_result_st *rstate = state;

    if(result->is_null)
        *result->is_null = rstate->indicator < 0;

    return 0;
}

typedef struct db_tds_record
{
    db_record_cb    record;
    void            *ctx;
} db_tds_record_st;

static int db_tds_proc_result(db_stmt_ct stmt, CS_COMMAND *cmd, void *ctx)
{
    db_tds_record_st *state = ctx;
    size_t row;
    int rc;

    if(db_stmt_result_map(stmt, db_tds_bind_result, cmd))
        return error_lift(E_DB_CALLBACK), -1;

    for(row = 0;; row++)
    {
        switch(EXEC(ct_fetch(cmd, CS_UNUSED, CS_UNUSED, CS_UNUSED, NULL)))
        {
        case CS_SUCCEED:
        case CS_ROW_FAIL:
            break;

        case CS_END_DATA:
            return 0;

        case CS_FAIL:
            return error_map_last_tds(ct_fetch, "Failed to fetch row."), -1;

        default:
            return error_map_last_tds(ct_fetch, "Unexpected fetch rc %i.", rc), -1;
        }

        if(db_stmt_result_map(stmt, db_tds_update_result, cmd))
            return error_lift(E_DB_CALLBACK), -1;

        if(!state->record)
        {
            if(EXEC0(ct_cancel(NULL, cmd, CS_CANCEL_CURRENT)))
                return error_map_last_tds(ct_cancel, "Failed to cancel results."), -1;

            return 0;
        }

        if((rc = state->record(stmt, row, state->ctx)))
            return error_pack_int(E_DB_CALLBACK, rc);
    }
}

/// Execute TDS statement.
///
/// \implements db_exec_cb
static int db_tds_exec(db_stmt_ct stmt, db_record_cb record, void *ctx)
{
    db_tds_record_st state  = { .record = record, .ctx = ctx };

    return error_pass_int(
        db_tds_cmd(stmt, CS_EXECUTE, NULL, CS_ROW_RESULT, db_tds_proc_result, &state));
}

/// Get number of TDS statement parameters.
///
/// \implements db_param_count_cb
static ssize_t db_tds_param_count(db_stmt_const_ct stmt)
{
    return db_stmt_param_count(stmt);
}

/// Bind TDS statement parameter.
///
/// \implements db_param_bind_cb
static int db_tds_param_bind(db_stmt_ct stmt, size_t index, const db_param_bind_st *bind)
{
    switch(bind->type)
    {
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

/// Get number of TDS result fields.
///
/// \implements db_result_count_cb
static ssize_t db_tds_result_count(db_stmt_const_ct stmt)
{
    return db_stmt_result_count(stmt);
}

/// Bind TDS result field.
///
/// \implements db_result_bind_cb
static int db_tds_result_bind(db_stmt_ct stmt, size_t index, const db_result_bind_st *bind)
{
    switch(bind->type)
    {
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

    return error_pass_int(db_stmt_result_bind(stmt, index, bind));
}

/// Get TDS field type.
///
/// \implements db_type_cb
static db_type_id db_tds_result_type(db_stmt_const_ct stmt, size_t index)
{
    return_error_if_reached(E_DB_UNSUPPORTED, DB_TYPE_INVALID);
}

/// Get TDS field name.
///
/// \implements db_name_cb
static const char *db_tds_result_name(db_stmt_const_ct stmt, size_t index, db_name_id type)
{
    return_error_if_reached(E_DB_UNSUPPORTED, NULL);
}

/// TDS interface definition
static const db_interface_st tds =
{
    .close          = db_tds_close,
    .prepare        = db_tds_prepare,
    .sql            = db_tds_sql,
    .exec           = db_tds_exec,
    .param_count    = db_tds_param_count,
    .param_bind     = db_tds_param_bind,
    .result_count   = db_tds_result_count,
    .result_bind    = db_tds_result_bind,
    .result_type    = db_tds_result_type,
    .result_name    = db_tds_result_name,
};

static db_tds_st *db_tds_new(void)
{
    db_tds_st *db;

    if(!(db = calloc(1, sizeof(db_tds_st))))
        return error_wrap_last_errno(calloc), NULL;

    if(EXEC0(cs_ctx_alloc(CS_VERSION_100, &db->ctx)))
        goto_error(cs_ctx_alloc, "Failed to alloc context.");

    if(EXEC0(ct_con_alloc(db->ctx, &db->con)))
        goto_error(ct_con_alloc, "Failed to alloc connection.");

    return db;

error:
    if(db->ctx)
        cs_ctx_drop(db->ctx);

    free(db);

    return NULL;
}

#include <stdio.h>
static void db_tds_set_error(int code, const char *desc)
{
    if(code == 5701 || code == 5703) // language/database changed
        return;

    printf("%5i %s\n", code, desc);

    if(error_depth() && error_type(0) == ERROR_TYPE(TDS))
        return;

    error_set_sd(TDS, code, desc);
}

static int db_tds_handle_cs_msg(CS_CONTEXT *ctx, CS_CLIENTMSG *msg)
{
    db_tds_set_error(msg->msgnumber, msg->msgstring);

    return CS_SUCCEED;
}

static int db_tds_handle_ct_msg(CS_CONTEXT *ctx, CS_CONNECTION *con, CS_CLIENTMSG *msg)
{
    db_tds_set_error(msg->msgnumber, msg->msgstring);

    return CS_SUCCEED;
}

static int db_tds_handle_sr_msg(CS_CONTEXT *ctx, CS_CONNECTION *con, CS_SERVERMSG *msg)
{
    db_tds_set_error(msg->msgnumber, msg->text);

    return CS_SUCCEED;
}

static int db_tds_init_context(db_tds_st *db, int timeout)
{
    if(EXEC0(cs_config(db->ctx, CS_SET, CS_MESSAGE_CB, FUNC_TO_POINTER(db_tds_handle_cs_msg), CS_UNUSED, NULL)))
        return error_map_last_tds(cs_config, "Failed to set cs msg handler."), -1;

    if(EXEC0(ct_init(db->ctx, CS_VERSION_100)))
        return error_map_last_tds(ct_init, "Failed to init context."), -1;

    if(EXEC0(ct_callback(db->ctx, NULL, CS_SET, CS_CLIENTMSG_CB, FUNC_TO_POINTER(db_tds_handle_ct_msg))))
        return error_map_last_tds(ct_callback, "Failed to set ct msg handler."), -1;

    if(EXEC0(ct_callback(db->ctx, NULL, CS_SET, CS_SERVERMSG_CB, FUNC_TO_POINTER(db_tds_handle_sr_msg))))
        return error_map_last_tds(ct_callback, "Failed to set sr msg handler."), -1;

    timeout = timeout < 0 ? CS_NO_LIMIT : timeout;

    if(timeout && EXEC0(ct_config(db->ctx, CS_SET, CS_LOGIN_TIMEOUT, &timeout, CS_UNUSED, NULL)))
        return error_map_last_tds(ct_config, "Failed to set login timeout."), -1;

    return 0;
}

static int db_tds_init_connection(db_tds_st *db, const char *app, const char *user, const char *pass)
{
    if(app && EXEC0(ct_con_props(db->con, CS_SET, CS_APPNAME, (char *)app, CS_NULLTERM, NULL)))
        return error_map_last_tds(ct_con_props, "Failed to set app name."), -1;

    if(user && EXEC0(ct_con_props(db->con, CS_SET, CS_USERNAME, (char *)user, CS_NULLTERM, NULL)))
        return error_map_last_tds(ct_con_props, "Failed to set user."), -1;

    if(pass && EXEC0(ct_con_props(db->con, CS_SET, CS_PASSWORD, (char *)pass, CS_NULLTERM, NULL)))
        return error_map_last_tds(ct_con_props, "Failed to set password."), -1;

    return 0;
}

static int db_tds_set_locale(db_tds_st *db, const char *language, const char *charset)
{
    CS_LOCALE *loc = NULL;

    if(EXEC0(cs_loc_alloc(db->ctx, &loc)))
        return error_map_last_tds(cs_loc_alloc, "Failed to alloc locale."), -1;

    if(EXEC0(cs_locale(db->ctx, CS_SET, loc, CS_LC_ALL, NULL, CS_UNUSED, NULL)))
        goto_error(cs_locale, "Failed to init locale.");

    if(EXEC0(cs_locale(db->ctx, CS_SET, loc, CS_SYB_LANG, (char *)language, CS_NULLTERM, NULL)))
        goto_error(cs_locale, "Failed to set language.");

    if(EXEC0(cs_locale(db->ctx, CS_SET, loc, CS_SYB_CHARSET, (char *)charset, CS_NULLTERM, NULL)))
        goto_error(cs_locale, "Failed to set charset.");

    if(EXEC0(ct_con_props(db->con, CS_SET, CS_LOC_PROP, loc, CS_UNUSED, NULL)))
        goto_error(ct_con_props, "Failed to set locale.");

    cs_loc_drop(db->ctx, loc);

    return 0;

error:
    cs_loc_drop(db->ctx, loc);

    return -1;
}

static int db_tds_connect_host(db_tds_st *db, const char *host)
{
    host = host ? host : "localhost";

    if(EXEC0(ct_connect(db->con, (char *)host, CS_NULLTERM)))
        return error_map_last_tds(ct_connect, "Failed to connect."), -1;

    return 0;
}

static int db_tds_use_db(db_tds_st *db, const char *default_db)
{
    CS_COMMAND *cmd = NULL;
    char *sql       = FMT("use [%s];", default_db); /// \todo correct escaping
    int type;

    if(EXEC0(ct_cmd_alloc(db->con, &cmd)))
        return error_map_last_tds(ct_cmd_alloc, "Failed to alloc cmd."), -1;

    if(EXEC0(ct_command(cmd, CS_LANG_CMD, sql, CS_NULLTERM, CS_UNUSED)))
        goto_error(ct_command, "Failed to prepare SQL cmd.");

    if(EXEC0(ct_send(cmd)))
        goto_error(ct_send, "Failed to send SQL cmd.");

    if(EXEC0(ct_results(cmd, &type))) // CS_SUCCEED + CS_CMD_SUCCEED
        goto_error(ct_results, "Failed to eval SQL cmd.");

    ct_cmd_drop(cmd);

    return 0;

error:
    ct_cmd_drop(cmd);

    return -1;
}

db_ct db_tds_connect(const char *app, const char *host, const char *user, const char *password, int timeout, const char *default_db)
{
    db_tds_st *tdb;
    db_ct db;

    if(!(tdb = db_tds_new()))
        return error_pass(), NULL;

    if(db_tds_init_context(tdb, timeout))
        goto error;

    if(db_tds_init_connection(tdb, app, user, password))
        goto error;

    if(db_tds_set_locale(tdb, "english", "UTF-8"))
        goto error;

    if(db_tds_connect_host(tdb, host))
        goto error;

    if(default_db && db_tds_use_db(tdb, default_db))
        goto error;

    if((db = db_new(&tds, tdb)))
        return db;

error:
    ct_con_drop(tdb->con);
    cs_ctx_drop(tdb->ctx);
    free(tdb);

    return error_pass(), NULL;
}
