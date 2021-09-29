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
#include <ytil/sys/path.h>
#include <ytil/db/sqlite.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include "test.h"


TEST_CASE_ABORT(db_sqlite_open_invalid_file)
{
    db_sqlite_open(NULL);
}

TEST_CASE(db_sqlite_open)
{
    path_ct path;
    str_ct str;
    db_ct db;
    struct stat st;

    test_ptr_success(path = path_get_base_dir(PATH_BASE_DIR_TMP));
    test_ptr_success(path_append_c(path, "ytil_test.sqlite", PATH_STYLE_POSIX));
    test_ptr_success(str = path_get(path, PATH_STYLE_NATIVE));

    test_int_maybe_errno(unlink(str_c(str)), ENOENT);
    test_ptr_success(db = db_sqlite_open(path));

    test_int_success_errno(stat(str_c(str), &st));

    test_void(db_close(db));
    db = NULL;
    test_int_success_errno(unlink(str_c(str)));

    test_void(str_unref(str));
    test_void(path_free(path));
}

TEST_CASE(db_sqlite_open_memory)
{
    db_ct db;

    test_ptr_success(db = db_sqlite_open_memory());
    test_void(db_close(db));
}

static int test_suite_db_sqlite_open(void *ctx)
{
    return error_pass_int(test_run_cases(NULL,
        test_case(db_sqlite_open_invalid_file),
        test_case(db_sqlite_open),
        test_case(db_sqlite_open_memory),
        NULL
    ));
}

int test_suite_db_sqlite(void *param)
{
    test_config_db_st config =
    {
        .open       = db_sqlite_open_memory,
        .db         = "temp",
        .dbl_dig    = DBL_DECIMAL_DIG - 2, // always 14 ?
        .nan        = "'NaN'",
        .inf        = "Inf",
    };

    return error_pass_int(test_run_suites("sqlite",
        test_suite(db_sqlite_open),

        test_suite_db_supported(prepare, config),
        test_suite_db_supported(exec, config),
        test_suite_db_supported(sql, config),
        test_suite_db_supported(trace, config),

        test_suite_db_supported(param_count, config),
        test_suite_db_supported(result_count, config),

        test_suite_db_supported(param_bind_null, config),

        test_suite_db_unsupported(type_bool, config),

        test_suite_db(type_sint8, sizeof(int8_t) == sizeof(int), config),
        test_suite_db_unsupported(type_uint8, config),

        test_suite_db(type_sint16, sizeof(int16_t) == sizeof(int), config),
        test_suite_db_unsupported(type_uint16, config),

        test_suite_db(type_sint32, sizeof(int32_t) == sizeof(int), config),
        test_suite_db_unsupported(type_uint32, config),

        test_suite_db_supported(type_sint64, config),
        test_suite_db_unsupported(type_uint64, config),

        test_suite_db_unsupported(type_float, config),
        test_suite_db_supported(type_double, config),
        test_suite_db_unsupported(type_ldouble, config),

        test_suite_db_supported(type_text, config),
        test_suite_db_supported(type_blob, config),

        test_suite_db_unsupported(type_date, config),
        test_suite_db_unsupported(type_time, config),
        test_suite_db_unsupported(type_datetime, config),
        test_suite_db_unsupported(type_timestamp, config),

        test_suite_db_unsupported(result_get_type, config),

        test_suite_db_supported(result_get_database_name, config),
        test_suite_db_unsupported(result_get_table_name, config),
        test_suite_db_supported(result_get_original_table_name, config),
        test_suite_db_supported(result_get_field_name, config),
        test_suite_db_supported(result_get_original_field_name, config),

        NULL
    ));
}
