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
#include <ytil/ext/string.h>
#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>

static test_suite_db_st *suite;
static db_ct db;
static db_stmt_ct stmt;


TEST_SETUP(db_open)
{
    test_ptr_success(db = suite->db_open());
}

TEST_SETUP(db_prepare)
{
    test_setup(db_open);
    test_ptr_success(stmt = db_prepare(db, "select ?;"));
}

TEST_SETUP(db_prepare_null)
{
    test_setup(db_open);
    test_ptr_success(stmt = db_prepare(db, "select NULL;"));
}

TEST_PSETUP(db_prepare, const char *sql)
{
    test_setup(db_open);
    test_ptr_success(stmt = db_prepare(db, sql));
}

TEST_PSETUP(db_prepare_table, const char *sql)
{
    test_setup(db_open);

    test_ptr_success(stmt = db_prepare(db, "create temporary table tmp as select 123 as i;"));
    test_int_success(db_exec(stmt));
    test_int_success(db_finalize(stmt));

    test_ptr_success(stmt = db_prepare(db, sql));
}

TEST_TEARDOWN(db_finalize)
{
    test_int_success(db_finalize(stmt));
    test_int_success(db_close(db));
}

TEST_CASE_FIX(db_result_count_unsupported, db_prepare, db_finalize)
{
    test_rc_error(db_result_count(stmt), -1, E_DB_UNSUPPORTED);
}

TEST_CASE_FIX(db_result_count, db_prepare, db_finalize)
{
    test_rc_success(db_result_count(stmt), 1, -1);
}

int test_suite_db_result_count(void *param)
{
    suite = param;

    if(!suite->supported)
        return error_pass_int(test_run_case(
            test_case(db_result_count_unsupported)));

    return error_pass_int(test_run_cases(NULL,
        test_case(db_result_count),
        NULL
    ));
}

TEST_CASE_FIX(db_result_bind_bool_unsupported, db_prepare, db_finalize)
{
    bool value  = false;
    bool null   = false;

    test_int_error(db_result_bind_bool(stmt, 0, &value, &null), E_DB_UNSUPPORTED_TYPE);
}

TEST_CASE_FIX(db_result_bind_bool_oob, db_prepare, db_finalize)
{
    bool value  = false;
    bool null   = false;

    test_int_error(db_result_bind_bool(stmt, 1, &value, &null), E_DB_OUT_OF_BOUNDS);
}

TEST_CASE_FIX(db_result_bind_bool_true, db_prepare, db_finalize)
{
    bool value  = false;
    bool null   = true;

    test_int_success(db_param_bind_bool(stmt, 0, true));
    test_int_success(db_result_bind_bool(stmt, 0, &value, &null));
    test_int_success(db_exec(stmt));

    test_false(null);
    test_true(value);
}

TEST_CASE_FIX(db_result_bind_bool_false, db_prepare, db_finalize)
{
    bool value  = true;
    bool null   = true;

    test_int_success(db_param_bind_bool(stmt, 0, false));
    test_int_success(db_result_bind_bool(stmt, 0, &value, &null));
    test_int_success(db_exec(stmt));

    test_false(null);
    test_false(value);
}

TEST_CASE_FIX(db_result_bind_bool_null, db_prepare_null, db_finalize)
{
    bool value  = false;
    bool null   = false;

    test_int_success(db_result_bind_bool(stmt, 0, &value, &null));
    test_int_success(db_exec(stmt));

    test_true(null);
}

TEST_CASE_FIX(db_result_bind_bool_no_null, db_prepare, db_finalize)
{
    bool value = false;

    test_int_success(db_param_bind_bool(stmt, 0, true));
    test_int_success(db_result_bind_bool(stmt, 0, &value, NULL));
    test_int_success(db_exec(stmt));

    test_true(value);
}

int test_suite_db_result_bind_bool(void *param)
{
    suite = param;

    if(!suite->supported)
        return error_pass_int(test_run_case(
            test_case(db_result_bind_bool_unsupported)));

    return error_pass_int(test_run_cases(NULL,
        test_case(db_result_bind_bool_oob),
        test_case(db_result_bind_bool_true),
        test_case(db_result_bind_bool_false),
        test_case(db_result_bind_bool_null),
        test_case(db_result_bind_bool_no_null),
        NULL
    ));
}

TEST_CASE_FIX(db_result_bind_sint8_unsupported, db_prepare, db_finalize)
{
    int8_t value    = 123;
    bool null       = false;

    test_int_error(db_result_bind_int8(stmt, 0, &value, &null), E_DB_UNSUPPORTED_TYPE);
}

TEST_CASE_FIX(db_result_bind_sint8_oob, db_prepare, db_finalize)
{
    int8_t value    = 123;
    bool null       = false;

    test_int_error(db_result_bind_int8(stmt, 1, &value, &null), E_DB_OUT_OF_BOUNDS);
}

TEST_CASE_FIX(db_result_bind_sint8_min, db_prepare, db_finalize)
{
    int8_t value    = 123;
    bool null       = true;

    test_int_success(db_param_bind_int8(stmt, 0, INT8_MIN));
    test_int_success(db_result_bind_int8(stmt, 0, &value, &null));
    test_int_success(db_exec(stmt));

    test_false(null);
    test_int_eq(value, INT8_MIN);
}

TEST_CASE_FIX(db_result_bind_sint8_max, db_prepare, db_finalize)
{
    int8_t value    = 123;
    bool null       = true;

    test_int_success(db_param_bind_int8(stmt, 0, INT8_MAX));
    test_int_success(db_result_bind_int8(stmt, 0, &value, &null));
    test_int_success(db_exec(stmt));

    test_false(null);
    test_int_eq(value, INT8_MAX);
}

TEST_CASE_FIX(db_result_bind_sint8_null, db_prepare_null, db_finalize)
{
    int8_t value    = 123;
    bool null       = false;

    test_int_success(db_result_bind_int8(stmt, 0, &value, &null));
    test_int_success(db_exec(stmt));

    test_true(null);
}

TEST_CASE_FIX(db_result_bind_sint8_no_null, db_prepare, db_finalize)
{
    int8_t value = 0;

    test_int_success(db_param_bind_int8(stmt, 0, 123));
    test_int_success(db_result_bind_int8(stmt, 0, &value, NULL));
    test_int_success(db_exec(stmt));

    test_int_eq(value, 123);
}

int test_suite_db_result_bind_sint8(void *param)
{
    suite = param;

    if(!suite->supported)
        return error_pass_int(test_run_case(
            test_case(db_result_bind_sint8_unsupported)));

    return error_pass_int(test_run_cases(NULL,
        test_case(db_result_bind_sint8_oob),
        test_case(db_result_bind_sint8_min),
        test_case(db_result_bind_sint8_max),
        test_case(db_result_bind_sint8_null),
        test_case(db_result_bind_sint8_no_null),
        NULL
    ));
}

TEST_CASE_FIX(db_result_bind_uint8_unsupported, db_prepare, db_finalize)
{
    uint8_t value   = 123;
    bool null       = false;

    test_int_error(db_result_bind_uint8(stmt, 0, &value, &null), E_DB_UNSUPPORTED_TYPE);
}

TEST_CASE_FIX(db_result_bind_uint8_oob, db_prepare, db_finalize)
{
    uint8_t value   = 123;
    bool null       = false;

    test_int_error(db_result_bind_uint8(stmt, 1, &value, &null), E_DB_OUT_OF_BOUNDS);
}

TEST_CASE_FIX(db_result_bind_uint8_min, db_prepare, db_finalize)
{
    uint8_t value   = 123;
    bool null       = true;

    test_int_success(db_param_bind_uint8(stmt, 0, 0));
    test_int_success(db_result_bind_uint8(stmt, 0, &value, &null));
    test_int_success(db_exec(stmt));

    test_false(null);
    test_uint_eq(value, 0);
}

TEST_CASE_FIX(db_result_bind_uint8_max, db_prepare, db_finalize)
{
    uint8_t value   = 123;
    bool null       = true;

    test_int_success(db_param_bind_uint8(stmt, 0, UINT8_MAX));
    test_int_success(db_result_bind_uint8(stmt, 0, &value, &null));
    test_int_success(db_exec(stmt));

    test_false(null);
    test_int_eq(value, UINT8_MAX);
}

TEST_CASE_FIX(db_result_bind_uint8_null, db_prepare_null, db_finalize)
{
    uint8_t value   = 123;
    bool null       = false;

    test_int_success(db_result_bind_uint8(stmt, 0, &value, &null));
    test_int_success(db_exec(stmt));

    test_true(null);
}

TEST_CASE_FIX(db_result_bind_uint8_no_null, db_prepare, db_finalize)
{
    uint8_t value = 0;

    test_int_success(db_param_bind_uint8(stmt, 0, 123));
    test_int_success(db_result_bind_uint8(stmt, 0, &value, NULL));
    test_int_success(db_exec(stmt));

    test_uint_eq(value, 123);
}

