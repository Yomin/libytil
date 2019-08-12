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


test_suite_ct test_suite_new(const char *name)
{
    test_suite_ct suite;
    
    return_err_if_fail(name, EINVAL, NULL);
    
    if(!(suite = calloc(1, sizeof(test_suite_st))))
        return NULL;
    
    suite->name = name;
    
    return suite;
}

test_suite_ct test_suite_new_with_suite(const char *name, test_suite_ct sub)
{
    return test_suite_add_suite(test_suite_new(name), sub);
}

test_suite_ct _test_suite_new_with_suites(const char *name, ...)
{
    test_suite_ct suite;
    va_list ap;
    
    va_start(ap, name);
    suite = test_suite_add_suites_v(test_suite_new(name), ap);
    va_end(ap);
    
    return suite;
}

test_suite_ct test_suite_new_with_suites_v(const char *name, va_list ap)
{
    return test_suite_add_suites_v(test_suite_new(name), ap);
}

test_suite_ct test_suite_new_with_case(const char *name, test_case_ct tcase)
{
    return test_suite_add_case(test_suite_new(name), tcase);
}

test_suite_ct _test_suite_new_with_cases(const char *name, ...)
{
    test_suite_ct suite;
    va_list ap;
    
    va_start(ap, name);
    suite = test_suite_add_cases_v(test_suite_new(name), ap);
    va_end(ap);
    
    return suite;
}

test_suite_ct test_suite_new_with_cases_v(const char *name, va_list ap)
{
    return test_suite_add_cases_v(test_suite_new(name), ap);
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
    return_err_if_fail(suite, EINVAL, NULL);
    
    return suite->name;
}

size_t test_suite_get_size(test_suite_const_ct suite)
{
    return_err_if_fail(suite, EINVAL, 0);
    
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
    
    return_err_if_fail(suite && name, EINVAL, NULL);
    return_err_if_fail(suite->entries, ENOENT, NULL);
    
    if(!(entry = vec_find(suite->entries, test_suite_vec_find_suite, (void*)name)))
        return NULL;
    
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
    
    return_err_if_fail(suite && name, EINVAL, NULL);
    return_err_if_fail(suite->entries, ENOENT, NULL);
    
    if(!(entry = vec_find(suite->entries, test_suite_vec_find_case, (void*)name)))
        return NULL;
    
    return entry->value.tcase;
}

static test_suite_ct test_suite_add_entry(test_suite_ct suite, test_entry_id type, void *entry)
{
    test_entry_st *sentry;
    
    if(!suite && !entry)
        return errno = EINVAL, NULL;
    
    if(!suite && entry)
        return test_entry_free(type, entry), errno = EINVAL, NULL;
    
    if(!entry)
        return test_suite_free(suite), NULL;
    
    if(!suite->entries && !(suite->entries = vec_new(10, sizeof(test_entry_st))))
        return test_suite_free(suite), test_entry_free(type, entry), NULL;
    
    if(!(sentry = vec_push(suite->entries)))
        return test_suite_free(suite), test_entry_free(type, entry), NULL;
    
    sentry->type = type;
    sentry->value.entry = entry;
    
    return suite;
}

test_suite_ct test_suite_add_suite(test_suite_ct suite, test_suite_ct sub)
{
    return test_suite_add_entry(suite, TEST_ENTRY_SUITE, sub);
}

test_suite_ct _test_suite_add_suites(test_suite_ct suite, ...)
{
    va_list ap;
    
    va_start(ap, suite);
    suite = test_suite_add_suites_v(suite, ap);
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
    
    if(!rc && suite)
        test_suite_free(suite);
    
    return rc;
}

test_suite_ct test_suite_add_case(test_suite_ct suite, test_case_ct tcase)
{
    return test_suite_add_entry(suite, TEST_ENTRY_CASE, tcase);
}

test_suite_ct _test_suite_add_cases(test_suite_ct suite, ...)
{
    va_list ap;
    
    va_start(ap, suite);
    suite = test_suite_add_cases_v(suite, ap);
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
    
    if(!rc && suite)
        test_suite_free(suite);
    
    return rc;
}

static int test_suite_vec_fold_entry(vec_const_ct vec, size_t index, void *elem, void *ctx)
{
    test_suite_fold_st *state = ctx;
    test_entry_st *entry = elem;
    
    return state->fold(state->suite, entry, state->ctx);
}

int test_suite_fold(test_suite_const_ct suite, test_suite_fold_cb fold, void *ctx)
{
    test_suite_fold_st state = { .suite = suite, .fold = fold, .ctx = ctx };
    
    return_err_if_fail(suite && fold, EINVAL, -1);
    
    return vec_fold(suite->entries, test_suite_vec_fold_entry, &state);
}
