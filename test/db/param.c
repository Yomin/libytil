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
#include <ytil/ext/stdio.h>
#include <inttypes.h>

static test_param_db_st *param;
static db_ct db;
static db_stmt_ct stmt;


#define test_sql(sql) do { \
    const char *sql_expanded; \
    test_ptr_success(sql_expanded = db_sql_expanded(stmt)); \
    test_str_eq(sql_expanded, sql); \
} while(0)

#define test_fsql(fsql, ...) do { \
    const char *sql = FMT(fsql, __VA_ARGS__), *sql_expanded; \
    test_ptr_success(sql_expanded = db_sql_expanded(stmt)); \
    test_str_eq(sql_expanded, sql); \
} while(0)


TEST_SETUP(db_prepare)
{
    test_ptr_success(db = param->config->open());
    test_ptr_success(stmt = db_prepare(db, "select ?;"));
}

TEST_TEARDOWN(db_finalize)
{
    test_int_success(db_finalize(stmt));
    test_int_success(db_close(db));
}

TEST_CASE_FIX(db_param_count_unsupported, db_prepare, db_finalize)
{
    test_rc_error(db_param_count(stmt), -1, E_DB_UNSUPPORTED);
}

TEST_CASE_FIX(db_param_count, db_prepare, db_finalize)
{
    test_rc_success(db_param_count(stmt), 1, -1);
}

int test_suite_db_param_count(void *vparam)
{
    param = vparam;

    if(!param->supported)
        return error_pass_int(test_run_case(
            test_case(db_param_count_unsupported)));

    return error_pass_int(test_run_cases(NULL,
        test_case(db_param_count),
        NULL
    ));
}

TEST_CASE_FIX(db_param_bind_null_unsupported, db_prepare, db_finalize)
{
    test_int_error(db_param_bind_null(stmt, 0), E_DB_UNSUPPORTED);
}

TEST_CASE_FIX(db_param_bind_null_oob, db_prepare, db_finalize)
{
    test_int_error(db_param_bind_null(stmt, 1), E_DB_OUT_OF_BOUNDS);
}

TEST_CASE_FIX(db_param_bind_null, db_prepare, db_finalize)
{
    test_int_success(db_param_bind_null(stmt, 0));
    test_sql("select NULL;");
}

int test_suite_db_param_bind_null(void *vparam)
{
    param = vparam;

    if(!param->supported)
        return error_pass_int(test_run_case(
            test_case(db_param_bind_null_unsupported)));

    return error_pass_int(test_run_cases(NULL,
        test_case(db_param_bind_null_oob),
        test_case(db_param_bind_null),
        NULL
    ));
}

TEST_CASE_FIX(db_param_bind_bool_tmp_unsupported, db_prepare, db_finalize)
{
    test_int_error(db_param_bind_bool(stmt, 0, false), E_DB_UNSUPPORTED_TYPE);
}

TEST_CASE_FIX(db_param_bind_bool_tmp_oob, db_prepare, db_finalize)
{
    test_int_error(db_param_bind_bool(stmt, 1, false), E_DB_OUT_OF_BOUNDS);
}

TEST_CASE_FIX(db_param_bind_bool_tmp_true, db_prepare, db_finalize)
{
    test_int_success(db_param_bind_bool(stmt, 0, true));
    test_sql("select 1;");
}

TEST_CASE_FIX(db_param_bind_bool_tmp_false, db_prepare, db_finalize)
{
    test_int_success(db_param_bind_bool(stmt, 0, false));
    test_sql("select 0;");
}

int test_suite_db_param_bind_bool_tmp(void *vparam)
{
    param = vparam;

    if(!param->supported)
        return error_pass_int(test_run_case(
            test_case(db_param_bind_bool_tmp_unsupported)));

    return error_pass_int(test_run_cases(NULL,
        test_case(db_param_bind_bool_tmp_oob),
        test_case(db_param_bind_bool_tmp_true),
        test_case(db_param_bind_bool_tmp_false),
        NULL
    ));
}

TEST_CASE_FIX(db_param_bind_bool_ref_unsupported, db_prepare, db_finalize)
{
    bool value = false;

    test_int_error(db_param_bind_bool_ref(stmt, 0, &value, NULL), E_DB_UNSUPPORTED_TYPE);
}

TEST_CASE_FIX(db_param_bind_bool_ref_oob, db_prepare, db_finalize)
{
    bool value = false;

    test_int_error(db_param_bind_bool_ref(stmt, 1, &value, NULL), E_DB_OUT_OF_BOUNDS);
}

TEST_CASE_FIX(db_param_bind_bool_ref, db_prepare, db_finalize)
{
    bool value = false;

    test_int_success(db_param_bind_bool_ref(stmt, 0, &value, NULL));

    value = true;
    test_sql("select 1;");

    value = false;
    test_sql("select 0;");
}

TEST_CASE_FIX(db_param_bind_bool_ref_null, db_prepare, db_finalize)
{
    bool value  = false;
    bool null   = false;

    test_int_success(db_param_bind_bool_ref(stmt, 0, &value, &null));

    null = true;
    test_sql("select NULL;");

    null = false;
    test_sql("select 0;");
}

int test_suite_db_param_bind_bool_ref(void *vparam)
{
    param = vparam;

    if(!param->supported)
        return error_pass_int(test_run_case(
            test_case(db_param_bind_bool_ref_unsupported)));

    return error_pass_int(test_run_cases(NULL,
        test_case(db_param_bind_bool_ref_oob),
        test_case(db_param_bind_bool_ref),
        test_case(db_param_bind_bool_ref_null),
        NULL
    ));
}

int test_suite_db_param_bind_bool(void *vparam)
{
    return error_pass_int(test_run_suites(NULL,
        test_suite_p(db_param_bind_bool_tmp, vparam),
        test_suite_p(db_param_bind_bool_ref, vparam),
        NULL
    ));
}

TEST_CASE_FIX(db_param_bind_sint8_tmp_unsupported, db_prepare, db_finalize)
{
    test_int_error(db_param_bind_int8(stmt, 0, 123), E_DB_UNSUPPORTED_TYPE);
}

TEST_CASE_FIX(db_param_bind_sint8_tmp_oob, db_prepare, db_finalize)
{
    test_int_error(db_param_bind_int8(stmt, 1, 123), E_DB_OUT_OF_BOUNDS);
}

TEST_CASE_FIX(db_param_bind_sint8_tmp_min, db_prepare, db_finalize)
{
    test_int_success(db_param_bind_int8(stmt, 0, INT8_MIN));
    test_fsql("select %" PRId8 ";", INT8_MIN);
}

TEST_CASE_FIX(db_param_bind_sint8_tmp_max, db_prepare, db_finalize)
{
    test_int_success(db_param_bind_int8(stmt, 0, INT8_MAX));
    test_fsql("select %" PRId8 ";", INT8_MAX);
}

int test_suite_db_param_bind_sint8_tmp(void *vparam)
{
    param = vparam;

    if(!param->supported)
        return error_pass_int(test_run_case(
            test_case(db_param_bind_sint8_tmp_unsupported)));

    return error_pass_int(test_run_cases(NULL,
        test_case(db_param_bind_sint8_tmp_oob),
        test_case(db_param_bind_sint8_tmp_min),
        test_case(db_param_bind_sint8_tmp_max),
        NULL
    ));
}

TEST_CASE_FIX(db_param_bind_sint8_ref_unsupported, db_prepare, db_finalize)
{
    int8_t value = 123;

    test_int_error(db_param_bind_int8_ref(stmt, 0, &value, NULL), E_DB_UNSUPPORTED_TYPE);
}

TEST_CASE_FIX(db_param_bind_sint8_ref_oob, db_prepare, db_finalize)
{
    int8_t value = 123;

    test_int_error(db_param_bind_int8_ref(stmt, 1, &value, NULL), E_DB_OUT_OF_BOUNDS);
}

TEST_CASE_FIX(db_param_bind_sint8_ref, db_prepare, db_finalize)
{
    int8_t value = 123;

    test_int_success(db_param_bind_int8_ref(stmt, 0, &value, NULL));

    value = INT8_MIN;
    test_fsql("select %" PRId8 ";", INT8_MIN);

    value = INT8_MAX;
    test_fsql("select %" PRId8 ";", INT8_MAX);
}

TEST_CASE_FIX(db_param_bind_sint8_ref_null, db_prepare, db_finalize)
{
    int8_t value    = 123;
    bool null       = false;

    test_int_success(db_param_bind_int8_ref(stmt, 0, &value, &null));

    null = true;
    test_sql("select NULL;");

    null = false;
    test_sql("select 123;");
}

int test_suite_db_param_bind_sint8_ref(void *vparam)
{
    param = vparam;

    if(!param->supported)
        return error_pass_int(test_run_case(
            test_case(db_param_bind_sint8_ref_unsupported)));

    return error_pass_int(test_run_cases(NULL,
        test_case(db_param_bind_sint8_ref_oob),
        test_case(db_param_bind_sint8_ref),
        test_case(db_param_bind_sint8_ref_null),
        NULL
    ));
}

int test_suite_db_param_bind_sint8(void *vparam)
{
    return error_pass_int(test_run_suites(NULL,
        test_suite_p(db_param_bind_sint8_tmp, vparam),
        test_suite_p(db_param_bind_sint8_ref, vparam),
        NULL
    ));
}

TEST_CASE_FIX(db_param_bind_uint8_tmp_unsupported, db_prepare, db_finalize)
{
    test_int_error(db_param_bind_uint8(stmt, 0, 123), E_DB_UNSUPPORTED_TYPE);
}

TEST_CASE_FIX(db_param_bind_uint8_tmp_oob, db_prepare, db_finalize)
{
    test_int_error(db_param_bind_uint8(stmt, 1, 123), E_DB_OUT_OF_BOUNDS);
}

