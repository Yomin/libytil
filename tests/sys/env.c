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

#include "env.h"
#include <ytil/test/test.h>
#include <ytil/sys/env.h>

str_const_ct value, str;


TEST_SETUP(env_init)
{
    test_int_success(env_init());
}

TEST_TEARDOWN(env_free)
{
    test_void(env_free());
}

TEST_CASE_ABORT_FIXTURE(env_is_set_invalid_name1, env_init, env_free)
{
    env_is_set(NULL);
}

TEST_CASE_FIXTURE(env_is_set_invalid_name2, env_init, env_free)
{
    test_false(env_is_set(LIT("")));
    test_error(0, E_ENV_INVALID_NAME);
}

TEST_CASE_FIXTURE(env_is_set_false, env_init, env_free)
{
    test_false(env_is_set(LIT("YTIL_ENV_TEST")));
}

TEST_CASE_FIXTURE(env_is_set_true, env_init, env_free)
{
    test_true(env_is_set(LIT("PATH")));
}

TEST_CASE_ABORT_FIXTURE(env_get_invalid_name1, env_init, env_free)
{
    env_get(NULL);
}

TEST_CASE_FIXTURE(env_get_invalid_name2, env_init, env_free)
{
    test_ptr_error(env_get(LIT("")), E_ENV_INVALID_NAME);
}

TEST_CASE_FIXTURE(env_get_not_found, env_init, env_free)
{
    test_ptr_error(env_get(LIT("YTIL_ENV_TEST")), E_ENV_NOT_FOUND);
}

TEST_CASE_FIXTURE(env_get, env_init, env_free)
{
    test_ptr_success(env_get(LIT("PATH")));
}

TEST_CASE_ABORT_FIXTURE(env_set_invalid_name1, env_init, env_free)
{
    env_set(NULL, LIT("foo"));
}

TEST_CASE_FIXTURE(env_set_invalid_name2, env_init, env_free)
{
    test_int_error(env_set(LIT(""), LIT("foo")), E_ENV_INVALID_NAME);
}

TEST_CASE_ABORT_FIXTURE(env_set_invalid_value, env_init, env_free)
{
    env_set(LIT("foo"), NULL);
}

TEST_CASE_FIXTURE(env_set_new, env_init, env_free)
{
    test_int_success(env_set(LIT("YTIL_ENV_TEST"), LIT("test")));
    test_true(env_is_set(LIT("YTIL_ENV_TEST")));
    test_ptr_success(value = env_get(LIT("YTIL_ENV_TEST")));
    test_str_eq(str_c(value), "test");
}

TEST_CASE_FIXTURE(env_set_new_empty, env_init, env_free)
{
    test_int_success(env_set(LIT("YTIL_ENV_TEST"), LIT("")));
    test_true(env_is_set(LIT("YTIL_ENV_TEST")));
    test_ptr_success(value = env_get(LIT("YTIL_ENV_TEST")));
    test_true(str_is_empty(value));
}

TEST_CASE_FIXTURE(env_set_overwrite_def, env_init, env_free)
{
    test_int_success(env_set(LIT("PATH"), LIT("foo")));
    test_true(env_is_set(LIT("PATH")));
    test_ptr_success(value = env_get(LIT("PATH")));
    test_str_eq(str_c(value), "foo");
}

TEST_CASE_FIXTURE(env_set_overwrite_new, env_init, env_free)
{
    test_int_success(env_set(LIT("YTIL_ENV_TEST"), LIT("foo")));
    test_int_success(env_set(LIT("YTIL_ENV_TEST"), LIT("bar")));
    test_true(env_is_set(LIT("YTIL_ENV_TEST")));
    test_ptr_success(value = env_get(LIT("YTIL_ENV_TEST")));
    test_str_eq(str_c(value), "bar");
}

TEST_CASE_FIXTURE(env_set_unset_def, env_init, env_free)
{
    test_int_success(env_unset(LIT("PATH")));
    test_int_success(env_set(LIT("PATH"), LIT("foo")));
    test_true(env_is_set(LIT("PATH")));
    test_ptr_success(value = env_get(LIT("PATH")));
    test_str_eq(str_c(value), "foo");
}

TEST_CASE_ABORT_FIXTURE(env_reset_invalid_name1, env_init, env_free)
{
    env_reset(NULL);
}

TEST_CASE_FIXTURE(env_reset_invalid_name2, env_init, env_free)
{
    test_int_error(env_reset(LIT("")), E_ENV_INVALID_NAME);
}

