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
#include "test.h"


TEST_CASE(db_mysql_connect_unknown_host)
{
    test_ptr_error(db_mysql_connect("foo", 123, NULL, NULL, NULL), E_DB_UNKNOWN_HOST);
}

TEST_CASE(db_mysql_connect_unreachable)
{
    test_ptr_error(db_mysql_connect("127.0.0.1", 1, NULL, NULL, NULL), E_DB_CONNECTION);
}

TEST_CASE(db_mysql_connect_access_denied)
{
    test_ptr_error(db_mysql_connect(NULL, 0, "foo", "bar", NULL), E_DB_ACCESS_DENIED);
}

TEST_CASE(db_mysql_connect_unknown_database)
{
    test_ptr_error(db_mysql_connect(NULL, 0, NULL, NULL, "foo"), E_DB_UNKNOWN_DATABASE);
}

TEST_CASE(db_mysql_connect)
{
    db_ct db;

    test_ptr_success(db = db_mysql_connect(NULL, 0, NULL, NULL, NULL));
    test_int_success(db_close(db));
}

TEST_CASE(db_mysql_connect_local_access_denied)
{
    test_ptr_error(db_mysql_connect_local("foo", "bar", NULL), E_DB_ACCESS_DENIED);
}

TEST_CASE(db_mysql_connect_local_unknown_database)
{
    test_ptr_error(db_mysql_connect_local(NULL, NULL, "foo"), E_DB_UNKNOWN_DATABASE);
}

TEST_CASE(db_mysql_connect_local)
{
    db_ct db;

    test_ptr_success(db = db_mysql_connect_local(NULL, NULL, NULL));
    test_int_success(db_close(db));
}

TEST_CASE(db_mysql_connect_unix_unreachable)
{
    test_ptr_error(db_mysql_connect_unix("foo", NULL, NULL, NULL), E_DB_CONNECTION);
}

TEST_CASE(db_mysql_connect_unix_access_denied)
{
    test_ptr_error(db_mysql_connect_unix(NULL, "foo", "bar", NULL), E_DB_ACCESS_DENIED);
}

TEST_CASE(db_mysql_connect_unix_unknown_database)
{
    test_ptr_error(db_mysql_connect_unix(NULL, NULL, NULL, "foo"), E_DB_UNKNOWN_DATABASE);
}

TEST_CASE(db_mysql_connect_unix)
{
    db_ct db;

    test_ptr_success(db = db_mysql_connect_unix(NULL, NULL, NULL, NULL));
    test_int_success(db_close(db));
}

static int test_suite_db_mysql_connect(void *ctx)
{
    return error_pass_int(test_run_cases(NULL,
        test_case(db_mysql_connect_unknown_host),
        test_case(db_mysql_connect_unreachable),
        test_case(db_mysql_connect_access_denied),
        test_case(db_mysql_connect_unknown_database),
        test_case(db_mysql_connect),

        test_case(db_mysql_connect_local_access_denied),
        test_case(db_mysql_connect_local_unknown_database),
        test_case(db_mysql_connect_local),

        test_case(db_mysql_connect_unix_unreachable),
        test_case(db_mysql_connect_unix_access_denied),
        test_case(db_mysql_connect_unix_unknown_database),
        test_case(db_mysql_connect_unix),

        NULL
    ));
}

static db_ct test_db_mysql_connect(void)
{
    return error_pass_ptr(db_mysql_connect_local(NULL, NULL, "ytil_test"));
}

int test_suite_db_mysql(void *param)
{
    return error_pass_int(test_run_suites("mysql",
        test_suite(db_mysql_connect),

        test_suite_db_supported(prepare, test_db_mysql_connect),
        test_suite_db_supported(exec, test_db_mysql_connect),
        //test_suite_db_supported(sql, test_db_mysql_connect),
        //test_suite_db_supported(trace, test_db_mysql_connect),

        test_suite_db_supported(param_count, test_db_mysql_connect),
        test_suite_db_supported(result_count, test_db_mysql_connect),

        test_suite_db_supported(param_bind_null, test_db_mysql_connect),

        /*test_suite_db_unsupported(type_bool, test_db_mysql_connect),

        test_suite_db(type_sint8, sizeof(int8_t) == sizeof(int), test_db_mysql_connect),
        test_suite_db_unsupported(type_uint8, test_db_mysql_connect),

        test_suite_db(type_sint16, sizeof(int16_t) == sizeof(int), test_db_mysql_connect),
        test_suite_db_unsupported(type_uint16, test_db_mysql_connect),

        test_suite_db(type_sint32, sizeof(int32_t) == sizeof(int), test_db_mysql_connect),
        test_suite_db_unsupported(type_uint32, test_db_mysql_connect),

        test_suite_db_supported(type_sint64, test_db_mysql_connect),
        test_suite_db_unsupported(type_uint64, test_db_mysql_connect),

        test_suite_db_unsupported(type_float, test_db_mysql_connect),
        test_suite_db_supported(type_double, test_db_mysql_connect),

        test_suite_db_supported(type_text, test_db_mysql_connect),
        test_suite_db_supported(type_blob, test_db_mysql_connect),

        test_suite_db_unsupported(type_date, test_db_mysql_connect),
        test_suite_db_unsupported(type_time, test_db_mysql_connect),
        test_suite_db_unsupported(type_datetime, test_db_mysql_connect),
        test_suite_db_unsupported(type_timestamp, test_db_mysql_connect),

        test_suite_db_supported(result_fetch, test_db_mysql_connect),

        test_suite_db_unsupported(result_get_type, test_db_mysql_connect),

        test_suite_db_supported(result_get_database_name, test_db_mysql_connect),
        test_suite_db_unsupported(result_get_table_name, test_db_mysql_connect),
        test_suite_db_supported(result_get_original_table_name, test_db_mysql_connect),
        test_suite_db_supported(result_get_field_name, test_db_mysql_connect),
        test_suite_db_supported(result_get_original_field_name, test_db_mysql_connect),*/

        NULL
    ));
}