TEST_CASE_FIX(db_param_bind_uint8_tmp_min, db_prepare, db_finalize)
{
    test_int_success(db_param_bind_uint8(stmt, 0, 0));
    test_sql("select 0;");
}

TEST_CASE_FIX(db_param_bind_uint8_tmp_max, db_prepare, db_finalize)
{
    test_int_success(db_param_bind_uint8(stmt, 0, UINT8_MAX));
    test_fsql("select %" PRIu8 ";", UINT8_MAX);
}

int test_suite_db_param_bind_uint8_tmp(void *vparam)
{
    param = vparam;

    if(!param->supported)
        return error_pass_int(test_run_case(
            test_case(db_param_bind_uint8_tmp_unsupported)));

    return error_pass_int(test_run_cases(NULL,
        test_case(db_param_bind_uint8_tmp_oob),
        test_case(db_param_bind_uint8_tmp_min),
        test_case(db_param_bind_uint8_tmp_max),
        NULL
    ));
}

TEST_CASE_FIX(db_param_bind_uint8_ref_unsupported, db_prepare, db_finalize)
{
    uint8_t value = 123;

    test_int_error(db_param_bind_uint8_ref(stmt, 0, &value, NULL), E_DB_UNSUPPORTED_TYPE);
}

TEST_CASE_FIX(db_param_bind_uint8_ref_oob, db_prepare, db_finalize)
{
    uint8_t value = 123;

    test_int_error(db_param_bind_uint8_ref(stmt, 1, &value, NULL), E_DB_OUT_OF_BOUNDS);
}

TEST_CASE_FIX(db_param_bind_uint8_ref, db_prepare, db_finalize)
{
    uint8_t value = 123;

    test_int_success(db_param_bind_uint8_ref(stmt, 0, &value, NULL));

    value = 0;
    test_sql("select 0;");

    value = UINT8_MAX;
    test_fsql("select %" PRIu8 ";", UINT8_MAX);
}

TEST_CASE_FIX(db_param_bind_uint8_ref_null, db_prepare, db_finalize)
{
    uint8_t value   = 123;
    bool null       = false;

    test_int_success(db_param_bind_uint8_ref(stmt, 0, &value, &null));

    null = true;
    test_sql("select NULL;");

    null = false;
    test_sql("select 123;");
}

int test_suite_db_param_bind_uint8_ref(void *vparam)
{
    param = vparam;

    if(!param->supported)
        return error_pass_int(test_run_case(
            test_case(db_param_bind_uint8_ref_unsupported)));

    return error_pass_int(test_run_cases(NULL,
        test_case(db_param_bind_uint8_ref_oob),
        test_case(db_param_bind_uint8_ref),
        test_case(db_param_bind_uint8_ref_null),
        NULL
    ));
}

int test_suite_db_param_bind_uint8(void *vparam)
{
    return error_pass_int(test_run_suites(NULL,
        test_suite_p(db_param_bind_uint8_tmp, vparam),
        test_suite_p(db_param_bind_uint8_ref, vparam),
        NULL
    ));
}

int test_suite_db_param_bind_int8(void *vparam)
{
    return error_pass_int(test_run_suites(NULL,
        test_suite_p(db_param_bind_sint8, vparam),
        test_suite_p(db_param_bind_uint8, vparam),
        NULL
    ));
}

TEST_CASE_FIX(db_param_bind_sint16_tmp_unsupported, db_prepare, db_finalize)
{
    test_int_error(db_param_bind_int16(stmt, 0, 123), E_DB_UNSUPPORTED_TYPE);
}

TEST_CASE_FIX(db_param_bind_sint16_tmp_oob, db_prepare, db_finalize)
{
    test_int_error(db_param_bind_int16(stmt, 1, 123), E_DB_OUT_OF_BOUNDS);
}

TEST_CASE_FIX(db_param_bind_sint16_tmp_min, db_prepare, db_finalize)
{
    test_int_success(db_param_bind_int16(stmt, 0, INT16_MIN));
    test_fsql("select %" PRId16 ";", INT16_MIN);
}

TEST_CASE_FIX(db_param_bind_sint16_tmp_max, db_prepare, db_finalize)
{
    test_int_success(db_param_bind_int16(stmt, 0, INT16_MAX));
    test_fsql("select %" PRId16 ";", INT16_MAX);
}

int test_suite_db_param_bind_sint16_tmp(void *vparam)
{
    param = vparam;

    if(!param->supported)
        return error_pass_int(test_run_case(
            test_case(db_param_bind_sint16_tmp_unsupported)));

    return error_pass_int(test_run_cases(NULL,
        test_case(db_param_bind_sint16_tmp_oob),
        test_case(db_param_bind_sint16_tmp_min),
        test_case(db_param_bind_sint16_tmp_max),
        NULL
    ));
}

TEST_CASE_FIX(db_param_bind_sint16_ref_unsupported, db_prepare, db_finalize)
{
    int16_t value = 123;

    test_int_error(db_param_bind_int16_ref(stmt, 0, &value, NULL), E_DB_UNSUPPORTED_TYPE);
}

TEST_CASE_FIX(db_param_bind_sint16_ref_oob, db_prepare, db_finalize)
{
    int16_t value = 123;

    test_int_error(db_param_bind_int16_ref(stmt, 1, &value, NULL), E_DB_OUT_OF_BOUNDS);
}

TEST_CASE_FIX(db_param_bind_sint16_ref, db_prepare, db_finalize)
{
    int16_t value = 0;

    test_int_success(db_param_bind_int16_ref(stmt, 0, &value, NULL));

    value = INT16_MIN;
    test_fsql("select %" PRId16 ";", INT16_MIN);

    value = INT16_MAX;
    test_fsql("select %" PRId16 ";", INT16_MAX);
}

TEST_CASE_FIX(db_param_bind_sint16_ref_null, db_prepare, db_finalize)
{
    int16_t value   = 123;
    bool null       = false;

    test_int_success(db_param_bind_int16_ref(stmt, 0, &value, &null));

    null = true;
    test_sql("select NULL;");

    null = false;
    test_sql("select 123;");
}

int test_suite_db_param_bind_sint16_ref(void *vparam)
{
    param = vparam;

    if(!param->supported)
        return error_pass_int(test_run_case(
            test_case(db_param_bind_sint16_ref_unsupported)));

    return error_pass_int(test_run_cases(NULL,
        test_case(db_param_bind_sint16_ref_oob),
        test_case(db_param_bind_sint16_ref),
        test_case(db_param_bind_sint16_ref_null),
        NULL
    ));
}

int test_suite_db_param_bind_sint16(void *vparam)
{
    return error_pass_int(test_run_suites(NULL,
        test_suite_p(db_param_bind_sint16_tmp, vparam),
        test_suite_p(db_param_bind_sint16_ref, vparam),
        NULL
    ));
}

TEST_CASE_FIX(db_param_bind_uint16_tmp_unsupported, db_prepare, db_finalize)
{
    test_int_error(db_param_bind_uint16(stmt, 0, 123), E_DB_UNSUPPORTED_TYPE);
}

TEST_CASE_FIX(db_param_bind_uint16_tmp_oob, db_prepare, db_finalize)
{
    test_int_error(db_param_bind_uint16(stmt, 1, 123), E_DB_OUT_OF_BOUNDS);
}

TEST_CASE_FIX(db_param_bind_uint16_tmp_min, db_prepare, db_finalize)
{
    test_int_success(db_param_bind_uint16(stmt, 0, 0));
    test_sql("select 0;");
}

TEST_CASE_FIX(db_param_bind_uint16_tmp_max, db_prepare, db_finalize)
{
    test_int_success(db_param_bind_uint16(stmt, 0, UINT16_MAX));
    test_fsql("select %" PRIu16 ";", UINT16_MAX);
}

int test_suite_db_param_bind_uint16_tmp(void *vparam)
{
    param = vparam;

    if(!param->supported)
        return error_pass_int(test_run_case(
            test_case(db_param_bind_uint16_tmp_unsupported)));

    return error_pass_int(test_run_cases(NULL,
        test_case(db_param_bind_uint16_tmp_oob),
        test_case(db_param_bind_uint16_tmp_min),
        test_case(db_param_bind_uint16_tmp_max),
        NULL
    ));
}

TEST_CASE_FIX(db_param_bind_uint16_ref_unsupported, db_prepare, db_finalize)
{
    uint16_t value = 123;

    test_int_error(db_param_bind_uint16_ref(stmt, 0, &value, NULL), E_DB_UNSUPPORTED_TYPE);
}

TEST_CASE_FIX(db_param_bind_uint16_ref_oob, db_prepare, db_finalize)
{
    uint16_t value = 123;

    test_int_error(db_param_bind_uint16_ref(stmt, 1, &value, NULL), E_DB_OUT_OF_BOUNDS);
}

TEST_CASE_FIX(db_param_bind_uint16_ref, db_prepare, db_finalize)
{
    uint16_t value = 123;

    test_int_success(db_param_bind_uint16_ref(stmt, 0, &value, NULL));

    value = 0;
    test_sql("select 0;");

    value = UINT16_MAX;
    test_fsql("select %" PRIu16 ";", UINT16_MAX);
}

TEST_CASE_FIX(db_param_bind_uint16_ref_null, db_prepare, db_finalize)
{
    uint16_t value  = 123;
    bool null       = false;

    test_int_success(db_param_bind_uint16_ref(stmt, 0, &value, &null));

    null = true;
    test_sql("select NULL;");

    null = false;
    test_sql("select 123;");
}

