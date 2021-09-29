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
#include <ytil/db/mysql.h>
#include <ytil/sys/env.h>
#include <ytil/ext/stdlib.h>
#include "test.h"

static const char *server, *user, *pass, *testdb;
static int port;


TEST_CASE(db_mysql_connect_unknown_host)
{
    test_ptr_error(db_mysql_connect("foo", 123, NULL, NULL, 1, NULL), E_DB_UNKNOWN_HOST);
}

TEST_CASE(db_mysql_connect_unreachable)
{
    test_ptr_error(db_mysql_connect("127.0.0.1", 1, NULL, NULL, 1, NULL), E_DB_CONNECTION);
}

TEST_CASE(db_mysql_connect_login_failed)
{
    test_ptr_error(db_mysql_connect(server, port, "foo", "bar", 1, NULL), E_DB_LOGIN_FAILED);
}

TEST_CASE(db_mysql_connect_unknown_database)
{
    test_ptr_error(db_mysql_connect(server, port, user, pass, 1, "foo"), E_DB_UNKNOWN_DATABASE);
}

TEST_CASE(db_mysql_connect)
{
    db_ct db;

    test_ptr_success(db = db_mysql_connect(server, port, user, pass, 1, testdb));
    test_int_success(db_close(db));
}

static int test_suite_db_mysql_connect(void *ctx)
{
    return error_pass_int(test_run_cases(NULL,
        test_case(db_mysql_connect_unknown_host),
        test_case(db_mysql_connect_unreachable),
        test_case(db_mysql_connect_login_failed),
        test_case(db_mysql_connect_unknown_database),
        test_case(db_mysql_connect),

        NULL
    ));
}

static void test_db_mysql_load_env(void)
{
    str_const_ct eserver    = env_get(LIT("MYSQL_SERVER"));
    str_const_ct eport      = env_get(LIT("MYSQL_PORT"));
    str_const_ct euser      = env_get(LIT("MYSQL_USER"));
    str_const_ct epass      = env_get(LIT("MYSQL_PASS"));
    str_const_ct edb        = env_get(LIT("MYSQL_DB"));
    long lport;

    server  = eserver ? str_c(eserver) : NULL;
    port    = eport ? (str2l(&lport, str_c(eport), 10) > 0 ? lport : 0) : 0;
    user    = euser ? str_c(euser) : NULL;
    pass    = epass ? str_c(epass) : NULL;
    testdb  = edb ? str_c(edb) : "ytil_test";
}

static db_ct test_db_mysql_connect(void)
{
    return error_pass_ptr(db_mysql_connect(server, port, user, pass, 1, testdb));
}

static const char *test_suite_db_mysql_check(void)
{
    db_ct db;

    if((db = test_db_mysql_connect()))
        return db_close(db), NULL;

    switch(error_code(0))
    {
    case E_DB_CONNECTION:
        return "Unabled to connect to MySQL server.";

    case E_DB_INCOMPATIBLE:
        return "MySQL Connector is not compatible with MySQL server.";

    case E_DB_LOGIN_FAILED:
        return "Unabled to login with user to MySQL server.";

    case E_DB_UNKNOWN_DATABASE:
        return "Test DB does not exist on MySQL server.";

    case E_DB_EXTENDED:
        return error_desc(1);

    default:
        return error_desc(0);
    }
}

int test_suite_db_mysql(void *param)
{
    test_config_db_st config =
    {
        .open       = test_db_mysql_connect,
        .flt_dig    = FLT_DECIMAL_DIG,
        .dbl_dig    = DBL_DECIMAL_DIG,
        .nan        = "nan",
        .inf        = "inf",
    };
    int rc;

    test_db_mysql_load_env();
    config.db = testdb;

    rc = error_pass_int(test_run_suites_check("mysql",
        test_suite_db_mysql_check,

        test_suite(db_mysql_connect),

        test_suite_db_supported(prepare, config),
        test_suite_db_supported(exec, config),
        test_suite_db_supported(sql, config),
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
        test_suite_db_supported(result_get_original_field_name, config),

        NULL
    ));

    env_free();

    return rc;
}