int test_suite_db_result_bind_uint8(void *param)
{
    suite = param;

    if(!suite->supported)
        return error_pass_int(test_run_case(
            test_case(db_result_bind_uint8_unsupported)));

    return error_pass_int(test_run_cases(NULL,
        test_case(db_result_bind_uint8_oob),
        test_case(db_result_bind_uint8_min),
        test_case(db_result_bind_uint8_max),
        test_case(db_result_bind_uint8_null),
        test_case(db_result_bind_uint8_no_null),
        NULL
    ));
}

int test_suite_db_result_bind_int8(void *param)
{
    return error_pass_int(test_run_suites(NULL,
        test_suite_p(db_result_bind_sint8, param),
        test_suite_p(db_result_bind_uint8, param),
        NULL
    ));
}

TEST_CASE_FIX(db_result_bind_sint16_unsupported, db_prepare, db_finalize)
{
    int16_t value   = 123;
    bool null       = false;

    test_int_error(db_result_bind_int16(stmt, 0, &value, &null), E_DB_UNSUPPORTED_TYPE);
}

TEST_CASE_FIX(db_result_bind_sint16_oob, db_prepare, db_finalize)
{
    int16_t value   = 123;
    bool null       = false;

    test_int_error(db_result_bind_int16(stmt, 1, &value, &null), E_DB_OUT_OF_BOUNDS);
}

TEST_CASE_FIX(db_result_bind_sint16_min, db_prepare, db_finalize)
{
    int16_t value   = 123;
    bool null       = true;

    test_int_success(db_param_bind_int16(stmt, 0, INT16_MIN));
    test_int_success(db_result_bind_int16(stmt, 0, &value, &null));
    test_int_success(db_exec(stmt));

    test_false(null);
    test_int_eq(value, INT16_MIN);
}

TEST_CASE_FIX(db_result_bind_sint16_max, db_prepare, db_finalize)
{
    int16_t value   = 123;
    bool null       = true;

    test_int_success(db_param_bind_int16(stmt, 0, INT16_MAX));
    test_int_success(db_result_bind_int16(stmt, 0, &value, &null));
    test_int_success(db_exec(stmt));

    test_false(null);
    test_int_eq(value, INT16_MAX);
}

TEST_CASE_FIX(db_result_bind_sint16_null, db_prepare_null, db_finalize)
{
    int16_t value   = 123;
    bool null       = false;

    test_int_success(db_result_bind_int16(stmt, 0, &value, &null));
    test_int_success(db_exec(stmt));

    test_true(null);
}

TEST_CASE_FIX(db_result_bind_sint16_no_null, db_prepare, db_finalize)
{
    int16_t value = 0;

    test_int_success(db_param_bind_int16(stmt, 0, 123));
    test_int_success(db_result_bind_int16(stmt, 0, &value, NULL));
    test_int_success(db_exec(stmt));

    test_int_eq(value, 123);
}

int test_suite_db_result_bind_sint16(void *param)
{
    suite = param;

    if(!suite->supported)
        return error_pass_int(test_run_case(
            test_case(db_result_bind_sint16_unsupported)));

    return error_pass_int(test_run_cases(NULL,
        test_case(db_result_bind_sint16_oob),
        test_case(db_result_bind_sint16_min),
        test_case(db_result_bind_sint16_max),
        test_case(db_result_bind_sint16_null),
        test_case(db_result_bind_sint16_no_null),
        NULL
    ));
}

TEST_CASE_FIX(db_result_bind_uint16_unsupported, db_prepare, db_finalize)
{
    uint16_t value  = 123;
    bool null       = false;

    test_int_error(db_result_bind_uint16(stmt, 0, &value, &null), E_DB_UNSUPPORTED_TYPE);
}

TEST_CASE_FIX(db_result_bind_uint16_oob, db_prepare, db_finalize)
{
    uint16_t value  = 123;
    bool null       = false;

    test_int_error(db_result_bind_uint16(stmt, 1, &value, &null), E_DB_OUT_OF_BOUNDS);
}

TEST_CASE_FIX(db_result_bind_uint16_min, db_prepare, db_finalize)
{
    uint16_t value  = 123;
    bool null       = true;

    test_int_success(db_param_bind_uint16(stmt, 0, 0));
    test_int_success(db_result_bind_uint16(stmt, 0, &value, &null));
    test_int_success(db_exec(stmt));

    test_false(null);
    test_uint_eq(value, 0);
}

TEST_CASE_FIX(db_result_bind_uint16_max, db_prepare, db_finalize)
{
    uint16_t value  = 123;
    bool null       = true;

    test_int_success(db_param_bind_uint16(stmt, 0, UINT16_MAX));
    test_int_success(db_result_bind_uint16(stmt, 0, &value, &null));
    test_int_success(db_exec(stmt));

    test_false(null);
    test_int_eq(value, UINT16_MAX);
}

TEST_CASE_FIX(db_result_bind_uint16_null, db_prepare_null, db_finalize)
{
    uint16_t value  = 123;
    bool null       = false;

    test_int_success(db_result_bind_uint16(stmt, 0, &value, &null));
    test_int_success(db_exec(stmt));

    test_true(null);
}

TEST_CASE_FIX(db_result_bind_uint16_no_null, db_prepare, db_finalize)
{
    uint16_t value = 0;

    test_int_success(db_param_bind_uint16(stmt, 0, 123));
    test_int_success(db_result_bind_uint16(stmt, 0, &value, NULL));
    test_int_success(db_exec(stmt));

    test_uint_eq(value, 123);
}

int test_suite_db_result_bind_uint16(void *param)
{
    suite = param;

    if(!suite->supported)
        return error_pass_int(test_run_case(
            test_case(db_result_bind_uint16_unsupported)));

    return error_pass_int(test_run_cases(NULL,
        test_case(db_result_bind_uint16_oob),
        test_case(db_result_bind_uint16_min),
        test_case(db_result_bind_uint16_max),
        test_case(db_result_bind_uint16_null),
        test_case(db_result_bind_uint16_no_null),
        NULL
    ));
}

int test_suite_db_result_bind_int16(void *param)
{
    return error_pass_int(test_run_suites(NULL,
        test_suite_p(db_result_bind_sint16, param),
        test_suite_p(db_result_bind_uint16, param),
        NULL
    ));
}

TEST_CASE_FIX(db_result_bind_sint32_unsupported, db_prepare, db_finalize)
{
    int32_t value   = 123;
    bool null       = false;

    test_int_error(db_result_bind_int32(stmt, 0, &value, &null), E_DB_UNSUPPORTED_TYPE);
}

TEST_CASE_FIX(db_result_bind_sint32_oob, db_prepare, db_finalize)
{
    int32_t value   = 123;
    bool null       = false;

    test_int_error(db_result_bind_int32(stmt, 1, &value, &null), E_DB_OUT_OF_BOUNDS);
}

TEST_CASE_FIX(db_result_bind_sint32_min, db_prepare, db_finalize)
{
    int32_t value   = 123;
    bool null       = true;

    test_int_success(db_param_bind_int32(stmt, 0, INT32_MIN));
    test_int_success(db_result_bind_int32(stmt, 0, &value, &null));
    test_int_success(db_exec(stmt));

    test_false(null);
    test_int_eq(value, INT32_MIN);
}

TEST_CASE_FIX(db_result_bind_sint32_max, db_prepare, db_finalize)
{
    int32_t value   = 123;
    bool null       = true;

    test_int_success(db_param_bind_int32(stmt, 0, INT32_MAX));
    test_int_success(db_result_bind_int32(stmt, 0, &value, &null));
    test_int_success(db_exec(stmt));

    test_false(null);
    test_int_eq(value, INT32_MAX);
}

TEST_CASE_FIX(db_result_bind_sint32_null, db_prepare_null, db_finalize)
{
    int32_t value   = 123;
    bool null       = false;

    test_int_success(db_result_bind_int32(stmt, 0, &value, &null));
    test_int_success(db_exec(stmt));

    test_true(null);
}

TEST_CASE_FIX(db_result_bind_sint32_no_null, db_prepare, db_finalize)
{
    int32_t value = 0;

    test_int_success(db_param_bind_int32(stmt, 0, 123));
    test_int_success(db_result_bind_int32(stmt, 0, &value, NULL));
    test_int_success(db_exec(stmt));

    test_int_eq(value, 123);
}

int test_suite_db_result_bind_sint32(void *param)
{
    suite = param;

    if(!suite->supported)
        return error_pass_int(test_run_case(
            test_case(db_result_bind_sint32_unsupported)));

    return error_pass_int(test_run_cases(NULL,
        test_case(db_result_bind_sint32_oob),
        test_case(db_result_bind_sint32_min),
        test_case(db_result_bind_sint32_max),
        test_case(db_result_bind_sint32_null),
        test_case(db_result_bind_sint32_no_null),
        NULL
    ));
}