int test_suite_db_param_bind_uint16_ref(void *vparam)
{
    param = vparam;

    if(!param->supported)
        return error_pass_int(test_run_case(
            test_case(db_param_bind_uint16_ref_unsupported)));

    return error_pass_int(test_run_cases(NULL,
        test_case(db_param_bind_uint16_ref_oob),
        test_case(db_param_bind_uint16_ref),
        test_case(db_param_bind_uint16_ref_null),
        NULL
    ));
}

int test_suite_db_param_bind_uint16(void *vparam)
{
    return error_pass_int(test_run_suites(NULL,
        test_suite_p(db_param_bind_uint16_tmp, vparam),
        test_suite_p(db_param_bind_uint16_ref, vparam),
        NULL
    ));
}

int test_suite_db_param_bind_int16(void *vparam)
{
    return error_pass_int(test_run_suites(NULL,
        test_suite_p(db_param_bind_sint16, vparam),
        test_suite_p(db_param_bind_uint16, vparam),
        NULL
    ));
}

TEST_CASE_FIX(db_param_bind_sint32_tmp_unsupported, db_prepare, db_finalize)
{
    test_int_error(db_param_bind_int32(stmt, 0, 123), E_DB_UNSUPPORTED_TYPE);
}

TEST_CASE_FIX(db_param_bind_sint32_tmp_oob, db_prepare, db_finalize)
{
    test_int_error(db_param_bind_int32(stmt, 1, 123), E_DB_OUT_OF_BOUNDS);
}

TEST_CASE_FIX(db_param_bind_sint32_tmp_min, db_prepare, db_finalize)
{
    test_int_success(db_param_bind_int32(stmt, 0, INT32_MIN));
    test_fsql("select %" PRId32 ";", INT32_MIN);
}

TEST_CASE_FIX(db_param_bind_sint32_tmp_max, db_prepare, db_finalize)
{
    test_int_success(db_param_bind_int32(stmt, 0, INT32_MAX));
    test_fsql("select %" PRId32 ";", INT32_MAX);
}

int test_suite_db_param_bind_sint32_tmp(void *vparam)
{
    param = vparam;

    if(!param->supported)
        return error_pass_int(test_run_case(
            test_case(db_param_bind_sint32_tmp_unsupported)));

    return error_pass_int(test_run_cases(NULL,
        test_case(db_param_bind_sint32_tmp_oob),
        test_case(db_param_bind_sint32_tmp_min),
        test_case(db_param_bind_sint32_tmp_max),
        NULL
    ));
}

TEST_CASE_FIX(db_param_bind_sint32_ref_unsupported, db_prepare, db_finalize)
{
    int32_t value = 123;

    test_int_error(db_param_bind_int32_ref(stmt, 0, &value, NULL), E_DB_UNSUPPORTED_TYPE);
}

TEST_CASE_FIX(db_param_bind_sint32_ref_oob, db_prepare, db_finalize)
{
    int32_t value = 0;

    test_int_error(db_param_bind_int32_ref(stmt, 1, &value, NULL), E_DB_OUT_OF_BOUNDS);
}

TEST_CASE_FIX(db_param_bind_sint32_ref, db_prepare, db_finalize)
{
    int32_t value = 0;

    test_int_success(db_param_bind_int32_ref(stmt, 0, &value, NULL));

    value = INT32_MIN;
    test_fsql("select %" PRId32 ";", INT32_MIN);

    value = INT32_MAX;
    test_fsql("select %" PRId32 ";", INT32_MAX);
}

TEST_CASE_FIX(db_param_bind_sint32_ref_null, db_prepare, db_finalize)
{
    int32_t value   = 123;
    bool null       = false;

    test_int_success(db_param_bind_int32_ref(stmt, 0, &value, &null));

    null = true;
    test_sql("select NULL;");

    null = false;
    test_sql("select 123;");
}

int test_suite_db_param_bind_sint32_ref(void *vparam)
{
    param = vparam;

    if(!param->supported)
        return error_pass_int(test_run_case(
            test_case(db_param_bind_sint32_ref_unsupported)));

    return error_pass_int(test_run_cases(NULL,
        test_case(db_param_bind_sint32_ref_oob),
        test_case(db_param_bind_sint32_ref),
        test_case(db_param_bind_sint32_ref_null),
        NULL
    ));
}

int test_suite_db_param_bind_sint32(void *vparam)
{
    return error_pass_int(test_run_suites(NULL,
        test_suite_p(db_param_bind_sint32_tmp, vparam),
        test_suite_p(db_param_bind_sint32_ref, vparam),
        NULL
    ));
}

TEST_CASE_FIX(db_param_bind_uint32_tmp_unsupported, db_prepare, db_finalize)
{
    test_int_error(db_param_bind_uint32(stmt, 0, 123), E_DB_UNSUPPORTED_TYPE);
}

TEST_CASE_FIX(db_param_bind_uint32_tmp_oob, db_prepare, db_finalize)
{
    test_int_error(db_param_bind_uint32(stmt, 1, 123), E_DB_OUT_OF_BOUNDS);
}

TEST_CASE_FIX(db_param_bind_uint32_tmp_min, db_prepare, db_finalize)
{
    test_int_success(db_param_bind_uint32(stmt, 0, 0));
    test_sql("select 0;");
}

TEST_CASE_FIX(db_param_bind_uint32_tmp_max, db_prepare, db_finalize)
{
    test_int_success(db_param_bind_uint32(stmt, 0, UINT32_MAX));
    test_fsql("select %" PRIu32 ";", UINT32_MAX);
}

int test_suite_db_param_bind_uint32_tmp(void *vparam)
{
    param = vparam;

    if(!param->supported)
        return error_pass_int(test_run_case(
            test_case(db_param_bind_uint32_tmp_unsupported)));

    return error_pass_int(test_run_cases(NULL,
        test_case(db_param_bind_uint32_tmp_oob),
        test_case(db_param_bind_uint32_tmp_min),
        test_case(db_param_bind_uint32_tmp_max),
        NULL
    ));
}

TEST_CASE_FIX(db_param_bind_uint32_ref_unsupported, db_prepare, db_finalize)
{
    uint32_t value = 123;

    test_int_error(db_param_bind_uint32_ref(stmt, 0, &value, NULL), E_DB_UNSUPPORTED_TYPE);
}

TEST_CASE_FIX(db_param_bind_uint32_ref_oob, db_prepare, db_finalize)
{
    uint32_t value = 123;

    test_int_error(db_param_bind_uint32_ref(stmt, 1, &value, NULL), E_DB_OUT_OF_BOUNDS);
}

TEST_CASE_FIX(db_param_bind_uint32_ref, db_prepare, db_finalize)
{
    uint32_t value = 123;

    test_int_success(db_param_bind_uint32_ref(stmt, 0, &value, NULL));

    value = 0;
    test_sql("select 0;");

    value = UINT32_MAX;
    test_fsql("select %" PRIu32 ";", UINT32_MAX);
}

TEST_CASE_FIX(db_param_bind_uint32_ref_null, db_prepare, db_finalize)
{
    uint32_t value  = 123;
    bool null       = false;

    test_int_success(db_param_bind_uint32_ref(stmt, 0, &value, &null));

    null = true;
    test_sql("select NULL;");

    null = false;
    test_sql("select 123;");
}

int test_suite_db_param_bind_uint32_ref(void *vparam)
{
    param = vparam;

    if(!param->supported)
        return error_pass_int(test_run_case(
            test_case(db_param_bind_uint32_ref_unsupported)));

    return error_pass_int(test_run_cases(NULL,
        test_case(db_param_bind_uint32_ref_oob),
        test_case(db_param_bind_uint32_ref),
        test_case(db_param_bind_uint32_ref_null),
        NULL
    ));
}

int test_suite_db_param_bind_uint32(void *vparam)
{
    return error_pass_int(test_run_suites(NULL,
        test_suite_p(db_param_bind_uint32_tmp, vparam),
        test_suite_p(db_param_bind_uint32_ref, vparam),
        NULL
    ));
}

int test_suite_db_param_bind_int32(void *vparam)
{
    return error_pass_int(test_run_suites(NULL,
        test_suite_p(db_param_bind_sint32, vparam),
        test_suite_p(db_param_bind_uint32, vparam),
        NULL
    ));
}

TEST_CASE_FIX(db_param_bind_sint64_tmp_unsupported, db_prepare, db_finalize)
{
    test_int_error(db_param_bind_int64(stmt, 0, 123), E_DB_UNSUPPORTED_TYPE);
}

TEST_CASE_FIX(db_param_bind_sint64_tmp_oob, db_prepare, db_finalize)
{
    test_int_error(db_param_bind_int64(stmt, 1, 123), E_DB_OUT_OF_BOUNDS);
}

TEST_CASE_FIX(db_param_bind_sint64_tmp_min, db_prepare, db_finalize)
{
    test_int_success(db_param_bind_int64(stmt, 0, INT64_MIN));
    test_fsql("select %" PRId64 ";", INT64_MIN);
}

TEST_CASE_FIX(db_param_bind_sint64_tmp_max, db_prepare, db_finalize)
{
    test_int_success(db_param_bind_int64(stmt, 0, INT64_MAX));
    test_fsql("select %" PRId64 ";", INT64_MAX);
}

int test_suite_db_param_bind_sint64_tmp(void *vparam)
{
    param = vparam;

    if(!param->supported)
        return error_pass_int(test_run_case(
            test_case(db_param_bind_sint64_tmp_unsupported)));

    return error_pass_int(test_run_cases(NULL,
        test_case(db_param_bind_sint64_tmp_oob),
        test_case(db_param_bind_sint64_tmp_min),
        test_case(db_param_bind_sint64_tmp_max),
        NULL
    ));
}

TEST_CASE_FIX(db_param_bind_sint64_ref_unsupported, db_prepare, db_finalize)
{
    int64_t value = 123;

    test_int_error(db_param_bind_int64_ref(stmt, 0, &value, NULL), E_DB_UNSUPPORTED_TYPE);
}

