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

#include "db.h"
#include <ytil/test/run.h>
#include <ytil/test/test.h>
#include <ytil/db/interface.h>

static const struct not_a_db
{
    int foo;
} not_a_db = { 123 }, not_a_stmt = { 123 };

static db_interface_st interface, mockup;
static db_ct db;
static db_stmt_ct stmt;


TEST_CASE_ABORT(db_new_invalid_interface)
{
    db_new(NULL, "foo");
}

TEST_CASE_ABORT(db_close_invalid_db1)
{
    db_close(NULL);
}

TEST_CASE_ABORT(db_close_invalid_db2)
{
    db_close((db_ct)&not_a_db);
}

TEST_CASE(db_new_close)
{
    test_ptr_success(db = db_new(&interface, NULL));
    test_int_success(db_close(db));
}

TEST_SETUP(db_new)
{
    test_ptr_success(db = db_new(&interface, "foo"));
}

TEST_TEARDOWN(db_close)
{
    test_int_success(db_close(db));
}

TEST_CASE_ABORT(db_get_ctx_invalid_db1)
{
    db_get_ctx(NULL);
}

TEST_CASE_ABORT(db_get_ctx_invalid_db2)
{
    db_get_ctx((db_ct)&not_a_db);
}

TEST_CASE_FIX(db_get_ctx, db_new, db_close)
{
    test_str_eq("foo", db_get_ctx(db));
}

TEST_CASE_ABORT(db_stmt_new_invalid_db1)
{
    db_stmt_new(NULL, "bar");
}

TEST_CASE_ABORT(db_stmt_new_invalid_db2)
{
    db_stmt_new((db_ct)&not_a_db, "bar");
}

TEST_CASE_ABORT(db_finalize_invalid_stmt1)
{
    db_finalize(NULL);
}

TEST_CASE_ABORT(db_finalize_invalid_stmt2)
{
    db_finalize((db_stmt_ct)&not_a_stmt);
}

TEST_CASE_FIX(db_stmt_new_finalize, db_new, db_close)
{
    test_ptr_success(stmt = db_stmt_new(db, NULL));
    test_int_success(db_finalize(stmt));
}

TEST_SETUP(db_stmt_new)
{
    test_setup(db_new);
    test_ptr_success(stmt = db_stmt_new(db, "bar"));
}

TEST_TEARDOWN(db_finalize)
{
    test_int_success(db_finalize(stmt));
    test_teardown(db_close);
}

TEST_CASE_FIX(db_close_open_stmt, db_stmt_new, db_finalize)
{
    test_int_error(db_close(db), E_DB_ILLEGAL);
}

TEST_CASE_ABORT(db_stmt_get_ctx_invalid_stmt1)
{
    db_stmt_get_ctx(NULL);
}

TEST_CASE_ABORT(db_stmt_get_ctx_invalid_stmt2)
{
    db_stmt_get_ctx((db_stmt_ct)&not_a_stmt);
}

TEST_CASE_FIX(db_stmt_get_ctx, db_stmt_new, db_finalize)
{
    test_str_eq("bar", db_stmt_get_ctx(stmt));
}

TEST_CASE_ABORT(db_stmt_get_db_invalid_stmt1)
{
    db_stmt_get_db(NULL);
}

TEST_CASE_ABORT(db_stmt_get_db_invalid_stmt2)
{
    db_stmt_get_db((db_stmt_ct)&not_a_stmt);
}

TEST_CASE_FIX(db_stmt_get_db, db_stmt_new, db_finalize)
{
    test_ptr_eq(db, db_stmt_get_db(stmt));
}

TEST_CASE_ABORT(db_prepare_invalid_db1)
{
    db_prepare(NULL, "sql");
}

TEST_CASE_ABORT(db_prepare_invalid_db2)
{
    db_prepare((db_ct)&not_a_db, "sql");
}

TEST_CASE_FIX_ABORT(db_prepare_invalid_sql, db_new, db_close)
{
    db_prepare(db, NULL);
}

TEST_CASE_FIX(db_prepare_unsupported, db_new, db_close)
{
    test_ptr_error(db_prepare(db, "sql"), E_DB_UNSUPPORTED);
}