TEST_CASE_FIX(db_result_bind_uint32_unsupported, db_prepare, db_finalize)
{
    uint32_t value  = 123;
    bool null       = false;

    test_int_error(db_result_bind_uint32(stmt, 0, &value, &null), E_DB_UNSUPPORTED_TYPE);
}

TEST_CASE_FIX(db_result_bind_uint32_oob, db_prepare, db_finalize)
{
    uint32_t value  = 123;
    bool null       = false;

    test_int_error(db_result_bind_uint32(stmt, 1, &value, &null), E_DB_OUT_OF_BOUNDS);
}

TEST_CASE_FIX(db_result_bind_uint32_min, db_prepare, db_finalize)
{
    uint32_t value  = 123;
    bool null       = true;

    test_int_success(db_param_bind_uint32(stmt, 0, 0));
    test_int_success(db_result_bind_uint32(stmt, 0, &value, &null));
    test_int_success(db_exec(stmt));

    test_false(null);
    test_uint_eq(value, 0);
}

TEST_CASE_FIX(db_result_bind_uint32_max, db_prepare, db_finalize)
{
    uint32_t value  = 123;
    bool null       = true;

    test_int_success(db_param_bind_uint32(stmt, 0, UINT32_MAX));
    test_int_success(db_result_bind_uint32(stmt, 0, &value, &null));
    test_int_success(db_exec(stmt));

    test_false(null);
    test_int_eq(value, UINT32_MAX);
}

TEST_CASE_FIX(db_result_bind_uint32_null, db_prepare_null, db_finalize)
{
    uint32_t value  = 123;
    bool null       = false;

    test_int_success(db_result_bind_uint32(stmt, 0, &value, &null));
    test_int_success(db_exec(stmt));

    test_true(null);
}

TEST_CASE_FIX(db_result_bind_uint32_no_null, db_prepare, db_finalize)
{
    uint32_t value = 0;

    test_int_success(db_param_bind_uint32(stmt, 0, 123));
    test_int_success(db_result_bind_uint32(stmt, 0, &value, NULL));
    test_int_success(db_exec(stmt));

    test_uint_eq(value, 123);
}

int test_suite_db_result_bind_uint32(void *param)
{
    suite = param;

    if(!suite->supported)
        return error_pass_int(test_run_case(
            test_case(db_result_bind_uint32_unsupported)));

    return error_pass_int(test_run_cases(NULL,
        test_case(db_result_bind_uint32_oob),
        test_case(db_result_bind_uint32_min),
        test_case(db_result_bind_uint32_max),
        test_case(db_result_bind_uint32_null),
        test_case(db_result_bind_uint32_no_null),
        NULL
    ));
}

int test_suite_db_result_bind_int32(void *param)
{
    return error_pass_int(test_run_suites(NULL,
        test_suite_p(db_result_bind_sint32, param),
        test_suite_p(db_result_bind_uint32, param),
        NULL
    ));
}

TEST_CASE_FIX(db_result_bind_sint64_unsupported, db_prepare, db_finalize)
{
    int64_t value   = 123;
    bool null       = false;

    test_int_error(db_result_bind_int64(stmt, 0, &value, &null), E_DB_UNSUPPORTED_TYPE);
}

TEST_CASE_FIX(db_result_bind_sint64_oob, db_prepare, db_finalize)
{
    int64_t value   = 123;
    bool null       = false;

    test_int_error(db_result_bind_int64(stmt, 1, &value, &null), E_DB_OUT_OF_BOUNDS);
}

TEST_CASE_FIX(db_result_bind_sint64_min, db_prepare, db_finalize)
{
    int64_t value   = 123;
    bool null       = true;

    test_int_success(db_param_bind_int64(stmt, 0, INT64_MIN));
    test_int_success(db_result_bind_int64(stmt, 0, &value, &null));
    test_int_success(db_exec(stmt));

    test_false(null);
    test_int_eq(value, INT64_MIN);
}

TEST_CASE_FIX(db_result_bind_sint64_max, db_prepare, db_finalize)
{
    int64_t value   = 123;
    bool null       = true;

    test_int_success(db_param_bind_int64(stmt, 0, INT64_MAX));
    test_int_success(db_result_bind_int64(stmt, 0, &value, &null));
    test_int_success(db_exec(stmt));

    test_false(null);
    test_int_eq(value, INT64_MAX);
}

TEST_CASE_FIX(db_result_bind_sint64_null, db_prepare_null, db_finalize)
{
    int64_t value   = 123;
    bool null       = false;

    test_int_success(db_result_bind_int64(stmt, 0, &value, &null));
    test_int_success(db_exec(stmt));

    test_true(null);
}

TEST_CASE_FIX(db_result_bind_sint64_no_null, db_prepare, db_finalize)
{
    int64_t value = 0;

    test_int_success(db_param_bind_int64(stmt, 0, 123));
    test_int_success(db_result_bind_int64(stmt, 0, &value, NULL));
    test_int_success(db_exec(stmt));

    test_int_eq(value, 123);
}

int test_suite_db_result_bind_sint64(void *param)
{
    suite = param;

    if(!suite->supported)
        return error_pass_int(test_run_case(
            test_case(db_result_bind_sint64_unsupported)));

    return error_pass_int(test_run_cases(NULL,
        test_case(db_result_bind_sint64_oob),
        test_case(db_result_bind_sint64_min),
        test_case(db_result_bind_sint64_max),
        test_case(db_result_bind_sint64_null),
        test_case(db_result_bind_sint64_no_null),
        NULL
    ));
}

TEST_CASE_FIX(db_result_bind_uint64_unsupported, db_prepare, db_finalize)
{
    uint64_t value  = 123;
    bool null       = false;

    test_int_error(db_result_bind_uint64(stmt, 0, &value, &null), E_DB_UNSUPPORTED_TYPE);
}

TEST_CASE_FIX(db_result_bind_uint64_oob, db_prepare, db_finalize)
{
    uint64_t value  = 123;
    bool null       = false;

    test_int_error(db_result_bind_uint64(stmt, 1, &value, &null), E_DB_OUT_OF_BOUNDS);
}

TEST_CASE_FIX(db_result_bind_uint64_min, db_prepare, db_finalize)
{
    uint64_t value  = 123;
    bool null       = true;

    test_int_success(db_param_bind_uint64(stmt, 0, 0));
    test_int_success(db_result_bind_uint64(stmt, 0, &value, &null));
    test_int_success(db_exec(stmt));

    test_false(null);
    test_uint_eq(value, 0);
}

TEST_CASE_FIX(db_result_bind_uint64_max, db_prepare, db_finalize)
{
    uint64_t value  = 123;
    bool null       = true;

    test_int_success(db_param_bind_uint64(stmt, 0, UINT64_MAX));
    test_int_success(db_result_bind_uint64(stmt, 0, &value, &null));
    test_int_success(db_exec(stmt));

    test_false(null);
    test_int_eq(value, UINT64_MAX);
}

TEST_CASE_FIX(db_result_bind_uint64_null, db_prepare_null, db_finalize)
{
    uint64_t value  = 123;
    bool null       = false;

    test_int_success(db_result_bind_uint64(stmt, 0, &value, &null));
    test_int_success(db_exec(stmt));

    test_true(null);
}

TEST_CASE_FIX(db_result_bind_uint64_no_null, db_prepare, db_finalize)
{
    uint64_t value = 0;

    test_int_success(db_param_bind_uint64(stmt, 0, 123));
    test_int_success(db_result_bind_uint64(stmt, 0, &value, NULL));
    test_int_success(db_exec(stmt));

    test_uint_eq(value, 123);
}

int test_suite_db_result_bind_uint64(void *param)
{
    suite = param;

    if(!suite->supported)
        return error_pass_int(test_run_case(
            test_case(db_result_bind_uint64_unsupported)));

    return error_pass_int(test_run_cases(NULL,
        test_case(db_result_bind_uint64_oob),
        test_case(db_result_bind_uint64_min),
        test_case(db_result_bind_uint64_max),
        test_case(db_result_bind_uint64_null),
        test_case(db_result_bind_uint64_no_null),
        NULL
    ));
}

int test_suite_db_result_bind_int64(void *param)
{
    return error_pass_int(test_run_suites(NULL,
        test_suite_p(db_result_bind_sint64, param),
        test_suite_p(db_result_bind_uint64, param),
        NULL
    ));
}

TEST_CASE_FIX(db_result_bind_float_unsupported, db_prepare, db_finalize)
{
    float value = 123;
    bool null   = false;

    test_int_error(db_result_bind_float(stmt, 0, &value, &null), E_DB_UNSUPPORTED_TYPE);
}

