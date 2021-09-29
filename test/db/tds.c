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
#include <ytil/db/tds.h>
#include <ytil/sys/env.h>
#include "test.h"

static const char *server, *user, *pass, *testdb;


TEST_CASE(db_tds_connect_unknown_host)
{
    test_ptr_error(db_tds_connect("ytil", "foo", NULL, NULL, 1, NULL), E_DB_UNKNOWN_HOST);
}

TEST_CASE(db_tds_connect_unreachable)
{
    test_ptr_error(db_tds_connect("ytil", "127.0.0.1:1", NULL, NULL, 1, NULL), E_DB_CONNECTION);
}

TEST_CASE(db_tds_connect_login_failed)
{
    test_ptr_error(db_tds_connect("ytil", server, "foo", "bar", 1, NULL), E_DB_LOGIN_FAILED);
}

TEST_CASE(db_tds_connect_unknown_database)
{
    test_ptr_error(db_tds_connect("ytil", server, user, pass, 1, "foo"), E_DB_UNKNOWN_DATABASE);
}

TEST_CASE(db_tds_connect_access_denied)
{
    test_ptr_error(db_tds_connect("ytil", server, user, pass, 1, "model"), E_DB_ACCESS_DENIED);
}

TEST_CASE(db_tds_connect)
{
    db_ct db;

    test_ptr_success(db = db_tds_connect("ytil", server, user, pass, 1, testdb));
    test_int_success(db_close(db));
}

#include <stdio.h>
static int foo(db_stmt_ct stmt, size_t row, void *ctx)
{
    printf("row %zu\n", row);
    return 0;
}

TEST_CASE(foo)
{
    db_ct db;
    db_stmt_ct stmt;
    int i = 0;
    bool null = true;

    test_ptr_success(db = db_tds_connect("ytil", server, user, pass, 1, testdb));

    test_ptr_success(stmt = db_prepare(db, "select 123 union select null;"));
    test_int_success(db_result_bind_int(stmt, 0, &i, &null));

    test_int_success(db_exec(stmt));
    printf("i = %i, null = %s\n", i, null ? "true" : "false");

    test_int_success(db_exec_f(stmt, foo, NULL));
    printf("i = %i, null = %s\n", i, null ? "true" : "false");

    test_int_success(db_finalize(stmt));
    test_int_success(db_close(db));
}

static int test_suite_db_tds_connect(void *ctx)
{
    return error_pass_int(test_run_cases(NULL,
        test_case(db_tds_connect_unknown_host),
        test_case(db_tds_connect_unreachable),
        test_case(db_tds_connect_login_failed),
        test_case(db_tds_connect_unknown_database),
        test_case(db_tds_connect_access_denied),
        test_case(db_tds_connect),
        test_case(foo),

        NULL
    ));
}

static db_ct test_db_tds_connect(void)
{
    return error_pass_ptr(db_tds_connect("ytil", server, user, pass, 3, testdb));
}

static void test_db_tds_load_env(void)
{
    str_const_ct eserver    = env_get(LIT("TDS_SERVER"));
    str_const_ct euser      = env_get(LIT("TDS_USER"));
    str_const_ct epass      = env_get(LIT("TDS_PASS"));
    str_const_ct edb        = env_get(LIT("TDS_DB"));

    server  = eserver ? str_c(eserver) : NULL;
    user    = euser ? str_c(euser) : NULL;
    pass    = epass ? str_c(epass) : NULL;
    testdb  = edb ? str_c(edb) : "ytil_test";
}

static const char *test_suite_db_tds_check(void)
{
    db_ct db;

    if((db = test_db_tds_connect()))
        return db_close(db), NULL;

    switch(error_code(0))
    {
    case E_DB_CONNECTION:
        return "Unable to connect to TDS server.";

    case E_DB_INCOMPATIBLE:
        return "TDS version is not compatible with TDS server.";

    case E_DB_LOGIN_FAILED:
        return "Unable to login with user to TDS server.";

    case E_DB_UNKNOWN_DATABASE:
        return "Test DB does not exist on TDS server.";

    case E_DB_ACCESS_DENIED:
        return "User has no access rights to test DB.";

    case E_DB_EXTENDED:
        return error_desc(1);

    default:
        return error_desc(0);
    }
}

int test_suite_db_tds(void *param)
{
    test_config_db_st config =
    {
        .open       = test_db_tds_connect,
        .flt_dig    = FLT_DECIMAL_DIG,
        .dbl_dig    = DBL_DECIMAL_DIG,
        .nan        = "nan",
        .inf        = "inf",
    };
    int rc;

    test_db_tds_load_env();
    config.db = testdb;

    // debug
    (void)(test_suite_db_tds_connect);
    (void)(config);

    rc = error_pass_int(test_run_suites_check("tds",
        test_suite_db_tds_check,

        test_suite(db_tds_connect),

        test_suite_db_supported(prepare, config),
        test_suite_db_supported(exec, config),
        /*test_suite_db_supported(sql, config),
        test_suite_db_unsupported(trace, config),

        test_suite_db_supported(param_count, config),
        test_suite_db_supported(result_count, config),

        test_suite_db_supported(param_bind_null, config),

        test_suite_db_unsupported(type_bool, config),

        test_suite_db_supported(type_int8, config),
        test_suite_db_supported(type_int16, config),
        test_suite_db_supported(type_int32, config),
        test_suite_db_supported(type_int64, config),

        test_suite_db_supported(type_float, config),
        test_suite_db_supported(type_double, config),

        test_suite_db_supported(type_text, config),
        test_suite_db_supported(type_blob, config),

        test_suite_db_supported(type_date, config),
        test_suite_db_supported(type_time, config),
        test_suite_db_supported(type_datetime, config),
        test_suite_db_supported(type_timestamp, config),

        test_suite_db_supported(result_get_type, config),

        test_suite_db_supported(result_get_database_name, config),
        test_suite_db_supported(result_get_table_name, config),
        test_suite_db_supported(result_get_original_table_name, config),
        test_suite_db_supported(result_get_field_name, config),
        test_suite_db_supported(result_get_original_field_name, config),*/

        NULL
    ));

    env_free();

    return rc;
}
