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

#include "gen.h"
#include <ytil/test/test.h>
#include <ytil/gen/error.h>
#include <ytil/ext/errno.h>

#ifdef _WIN32
    #include <windows.h>
    #include <ntstatus.h>
#endif

typedef enum test_error
{
    E_TEST_ERROR_1,
    E_TEST_ERROR_2,
    E_TEST_ERROR_3,
} test_error_id;

ERROR_DEFINE_LIST(TEST_ERROR,
    ERROR_INFO(E_TEST_ERROR_1, "Test error 1."),
    ERROR_INFO(E_TEST_ERROR_2, "Test error 2."),
    ERROR_INFO(E_TEST_ERROR_3, "Test error 3.")
);

#define ERROR_TYPE_DEFAULT ERROR_TYPE_TEST_ERROR


TEST_CASE_ABORT(error_type_name_invalid_type)
{
    error_type_name(NULL);
}

TEST_CASE(error_type_name)
{
    test_str_eq(error_type_name(&ERROR_TYPE_TEST_ERROR), "TEST_ERROR");
}

TEST_CASE_ABORT(error_type_get_name_invalid_type)
{
    error_type_get_name(NULL, E_TEST_ERROR_1);
}

TEST_CASE(error_type_get_name)
{
    test_str_eq(
        error_type_get_name(&ERROR_TYPE_TEST_ERROR, E_TEST_ERROR_1),
        "E_TEST_ERROR_1");
}

TEST_CASE_ABORT(error_type_get_desc_invalid_type)
{
    error_type_get_desc(NULL, E_TEST_ERROR_1);
}

TEST_CASE(error_type_get_desc)
{
    test_str_eq(
        error_type_get_desc(&ERROR_TYPE_TEST_ERROR, E_TEST_ERROR_1),
        "Test error 1.");
}

TEST_CASE_ABORT(error_type_is_oom_invalid_type)
{
    error_type_is_oom(NULL, ENOMEM);
}

TEST_CASE(error_type_is_oom_unsupported)
{
    test_false(error_type_is_oom(&ERROR_TYPE_TEST_ERROR, E_TEST_ERROR_1));
}

TEST_CASE(error_type_is_oom)
{
    test_true(error_type_is_oom(&ERROR_TYPE_ERRNO, ENOMEM));
    test_false(error_type_is_oom(&ERROR_TYPE_ERRNO, EINVAL));
}

TEST_CASE_ABORT(error_type_get_last_invalid_type)
{
    error_type_get_last(NULL, NULL);
}

TEST_CASE_ABORT(error_type_get_last_unsupported)
{
    error_type_get_last(&ERROR_TYPE_TEST_ERROR, NULL);
}

TEST_CASE(error_type_get_last)
{
    errno = E2BIG;
    test_int_eq(error_type_get_last(&ERROR_TYPE_ERRNO, NULL), E2BIG);
}

TEST_CASE_ABORT(error_stack_get_func_oob)
{
    error_clear();
    error_stack_get_func(0);
}

TEST_CASE(error_stack_get_func)
{
    test_void(error_set_s(TEST_ERROR, E_TEST_ERROR_1));
    test_str_eq(error_stack_get_func(0), __func__);
}

TEST_CASE_ABORT(error_stack_get_type_oob)
{
    error_clear();
    error_stack_get_type(0);
}

TEST_CASE(error_stack_get_type)
{
    test_void(error_set_s(TEST_ERROR, E_TEST_ERROR_1));
    test_ptr_eq(error_stack_get_type(0), &ERROR_TYPE_TEST_ERROR);
}

TEST_CASE_ABORT(error_stack_get_code_oob)
{
    error_clear();
    error_stack_get_code(0);
}

TEST_CASE(error_stack_get_code)
{
    test_void(error_set_s(TEST_ERROR, E_TEST_ERROR_1));
    test_int_eq(error_stack_get_code(0), E_TEST_ERROR_1);
}

TEST_CASE_ABORT(error_stack_get_name_oob)
{
    error_clear();
    error_stack_get_name(0);
}

TEST_CASE(error_stack_get_name)
{
    test_void(error_set_s(TEST_ERROR, E_TEST_ERROR_1));
    test_str_eq(error_stack_get_name(0), "E_TEST_ERROR_1");
}

TEST_CASE_ABORT(error_stack_get_desc_oob)
{
    error_clear();
    error_stack_get_desc(0);
}

TEST_CASE(error_stack_get_desc)
{
    test_void(error_set_s(TEST_ERROR, E_TEST_ERROR_1));
    test_str_eq(error_stack_get_desc(0), "Test error 1.");
}

TEST_CASE_ABORT(error_stack_is_oom_oob)
{
    error_clear();
    error_stack_is_oom(0);
}

TEST_CASE(error_stack_is_oom)
{
    test_void(error_set_s(ERRNO, ENOMEM));
    test_true(error_stack_is_oom(0));
}

TEST_CASE_ABORT(error_func_oob)
{
    error_clear();
    error_func(0);
}

TEST_CASE(error_func)
{
    test_void(error_set_s(TEST_ERROR, E_TEST_ERROR_1));
    test_str_eq(error_func(0), __func__);
}

TEST_CASE_ABORT(error_type_oob)
{
    error_clear();
    error_type(0);
}

TEST_CASE(error_type)
{
    test_void(error_set_s(TEST_ERROR, E_TEST_ERROR_1));
    test_ptr_eq(error_type(0), &ERROR_TYPE_TEST_ERROR);
}