TEST_CASE_FIX(db_result_bind_float_oob, db_prepare, db_finalize)
{
    float value = 123;
    bool null   = false;

    test_int_error(db_result_bind_float(stmt, 1, &value, &null), E_DB_OUT_OF_BOUNDS);
}

TEST_CASE_FIX(db_result_bind_float, db_prepare, db_finalize)
{
    float value = 123;
    bool null   = true;

    test_int_success(db_param_bind_float(stmt, 0, 123456.789012));
    test_int_success(db_result_bind_float(stmt, 0, &value, &null));
    test_int_success(db_exec(stmt));

    test_false(null);
    test_float_eq(value, 123456.789012);
}

TEST_CASE_FIX(db_result_bind_float_nan, db_prepare, db_finalize)
{
    float value = 123;
    bool null   = true;

    test_int_success(db_param_bind_float(stmt, 0, NAN));
    test_int_success(db_result_bind_float(stmt, 0, &value, &null));
    test_int_success(db_exec(stmt));

    test_true(null); // NULL == NAN ?
    test_float_is_nan(value);
}

TEST_CASE_FIX(db_result_bind_float_inf, db_prepare, db_finalize)
{
    float value = 123;
    bool null   = true;

    test_int_success(db_param_bind_float(stmt, 0, INFINITY));
    test_int_success(db_result_bind_float(stmt, 0, &value, &null));
    test_int_success(db_exec(stmt));

    test_false(null);
    test_float_is_infinite(value);
}

TEST_CASE_FIX(db_result_bind_float_null, db_prepare_null, db_finalize)
{
    float value = 123;
    bool null   = false;

    test_int_success(db_result_bind_float(stmt, 0, &value, &null));
    test_int_success(db_exec(stmt));

    test_true(null);
}

TEST_CASE_FIX(db_result_bind_float_no_null, db_prepare, db_finalize)
{
    float value = 123;

    test_int_success(db_param_bind_float(stmt, 0, 123456.789012));
    test_int_success(db_result_bind_float(stmt, 0, &value, NULL));
    test_int_success(db_exec(stmt));

    test_float_eq(value, 123456.789012);
}

int test_suite_db_result_bind_float(void *param)
{
    suite = param;

    if(!suite->supported)
        return error_pass_int(test_run_case(
            test_case(db_result_bind_float_unsupported)));

    return error_pass_int(test_run_cases(NULL,
        test_case(db_result_bind_float_oob),
        test_case(db_result_bind_float),
        test_case(db_result_bind_float_nan),
        test_case(db_result_bind_float_inf),
        test_case(db_result_bind_float_null),
        test_case(db_result_bind_float_no_null),
        NULL
    ));
}

TEST_CASE_FIX(db_result_bind_double_unsupported, db_prepare, db_finalize)
{
    double value    = 123;
    bool null       = false;

    test_int_error(db_result_bind_double(stmt, 0, &value, &null), E_DB_UNSUPPORTED_TYPE);
}

TEST_CASE_FIX(db_result_bind_double_oob, db_prepare, db_finalize)
{
    double value    = 123;
    bool null       = false;

    test_int_error(db_result_bind_double(stmt, 1, &value, &null), E_DB_OUT_OF_BOUNDS);
}

TEST_CASE_FIX(db_result_bind_double, db_prepare, db_finalize)
{
    double value    = 123;
    bool null       = true;

    test_int_success(db_param_bind_double(stmt, 0, 123456.789012));
    test_int_success(db_result_bind_double(stmt, 0, &value, &null));
    test_int_success(db_exec(stmt));

    test_false(null);
    test_float_eq(value, 123456.789012);
}

TEST_CASE_FIX(db_result_bind_double_nan, db_prepare, db_finalize)
{
    double value    = 123;
    bool null       = true;

    test_int_success(db_param_bind_double(stmt, 0, NAN));
    test_int_success(db_result_bind_double(stmt, 0, &value, &null));
    test_int_success(db_exec(stmt));

    test_true(null); // NULL == NAN ?
    test_float_is_nan(value);
}

TEST_CASE_FIX(db_result_bind_double_inf, db_prepare, db_finalize)
{
    double value    = 123;
    bool null       = true;

    test_int_success(db_param_bind_double(stmt, 0, INFINITY));
    test_int_success(db_result_bind_double(stmt, 0, &value, &null));
    test_int_success(db_exec(stmt));

    test_false(null);
    test_float_is_infinite(value);
}

TEST_CASE_FIX(db_result_bind_double_null, db_prepare_null, db_finalize)
{
    double value    = 123;
    bool null       = false;

    test_int_success(db_result_bind_double(stmt, 0, &value, &null));
    test_int_success(db_exec(stmt));

    test_true(null);
}

TEST_CASE_FIX(db_result_bind_double_no_null, db_prepare, db_finalize)
{
    double value = 123;

    test_int_success(db_param_bind_double(stmt, 0, 123456.789012));
    test_int_success(db_result_bind_double(stmt, 0, &value, NULL));
    test_int_success(db_exec(stmt));

    test_float_eq(value, 123456.789012);
}

int test_suite_db_result_bind_double(void *param)
{
    suite = param;

    if(!suite->supported)
        return error_pass_int(test_run_case(
            test_case(db_result_bind_double_unsupported)));

    return error_pass_int(test_run_cases(NULL,
        test_case(db_result_bind_double_oob),
        test_case(db_result_bind_double),
        test_case(db_result_bind_double_nan),
        test_case(db_result_bind_double_inf),
        test_case(db_result_bind_double_null),
        test_case(db_result_bind_double_no_null),
        NULL
    ));
}

typedef struct test_db_data
{
    void *data;
    size_t size;
    bool text;
} test_db_data_st;

static int test_db_dup(db_stmt_ct stmt, size_t row, void *ctx)
{
    test_db_data_st *state = ctx;

    if(state->text)
        test_ptr_success(state->data = strdup(state->data));
    else
        test_ptr_success(state->data = memdup(state->data, state->size));

    return 0;
}

TEST_CASE_FIX(db_result_bind_text_tmp_unsupported, db_prepare, db_finalize)
{
    char *text  = NULL;
    size_t size = 0;

    test_int_error(db_result_bind_text(stmt, 0, &text, &size), E_DB_UNSUPPORTED_TYPE);
}

TEST_CASE_FIX(db_result_bind_text_tmp_oob, db_prepare, db_finalize)
{
    char *text  = NULL;
    size_t size = 0;

    test_int_error(db_result_bind_text(stmt, 1, &text, &size), E_DB_OUT_OF_BOUNDS);
}

TEST_CASE_FIX(db_result_bind_text_tmp, db_prepare, db_finalize)
{
    test_db_data_st state = { .data = NULL, .size = 0, .text = true };

    test_int_success(db_param_bind_text(stmt, 0, "foo", -1));
    test_int_success(db_result_bind_text(stmt, 0, (char **)&state.data, &state.size));
    test_int_success(db_exec_f(stmt, test_db_dup, &state));

    test_uint_eq(state.size, 3);
    test_str_eq(state.data, "foo");

    test_void(free(state.data));
}

TEST_CASE_FIX(db_result_bind_text_tmp_null, db_prepare_null, db_finalize)
{
    char *text  = "foo";
    size_t size = 3;

    test_int_success(db_result_bind_text(stmt, 0, &text, &size));
    test_int_success(db_exec(stmt));

    test_uint_eq(size, 0);
    test_ptr_eq(text, NULL);
}

TEST_CASE_FIX(db_result_bind_text_tmp_no_size, db_prepare, db_finalize)
{
    test_db_data_st state = { .data = NULL, .text = true };

    test_int_success(db_param_bind_text(stmt, 0, "foo", -1));
    test_int_success(db_result_bind_text(stmt, 0, (char **)&state.data, NULL));
    test_int_success(db_exec_f(stmt, test_db_dup, &state));

    test_str_eq(state.data, "foo");

    test_void(free(state.data));
}

int test_suite_db_result_bind_text_tmp(void *param)
{
    suite = param;

    if(!suite->supported)
        return error_pass_int(test_run_case(
            test_case(db_result_bind_text_tmp_unsupported)));

    return error_pass_int(test_run_cases(NULL,
        test_case(db_result_bind_text_tmp_oob),
        test_case(db_result_bind_text_tmp),
        test_case(db_result_bind_text_tmp_null),
        test_case(db_result_bind_text_tmp_no_size),
        NULL
    ));
}

TEST_CASE_FIX(db_result_bind_text_dup_unsupported, db_prepare, db_finalize)
{
    char *text  = NULL;
    size_t size = 0;

    test_int_error(db_result_bind_text_dup(stmt, 0, &text, &size), E_DB_UNSUPPORTED_TYPE);
}

TEST_CASE_FIX(db_result_bind_text_dup_oob, db_prepare, db_finalize)
{
    char *text  = NULL;
    size_t size = 0;

    test_int_error(db_result_bind_text_dup(stmt, 1, &text, &size), E_DB_OUT_OF_BOUNDS);
}