TEST_SETUP(db_mockup_new)
{
    test_ptr_success(db = db_new(&mockup, "mockup"));
}

static db_stmt_ct db_mockup_prepare(db_ct db2, const char *sql)
{
    test_ptr_eq(db, db2);
    test_str_eq(sql, "sql");

    return test_trace_rc(db_stmt_new(db, "mockup"));
}

TEST_CASE_FIX(db_prepare, db_mockup_new, db_close)
{
    mockup.prepare = db_mockup_prepare;

    test_ptr_success(stmt = db_prepare(db, "sql"));
    test_int_success(db_finalize(stmt));
}

TEST_CASE_ABORT(db_prepare_f_invalid_db1)
{
    db_prepare_f(NULL, "select %d;", 123);
}

TEST_CASE_ABORT(db_prepare_f_invalid_db2)
{
    db_prepare_f((db_ct)&not_a_db, "select %d;", 123);
}

TEST_CASE_FIX_ABORT(db_prepare_f_invalid_sql, db_new, db_close)
{
    db_prepare_f(db, NULL);
}

TEST_CASE_FIX(db_prepare_f_unsupported, db_new, db_close)
{
    test_ptr_error(db_prepare_f(db, "select %d;", 123), E_DB_UNSUPPORTED);
}

static db_stmt_ct db_mockup_prepare_f(db_ct db2, const char *sql)
{
    test_ptr_eq(db, db2);
    test_str_eq(sql, "select 123;");

    return test_trace_rc(db_stmt_new(db, "mockup"));
}

TEST_CASE_FIX(db_prepare_f, db_mockup_new, db_close)
{
    mockup.prepare = db_mockup_prepare_f;

    test_ptr_success(stmt = db_prepare_f(db, "select %d;", 123));
    test_int_success(db_finalize(stmt));
}

TEST_CASE_ABORT(db_sql_invalid_stmt1)
{
    db_sql(NULL);
}

TEST_CASE_ABORT(db_sql_invalid_stmt2)
{
    db_sql((db_stmt_ct)&not_a_stmt);
}

TEST_CASE_FIX(db_sql_unsupported, db_stmt_new, db_finalize)
{
    test_ptr_error(db_sql(stmt), E_DB_UNSUPPORTED);
}

TEST_SETUP(db_mockup_prepare)
{
    test_setup(db_mockup_new);
    test_ptr_success(stmt = db_stmt_new(db, "mockup"));
}

static const char *db_mockup_sql(db_stmt_ct stmt2, db_sql_id type)
{
    test_ptr_eq(stmt2, stmt);
    test_uint_eq(type, DB_SQL_PLAIN);

    return "sql\nsql";
}

TEST_CASE_FIX(db_sql, db_mockup_prepare, db_finalize)
{
    const char *sql;

    mockup.sql = db_mockup_sql;

    test_ptr_success(sql = db_sql(stmt));
    test_str_eq(sql, "sql\nsql");
}

TEST_CASE_ABORT(db_sql_e_invalid_stmt1)
{
    db_sql_e(NULL);
}

TEST_CASE_ABORT(db_sql_e_invalid_stmt2)
{
    db_sql_e((db_stmt_ct)&not_a_stmt);
}

TEST_CASE_FIX(db_sql_e_unsupported, db_stmt_new, db_finalize)
{
    test_ptr_error(db_sql_e(stmt), E_DB_UNSUPPORTED);
}

TEST_CASE_FIX(db_sql_e, db_mockup_prepare, db_finalize)
{
    const char *sql;

    mockup.sql = db_mockup_sql;

    test_ptr_success(sql = db_sql_e(stmt));
    test_str_eq(sql, "sql\\nsql");
}

TEST_CASE_ABORT(db_sql_expanded_invalid_stmt1)
{
    db_sql_expanded(NULL);
}

TEST_CASE_ABORT(db_sql_expanded_invalid_stmt2)
{
    db_sql_expanded((db_stmt_ct)&not_a_stmt);
}

TEST_CASE_FIX(db_sql_expanded_unsupported, db_stmt_new, db_finalize)
{
    test_ptr_error(db_sql_expanded(stmt), E_DB_UNSUPPORTED);
}

