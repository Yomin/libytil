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

#ifndef YTIL_TEST_SUITE_DB_PARAM_INCLUDED
#define YTIL_TEST_SUITE_DB_PARAM_INCLUDED


int test_suite_db_param_count(void *param);

int test_suite_db_param_bind_null(void *param);

int test_suite_db_param_bind_bool_tmp(void *param);
int test_suite_db_param_bind_bool_ref(void *param);
int test_suite_db_param_bind_bool(void *param);

int test_suite_db_param_bind_sint8_tmp(void *param);
int test_suite_db_param_bind_sint8_ref(void *param);
int test_suite_db_param_bind_sint8(void *param);
int test_suite_db_param_bind_uint8_tmp(void *param);
int test_suite_db_param_bind_uint8_ref(void *param);
int test_suite_db_param_bind_uint8(void *param);
int test_suite_db_param_bind_int8(void *param);

int test_suite_db_param_bind_sint16_tmp(void *param);
int test_suite_db_param_bind_sint16_ref(void *param);
int test_suite_db_param_bind_sint16(void *param);
int test_suite_db_param_bind_uint16_tmp(void *param);
int test_suite_db_param_bind_uint16_ref(void *param);
int test_suite_db_param_bind_uint16(void *param);
int test_suite_db_param_bind_int16(void *param);

int test_suite_db_param_bind_sint32_tmp(void *param);
int test_suite_db_param_bind_sint32_ref(void *param);
int test_suite_db_param_bind_sint32(void *param);
int test_suite_db_param_bind_uint32_tmp(void *param);
int test_suite_db_param_bind_uint32_ref(void *param);
int test_suite_db_param_bind_uint32(void *param);
int test_suite_db_param_bind_int32(void *param);

int test_suite_db_param_bind_sint64_tmp(void *param);
int test_suite_db_param_bind_sint64_ref(void *param);
int test_suite_db_param_bind_sint64(void *param);
int test_suite_db_param_bind_uint64_tmp(void *param);
int test_suite_db_param_bind_uint64_ref(void *param);
int test_suite_db_param_bind_uint64(void *param);
int test_suite_db_param_bind_int64(void *param);

int test_suite_db_param_bind_float_tmp(void *param);
int test_suite_db_param_bind_float_ref(void *param);
int test_suite_db_param_bind_float(void *param);
int test_suite_db_param_bind_double_tmp(void *param);
int test_suite_db_param_bind_double_ref(void *param);
int test_suite_db_param_bind_double(void *param);

int test_suite_db_param_bind_text_fix(void *param);
int test_suite_db_param_bind_text_tmp(void *param);
int test_suite_db_param_bind_text_ref(void *param);
int test_suite_db_param_bind_text(void *param);

int test_suite_db_param_bind_blob_fix(void *param);
int test_suite_db_param_bind_blob_tmp(void *param);
int test_suite_db_param_bind_blob_ref(void *param);
int test_suite_db_param_bind_blob(void *param);

int test_suite_db_param_bind_date_fix(void *param);
int test_suite_db_param_bind_date_tmp(void *param);
int test_suite_db_param_bind_date_ref(void *param);
int test_suite_db_param_bind_date(void *param);

int test_suite_db_param_bind_time_fix(void *param);
int test_suite_db_param_bind_time_tmp(void *param);
int test_suite_db_param_bind_time_ref(void *param);
int test_suite_db_param_bind_time(void *param);

int test_suite_db_param_bind_datetime_fix(void *param);
int test_suite_db_param_bind_datetime_tmp(void *param);
int test_suite_db_param_bind_datetime_ref(void *param);
int test_suite_db_param_bind_datetime(void *param);

int test_suite_db_param_bind_timestamp_tmp(void *param);
int test_suite_db_param_bind_timestamp_ref(void *param);
int test_suite_db_param_bind_timestamp(void *param);


#endif