TEST_CASE_FIX(db_param_bind_sint64_ref_oob, db_prepare, db_finalize)
{
    int64_t value = 123;

    test_int_error(db_param_bind_int64_ref(stmt, 1, &value, NULL), E_DB_OUT_OF_BOUNDS);
}

TEST_CASE_FIX(db_param_bind_sint64_ref, db_prepare, db_finalize)
{
    int64_t value = 0;

    test_int_success(db_param_bind_int64_ref(stmt, 0, &value, NULL));

    value = INT64_MIN;
    test_fsql("select %" PRId64 ";", INT64_MIN);

    value = INT64_MAX;
    test_fsql("select %" PRId64 ";", INT64_MAX);
}

TEST_CASE_FIX(db_param_bind_sint64_ref_null, db_prepare, db_finalize)
{
    int64_t value   = 123;
    bool null       = false;

    test_int_success(db_param_bind_int64_ref(stmt, 0, &value, &null));

    null = true;
    test_sql("select NULL;");

    null = false;
    test_sql("select 123;");
}

int test_suite_db_param_bind_sint64_ref(void *vparam)
{
    param = vparam;

    if(!param->supported)
        return error_pass_int(test_run_case(
            test_case(db_param_bind_sint64_ref_unsupported)));

    return error_pass_int(test_run_cases(NULL,
        test_case(db_param_bind_sint64_ref_oob),
        test_case(db_param_bind_sint64_ref),
        test_case(db_param_bind_sint64_ref_null),
        NULL
    ));
}

int test_suite_db_param_bind_sint64(void *vparam)
{
    return error_pass_int(test_run_suites(NULL,
        test_suite_p(db_param_bind_sint64_tmp, vparam),
        test_suite_p(db_param_bind_sint64_ref, vparam),
        NULL
    ));
}

TEST_CASE_FIX(db_param_bind_uint64_tmp_unsupported, db_prepare, db_finalize)
{
    test_int_error(db_param_bind_uint64(stmt, 0, 123), E_DB_UNSUPPORTED_TYPE);
}

TEST_CASE_FIX(db_param_bind_uint64_tmp_oob, db_prepare, db_finalize)
{
    test_int_error(db_param_bind_uint64(stmt, 1, 123), E_DB_OUT_OF_BOUNDS);
}

TEST_CASE_FIX(db_param_bind_uint64_tmp_min, db_prepare, db_finalize)
{
    test_int_success(db_param_bind_uint64(stmt, 0, 0));
    test_sql("select 0;");
}

TEST_CASE_FIX(db_param_bind_uint64_tmp_max, db_prepare, db_finalize)
{
    test_int_success(db_param_bind_uint64(stmt, 0, UINT64_MAX));
    test_fsql("select %" PRIu64 ";", UINT64_MAX);
}

int test_suite_db_param_bind_uint64_tmp(void *vparam)
{
    param = vparam;

    if(!param->supported)
        return error_pass_int(test_run_case(
            test_case(db_param_bind_uint64_tmp_unsupported)));

    return error_pass_int(test_run_cases(NULL,
        test_case(db_param_bind_uint64_tmp_oob),
        test_case(db_param_bind_uint64_tmp_min),
        test_case(db_param_bind_uint64_tmp_max),
        NULL
    ));
}

TEST_CASE_FIX(db_param_bind_uint64_ref_unsupported, db_prepare, db_finalize)
{
    uint64_t value = 123;

    test_int_error(db_param_bind_uint64_ref(stmt, 0, &value, NULL), E_DB_UNSUPPORTED_TYPE);
}

TEST_CASE_FIX(db_param_bind_uint64_ref_oob, db_prepare, db_finalize)
{
    uint64_t value = 123;

    test_int_error(db_param_bind_uint64_ref(stmt, 1, &value, NULL), E_DB_OUT_OF_BOUNDS);
}

TEST_CASE_FIX(db_param_bind_uint64_ref, db_prepare, db_finalize)
{
    uint64_t value = 123;

    test_int_success(db_param_bind_uint64_ref(stmt, 0, &value, NULL));

    value = 0;
    test_sql("select 0;");

    value = UINT64_MAX;
    test_fsql("select %" PRIu64 ";", UINT64_MAX);
}

TEST_CASE_FIX(db_param_bind_uint64_ref_null, db_prepare, db_finalize)
{
    uint64_t value  = 123;
    bool null       = false;

    test_int_success(db_param_bind_uint64_ref(stmt, 0, &value, &null));

    null = true;
    test_sql("select NULL;");

    null = false;
    test_sql("select 123;");
}

int test_suite_db_param_bind_uint64_ref(void *vparam)
{
    param = vparam;

    if(!param->supported)
        return error_pass_int(test_run_case(
            test_case(db_param_bind_uint64_ref_unsupported)));

    return error_pass_int(test_run_cases(NULL,
        test_case(db_param_bind_uint64_ref_oob),
        test_case(db_param_bind_uint64_ref),
        test_case(db_param_bind_uint64_ref_null),
        NULL
    ));
}

int test_suite_db_param_bind_uint64(void *vparam)
{
    return error_pass_int(test_run_suites(NULL,
        test_suite_p(db_param_bind_uint64_tmp, vparam),
        test_suite_p(db_param_bind_uint64_ref, vparam),
        NULL
    ));
}

int test_suite_db_param_bind_int64(void *vparam)
{
    return error_pass_int(test_run_suites(NULL,
        test_suite_p(db_param_bind_sint64, vparam),
        test_suite_p(db_param_bind_uint64, vparam),
        NULL
    ));
}

TEST_CASE_FIX(db_param_bind_float_tmp_unsupported, db_prepare, db_finalize)
{
    test_int_error(db_param_bind_float(stmt, 0, 123), E_DB_UNSUPPORTED_TYPE);
}

TEST_CASE_FIX(db_param_bind_float_tmp_oob, db_prepare, db_finalize)
{
    test_int_error(db_param_bind_float(stmt, 1, 123), E_DB_OUT_OF_BOUNDS);
}

TEST_CASE_FIX(db_param_bind_float_tmp_min, db_prepare, db_finalize)
{
    test_int_success(db_param_bind_float(stmt, 0, FLT_MIN));
    test_fsql("select %.*g;", param->config->flt_dig, FLT_MIN);
}

TEST_CASE_FIX(db_param_bind_float_tmp_max, db_prepare, db_finalize)
{
    test_int_success(db_param_bind_float(stmt, 0, FLT_MAX));
    test_fsql("select %.*g;", param->config->flt_dig, FLT_MAX);
}

TEST_CASE_FIX(db_param_bind_float_tmp_nan, db_prepare, db_finalize)
{
    test_int_success(db_param_bind_float(stmt, 0, NAN));
    test_fsql("select %s;", param->config->nan);
}

TEST_CASE_FIX(db_param_bind_float_tmp_inf, db_prepare, db_finalize)
{
    test_int_success(db_param_bind_float(stmt, 0, INFINITY));
    test_fsql("select %s;", param->config->inf);
}

int test_suite_db_param_bind_float_tmp(void *vparam)
{
    param = vparam;

    if(!param->supported)
        return error_pass_int(test_run_case(
            test_case(db_param_bind_float_tmp_unsupported)));

    return error_pass_int(test_run_cases(NULL,
        test_case(db_param_bind_float_tmp_oob),
        test_case(db_param_bind_float_tmp_min),
        test_case(db_param_bind_float_tmp_max),
        test_case(db_param_bind_float_tmp_nan),
        test_case(db_param_bind_float_tmp_inf),
        NULL
    ));
}

TEST_CASE_FIX(db_param_bind_float_ref_unsupported, db_prepare, db_finalize)
{
    float value = 123;

    test_int_error(db_param_bind_float_ref(stmt, 0, &value, NULL), E_DB_UNSUPPORTED_TYPE);
}

TEST_CASE_FIX(db_param_bind_float_ref_oob, db_prepare, db_finalize)
{
    float value = 123;

    test_int_error(db_param_bind_float_ref(stmt, 1, &value, NULL), E_DB_OUT_OF_BOUNDS);
}

TEST_CASE_FIX(db_param_bind_float_ref, db_prepare, db_finalize)
{
    float value = 0;

    test_int_success(db_param_bind_float_ref(stmt, 0, &value, NULL));

    value = FLT_MIN;
    test_fsql("select %.*g;", param->config->flt_dig, value);

    value = FLT_MAX;
    test_fsql("select %.*g;", param->config->flt_dig, value);
}

TEST_CASE_FIX(db_param_bind_float_ref_nan, db_prepare, db_finalize)
{
    float value = 0;

    test_int_success(db_param_bind_float_ref(stmt, 0, &value, NULL));

    value = NAN;
    test_fsql("select %s;", param->config->nan);

    value = 123.5;
    test_sql("select 123.5;");
}

TEST_CASE_FIX(db_param_bind_float_ref_inf, db_prepare, db_finalize)
{
    float value = 0;

    test_int_success(db_param_bind_float_ref(stmt, 0, &value, NULL));

    value = INFINITY;
    test_fsql("select %s;", param->config->inf);

    value = -INFINITY;
    test_fsql("select -%s;", param->config->inf);
}

TEST_CASE_FIX(db_param_bind_float_ref_null, db_prepare, db_finalize)
{
    float value = 123.5;
    bool null   = false;

    test_int_success(db_param_bind_float_ref(stmt, 0, &value, &null));

    null = true;
    test_sql("select NULL;");

    null = false;
    test_sql("select 123.5;");
}

