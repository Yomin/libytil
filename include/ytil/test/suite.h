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

#ifndef __YTIL_TEST_SUITE_H__
#define __YTIL_TEST_SUITE_H__

#include <ytil/test/case.h>
#include <stddef.h>
#include <stdarg.h>
#include <stdbool.h>


typedef enum test_suite_error
{
     E_TEST_SUITE_INVALID_CALLBACK
   , E_TEST_SUITE_INVALID_NAME
   , E_TEST_SUITE_INVALID_OBJECT
   , E_TEST_SUITE_NOT_FOUND
} test_suite_error_id;


struct test_suite;

typedef       struct test_suite *test_suite_ct;
typedef const struct test_suite *test_suite_const_ct;

typedef enum test_entry_type
{
      TEST_ENTRY_SUITE
    , TEST_ENTRY_CASE
} test_entry_id;

typedef struct test_entry
{
    test_entry_id type;
    union uvalue
    {
        void *entry;
        test_suite_ct suite;
        test_case_ct tcase;
    } value;
} test_entry_st;

typedef int (*test_suite_fold_cb)(test_suite_const_ct suite, test_entry_st *entry, void *ctx);


#define TEST_SUITE_SENTINEL ((void*)1)

test_suite_ct  test_suite_new(const char *name);
test_suite_ct  test_suite_new_with_suite(const char *name, test_suite_ct sub);
#define        test_suite_new_with_suites(name, ...) \
              _test_suite_new_with_suites(name, __VA_ARGS__, TEST_SUITE_SENTINEL)
test_suite_ct _test_suite_new_with_suites(const char *name, ...);
test_suite_ct  test_suite_new_with_suites_v(const char *name, va_list ap);
test_suite_ct  test_suite_new_with_case(const char *name, test_case_ct tcase);
#define        test_suite_new_with_cases(name, ...) \
              _test_suite_new_with_cases(name, __VA_ARGS__, TEST_SUITE_SENTINEL)
test_suite_ct _test_suite_new_with_cases(const char *name, ...);
test_suite_ct  test_suite_new_with_cases_v(const char *name, va_list ap);
void           test_suite_free(test_suite_ct suite);

const char    *test_suite_get_name(test_suite_const_ct suite);
size_t         test_suite_get_size(test_suite_const_ct suite);
test_suite_ct  test_suite_get_suite(test_suite_const_ct suite, const char *name);
test_case_ct   test_suite_get_case(test_suite_const_ct suite, const char *name);

test_suite_ct  test_suite_add_suite(test_suite_ct suite, test_suite_ct sub);
#define        test_suite_add_suites(suite, ...) \
              _test_suite_add_suites(suite, __VA_ARGS__, TEST_SUITE_SENTINEL)
test_suite_ct _test_suite_add_suites(test_suite_ct suite, ...);
test_suite_ct  test_suite_add_suites_v(test_suite_ct suite, va_list ap);

test_suite_ct  test_suite_add_case(test_suite_ct suite, test_case_ct tcase);
#define        test_suite_add_cases(suite, ...) \
              _test_suite_add_cases(suite, __VA_ARGS__, TEST_SUITE_SENTINEL)
test_suite_ct _test_suite_add_cases(test_suite_ct suite, ...);
test_suite_ct  test_suite_add_cases_v(test_suite_ct suite, va_list ap);

int test_suite_fold(test_suite_const_ct suite, test_suite_fold_cb fold, void *ctx);

#endif