TEST_CASE_ABORT(error_code_oob)
{
    error_clear();
    error_code(0);
}

TEST_CASE(error_code)
{
    test_void(error_set_s(TEST_ERROR, E_TEST_ERROR_1));
    test_int_eq(error_code(0), E_TEST_ERROR_1);
}

TEST_CASE_ABORT(error_name_oob)
{
    error_clear();
    error_name(0);
}

TEST_CASE(error_name)
{
    test_void(error_set_s(TEST_ERROR, E_TEST_ERROR_1));
    test_str_eq(error_name(0), "E_TEST_ERROR_1");
}

TEST_CASE_ABORT(error_desc_oob)
{
    error_clear();
    error_desc(0);
}

TEST_CASE(error_desc)
{
    test_void(error_set_s(TEST_ERROR, E_TEST_ERROR_1));
    test_str_eq(error_desc(0), "Test error 1.");
}

TEST_CASE_ABORT(error_is_oom_oob)
{
    error_clear();
    error_is_oom(0);
}

TEST_CASE(error_is_oom)
{
    test_void(error_set_s(ERRNO, ENOMEM));
    test_true(error_is_oom(0));
}

TEST_CASE_ABORT(error_check_oob)
{
    error_clear();
    error_check(0, 1, E_TEST_ERROR_1);
}

TEST_CASE(error_check)
{
    test_void(error_set_s(TEST_ERROR, E_TEST_ERROR_2));
    test_false(error_check(0, 1, E_TEST_ERROR_1));
    test_true(error_check(0, 1, E_TEST_ERROR_2));
}

TEST_CASE(error_check_multiple)
{
    test_void(error_set_s(TEST_ERROR, E_TEST_ERROR_2));
    test_true(error_check(0, 3, E_TEST_ERROR_1, E_TEST_ERROR_2, E_TEST_ERROR_3));
}

TEST_CASE_ABORT(error_set_invalid_type)
{
    error_set_f(__func__, NULL, E_TEST_ERROR_1);
}

TEST_CASE(error_set)
{
    test_void(error_set_s(TEST_ERROR, E_TEST_ERROR_1));

    test_uint_eq(error_depth(), 1);
    test_stack_error(0, TEST_ERROR, E_TEST_ERROR_1);
    test_str_eq(error_stack_get_func(0), __func__);
    test_error(0, TEST_ERROR, E_TEST_ERROR_1);
}

TEST_CASE(error_set_default)
{
    test_void(error_set(E_TEST_ERROR_1));
    test_stack_error(0, TEST_ERROR, E_TEST_ERROR_1);
    test_str_eq(error_stack_get_func(0), __func__);
}

TEST_CASE_ABORT(error_set_last_invalid_type)
{
    error_set_last_f(__func__, NULL, NULL);
}

TEST_CASE_ABORT(error_set_last_unsupported)
{
    error_set_last_s(TEST_ERROR, NULL);
}

TEST_CASE(error_set_last)
{
    errno = EINVAL;
    test_void(error_set_last_s(ERRNO, NULL));
    test_stack_error(0, ERRNO, EINVAL);
    test_str_eq(error_stack_get_func(0), __func__);
    test_error(0, ERRNO, EINVAL);
}

TEST_CASE_ABORT(error_push_invalid_type)
{
    test_void(error_set_s(TEST_ERROR, E_TEST_ERROR_1));
    error_push_f(__func__, NULL, E_TEST_ERROR_2);
}

TEST_CASE(error_push)
{
    test_void(error_set_s(TEST_ERROR, E_TEST_ERROR_1));
    test_void(error_push_s(TEST_ERROR, E_TEST_ERROR_2));

    test_uint_eq(error_depth(), 2);

    test_stack_error(0, TEST_ERROR, E_TEST_ERROR_1);
    test_stack_error(1, TEST_ERROR, E_TEST_ERROR_2);
    test_str_eq(error_stack_get_func(0), __func__);
    test_str_eq(error_stack_get_func(1), __func__);
    test_error(0, TEST_ERROR, E_TEST_ERROR_2);
    test_error(1, TEST_ERROR, E_TEST_ERROR_1);
}

TEST_CASE(error_push_default)
{
    test_void(error_set_s(TEST_ERROR, E_TEST_ERROR_1));
    test_void(error_push(E_TEST_ERROR_2));

    test_uint_eq(error_depth(), 2);
    test_stack_error(0, TEST_ERROR, E_TEST_ERROR_1);
    test_stack_error(1, TEST_ERROR, E_TEST_ERROR_2);
    test_str_eq(error_stack_get_func(0), __func__);
    test_str_eq(error_stack_get_func(1), __func__);
}

TEST_CASE_ABORT(error_push_last_invalid_type)
{
    error_push_last_f(__func__, NULL, NULL);
}

TEST_CASE_ABORT(error_push_last_unsupported)
{
    error_push_last_s(TEST_ERROR, NULL);
}

TEST_CASE(error_push_last)
{
    error_clear();
    errno = ENOENT;
    test_void(error_push_last_s(ERRNO, NULL));
    test_stack_error(0, ERRNO, ENOENT);
    test_str_eq(error_stack_get_func(0), __func__);
    test_error(0, ERRNO, ENOENT);
}