static const char *db_mockup_sql_expanded(db_stmt_ct stmt2, db_sql_id type)
{
    test_ptr_eq(stmt2, stmt);
    test_uint_eq(type, DB_SQL_EXPANDED);

    return "esql\nesql";
}

TEST_CASE_FIX(db_sql_expanded, db_mockup_prepare, db_finalize)
{
    const char *sql;

    mockup.sql = db_mockup_sql_expanded;

    test_ptr_success(sql = db_sql_expanded(stmt));
    test_str_eq(sql, "esql\nesql");
}

TEST_CASE_ABORT(db_sql_expanded_e_invalid_stmt1)
{
    db_sql_expanded_e(NULL);
}

TEST_CASE_ABORT(db_sql_expanded_e_invalid_stmt2)
{
    db_sql_expanded_e((db_stmt_ct)&not_a_stmt);
}

TEST_CASE_FIX(db_sql_expanded_e_unsupported, db_stmt_new, db_finalize)
{
    test_ptr_error(db_sql_expanded_e(stmt), E_DB_UNSUPPORTED);
}

TEST_CASE_FIX(db_sql_expanded_e, db_mockup_prepare, db_finalize)
{
    const char *sql;

    mockup.sql = db_mockup_sql_expanded;

    test_ptr_success(sql = db_sql_expanded_e(stmt));
    test_str_eq(sql, "esql\\nesql");
}

TEST_CASE_FIX_ABORT(db_exec_invalid_stmt1, db_new, db_close)
{
    db_exec(NULL);
}

TEST_CASE_FIX_ABORT(db_exec_invalid_stmt2, db_new, db_close)
{
    db_exec((db_stmt_ct)&not_a_stmt);
}

TEST_CASE_FIX(db_exec_unsupported, db_stmt_new, db_finalize)
{
    test_int_error(db_exec(stmt), E_DB_UNSUPPORTED);
}

static int db_mockup_exec(db_stmt_ct stmt2, db_record_cb record, void *ctx)
{
    test_ptr_eq(stmt2, stmt);
    test_true(!record);

    return 0;
}

TEST_CASE_FIX(db_exec, db_mockup_prepare, db_finalize)
{
    mockup.exec = db_mockup_exec;

    test_int_success(db_exec(stmt));
}

TEST_CASE_FIX_ABORT(db_exec_f_invalid_stmt1, db_new, db_close)
{
    db_exec_f(NULL, NULL, NULL);
}

TEST_CASE_FIX_ABORT(db_exec_f_invalid_stmt2, db_new, db_close)
{
    db_exec_f((db_stmt_ct)&not_a_stmt, NULL, NULL);
}

static int test_db_record(db_stmt_ct stmt, size_t row, void *ctx)
{
    return 0;
}

TEST_CASE_FIX(db_exec_f_unsupported, db_stmt_new, db_finalize)
{
    test_int_error(db_exec_f(stmt, test_db_record, "foo"), E_DB_UNSUPPORTED);
}

static int db_mockup_exec_f(db_stmt_ct stmt2, db_record_cb record, void *ctx)
{
    test_ptr_eq(stmt2, stmt);
    test_true(record == test_db_record);
    test_str_eq(ctx, "foo");

    return 0;
}

TEST_CASE_FIX(db_exec_f, db_mockup_prepare, db_finalize)
{
    mockup.exec = db_mockup_exec_f;

    test_int_success(db_exec_f(stmt, test_db_record, "foo"));
}

static int db_mockup_exec_finalize(db_stmt_ct stmt, db_record_cb record, void *ctx)
{
    test_int_error(db_finalize(stmt), E_DB_ILLEGAL);

    return 0;
}

TEST_CASE_FIX(db_finalize_executing_stmt, db_mockup_prepare, db_finalize)
{
    mockup.exec = db_mockup_exec_finalize;

    test_int_success(db_exec(stmt));
}

TEST_CASE_ABORT(db_event_invalid_db1)
{
    db_event(NULL, "sql");
}

TEST_CASE_ABORT(db_event_invalid_db2)
{
    db_event((db_ct)&not_a_db, "sql");
}