int test_suite_db_param_bind_float_ref(void *vparam)
{
    param = vparam;

    if(!param->supported)
        return error_pass_int(test_run_case(
            test_case(db_param_bind_float_ref_unsupported)));

    return error_pass_int(test_run_cases(NULL,
        test_case(db_param_bind_float_ref_oob),
        test_case(db_param_bind_float_ref),
        test_case(db_param_bind_float_ref_nan),
        test_case(db_param_bind_float_ref_inf),
        test_case(db_param_bind_float_ref_null),
        NULL
    ));
}

int test_suite_db_param_bind_float(void *vparam)
{
    return error_pass_int(test_run_suites(NULL,
        test_suite_p(db_param_bind_float_tmp, vparam),
        test_suite_p(db_param_bind_float_ref, vparam),
        NULL
    ));
}

TEST_CASE_FIX(db_param_bind_double_tmp_unsupported, db_prepare, db_finalize)
{
    test_int_error(db_param_bind_double(stmt, 0, 123), E_DB_UNSUPPORTED_TYPE);
}

TEST_CASE_FIX(db_param_bind_double_tmp_oob, db_prepare, db_finalize)
{
    test_int_error(db_param_bind_double(stmt, 1, 123), E_DB_OUT_OF_BOUNDS);
}

TEST_CASE_FIX(db_param_bind_double_tmp_min, db_prepare, db_finalize)
{
    test_int_success(db_param_bind_double(stmt, 0, DBL_MIN));
    test_fsql("select %.*g;", param->config->dbl_dig, DBL_MIN);
}

TEST_CASE_FIX(db_param_bind_double_tmp_max, db_prepare, db_finalize)
{
    test_int_success(db_param_bind_double(stmt, 0, DBL_MAX));
    test_fsql("select %.*g;", param->config->dbl_dig, DBL_MAX);
}

TEST_CASE_FIX(db_param_bind_double_tmp_nan, db_prepare, db_finalize)
{
    test_int_success(db_param_bind_double(stmt, 0, NAN));
    test_fsql("select %s;", param->config->nan);
}

TEST_CASE_FIX(db_param_bind_double_tmp_inf, db_prepare, db_finalize)
{
    test_int_success(db_param_bind_double(stmt, 0, INFINITY));
    test_fsql("select %s;", param->config->inf);
}

int test_suite_db_param_bind_double_tmp(void *vparam)
{
    param = vparam;

    if(!param->supported)
        return error_pass_int(test_run_case(
            test_case(db_param_bind_double_tmp_unsupported)));

    return error_pass_int(test_run_cases(NULL,
        test_case(db_param_bind_double_tmp_oob),
        test_case(db_param_bind_double_tmp_min),
        test_case(db_param_bind_double_tmp_max),
        test_case(db_param_bind_double_tmp_nan),
        test_case(db_param_bind_double_tmp_inf),
        NULL
    ));
}

TEST_CASE_FIX(db_param_bind_double_ref_unsupported, db_prepare, db_finalize)
{
    double value = 123;

    test_int_error(db_param_bind_double_ref(stmt, 0, &value, NULL), E_DB_UNSUPPORTED_TYPE);
}

TEST_CASE_FIX(db_param_bind_double_ref_oob, db_prepare, db_finalize)
{
    double value = 123;

    test_int_error(db_param_bind_double_ref(stmt, 1, &value, NULL), E_DB_OUT_OF_BOUNDS);
}

TEST_CASE_FIX(db_param_bind_double_ref, db_prepare, db_finalize)
{
    double value = 0;

    test_int_success(db_param_bind_double_ref(stmt, 0, &value, NULL));

    value = DBL_MIN;
    test_fsql("select %.*g;", param->config->dbl_dig, value);

    value = DBL_MAX;
    test_fsql("select %.*g;", param->config->dbl_dig, value);
}

TEST_CASE_FIX(db_param_bind_double_ref_nan, db_prepare, db_finalize)
{
    double value = 0;

    test_int_success(db_param_bind_double_ref(stmt, 0, &value, NULL));

    value = NAN;
    test_fsql("select %s;", param->config->nan);

    value = 123.5;
    test_sql("select 123.5;");
}

TEST_CASE_FIX(db_param_bind_double_ref_inf, db_prepare, db_finalize)
{
    double value = 0;

    test_int_success(db_param_bind_double_ref(stmt, 0, &value, NULL));

    value = INFINITY;
    test_fsql("select %s;", param->config->inf);

    value = -INFINITY;
    test_fsql("select -%s;", param->config->inf);
}

TEST_CASE_FIX(db_param_bind_double_ref_null, db_prepare, db_finalize)
{
    double value    = 123.5;
    bool null       = false;

    test_int_success(db_param_bind_double_ref(stmt, 0, &value, &null));

    null = true;
    test_sql("select NULL;");

    null = false;
    test_sql("select 123.5;");
}

int test_suite_db_param_bind_double_ref(void *vparam)
{
    param = vparam;

    if(!param->supported)
        return error_pass_int(test_run_case(
            test_case(db_param_bind_double_ref_unsupported)));

    return error_pass_int(test_run_cases(NULL,
        test_case(db_param_bind_double_ref_oob),
        test_case(db_param_bind_double_ref),
        test_case(db_param_bind_double_ref_nan),
        test_case(db_param_bind_double_ref_inf),
        test_case(db_param_bind_double_ref_null),
        NULL
    ));
}

int test_suite_db_param_bind_double(void *vparam)
{
    return error_pass_int(test_run_suites(NULL,
        test_suite_p(db_param_bind_double_tmp, vparam),
        test_suite_p(db_param_bind_double_ref, vparam),
        NULL
    ));
}

TEST_CASE_FIX(db_param_bind_ldouble_tmp_unsupported, db_prepare, db_finalize)
{
    test_int_error(db_param_bind_ldouble(stmt, 0, 123), E_DB_UNSUPPORTED_TYPE);
}

TEST_CASE_FIX(db_param_bind_ldouble_tmp_oob, db_prepare, db_finalize)
{
    test_int_error(db_param_bind_ldouble(stmt, 1, 123), E_DB_OUT_OF_BOUNDS);
}

TEST_CASE_FIX(db_param_bind_ldouble_tmp_min, db_prepare, db_finalize)
{
    test_int_success(db_param_bind_ldouble(stmt, 0, LDBL_MIN));
    test_fsql("select %.*Lg;", param->config->ldbl_dig, LDBL_MIN);
}

TEST_CASE_FIX(db_param_bind_ldouble_tmp_max, db_prepare, db_finalize)
{
    test_int_success(db_param_bind_ldouble(stmt, 0, LDBL_MAX));
    test_fsql("select %.*Lg;", param->config->ldbl_dig, LDBL_MAX);
}

TEST_CASE_FIX(db_param_bind_ldouble_tmp_nan, db_prepare, db_finalize)
{
    test_int_success(db_param_bind_ldouble(stmt, 0, NAN));
    test_fsql("select %s;", param->config->nan);
}

TEST_CASE_FIX(db_param_bind_ldouble_tmp_inf, db_prepare, db_finalize)
{
    test_int_success(db_param_bind_ldouble(stmt, 0, INFINITY));
    test_fsql("select %s;", param->config->inf);
}

int test_suite_db_param_bind_ldouble_tmp(void *vparam)
{
    param = vparam;

    if(!param->supported)
        return error_pass_int(test_run_case(
            test_case(db_param_bind_ldouble_tmp_unsupported)));

    return error_pass_int(test_run_cases(NULL,
        test_case(db_param_bind_ldouble_tmp_oob),
        test_case(db_param_bind_ldouble_tmp_min),
        test_case(db_param_bind_ldouble_tmp_max),
        test_case(db_param_bind_ldouble_tmp_nan),
        test_case(db_param_bind_ldouble_tmp_inf),
        NULL
    ));
}

TEST_CASE_FIX(db_param_bind_ldouble_ref_unsupported, db_prepare, db_finalize)
{
    long double value = 123;

    test_int_error(db_param_bind_ldouble_ref(stmt, 0, &value, NULL), E_DB_UNSUPPORTED_TYPE);
}

TEST_CASE_FIX(db_param_bind_ldouble_ref_oob, db_prepare, db_finalize)
{
    long double value = 123;

    test_int_error(db_param_bind_ldouble_ref(stmt, 1, &value, NULL), E_DB_OUT_OF_BOUNDS);
}

TEST_CASE_FIX(db_param_bind_ldouble_ref, db_prepare, db_finalize)
{
    long double value = 0;

    test_int_success(db_param_bind_ldouble_ref(stmt, 0, &value, NULL));

    value = LDBL_MIN;
    test_fsql("select %.*Lg;", param->config->ldbl_dig, value);

    value = LDBL_MAX;
    test_fsql("select %.*Lg;", param->config->ldbl_dig, value);
}

TEST_CASE_FIX(db_param_bind_ldouble_ref_nan, db_prepare, db_finalize)
{
    long double value = 0;

    test_int_success(db_param_bind_ldouble_ref(stmt, 0, &value, NULL));

    value = NAN;
    test_fsql("select %s;", param->config->nan);

    value = 123.5;
    test_sql("select 123.5;");
}

TEST_CASE_FIX(db_param_bind_ldouble_ref_inf, db_prepare, db_finalize)
{
    long double value = 0;

    test_int_success(db_param_bind_ldouble_ref(stmt, 0, &value, NULL));

    value = INFINITY;
    test_fsql("select %s;", param->config->inf);

    value = -INFINITY;
    test_fsql("select -%s;", param->config->inf);
}

TEST_CASE_FIX(db_param_bind_ldouble_ref_null, db_prepare, db_finalize)
{
    long double value   = 123.5;
    bool null           = false;

    test_int_success(db_param_bind_ldouble_ref(stmt, 0, &value, &null));

    null = true;
    test_sql("select NULL;");

    null = false;
    test_sql("select 123.5;");
}

