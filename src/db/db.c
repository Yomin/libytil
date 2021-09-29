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
#include <ytil/def/magic.h>
#include <ytil/ext/stdio.h>
#include <stdlib.h>


#define MAGIC define_magic("DBO")   ///< database object magic

/// database
typedef struct db
{
    DEBUG_MAGIC

    const db_interface_st   *interface;     ///< database interface
    void                    *ctx;           ///< database context
    size_t                  ref;            ///< statement references
    db_event_cb             event;          ///< event callback
    void                    *event_ctx;     ///< event context
} db_st;

/// DB error type definition
ERROR_DEFINE_LIST(DB,
    ERROR_INFO(E_DB_ACCESS_DENIED,      "Access denied."),
    ERROR_INFO(E_DB_BUSY,               "Server is busy."),
    ERROR_INFO(E_DB_CALLBACK,           "Callback error."),
    ERROR_INFO(E_DB_CONNECTION,         "Connection error."),
    ERROR_INFO(E_DB_CONSTRAINT,         "Constraint violation."),
    ERROR_INFO(E_DB_EXTENDED,           "Extended backend error."),
    ERROR_INFO(E_DB_FULL,               "Database is full."),
    ERROR_INFO(E_DB_ILLEGAL,            "Illegal operation."),
    ERROR_INFO(E_DB_INCOMPATIBLE,       "Client/Server are not compatible."),
    ERROR_INFO(E_DB_LOGIN_FAILED,       "Failed to login."),
    ERROR_INFO(E_DB_MALFORMED_SQL,      "Malformed SQL."),
    ERROR_INFO(E_DB_MAX_CONNECTIONS,    "Maximum connections."),
    ERROR_INFO(E_DB_MULTI_STMT,         "Multi statements unsupported."),
    ERROR_INFO(E_DB_NO_DB,              "No database selected."),
    ERROR_INFO(E_DB_NO_NAME,            "No name for result field available."),
    ERROR_INFO(E_DB_OUT_OF_BOUNDS,      "Index out of bounds."),
    ERROR_INFO(E_DB_OUT_OF_RANGE,       "Parameter out of range."),
    ERROR_INFO(E_DB_TYPE_MISMATCH,      "Type conversion not possible."),
    ERROR_INFO(E_DB_UNKNOWN_DATABASE,   "Unknown database."),
    ERROR_INFO(E_DB_UNKNOWN_HOST,       "Unknown host."),
    ERROR_INFO(E_DB_UNKNOWN_LANGUAGE,   "Unknown language."),
    ERROR_INFO(E_DB_UNKNOWN_TYPE,       "Unknown result type."),
    ERROR_INFO(E_DB_UNSUPPORTED,        "Unsupported function."),
    ERROR_INFO(E_DB_UNSUPPORTED_MODE,   "Unsupported mode."),
    ERROR_INFO(E_DB_UNSUPPORTED_TYPE,   "Unsupported type."),
);

/// default error type for db module
#define ERROR_TYPE_DEFAULT ERROR_TYPE_DB


db_ct db_new(const db_interface_st *interface, const void *ctx)
{
    db_ct db;

    assert(interface);

    if(!(db = calloc(1, sizeof(db_st))))
        return error_wrap_last_errno(calloc), NULL;

    init_magic(db);
    db->interface   = interface;
    db->ctx         = (void *)ctx;

    return db;
}

void db_ref(db_ct db)
{
    assert_magic(db);

    db->ref++;
}

void db_unref(db_ct db)
{
    assert_magic(db);
    assert(db->ref);

    db->ref--;
}

void *db_get_ctx(db_const_ct db)
{
    assert_magic(db);

    return db->ctx;
}

const db_interface_st *db_get_interface(db_const_ct db)
{
    assert_magic(db);

    return db->interface;
}

int db_close(db_ct db)
{
    assert_magic(db);

    return_error_if_pass(db->ref, E_DB_ILLEGAL, -1);

    if(db->interface->close)
        db->interface->close(db);

    free(db);

    return 0;
}

db_stmt_ct db_prepare(db_ct db, const char *sql)
{
    assert_magic(db);
    assert(sql);

    return_error_if_fail(db->interface->prepare, E_DB_UNSUPPORTED, NULL);

    return error_pass_ptr(db->interface->prepare(db, sql));
}

db_stmt_ct db_prepare_f(db_ct db, const char *sql, ...)
{
    assert_magic(db);
    assert(sql);

    return_error_if_fail(db->interface->prepare, E_DB_UNSUPPORTED, NULL);

    return error_pass_ptr(db->interface->prepare(db, VVFMT(sql)));
}

int db_finalize(db_stmt_ct stmt)
{
    return error_pass_int(db_stmt_finalize(stmt));
}

const char *db_sql(db_stmt_ct stmt)
{
    db_sql_cb get_sql = db_stmt_get_interface(stmt)->sql;

    return_error_if_fail(get_sql, E_DB_UNSUPPORTED, NULL);

    return error_pass_ptr(get_sql(stmt, DB_SQL_PLAIN));
}

const char *db_sql_e(db_stmt_ct stmt)
{
    db_sql_cb get_sql = db_stmt_get_interface(stmt)->sql;
    const char *sql;

    return_error_if_fail(get_sql, E_DB_UNSUPPORTED, NULL);

    if(!(sql = get_sql(stmt, DB_SQL_PLAIN)))
        return error_pass(), NULL;

    return error_pass_ptr(db_stmt_escape_sql(stmt, sql));
}

const char *db_sql_expanded(db_stmt_ct stmt)
{
    db_sql_cb get_sql = db_stmt_get_interface(stmt)->sql;

    return_error_if_fail(get_sql, E_DB_UNSUPPORTED, NULL);

    return error_pass_ptr(get_sql(stmt, DB_SQL_EXPANDED));
}

const char *db_sql_expanded_e(db_stmt_ct stmt)
{
    db_sql_cb get_sql = db_stmt_get_interface(stmt)->sql;
    const char *sql;

    return_error_if_fail(get_sql, E_DB_UNSUPPORTED, NULL);

    if(!(sql = get_sql(stmt, DB_SQL_EXPANDED)))
        return error_pass(), NULL;

    return error_pass_ptr(db_stmt_escape_sql(stmt, sql));
}

int db_exec(db_stmt_ct stmt)
{
    return error_pass_int(db_exec_f(stmt, NULL, NULL));
}

int db_exec_f(db_stmt_ct stmt, db_record_cb record, const void *ctx)
{
    db_exec_cb exec = db_stmt_get_interface(stmt)->exec;
    int rc;

    return_error_if_fail(exec, E_DB_UNSUPPORTED, -1);

    if(db_stmt_set_executing(stmt, true))
        return error_pass(), -1;

    rc = error_pass_int(exec(stmt, record, (void *)ctx));

    db_stmt_set_executing(stmt, false);

    return rc;
}

void db_event(db_const_ct db, const char *sql)
{
    assert_magic(db);
    assert(db->event);

    db->event(sql, db->event_ctx);
}

int db_trace(db_ct db, db_event_cb event, const void *ctx)
{
    int rc;

    assert_magic(db);

    return_error_if_fail(db->interface->trace, E_DB_UNSUPPORTED, -1);

    if((rc = db->interface->trace(db, !!event)))
        return error_pass_int(rc);

    db->event       = event;
    db->event_ctx   = (void *)ctx;

    return 0;
}