TEST_CASE(error_reset)
{
    test_void(error_set(E_TEST_ERROR_1));
    test_void(error_push(E_TEST_ERROR_2));
    test_void(error_set(E_TEST_ERROR_3));

    test_uint_eq(error_depth(), 1);
    test_stack_error(0, TEST_ERROR, E_TEST_ERROR_3);
    test_error(0, TEST_ERROR, E_TEST_ERROR_3);
}

TEST_CASE_ABORT(error_wrap_missing)
{
    error_clear();
    error_wrap();
}

TEST_CASE(error_wrap)
{
    test_void(error_set_s(TEST_ERROR, E_TEST_ERROR_1));
    test_void(error_wrap());

    test_uint_eq(error_depth(), 2);
    test_stack_error(0, TEST_ERROR, E_TEST_ERROR_1);
    test_stack_error(1, GENERIC, E_GENERIC_WRAP);
    test_str_eq(error_stack_get_func(0), __func__);
    test_str_eq(error_stack_get_func(1), __func__);
    test_error(0, GENERIC, E_GENERIC_WRAP);
    test_error(1, TEST_ERROR, E_TEST_ERROR_1);
}

TEST_CASE(error_wrap_system)
{
    test_void(error_set_s(GENERIC, E_GENERIC_SYSTEM));
    test_void(error_wrap());

    test_uint_eq(error_depth(), 2);
    test_stack_error(0, GENERIC, E_GENERIC_SYSTEM);
    test_stack_error(1, GENERIC, E_GENERIC_PASS);
    test_error(0, GENERIC, E_GENERIC_SYSTEM);
}

TEST_CASE(error_wrap_oom)
{
    test_void(error_set_s(ERRNO, ENOMEM));
    test_void(error_wrap());

    test_uint_eq(error_depth(), 2);
    test_stack_error(0, ERRNO, ENOMEM);
    test_stack_error(1, GENERIC, E_GENERIC_OOM);
    test_error(0, GENERIC, E_GENERIC_OOM);
    test_error(1, ERRNO, ENOMEM);
}

TEST_CASE_ABORT(error_pack_invalid_type)
{
    test_void(error_set_s(TEST_ERROR, E_TEST_ERROR_1));
    error_pack_f(__func__, NULL, E_TEST_ERROR_2);
}

TEST_CASE_ABORT(error_pack_missing)
{
    error_clear();
    error_pack_s(TEST_ERROR, E_TEST_ERROR_2);
}

TEST_CASE(error_pack)
{
    test_void(error_set_s(TEST_ERROR, E_TEST_ERROR_1));
    test_void(error_pack_s(TEST_ERROR, E_TEST_ERROR_2));

    test_uint_eq(error_depth(), 2);
    test_stack_error(0, TEST_ERROR, E_TEST_ERROR_1);
    test_stack_error(1, TEST_ERROR, E_TEST_ERROR_2);
    test_str_eq(error_stack_get_func(0), __func__);
    test_str_eq(error_stack_get_func(1), __func__);
    test_error(0, TEST_ERROR, E_TEST_ERROR_2);
    test_error(1, TEST_ERROR, E_TEST_ERROR_1);
}

TEST_CASE(error_pack_default)
{
    test_void(error_set_s(TEST_ERROR, E_TEST_ERROR_1));
    test_void(error_pack(E_TEST_ERROR_2));

    test_uint_eq(error_depth(), 2);
    test_stack_error(0, TEST_ERROR, E_TEST_ERROR_1);
    test_stack_error(1, TEST_ERROR, E_TEST_ERROR_2);
    test_str_eq(error_stack_get_func(0), __func__);
    test_str_eq(error_stack_get_func(1), __func__);
}

TEST_CASE(error_pack_wrap)
{
    test_void(error_set_s(TEST_ERROR, E_TEST_ERROR_1));
    test_void(error_wrap());
    test_void(error_pack_s(TEST_ERROR, E_TEST_ERROR_2));

    test_uint_eq(error_depth(), 3);
    test_stack_error(0, TEST_ERROR, E_TEST_ERROR_1);
    test_stack_error(1, GENERIC, E_GENERIC_WRAP);
    test_stack_error(2, TEST_ERROR, E_TEST_ERROR_2);
    test_error(0, TEST_ERROR, E_TEST_ERROR_2);
    test_error(1, GENERIC, E_GENERIC_WRAP);
    test_error(2, TEST_ERROR, E_TEST_ERROR_1);
}

TEST_CASE(error_pack_system)
{
    test_void(error_set_s(GENERIC, E_GENERIC_SYSTEM));
    test_void(error_pack_s(TEST_ERROR, E_TEST_ERROR_1));

    test_uint_eq(error_depth(), 2);
    test_stack_error(0, GENERIC, E_GENERIC_SYSTEM);
    test_stack_error(1, GENERIC, E_GENERIC_PASS);
    test_error(0, GENERIC, E_GENERIC_SYSTEM);
}

TEST_CASE(error_pack_oom)
{
    test_void(error_set_s(ERRNO, ENOMEM));
    test_void(error_pack_s(TEST_ERROR, E_TEST_ERROR_1));

    test_uint_eq(error_depth(), 2);
    test_stack_error(0, ERRNO, ENOMEM);
    test_stack_error(1, GENERIC, E_GENERIC_OOM);
    test_error(0, GENERIC, E_GENERIC_OOM);
    test_error(1, ERRNO, ENOMEM);
}