int test_suite_db_param_bind_ldouble_ref(void *vparam)
{
    param = vparam;

    if(!param->supported)
        return error_pass_int(test_run_case(
            test_case(db_param_bind_ldouble_ref_unsupported)));

    return error_pass_int(test_run_cases(NULL,
        test_case(db_param_bind_ldouble_ref_oob),
        test_case(db_param_bind_ldouble_ref),
        test_case(db_param_bind_ldouble_ref_nan),
        test_case(db_param_bind_ldouble_ref_inf),
        test_case(db_param_bind_ldouble_ref_null),
        NULL
    ));
}

int test_suite_db_param_bind_ldouble(void *vparam)
{
    return error_pass_int(test_run_suites(NULL,
        test_suite_p(db_param_bind_ldouble_tmp, vparam),
        test_suite_p(db_param_bind_ldouble_ref, vparam),
        NULL
    ));
}

TEST_CASE_FIX(db_param_bind_text_fix_unsupported, db_prepare, db_finalize)
{
    test_int_error(db_param_bind_text(stmt, 0, "foo", -1), E_DB_UNSUPPORTED_TYPE);
}

TEST_CASE_FIX(db_param_bind_text_fix_oob, db_prepare, db_finalize)
{
    test_int_error(db_param_bind_text(stmt, 1, "foo", -1), E_DB_OUT_OF_BOUNDS);
}

TEST_CASE_FIX(db_param_bind_text_fix, db_prepare, db_finalize)
{
    test_int_success(db_param_bind_text(stmt, 0, "foo", -1));
    test_sql("select 'foo';");
}

TEST_CASE_FIX(db_param_bind_text_fix_len, db_prepare, db_finalize)
{
    test_int_success(db_param_bind_text(stmt, 0, "foobarbaz", 6));
    test_sql("select 'foobar';");
}

int test_suite_db_param_bind_text_fix(void *vparam)
{
    param = vparam;

    if(!param->supported)
        return error_pass_int(test_run_case(
            test_case(db_param_bind_text_fix_unsupported)));

    return error_pass_int(test_run_cases(NULL,
        test_case(db_param_bind_text_fix_oob),
        test_case(db_param_bind_text_fix),
        test_case(db_param_bind_text_fix_len),
        NULL
    ));
}

TEST_CASE_FIX(db_param_bind_text_tmp_unsupported, db_prepare, db_finalize)
{
    test_int_error(db_param_bind_text_tmp(stmt, 0, "foo", -1), E_DB_UNSUPPORTED_TYPE);
}

TEST_CASE_FIX(db_param_bind_text_tmp_oob, db_prepare, db_finalize)
{
    test_int_error(db_param_bind_text_tmp(stmt, 1, "foo", -1), E_DB_OUT_OF_BOUNDS);
}

TEST_CASE_FIX(db_param_bind_text_tmp, db_prepare, db_finalize)
{
    char text[10] = "foo";

    test_int_success(db_param_bind_text_tmp(stmt, 0, text, -1));

    strncpy(text, "bar", sizeof(text));
    test_sql("select 'foo';");
}

TEST_CASE_FIX(db_param_bind_text_tmp_len, db_prepare, db_finalize)
{
    char text[10] = "foobarbaz";

    test_int_success(db_param_bind_text_tmp(stmt, 0, text, 6));

    strncpy(text, "blubb", sizeof(text));
    test_sql("select 'foobar';");
}

int test_suite_db_param_bind_text_tmp(void *vparam)
{
    param = vparam;

    if(!param->supported)
        return error_pass_int(test_run_case(
            test_case(db_param_bind_text_tmp_unsupported)));

    return error_pass_int(test_run_cases(NULL,
        test_case(db_param_bind_text_tmp_oob),
        test_case(db_param_bind_text_tmp),
        test_case(db_param_bind_text_tmp_len),
        NULL
    ));
}

TEST_CASE_FIX(db_param_bind_text_ref_unsupported, db_prepare, db_finalize)
{
    const char *text = "foo";

    test_int_error(db_param_bind_text_ref(stmt, 0, &text, NULL), E_DB_UNSUPPORTED_TYPE);
}

TEST_CASE_FIX(db_param_bind_text_ref_oob, db_prepare, db_finalize)
{
    const char *text = "foo";

    test_int_error(db_param_bind_text_ref(stmt, 1, &text, NULL), E_DB_OUT_OF_BOUNDS);
}

TEST_CASE_FIX(db_param_bind_text_ref, db_prepare, db_finalize)
{
    const char *text = "foo";

    test_int_success(db_param_bind_text_ref(stmt, 0, &text, NULL));

    text = "bar";
    test_sql("select 'bar';");

    text = "baz";
    test_sql("select 'baz';");
}

TEST_CASE_FIX(db_param_bind_text_ref_len, db_prepare, db_finalize)
{
    const char *text    = "foobarbaz";
    size_t size         = 0;

    test_int_success(db_param_bind_text_ref(stmt, 0, &text, &size));

    size = 6;
    test_sql("select 'foobar';");

    size = 3;
    test_sql("select 'foo';");
}

TEST_CASE_FIX(db_param_bind_text_ref_null, db_prepare, db_finalize)
{
    const char *text = "foo";

    test_int_success(db_param_bind_text_ref(stmt, 0, &text, NULL));

    text = NULL;
    test_sql("select NULL;");

    text = "bar";
    test_sql("select 'bar';");
}

int test_suite_db_param_bind_text_ref(void *vparam)
{
    param = vparam;

    if(!param->supported)
        return error_pass_int(test_run_case(
            test_case(db_param_bind_text_ref_unsupported)));

    return error_pass_int(test_run_cases(NULL,
        test_case(db_param_bind_text_ref_oob),
        test_case(db_param_bind_text_ref),
        test_case(db_param_bind_text_ref_len),
        test_case(db_param_bind_text_ref_null),
        NULL
    ));
}

int test_suite_db_param_bind_text(void *vparam)
{
    return error_pass_int(test_run_suites(NULL,
        test_suite_p(db_param_bind_text_fix, vparam),
        test_suite_p(db_param_bind_text_tmp, vparam),
        test_suite_p(db_param_bind_text_ref, vparam),
        NULL
    ));
}

TEST_CASE_FIX(db_param_bind_blob_fix_unsupported, db_prepare, db_finalize)
{
    test_int_error(db_param_bind_blob(stmt, 0, "\x12\x34", 2), E_DB_UNSUPPORTED_TYPE);
}

TEST_CASE_FIX(db_param_bind_blob_fix_oob, db_prepare, db_finalize)
{
    test_int_error(db_param_bind_blob(stmt, 1, "\x12\x34", 2), E_DB_OUT_OF_BOUNDS);
}

TEST_CASE_FIX(db_param_bind_blob_fix, db_prepare, db_finalize)
{
    test_int_success(db_param_bind_blob(stmt, 0, "\x12\x34", 2));
    test_sql("select x'1234';");
}

int test_suite_db_param_bind_blob_fix(void *vparam)
{
    param = vparam;

    if(!param->supported)
        return error_pass_int(test_run_case(
            test_case(db_param_bind_blob_fix_unsupported)));

    return error_pass_int(test_run_cases(NULL,
        test_case(db_param_bind_blob_fix_oob),
        test_case(db_param_bind_blob_fix),
        NULL
    ));
}

TEST_CASE_FIX(db_param_bind_blob_tmp_unsupported, db_prepare, db_finalize)
{
    test_int_error(db_param_bind_blob_tmp(stmt, 0, "\x12\x34", 2), E_DB_UNSUPPORTED_TYPE);
}

TEST_CASE_FIX(db_param_bind_blob_tmp_oob, db_prepare, db_finalize)
{
    test_int_error(db_param_bind_blob_tmp(stmt, 1, "\xab\xcd", 2), E_DB_OUT_OF_BOUNDS);
}

TEST_CASE_FIX(db_param_bind_blob_tmp, db_prepare, db_finalize)
{
    char blob[10] = "\x12\x34\x56\x78\x90";

    test_int_success(db_param_bind_blob_tmp(stmt, 0, blob, 4));

    strncpy(blob, "\xde\xad\xbe\xef", sizeof(blob));
    test_sql("select x'12345678';");
}

int test_suite_db_param_bind_blob_tmp(void *vparam)
{
    param = vparam;

    if(!param->supported)
        return error_pass_int(test_run_case(
            test_case(db_param_bind_blob_tmp_unsupported)));

    return error_pass_int(test_run_cases(NULL,
        test_case(db_param_bind_blob_tmp_oob),
        test_case(db_param_bind_blob_tmp),
        NULL
    ));
}

TEST_CASE_FIX(db_param_bind_blob_ref_unsupported, db_prepare, db_finalize)
{
    const void *blob    = "\x12\x34\x56\x78\x90";
    size_t size         = 4;

    test_int_error(db_param_bind_blob_ref(stmt, 0, &blob, &size), E_DB_UNSUPPORTED_TYPE);
}

TEST_CASE_FIX(db_param_bind_blob_ref_oob, db_prepare, db_finalize)
{
    const void *blob    = "\x12\x34\x56\x78\x90";
    size_t size         = 4;

    test_int_error(db_param_bind_blob_ref(stmt, 1, &blob, &size), E_DB_OUT_OF_BOUNDS);
}

TEST_CASE_FIX(db_param_bind_blob_ref, db_prepare, db_finalize)
{
    const void *blob    = "\x12\x34\x56\x78\x90";
    size_t size         = 0;

    test_int_success(db_param_bind_blob_ref(stmt, 0, &blob, &size));

    blob    = "\xde\xad\xbe\xef";
    size    = 4;
    test_sql("select x'deadbeef';");

    blob    = "\x12\x34\x56\x78";
    size    = 3;
    test_sql("select x'123456';");
}

