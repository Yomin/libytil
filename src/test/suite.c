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

#include <ytil/test/suite.h>
#include <ytil/def.h>
#include <stdlib.h>
#include <string.h>
#include <ytil/con/vec.h>

typedef struct test_suite
{
    const char *name;
    vec_ct entries;
} test_suite_st;

typedef struct test_suite_fold_state
{
    test_suite_const_ct suite;
    test_suite_fold_cb fold;
    void *ctx;
} test_suite_fold_st;

static const error_info_st error_infos[] =
{
     ERROR_INFO(E_TEST_SUITE_INVALID_CALLBACK, "Invalid callback.")
   , ERROR_INFO(E_TEST_SUITE_INVALID_NAME, "Invalid name.")
   , ERROR_INFO(E_TEST_SUITE_INVALID_OBJECT, "Invalid test suite object.")
   , ERROR_INFO(E_TEST_SUITE_NOT_FOUND, "Test suite/case not found.")
};


test_suite_ct test_suite_new(const char *name)
{
    test_suite_ct suite;
    
    return_error_if_fail(name, E_TEST_SUITE_INVALID_NAME, NULL);
    
    if(!(suite = calloc(1, sizeof(test_suite_st))))
        return error_wrap_errno(calloc), NULL;
    
    suite->name = name;
    
    return suite;
}

test_suite_ct test_suite_new_with_suite(const char *name, test_suite_ct sub)
{
    test_suite_ct suite;
    
    if(!(suite = test_suite_new(name)))
        return error_propagate(), NULL;
    
    if(!(suite = test_suite_add_suite(suite, sub)))
        return error_propagate(), NULL;
    
    return sub;
}

test_suite_ct _test_suite_new_with_suites(const char *name, ...)
{
    test_suite_ct suite;
    va_list ap;
    
    if(!(suite = test_suite_new(name)))
        return error_propagate(), NULL;
    
    va_start(ap, name);
    suite = error_propagate_ptr(test_suite_add_suites_v(suite, ap));
    va_end(ap);
    
    return suite;
}

test_suite_ct test_suite_new_with_suites_v(const char *name, va_list ap)
{
    test_suite_ct suite;
    
    if(!(suite = test_suite_new(name)))
        return error_propagate(), NULL;
    
    if(!(suite = test_suite_add_suites_v(suite, ap)))
        return error_propagate(), NULL;
    
    return suite;
}

test_suite_ct test_suite_new_with_case(const char *name, test_case_ct tcase)
{
    test_suite_ct suite;
    
    if(!(suite = test_suite_new(name)))
        return error_propagate(), NULL;
    
    if(!(suite = test_suite_add_case(suite, tcase)))
        return error_propagate(), NULL;
    
    return suite;
}

test_suite_ct _test_suite_new_with_cases(const char *name, ...)
{
    test_suite_ct suite;
    va_list ap;
    
    if(!(suite = test_suite_new(name)))
        return error_propagate(), NULL;
    
    va_start(ap, name);
    suite = error_propagate_ptr(test_suite_add_cases_v(suite, ap));
    va_end(ap);
    
    return suite;
}

test_suite_ct test_suite_new_with_cases_v(const char *name, va_list ap)
{
    test_suite_ct suite;
    
    if(!(suite = test_suite_new(name)))
        return error_propagate(), NULL;
    
    if(!(suite = test_suite_add_cases_v(suite, ap)))
        return error_propagate(), NULL;
    
    return suite;
}

static void test_entry_free(test_entry_id type, void *entry)
{
    switch(type)
    {
    case TEST_ENTRY_SUITE:  test_suite_free(entry); break;
    case TEST_ENTRY_CASE:   test_case_free(entry); break;
    default:                abort();
    }
}

static void test_suite_vec_free_entry(vec_const_ct vec, void *elem, void *ctx)
{
    test_entry_st *entry = elem;
    
    test_entry_free(entry->type, entry->value.entry);
}

void test_suite_free(test_suite_ct suite)
{
    assert(suite);
    
    if(suite->entries)
        vec_free_f(suite->entries, test_suite_vec_free_entry, NULL);
    
    free(suite);
}

const char *test_suite_get_name(test_suite_const_ct suite)
{
    return_error_if_fail(suite, E_TEST_SUITE_INVALID_OBJECT, NULL);
    
    return suite->name;
}

size_t test_suite_get_size(test_suite_const_ct suite)
{
    return_value_if_fail(suite, 0);
    
    return suite->entries ? vec_size(suite->entries) : 0;
}

static bool test_suite_vec_find_suite(vec_const_ct vec, const void *elem, void *ctx)
{
    const test_entry_st *entry = elem;
    const char *name = ctx;
    
    return entry->type == TEST_ENTRY_SUITE && !strcmp(name, entry->value.suite->name);
}

test_suite_ct test_suite_get_suite(test_suite_const_ct suite, const char *name)
{
    test_entry_st *entry;
    
    return_error_if_fail(suite, E_TEST_SUITE_INVALID_OBJECT, NULL);
    return_error_if_fail(name, E_TEST_SUITE_INVALID_NAME, NULL);
    return_error_if_fail(suite->entries, E_TEST_SUITE_NOT_FOUND, NULL);
    
    if(!(entry = vec_find(suite->entries, test_suite_vec_find_suite, (void*)name)))
        return error_set(E_TEST_SUITE_NOT_FOUND), NULL;
    
    return entry->value.suite;
}