TEST_CASE_ABORT(error_pack_last_invalid_type)
{
    test_void(error_set_s(TEST_ERROR, E_TEST_ERROR_1));
    error_pack_last_f(__func__, NULL, NULL);
}

TEST_CASE_ABORT(error_pack_last_unsupported)
{
    test_void(error_set_s(TEST_ERROR, E_TEST_ERROR_1));
    error_pack_last_s(TEST_ERROR, NULL);
}

TEST_CASE(error_pack_last)
{
    test_void(error_set_s(TEST_ERROR, E_TEST_ERROR_1));
    errno = EBADF;
    test_void(error_pack_last_s(ERRNO, NULL));

    test_uint_eq(error_depth(), 2);
    test_stack_error(0, TEST_ERROR, E_TEST_ERROR_1);
    test_stack_error(1, ERRNO, EBADF);
    test_str_eq(error_stack_get_func(0), __func__);
    test_str_eq(error_stack_get_func(1), __func__);
    test_error(0, ERRNO, EBADF);
    test_error(1, TEST_ERROR, E_TEST_ERROR_1);
}

TEST_CASE_ABORT(error_pass_missing)
{
    error_clear();
    error_pass();
}

TEST_CASE(error_pass)
{
    test_void(error_set_s(TEST_ERROR, E_TEST_ERROR_1));
    test_void(error_pass());

    test_uint_eq(error_depth(), 2);
    test_stack_error(0, TEST_ERROR, E_TEST_ERROR_1);
    test_stack_error(1, GENERIC, E_GENERIC_PASS);
    test_str_eq(error_stack_get_func(0), __func__);
    test_str_eq(error_stack_get_func(1), __func__);
    test_error(0, TEST_ERROR, E_TEST_ERROR_1);
}

TEST_CASE(error_pass_double)
{
    test_void(error_set_s(TEST_ERROR, E_TEST_ERROR_1));
    test_void(error_pass());
    test_void(error_push_s(TEST_ERROR, E_TEST_ERROR_2));
    test_void(error_pass());

    test_uint_eq(error_depth(), 4);
    test_stack_error(0, TEST_ERROR, E_TEST_ERROR_1);
    test_stack_error(1, GENERIC, E_GENERIC_PASS);
    test_stack_error(2, TEST_ERROR, E_TEST_ERROR_2);
    test_stack_error(3, GENERIC, E_GENERIC_PASS);
    test_error(0, TEST_ERROR, E_TEST_ERROR_2);
    test_error(1, TEST_ERROR, E_TEST_ERROR_1);
}

TEST_CASE_ABORT(error_skip_missing)
{
    error_clear();
    error_skip();
}

TEST_CASE(error_skip)
{
    test_void(error_set_s(TEST_ERROR, E_TEST_ERROR_1));
    test_void(error_push_s(TEST_ERROR, E_TEST_ERROR_2));
    test_void(error_skip());

    test_uint_eq(error_depth(), 3);
    test_stack_error(0, TEST_ERROR, E_TEST_ERROR_1);
    test_stack_error(1, TEST_ERROR, E_TEST_ERROR_2);
    test_stack_error(2, GENERIC, E_GENERIC_SKIP);
    test_str_eq(error_stack_get_func(0), __func__);
    test_str_eq(error_stack_get_func(1), __func__);
    test_str_eq(error_stack_get_func(2), __func__);
    test_error(0, TEST_ERROR, E_TEST_ERROR_1);
}

TEST_CASE_ABORT(error_pick_missing)
{
    error_clear();
    error_pick(E_TEST_ERROR_1);
}

TEST_CASE(error_pick)
{
    test_void(error_set_s(TEST_ERROR, E_TEST_ERROR_1));
    test_void(error_push_s(TEST_ERROR, E_TEST_ERROR_2));
    test_void(error_pick(E_TEST_ERROR_2));

    test_uint_eq(error_depth(), 3);
    test_stack_error(0, TEST_ERROR, E_TEST_ERROR_1);
    test_stack_error(1, TEST_ERROR, E_TEST_ERROR_2);
    test_stack_error(2, GENERIC, E_GENERIC_SKIP);
    test_str_eq(error_stack_get_func(0), __func__);
    test_str_eq(error_stack_get_func(1), __func__);
    test_str_eq(error_stack_get_func(2), __func__);
    test_error(0, TEST_ERROR, E_TEST_ERROR_1);
}

TEST_CASE(error_pick_wrap)
{
    test_void(error_set_s(TEST_ERROR, E_TEST_ERROR_1));
    test_void(error_push_s(TEST_ERROR, E_TEST_ERROR_2));
    test_void(error_pick(E_TEST_ERROR_3));

    test_uint_eq(error_depth(), 3);
    test_stack_error(0, TEST_ERROR, E_TEST_ERROR_1);
    test_stack_error(1, TEST_ERROR, E_TEST_ERROR_2);
    test_stack_error(2, GENERIC, E_GENERIC_WRAP);
    test_error(0, GENERIC, E_GENERIC_WRAP);
}

TEST_CASE_ABORT(error_lift_missing)
{
    error_clear();
    error_lift(E_TEST_ERROR_1);
}