TEST_CASE_FIX(db_result_bind_text_dup, db_prepare, db_finalize)
{
    char *text  = NULL;
    size_t size = 0;

    test_int_success(db_param_bind_text(stmt, 0, "foo", -1));
    test_int_success(db_result_bind_text_dup(stmt, 0, &text, &size));
    test_int_success(db_exec(stmt));

    test_uint_eq(size, 3);
    test_str_eq(text, "foo");

    test_void(free(text));
}

TEST_CASE_FIX(db_result_bind_text_dup_null, db_prepare_null, db_finalize)
{
    char *text  = "foo";
    size_t size = 3;

    test_int_success(db_result_bind_text_dup(stmt, 0, &text, &size));
    test_int_success(db_exec(stmt));

    test_uint_eq(size, 0);
    test_ptr_eq(text, NULL);
}

TEST_CASE_FIX(db_result_bind_text_dup_no_size, db_prepare, db_finalize)
{
    char *text = NULL;

    test_int_success(db_param_bind_text(stmt, 0, "foo", -1));
    test_int_success(db_result_bind_text_dup(stmt, 0, &text, NULL));
    test_int_success(db_exec(stmt));

    test_str_eq(text, "foo");

    test_void(free(text));
}

int test_suite_db_result_bind_text_dup(void *param)
{
    suite = param;

    if(!suite->supported)
        return error_pass_int(test_run_case(
            test_case(db_result_bind_text_dup_unsupported)));

    return error_pass_int(test_run_cases(NULL,
        test_case(db_result_bind_text_dup_oob),
        test_case(db_result_bind_text_dup),
        test_case(db_result_bind_text_dup_null),
        test_case(db_result_bind_text_dup_no_size),
        NULL
    ));
}

TEST_CASE_FIX(db_result_bind_text_fix_unsupported, db_prepare, db_finalize)
{
    char text[10]   = "foo";
    size_t size     = 0;
    bool null       = false;

    test_int_error(db_result_bind_text_fix(stmt, 0, text, sizeof(text), &size, &null), E_DB_UNSUPPORTED_TYPE);
}

TEST_CASE_FIX(db_result_bind_text_fix_oob, db_prepare, db_finalize)
{
    char text[10]   = "foo";
    size_t size     = 0;
    bool null       = false;

    test_int_error(db_result_bind_text_fix(stmt, 1, text, sizeof(text), &size, &null), E_DB_OUT_OF_BOUNDS);
}

TEST_CASE_FIX(db_result_bind_text_fix, db_prepare, db_finalize)
{
    char text[10]   = "foo";
    size_t size     = 0;
    bool null       = true;

    test_int_success(db_param_bind_text(stmt, 0, "bar", -1));
    test_int_success(db_result_bind_text_fix(stmt, 0, text, sizeof(text), &size, &null));
    test_int_success(db_exec(stmt));

    test_false(null);
    test_uint_eq(size, 3);
    test_str_eq(text, "bar");
}

TEST_CASE_FIX(db_result_bind_text_fix_null, db_prepare_null, db_finalize)
{
    char text[10]   = "foo";
    size_t size     = 3;
    bool null       = false;

    test_int_success(db_result_bind_text_fix(stmt, 0, text, sizeof(text), &size, &null));
    test_int_success(db_exec(stmt));

    test_true(null);
    test_uint_eq(size, 0);
    test_str_eq(text, "");
}

TEST_CASE_FIX(db_result_bind_text_fix_truncated, db_prepare, db_finalize)
{
    char text[5]   = "foo";
    size_t size     = 0;
    bool null       = true;

    test_int_success(db_param_bind_text(stmt, 0, "foobar", -1));
    test_int_success(db_result_bind_text_fix(stmt, 0, text, sizeof(text), &size, &null));
    test_int_success(db_exec(stmt));

    test_false(null);
    test_uint_eq(size, 6);
    test_str_eq(text, "foob");
}

TEST_CASE_FIX(db_result_bind_text_fix_no_buffer, db_prepare, db_finalize)
{
    size_t size = 0;
    bool null   = true;

    test_int_success(db_param_bind_text(stmt, 0, "foo", -1));
    test_int_success(db_result_bind_text_fix(stmt, 0, NULL, 0, &size, &null));
    test_int_success(db_exec(stmt));

    test_false(null);
    test_uint_eq(size, 3);
}

TEST_CASE_FIX(db_result_bind_text_fix_no_size, db_prepare, db_finalize)
{
    char text[10]   = "foo";
    bool null       = true;

    test_int_success(db_param_bind_text(stmt, 0, "bar", -1));
    test_int_success(db_result_bind_text_fix(stmt, 0, text, sizeof(text), NULL, &null));
    test_int_success(db_exec(stmt));

    test_false(null);
    test_str_eq(text, "bar");
}

TEST_CASE_FIX(db_result_bind_text_fix_no_null, db_prepare, db_finalize)
{
    char text[10]   = "foo";
    size_t size     = 0;

    test_int_success(db_param_bind_text(stmt, 0, "bar", -1));
    test_int_success(db_result_bind_text_fix(stmt, 0, text, sizeof(text), &size, NULL));
    test_int_success(db_exec(stmt));

    test_uint_eq(size, 3);
    test_str_eq(text, "bar");
}

int test_suite_db_result_bind_text_fix(void *param)
{
    suite = param;

    if(!suite->supported)
        return error_pass_int(test_run_case(
            test_case(db_result_bind_text_fix_unsupported)));

    return error_pass_int(test_run_cases(NULL,
        test_case(db_result_bind_text_fix_oob),
        test_case(db_result_bind_text_fix),
        test_case(db_result_bind_text_fix_null),
        test_case(db_result_bind_text_fix_truncated),
        test_case(db_result_bind_text_fix_no_buffer),
        test_case(db_result_bind_text_fix_no_size),
        test_case(db_result_bind_text_fix_no_null),
        NULL
    ));
}

int test_suite_db_result_bind_text(void *param)
{
    return error_pass_int(test_run_suites(NULL,
        test_suite_p(db_result_bind_text_tmp, param),
        test_suite_p(db_result_bind_text_dup, param),
        test_suite_p(db_result_bind_text_fix, param),
        NULL
    ));
}

TEST_CASE_FIX(db_result_bind_blob_tmp_unsupported, db_prepare, db_finalize)
{
    void *blob  = NULL;
    size_t size = 0;

    test_int_error(db_result_bind_blob(stmt, 0, &blob, &size), E_DB_UNSUPPORTED_TYPE);
}

TEST_CASE_FIX(db_result_bind_blob_tmp_oob, db_prepare, db_finalize)
{
    void *blob  = NULL;
    size_t size = 0;

    test_int_error(db_result_bind_blob(stmt, 1, &blob, &size), E_DB_OUT_OF_BOUNDS);
}

TEST_CASE_FIX(db_result_bind_blob_tmp, db_prepare, db_finalize)
{
    test_db_data_st state = { .data = NULL, .size = 0 };

    test_int_success(db_param_bind_blob(stmt, 0, "\xab\xcd", 2));
    test_int_success(db_result_bind_blob(stmt, 0, &state.data, &state.size));
    test_int_success(db_exec_f(stmt, test_db_dup, &state));

    test_uint_eq(state.size, 2);
    test_mem_eq(state.data, "\xab\xcd", 2);

    test_void(free(state.data));
}

TEST_CASE_FIX(db_result_bind_blob_tmp_null, db_prepare_null, db_finalize)
{
    void *blob  = "\x12\x34";
    size_t size = 2;

    test_int_success(db_result_bind_blob(stmt, 0, &blob, &size));
    test_int_success(db_exec(stmt));

    test_uint_eq(size, 0);
    test_ptr_eq(blob, NULL);
}

TEST_CASE_FIX(db_result_bind_blob_tmp_no_size, db_prepare, db_finalize)
{
    test_db_data_st state = { .data = NULL, .size = 2 };

    test_int_success(db_param_bind_blob(stmt, 0, "\xab\xcd", 2));
    test_int_success(db_result_bind_blob(stmt, 0, &state.data, NULL));
    test_int_success(db_exec_f(stmt, test_db_dup, &state));

    test_mem_eq(state.data, "\xab\xcd", 2);

    test_void(free(state.data));
}

int test_suite_db_result_bind_blob_tmp(void *param)
{
    suite = param;

    if(!suite->supported)
        return error_pass_int(test_run_case(
            test_case(db_result_bind_blob_tmp_unsupported)));

    return error_pass_int(test_run_cases(NULL,
        test_case(db_result_bind_blob_tmp_oob),
        test_case(db_result_bind_blob_tmp),
        test_case(db_result_bind_blob_tmp_null),
        test_case(db_result_bind_blob_tmp_no_size),
        NULL
    ));
}

