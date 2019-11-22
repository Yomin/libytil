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

#ifndef _WIN32
#   include <pwd.h>
#endif

str_const_ct value, str;
path_ct path;


TEST_SETUP(env_init)
{
    test_int_success(env_init());
}

TEST_TEARDOWN(env_free)
{
    test_void(env_free());
}

TEST_CASE_FIXTURE_SIGNAL(env_get_invalid_name1, env_init, env_free, SIGABRT)
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

TEST_CASE_FIXTURE_SIGNAL(env_set_invalid_name1, env_init, env_free, SIGABRT)
{
    env_set(NULL, LIT("foo"));
}

TEST_CASE_FIXTURE(env_set_invalid_name2, env_init, env_free)
{
    test_int_error(env_set(LIT(""), LIT("foo")), E_ENV_INVALID_NAME);
}

TEST_CASE_FIXTURE_SIGNAL(env_set_invalid_value, env_init, env_free, SIGABRT)
{
    env_set(LIT("foo"), NULL);
}

TEST_CASE_FIXTURE(env_set_new, env_init, env_free)
{
    test_int_success(env_set(LIT("YTIL_ENV_TEST"), LIT("test")));
    test_ptr_success(value = env_get(LIT("YTIL_ENV_TEST")));
    test_str_eq(str_c(value), "test");
}

TEST_CASE_FIXTURE(env_set_new_empty, env_init, env_free)
{
    test_int_success(env_set(LIT("YTIL_ENV_TEST"), LIT("")));
    test_ptr_success(value = env_get(LIT("YTIL_ENV_TEST")));
    test_true(str_is_empty(value));
}

TEST_CASE_FIXTURE(env_set_overwrite_def, env_init, env_free)
{
    test_int_success(env_set(LIT("PATH"), LIT("foo")));
    test_ptr_success(value = env_get(LIT("PATH")));
    test_str_eq(str_c(value), "foo");
}

TEST_CASE_FIXTURE(env_set_overwrite_new, env_init, env_free)
{
    test_int_success(env_set(LIT("YTIL_ENV_TEST"), LIT("foo")));
    test_int_success(env_set(LIT("YTIL_ENV_TEST"), LIT("bar")));
    test_ptr_success(value = env_get(LIT("YTIL_ENV_TEST")));
    test_str_eq(str_c(value), "bar");
}

TEST_CASE_FIXTURE(env_set_unset_def, env_init, env_free)
{
    test_int_success(env_unset(LIT("PATH")));
    test_int_success(env_set(LIT("PATH"), LIT("foo")));
    test_ptr_success(value = env_get(LIT("PATH")));
    test_str_eq(str_c(value), "foo");
}

TEST_CASE_FIXTURE_SIGNAL(env_reset_invalid_name1, env_init, env_free, SIGABRT)
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
    test_ptr_error(env_get(LIT("YTIL_ENV_TEST")), E_ENV_NOT_FOUND);
}

TEST_CASE_FIXTURE(env_reset_def, env_init, env_free)
{
    test_int_success(env_set(LIT("PATH"), LIT("test")));
    test_int_success(env_reset(LIT("PATH")));
    test_ptr_success(value = env_get(LIT("PATH")));
    test_str_ne(str_c(value), "test");
}

TEST_CASE_FIXTURE(env_reset_new, env_init, env_free)
{
    test_int_success(env_set(LIT("YTIL_ENV_TEST"), LIT("test")));
    test_int_success(env_reset(LIT("YTIL_ENV_TEST")));
    test_ptr_error(env_get(LIT("YTIL_ENV_TEST")), E_ENV_NOT_FOUND);
}

TEST_CASE_FIXTURE(env_reset_unset_def, env_init, env_free)
{
    test_int_success(env_unset(LIT("PATH")));
    test_int_success(env_reset(LIT("PATH")));
    test_ptr_success(env_get(LIT("PATH")));
}

TEST_CASE_FIXTURE_SIGNAL(env_unset_invalid_name1, env_init, env_free, SIGABRT)
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
    test_ptr_error(env_get(LIT("YTIL_ENV_TEST")), E_ENV_NOT_FOUND);
}

TEST_CASE_FIXTURE(env_unset_def, env_init, env_free)
{
    test_int_success(env_unset(LIT("PATH")));
    test_ptr_error(env_get(LIT("PATH")), E_ENV_NOT_FOUND);
}

TEST_CASE_FIXTURE(env_unset_new, env_init, env_free)
{
    test_int_success(env_set(LIT("YTIL_ENV_TEST"), LIT("test")));
    test_int_success(env_unset(LIT("YTIL_ENV_TEST")));
    test_ptr_error(env_get(LIT("YTIL_ENV_TEST")), E_ENV_NOT_FOUND);
}

TEST_CASE_FIXTURE(env_unset_unset_def, env_init, env_free)
{
    test_int_success(env_unset(LIT("PATH")));
    test_int_success(env_unset(LIT("PATH")));
    test_ptr_error(env_get(LIT("PATH")), E_ENV_NOT_FOUND);
}

TEST_CASE_FIXTURE_SIGNAL(env_get_path_invalid_ident, env_init, env_free, SIGABRT)
{
    env_get_path(999, ENV_MODE_NATIVE);
}

TEST_CASE_FIXTURE_SIGNAL(env_get_path_invalid_mode, env_init, env_free, SIGABRT)
{
    env_get_path(ENV_PATH_USER_HOME, 999);
}

TEST_CASE_FIXTURE(env_get_path_user_cache_native, env_init, env_free)
{
    test_int_success(env_set(LIT("XDG_CACHE_HOME"), LIT("/home/foo/my_cache")));
    test_ptr_success(path = env_get_path(ENV_PATH_USER_CACHE, ENV_MODE_NATIVE));
    test_ptr_success(str = path_get(path, PATH_STYLE_POSIX));
    test_str_eq(str_c(str), "/home/foo/my_cache");
    str_unref(str);
}