TEST_CASE(error_lift)
{
    test_void(error_set_s(TEST_ERROR, E_TEST_ERROR_1));
    test_void(error_push_s(TEST_ERROR, E_TEST_ERROR_2));
    test_void(error_lift(E_TEST_ERROR_2));

    test_uint_eq(error_depth(), 3);
    test_stack_error(0, TEST_ERROR, E_TEST_ERROR_1);
    test_stack_error(1, TEST_ERROR, E_TEST_ERROR_2);
    test_stack_error(2, GENERIC, E_GENERIC_SKIP);
    test_str_eq(error_stack_get_func(0), __func__);
    test_str_eq(error_stack_get_func(1), __func__);
    test_str_eq(error_stack_get_func(2), __func__);
    test_error(0, TEST_ERROR, E_TEST_ERROR_1);
}

TEST_CASE(error_lift_pass)
{
    test_void(error_set_s(TEST_ERROR, E_TEST_ERROR_1));
    test_void(error_push_s(TEST_ERROR, E_TEST_ERROR_2));
    test_void(error_lift(E_TEST_ERROR_3));

    test_uint_eq(error_depth(), 3);
    test_stack_error(0, TEST_ERROR, E_TEST_ERROR_1);
    test_stack_error(1, TEST_ERROR, E_TEST_ERROR_2);
    test_stack_error(2, GENERIC, E_GENERIC_PASS);
    test_error(0, TEST_ERROR, E_TEST_ERROR_2);
}

TEST_CASE_ABORT(error_pass_sub_invalid_type)
{
    error_pass_sub_f(__func__, "foo", NULL, ENOSYS);
}

TEST_CASE(error_pass_sub)
{
    test_void(error_pass_sub(foo, ERRNO, ENOSYS));

    test_uint_eq(error_depth(), 2);
    test_stack_error(0, ERRNO, ENOSYS);
    test_stack_error(1, GENERIC, E_GENERIC_PASS);
    test_str_eq(error_stack_get_func(0), "foo");
    test_str_eq(error_stack_get_func(1), __func__);
}

TEST_CASE_ABORT(error_pass_last_sub_invalid_type)
{
    error_pass_last_sub_f(__func__, "bar", NULL, NULL);
}

TEST_CASE(error_pass_last_sub)
{
    errno = EBUSY;
    test_void(error_pass_last_sub(bar, ERRNO, NULL));

    test_uint_eq(error_depth(), 2);
    test_stack_error(0, ERRNO, EBUSY);
    test_stack_error(1, GENERIC, E_GENERIC_PASS);
    test_str_eq(error_stack_get_func(0), "bar");
    test_str_eq(error_stack_get_func(1), __func__);
}

TEST_CASE_ABORT(error_wrap_sub_invalid_type)
{
    error_wrap_sub_f(__func__, "baz", NULL, ENODEV);
}

TEST_CASE(error_wrap_sub)
{
    test_void(error_wrap_sub(baz, ERRNO, ENODEV));

    test_uint_eq(error_depth(), 2);
    test_stack_error(0, ERRNO, ENODEV);
    test_stack_error(1, GENERIC, E_GENERIC_WRAP);
    test_str_eq(error_stack_get_func(0), "baz");
    test_str_eq(error_stack_get_func(1), __func__);
}

TEST_CASE_ABORT(error_wrap_last_sub_invalid_type)
{
    error_wrap_last_sub_f(__func__, "boz", NULL, NULL);
}

TEST_CASE(error_wrap_last_sub)
{
    errno = ENOENT;
    test_void(error_wrap_last_sub(boz, ERRNO, NULL));

    test_uint_eq(error_depth(), 2);
    test_stack_error(0, ERRNO, ENOENT);
    test_stack_error(1, GENERIC, E_GENERIC_WRAP);
    test_str_eq(error_stack_get_func(0), "boz");
    test_str_eq(error_stack_get_func(1), __func__);
}

TEST_CASE_ABORT(error_pack_sub_invalid_type1)
{
    error_pack_sub_f(__func__, NULL, E_TEST_ERROR_1, "buzz", &ERROR_TYPE_ERRNO, EPERM);
}

TEST_CASE_ABORT(error_pack_sub_invalid_type2)
{
    error_pack_sub_f(__func__, &ERROR_TYPE_TEST_ERROR, E_TEST_ERROR_1, "buzz", NULL, EPERM);
}

TEST_CASE(error_pack_sub)
{
    test_void(error_pack_sub(E_TEST_ERROR_1, buzz, ERRNO, EPERM));

    test_uint_eq(error_depth(), 2);
    test_stack_error(0, ERRNO, EPERM);
    test_stack_error(1, TEST_ERROR, E_TEST_ERROR_1);
    test_str_eq(error_stack_get_func(0), "buzz");
    test_str_eq(error_stack_get_func(1), __func__);
}

TEST_CASE(error_pack_sub_oom)
{
    test_void(error_pack_sub(E_TEST_ERROR_1, bozz, ERRNO, ENOMEM));

    test_uint_eq(error_depth(), 2);
    test_stack_error(0, ERRNO, ENOMEM);
    test_stack_error(1, GENERIC, E_GENERIC_OOM);
    test_str_eq(error_stack_get_func(0), "bozz");
    test_str_eq(error_stack_get_func(1), __func__);
}

TEST_CASE_ABORT(error_pack_last_sub_invalid_type1)
{
    error_pack_last_sub_f(__func__, NULL, E_TEST_ERROR_2, "bazz", &ERROR_TYPE_ERRNO, NULL);
}

TEST_CASE_ABORT(error_pack_last_sub_invalid_type2)
{
    error_pack_last_sub_f(__func__, &ERROR_TYPE_TEST_ERROR, E_TEST_ERROR_2, "bazz", NULL, NULL);
}