TEST_CASE_FIX(db_result_bind_blob_dup_unsupported, db_prepare, db_finalize)
{
    void *blob  = NULL;
    size_t size = 0;

    test_int_error(db_result_bind_blob_dup(stmt, 0, &blob, &size), E_DB_UNSUPPORTED_TYPE);
}

TEST_CASE_FIX(db_result_bind_blob_dup_oob, db_prepare, db_finalize)
{
    void *blob  = NULL;
    size_t size = 0;

    test_int_error(db_result_bind_blob_dup(stmt, 1, &blob, &size), E_DB_OUT_OF_BOUNDS);
}

TEST_CASE_FIX(db_result_bind_blob_dup, db_prepare, db_finalize)
{
    void *blob  = NULL;
    size_t size = 0;

    test_int_success(db_param_bind_blob(stmt, 0, "\xab\xcd", 2));
    test_int_success(db_result_bind_blob_dup(stmt, 0, &blob, &size));
    test_int_success(db_exec(stmt));

    test_uint_eq(size, 2);
    test_mem_eq(blob, "\xab\xcd", 2);

    test_void(free(blob));
}

TEST_CASE_FIX(db_result_bind_blob_dup_null, db_prepare_null, db_finalize)
{
    void *blob  = "\x12\x34";
    size_t size = 2;

    test_int_success(db_result_bind_blob_dup(stmt, 0, &blob, &size));
    test_int_success(db_exec(stmt));

    test_uint_eq(size, 0);
    test_ptr_eq(blob, NULL);
}

TEST_CASE_FIX(db_result_bind_blob_dup_no_size, db_prepare, db_finalize)
{
    void *blob = NULL;

    test_int_success(db_param_bind_blob(stmt, 0, "\xab\xcd", 2));
    test_int_success(db_result_bind_blob_dup(stmt, 0, &blob, NULL));
    test_int_success(db_exec(stmt));

    test_mem_eq(blob, "\xab\xcd", 2);

    test_void(free(blob));
}

int test_suite_db_result_bind_blob_dup(void *param)
{
    suite = param;

    if(!suite->supported)
        return error_pass_int(test_run_case(
            test_case(db_result_bind_blob_dup_unsupported)));

    return error_pass_int(test_run_cases(NULL,
        test_case(db_result_bind_blob_dup_oob),
        test_case(db_result_bind_blob_dup),
        test_case(db_result_bind_blob_dup_null),
        test_case(db_result_bind_blob_dup_no_size),
        NULL
    ));
}

TEST_CASE_FIX(db_result_bind_blob_fix_unsupported, db_prepare, db_finalize)
{
    char blob[10]   = "\xff\xff";
    size_t size     = 0;
    bool null       = false;

    test_int_error(db_result_bind_blob_fix(stmt, 0, blob, sizeof(blob), &size, &null), E_DB_UNSUPPORTED_TYPE);
}

TEST_CASE_FIX(db_result_bind_blob_fix_oob, db_prepare, db_finalize)
{
    char blob[10]   = "\xff\xff";
    size_t size     = 0;
    bool null       = false;

    test_int_error(db_result_bind_blob_fix(stmt, 1, blob, sizeof(blob), &size, &null), E_DB_OUT_OF_BOUNDS);
}

TEST_CASE_FIX(db_result_bind_blob_fix, db_prepare, db_finalize)
{
    char blob[10]   = "\xff\xff";
    size_t size     = 0;
    bool null       = true;

    test_int_success(db_param_bind_blob(stmt, 0, "\xab\xcd", 2));
    test_int_success(db_result_bind_blob_fix(stmt, 0, blob, sizeof(blob), &size, &null));
    test_int_success(db_exec(stmt));

    test_false(null);
    test_uint_eq(size, 2);
    test_mem_eq(blob, "\xab\xcd", 2);
}

TEST_CASE_FIX(db_result_bind_blob_fix_null, db_prepare_null, db_finalize)
{
    char blob[10]   = "\xff\xff";
    size_t size     = 2;
    bool null       = false;

    test_int_success(db_result_bind_blob_fix(stmt, 0, blob, sizeof(blob), &size, &null));
    test_int_success(db_exec(stmt));

    test_true(null);
    test_uint_eq(size, 0);
}

TEST_CASE_FIX(db_result_bind_blob_fix_truncated, db_prepare, db_finalize)
{
    char blob[3]   = "\xff\xff";
    size_t size     = 0;
    bool null       = true;

    test_int_success(db_param_bind_blob(stmt, 0, "\x12\x34\x56\x78", 4));
    test_int_success(db_result_bind_blob_fix(stmt, 0, blob, sizeof(blob), &size, &null));
    test_int_success(db_exec(stmt));

    test_false(null);
    test_uint_eq(size, 4);
    test_mem_eq(blob, "\x12\x34\x56", 3);
}

TEST_CASE_FIX(db_result_bind_blob_fix_no_buffer, db_prepare, db_finalize)
{
    size_t size = 0;
    bool null   = true;

    test_int_success(db_param_bind_blob(stmt, 0, "\xab\xcd", 2));
    test_int_success(db_result_bind_blob_fix(stmt, 0, NULL, 0, &size, &null));
    test_int_success(db_exec(stmt));

    test_false(null);
    test_uint_eq(size, 2);
}

TEST_CASE_FIX(db_result_bind_blob_fix_no_size, db_prepare, db_finalize)
{
    char blob[10]   = "\xff\xff";
    bool null       = true;

    test_int_success(db_param_bind_blob(stmt, 0, "\xab\xcd", 2));
    test_int_success(db_result_bind_blob_fix(stmt, 0, blob, sizeof(blob), NULL, &null));
    test_int_success(db_exec(stmt));

    test_false(null);
    test_mem_eq(blob, "\xab\xcd", 2);
}

TEST_CASE_FIX(db_result_bind_blob_fix_no_null, db_prepare, db_finalize)
{
    char blob[10]   = "\xff\xff";
    size_t size     = 0;

    test_int_success(db_param_bind_blob(stmt, 0, "\xab\xcd", 2));
    test_int_success(db_result_bind_blob_fix(stmt, 0, blob, sizeof(blob), &size, NULL));
    test_int_success(db_exec(stmt));

    test_uint_eq(size, 2);
    test_mem_eq(blob, "\xab\xcd", 2);
}

int test_suite_db_result_bind_blob_fix(void *param)
{
    suite = param;

    if(!suite->supported)
        return error_pass_int(test_run_case(
            test_case(db_result_bind_blob_fix_unsupported)));

    return error_pass_int(test_run_cases(NULL,
        test_case(db_result_bind_blob_fix_oob),
        test_case(db_result_bind_blob_fix),
        test_case(db_result_bind_blob_fix_null),
        test_case(db_result_bind_blob_fix_truncated),
        test_case(db_result_bind_blob_fix_no_buffer),
        test_case(db_result_bind_blob_fix_no_size),
        test_case(db_result_bind_blob_fix_no_null),
        NULL
    ));
}

int test_suite_db_result_bind_blob(void *param)
{
    return error_pass_int(test_run_suites(NULL,
        test_suite_p(db_result_bind_blob_tmp, param),
        test_suite_p(db_result_bind_blob_dup, param),
        test_suite_p(db_result_bind_blob_fix, param),
        NULL
    ));
}

TEST_CASE_FIX(db_result_bind_date_unsupported, db_prepare, db_finalize)
{
    db_date_st date = { 0 };
    bool null       = false;

    test_int_error(db_result_bind_date(stmt, 0, &date, &null), E_DB_UNSUPPORTED_TYPE);
}

TEST_CASE_FIX(db_result_bind_date_oob, db_prepare, db_finalize)
{
    db_date_st date = { 0 };
    bool null       = false;

    test_int_error(db_result_bind_date(stmt, 1, &date, &null), E_DB_OUT_OF_BOUNDS);
}

TEST_CASE_FIX(db_result_bind_date, db_prepare, db_finalize)
{
    db_date_st date = { 0 };
    bool null       = true;

    test_int_success(db_param_bind_date_full(stmt, 0, 2020, 04, 20));
    test_int_success(db_result_bind_date(stmt, 0, &date, &null));
    test_int_success(db_exec(stmt));

    test_false(null);
    test_uint_eq(date.year, 2020);
    test_uint_eq(date.month, 04);
    test_uint_eq(date.day, 20);
}

TEST_CASE_FIX(db_result_bind_date_null, db_prepare_null, db_finalize)
{
    db_date_st date = { 0 };
    bool null       = false;

    test_int_success(db_result_bind_date(stmt, 0, &date, &null));
    test_int_success(db_exec(stmt));

    test_true(null);
}

