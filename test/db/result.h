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

#ifndef YTIL_TEST_SUITE_DB_RESULT_INCLUDED
#define YTIL_TEST_SUITE_DB_RESULT_INCLUDED


int test_suite_db_result_count(void *ctx);

int test_suite_db_result_bind_bool(void *ctx);

int test_suite_db_result_bind_sint8(void *ctx);
int test_suite_db_result_bind_uint8(void *ctx);
int test_suite_db_result_bind_int8(void *ctx);

int test_suite_db_result_bind_sint16(void *ctx);
int test_suite_db_result_bind_uint16(void *ctx);
int test_suite_db_result_bind_int16(void *ctx);

int test_suite_db_result_bind_sint32(void *ctx);
int test_suite_db_result_bind_uint32(void *ctx);
int test_suite_db_result_bind_int32(void *ctx);

int test_suite_db_result_bind_sint64(void *ctx);
int test_suite_db_result_bind_uint64(void *ctx);
int test_suite_db_result_bind_int64(void *ctx);

int test_suite_db_result_bind_float(void *ctx);
int test_suite_db_result_bind_double(void *ctx);
int test_suite_db_result_bind_ldouble(void *ctx);

int test_suite_db_result_bind_text_tmp(void *ctx);
int test_suite_db_result_bind_text_dup(void *ctx);
int test_suite_db_result_bind_text_fix(void *ctx);
int test_suite_db_result_bind_text(void *ctx);

int test_suite_db_result_bind_blob_tmp(void *ctx);
int test_suite_db_result_bind_blob_dup(void *ctx);
int test_suite_db_result_bind_blob_fix(void *ctx);
int test_suite_db_result_bind_blob(void *ctx);

int test_suite_db_result_bind_date(void *ctx);
int test_suite_db_result_bind_time(void *ctx);
int test_suite_db_result_bind_datetime(void *ctx);
int test_suite_db_result_bind_timestamp(void *ctx);

int test_suite_db_result_get_type(void *ctx);

int test_suite_db_result_get_database_name(void *ctx);
int test_suite_db_result_get_table_name(void *ctx);
int test_suite_db_result_get_original_table_name(void *ctx);
int test_suite_db_result_get_field_name(void *ctx);
int test_suite_db_result_get_original_field_name(void *ctx);


#endif
