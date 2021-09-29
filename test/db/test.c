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

#include "test.h"
#include <ytil/test/run.h>
#include <ytil/test/test.h>
#include <ytil/gen/str.h>

static test_param_db_st *param;
static db_ct db;
static db_stmt_ct stmt;


TEST_SETUP(db_open)
{
    test_ptr_success(db = param->config->open());
}

TEST_TEARDOWN(db_close)
{
    test_int_success(db_close(db));
}

TEST_CASE_FIX(db_prepare_unsupported, db_open, db_close)
{
    test_ptr_error(db_prepare(db, "select 123;"), E_DB_UNSUPPORTED);
}

TEST_CASE_FIX(db_prepare_malformed_query, db_open, db_close)
{
    test_ptr_error(db_prepare(db, "123"), E_DB_MALFORMED_SQL);
}

TEST_CASE_FIX(db_prepare_multi_stmt, db_open, db_close)
{
    test_ptr_error(db_prepare(db, "select 1; select 2;"), E_DB_MULTI_STMT);
}

TEST_CASE_FIX(db_prepare, db_open, db_close)
{
    test_ptr_success(stmt = db_prepare(db, "select 123;"));
    test_int_success(db_finalize(stmt));
}

int test_suite_db_prepare(void *vparam)
{
    param = vparam;

    if(!param->supported)
        return error_pass_int(test_run_case(
            test_case(db_prepare_unsupported)));

    return error_pass_int(test_run_cases(NULL,
        test_case(db_prepare_malformed_query),
        test_case(db_prepare_multi_stmt),
        test_case(db_prepare),
        NULL
    ));
}

TEST_SETUP(db_prepare1)
{
    test_setup(db_open);
    test_ptr_success(stmt = db_prepare(db, "select 123;"));
}

TEST_SETUP(db_prepare2)
{
    test_setup(db_open);
    test_ptr_success(stmt = db_prepare(db, "select 123, 456;"));
}

TEST_SETUP(db_prepare_multi)
{
    test_setup(db_open);
    test_ptr_success(stmt = db_prepare(db, "select 123 union select 456;"));
}

TEST_PSETUP(db_prepare_table, const char *sql)
{
    test_setup(db_open);

    test_ptr_success(stmt = db_prepare(db, "create temporary table tmp(i int primary key);"));
    test_int_success(db_exec(stmt));
    test_int_success(db_finalize(stmt));

    test_ptr_success(stmt = db_prepare(db, "insert into tmp(i) values(123);"));
    test_int_success(db_exec(stmt));
    test_int_success(db_finalize(stmt));

    test_ptr_success(stmt = db_prepare(db, sql));
}

TEST_TEARDOWN(db_finalize)
{
    test_int_success(db_finalize(stmt));
    test_teardown(db_close);
}

TEST_CASE_FIX(db_exec_unsupported, db_prepare1, db_finalize)
{
    test_int_error(db_exec(stmt), E_DB_UNSUPPORTED);
}

static int test_db_row_count(db_stmt_ct stmt, size_t row, void *ctx)
{
    int *count = ctx;

    *count += 1;

    return 0;
}

TEST_CASE_FIX(db_exec, db_prepare_multi, db_finalize)
{
    int count = 0;

    test_int_success(db_exec_f(stmt, test_db_row_count, &count));
    test_int_success(db_exec_f(stmt, test_db_row_count, &count));
    test_int_eq(count, 4);
}

TEST_CASE_PFIX(db_exec_constraint, db_prepare_table, db_finalize,
    "insert into tmp(i) values(123);")
{
    test_int_error(db_exec(stmt), E_DB_CONSTRAINT);
}

TEST_CASE_FIX(db_exec_result_unbound, db_prepare1, db_finalize)
{
    test_int_success(db_exec(stmt));
}

TEST_CASE_FIX(db_exec_result_unbound_partly, db_prepare2, db_finalize)
{
    int value;

    test_int_success(db_result_bind_int(stmt, 0, &value, NULL));
    test_int_success(db_exec(stmt));
}

TEST_CASE_FIX(db_exec_result_unused, db_prepare_multi, db_finalize)
{
    test_int_success(db_exec(stmt));
    test_int_success(db_exec(stmt));
}

int test_suite_db_exec(void *vparam)
{
    param = vparam;

    if(!param->supported)
        return error_pass_int(test_run_case(
            test_case(db_exec_unsupported)));

    return error_pass_int(test_run_cases(NULL,
        test_case(db_exec),
        test_case(db_exec_constraint),
        test_case(db_exec_result_unbound),
        test_case(db_exec_result_unbound_partly),
        test_case(db_exec_result_unused),
        NULL
    ));
}

