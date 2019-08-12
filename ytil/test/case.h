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

#ifndef __YTIL_TEST_CASE_H__
#define __YTIL_TEST_CASE_H__

#include <ytil/ext/time.h>
#include <stddef.h>
#include <stdbool.h>
#include <signal.h>


struct test_case;
struct test_suite;

typedef       struct test_case *test_case_ct;
typedef const struct test_case *test_case_const_ct;


typedef void (*test_case_cb)(void *ctx, void **state);

typedef enum test_case_end
{
      TEST_CASE_END_ANY
    , TEST_CASE_END_EXIT
    , TEST_CASE_END_SIGNAL
} test_case_end_id;

typedef struct test_case_config
{
    test_case_cb setup, teardown;
    test_case_end_id end;
    int endval;
} test_case_config_st;


#define _TEST_CASE(name, _setup, _teardown, _end, _endval) \
    \
    static const test_case_config_st _test_config_##name = { \
        .setup = _setup, .teardown = _teardown, .end = _end, .endval = _endval }; \
    \
    static void _test_case_##name(void *_test_case_ctx, void **_test_case_state)

#define TEST_CASE(name) \
    _TEST_CASE(name, NULL, NULL, TEST_CASE_END_ANY, 0)
#define TEST_CASE_EXIT(name, rc) \
    _TEST_CASE(name, NULL, NULL, TEST_CASE_END_EXIT, rc)
#define TEST_CASE_SIGNAL(name, signal) \
    _TEST_CASE(name, NULL, NULL, TEST_CASE_END_SIGNAL, signal)

#define TEST_CASE_FIXTURE(name, setup, teardown) \
    _TEST_CASE(name, _test_setup_##setup, _test_teardown_##teardown, TEST_CASE_END_ANY, 0)
#define TEST_CASE_FIXTURE_EXIT(name, setup, teardown, rc) \
    _TEST_CASE(name, _test_setup_##setup, _test_teardown_##teardown, TEST_CASE_END_EXIT, rc)
#define TEST_CASE_FIXTURE_SIGNAL(name, setup, teardown, signal) \
    _TEST_CASE(name, _test_setup_##setup, _test_teardown_##teardown, TEST_CASE_END_SIGNAL, signal)

#define TEST_SETUP(name) \
    static void _test_setup_##name(void *_test_case_ctx, void **_test_case_state)

#define TEST_TEARDOWN(name) \
    static void _test_teardown_##name(void *_test_case_ctx, void **_test_case_state)

#define TEST_STATE \
    (*_test_case_state)


#define       test_case_new(name) \
             _test_case_new(#name, _test_case_##name, &_test_config_##name)
test_case_ct _test_case_new(const char *name, test_case_cb run, const test_case_config_st *config);
void          test_case_free(test_case_ct tcase);

void test_case_set_timeout(test_case_ct tcase, size_t ms);
void test_case_set_exit(test_case_ct tcase, int rc);
void test_case_set_signal(test_case_ct tcase, int signal);
int  test_case_set_fixture(test_case_ct tcase, test_case_cb setup, test_case_cb teardown);

bool test_case_expects_exit(test_case_const_ct tcase);
bool test_case_expects_signal(test_case_const_ct tcase);

const char  *test_case_get_name(test_case_const_ct tcase);
size_t       test_case_get_timeout(test_case_const_ct tcase);
int          test_case_get_exit(test_case_const_ct tcase);
int          test_case_get_signal(test_case_const_ct tcase);
test_case_cb test_case_get_run(test_case_const_ct tcase);
test_case_cb test_case_get_setup(test_case_const_ct tcase);
test_case_cb test_case_get_teardown(test_case_const_ct tcase);
void        *test_case_get_state(test_case_const_ct tcase);

#endif