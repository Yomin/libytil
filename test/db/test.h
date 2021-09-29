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

#ifndef YTIL_TEST_SUITE_DB_TEST_INCLUDED
#define YTIL_TEST_SUITE_DB_TEST_INCLUDED

#include <ytil/db/db.h>

typedef db_ct (*db_open_cb)(void);

typedef struct test_config_db
{
    db_open_cb  open;
    const char  *db;
    uint8_t     flt_dig;
    uint8_t     dbl_dig;
    uint8_t     ldbl_dig;
    const char  *nan;
    const char  *inf;
} test_config_db_st;

typedef struct test_param_db
{
    bool                    supported;
    const test_config_db_st *config;
} test_param_db_st;

#define test_suite_db(suite, supported, config)   \
    test_suite_p(db_ ## suite, (&(test_param_db_st) { (supported), &(config) }))

#define test_suite_db_supported(suite, config) \
    test_suite_db(suite, true, (config))

#define test_suite_db_unsupported(suite, config) \
    test_suite_db(suite, false, (config))


int test_suite_db_prepare(void *param);
int test_suite_db_exec(void *param);
int test_suite_db_trace(void *param);

int test_suite_db_sql_plain(void *param);
int test_suite_db_sql_expanded(void *param);
int test_suite_db_sql(void *param);


#include "param.h"
#include "result.h"


int test_suite_db_type_bool(void *param);

int test_suite_db_type_sint8(void *param);
int test_suite_db_type_uint8(void *param);
int test_suite_db_type_int8(void *param);

int test_suite_db_type_sint16(void *param);
int test_suite_db_type_uint16(void *param);
int test_suite_db_type_int16(void *param);

int test_suite_db_type_sint32(void *param);
int test_suite_db_type_uint32(void *param);
int test_suite_db_type_int32(void *param);

int test_suite_db_type_sint64(void *param);
int test_suite_db_type_uint64(void *param);
int test_suite_db_type_int64(void *param);

int test_suite_db_type_float(void *param);
int test_suite_db_type_double(void *param);
int test_suite_db_type_ldouble(void *param);

int test_suite_db_type_text(void *param);
int test_suite_db_type_blob(void *param);

int test_suite_db_type_date(void *param);
int test_suite_db_type_time(void *param);
int test_suite_db_type_datetime(void *param);
int test_suite_db_type_timestamp(void *param);


#endif