TEST_SETUP(db_prepare_e)
{
    test_setup(db_open);
    test_ptr_success(stmt = db_prepare(db, "select 'foo\nbar';"));
}

TEST_CASE_FIX(db_sql_plain_unsupported, db_prepare_e, db_finalize)
{
    test_ptr_error(db_sql(stmt), E_DB_UNSUPPORTED);
}

TEST_CASE_FIX(db_sql_plain, db_prepare_e, db_finalize)
{
    const char *sql;

    test_ptr_success(sql = db_sql(stmt));
    test_str_eq(sql, "select 'foo\nbar';");
}

int test_suite_db_sql_plain(void *vparam)
{
    param = vparam;

    if(!param->supported)
        return error_pass_int(test_run_case(
            test_case(db_sql_plain_unsupported)));

    return error_pass_int(test_run_cases(NULL,
        test_case(db_sql_plain),
        NULL
    ));
}

TEST_SETUP(db_prepare_p)
{
    test_setup(db_open);
    test_ptr_success(stmt = db_prepare(db, "select ?;"));
}

TEST_CASE_FIX(db_sql_expanded_unsupported, db_prepare_p, db_finalize)
{
    test_ptr_error(db_sql_expanded(stmt), E_DB_UNSUPPORTED);
}

TEST_CASE_FIX(db_sql_expanded1, db_prepare_p, db_finalize)
{
    const char *sql;

    test_int_success(db_param_bind_text(stmt, 0, "foo\nbar", -1));
    test_ptr_success(sql = db_sql_expanded(stmt));
    test_str_eq(sql, "select 'foo\nbar';");
}

TEST_CASE_FIX(db_sql_expanded2, db_prepare_p, db_finalize)
{
    const char *sql;

    test_int_success(db_param_bind_text(stmt, 0, "foo\nbar", -1));
    test_ptr_success(sql = db_sql_expanded(stmt));
    test_str_eq(sql, "select 'foo\nbar';");

    test_int_success(db_param_bind_text(stmt, 0, "baz\rboz", -1));
    test_ptr_success(sql = db_sql_expanded(stmt));
    test_str_eq(sql, "select 'baz\rboz';");
}

int test_suite_db_sql_expanded(void *vparam)
{
    param = vparam;

    if(!param->supported)
        return error_pass_int(test_run_case(
            test_case(db_sql_expanded_unsupported)));

    return error_pass_int(test_run_cases(NULL,
        test_case(db_sql_expanded1),
        test_case(db_sql_expanded2),
        NULL
    ));
}

int test_suite_db_sql(void *vparam)
{
    return error_pass_int(test_run_suites(NULL,
        test_suite_p(db_sql_plain, vparam),
        test_suite_p(db_sql_expanded, vparam),
        NULL
    ));
}

static void test_db_event(const char *sql, void *ctx)
{
    str_append_c(ctx, sql);
}

TEST_CASE_FIX(db_trace_unsupported, db_open, db_close)
{
    test_int_error(db_trace(db, test_db_event, NULL), E_DB_UNSUPPORTED);
}

TEST_CASE_FIX(db_trace, db_prepare1, db_finalize)
{
    str_ct str;

    test_ptr_success(str = str_prepare(0));
    test_int_success(db_trace(db, test_db_event, str));

    test_int_success(db_exec(stmt));
    test_int_success(db_exec(stmt));
    test_int_success(db_exec(stmt));

    test_str_eq(str_c(str), "select 123;select 123;select 123;");
    test_void(str_unref(str));
}

int test_suite_db_trace(void *vparam)
{
    param = vparam;

    if(!param->supported)
        return error_pass_int(test_run_case(
            test_case(db_trace_unsupported)));

    return error_pass_int(test_run_cases(NULL,
        test_case(db_trace),
        NULL
    ));
}

int test_suite_db_type_bool(void *vparam) {
    return error_pass_int(test_run_suites(NULL,
        test_suite_p(db_param_bind_bool, vparam),
        test_suite_p(db_result_bind_bool, vparam),
        NULL
    ));
}

int test_suite_db_type_sint8(void *vparam)
{
    return error_pass_int(test_run_suites(NULL,
        test_suite_p(db_param_bind_sint8, vparam),
        test_suite_p(db_result_bind_sint8, vparam),
        NULL
    ));
}

int test_suite_db_type_uint8(void *vparam)
{
    return error_pass_int(test_run_suites(NULL,
        test_suite_p(db_param_bind_uint8, vparam),
        test_suite_p(db_result_bind_uint8, vparam),
        NULL
    ));
}

int test_suite_db_type_int8(void *vparam)
{
    return error_pass_int(test_run_suites(NULL,
        test_suite_p(db_param_bind_int8, vparam),
        test_suite_p(db_result_bind_int8, vparam),
        NULL
    ));
}