TEST_CASE(error_pack_last_sub)
{
    errno = EACCES;
    test_void(error_pack_last_sub(E_TEST_ERROR_2, bazz, ERRNO, NULL));

    test_uint_eq(error_depth(), 2);
    test_stack_error(0, ERRNO, EACCES);
    test_stack_error(1, TEST_ERROR, E_TEST_ERROR_2);
    test_str_eq(error_stack_get_func(0), "bazz");
    test_str_eq(error_stack_get_func(1), __func__);
}

TEST_CASE(error_pack_last_sub_oom)
{
    errno = ENOMEM;
    test_void(error_pack_last_sub(E_TEST_ERROR_1, bizz, ERRNO, NULL));

    test_uint_eq(error_depth(), 2);
    test_stack_error(0, ERRNO, ENOMEM);
    test_stack_error(1, GENERIC, E_GENERIC_OOM);
    test_str_eq(error_stack_get_func(0), "bizz");
    test_str_eq(error_stack_get_func(1), __func__);
}

TEST_CASE(error_info_generic)
{
    test_void(error_set_s(GENERIC, E_GENERIC_SYSTEM));

    test_uint_eq(error_depth(), 1);
    test_ptr_eq(error_stack_get_type(0), &ERROR_TYPE_GENERIC);
    test_int_eq(error_stack_get_code(0), E_GENERIC_SYSTEM);
    test_str_eq(error_stack_get_name(0), "E_GENERIC_SYSTEM");
    test_str_eq(error_stack_get_desc(0), "System error.");
}

TEST_CASE(error_info_errno)
{
    test_void(error_set_s(ERRNO, ENOMEM));

    test_uint_eq(error_depth(), 1);
    test_ptr_eq(error_stack_get_type(0), &ERROR_TYPE_ERRNO);
    test_int_eq(error_stack_get_code(0), ENOMEM);
    test_str_eq(error_stack_get_name(0), strerrno(ENOMEM));
    test_str_eq(error_stack_get_desc(0), strerror(ENOMEM));
    test_true(error_stack_is_oom(0));
}

TEST_CASE(error_pass_errno)
{
    errno = EFAULT;
    test_void(error_pass_errno(foo));

    test_uint_eq(error_depth(), 2);
    test_stack_error(0, ERRNO, EFAULT);
    test_stack_error(1, GENERIC, E_GENERIC_PASS);
}

TEST_CASE(error_wrap_errno)
{
    errno = EFAULT;
    test_void(error_wrap_errno(foo));

    test_uint_eq(error_depth(), 2);
    test_stack_error(0, ERRNO, EFAULT);
    test_stack_error(1, GENERIC, E_GENERIC_WRAP);
}

TEST_CASE(error_wrap_errno_ENOMEM)
{
    errno = ENOMEM;
    test_void(error_wrap_errno(foo));

    test_uint_eq(error_depth(), 2);
    test_stack_error(0, ERRNO, ENOMEM);
    test_stack_error(1, GENERIC, E_GENERIC_OOM);
}

TEST_CASE(error_pack_errno)
{
    errno = EFAULT;
    test_void(error_pack_errno(E_TEST_ERROR_1, foo));

    test_uint_eq(error_depth(), 2);
    test_stack_error(0, ERRNO, EFAULT);
    test_stack_error(1, TEST_ERROR, E_TEST_ERROR_1);
}

TEST_CASE(error_pack_errno_ENOMEM)
{
    errno = ENOMEM;
    test_void(error_pack_errno(E_TEST_ERROR_1, foo));

    test_uint_eq(error_depth(), 2);
    test_stack_error(0, ERRNO, ENOMEM);
    test_stack_error(1, GENERIC, E_GENERIC_OOM);
}

#ifdef _WIN32

static char *_test_error_format_win32(DWORD error)
{
    char *msg;
    DWORD rc;

    rc = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_ALLOCATE_BUFFER,
        NULL, error, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (char *)&msg, 0, NULL);

    for(; rc && (msg[rc - 1] == '\n' || msg[rc - 1] == '\r'); rc--)
        msg[rc - 1] = '\0';

    return msg;
}

TEST_CASE(error_info_win32)
{
    char *msg;

    test_void(error_set_s(WIN32, ERROR_NOT_ENOUGH_MEMORY));
    test_ptr_success(msg = _test_error_format_win32(ERROR_NOT_ENOUGH_MEMORY));

    test_uint_eq(error_depth(), 1);
    test_ptr_eq(error_stack_get_type(0), &ERROR_TYPE_WIN32);
    test_int_eq(error_stack_get_code(0), ERROR_NOT_ENOUGH_MEMORY);
    test_str_eq(error_stack_get_name(0), "WIN32_00000008");
    test_str_eq(error_stack_get_desc(0), msg);
    test_true(error_stack_is_oom(0));

    LocalFree(msg);
}

TEST_CASE(error_pass_win32)
{
    SetLastError(ERROR_FILE_NOT_FOUND);
    test_void(error_pass_win32(foo));

    test_uint_eq(error_depth(), 2);
    test_stack_error(0, WIN32, ERROR_FILE_NOT_FOUND);
    test_stack_error(1, GENERIC, E_GENERIC_PASS);
}

