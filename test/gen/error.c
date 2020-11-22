/*
 * Copyright (c) 2019-2020 Martin Rödel a.k.a. Yomin Nimoy
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
    E_TEST_ERROR_OV1,
    E_TEST_ERROR_OV2,
} test_error_id;

ERROR_DEFINE_LIST(TEST_ERROR,
    ERROR_INFO(E_TEST_ERROR_1,   "Test error 1."),
    ERROR_INFO(E_TEST_ERROR_2,   "Test error 2."),
    ERROR_INFO(E_TEST_ERROR_3,   "Test error 3."),
    ERROR_INFO(E_TEST_ERROR_OV1, "Test error Override 1."),
    ERROR_INFO(E_TEST_ERROR_OV2, "Test error Override 2.")
);

static int test_error_last(const error_type_st *type, const char **desc, const char *ctx_type, void *ctx)
{
    if(desc)
        *desc = ctx;

    if(ctx_type && !strcmp(ctx_type, "override"))
        return E_TEST_ERROR_OV2;
    else
        return E_TEST_ERROR_OV1;
}

ERROR_DEFINE_CALLBACK(TEST_ERROR_OVERRIDE, NULL, NULL, NULL, test_error_last);

#define ERROR_TYPE_DEFAULT ERROR_TYPE_TEST_ERROR


TEST_CASE_ABORT(error_type_name_invalid_type)
{
    error_type_name(NULL);
}

TEST_CASE(error_type_name)
{
    test_str_eq(error_type_name(ERROR_TYPE(TEST_ERROR)), "TEST_ERROR");
}

TEST_CASE_ABORT(error_type_get_name_invalid_type)
{
    error_type_get_name(NULL, E_TEST_ERROR_1);
}

TEST_CASE(error_type_get_name)
{
    test_str_eq(
        error_type_get_name(ERROR_TYPE(TEST_ERROR), E_TEST_ERROR_1),
        "E_TEST_ERROR_1");
}

TEST_CASE_ABORT(error_type_get_desc_invalid_type)
{
    error_type_get_desc(NULL, E_TEST_ERROR_1);
}

TEST_CASE(error_type_get_desc)
{
    test_str_eq(
        error_type_get_desc(ERROR_TYPE(TEST_ERROR), E_TEST_ERROR_1),
        "Test error 1.");
}

TEST_CASE_ABORT(error_type_is_oom_invalid_type)
{
    error_type_is_oom(NULL, ENOMEM);
}

TEST_CASE(error_type_is_oom_unsupported)
{
    test_false(error_type_is_oom(ERROR_TYPE(TEST_ERROR), E_TEST_ERROR_1));
}

TEST_CASE(error_type_is_oom)
{
    test_true(error_type_is_oom(ERROR_TYPE(ERRNO), ENOMEM));
    test_false(error_type_is_oom(ERROR_TYPE(ERRNO), EINVAL));
}

TEST_CASE_ABORT(error_type_get_last_invalid_type)
{
    error_type_get_last(NULL, NULL);
}

TEST_CASE_ABORT(error_type_get_last_unsupported)
{
    error_type_get_last(ERROR_TYPE(TEST_ERROR), NULL);
}

TEST_CASE(error_type_get_last)
{
    errno = E2BIG;
    test_int_eq(error_type_get_last(ERROR_TYPE(ERRNO), NULL), E2BIG);
}

TEST_CASE_ABORT(error_type_get_last_x_invalid_type)
{
    error_type_get_last_x(NULL, NULL, NULL, NULL);
}

TEST_CASE_ABORT(error_type_get_last_x_unsupported)
{
    error_type_get_last_x(ERROR_TYPE(TEST_ERROR), NULL, NULL, NULL);
}

TEST_CASE(error_type_get_last_x)
{
    errno = E2BIG;
    test_int_eq(error_type_get_last_x(ERROR_TYPE(ERRNO), NULL, NULL, NULL), E2BIG);
}

TEST_CASE(error_type_get_last_x_override_desc)
{
    const char *desc = NULL;

    test_int_eq(error_type_get_last_x(ERROR_TYPE(TEST_ERROR_OVERRIDE), &desc, NULL, "override"), E_TEST_ERROR_OV1);
    test_str_eq(desc, "override");
}

TEST_CASE(error_type_get_last_x_override_ctx)
{
    test_int_eq(error_type_get_last_x(ERROR_TYPE(TEST_ERROR_OVERRIDE), NULL, "override", NULL), E_TEST_ERROR_OV2);
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
    test_ptr_eq(error_stack_get_type(0), ERROR_TYPE(TEST_ERROR));
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
    test_ptr_eq(error_type(0), ERROR_TYPE(TEST_ERROR));
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
    error_set_f(__func__, NULL, E_TEST_ERROR_1, NULL);
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

TEST_CASE(error_set_override_desc)
{
    test_void(error_set_sd(TEST_ERROR, E_TEST_ERROR_1, "override"));
    test_stack_error(0, TEST_ERROR, E_TEST_ERROR_1);
    test_str_eq(error_stack_get_desc(0), "override");
}

TEST_CASE(error_set_default_override_desc)
{
    test_void(error_set_d(E_TEST_ERROR_1, "override"));
    test_stack_error(0, TEST_ERROR, E_TEST_ERROR_1);
    test_str_eq(error_stack_get_desc(0), "override");
}

TEST_CASE_ABORT(error_set_last_invalid_type)
{
    error_set_last_f(__func__, NULL, NULL, NULL);
}

TEST_CASE_ABORT(error_set_last_unsupported)
{
    error_set_last_s(TEST_ERROR);
}

TEST_CASE(error_set_last)
{
    errno = EINVAL;
    test_void(error_set_last_s(ERRNO));
    test_stack_error(0, ERRNO, EINVAL);
    test_str_eq(error_stack_get_func(0), __func__);
    test_error(0, ERRNO, EINVAL);
}

TEST_CASE(error_set_last_override_desc)
{
    test_void(error_set_last_sx(TEST_ERROR_OVERRIDE, NULL, "override"));
    test_stack_error(0, TEST_ERROR_OVERRIDE, E_TEST_ERROR_OV1);
    test_str_eq(error_stack_get_desc(0), "override");
}

TEST_CASE(error_set_last_override_ctx)
{
    test_void(error_set_last_sx(TEST_ERROR_OVERRIDE, "override", NULL));
    test_stack_error(0, TEST_ERROR_OVERRIDE, E_TEST_ERROR_OV2);
}

TEST_CASE_ABORT(error_push_invalid_type)
{
    test_void(error_set_s(TEST_ERROR, E_TEST_ERROR_1));
    error_push_f(__func__, NULL, E_TEST_ERROR_2, NULL);
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

TEST_CASE(error_push_override_desc)
{
    test_void(error_set_s(TEST_ERROR, E_TEST_ERROR_1));
    test_void(error_push_sd(TEST_ERROR, E_TEST_ERROR_2, "override"));

    test_stack_error(1, TEST_ERROR, E_TEST_ERROR_2);
    test_str_eq(error_stack_get_desc(1), "override");
}

TEST_CASE(error_push_default_override_desc)
{
    test_void(error_set_s(TEST_ERROR, E_TEST_ERROR_1));
    test_void(error_push_d(E_TEST_ERROR_2, "override"));

    test_stack_error(1, TEST_ERROR, E_TEST_ERROR_2);
    test_str_eq(error_stack_get_desc(1), "override");
}

TEST_CASE_ABORT(error_push_last_invalid_type)
{
    error_push_last_f(__func__, NULL, NULL, NULL);
}

TEST_CASE_ABORT(error_push_last_unsupported)
{
    error_push_last_s(TEST_ERROR);
}

TEST_CASE(error_push_last)
{
    error_clear();
    errno = ENOENT;
    test_void(error_push_last_s(ERRNO));
    test_stack_error(0, ERRNO, ENOENT);
    test_str_eq(error_stack_get_func(0), __func__);
    test_error(0, ERRNO, ENOENT);
}

TEST_CASE(error_push_last_override_desc)
{
    error_clear();
    test_void(error_push_last_sx(TEST_ERROR_OVERRIDE, NULL, "override"));
    test_stack_error(0, TEST_ERROR_OVERRIDE, E_TEST_ERROR_OV1);
    test_str_eq(error_stack_get_desc(0), "override");
}

TEST_CASE(error_push_last_override_ctx)
{
    error_clear();
    test_void(error_push_last_sx(TEST_ERROR_OVERRIDE, "override", NULL));
    test_stack_error(0, TEST_ERROR_OVERRIDE, E_TEST_ERROR_OV2);
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
    error_pack_f(__func__, NULL, E_TEST_ERROR_2, NULL);
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

TEST_CASE(error_pack_override_desc)
{
    test_void(error_set_s(TEST_ERROR, E_TEST_ERROR_1));
    test_void(error_pack_sd(TEST_ERROR, E_TEST_ERROR_2, "override"));

    test_stack_error(1, TEST_ERROR, E_TEST_ERROR_2);
    test_str_eq(error_stack_get_desc(1), "override");
}

TEST_CASE(error_pack_default_override_desc)
{
    test_void(error_set_s(TEST_ERROR, E_TEST_ERROR_1));
    test_void(error_pack_d(E_TEST_ERROR_2, "override"));

    test_stack_error(1, TEST_ERROR, E_TEST_ERROR_2);
    test_str_eq(error_stack_get_desc(1), "override");
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
    error_pack_last_f(__func__, NULL, NULL, NULL);
}

TEST_CASE_ABORT(error_pack_last_unsupported)
{
    test_void(error_set_s(TEST_ERROR, E_TEST_ERROR_1));
    error_pack_last_s(TEST_ERROR);
}

TEST_CASE(error_pack_last)
{
    test_void(error_set_s(TEST_ERROR, E_TEST_ERROR_1));
    errno = EBADF;
    test_void(error_pack_last_s(ERRNO));

    test_uint_eq(error_depth(), 2);
    test_stack_error(0, TEST_ERROR, E_TEST_ERROR_1);
    test_stack_error(1, ERRNO, EBADF);
    test_str_eq(error_stack_get_func(0), __func__);
    test_str_eq(error_stack_get_func(1), __func__);
    test_error(0, ERRNO, EBADF);
    test_error(1, TEST_ERROR, E_TEST_ERROR_1);
}

TEST_CASE(error_pack_last_override_desc)
{
    test_void(error_set_s(TEST_ERROR, E_TEST_ERROR_1));
    test_void(error_pack_last_sx(TEST_ERROR_OVERRIDE, NULL, "override"));

    test_stack_error(1, TEST_ERROR_OVERRIDE, E_TEST_ERROR_OV1);
    test_str_eq(error_stack_get_desc(1), "override");
}

TEST_CASE(error_pack_last_override_ctx)
{
    test_void(error_set_s(TEST_ERROR, E_TEST_ERROR_1));
    test_void(error_pack_last_sx(TEST_ERROR_OVERRIDE, "override", NULL));

    test_stack_error(1, TEST_ERROR_OVERRIDE, E_TEST_ERROR_OV2);
}

static int _test_error_map(const error_type_st *type, int code)
{
    switch(code)
    {
    case EINVAL:
        return E_TEST_ERROR_1;

    case EISDIR:
        return E_TEST_ERROR_2;

    case EACCES:
        return E_TEST_ERROR_3;

    default:
        return E_GENERIC_WRAP;
    }
}

TEST_CASE_ABORT(error_map_invalid_type)
{
    test_void(error_set_s(ERRNO, EINVAL));
    error_map_f(__func__, NULL, _test_error_map);
}

TEST_CASE_ABORT(error_map_missing)
{
    error_clear();
    error_map_s(TEST_ERROR, _test_error_map);
}

TEST_CASE(error_map)
{
    test_void(error_set_s(ERRNO, EINVAL));
    test_void(error_map_s(TEST_ERROR, _test_error_map));

    test_uint_eq(error_depth(), 2);
    test_stack_error(0, ERRNO, EINVAL);
    test_stack_error(1, TEST_ERROR, E_TEST_ERROR_1);
    test_str_eq(error_stack_get_func(0), __func__);
    test_str_eq(error_stack_get_func(1), __func__);
    test_error(0, TEST_ERROR, E_TEST_ERROR_1);
    test_error(1, ERRNO, EINVAL);
}

TEST_CASE(error_map_default)
{
    test_void(error_set_s(ERRNO, EINVAL));
    test_void(error_map(_test_error_map));

    test_uint_eq(error_depth(), 2);
    test_stack_error(0, ERRNO, EINVAL);
    test_stack_error(1, TEST_ERROR, E_TEST_ERROR_1);
    test_str_eq(error_stack_get_func(0), __func__);
    test_str_eq(error_stack_get_func(1), __func__);
}

TEST_CASE(error_map_no_match)
{
    test_void(error_set_s(ERRNO, ENOSYS));
    test_void(error_map_s(TEST_ERROR, _test_error_map));

    test_uint_eq(error_depth(), 2);
    test_stack_error(0, ERRNO, ENOSYS);
    test_stack_error(1, GENERIC, E_GENERIC_WRAP);
    test_str_eq(error_stack_get_func(0), __func__);
    test_str_eq(error_stack_get_func(1), __func__);
    test_error(0, GENERIC, E_GENERIC_WRAP);
    test_error(1, ERRNO, ENOSYS);
}

TEST_CASE(error_map_system)
{
    test_void(error_set_s(GENERIC, E_GENERIC_SYSTEM));
    test_void(error_map_s(TEST_ERROR, _test_error_map));

    test_uint_eq(error_depth(), 2);
    test_stack_error(0, GENERIC, E_GENERIC_SYSTEM);
    test_stack_error(1, GENERIC, E_GENERIC_PASS);
    test_error(0, GENERIC, E_GENERIC_SYSTEM);
}

TEST_CASE(error_map_oom)
{
    test_void(error_set_s(ERRNO, ENOMEM));
    test_void(error_map_s(TEST_ERROR, _test_error_map));

    test_uint_eq(error_depth(), 2);
    test_stack_error(0, ERRNO, ENOMEM);
    test_stack_error(1, GENERIC, E_GENERIC_OOM);
    test_error(0, GENERIC, E_GENERIC_OOM);
    test_error(1, ERRNO, ENOMEM);
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
    error_pass_sub_f(__func__, "foo", NULL, ENOSYS, NULL);
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

TEST_CASE(error_pass_sub_override_desc)
{
    test_void(error_pass_sub_d(foo, ERRNO, ENOSYS, "override"));

    test_stack_error(0, ERRNO, ENOSYS);
    test_str_eq(error_stack_get_desc(0), "override");
}

TEST_CASE_ABORT(error_pass_last_sub_invalid_type)
{
    error_pass_last_sub_f(__func__, "bar", NULL, NULL, NULL);
}

TEST_CASE(error_pass_last_sub)
{
    errno = EBUSY;
    test_void(error_pass_last_sub(bar, ERRNO));

    test_uint_eq(error_depth(), 2);
    test_stack_error(0, ERRNO, EBUSY);
    test_stack_error(1, GENERIC, E_GENERIC_PASS);
    test_str_eq(error_stack_get_func(0), "bar");
    test_str_eq(error_stack_get_func(1), __func__);
}

TEST_CASE(error_pass_last_sub_override_desc)
{
    test_void(error_pass_last_sub_x(bar, TEST_ERROR_OVERRIDE, NULL, "override"));

    test_stack_error(0, TEST_ERROR_OVERRIDE, E_TEST_ERROR_OV1);
    test_str_eq(error_stack_get_desc(0), "override");
}

TEST_CASE(error_pass_last_sub_override_ctx)
{
    test_void(error_pass_last_sub_x(bar, TEST_ERROR_OVERRIDE, "override", NULL));

    test_stack_error(0, TEST_ERROR_OVERRIDE, E_TEST_ERROR_OV2);
}

TEST_CASE_ABORT(error_wrap_sub_invalid_type)
{
    error_wrap_sub_f(__func__, "baz", NULL, ENODEV, NULL);
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

TEST_CASE(error_wrap_sub_override_desc)
{
    test_void(error_wrap_sub_d(baz, ERRNO, ENODEV, "override"));

    test_stack_error(0, ERRNO, ENODEV);
    test_str_eq(error_stack_get_desc(0), "override");
}

TEST_CASE_ABORT(error_wrap_last_sub_invalid_type)
{
    error_wrap_last_sub_f(__func__, "boz", NULL, NULL, NULL);
}

TEST_CASE(error_wrap_last_sub)
{
    errno = ENOENT;
    test_void(error_wrap_last_sub(boz, ERRNO));

    test_uint_eq(error_depth(), 2);
    test_stack_error(0, ERRNO, ENOENT);
    test_stack_error(1, GENERIC, E_GENERIC_WRAP);
    test_str_eq(error_stack_get_func(0), "boz");
    test_str_eq(error_stack_get_func(1), __func__);
}

TEST_CASE(error_wrap_last_sub_override_desc)
{
    test_void(error_wrap_last_sub_x(boz, TEST_ERROR_OVERRIDE, NULL, "override"));

    test_stack_error(0, TEST_ERROR_OVERRIDE, E_TEST_ERROR_OV1);
    test_str_eq(error_stack_get_desc(0), "override");
}

TEST_CASE(error_wrap_last_sub_override_ctx)
{
    test_void(error_wrap_last_sub_x(boz, TEST_ERROR_OVERRIDE, "override", NULL));

    test_stack_error(0, TEST_ERROR_OVERRIDE, E_TEST_ERROR_OV2);
}

TEST_CASE_ABORT(error_pack_sub_invalid_type1)
{
    error_pack_sub_f(__func__, NULL, E_TEST_ERROR_1, NULL,
        "buzz", ERROR_TYPE(ERRNO), EPERM, NULL);
}

TEST_CASE_ABORT(error_pack_sub_invalid_type2)
{
    error_pack_sub_f(__func__, ERROR_TYPE(TEST_ERROR), E_TEST_ERROR_1, NULL,
        "buzz", NULL, EPERM, NULL);
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

TEST_CASE(error_pack_sub_override_desc)
{
    test_void(error_pack_sub_d(E_TEST_ERROR_1, buzz, ERRNO, EPERM, "override"));

    test_stack_error(0, ERRNO, EPERM);
    test_str_eq(error_stack_get_desc(0), "override");
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
    error_pack_last_sub_f(__func__, NULL, E_TEST_ERROR_2, NULL,
        "bazz", ERROR_TYPE(ERRNO), NULL, NULL);
}

TEST_CASE_ABORT(error_pack_last_sub_invalid_type2)
{
    error_pack_last_sub_f(__func__, ERROR_TYPE(TEST_ERROR), E_TEST_ERROR_2, NULL,
        "bazz", NULL, NULL, NULL);
}

TEST_CASE(error_pack_last_sub)
{
    errno = EACCES;
    test_void(error_pack_last_sub(E_TEST_ERROR_2, bazz, ERRNO));

    test_uint_eq(error_depth(), 2);
    test_stack_error(0, ERRNO, EACCES);
    test_stack_error(1, TEST_ERROR, E_TEST_ERROR_2);
    test_str_eq(error_stack_get_func(0), "bazz");
    test_str_eq(error_stack_get_func(1), __func__);
}

TEST_CASE(error_pack_last_sub_override_desc)
{
    test_void(error_pack_last_sub_x(E_TEST_ERROR_2, bazz, TEST_ERROR_OVERRIDE, NULL, "override"));

    test_stack_error(0, TEST_ERROR_OVERRIDE, E_TEST_ERROR_OV1);
    test_str_eq(error_stack_get_desc(0), "override");
}

TEST_CASE(error_pack_last_sub_override_ctx)
{
    test_void(error_pack_last_sub_x(E_TEST_ERROR_2, bazz, TEST_ERROR_OVERRIDE, "override", NULL));

    test_stack_error(0, TEST_ERROR_OVERRIDE, E_TEST_ERROR_OV2);
}

TEST_CASE(error_pack_last_sub_oom)
{
    errno = ENOMEM;
    test_void(error_pack_last_sub(E_TEST_ERROR_1, bizz, ERRNO));

    test_uint_eq(error_depth(), 2);
    test_stack_error(0, ERRNO, ENOMEM);
    test_stack_error(1, GENERIC, E_GENERIC_OOM);
    test_str_eq(error_stack_get_func(0), "bizz");
    test_str_eq(error_stack_get_func(1), __func__);
}

TEST_CASE_ABORT(error_map_sub_invalid_type1)
{
    error_map_sub_f(__func__, NULL, _test_error_map,
        "buff", ERROR_TYPE(ERRNO), EISDIR, NULL);
}

TEST_CASE_ABORT(error_map_sub_invalid_type2)
{
    error_map_sub_f(__func__, ERROR_TYPE(TEST_ERROR), _test_error_map,
        "buff", NULL, EISDIR, NULL);
}

TEST_CASE(error_map_sub)
{
    test_void(error_map_sub(_test_error_map, buff, ERRNO, EISDIR));

    test_uint_eq(error_depth(), 2);
    test_stack_error(0, ERRNO, EISDIR);
    test_stack_error(1, TEST_ERROR, E_TEST_ERROR_2);
    test_str_eq(error_stack_get_func(0), "buff");
    test_str_eq(error_stack_get_func(1), __func__);
}

TEST_CASE(error_map_sub_override_desc)
{
    test_void(error_map_sub_d(_test_error_map, buff, ERRNO, EISDIR, "override"));

    test_stack_error(0, ERRNO, EISDIR);
    test_str_eq(error_stack_get_desc(0), "override");
}

TEST_CASE(error_map_sub_no_match)
{
    test_void(error_map_sub(_test_error_map, buff, ERRNO, ENOSYS));

    test_uint_eq(error_depth(), 2);
    test_stack_error(0, ERRNO, ENOSYS);
    test_stack_error(1, GENERIC, E_GENERIC_WRAP);
    test_str_eq(error_stack_get_func(0), "buff");
    test_str_eq(error_stack_get_func(1), __func__);
}

TEST_CASE(error_map_sub_oom)
{
    test_void(error_map_sub(_test_error_map, boff, ERRNO, ENOMEM));

    test_uint_eq(error_depth(), 2);
    test_stack_error(0, ERRNO, ENOMEM);
    test_stack_error(1, GENERIC, E_GENERIC_OOM);
    test_str_eq(error_stack_get_func(0), "boff");
    test_str_eq(error_stack_get_func(1), __func__);
}

TEST_CASE_ABORT(error_map_last_sub_invalid_type1)
{
    error_map_last_sub_f(__func__, NULL, _test_error_map, "bazz", ERROR_TYPE(ERRNO), NULL, NULL);
}

TEST_CASE_ABORT(error_map_last_sub_invalid_type2)
{
    error_map_last_sub_f(__func__, ERROR_TYPE(TEST_ERROR), _test_error_map, "bazz", NULL, NULL, NULL);
}

TEST_CASE(error_map_last_sub)
{
    errno = EACCES;
    test_void(error_map_last_sub(_test_error_map, bazz, ERRNO));

    test_uint_eq(error_depth(), 2);
    test_stack_error(0, ERRNO, EACCES);
    test_stack_error(1, TEST_ERROR, E_TEST_ERROR_3);
    test_str_eq(error_stack_get_func(0), "bazz");
    test_str_eq(error_stack_get_func(1), __func__);
}

TEST_CASE(error_map_last_sub_override_desc)
{
    test_void(error_map_last_sub_x(_test_error_map, bazz, TEST_ERROR_OVERRIDE, NULL, "override"));

    test_stack_error(0, TEST_ERROR_OVERRIDE, E_TEST_ERROR_OV1);
    test_str_eq(error_stack_get_desc(0), "override");
}

TEST_CASE(error_map_last_sub_override_ctx)
{
    test_void(error_map_last_sub_x(_test_error_map, bazz, TEST_ERROR_OVERRIDE, "override", NULL));

    test_stack_error(0, TEST_ERROR_OVERRIDE, E_TEST_ERROR_OV2);
}

TEST_CASE(error_map_last_sub_no_match)
{
    errno = ENOSYS;
    test_void(error_map_last_sub(_test_error_map, buzz, ERRNO));

    test_uint_eq(error_depth(), 2);
    test_stack_error(0, ERRNO, ENOSYS);
    test_stack_error(1, GENERIC, E_GENERIC_WRAP);
    test_str_eq(error_stack_get_func(0), "buzz");
    test_str_eq(error_stack_get_func(1), __func__);
}

TEST_CASE(error_map_last_sub_oom)
{
    errno = ENOMEM;
    test_void(error_map_last_sub(_test_error_map, bizz, ERRNO));

    test_uint_eq(error_depth(), 2);
    test_stack_error(0, ERRNO, ENOMEM);
    test_stack_error(1, GENERIC, E_GENERIC_OOM);
    test_str_eq(error_stack_get_func(0), "bizz");
    test_str_eq(error_stack_get_func(1), __func__);
}

TEST_CASE_ABORT(error_map_pre_sub_invalid_type)
{
    error_map_pre_sub_f(__func__, NULL, _test_error_map, "bazz");
}

TEST_CASE(error_map_pre_sub)
{
    test_void(error_set_s(ERRNO, EACCES));
    test_void(error_map_pre_sub(_test_error_map, bazz));

    test_uint_eq(error_depth(), 2);
    test_stack_error(0, ERRNO, EACCES);
    test_stack_error(1, TEST_ERROR, E_TEST_ERROR_3);
    test_str_eq(error_stack_get_func(0), "bazz");
    test_str_eq(error_stack_get_func(1), __func__);
}

TEST_CASE(error_map_pre_sub_no_match)
{
    test_void(error_set_s(ERRNO, ENOSYS));
    test_void(error_map_pre_sub(_test_error_map, buzz));

    test_uint_eq(error_depth(), 2);
    test_stack_error(0, ERRNO, ENOSYS);
    test_stack_error(1, GENERIC, E_GENERIC_WRAP);
    test_str_eq(error_stack_get_func(0), "buzz");
    test_str_eq(error_stack_get_func(1), __func__);
}

TEST_CASE(error_map_pre_sub_oom)
{
    test_void(error_set_s(ERRNO, ENOMEM));
    test_void(error_map_pre_sub(_test_error_map, bizz));

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
    test_ptr_eq(error_stack_get_type(0), ERROR_TYPE(GENERIC));
    test_int_eq(error_stack_get_code(0), E_GENERIC_SYSTEM);
    test_str_eq(error_stack_get_name(0), "E_GENERIC_SYSTEM");
    test_str_eq(error_stack_get_desc(0), "System error.");
}

TEST_CASE(error_info_errno)
{
    test_void(error_set_s(ERRNO, ENOMEM));

    test_uint_eq(error_depth(), 1);
    test_ptr_eq(error_stack_get_type(0), ERROR_TYPE(ERRNO));
    test_int_eq(error_stack_get_code(0), ENOMEM);
    test_str_eq(error_stack_get_name(0), strerrno(ENOMEM));
    test_str_eq(error_stack_get_desc(0), strerror(ENOMEM));
    test_true(error_stack_is_oom(0));
}

TEST_CASE(error_pass_errno)
{
    test_void(error_pass_errno(foo, EFAULT));

    test_uint_eq(error_depth(), 2);
    test_stack_error(0, ERRNO, EFAULT);
    test_stack_error(1, GENERIC, E_GENERIC_PASS);
    test_str_eq(error_stack_get_func(0), "foo");
}

TEST_CASE(error_pass_last_errno)
{
    errno = EEXIST;
    test_void(error_pass_last_errno(foo));

    test_uint_eq(error_depth(), 2);
    test_stack_error(0, ERRNO, EEXIST);
    test_stack_error(1, GENERIC, E_GENERIC_PASS);
    test_str_eq(error_stack_get_func(0), "foo");
}

TEST_CASE(error_wrap_errno)
{
    test_void(error_wrap_errno(foo, EFAULT));

    test_uint_eq(error_depth(), 2);
    test_stack_error(0, ERRNO, EFAULT);
    test_stack_error(1, GENERIC, E_GENERIC_WRAP);
    test_str_eq(error_stack_get_func(0), "foo");
}

TEST_CASE(error_wrap_last_errno)
{
    errno = EEXIST;
    test_void(error_wrap_last_errno(foo));

    test_uint_eq(error_depth(), 2);
    test_stack_error(0, ERRNO, EEXIST);
    test_stack_error(1, GENERIC, E_GENERIC_WRAP);
    test_str_eq(error_stack_get_func(0), "foo");
}

TEST_CASE(error_wrap_errno_ENOMEM)
{
    test_void(error_wrap_errno(foo, ENOMEM));

    test_uint_eq(error_depth(), 2);
    test_stack_error(0, ERRNO, ENOMEM);
    test_stack_error(1, GENERIC, E_GENERIC_OOM);
}

TEST_CASE(error_pack_errno)
{
    test_void(error_pack_errno(E_TEST_ERROR_1, foo, EFAULT));

    test_uint_eq(error_depth(), 2);
    test_stack_error(0, ERRNO, EFAULT);
    test_stack_error(1, TEST_ERROR, E_TEST_ERROR_1);
    test_str_eq(error_stack_get_func(0), "foo");
}

TEST_CASE(error_pack_last_errno)
{
    errno = EEXIST;
    test_void(error_pack_last_errno(E_TEST_ERROR_1, foo));

    test_uint_eq(error_depth(), 2);
    test_stack_error(0, ERRNO, EEXIST);
    test_stack_error(1, TEST_ERROR, E_TEST_ERROR_1);
    test_str_eq(error_stack_get_func(0), "foo");
}

TEST_CASE(error_pack_errno_ENOMEM)
{
    test_void(error_pack_errno(E_TEST_ERROR_1, foo, ENOMEM));

    test_uint_eq(error_depth(), 2);
    test_stack_error(0, ERRNO, ENOMEM);
    test_stack_error(1, GENERIC, E_GENERIC_OOM);
}

TEST_CASE(error_map_errno)
{
    test_void(error_map_errno(_test_error_map, foo, EINVAL));

    test_uint_eq(error_depth(), 2);
    test_stack_error(0, ERRNO, EINVAL);
    test_stack_error(1, TEST_ERROR, E_TEST_ERROR_1);
    test_str_eq(error_stack_get_func(0), "foo");
}

TEST_CASE(error_map_last_errno)
{
    errno = EISDIR;
    test_void(error_map_last_errno(_test_error_map, foo));

    test_uint_eq(error_depth(), 2);
    test_stack_error(0, ERRNO, EISDIR);
    test_stack_error(1, TEST_ERROR, E_TEST_ERROR_2);
    test_str_eq(error_stack_get_func(0), "foo");
}

TEST_CASE(error_map_errno_no_match)
{
    test_void(error_map_errno(_test_error_map, foo, ENOSYS));

    test_uint_eq(error_depth(), 2);
    test_stack_error(0, ERRNO, ENOSYS);
    test_stack_error(1, GENERIC, E_GENERIC_WRAP);
}

TEST_CASE(error_map_errno_ENOMEM)
{
    test_void(error_map_errno(_test_error_map, foo, ENOMEM));

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
    test_ptr_eq(error_stack_get_type(0), ERROR_TYPE(WIN32));
    test_int_eq(error_stack_get_code(0), ERROR_NOT_ENOUGH_MEMORY);
    test_str_eq(error_stack_get_name(0), "WIN32_00000008");
    test_str_eq(error_stack_get_desc(0), msg);
    test_true(error_stack_is_oom(0));

    LocalFree(msg);
}

TEST_CASE(error_pass_win32)
{
    test_void(error_pass_win32(foo, ERROR_FILE_NOT_FOUND));

    test_uint_eq(error_depth(), 2);
    test_stack_error(0, WIN32, ERROR_FILE_NOT_FOUND);
    test_stack_error(1, GENERIC, E_GENERIC_PASS);
    test_str_eq(error_stack_get_func(0), "foo");
}

TEST_CASE(error_pass_last_win32)
{
    SetLastError(ERROR_PATH_NOT_FOUND);
    test_void(error_pass_last_win32(foo));

    test_uint_eq(error_depth(), 2);
    test_stack_error(0, WIN32, ERROR_PATH_NOT_FOUND);
    test_stack_error(1, GENERIC, E_GENERIC_PASS);
    test_str_eq(error_stack_get_func(0), "foo");
}

TEST_CASE(error_wrap_win32)
{
    test_void(error_wrap_win32(foo, ERROR_FILE_NOT_FOUND));

    test_uint_eq(error_depth(), 2);
    test_stack_error(0, WIN32, ERROR_FILE_NOT_FOUND);
    test_stack_error(1, GENERIC, E_GENERIC_WRAP);
    test_str_eq(error_stack_get_func(0), "foo");
}

TEST_CASE(error_wrap_last_win32)
{
    SetLastError(ERROR_PATH_NOT_FOUND);
    test_void(error_wrap_last_win32(foo));

    test_uint_eq(error_depth(), 2);
    test_stack_error(0, WIN32, ERROR_PATH_NOT_FOUND);
    test_stack_error(1, GENERIC, E_GENERIC_WRAP);
    test_str_eq(error_stack_get_func(0), "foo");
}

TEST_CASE(error_wrap_win32_ERROR_NOT_ENOUGH_MEMORY)
{
    test_void(error_wrap_win32(foo, ERROR_NOT_ENOUGH_MEMORY));

    test_uint_eq(error_depth(), 2);
    test_stack_error(0, WIN32, ERROR_NOT_ENOUGH_MEMORY);
    test_stack_error(1, GENERIC, E_GENERIC_OOM);
}

TEST_CASE(error_wrap_win32_ERROR_OUTOFMEMORY)
{
    test_void(error_wrap_win32(foo, ERROR_OUTOFMEMORY));

    test_uint_eq(error_depth(), 2);
    test_stack_error(0, WIN32, ERROR_OUTOFMEMORY);
    test_stack_error(1, GENERIC, E_GENERIC_OOM);
}

TEST_CASE(error_pack_win32)
{
    test_void(error_pack_win32(E_TEST_ERROR_1, foo, ERROR_FILE_NOT_FOUND));

    test_uint_eq(error_depth(), 2);
    test_stack_error(0, WIN32, ERROR_FILE_NOT_FOUND);
    test_stack_error(1, TEST_ERROR, E_TEST_ERROR_1);
    test_str_eq(error_stack_get_func(0), "foo");
}

TEST_CASE(error_pack_last_win32)
{
    SetLastError(ERROR_PATH_NOT_FOUND);
    test_void(error_pack_last_win32(E_TEST_ERROR_1, foo));

    test_uint_eq(error_depth(), 2);
    test_stack_error(0, WIN32, ERROR_PATH_NOT_FOUND);
    test_stack_error(1, TEST_ERROR, E_TEST_ERROR_1);
    test_str_eq(error_stack_get_func(0), "foo");
}

TEST_CASE(error_pack_win32_ERROR_NOT_ENOUGH_MEMORY)
{
    test_void(error_pack_win32(E_TEST_ERROR_1, foo, ERROR_NOT_ENOUGH_MEMORY));

    test_uint_eq(error_depth(), 2);
    test_stack_error(0, WIN32, ERROR_NOT_ENOUGH_MEMORY);
    test_stack_error(1, GENERIC, E_GENERIC_OOM);
}

TEST_CASE(error_pack_win32_ERROR_OUTOFMEMORY)
{
    test_void(error_pack_win32(E_TEST_ERROR_1, foo, ERROR_OUTOFMEMORY));

    test_uint_eq(error_depth(), 2);
    test_stack_error(0, WIN32, ERROR_OUTOFMEMORY);
    test_stack_error(1, GENERIC, E_GENERIC_OOM);
}

static int _test_error_map_win(const error_type_st *type, int code)
{
    switch(code)
    {
    case ERROR_FILE_NOT_FOUND:
    case HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND):
    case STATUS_TIMEOUT:
        return E_TEST_ERROR_1;

    case ERROR_PATH_NOT_FOUND:
        return E_TEST_ERROR_2;

    default:
        return E_GENERIC_WRAP;
    }
}

TEST_CASE(error_map_win32)
{
    test_void(error_map_win32(_test_error_map_win, foo, ERROR_FILE_NOT_FOUND));

    test_uint_eq(error_depth(), 2);
    test_stack_error(0, WIN32, ERROR_FILE_NOT_FOUND);
    test_stack_error(1, TEST_ERROR, E_TEST_ERROR_1);
    test_str_eq(error_stack_get_func(0), "foo");
}

TEST_CASE(error_map_last_win32)
{
    SetLastError(ERROR_PATH_NOT_FOUND);
    test_void(error_map_last_win32(_test_error_map_win, foo));

    test_uint_eq(error_depth(), 2);
    test_stack_error(0, WIN32, ERROR_PATH_NOT_FOUND);
    test_stack_error(1, TEST_ERROR, E_TEST_ERROR_2);
    test_str_eq(error_stack_get_func(0), "foo");
}

TEST_CASE(error_map_win32_no_match)
{
    test_void(error_map_win32(_test_error_map_win, foo, ERROR_INVALID_FUNCTION));

    test_uint_eq(error_depth(), 2);
    test_stack_error(0, WIN32, ERROR_INVALID_FUNCTION);
    test_stack_error(1, GENERIC, E_GENERIC_WRAP);
}

TEST_CASE(error_map_win32_ERROR_NOT_ENOUGH_MEMORY)
{
    test_void(error_map_win32(_test_error_map_win, foo, ERROR_NOT_ENOUGH_MEMORY));

    test_uint_eq(error_depth(), 2);
    test_stack_error(0, WIN32, ERROR_NOT_ENOUGH_MEMORY);
    test_stack_error(1, GENERIC, E_GENERIC_OOM);
}

TEST_CASE(error_map_win32_ERROR_OUTOFMEMORY)
{
    test_void(error_map_win32(_test_error_map_win, foo, ERROR_OUTOFMEMORY));

    test_uint_eq(error_depth(), 2);
    test_stack_error(0, WIN32, ERROR_OUTOFMEMORY);
    test_stack_error(1, GENERIC, E_GENERIC_OOM);
}

TEST_CASE(error_info_hresult)
{
    test_void(error_set_s(HRESULT, HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND)));

    test_uint_eq(error_depth(), 1);
    test_ptr_eq(error_stack_get_type(0), ERROR_TYPE(HRESULT));
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
    test_str_eq(error_stack_get_func(0), "foo");
}

TEST_CASE(error_wrap_hresult)
{
    test_void(error_wrap_hresult(foo, HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND)));

    test_uint_eq(error_depth(), 2);
    test_stack_error(0, HRESULT, HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND));
    test_stack_error(1, GENERIC, E_GENERIC_WRAP);
    test_str_eq(error_stack_get_func(0), "foo");
}

TEST_CASE(error_pack_hresult)
{
    test_void(error_pack_hresult(E_TEST_ERROR_1, foo, HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND)));

    test_uint_eq(error_depth(), 2);
    test_stack_error(0, HRESULT, HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND));
    test_stack_error(1, TEST_ERROR, E_TEST_ERROR_1);
    test_str_eq(error_stack_get_func(0), "foo");
}

TEST_CASE(error_map_hresult)
{
    test_void(error_map_hresult(_test_error_map_win, foo, HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND)));

    test_uint_eq(error_depth(), 2);
    test_stack_error(0, HRESULT, HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND));
    test_stack_error(1, TEST_ERROR, E_TEST_ERROR_1);
    test_str_eq(error_stack_get_func(0), "foo");
}

TEST_CASE(error_map_hresult_no_match)
{
    test_void(error_map_hresult(_test_error_map_win, foo, HRESULT_FROM_WIN32(ERROR_INVALID_FUNCTION)));

    test_uint_eq(error_depth(), 2);
    test_stack_error(0, HRESULT, HRESULT_FROM_WIN32(ERROR_INVALID_FUNCTION));
    test_stack_error(1, GENERIC, E_GENERIC_WRAP);
}

TEST_CASE(error_info_ntstatus)
{
    test_void(error_set_s(NTSTATUS, STATUS_TIMEOUT));

    test_uint_eq(error_depth(), 1);
    test_ptr_eq(error_stack_get_type(0), ERROR_TYPE(NTSTATUS));
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
    test_str_eq(error_stack_get_func(0), "foo");
}

TEST_CASE(error_wrap_ntstatus)
{
    test_void(error_wrap_ntstatus(foo, STATUS_TIMEOUT));

    test_uint_eq(error_depth(), 2);
    test_stack_error(0, NTSTATUS, STATUS_TIMEOUT);
    test_stack_error(1, GENERIC, E_GENERIC_WRAP);
    test_str_eq(error_stack_get_func(0), "foo");
}

TEST_CASE(error_pack_ntstatus)
{
    test_void(error_pack_ntstatus(E_TEST_ERROR_1, foo, STATUS_TIMEOUT));

    test_uint_eq(error_depth(), 2);
    test_stack_error(0, NTSTATUS, STATUS_TIMEOUT);
    test_stack_error(1, TEST_ERROR, E_TEST_ERROR_1);
    test_str_eq(error_stack_get_func(0), "foo");
}

TEST_CASE(error_map_ntstatus)
{
    test_void(error_map_ntstatus(_test_error_map_win, foo, STATUS_TIMEOUT));

    test_uint_eq(error_depth(), 2);
    test_stack_error(0, NTSTATUS, STATUS_TIMEOUT);
    test_stack_error(1, TEST_ERROR, E_TEST_ERROR_1);
    test_str_eq(error_stack_get_func(0), "foo");
}

TEST_CASE(error_map_ntstatus_no_match)
{
    test_void(error_map_ntstatus(_test_error_map_win, foo, STATUS_ABANDONED));

    test_uint_eq(error_depth(), 2);
    test_stack_error(0, NTSTATUS, STATUS_ABANDONED);
    test_stack_error(1, GENERIC, E_GENERIC_WRAP);
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
        test_case_new(error_type_get_last_x_invalid_type),
        test_case_new(error_type_get_last_x_unsupported),
        test_case_new(error_type_get_last_x),
        test_case_new(error_type_get_last_x_override_desc),
        test_case_new(error_type_get_last_x_override_ctx),

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
        test_case_new(error_set_override_desc),
        test_case_new(error_set_default_override_desc),
        test_case_new(error_set_last_invalid_type),
        test_case_new(error_set_last_unsupported),
        test_case_new(error_set_last),
        test_case_new(error_set_last_override_desc),
        test_case_new(error_set_last_override_ctx),

        test_case_new(error_push_invalid_type),
        test_case_new(error_push),
        test_case_new(error_push_default),
        test_case_new(error_push_override_desc),
        test_case_new(error_push_default_override_desc),
        test_case_new(error_push_last_invalid_type),
        test_case_new(error_push_last_unsupported),
        test_case_new(error_push_last),
        test_case_new(error_push_last_override_desc),
        test_case_new(error_push_last_override_ctx),

        test_case_new(error_reset),

        test_case_new(error_wrap_missing),
        test_case_new(error_wrap),
        test_case_new(error_wrap_system),
        test_case_new(error_wrap_oom),

        test_case_new(error_pack_invalid_type),
        test_case_new(error_pack_missing),
        test_case_new(error_pack),
        test_case_new(error_pack_default),
        test_case_new(error_pack_override_desc),
        test_case_new(error_pack_default_override_desc),
        test_case_new(error_pack_wrap),
        test_case_new(error_pack_system),
        test_case_new(error_pack_oom),
        test_case_new(error_pack_last_invalid_type),
        test_case_new(error_pack_last_unsupported),
        test_case_new(error_pack_last),
        test_case_new(error_pack_last_override_desc),
        test_case_new(error_pack_last_override_ctx),

        test_case_new(error_map_invalid_type),
        test_case_new(error_map_missing),
        test_case_new(error_map),
        test_case_new(error_map_default),
        test_case_new(error_map_no_match),
        test_case_new(error_map_system),
        test_case_new(error_map_oom),

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
        test_case_new(error_pass_sub_override_desc),
        test_case_new(error_pass_last_sub_invalid_type),
        test_case_new(error_pass_last_sub),
        test_case_new(error_pass_last_sub_override_desc),
        test_case_new(error_pass_last_sub_override_ctx),

        test_case_new(error_wrap_sub_invalid_type),
        test_case_new(error_wrap_sub),
        test_case_new(error_wrap_sub_override_desc),
        test_case_new(error_wrap_last_sub_invalid_type),
        test_case_new(error_wrap_last_sub),
        test_case_new(error_wrap_last_sub_override_desc),
        test_case_new(error_wrap_last_sub_override_ctx),

        test_case_new(error_pack_sub_invalid_type1),
        test_case_new(error_pack_sub_invalid_type2),
        test_case_new(error_pack_sub),
        test_case_new(error_pack_sub_override_desc),
        test_case_new(error_pack_sub_oom),
        test_case_new(error_pack_last_sub_invalid_type1),
        test_case_new(error_pack_last_sub_invalid_type2),
        test_case_new(error_pack_last_sub),
        test_case_new(error_pack_last_sub_override_desc),
        test_case_new(error_pack_last_sub_override_ctx),
        test_case_new(error_pack_last_sub_oom),

        test_case_new(error_map_sub_invalid_type1),
        test_case_new(error_map_sub_invalid_type2),
        test_case_new(error_map_sub),
        test_case_new(error_map_sub_override_desc),
        test_case_new(error_map_sub_no_match),
        test_case_new(error_map_sub_oom),
        test_case_new(error_map_last_sub_invalid_type1),
        test_case_new(error_map_last_sub_invalid_type2),
        test_case_new(error_map_last_sub),
        test_case_new(error_map_last_sub_override_desc),
        test_case_new(error_map_last_sub_override_ctx),
        test_case_new(error_map_last_sub_no_match),
        test_case_new(error_map_last_sub_oom),

        test_case_new(error_map_pre_sub_invalid_type),
        test_case_new(error_map_pre_sub),
        test_case_new(error_map_pre_sub_no_match),
        test_case_new(error_map_pre_sub_oom),

        test_case_new(error_info_generic),

        test_case_new(error_info_errno),
        test_case_new(error_pass_errno),
        test_case_new(error_pass_last_errno),
        test_case_new(error_wrap_errno),
        test_case_new(error_wrap_last_errno),
        test_case_new(error_wrap_errno_ENOMEM),
        test_case_new(error_pack_errno),
        test_case_new(error_pack_last_errno),
        test_case_new(error_pack_errno_ENOMEM),
        test_case_new(error_map_errno),
        test_case_new(error_map_last_errno),
        test_case_new(error_map_errno_no_match),
        test_case_new(error_map_errno_ENOMEM),

        test_case_new_windows(error_info_win32),
        test_case_new_windows(error_pass_win32),
        test_case_new_windows(error_pass_last_win32),
        test_case_new_windows(error_wrap_win32),
        test_case_new_windows(error_wrap_last_win32),
        test_case_new_windows(error_wrap_win32_ERROR_NOT_ENOUGH_MEMORY),
        test_case_new_windows(error_wrap_win32_ERROR_OUTOFMEMORY),
        test_case_new_windows(error_pack_win32),
        test_case_new_windows(error_pack_last_win32),
        test_case_new_windows(error_pack_win32_ERROR_NOT_ENOUGH_MEMORY),
        test_case_new_windows(error_pack_win32_ERROR_OUTOFMEMORY),
        test_case_new_windows(error_map_win32),
        test_case_new_windows(error_map_last_win32),
        test_case_new_windows(error_map_win32_no_match),
        test_case_new_windows(error_map_win32_ERROR_NOT_ENOUGH_MEMORY),
        test_case_new_windows(error_map_win32_ERROR_OUTOFMEMORY),

        test_case_new_windows(error_info_hresult),
        test_case_new_windows(error_pass_hresult),
        test_case_new_windows(error_wrap_hresult),
        test_case_new_windows(error_pack_hresult),
        test_case_new_windows(error_map_hresult),
        test_case_new_windows(error_map_hresult_no_match),

        test_case_new_windows(error_info_ntstatus),
        test_case_new_windows(error_pass_ntstatus),
        test_case_new_windows(error_wrap_ntstatus),
        test_case_new_windows(error_pack_ntstatus),
        test_case_new_windows(error_map_ntstatus),
        test_case_new_windows(error_map_ntstatus_no_match)
    );
}