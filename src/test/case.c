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

#include <ytil/test/case.h>
#include <ytil/def.h>
#include <stdlib.h>


enum def
{
      TEST_CASE_TIMEOUT = 5 // sec
};

typedef struct test_case
{
    const char *name;
    test_case_cb run, setup, teardown;
    size_t timeout;
    test_case_end_id end;
    int endval;
} test_case_st;

/// test_case error type definition
ERROR_DEFINE_LIST(TEST_CASE,
      ERROR_INFO(E_TEST_CASE_INVALID_OBJECT, "Invalid test case object.")
    , ERROR_INFO(E_TEST_CASE_INVALID_NAME, "Invalid test case name.")
    , ERROR_INFO(E_TEST_CASE_INVALID_CALLBACK, "Invalid test case callback.")
);

/// default error type for test_case module
#define ERROR_TYPE_DEFAULT ERROR_TYPE_TEST_CASE


test_case_ct _test_case_new(const char *name, test_case_cb run, const test_case_config_st *config)
{
    test_case_ct tcase;
    
    return_error_if_fail(name, E_TEST_CASE_INVALID_NAME, NULL);
    return_error_if_fail(run, E_TEST_CASE_INVALID_CALLBACK, NULL);
    
    if(!(tcase = calloc(1, sizeof(test_case_st))))
        return error_wrap_errno(calloc), NULL;
    
    tcase->name = name;
    tcase->run = run;
    tcase->timeout = TEST_CASE_TIMEOUT;
    
    if(config)
    {
        tcase->setup = config->setup;
        tcase->teardown = config->teardown;
        tcase->end = config->end;
        tcase->endval = config->endval;
    }
    
    return tcase;
}

void test_case_free(test_case_ct tcase)
{
    assert(tcase);
    
    if(tcase != TEST_CASE_NOP)
        free(tcase);
}

void test_case_set_timeout(test_case_ct tcase, size_t ms)
{
    assert(tcase);
    
    tcase->timeout = ms;
}

void test_case_set_exit(test_case_ct tcase, int rc)
{
    assert(tcase);
    
    tcase->end = TEST_CASE_END_EXIT;
    tcase->endval = rc;
}

void test_case_set_signal(test_case_ct tcase, int signal)
{
    assert(tcase);
    
    tcase->end = TEST_CASE_END_SIGNAL;
    tcase->endval = signal;
}

int test_case_set_fixture(test_case_ct tcase, test_case_cb setup, test_case_cb teardown)
{
    return_error_if_fail(tcase, E_TEST_CASE_INVALID_OBJECT, -1);
    return_error_if_fail(setup && teardown, E_TEST_CASE_INVALID_CALLBACK, -1);
    
    tcase->setup = setup;
    tcase->teardown = teardown;
    
    return 0;
}

bool test_case_expects_nothing(test_case_const_ct tcase)
{
    return tcase->end == TEST_CASE_END_NORMAL;
}

bool test_case_expects_exit(test_case_const_ct tcase)
{
    return tcase->end == TEST_CASE_END_EXIT;
}

bool test_case_expects_signal(test_case_const_ct tcase)
{
    return tcase->end == TEST_CASE_END_SIGNAL;
}

const char *test_case_get_name(test_case_const_ct tcase)
{
    return_error_if_fail(tcase, E_TEST_CASE_INVALID_OBJECT, NULL);
    
    return tcase->name;
}

size_t test_case_get_timeout(test_case_const_ct tcase)
{
    return_value_if_fail(tcase, TEST_CASE_TIMEOUT);
    
    return tcase->timeout;
}

int test_case_get_exit(test_case_const_ct tcase)
{
    return_value_if_fail(tcase && tcase->end == TEST_CASE_END_EXIT, 0);
    
    return tcase->endval;
}

int test_case_get_signal(test_case_const_ct tcase)
{
    return_value_if_fail(tcase && tcase->end == TEST_CASE_END_SIGNAL, 0);
    
    return tcase->endval;
}

test_case_cb test_case_get_run(test_case_const_ct tcase)
{
    return_error_if_fail(tcase, E_TEST_CASE_INVALID_OBJECT, NULL);
    
    return tcase->run;
}

test_case_cb test_case_get_setup(test_case_const_ct tcase)
{
    return_error_if_fail(tcase, E_TEST_CASE_INVALID_OBJECT, NULL);
    
    return tcase->setup;
}

test_case_cb test_case_get_teardown(test_case_const_ct tcase)
{
    return_error_if_fail(tcase, E_TEST_CASE_INVALID_OBJECT, NULL);
    
    return tcase->teardown;
}
