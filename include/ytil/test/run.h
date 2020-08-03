/*
 * Copyright (c) 2019 Martin Rödel a.k.a. Yomin Nimoy
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

#ifndef YTIL_TEST_RUN_H_INCLUDED
#define YTIL_TEST_RUN_H_INCLUDED

#include <ytil/test/suite.h>
#include <stdbool.h>

typedef enum test_run_error
{
      E_TEST_RUN_INVALID_OBJECT
    , E_TEST_RUN_INVALID_SUITE
    , E_TEST_RUN_INVALID_FILTER
    , E_TEST_RUN_INVALID_OPTION
    , E_TEST_RUN_MISSING_ARG
    , E_TEST_RUN_MALFORMED_ARG
    , E_TEST_RUN_NOT_AVAILABLE
    , E_TEST_RUN_TRACE_CHECK
} test_run_error_id;

struct test_run;
typedef struct test_run *test_run_ct;

test_run_ct test_run_new(void);
test_run_ct test_run_new_with_args(int argc, char *argv[]);
void        test_run_free(test_run_ct run);

int test_run_enable_clean(test_run_ct run, bool clean);
int test_run_enable_dump(test_run_ct run, bool dump);
int test_run_enable_fork(test_run_ct run, bool fork);
int test_run_enable_skip(test_run_ct run, bool skip);

int test_run_add_filter(test_run_ct run, const char *filter);

int test_run_exec(test_run_ct run, test_suite_const_ct suite);

#endif