TEST_CASE_FIX(db_result_bind_date_no_null, db_prepare, db_finalize)
{
    db_date_st date = { 0 };

    test_int_success(db_param_bind_date_full(stmt, 0, 2020, 04, 20));
    test_int_success(db_result_bind_date(stmt, 0, &date, NULL));
    test_int_success(db_exec(stmt));

    test_uint_eq(date.year, 2020);
    test_uint_eq(date.month, 04);
    test_uint_eq(date.day, 20);
}

int test_suite_db_result_bind_date(void *param)
{
    suite = param;

    if(!suite->supported)
        return error_pass_int(test_run_case(
            test_case(db_result_bind_date_unsupported)));

    return error_pass_int(test_run_cases(NULL,
        test_case(db_result_bind_date_oob),
        test_case(db_result_bind_date),
        test_case(db_result_bind_date_null),
        test_case(db_result_bind_date_no_null),
        NULL
    ));
}

TEST_CASE_FIX(db_result_bind_time_unsupported, db_prepare, db_finalize)
{
    db_time_st time = { 0 };
    bool null       = false;

    test_int_error(db_result_bind_time(stmt, 0, &time, &null), E_DB_UNSUPPORTED_TYPE);
}

TEST_CASE_FIX(db_result_bind_time_oob, db_prepare, db_finalize)
{
    db_time_st time = { 0 };
    bool null       = false;

    test_int_error(db_result_bind_time(stmt, 1, &time, &null), E_DB_OUT_OF_BOUNDS);
}

TEST_CASE_FIX(db_result_bind_time, db_prepare, db_finalize)
{
    db_time_st time = { 0 };
    bool null       = true;

    test_int_success(db_param_bind_time_full(stmt, 0, 12, 34, 56));
    test_int_success(db_result_bind_time(stmt, 0, &time, &null));
    test_int_success(db_exec(stmt));

    test_false(null);
    test_uint_eq(time.hour, 12);
    test_uint_eq(time.minute, 34);
    test_uint_eq(time.second, 56);
}

TEST_CASE_FIX(db_result_bind_time_null, db_prepare_null, db_finalize)
{
    db_time_st time = { 0 };
    bool null       = false;

    test_int_success(db_result_bind_time(stmt, 0, &time, &null));
    test_int_success(db_exec(stmt));

    test_true(null);
}

TEST_CASE_FIX(db_result_bind_time_no_null, db_prepare, db_finalize)
{
    db_time_st time = { 0 };

    test_int_success(db_param_bind_time_full(stmt, 0, 12, 34, 56));
    test_int_success(db_result_bind_time(stmt, 0, &time, NULL));
    test_int_success(db_exec(stmt));

    test_uint_eq(time.hour, 12);
    test_uint_eq(time.minute, 34);
    test_uint_eq(time.second, 56);
}

int test_suite_db_result_bind_time(void *param)
{
    suite = param;

    if(!suite->supported)
        return error_pass_int(test_run_case(
            test_case(db_result_bind_time_unsupported)));

    return error_pass_int(test_run_cases(NULL,
        test_case(db_result_bind_time_oob),
        test_case(db_result_bind_time),
        test_case(db_result_bind_time_null),
        test_case(db_result_bind_time_no_null),
        NULL
    ));
}

TEST_CASE_FIX(db_result_bind_datetime_unsupported, db_prepare, db_finalize)
{
    db_datetime_st dt   = { 0 };
    bool null           = false;

    test_int_error(db_result_bind_datetime(stmt, 0, &dt, &null), E_DB_UNSUPPORTED_TYPE);
}

TEST_CASE_FIX(db_result_bind_datetime_oob, db_prepare, db_finalize)
{
    db_datetime_st dt   = { 0 };
    bool null           = false;

    test_int_error(db_result_bind_datetime(stmt, 1, &dt, &null), E_DB_OUT_OF_BOUNDS);
}

TEST_CASE_FIX(db_result_bind_datetime, db_prepare, db_finalize)
{
    db_datetime_st dt   = { 0 };
    bool null           = true;

    test_int_success(db_param_bind_datetime_full(stmt, 0, 2020, 04, 20, 12, 34, 56));
    test_int_success(db_result_bind_datetime(stmt, 0, &dt, &null));
    test_int_success(db_exec(stmt));

    test_false(null);
    test_uint_eq(dt.date.year, 2020);
    test_uint_eq(dt.date.month, 04);
    test_uint_eq(dt.date.day, 20);
    test_uint_eq(dt.time.hour, 12);
    test_uint_eq(dt.time.minute, 34);
    test_uint_eq(dt.time.second, 56);
}

TEST_CASE_FIX(db_result_bind_datetime_null, db_prepare_null, db_finalize)
{
    db_datetime_st dt   = { 0 };
    bool null           = false;

    test_int_success(db_result_bind_datetime(stmt, 0, &dt, &null));
    test_int_success(db_exec(stmt));

    test_true(null);
}

TEST_CASE_FIX(db_result_bind_datetime_no_null, db_prepare, db_finalize)
{
    db_datetime_st dt = { 0 };

    test_int_success(db_param_bind_datetime_full(stmt, 0, 2020, 04, 20, 12, 34, 56));
    test_int_success(db_result_bind_datetime(stmt, 0, &dt, NULL));
    test_int_success(db_exec(stmt));

    test_uint_eq(dt.date.year, 2020);
    test_uint_eq(dt.date.month, 04);
    test_uint_eq(dt.date.day, 20);
    test_uint_eq(dt.time.hour, 12);
    test_uint_eq(dt.time.minute, 34);
    test_uint_eq(dt.time.second, 56);
}

int test_suite_db_result_bind_datetime(void *param)
{
    suite = param;

    if(!suite->supported)
        return error_pass_int(test_run_case(
            test_case(db_result_bind_datetime_unsupported)));

    return error_pass_int(test_run_cases(NULL,
        test_case(db_result_bind_datetime_oob),
        test_case(db_result_bind_datetime),
        test_case(db_result_bind_datetime_null),
        test_case(db_result_bind_datetime_no_null),
        NULL
    ));
}

TEST_CASE_FIX(db_result_bind_timestamp_unsupported, db_prepare, db_finalize)
{
    time_t ts   = 0;
    bool null   = false;

    test_int_error(db_result_bind_timestamp(stmt, 0, &ts, &null), E_DB_UNSUPPORTED_TYPE);
}

TEST_CASE_FIX(db_result_bind_timestamp_oob, db_prepare, db_finalize)
{
    time_t ts   = 0;
    bool null   = false;

    test_int_error(db_result_bind_timestamp(stmt, 1, &ts, &null), E_DB_OUT_OF_BOUNDS);
}

TEST_CASE_FIX(db_result_bind_timestamp, db_prepare, db_finalize)
{
    time_t ts   = 0;
    bool null   = true;

    test_int_success(db_param_bind_timestamp(stmt, 0, 1587386096));
    test_int_success(db_result_bind_timestamp(stmt, 0, &ts, &null));
    test_int_success(db_exec(stmt));

    test_false(null);
    test_int_eq(ts, 1587386096);
}

TEST_CASE_FIX(db_result_bind_timestamp_null, db_prepare_null, db_finalize)
{
    time_t ts   = 0;
    bool null   = false;

    test_int_success(db_result_bind_timestamp(stmt, 0, &ts, &null));
    test_int_success(db_exec(stmt));

    test_true(null);
}

TEST_CASE_FIX(db_result_bind_timestamp_no_null, db_prepare, db_finalize)
{
    time_t ts = 0;

    test_int_success(db_param_bind_timestamp(stmt, 0, 1587386096));
    test_int_success(db_result_bind_timestamp(stmt, 0, &ts, NULL));
    test_int_success(db_exec(stmt));

    test_int_eq(ts, 1587386096);
}

int test_suite_db_result_bind_timestamp(void *param)
{
    suite = param;

    if(!suite->supported)
        return error_pass_int(test_run_case(
            test_case(db_result_bind_timestamp_unsupported)));

    return error_pass_int(test_run_cases(NULL,
        test_case(db_result_bind_timestamp_oob),
        test_case(db_result_bind_timestamp),
        test_case(db_result_bind_timestamp_null),
        test_case(db_result_bind_timestamp_no_null),
        NULL
    ));
}

static int test_db_fetch_unsupported(db_stmt_ct stmt, size_t row, void *ctx)
{
    test_int_error(db_result_fetch(stmt, 0, 0), E_DB_UNSUPPORTED);

    return 0;
}

TEST_CASE_PFIX(db_result_fetch_unsupported, db_prepare, db_finalize, "select 'foo';")
{
    test_int_success(db_exec_f(stmt, test_db_fetch_unsupported, NULL));
}

static int test_db_fetch_oob(db_stmt_ct stmt, size_t row, void *ctx)
{
    test_int_error(db_result_fetch(stmt, 1, 0), E_DB_OUT_OF_BOUNDS);

    return 0;
}

TEST_CASE_PFIX(db_result_fetch_oob, db_prepare, db_finalize, "select 'foo';")
{
    test_int_success(db_exec_f(stmt, test_db_fetch_oob, NULL));
}