TEST_CASE(error_wrap_win32)
{
    SetLastError(ERROR_FILE_NOT_FOUND);
    test_void(error_wrap_win32(foo));

    test_uint_eq(error_depth(), 2);
    test_stack_error(0, WIN32, ERROR_FILE_NOT_FOUND);
    test_stack_error(1, GENERIC, E_GENERIC_WRAP);
}

TEST_CASE(error_wrap_win32_ERROR_NOT_ENOUGH_MEMORY)
{
    SetLastError(ERROR_NOT_ENOUGH_MEMORY);
    test_void(error_wrap_win32(foo));

    test_uint_eq(error_depth(), 2);
    test_stack_error(0, WIN32, ERROR_NOT_ENOUGH_MEMORY);
    test_stack_error(1, GENERIC, E_GENERIC_OOM);
}

TEST_CASE(error_wrap_win32_ERROR_OUTOFMEMORY)
{
    SetLastError(ERROR_OUTOFMEMORY);
    test_void(error_wrap_win32(foo));

    test_uint_eq(error_depth(), 2);
    test_stack_error(0, WIN32, ERROR_OUTOFMEMORY);
    test_stack_error(1, GENERIC, E_GENERIC_OOM);
}

TEST_CASE(error_pack_win32)
{
    SetLastError(ERROR_FILE_NOT_FOUND);
    test_void(error_pack_win32(E_TEST_ERROR_1, foo));

    test_uint_eq(error_depth(), 2);
    test_stack_error(0, WIN32, ERROR_FILE_NOT_FOUND);
    test_stack_error(1, TEST_ERROR, E_TEST_ERROR_1);
}

TEST_CASE(error_pack_win32_ERROR_NOT_ENOUGH_MEMORY)
{
    SetLastError(ERROR_NOT_ENOUGH_MEMORY);
    test_void(error_pack_win32(E_TEST_ERROR_1, foo));

    test_uint_eq(error_depth(), 2);
    test_stack_error(0, WIN32, ERROR_NOT_ENOUGH_MEMORY);
    test_stack_error(1, GENERIC, E_GENERIC_OOM);
}

TEST_CASE(error_pack_win32_ERROR_OUTOFMEMORY)
{
    SetLastError(ERROR_OUTOFMEMORY);
    test_void(error_pack_win32(E_TEST_ERROR_1, foo));

    test_uint_eq(error_depth(), 2);
    test_stack_error(0, WIN32, ERROR_OUTOFMEMORY);
    test_stack_error(1, GENERIC, E_GENERIC_OOM);
}

TEST_CASE(error_info_hresult)
{
    test_void(error_set_s(HRESULT, HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND)));

    test_uint_eq(error_depth(), 1);
    test_ptr_eq(error_stack_get_type(0), &ERROR_TYPE_HRESULT);
    test_int_eq(error_stack_get_code(0), HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND));
    test_str_eq(error_stack_get_name(0), "HRESULT_80070002");
    // test_str_eq(error_stack_get_desc(0), );
}

TEST_CASE(error_pass_hresult)
{
    test_void(error_pass_hresult(foo, HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND)));

    test_uint_eq(error_depth(), 2);
    test_stack_error(0, HRESULT, HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND));
    test_stack_error(1, GENERIC, E_GENERIC_PASS);
}

TEST_CASE(error_wrap_hresult)
{
    test_void(error_wrap_hresult(foo, HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND)));

    test_uint_eq(error_depth(), 2);
    test_stack_error(0, HRESULT, HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND));
    test_stack_error(1, GENERIC, E_GENERIC_WRAP);
}

TEST_CASE(error_pack_hresult)
{
    test_void(error_pack_hresult(E_TEST_ERROR_1, foo, HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND)));

    test_uint_eq(error_depth(), 2);
    test_stack_error(0, HRESULT, HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND));
    test_stack_error(1, TEST_ERROR, E_TEST_ERROR_1);
}

TEST_CASE(error_info_ntstatus)
{
    test_void(error_set_s(NTSTATUS, STATUS_TIMEOUT));

    test_uint_eq(error_depth(), 1);
    test_ptr_eq(error_stack_get_type(0), &ERROR_TYPE_NTSTATUS);
    test_int_eq(error_stack_get_code(0), STATUS_TIMEOUT);
    test_str_eq(error_stack_get_name(0), "NTSTATUS_00000102");
    // test_str_eq(error_stack_get_desc(0), );
}

TEST_CASE(error_pass_ntstatus)
{
    test_void(error_pass_ntstatus(foo, STATUS_TIMEOUT));

    test_uint_eq(error_depth(), 2);
    test_stack_error(0, NTSTATUS, STATUS_TIMEOUT);
    test_stack_error(1, GENERIC, E_GENERIC_PASS);
}

TEST_CASE(error_wrap_ntstatus)
{
    test_void(error_wrap_ntstatus(foo, STATUS_TIMEOUT));

    test_uint_eq(error_depth(), 2);
    test_stack_error(0, NTSTATUS, STATUS_TIMEOUT);
    test_stack_error(1, GENERIC, E_GENERIC_WRAP);
}

TEST_CASE(error_pack_ntstatus)
{
    test_void(error_pack_ntstatus(E_TEST_ERROR_1, foo, STATUS_TIMEOUT));

    test_uint_eq(error_depth(), 2);
    test_stack_error(0, NTSTATUS, STATUS_TIMEOUT);
    test_stack_error(1, TEST_ERROR, E_TEST_ERROR_1);
}

#endif // _WIN32