static void test_db_event(const char *sql, void *ctx)
{
    test_str_eq(sql, "sql");
    test_str_eq(ctx, "foo");
}

TEST_CASE_ABORT(db_trace_invalid_db1)
{
    db_trace(NULL, test_db_event, "foo");
}

TEST_CASE_ABORT(db_trace_invalid_db2)
{
    db_trace((db_ct)&not_a_db, test_db_event, "foo");
}

static int db_mockup_trace_enable(db_ct db2, bool enable)
{
    test_ptr_eq(db2, db);
    test_true(enable);

    return 0;
}

TEST_CASE_FIX(db_trace_enable, db_mockup_new, db_close)
{
    mockup.trace = db_mockup_trace_enable;

    test_int_success(db_trace(db, test_db_event, "foo"));
    test_trace(db_event(db, "sql"));
}

static int db_mockup_trace_disable(db_ct db2, bool enable)
{
    test_ptr_eq(db2, db);
    test_false(enable);

    return 0;
}

TEST_CASE_FIX(db_trace_disable, db_mockup_new, db_close)
{
    mockup.trace = db_mockup_trace_disable;

    test_int_success(db_trace(db, NULL, NULL));
}

int test_suite_db_interface(void *param)
{
    return error_pass_int(test_run_cases("interface",
        test_case(db_new_invalid_interface),
        test_case(db_close_invalid_db1),
        test_case(db_close_invalid_db2),
        test_case(db_new_close),

        test_case(db_get_ctx_invalid_db1),
        test_case(db_get_ctx_invalid_db2),
        test_case(db_get_ctx),

        test_case(db_stmt_new_invalid_db1),
        test_case(db_stmt_new_invalid_db2),
        test_case(db_finalize_invalid_stmt1),
        test_case(db_finalize_invalid_stmt2),
        test_case(db_stmt_new_finalize),

        test_case(db_close_open_stmt),

        test_case(db_stmt_get_ctx_invalid_stmt1),
        test_case(db_stmt_get_ctx_invalid_stmt2),
        test_case(db_stmt_get_ctx),

        test_case(db_stmt_get_db_invalid_stmt1),
        test_case(db_stmt_get_db_invalid_stmt2),
        test_case(db_stmt_get_db),

        test_case(db_prepare_invalid_db1),
        test_case(db_prepare_invalid_db2),
        test_case(db_prepare_invalid_sql),
        test_case(db_prepare_unsupported),
        test_case(db_prepare),
        test_case(db_prepare_f_invalid_db1),
        test_case(db_prepare_f_invalid_db2),
        test_case(db_prepare_f_invalid_sql),
        test_case(db_prepare_f_unsupported),
        test_case(db_prepare_f),

        test_case(db_sql_invalid_stmt1),
        test_case(db_sql_invalid_stmt2),
        test_case(db_sql_unsupported),
        test_case(db_sql),
        test_case(db_sql_e_invalid_stmt1),
        test_case(db_sql_e_invalid_stmt2),
        test_case(db_sql_e_unsupported),
        test_case(db_sql_e),
        test_case(db_sql_expanded_invalid_stmt1),
        test_case(db_sql_expanded_invalid_stmt2),
        test_case(db_sql_expanded_unsupported),
        test_case(db_sql_expanded),
        test_case(db_sql_expanded_e_invalid_stmt1),
        test_case(db_sql_expanded_e_invalid_stmt2),
        test_case(db_sql_expanded_e_unsupported),
        test_case(db_sql_expanded_e),

        test_case(db_exec_invalid_stmt1),
        test_case(db_exec_invalid_stmt2),
        test_case(db_exec_unsupported),
        test_case(db_exec),
        test_case(db_exec_f_invalid_stmt1),
        test_case(db_exec_f_invalid_stmt2),
        test_case(db_exec_f_unsupported),
        test_case(db_exec_f),

        test_case(db_finalize_executing_stmt),

        test_case(db_event_invalid_db1),
        test_case(db_event_invalid_db2),
        test_case(db_trace_invalid_db1),
        test_case(db_trace_invalid_db2),
        test_case(db_trace_enable),
        test_case(db_trace_disable),

        NULL
    ));
}