static int test_db_fetch(db_stmt_ct stmt, size_t row, void *ctx)
{
    char *text  = NULL;
    size_t size = 0;

    test_int_success(db_result_bind_text(stmt, 0, &text, &size));
    test_int_success(db_result_fetch(stmt, 0, 0));

    test_uint_eq(size, 3);
    test_str_eq(text, "foo");

    return 0;
}

TEST_CASE_PFIX(db_result_fetch_bound, db_prepare, db_finalize, "select 'foo';")
{
    char *text = NULL;

    test_int_success(db_result_bind_text(stmt, 0, &text, NULL));
    test_int_success(db_exec_f(stmt, test_db_fetch, NULL));
}

TEST_CASE_PFIX(db_result_fetch_unbound, db_prepare, db_finalize, "select 'foo';")
{
    test_int_success(db_exec_f(stmt, test_db_fetch, NULL));
}

int test_suite_db_result_fetch(void *param)
{
    suite = param;

    if(!suite->supported)
        return error_pass_int(test_run_case(
            test_case(db_result_fetch_unsupported)));

    return error_pass_int(test_run_cases(NULL,
        test_case(db_result_fetch_oob),
        test_case(db_result_fetch_bound),
        test_case(db_result_fetch_unbound),
        NULL
    ));
}

TEST_CASE_FIX(db_result_get_type_unsupported, db_prepare_null, db_finalize)
{
    test_rc_error(db_result_get_type(stmt, 0), DB_TYPE_INVALID, E_DB_UNSUPPORTED);
}

TEST_CASE_FIX(db_result_get_type_oob, db_prepare_null, db_finalize)
{
    test_rc_error(db_result_get_type(stmt, 1), DB_TYPE_INVALID, E_DB_OUT_OF_BOUNDS);
}

TEST_CASE_PFIX(db_result_get_type, db_prepare, db_finalize, "select 'foo';")
{
    test_rc_success(db_result_get_type(stmt, 0), DB_TYPE_TEXT, DB_TYPE_INVALID);
}

int test_suite_db_result_get_type(void *param)
{
    suite = param;

    if(!suite->supported)
        return error_pass_int(test_run_case(
            test_case(db_result_get_type_unsupported)));

    return error_pass_int(test_run_cases(NULL,
        test_case(db_result_get_type_oob),
        test_case(db_result_get_type),
        NULL
    ));
}

TEST_CASE_FIX(db_result_get_database_name_unsupported, db_prepare_null, db_finalize)
{
    test_ptr_error(db_result_get_database_name(stmt, 0), E_DB_UNSUPPORTED);
}

TEST_CASE_FIX(db_result_get_database_name_oob, db_prepare_null, db_finalize)
{
    test_ptr_error(db_result_get_database_name(stmt, 1), E_DB_OUT_OF_BOUNDS);
}

TEST_CASE_PFIX(db_result_get_database_name, db_prepare_table, db_finalize,
    "select i from tmp;")
{
    const char *name;

    test_ptr_success(name = db_result_get_database_name(stmt, 0));
    test_str_eq(name, "temp");
}

TEST_CASE_FIX(db_result_get_database_name_no_name, db_prepare_null, db_finalize)
{
    test_ptr_error(db_result_get_database_name(stmt, 0), E_DB_NO_NAME);
}

int test_suite_db_result_get_database_name(void *param)
{
    suite = param;

    if(!suite->supported)
        return error_pass_int(test_run_case(
            test_case(db_result_get_database_name_unsupported)));

    return error_pass_int(test_run_cases(NULL,
        test_case(db_result_get_database_name_oob),
        test_case(db_result_get_database_name),
        test_case(db_result_get_database_name_no_name),
        NULL
    ));
}

TEST_CASE_FIX(db_result_get_table_name_unsupported, db_prepare_null, db_finalize)
{
    test_ptr_error(db_result_get_table_name(stmt, 0), E_DB_UNSUPPORTED);
}

TEST_CASE_FIX(db_result_get_table_name_oob, db_prepare_null, db_finalize)
{
    test_ptr_error(db_result_get_table_name(stmt, 1), E_DB_OUT_OF_BOUNDS);
}

TEST_CASE_PFIX(db_result_get_table_name, db_prepare_table, db_finalize,
    "select * from foo")
{
    const char *name;

    test_ptr_success(name = db_result_get_table_name(stmt, 0));
    test_str_eq(name, "main");
}

TEST_CASE_FIX(db_result_get_table_name_no_name, db_prepare_null, db_finalize)
{
    test_ptr_error(db_result_get_table_name(stmt, 0), E_DB_NO_NAME);
}

int test_suite_db_result_get_table_name(void *param)
{
    suite = param;

    if(!suite->supported)
        return error_pass_int(test_run_case(
            test_case(db_result_get_table_name_unsupported)));

    return error_pass_int(test_run_cases(NULL,
        test_case(db_result_get_table_name_oob),
        test_case(db_result_get_table_name),
        test_case(db_result_get_table_name_no_name),
        NULL
    ));
}

TEST_CASE_FIX(db_result_get_original_table_name_unsupported, db_prepare_null, db_finalize)
{
    test_ptr_error(db_result_get_original_table_name(stmt, 0), E_DB_UNSUPPORTED);
}

TEST_CASE_FIX(db_result_get_original_table_name_oob, db_prepare_null, db_finalize)
{
    test_ptr_error(db_result_get_original_table_name(stmt, 1), E_DB_OUT_OF_BOUNDS);
}

TEST_CASE_PFIX(db_result_get_original_table_name, db_prepare_table, db_finalize,
    "select * from tmp as foo;")
{
    const char *name;

    test_ptr_success(name = db_result_get_original_table_name(stmt, 0));
    test_str_eq(name, "tmp");
}

TEST_CASE_FIX(db_result_get_original_table_name_no_name, db_prepare_null, db_finalize)
{
    test_ptr_error(db_result_get_original_table_name(stmt, 0), E_DB_NO_NAME);
}

int test_suite_db_result_get_original_table_name(void *param)
{
    suite = param;

    if(!suite->supported)
        return error_pass_int(test_run_case(
            test_case(db_result_get_original_table_name_unsupported)));

    return error_pass_int(test_run_cases(NULL,
        test_case(db_result_get_original_table_name_oob),
        test_case(db_result_get_original_table_name),
        test_case(db_result_get_original_table_name_no_name),
        NULL
    ));
}

TEST_CASE_FIX(db_result_get_field_name_unsupported, db_prepare_null, db_finalize)
{
    test_ptr_error(db_result_get_field_name(stmt, 0), E_DB_UNSUPPORTED);
}

TEST_CASE_FIX(db_result_get_field_name_oob, db_prepare_null, db_finalize)
{
    test_ptr_error(db_result_get_field_name(stmt, 1), E_DB_OUT_OF_BOUNDS);
}

TEST_CASE_PFIX(db_result_get_field_name, db_prepare, db_finalize,
    "select 123 as foo;")
{
    const char *name;

    test_ptr_success(name = db_result_get_field_name(stmt, 0));
    test_str_eq(name, "foo");
}

int test_suite_db_result_get_field_name(void *param)
{
    suite = param;

    if(!suite->supported)
        return error_pass_int(test_run_case(
            test_case(db_result_get_field_name_unsupported)));

    return error_pass_int(test_run_cases(NULL,
        test_case(db_result_get_field_name_oob),
        test_case(db_result_get_field_name),
        NULL
    ));
}

TEST_CASE_FIX(db_result_get_original_field_name_unsupported, db_prepare_null, db_finalize)
{
    test_ptr_error(db_result_get_original_field_name(stmt, 0), E_DB_UNSUPPORTED);
}

TEST_CASE_FIX(db_result_get_original_field_name_oob, db_prepare_null, db_finalize)
{
    test_ptr_error(db_result_get_original_field_name(stmt, 1), E_DB_OUT_OF_BOUNDS);
}

TEST_CASE_PFIX(db_result_get_original_field_name, db_prepare_table, db_finalize,
    "select i from tmp;")
{
    const char *name;

    test_ptr_success(name = db_result_get_original_field_name(stmt, 0));
    test_str_eq(name, "i");
}

TEST_CASE_FIX(db_result_get_original_field_name_no_name, db_prepare_null, db_finalize)
{
    test_ptr_error(db_result_get_original_field_name(stmt, 0), E_DB_NO_NAME);
}

int test_suite_db_result_get_original_field_name(void *param)
{
    suite = param;

    if(!suite->supported)
        return error_pass_int(test_run_case(
            test_case(db_result_get_original_field_name_unsupported)));

    return error_pass_int(test_run_cases(NULL,
        test_case(db_result_get_original_field_name_oob),
        test_case(db_result_get_original_field_name),
        test_case(db_result_get_original_field_name_no_name),
        NULL
    ));
}