TEST_CASE_FIXTURE(env_reset_missing, env_init, env_free)
{
    test_int_success(env_reset(LIT("YTIL_ENV_TEST")));
    test_false(env_is_set(LIT("YTIL_ENV_TEST")));
    test_ptr_error(env_get(LIT("YTIL_ENV_TEST")), E_ENV_NOT_FOUND);
}

TEST_CASE_FIXTURE(env_reset_def, env_init, env_free)
{
    test_int_success(env_set(LIT("PATH"), LIT("test")));
    test_int_success(env_reset(LIT("PATH")));
    test_true(env_is_set(LIT("PATH")));
    test_ptr_success(value = env_get(LIT("PATH")));
    test_str_ne(str_c(value), "test");
}

TEST_CASE_FIXTURE(env_reset_new, env_init, env_free)
{
    test_int_success(env_set(LIT("YTIL_ENV_TEST"), LIT("test")));
    test_int_success(env_reset(LIT("YTIL_ENV_TEST")));
    test_false(env_is_set(LIT("YTIL_ENV_TEST")));
    test_ptr_error(env_get(LIT("YTIL_ENV_TEST")), E_ENV_NOT_FOUND);
}

TEST_CASE_FIXTURE(env_reset_unset_def, env_init, env_free)
{
    test_int_success(env_unset(LIT("PATH")));
    test_int_success(env_reset(LIT("PATH")));
    test_true(env_is_set(LIT("PATH")));
    test_ptr_success(env_get(LIT("PATH")));
}

TEST_CASE_ABORT_FIXTURE(env_unset_invalid_name1, env_init, env_free)
{
    env_unset(NULL);
}

TEST_CASE_FIXTURE(env_unset_invalid_name2, env_init, env_free)
{
    test_int_error(env_unset(LIT("")), E_ENV_INVALID_NAME);
}

TEST_CASE_FIXTURE(env_unset_missing, env_init, env_free)
{
    test_int_success(env_unset(LIT("YTIL_ENV_TEST")));
    test_false(env_is_set(LIT("YTIL_ENV_TEST")));
    test_ptr_error(env_get(LIT("YTIL_ENV_TEST")), E_ENV_NOT_FOUND);
}

TEST_CASE_FIXTURE(env_unset_def, env_init, env_free)
{
    test_int_success(env_unset(LIT("PATH")));
    test_false(env_is_set(LIT("YTIL_ENV_TEST")));
    test_ptr_error(env_get(LIT("PATH")), E_ENV_NOT_FOUND);
}

TEST_CASE_FIXTURE(env_unset_new, env_init, env_free)
{
    test_int_success(env_set(LIT("YTIL_ENV_TEST"), LIT("test")));
    test_int_success(env_unset(LIT("YTIL_ENV_TEST")));
    test_false(env_is_set(LIT("YTIL_ENV_TEST")));
    test_ptr_error(env_get(LIT("YTIL_ENV_TEST")), E_ENV_NOT_FOUND);
}

TEST_CASE_FIXTURE(env_unset_unset_def, env_init, env_free)
{
    test_int_success(env_unset(LIT("PATH")));
    test_int_success(env_unset(LIT("PATH")));
    test_false(env_is_set(LIT("PATH")));
    test_ptr_error(env_get(LIT("PATH")), E_ENV_NOT_FOUND);
}

test_suite_ct test_suite_sys_env(void)
{
    return test_suite_new_with_cases("env"
        , test_case_new(env_is_set_invalid_name1)
        , test_case_new(env_is_set_invalid_name2)
        , test_case_new(env_is_set_false)
        , test_case_new(env_is_set_true)
        
        , test_case_new(env_get_invalid_name1)
        , test_case_new(env_get_invalid_name2)
        , test_case_new(env_get_not_found)
        , test_case_new(env_get)
        
        , test_case_new(env_set_invalid_name1)
        , test_case_new(env_set_invalid_name2)
        , test_case_new(env_set_invalid_value)
        , test_case_new(env_set_new)
        , test_case_new(env_set_new_empty)
        , test_case_new(env_set_overwrite_def)
        , test_case_new(env_set_overwrite_new)
        , test_case_new(env_set_unset_def)
        
        , test_case_new(env_reset_invalid_name1)
        , test_case_new(env_reset_invalid_name2)
        , test_case_new(env_reset_missing)
        , test_case_new(env_reset_def)
        , test_case_new(env_reset_new)
        , test_case_new(env_reset_unset_def)
        
        , test_case_new(env_unset_invalid_name1)
        , test_case_new(env_unset_invalid_name2)
        , test_case_new(env_unset_missing)
        , test_case_new(env_unset_def)
        , test_case_new(env_unset_new)
        , test_case_new(env_unset_unset_def)
    );
}