TEST_CASE_FIX(db_param_bind_blob_ref_null, db_prepare, db_finalize)
{
    const void *blob    = "\x12\x34\x56\x78\x90";
    size_t size         = 4;

    test_int_success(db_param_bind_blob_ref(stmt, 0, &blob, &size));

    blob = NULL;
    test_sql("select NULL;");

    blob = "\xde\xad\xbe\xef";
    test_sql("select x'deadbeef';");
}

int test_suite_db_param_bind_blob_ref(void *vparam)
{
    param = vparam;

    if(!param->supported)
        return error_pass_int(test_run_case(
            test_case(db_param_bind_blob_ref_unsupported)));

    return error_pass_int(test_run_cases(NULL,
        test_case(db_param_bind_blob_ref_oob),
        test_case(db_param_bind_blob_ref),
        test_case(db_param_bind_blob_ref_null),
        NULL
    ));
}

int test_suite_db_param_bind_blob(void *vparam)
{
    return error_pass_int(test_run_suites(NULL,
        test_suite_p(db_param_bind_blob_fix, vparam),
        test_suite_p(db_param_bind_blob_tmp, vparam),
        test_suite_p(db_param_bind_blob_ref, vparam),
        NULL
    ));
}

TEST_CASE_FIX(db_param_bind_date_fix_unsupported, db_prepare, db_finalize)
{
    db_date_st date = { .year = 2020, .month = 04, .day = 20 };

    test_int_error(db_param_bind_date(stmt, 0, &date), E_DB_UNSUPPORTED_TYPE);
}

TEST_CASE_FIX(db_param_bind_date_fix_oob, db_prepare, db_finalize)
{
    db_date_st date = { .year = 2020, .month = 04, .day = 20 };

    test_int_error(db_param_bind_date(stmt, 1, &date), E_DB_OUT_OF_BOUNDS);
}

TEST_CASE_FIX(db_param_bind_date_fix, db_prepare, db_finalize)
{
    db_date_st date = { .year = 2020, .month = 04, .day = 20 };

    test_int_success(db_param_bind_date(stmt, 0, &date));
    test_sql("select '2020-04-20';");
}

int test_suite_db_param_bind_date_fix(void *vparam)
{
    param = vparam;

    if(!param->supported)
        return error_pass_int(test_run_case(
            test_case(db_param_bind_date_fix_unsupported)));

    return error_pass_int(test_run_cases(NULL,
        test_case(db_param_bind_date_fix_oob),
        test_case(db_param_bind_date_fix),
        NULL
    ));
}

TEST_CASE_FIX(db_param_bind_date_tmp_unsupported, db_prepare, db_finalize)
{
    db_date_st date = { .year = 2020, .month = 04, .day = 20 };

    test_int_error(db_param_bind_date_tmp(stmt, 0, &date), E_DB_UNSUPPORTED_TYPE);
}

TEST_CASE_FIX(db_param_bind_date_tmp_oob, db_prepare, db_finalize)
{
    db_date_st date = { .year = 2020, .month = 04, .day = 20 };

    test_int_error(db_param_bind_date_tmp(stmt, 1, &date), E_DB_OUT_OF_BOUNDS);
}

TEST_CASE_FIX(db_param_bind_date_tmp, db_prepare, db_finalize)
{
    db_date_st date = { .year = 2020, .month = 04, .day = 20 };

    test_int_success(db_param_bind_date_tmp(stmt, 0, &date));

    date.year++;
    test_sql("select '2020-04-20';");
}

int test_suite_db_param_bind_date_tmp(void *vparam)
{
    param = vparam;

    if(!param->supported)
        return error_pass_int(test_run_case(
            test_case(db_param_bind_date_tmp_unsupported)));

    return error_pass_int(test_run_cases(NULL,
        test_case(db_param_bind_date_tmp_oob),
        test_case(db_param_bind_date_tmp),
        NULL
    ));
}

TEST_CASE_FIX(db_param_bind_date_ref_unsupported, db_prepare, db_finalize)
{
    db_date_st date = { .year = 2020, .month = 04, .day = 20 };

    test_int_error(db_param_bind_date_ref(stmt, 0, &date, NULL), E_DB_UNSUPPORTED_TYPE);
}

TEST_CASE_FIX(db_param_bind_date_ref_oob, db_prepare, db_finalize)
{
    db_date_st date = { .year = 2020, .month = 04, .day = 20 };

    test_int_error(db_param_bind_date_ref(stmt, 1, &date, NULL), E_DB_OUT_OF_BOUNDS);
}

TEST_CASE_FIX(db_param_bind_date_ref, db_prepare, db_finalize)
{
    db_date_st date = { .year = 2020, .month = 04, .day = 20 };

    test_int_success(db_param_bind_date_ref(stmt, 0, &date, NULL));

    date.year++;
    test_sql("select '2021-04-20';");

    date.month++;
    test_sql("select '2021-05-20';");
}

TEST_CASE_FIX(db_param_bind_date_ref_null, db_prepare, db_finalize)
{
    db_date_st date = { .year = 2020, .month = 04, .day = 20 };
    bool null       = false;

    test_int_success(db_param_bind_date_ref(stmt, 0, &date, &null));

    null = true;
    test_sql("select NULL;");

    null = false;
    test_sql("select '2020-04-20';");
}

int test_suite_db_param_bind_date_ref(void *vparam)
{
    param = vparam;

    if(!param->supported)
        return error_pass_int(test_run_case(
            test_case(db_param_bind_date_ref_unsupported)));

    return error_pass_int(test_run_cases(NULL,
        test_case(db_param_bind_date_ref_oob),
        test_case(db_param_bind_date_ref),
        test_case(db_param_bind_date_ref_null),
        NULL
    ));
}

int test_suite_db_param_bind_date(void *vparam)
{
    return error_pass_int(test_run_suites(NULL,
        test_suite_p(db_param_bind_date_fix, vparam),
        test_suite_p(db_param_bind_date_tmp, vparam),
        test_suite_p(db_param_bind_date_ref, vparam),
        NULL
    ));
}

TEST_CASE_FIX(db_param_bind_time_fix_unsupported, db_prepare, db_finalize)
{
    db_time_st time = { .hour = 20, .minute = 04, .second = 20 };

    test_int_error(db_param_bind_time(stmt, 0, &time), E_DB_UNSUPPORTED_TYPE);
}

TEST_CASE_FIX(db_param_bind_time_fix_oob, db_prepare, db_finalize)
{
    db_time_st time = { .hour = 20, .minute = 04, .second = 20 };

    test_int_error(db_param_bind_time(stmt, 1, &time), E_DB_OUT_OF_BOUNDS);
}

TEST_CASE_FIX(db_param_bind_time_fix, db_prepare, db_finalize)
{
    db_time_st time = { .hour = 20, .minute = 04, .second = 20 };

    test_int_success(db_param_bind_time(stmt, 0, &time));
    test_sql("select '20:04:20';");
}

int test_suite_db_param_bind_time_fix(void *vparam)
{
    param = vparam;

    if(!param->supported)
        return error_pass_int(test_run_case(
            test_case(db_param_bind_time_fix_unsupported)));

    return error_pass_int(test_run_cases(NULL,
        test_case(db_param_bind_time_fix_oob),
        test_case(db_param_bind_time_fix),
        NULL
    ));
}

TEST_CASE_FIX(db_param_bind_time_tmp_unsupported, db_prepare, db_finalize)
{
    db_time_st time = { .hour = 20, .minute = 04, .second = 20 };

    test_int_error(db_param_bind_time_tmp(stmt, 0, &time), E_DB_UNSUPPORTED_TYPE);
}

TEST_CASE_FIX(db_param_bind_time_tmp_oob, db_prepare, db_finalize)
{
    db_time_st time = { .hour = 20, .minute = 04, .second = 20 };

    test_int_error(db_param_bind_time_tmp(stmt, 1, &time), E_DB_OUT_OF_BOUNDS);
}

TEST_CASE_FIX(db_param_bind_time_tmp, db_prepare, db_finalize)
{
    db_time_st time = { .hour = 20, .minute = 04, .second = 20 };

    test_int_success(db_param_bind_time_tmp(stmt, 0, &time));

    time.hour++;
    test_sql("select '20:04:20';");
}

int test_suite_db_param_bind_time_tmp(void *vparam)
{
    param = vparam;

    if(!param->supported)
        return error_pass_int(test_run_case(
            test_case(db_param_bind_time_tmp_unsupported)));

    return error_pass_int(test_run_cases(NULL,
        test_case(db_param_bind_time_tmp_oob),
        test_case(db_param_bind_time_tmp),
        NULL
    ));
}

TEST_CASE_FIX(db_param_bind_time_ref_unsupported, db_prepare, db_finalize)
{
    db_time_st time = { .hour = 20, .minute = 04, .second = 20 };

    test_int_error(db_param_bind_time_ref(stmt, 0, &time, NULL), E_DB_UNSUPPORTED_TYPE);
}

TEST_CASE_FIX(db_param_bind_time_ref_oob, db_prepare, db_finalize)
{
    db_time_st time = { .hour = 20, .minute = 04, .second = 20 };

    test_int_error(db_param_bind_time_ref(stmt, 1, &time, NULL), E_DB_OUT_OF_BOUNDS);
}

TEST_CASE_FIX(db_param_bind_time_ref, db_prepare, db_finalize)
{
    db_time_st time = { .hour = 20, .minute = 04, .second = 20 };

    test_int_success(db_param_bind_time_ref(stmt, 0, &time, NULL));

    time.hour++;
    test_sql("select '21:04:20';");

    time.minute++;
    test_sql("select '21:05:20';");
}