test_suite_ct test_suite_gen_error(void)
{
    return test_suite_new_with_cases("error",
        test_case_new(error_type_name_invalid_type),
        test_case_new(error_type_name),
        test_case_new(error_type_get_name_invalid_type),
        test_case_new(error_type_get_name),
        test_case_new(error_type_get_desc_invalid_type),
        test_case_new(error_type_get_desc),
        test_case_new(error_type_is_oom_invalid_type),
        test_case_new(error_type_is_oom_unsupported),
        test_case_new(error_type_is_oom),
        test_case_new(error_type_get_last_invalid_type),
        test_case_new(error_type_get_last_unsupported),
        test_case_new(error_type_get_last),

        test_case_new(error_stack_get_func_oob),
        test_case_new(error_stack_get_func),
        test_case_new(error_stack_get_type_oob),
        test_case_new(error_stack_get_type),
        test_case_new(error_stack_get_code_oob),
        test_case_new(error_stack_get_code),
        test_case_new(error_stack_get_name_oob),
        test_case_new(error_stack_get_name),
        test_case_new(error_stack_get_desc_oob),
        test_case_new(error_stack_get_desc),
        test_case_new(error_stack_is_oom_oob),
        test_case_new(error_stack_is_oom),

        test_case_new(error_func_oob),
        test_case_new(error_func),
        test_case_new(error_type_oob),
        test_case_new(error_type),
        test_case_new(error_code_oob),
        test_case_new(error_code),
        test_case_new(error_name_oob),
        test_case_new(error_name),
        test_case_new(error_desc_oob),
        test_case_new(error_desc),
        test_case_new(error_is_oom_oob),
        test_case_new(error_is_oom),

        test_case_new(error_check_oob),
        test_case_new(error_check),
        test_case_new(error_check_multiple),

        test_case_new(error_set_invalid_type),
        test_case_new(error_set),
        test_case_new(error_set_default),
        test_case_new(error_set_last_invalid_type),
        test_case_new(error_set_last_unsupported),
        test_case_new(error_set_last),

        test_case_new(error_push_invalid_type),
        test_case_new(error_push),
        test_case_new(error_push_default),
        test_case_new(error_push_last_invalid_type),
        test_case_new(error_push_last_unsupported),
        test_case_new(error_push_last),

        test_case_new(error_reset),

        test_case_new(error_wrap_missing),
        test_case_new(error_wrap),
        test_case_new(error_wrap_system),
        test_case_new(error_wrap_oom),

        test_case_new(error_pack_invalid_type),
        test_case_new(error_pack_missing),
        test_case_new(error_pack),
        test_case_new(error_pack_default),
        test_case_new(error_pack_wrap),
        test_case_new(error_pack_system),
        test_case_new(error_pack_oom),
        test_case_new(error_pack_last_invalid_type),
        test_case_new(error_pack_last_unsupported),
        test_case_new(error_pack_last),

        test_case_new(error_pass_missing),
        test_case_new(error_pass),
        test_case_new(error_pass_double),

        test_case_new(error_skip_missing),
        test_case_new(error_skip),

        test_case_new(error_pick_missing),
        test_case_new(error_pick),
        test_case_new(error_pick_wrap),

        test_case_new(error_lift_missing),
        test_case_new(error_lift),
        test_case_new(error_lift_pass),

        test_case_new(error_pass_sub_invalid_type),
        test_case_new(error_pass_sub),
        test_case_new(error_pass_last_sub_invalid_type),
        test_case_new(error_pass_last_sub),

        test_case_new(error_wrap_sub_invalid_type),
        test_case_new(error_wrap_sub),
        test_case_new(error_wrap_last_sub_invalid_type),
        test_case_new(error_wrap_last_sub),

        test_case_new(error_pack_sub_invalid_type1),
        test_case_new(error_pack_sub_invalid_type2),
        test_case_new(error_pack_sub),
        test_case_new(error_pack_sub_oom),
        test_case_new(error_pack_last_sub_invalid_type1),
        test_case_new(error_pack_last_sub_invalid_type2),
        test_case_new(error_pack_last_sub),
        test_case_new(error_pack_last_sub_oom),

        test_case_new(error_info_generic),

        test_case_new(error_info_errno),
        test_case_new(error_pass_errno),
        test_case_new(error_wrap_errno),
        test_case_new(error_wrap_errno_ENOMEM),
        test_case_new(error_pack_errno),
        test_case_new(error_pack_errno_ENOMEM),

        test_case_new_windows(error_info_win32),
        test_case_new_windows(error_pass_win32),
        test_case_new_windows(error_wrap_win32),
        test_case_new_windows(error_wrap_win32_ERROR_NOT_ENOUGH_MEMORY),
        test_case_new_windows(error_wrap_win32_ERROR_OUTOFMEMORY),
        test_case_new_windows(error_pack_win32),
        test_case_new_windows(error_pack_win32_ERROR_NOT_ENOUGH_MEMORY),
        test_case_new_windows(error_pack_win32_ERROR_OUTOFMEMORY),

        test_case_new_windows(error_info_hresult),
        test_case_new_windows(error_pass_hresult),
        test_case_new_windows(error_wrap_hresult),
        test_case_new_windows(error_pack_hresult),

        test_case_new_windows(error_info_ntstatus),
        test_case_new_windows(error_pass_ntstatus),
        test_case_new_windows(error_wrap_ntstatus),
        test_case_new_windows(error_pack_ntstatus)
    );
}