int test_suite_db_type_sint16(void *vparam)
{
    return error_pass_int(test_run_suites(NULL,
        test_suite_p(db_param_bind_sint16, vparam),
        test_suite_p(db_result_bind_sint16, vparam),
        NULL
    ));
}

int test_suite_db_type_uint16(void *vparam)
{
    return error_pass_int(test_run_suites(NULL,
        test_suite_p(db_param_bind_uint16, vparam),
        test_suite_p(db_result_bind_uint16, vparam),
        NULL
    ));
}

int test_suite_db_type_int16(void *vparam)
{
    return error_pass_int(test_run_suites(NULL,
        test_suite_p(db_param_bind_int16, vparam),
        test_suite_p(db_result_bind_int16, vparam),
        NULL
    ));
}

int test_suite_db_type_sint32(void *vparam)
{
    return error_pass_int(test_run_suites(NULL,
        test_suite_p(db_param_bind_sint32, vparam),
        test_suite_p(db_result_bind_sint32, vparam),
        NULL
    ));
}

int test_suite_db_type_uint32(void *vparam)
{
    return error_pass_int(test_run_suites(NULL,
        test_suite_p(db_param_bind_uint32, vparam),
        test_suite_p(db_result_bind_uint32, vparam),
        NULL
    ));
}

int test_suite_db_type_int32(void *vparam)
{
    return error_pass_int(test_run_suites(NULL,
        test_suite_p(db_param_bind_int32, vparam),
        test_suite_p(db_result_bind_int32, vparam),
        NULL
    ));
}

int test_suite_db_type_sint64(void *vparam)
{
    return error_pass_int(test_run_suites(NULL,
        test_suite_p(db_param_bind_sint64, vparam),
        test_suite_p(db_result_bind_sint64, vparam),
        NULL
    ));
}

int test_suite_db_type_uint64(void *vparam)
{
    return error_pass_int(test_run_suites(NULL,
        test_suite_p(db_param_bind_uint64, vparam),
        test_suite_p(db_result_bind_uint64, vparam),
        NULL
    ));
}

int test_suite_db_type_int64(void *vparam)
{
    return error_pass_int(test_run_suites(NULL,
        test_suite_p(db_param_bind_int64, vparam),
        test_suite_p(db_result_bind_int64, vparam),
        NULL
    ));
}

int test_suite_db_type_float(void *vparam)
{
    return error_pass_int(test_run_suites(NULL,
        test_suite_p(db_param_bind_float, vparam),
        test_suite_p(db_result_bind_float, vparam),
        NULL
    ));
}

int test_suite_db_type_double(void *vparam)
{
    return error_pass_int(test_run_suites(NULL,
        test_suite_p(db_param_bind_double, vparam),
        test_suite_p(db_result_bind_double, vparam),
        NULL
    ));
}

int test_suite_db_type_ldouble(void *vparam)
{
    return error_pass_int(test_run_suites(NULL,
        test_suite_p(db_param_bind_ldouble, vparam),
        test_suite_p(db_result_bind_ldouble, vparam),
        NULL
    ));
}

int test_suite_db_type_text(void *vparam)
{
    return error_pass_int(test_run_suites(NULL,
        test_suite_p(db_param_bind_text, vparam),
        test_suite_p(db_result_bind_text, vparam),
        NULL
    ));
}

int test_suite_db_type_blob(void *vparam)
{
    return error_pass_int(test_run_suites(NULL,
        test_suite_p(db_param_bind_blob, vparam),
        test_suite_p(db_result_bind_blob, vparam),
        NULL
    ));
}

int test_suite_db_type_date(void *vparam)
{
    return error_pass_int(test_run_suites(NULL,
        test_suite_p(db_param_bind_date, vparam),
        test_suite_p(db_result_bind_date, vparam),
        NULL
    ));
}

int test_suite_db_type_time(void *vparam)
{
    return error_pass_int(test_run_suites(NULL,
        test_suite_p(db_param_bind_time, vparam),
        test_suite_p(db_result_bind_time, vparam),
        NULL
    ));
}

int test_suite_db_type_datetime(void *vparam)
{
    return error_pass_int(test_run_suites(NULL,
        test_suite_p(db_param_bind_datetime, vparam),
        test_suite_p(db_result_bind_datetime, vparam),
        NULL
    ));
}

int test_suite_db_type_timestamp(void *vparam)
{
    return error_pass_int(test_run_suites(NULL,
        test_suite_p(db_param_bind_timestamp, vparam),
        test_suite_p(db_result_bind_timestamp, vparam),
        NULL
    ));
}