TEST_CASE_FIX(db_param_bind_time_ref_null, db_prepare, db_finalize)
{
    db_time_st time = { .hour = 20, .minute = 04, .second = 20 };
    bool null       = false;

    test_int_success(db_param_bind_time_ref(stmt, 0, &time, &null));

    null = true;
    test_sql("select NULL;");

    null = false;
    test_sql("select '20:04:20';");
}

int test_suite_db_param_bind_time_ref(void *vparam)
{
    param = vparam;

    if(!param->supported)
        return error_pass_int(test_run_case(
            test_case(db_param_bind_time_ref_unsupported)));

    return error_pass_int(test_run_cases(NULL,
        test_case(db_param_bind_time_ref_oob),
        test_case(db_param_bind_time_ref),
        test_case(db_param_bind_time_ref_null),
        NULL
    ));
}

int test_suite_db_param_bind_time(void *vparam)
{
    return error_pass_int(test_run_suites(NULL,
        test_suite_p(db_param_bind_time_fix, vparam),
        test_suite_p(db_param_bind_time_tmp, vparam),
        test_suite_p(db_param_bind_time_ref, vparam),
        NULL
    ));
}

TEST_CASE_FIX(db_param_bind_datetime_fix_unsupported, db_prepare, db_finalize)
{
    db_datetime_st dt = { { 2020, 04, 20 }, { 12, 34, 56 } };

    test_int_error(db_param_bind_datetime(stmt, 0, &dt), E_DB_UNSUPPORTED_TYPE);
}

TEST_CASE_FIX(db_param_bind_datetime_fix_oob, db_prepare, db_finalize)
{
    db_datetime_st dt = { { 2020, 04, 20 }, { 12, 34, 56 } };

    test_int_error(db_param_bind_datetime(stmt, 1, &dt), E_DB_OUT_OF_BOUNDS);
}

TEST_CASE_FIX(db_param_bind_datetime_fix, db_prepare, db_finalize)
{
    db_datetime_st dt = { { 2020, 04, 20 }, { 12, 34, 56 } };

    test_int_success(db_param_bind_datetime(stmt, 0, &dt));
    test_sql("select '2020-04-20 12:34:56';");
}

int test_suite_db_param_bind_datetime_fix(void *vparam)
{
    param = vparam;

    if(!param->supported)
        return error_pass_int(test_run_case(
            test_case(db_param_bind_datetime_fix_unsupported)));

    return error_pass_int(test_run_cases(NULL,
        test_case(db_param_bind_datetime_fix_oob),
        test_case(db_param_bind_datetime_fix),
        NULL
    ));
}

TEST_CASE_FIX(db_param_bind_datetime_tmp_unsupported, db_prepare, db_finalize)
{
    db_datetime_st dt = { { 2020, 04, 20 }, { 12, 34, 56 } };

    test_int_error(db_param_bind_datetime_tmp(stmt, 0, &dt), E_DB_UNSUPPORTED_TYPE);
}

TEST_CASE_FIX(db_param_bind_datetime_tmp_oob, db_prepare, db_finalize)
{
    db_datetime_st dt = { { 2020, 04, 20 }, { 12, 34, 56 } };

    test_int_error(db_param_bind_datetime_tmp(stmt, 1, &dt), E_DB_OUT_OF_BOUNDS);
}

TEST_CASE_FIX(db_param_bind_datetime_tmp, db_prepare, db_finalize)
{
    db_datetime_st dt = { { 2020, 04, 20 }, { 12, 34, 56 } };

    test_int_success(db_param_bind_datetime_tmp(stmt, 0, &dt));

    dt.date.year++;
    dt.time.hour++;
    test_sql("select '2020-04-20 12:34:56';");
}

int test_suite_db_param_bind_datetime_tmp(void *vparam)
{
    param = vparam;

    if(!param->supported)
        return error_pass_int(test_run_case(
            test_case(db_param_bind_datetime_tmp_unsupported)));

    return error_pass_int(test_run_cases(NULL,
        test_case(db_param_bind_datetime_tmp_oob),
        test_case(db_param_bind_datetime_tmp),
        NULL
    ));
}

TEST_CASE_FIX(db_param_bind_datetime_ref_unsupported, db_prepare, db_finalize)
{
    db_datetime_st dt = { { 2020, 04, 20 }, { 12, 34, 56 } };

    test_int_error(db_param_bind_datetime_ref(stmt, 0, &dt, NULL), E_DB_UNSUPPORTED_TYPE);
}

TEST_CASE_FIX(db_param_bind_datetime_ref_oob, db_prepare, db_finalize)
{
    db_datetime_st dt = { { 2020, 04, 20 }, { 12, 34, 56 } };

    test_int_error(db_param_bind_datetime_ref(stmt, 1, &dt, NULL), E_DB_OUT_OF_BOUNDS);
}

TEST_CASE_FIX(db_param_bind_datetime_ref, db_prepare, db_finalize)
{
    db_datetime_st dt = { { 2020, 04, 20 }, { 12, 34, 56 } };

    test_int_success(db_param_bind_datetime_ref(stmt, 0, &dt, NULL));

    dt.date.year++;
    dt.time.hour++;
    test_sql("select '2021-04-20 13:34:56';");

    dt.date.month++;
    dt.time.minute++;
    test_sql("select '2021-05-20 13:35:56';");
}

TEST_CASE_FIX(db_param_bind_datetime_ref_null, db_prepare, db_finalize)
{
    db_datetime_st dt   = { { 2020, 04, 20 }, { 12, 34, 56 } };
    bool null           = false;

    test_int_success(db_param_bind_datetime_ref(stmt, 0, &dt, &null));

    null = true;
    test_sql("select NULL;");

    null = false;
    test_sql("select '2020-04-20 12:34:56';");
}

int test_suite_db_param_bind_datetime_ref(void *vparam)
{
    param = vparam;

    if(!param->supported)
        return error_pass_int(test_run_case(
            test_case(db_param_bind_datetime_ref_unsupported)));

    return error_pass_int(test_run_cases(NULL,
        test_case(db_param_bind_datetime_ref_oob),
        test_case(db_param_bind_datetime_ref),
        test_case(db_param_bind_datetime_ref_null),
        NULL
    ));
}

int test_suite_db_param_bind_datetime(void *vparam)
{
    return error_pass_int(test_run_suites(NULL,
        test_suite_p(db_param_bind_datetime_fix, vparam),
        test_suite_p(db_param_bind_datetime_tmp, vparam),
        test_suite_p(db_param_bind_datetime_ref, vparam),
        NULL
    ));
}

TEST_CASE_FIX(db_param_bind_timestamp_tmp_unsupported, db_prepare, db_finalize)
{
    test_int_error(db_param_bind_timestamp(stmt, 0, 0), E_DB_UNSUPPORTED_TYPE);
}

TEST_CASE_FIX(db_param_bind_timestamp_tmp_oob, db_prepare, db_finalize)
{
    test_int_error(db_param_bind_timestamp(stmt, 1, 0), E_DB_OUT_OF_BOUNDS);
}

TEST_CASE_FIX(db_param_bind_timestamp_tmp, db_prepare, db_finalize)
{
    test_int_success(db_param_bind_timestamp(stmt, 0, 1587386096));
    test_sql("select '2020-04-20 12:34:56';");
}

int test_suite_db_param_bind_timestamp_tmp(void *vparam)
{
    param = vparam;

    if(!param->supported)
        return error_pass_int(test_run_case(
            test_case(db_param_bind_timestamp_tmp_unsupported)));

    return error_pass_int(test_run_cases(NULL,
        test_case(db_param_bind_timestamp_tmp_oob),
        test_case(db_param_bind_timestamp_tmp),
        NULL
    ));
}

TEST_CASE_FIX(db_param_bind_timestamp_ref_unsupported, db_prepare, db_finalize)
{
    time_t ts = 0;

    test_int_error(db_param_bind_timestamp_ref(stmt, 0, &ts, NULL), E_DB_UNSUPPORTED_TYPE);
}

TEST_CASE_FIX(db_param_bind_timestamp_ref_oob, db_prepare, db_finalize)
{
    time_t ts = 0;

    test_int_error(db_param_bind_timestamp_ref(stmt, 1, &ts, NULL), E_DB_OUT_OF_BOUNDS);
}

TEST_CASE_FIX(db_param_bind_timestamp_ref, db_prepare, db_finalize)
{
    time_t ts = 0;

    test_int_success(db_param_bind_timestamp_ref(stmt, 0, &ts, NULL));

    ts = 1587386096;
    test_sql("select '2020-04-20 12:34:56';");

    ts = 1;
    test_sql("select '1970-01-01 00:00:01';");
}

TEST_CASE_FIX(db_param_bind_timestamp_ref_null, db_prepare, db_finalize)
{
    time_t ts   = 1587386096;
    bool null   = false;

    test_int_success(db_param_bind_timestamp_ref(stmt, 0, &ts, &null));

    null = true;
    test_sql("select NULL;");

    null = false;
    test_sql("select '2020-04-20 12:34:56';");
}

int test_suite_db_param_bind_timestamp_ref(void *vparam)
{
    param = vparam;

    if(!param->supported)
        return error_pass_int(test_run_case(
            test_case(db_param_bind_timestamp_ref_unsupported)));

    return error_pass_int(test_run_cases(NULL,
        test_case(db_param_bind_timestamp_ref_oob),
        test_case(db_param_bind_timestamp_ref),
        test_case(db_param_bind_timestamp_ref_null),
        NULL
    ));
}

int test_suite_db_param_bind_timestamp(void *vparam)
{
    return error_pass_int(test_run_suites(NULL,
        test_suite_p(db_param_bind_timestamp_tmp, vparam),
        test_suite_p(db_param_bind_timestamp_ref, vparam),
        NULL
    ));
}