static bool test_suite_vec_find_case(vec_const_ct vec, const void *elem, void *ctx)
{
    const test_entry_st *entry = elem;
    const char *name = ctx;
    
    return entry->type == TEST_ENTRY_CASE
        && !strcmp(name, test_case_get_name(entry->value.tcase));
}

test_case_ct test_suite_get_case(test_suite_const_ct suite, const char *name)
{
    test_entry_st *entry;
    
    return_error_if_fail(suite, E_TEST_SUITE_INVALID_OBJECT, NULL);
    return_error_if_fail(name, E_TEST_SUITE_INVALID_NAME, NULL);
    return_error_if_fail(suite->entries, E_TEST_SUITE_NOT_FOUND, NULL);
    
    if(!(entry = vec_find(suite->entries, test_suite_vec_find_case, (void*)name)))
        return error_set(E_TEST_SUITE_NOT_FOUND), NULL;
    
    return entry->value.tcase;
}

static test_suite_ct test_suite_add_entry(test_suite_ct suite, test_entry_id type, void *entry)
{
    test_entry_st *sentry;
    
    if(!suite && entry)
        test_entry_free(type, entry);
    
    return_error_if_fail(suite, E_TEST_SUITE_INVALID_OBJECT, NULL);
    
    if(!entry)
        return error_wrap(), test_suite_free(suite), NULL;
    
    if(!suite->entries && !(suite->entries = vec_new(10, sizeof(test_entry_st))))
        return error_wrap(), test_suite_free(suite), test_entry_free(type, entry), NULL;
    
    if(!(sentry = vec_push(suite->entries)))
        return error_wrap(), test_suite_free(suite), test_entry_free(type, entry), NULL;
    
    sentry->type = type;
    sentry->value.entry = entry;
    
    return suite;
}

test_suite_ct test_suite_add_suite(test_suite_ct suite, test_suite_ct sub)
{
    return error_propagate_ptr(test_suite_add_entry(suite, TEST_ENTRY_SUITE, sub));
}

test_suite_ct _test_suite_add_suites(test_suite_ct suite, ...)
{
    va_list ap;
    
    va_start(ap, suite);
    suite = error_propagate_ptr(test_suite_add_suites_v(suite, ap));
    va_end(ap);
    
    return suite;
}

test_suite_ct test_suite_add_suites_v(test_suite_ct suite, va_list ap)
{
    test_suite_ct sub, rc = suite;
    
    while(1)
    {
        if(!(sub = va_arg(ap, test_suite_ct)))
            rc = NULL;
        else if(sub == TEST_SUITE_SENTINEL)
            break;
        else if(!rc)
            test_suite_free(sub);
        else
            rc = suite = test_suite_add_entry(suite, TEST_ENTRY_SUITE, sub);
    }
    
    if(rc)
        return rc;
    
    if(suite)
        test_suite_free(suite);
    
    return error_wrap(), NULL;
}

test_suite_ct test_suite_add_case(test_suite_ct suite, test_case_ct tcase)
{
    return error_propagate_ptr(test_suite_add_entry(suite, TEST_ENTRY_CASE, tcase));
}

test_suite_ct _test_suite_add_cases(test_suite_ct suite, ...)
{
    va_list ap;
    
    va_start(ap, suite);
    suite = error_propagate_ptr(test_suite_add_cases_v(suite, ap));
    va_end(ap);
    
    return suite;
}

test_suite_ct test_suite_add_cases_v(test_suite_ct suite, va_list ap)
{
    test_suite_ct rc = suite;
    test_case_ct tcase;
    
    while(1)
    {
        if(!(tcase = va_arg(ap, test_case_ct)))
            rc = NULL;
        else if(tcase == TEST_SUITE_SENTINEL)
            break;
        else if(!rc)
            test_case_free(tcase);
        else
            rc = suite = test_suite_add_entry(suite, TEST_ENTRY_CASE, tcase);
    }
    
    if(rc)
        return rc;
    
    if(suite)
        test_suite_free(suite);
    
    return error_wrap(), NULL;
}

static int test_suite_vec_fold_entry(vec_const_ct vec, size_t index, void *elem, void *ctx)
{
    test_suite_fold_st *state = ctx;
    test_entry_st *entry = elem;
    
    return error_wrap_int(state->fold(state->suite, entry, state->ctx));
}

int test_suite_fold(test_suite_const_ct suite, test_suite_fold_cb fold, void *ctx)
{
    test_suite_fold_st state = { .suite = suite, .fold = fold, .ctx = ctx };
    
    return_error_if_fail(suite, E_TEST_SUITE_INVALID_OBJECT, -1);
    return_error_if_fail(fold, E_TEST_SUITE_INVALID_CALLBACK, -1);
    
    return error_wrap_int(vec_fold(suite->entries, test_suite_vec_fold_entry, &state));
}