TEST_CASE_FIXTURE(env_get_path_user_cache_native_def, env_init, env_free)
{
    test_int_success(env_set(LIT("HOME"), LIT("/home/foo")));
    test_int_success(env_unset(LIT("XDG_CACHE_HOME")));
    test_ptr_success(path = env_get_path(ENV_PATH_USER_CACHE, ENV_MODE_NATIVE));
    test_ptr_success(str = path_get(path, PATH_STYLE_POSIX));
    test_str_eq(str_c(str), "/home/foo/.cache");
    str_unref(str);
}

TEST_CASE_FIXTURE(env_get_path_user_config_native, env_init, env_free)
{
    test_int_success(env_set(LIT("XDG_CONFIG_HOME"), LIT("/home/foo/my_config")));
    test_ptr_success(path = env_get_path(ENV_PATH_USER_CONFIG, ENV_MODE_NATIVE));
    test_ptr_success(str = path_get(path, PATH_STYLE_POSIX));
    test_str_eq(str_c(str), "/home/foo/my_config");
    str_unref(str);
}

TEST_CASE_FIXTURE(env_get_path_user_config_native_def, env_init, env_free)
{
    test_int_success(env_set(LIT("HOME"), LIT("/home/foo")));
    test_int_success(env_unset(LIT("XDG_CONFIG_HOME")));
    test_ptr_success(path = env_get_path(ENV_PATH_USER_CONFIG, ENV_MODE_NATIVE));
    test_ptr_success(str = path_get(path, PATH_STYLE_POSIX));
    test_str_eq(str_c(str), "/home/foo/.config");
    str_unref(str);
}

TEST_CASE_FIXTURE(env_get_path_user_data_native, env_init, env_free)
{
    test_int_success(env_set(LIT("XDG_DATA_HOME"), LIT("/home/foo/my_data")));
    test_ptr_success(path = env_get_path(ENV_PATH_USER_DATA, ENV_MODE_NATIVE));
    test_ptr_success(str = path_get(path, PATH_STYLE_POSIX));
    test_str_eq(str_c(str), "/home/foo/my_data");
    str_unref(str);
}

TEST_CASE_FIXTURE(env_get_path_user_data_native_def, env_init, env_free)
{
    test_int_success(env_set(LIT("HOME"), LIT("/home/foo")));
    test_int_success(env_unset(LIT("XDG_DATA_HOME")));
    test_ptr_success(path = env_get_path(ENV_PATH_USER_DATA, ENV_MODE_NATIVE));
    test_ptr_success(str = path_get(path, PATH_STYLE_POSIX));
    test_str_eq(str_c(str), "/home/foo/.local/share");
    str_unref(str);
}

TEST_CASE_FIXTURE(env_get_path_user_home_native, env_init, env_free)
{
    test_int_success(env_set(LIT("HOME"), LIT("/home/foo")));
    test_ptr_success(path = env_get_path(ENV_PATH_USER_HOME, ENV_MODE_NATIVE));
    test_ptr_success(str = path_get(path, PATH_STYLE_POSIX));
    test_str_eq(str_c(str), "/home/foo");
    str_unref(str);
}

TEST_CASE_FIXTURE(env_get_path_user_home_native_missing, env_init, env_free)
{
    test_int_success(env_unset(LIT("HOME")));
    
#ifdef _WIN32
    test_ptr_error(env_get_path(ENV_PATH_USER_HOME, ENV_MODE_NATIVE), E_ENV_NOT_AVAILABLE);
#else
    struct passwd *pwd = getpwuid(getuid());
    
    test_ptr_success(path = env_get_path(ENV_PATH_USER_HOME, ENV_MODE_NATIVE));
    test_ptr_success(str = path_get(path, PATH_STYLE_SYSTEM));
    test_str_eq(str_c(str), pwd->pw_dir);
    str_unref(str);
#endif
}

TEST_CASE_FIXTURE(env_get_path_user_volatile_native, env_init, env_free)
{
    test_int_success(env_set(LIT("XDG_RUNTIME_DIR"), LIT("/home/foo/my_sockets")));
    test_ptr_success(path = env_get_path(ENV_PATH_USER_VOLATILE, ENV_MODE_NATIVE));
    test_ptr_success(str = path_get(path, PATH_STYLE_POSIX));
    test_str_eq(str_c(str), "/home/foo/my_sockets");
    str_unref(str);
}

TEST_CASE_FIXTURE(env_get_path_user_volatile_native_missing, env_init, env_free)
{
    test_int_success(env_unset(LIT("XDG_RUNTIME_DIR")));
    test_ptr_error(env_get_path(ENV_PATH_USER_VOLATILE, ENV_MODE_NATIVE), E_ENV_NOT_AVAILABLE);
}

test_suite_ct test_suite_env(void)
{
    return test_suite_new_with_cases("env"
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
        
        , test_case_new(env_get_path_invalid_ident)
        , test_case_new(env_get_path_invalid_mode)
        , test_case_new(env_get_path_user_cache_native)
        , test_case_new(env_get_path_user_cache_native_def)
        , test_case_new(env_get_path_user_config_native)
        , test_case_new(env_get_path_user_config_native_def)
        , test_case_new(env_get_path_user_data_native)
        , test_case_new(env_get_path_user_data_native_def)
        , test_case_new(env_get_path_user_home_native)
        , test_case_new(env_get_path_user_home_native_missing)
        , test_case_new(env_get_path_user_volatile_native)
        , test_case_new(env_get_path_user_volatile_native_missing)
    );
}
