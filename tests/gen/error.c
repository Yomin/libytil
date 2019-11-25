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

#include "error.h"
#include <ytil/test/test.h>
#include <ytil/gen/error.h>
#include <ytil/ext/errno.h>

#ifdef _WIN32
#   include <winerror.h>
#   include <ntstatus.h>
#endif

typedef enum test_error
{
      E_TEST_ERROR_1
    , E_TEST_ERROR_2
    , E_TEST_ERROR_3
} test_error_id;

static const error_info_st error_infos[] =
{
      ERROR_INFO(E_TEST_ERROR_1, "Test error 1.")
    , ERROR_INFO(E_TEST_ERROR_2, "Test error 2.")
    , ERROR_INFO(E_TEST_ERROR_3, "Test error 3.")
};


TEST_CASE(error_set)
{
    test_void(error_set(E_TEST_ERROR_1));
    
    test_uint_eq(error_depth(), 1);
    
    test_int_eq(error_get(0), E_TEST_ERROR_1);
    test_uint_eq(error_type(0), ERROR_TYPE_ERROR);
    test_true(error_check(0, E_TEST_ERROR_1));
    
    test_str_eq(error_func(0), __func__);
    test_str_eq(error_name(0), error_infos[E_TEST_ERROR_1].name);
    test_str_eq(error_desc(0), error_infos[E_TEST_ERROR_1].desc);
    
    test_int_eq(error_get(1), E_ERROR_UNSET);
}

TEST_CASE(error_push)
{
    test_void(error_set(E_TEST_ERROR_1));
    test_void(error_push(E_TEST_ERROR_2));
    
    test_uint_eq(error_depth(), 2);
    
    test_uint_eq(error_type(0), ERROR_TYPE_ERROR);
    test_int_eq(error_get(0), E_TEST_ERROR_2);
    
    test_uint_eq(error_type(1), ERROR_TYPE_ERROR);
    test_int_eq(error_get(1), E_TEST_ERROR_1);
    
    test_int_eq(error_get(2), E_ERROR_UNSET);
}

TEST_CASE(error_reset)
{
    test_void(error_set(E_TEST_ERROR_1));
    test_void(error_push(E_TEST_ERROR_2));
    test_void(error_set(E_TEST_ERROR_3));
    
    test_uint_eq(error_depth(), 1);
    test_int_eq(error_get(0), E_TEST_ERROR_3);
    test_int_eq(error_get(1), E_ERROR_UNSET);
}

TEST_CASE(error_wrap)
{
    test_void(error_set(E_TEST_ERROR_1));
    test_void(error_wrap());
    
    test_uint_eq(error_depth(), 2);
    
    test_uint_eq(error_type(0), ERROR_TYPE_ERROR);
    test_int_eq(error_get(0), E_ERROR_WRAPPER);
    test_str_eq(error_name(0), "E_ERROR_WRAPPER");
    test_str_eq(error_desc(0), "Error Wrapper");
    
    test_uint_eq(error_type(1), ERROR_TYPE_ERROR);
    test_int_eq(error_get(1), E_TEST_ERROR_1);
    
    test_int_eq(error_get(2), E_ERROR_UNSET);
}

TEST_CASE(error_pack_wrapper)
{
    test_void(error_set(E_TEST_ERROR_1));
    test_void(error_wrap());
    test_void(error_pack(E_TEST_ERROR_2));
    
    test_uint_eq(error_depth(), 3);
    
    test_uint_eq(error_type(0), ERROR_TYPE_ERROR);
    test_int_eq(error_get(0), E_ERROR_WRAPPER);
    test_uint_eq(error_type(1), ERROR_TYPE_ERROR);
    test_int_eq(error_get(1), E_ERROR_WRAPPER);
}

TEST_CASE(error_pack_error)
{
    test_void(error_set(E_TEST_ERROR_1));
    test_void(error_pack(E_TEST_ERROR_2));
    
    test_uint_eq(error_depth(), 2);
    
    test_uint_eq(error_type(0), ERROR_TYPE_ERROR);
    test_int_eq(error_get(0), E_TEST_ERROR_2);
    test_uint_eq(error_type(1), ERROR_TYPE_ERROR);
    test_int_eq(error_get(1), E_TEST_ERROR_1);
}

TEST_CASE(error_map_not_found)
{
    test_void(error_set(E_TEST_ERROR_1));
    test_void(error_map(0, E_TEST_ERROR_2, E_TEST_ERROR_3));
    
    test_uint_eq(error_depth(), 2);
    
    test_uint_eq(error_type(0), ERROR_TYPE_ERROR);
    test_int_eq(error_get(0), E_ERROR_WRAPPER);
}

TEST_CASE(error_map_single)
{
    test_void(error_set(E_TEST_ERROR_1));
    test_void(error_map(0, E_TEST_ERROR_1, E_TEST_ERROR_2));
    
    test_uint_eq(error_depth(), 2);
    
    test_uint_eq(error_type(0), ERROR_TYPE_ERROR);
    test_int_eq(error_get(0), E_TEST_ERROR_2);
}

TEST_CASE(error_map_double)
{
    test_void(error_set(E_TEST_ERROR_1));
    test_void(error_map(0, E_TEST_ERROR_2, E_TEST_ERROR_1, E_TEST_ERROR_1, E_TEST_ERROR_3));
    
    test_uint_eq(error_depth(), 2);
    
    test_uint_eq(error_type(0), ERROR_TYPE_ERROR);
    test_int_eq(error_get(0), E_TEST_ERROR_3);
}

TEST_CASE(error_map_wrapper)
{
    test_void(error_set(E_TEST_ERROR_1));
    test_void(error_wrap());
    test_void(error_map(0, E_ERROR_WRAPPER, E_TEST_ERROR_2));
    
    test_uint_eq(error_depth(), 3);
    
    test_uint_eq(error_type(0), ERROR_TYPE_ERROR);
    test_int_eq(error_get(0), E_TEST_ERROR_2);
}

TEST_CASE(error_map_sub)
{
    test_void(error_set(E_TEST_ERROR_1));
    test_void(error_push(E_TEST_ERROR_2));
    test_void(error_map(1, E_TEST_ERROR_1, E_TEST_ERROR_3));
    
    test_uint_eq(error_depth(), 3);
    
    test_uint_eq(error_type(0), ERROR_TYPE_ERROR);
    test_int_eq(error_get(0), E_TEST_ERROR_3);
}

TEST_CASE(error_pass_single)
{
    test_void(error_set(E_TEST_ERROR_1));
    test_void(error_pass());
    
    test_uint_eq(error_depth(), 2);
    
    test_uint_eq(error_stack_get_type(0), ERROR_TYPE_ERROR);
    test_uint_eq(error_stack_get_error(0), E_TEST_ERROR_1);
    
    test_uint_eq(error_stack_get_type(1), ERROR_TYPE_ERROR);
    test_uint_eq(error_stack_get_error(1), E_ERROR_PASS);
    test_str_eq(error_stack_get_name(1), "E_ERROR_PASS");
    test_str_eq(error_stack_get_desc(1), "Error Pass");
    
    test_int_eq(error_get(0), E_TEST_ERROR_1);
    test_int_eq(error_get(1), E_ERROR_UNSET);
}

TEST_CASE(error_pass_double)
{
    test_void(error_set(E_TEST_ERROR_1));
    test_void(error_pass());
    test_void(error_push(E_TEST_ERROR_2));
    test_void(error_pass());
    
    test_uint_eq(error_depth(), 4);
    
    test_uint_eq(error_stack_get_type(0), ERROR_TYPE_ERROR);
    test_uint_eq(error_stack_get_error(0), E_TEST_ERROR_1);
    
    test_uint_eq(error_stack_get_type(1), ERROR_TYPE_ERROR);
    test_uint_eq(error_stack_get_error(1), E_ERROR_PASS);
    
    test_uint_eq(error_stack_get_type(2), ERROR_TYPE_ERROR);
    test_uint_eq(error_stack_get_error(2), E_TEST_ERROR_2);
    
    test_uint_eq(error_stack_get_type(3), ERROR_TYPE_ERROR);
    test_uint_eq(error_stack_get_error(3), E_ERROR_PASS);
    
    test_int_eq(error_get(0), E_TEST_ERROR_2);
    test_int_eq(error_get(1), E_TEST_ERROR_1);
    test_int_eq(error_get(2), E_ERROR_UNSET);
}

TEST_CASE(errno_set)
{
    test_void(errno_set(EINVAL));
    
    test_uint_eq(error_depth(), 1);
    
    test_uint_eq(error_stack_get_type(0), ERROR_TYPE_ERRNO);
    test_uint_eq(error_stack_get_errno(0), EINVAL);
    
    test_uint_eq(error_type(0), ERROR_TYPE_ERRNO);
    test_int_eq(error_get_errno(0), EINVAL);
    
    test_str_eq(error_func(0), __func__);
    test_str_eq(error_name(0), strerrno(EINVAL));
    test_str_eq(error_desc(0), strerror(EINVAL));
    
    test_int_eq(error_get(1), E_ERROR_UNSET);
}

TEST_CASE(errno_push)
{
    test_void(errno_set(EINVAL));
    test_void(errno_push(ENOENT));
    
    test_uint_eq(error_depth(), 2);
    
    test_uint_eq(error_stack_get_type(0), ERROR_TYPE_ERRNO);
    test_uint_eq(error_stack_get_errno(0), EINVAL);
    test_uint_eq(error_stack_get_type(1), ERROR_TYPE_ERRNO);
    test_uint_eq(error_stack_get_errno(1), ENOENT);
    
    test_uint_eq(error_type(0), ERROR_TYPE_ERRNO);
    test_int_eq(error_get_errno(0), ENOENT);
    
    test_uint_eq(error_type(1), ERROR_TYPE_ERRNO);
    test_int_eq(error_get_errno(1), EINVAL);
    
    test_int_eq(error_get(2), E_ERROR_UNSET);
}

TEST_CASE(error_push_errno)
{
    errno = EFAULT;
    test_void(error_push_errno(E_TEST_ERROR_1, foo));
    
    test_uint_eq(error_depth(), 2);
    
    test_uint_eq(error_stack_get_type(0), ERROR_TYPE_ERRNO);
    test_uint_eq(error_stack_get_errno(0), EFAULT);
    test_uint_eq(error_stack_get_type(1), ERROR_TYPE_ERROR);
    test_uint_eq(error_stack_get_error(1), E_TEST_ERROR_1);
    
    test_uint_eq(error_type(0), ERROR_TYPE_ERROR);
    test_int_eq(error_get(0), E_TEST_ERROR_1);
    
    test_uint_eq(error_type(1), ERROR_TYPE_ERRNO);
    test_int_eq(error_get_errno(1), EFAULT);
    test_str_eq(error_func(1), "foo");
    
    test_int_eq(error_get(2), E_ERROR_UNSET);
}

TEST_CASE(error_wrap_errno)
{
    errno = EFAULT;
    test_void(error_wrap_errno(foo));
    
    test_uint_eq(error_depth(), 2);
    
    test_uint_eq(error_stack_get_type(0), ERROR_TYPE_ERRNO);
    test_uint_eq(error_stack_get_errno(0), EFAULT);
    test_uint_eq(error_stack_get_type(1), ERROR_TYPE_ERROR);
    test_uint_eq(error_stack_get_error(1), E_ERROR_WRAPPER);
    
    test_uint_eq(error_type(0), ERROR_TYPE_ERROR);
    test_int_eq(error_get(0), E_ERROR_WRAPPER);
    
    test_uint_eq(error_type(1), ERROR_TYPE_ERRNO);
    test_int_eq(error_get_errno(1), EFAULT);
    test_str_eq(error_func(1), "foo");
    
    test_int_eq(error_get(2), E_ERROR_UNSET);
}

TEST_CASE(error_pass_errno)
{
    errno = EFAULT;
    test_void(error_pass_errno(foo));
    
    test_uint_eq(error_depth(), 2);
    
    test_uint_eq(error_stack_get_type(0), ERROR_TYPE_ERRNO);
    test_uint_eq(error_stack_get_errno(0), EFAULT);
    test_uint_eq(error_stack_get_type(1), ERROR_TYPE_ERROR);
    test_uint_eq(error_stack_get_error(1), E_ERROR_PASS);
    
    test_uint_eq(error_type(0), ERROR_TYPE_ERRNO);
    test_int_eq(error_get_errno(0), EFAULT);
    test_str_eq(error_func(0), "foo");
    
    test_int_eq(error_get(1), E_ERROR_UNSET);
}

#ifdef _WIN32

static char *_test_error_format(DWORD error)
{
    static char *tmp;
    DWORD rc;
    
    if(tmp)
        LocalFree(tmp);
    
    if(error == ERROR_SUCCESS)
        return NULL;
    
    tmp = NULL;
    rc = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_IGNORE_INSERTS|FORMAT_MESSAGE_ALLOCATE_BUFFER,
        NULL, error, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (char*)&tmp, 0, NULL);
    
    for(; rc && (tmp[rc-1] == '\n' || tmp[rc-1] == '\r'); rc--)
        tmp[rc-1] = '\0';
    
    return tmp;
}

TEST_CASE(error_push_win32)
{
    test_void(error_push_win32(E_TEST_ERROR_1, foo, ERROR_FILE_NOT_FOUND));
    
    test_uint_eq(error_depth(), 2);
    
    test_uint_eq(error_stack_get_type(0), ERROR_TYPE_WIN32);
    test_uint_eq(error_stack_get_win32(0), ERROR_FILE_NOT_FOUND);
    test_uint_eq(error_stack_get_type(1), ERROR_TYPE_ERROR);
    test_uint_eq(error_stack_get_error(1), E_TEST_ERROR_1);
    
    test_uint_eq(error_type(0), ERROR_TYPE_ERROR);
    test_int_eq(error_get(0), E_TEST_ERROR_1);
    
    test_uint_eq(error_type(1), ERROR_TYPE_WIN32);
    test_int_eq(error_get_win32(1), ERROR_FILE_NOT_FOUND);
    test_str_eq(error_func(1), "foo");
    test_str_eq(error_name(1), "WIN32_00000002");
    test_str_eq(error_desc(1), _test_error_format(ERROR_FILE_NOT_FOUND));
    _test_error_format(ERROR_SUCCESS);
    
    test_int_eq(error_get(2), E_ERROR_UNSET);
}

TEST_CASE(error_push_last_win32)
{
    SetLastError(ERROR_FILE_NOT_FOUND);
    test_void(error_push_last_win32(E_TEST_ERROR_1, foo));
    
    test_uint_eq(error_depth(), 2);
    
    test_uint_eq(error_stack_get_type(0), ERROR_TYPE_WIN32);
    test_uint_eq(error_stack_get_win32(0), ERROR_FILE_NOT_FOUND);
    test_uint_eq(error_stack_get_type(1), ERROR_TYPE_ERROR);
    test_uint_eq(error_stack_get_error(1), E_TEST_ERROR_1);
    
    test_uint_eq(error_type(0), ERROR_TYPE_ERROR);
    test_int_eq(error_get(0), E_TEST_ERROR_1);
    
    test_uint_eq(error_type(1), ERROR_TYPE_WIN32);
    test_int_eq(error_get_win32(1), ERROR_FILE_NOT_FOUND);
    test_str_eq(error_func(1), "foo");
    
    test_int_eq(error_get(2), E_ERROR_UNSET);
}

TEST_CASE(error_wrap_win32)
{
    test_void(error_wrap_win32(foo, ERROR_FILE_NOT_FOUND));
    
    test_uint_eq(error_depth(), 2);
    
    test_uint_eq(error_stack_get_type(0), ERROR_TYPE_WIN32);
    test_uint_eq(error_stack_get_win32(0), ERROR_FILE_NOT_FOUND);
    test_uint_eq(error_stack_get_type(1), ERROR_TYPE_ERROR);
    test_uint_eq(error_stack_get_error(1), E_ERROR_WRAPPER);
    
    test_uint_eq(error_type(0), ERROR_TYPE_ERROR);
    test_int_eq(error_get(0), E_ERROR_WRAPPER);
    
    test_uint_eq(error_type(1), ERROR_TYPE_WIN32);
    test_int_eq(error_get_win32(1), ERROR_FILE_NOT_FOUND);
    test_str_eq(error_func(1), "foo");
    
    test_int_eq(error_get(2), E_ERROR_UNSET);
}

TEST_CASE(error_wrap_last_win32)
{
    SetLastError(ERROR_FILE_NOT_FOUND);
    test_void(error_wrap_last_win32(foo));
    
    test_uint_eq(error_depth(), 2);
    
    test_uint_eq(error_stack_get_type(0), ERROR_TYPE_WIN32);
    test_uint_eq(error_stack_get_win32(0), ERROR_FILE_NOT_FOUND);
    test_uint_eq(error_stack_get_type(1), ERROR_TYPE_ERROR);
    test_uint_eq(error_stack_get_error(1), E_ERROR_WRAPPER);
    
    test_uint_eq(error_type(0), ERROR_TYPE_ERROR);
    test_int_eq(error_get(0), E_ERROR_WRAPPER);
    
    test_uint_eq(error_type(1), ERROR_TYPE_WIN32);
    test_int_eq(error_get_win32(1), ERROR_FILE_NOT_FOUND);
    test_str_eq(error_func(1), "foo");
    
    test_int_eq(error_get(2), E_ERROR_UNSET);
}

TEST_CASE(error_pass_win32)
{
    test_void(error_pass_win32(foo, ERROR_FILE_NOT_FOUND));
    
    test_uint_eq(error_depth(), 2);
    
    test_uint_eq(error_stack_get_type(0), ERROR_TYPE_WIN32);
    test_uint_eq(error_stack_get_win32(0), ERROR_FILE_NOT_FOUND);
    test_uint_eq(error_stack_get_type(1), ERROR_TYPE_ERROR);
    test_uint_eq(error_stack_get_error(1), E_ERROR_PASS);
    
    test_uint_eq(error_type(0), ERROR_TYPE_WIN32);
    test_int_eq(error_get_win32(0), ERROR_FILE_NOT_FOUND);
    test_str_eq(error_func(0), "foo");
    
    test_int_eq(error_get(1), E_ERROR_UNSET);
}

TEST_CASE(error_pass_last_win32)
{
    SetLastError(ERROR_FILE_NOT_FOUND);
    test_void(error_pass_last_win32(foo));
    
    test_uint_eq(error_depth(), 2);
    
    test_uint_eq(error_stack_get_type(0), ERROR_TYPE_WIN32);
    test_uint_eq(error_stack_get_win32(0), ERROR_FILE_NOT_FOUND);
    test_uint_eq(error_stack_get_type(1), ERROR_TYPE_ERROR);
    test_uint_eq(error_stack_get_error(1), E_ERROR_PASS);
    
    test_uint_eq(error_type(0), ERROR_TYPE_WIN32);
    test_int_eq(error_get_win32(0), ERROR_FILE_NOT_FOUND);
    test_str_eq(error_func(0), "foo");
    
    test_int_eq(error_get(1), E_ERROR_UNSET);
}

TEST_CASE(error_push_hresult)
{
    test_void(error_push_hresult(E_TEST_ERROR_1, foo, HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND)));
    
    test_uint_eq(error_depth(), 2);
    
    test_uint_eq(error_stack_get_type(0), ERROR_TYPE_HRESULT);
    test_uint_eq(error_stack_get_hresult(0), HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND));
    test_uint_eq(error_stack_get_type(1), ERROR_TYPE_ERROR);
    test_uint_eq(error_stack_get_error(1), E_TEST_ERROR_1);
    
    test_uint_eq(error_type(0), ERROR_TYPE_ERROR);
    test_int_eq(error_get(0), E_TEST_ERROR_1);
    
    test_uint_eq(error_type(1), ERROR_TYPE_HRESULT);
    test_int_eq(error_get_hresult(1), HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND));
    test_str_eq(error_func(1), "foo");
    test_str_eq(error_name(1), "HRESULT_80070002");
    test_str_eq(error_desc(1), "<HRESULT_MESSAGE>");
    
    test_int_eq(error_get(2), E_ERROR_UNSET);
}

TEST_CASE(error_wrap_hresult)
{
    test_void(error_wrap_hresult(foo, HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND)));
    
    test_uint_eq(error_depth(), 2);
    
    test_uint_eq(error_stack_get_type(0), ERROR_TYPE_HRESULT);
    test_uint_eq(error_stack_get_hresult(0), HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND));
    test_uint_eq(error_stack_get_type(1), ERROR_TYPE_ERROR);
    test_uint_eq(error_stack_get_error(1), E_ERROR_WRAPPER);
    
    test_uint_eq(error_type(0), ERROR_TYPE_ERROR);
    test_int_eq(error_get(0), E_ERROR_WRAPPER);
    
    test_uint_eq(error_type(1), ERROR_TYPE_HRESULT);
    test_int_eq(error_get_hresult(1), HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND));
    test_str_eq(error_func(1), "foo");
    
    test_int_eq(error_get(2), E_ERROR_UNSET);
}

TEST_CASE(error_pass_hresult)
{
    test_void(error_pass_hresult(foo, HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND)));
    
    test_uint_eq(error_depth(), 2);
    
    test_uint_eq(error_stack_get_type(0), ERROR_TYPE_HRESULT);
    test_uint_eq(error_stack_get_hresult(0), HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND));
    test_uint_eq(error_stack_get_type(1), ERROR_TYPE_ERROR);
    test_uint_eq(error_stack_get_error(1), E_ERROR_PASS);
    
    test_uint_eq(error_type(0), ERROR_TYPE_HRESULT);
    test_int_eq(error_get_hresult(0), HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND));
    test_str_eq(error_func(0), "foo");
    
    test_int_eq(error_get(1), E_ERROR_UNSET);
}

TEST_CASE(error_push_ntstatus)
{
    test_void(error_push_ntstatus(E_TEST_ERROR_1, foo, STATUS_TIMEOUT));
    
    test_uint_eq(error_depth(), 2);
    
    test_uint_eq(error_stack_get_type(0), ERROR_TYPE_NTSTATUS);
    test_uint_eq(error_stack_get_ntstatus(0), STATUS_TIMEOUT);
    test_uint_eq(error_stack_get_type(1), ERROR_TYPE_ERROR);
    test_uint_eq(error_stack_get_error(1), E_TEST_ERROR_1);
    
    test_uint_eq(error_type(0), ERROR_TYPE_ERROR);
    test_int_eq(error_get(0), E_TEST_ERROR_1);
    
    test_uint_eq(error_type(1), ERROR_TYPE_NTSTATUS);
    test_int_eq(error_get_ntstatus(1), STATUS_TIMEOUT);
    test_str_eq(error_func(1), "foo");
    test_str_eq(error_name(1), "NTSTATUS_00000102");
    test_str_eq(error_desc(1), "<NTSTATUS_MESSAGE>");
    
    test_int_eq(error_get(2), E_ERROR_UNSET);
}

TEST_CASE(error_wrap_ntstatus)
{
    test_void(error_wrap_ntstatus(foo, STATUS_TIMEOUT));
    
    test_uint_eq(error_depth(), 2);
    
    test_uint_eq(error_stack_get_type(0), ERROR_TYPE_NTSTATUS);
    test_uint_eq(error_stack_get_ntstatus(0), STATUS_TIMEOUT);
    test_uint_eq(error_stack_get_type(1), ERROR_TYPE_ERROR);
    test_uint_eq(error_stack_get_error(1), E_ERROR_WRAPPER);
    
    test_uint_eq(error_type(0), ERROR_TYPE_ERROR);
    test_int_eq(error_get(0), E_ERROR_WRAPPER);
    
    test_uint_eq(error_type(1), ERROR_TYPE_NTSTATUS);
    test_int_eq(error_get_ntstatus(1), STATUS_TIMEOUT);
    test_str_eq(error_func(1), "foo");
    
    test_int_eq(error_get(2), E_ERROR_UNSET);
}

TEST_CASE(error_pass_ntstatus)
{
    test_void(error_pass_ntstatus(foo, STATUS_TIMEOUT));
    
    test_uint_eq(error_depth(), 2);
    
    test_uint_eq(error_stack_get_type(0), ERROR_TYPE_NTSTATUS);
    test_uint_eq(error_stack_get_ntstatus(0), STATUS_TIMEOUT);
    test_uint_eq(error_stack_get_type(1), ERROR_TYPE_ERROR);
    test_uint_eq(error_stack_get_error(1), E_ERROR_PASS);
    
    test_uint_eq(error_type(0), ERROR_TYPE_NTSTATUS);
    test_int_eq(error_get_ntstatus(0), STATUS_TIMEOUT);
    test_str_eq(error_func(0), "foo");
    
    test_int_eq(error_get(1), E_ERROR_UNSET);
}

#endif // _WIN32

TEST_CASE(error_get_oob)
{
    test_void(error_set(E_TEST_ERROR_1));
    test_int_eq(error_get(1), E_ERROR_UNSET);
}

TEST_CASE_ABORT(error_get_wrong_type)
{
    error_pass_errno(foo);
    error_get(0);
}

TEST_CASE(error_check_oob)
{
    test_void(error_set(E_TEST_ERROR_1));
    test_false(error_check(1, E_TEST_ERROR_1));
}

TEST_CASE_ABORT(error_check_wrong_type)
{
    error_pass_errno(foo);
    error_check(0, E_TEST_ERROR_1);
}

TEST_CASE(error_get_errno_oob)
{
    test_void(errno_set(EINVAL));
    test_int_eq(error_get_errno(1), 0);
}

TEST_CASE_ABORT(error_get_errno_wrong_type)
{
    error_set(E_TEST_ERROR_1);
    error_get_errno(0);
}

TEST_CASE(error_check_errno_oob)
{
    test_void(errno_set(EINVAL));
    test_false(error_check_errno(1, EINVAL));
}

TEST_CASE_ABORT(error_check_errno_wrong_type)
{
    error_set(E_TEST_ERROR_1);
    error_check_errno(0, EINVAL);
}

#ifdef _WIN32

TEST_CASE(error_get_win32_oob)
{
    test_void(error_pass_win32(foo, ERROR_FILE_NOT_FOUND));
    test_uint_eq(error_get_win32(1), ERROR_SUCCESS);
}

TEST_CASE_ABORT(error_get_win32_wrong_type)
{
    error_set(E_TEST_ERROR_1);
    error_get_win32(0);
}

TEST_CASE(error_check_win32_oob)
{
    test_void(error_pass_win32(foo, ERROR_FILE_NOT_FOUND));
    test_false(error_check_win32(1, ERROR_FILE_NOT_FOUND));
}

TEST_CASE_ABORT(error_check_win32_wrong_type)
{
    error_set(E_TEST_ERROR_1);
    error_check_win32(0, ERROR_FILE_NOT_FOUND);
}

TEST_CASE(error_get_hresult_oob)
{
    test_void(error_pass_hresult(foo, HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND)));
    test_uint_eq(error_get_hresult(1), S_OK);
}

TEST_CASE_ABORT(error_get_hresult_wrong_type)
{
    error_set(E_TEST_ERROR_1);
    error_get_hresult(0);
}

TEST_CASE(error_check_hresult_oob)
{
    test_void(error_pass_hresult(foo, HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND)));
    test_false(error_check_hresult(1, HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND)));
}

TEST_CASE_ABORT(error_check_hresult_wrong_type)
{
    error_set(E_TEST_ERROR_1);
    error_check_hresult(0, HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND));
}

TEST_CASE(error_get_ntstatus_oob)
{
    test_void(error_pass_ntstatus(foo, STATUS_TIMEOUT));
    test_uint_eq(error_get_ntstatus(1), STATUS_SUCCESS);
}

TEST_CASE_ABORT(error_get_ntstatus_wrong_type)
{
    error_set(E_TEST_ERROR_1);
    error_get_ntstatus(0);
}

TEST_CASE(error_check_ntstatus_oob)
{
    test_void(error_pass_ntstatus(foo, STATUS_TIMEOUT));
    test_false(error_check_ntstatus(1, STATUS_TIMEOUT));
}

TEST_CASE_ABORT(error_check_ntstatus_wrong_type)
{
    error_set(E_TEST_ERROR_1);
    error_check_ntstatus(0, STATUS_TIMEOUT);
}

#endif // _WIN32

TEST_CASE(error_type_oob)
{
    test_void(error_set(E_TEST_ERROR_1));
    test_uint_eq(error_type(1), ERROR_TYPE_INVALID);
}

TEST_CASE(error_func_oob)
{
    test_void(error_set(E_TEST_ERROR_1));
    test_ptr_eq(error_func(1), NULL);
}

TEST_CASE(error_name_oob)
{
    test_void(error_set(E_TEST_ERROR_1));
    test_ptr_eq(error_name(1), NULL);
}

TEST_CASE(error_desc_oob)
{
    test_void(error_set(E_TEST_ERROR_1));
    test_ptr_eq(error_desc(1), NULL);
}

TEST_CASE(error_stack_get_error_oob)
{
    test_void(error_set(E_TEST_ERROR_1));
    test_int_eq(error_stack_get_error(1), E_ERROR_UNSET);
}

TEST_CASE_ABORT(error_stack_get_error_wrong_type)
{
    error_pass_errno(foo);
    error_stack_get_error(0);
}

TEST_CASE(error_stack_check_error_oob)
{
    test_void(error_set(E_TEST_ERROR_1));
    test_false(error_stack_check_error(1, E_TEST_ERROR_1));
}

TEST_CASE_ABORT(error_stack_check_error_wrong_type)
{
    error_pass_errno(foo);
    error_stack_check_error(0, E_TEST_ERROR_1);
}

TEST_CASE(error_stack_get_errno_oob)
{
    test_void(errno_set(EINVAL));
    test_int_eq(error_stack_get_errno(1), 0);
}

TEST_CASE_ABORT(error_stack_get_errno_wrong_type)
{
    error_set(E_TEST_ERROR_1);
    error_stack_get_errno(0);
}

TEST_CASE(error_stack_check_errno_oob)
{
    test_void(errno_set(EINVAL));
    test_false(error_stack_check_errno(1, EINVAL));
}

TEST_CASE_ABORT(error_stack_check_errno_wrong_type)
{
    error_set(E_TEST_ERROR_1);
    error_stack_check_errno(0, EINVAL);
}

#ifdef _WIN32

TEST_CASE(error_stack_get_win32_oob)
{
    test_void(error_pass_win32(foo, ERROR_FILE_NOT_FOUND));
    test_int_eq(error_stack_get_win32(2), ERROR_SUCCESS);
}

TEST_CASE_ABORT(error_stack_get_win32_wrong_type)
{
    error_set(E_TEST_ERROR_1);
    error_stack_get_win32(0);
}

TEST_CASE(error_stack_check_win32_oob)
{
    test_void(error_pass_win32(foo, ERROR_FILE_NOT_FOUND));
    test_false(error_stack_check_win32(2, ERROR_FILE_NOT_FOUND));
}

TEST_CASE_ABORT(error_stack_check_win32_wrong_type)
{
    error_set(E_TEST_ERROR_1);
    error_stack_check_win32(0, ERROR_FILE_NOT_FOUND);
}

TEST_CASE(error_stack_get_hresult_oob)
{
    test_void(error_pass_hresult(foo, HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND)));
    test_int_eq(error_stack_get_hresult(2), S_OK);
}

TEST_CASE_ABORT(error_stack_get_hresult_wrong_type)
{
    error_set(E_TEST_ERROR_1);
    error_stack_get_hresult(0);
}

TEST_CASE(error_stack_check_hresult_oob)
{
    test_void(error_pass_hresult(foo, HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND)));
    test_false(error_stack_check_hresult(2, HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND)));
}

TEST_CASE_ABORT(error_stack_check_hresult_wrong_type)
{
    error_set(E_TEST_ERROR_1);
    error_stack_check_hresult(0, HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND));
}

TEST_CASE(error_stack_get_ntstatus_oob)
{
    test_void(error_pass_ntstatus(foo, STATUS_TIMEOUT));
    test_int_eq(error_stack_get_ntstatus(2), STATUS_SUCCESS);
}

TEST_CASE_ABORT(error_stack_get_ntstatus_wrong_type)
{
    error_set(E_TEST_ERROR_1);
    error_stack_get_ntstatus(0);
}

TEST_CASE(error_stack_check_ntstatus_oob)
{
    test_void(error_pass_ntstatus(foo, STATUS_TIMEOUT));
    test_false(error_stack_check_ntstatus(2, STATUS_TIMEOUT));
}

TEST_CASE_ABORT(error_stack_check_ntstatus_wrong_type)
{
    error_set(E_TEST_ERROR_1);
    error_stack_check_ntstatus(0, STATUS_TIMEOUT);
}

#endif // _WIN32

TEST_CASE(error_stack_get_type_oob)
{
    test_void(error_set(E_TEST_ERROR_1));
    test_uint_eq(error_stack_get_type(1), ERROR_TYPE_INVALID);
}

TEST_CASE(error_stack_get_func_oob)
{
    test_void(error_set(E_TEST_ERROR_1));
    test_ptr_eq(error_stack_get_func(1), NULL);
}

TEST_CASE(error_stack_get_name_oob)
{
    test_void(error_set(E_TEST_ERROR_1));
    test_ptr_eq(error_stack_get_name(1), NULL);
}

TEST_CASE(error_stack_get_desc_oob)
{
    test_void(error_set(E_TEST_ERROR_1));
    test_ptr_eq(error_stack_get_desc(1), NULL);
}

test_suite_ct test_suite_error(void)
{
    return test_suite_new_with_cases("error"
        , test_case_new(error_set)
        , test_case_new(error_push)
        , test_case_new(error_reset)
        , test_case_new(error_wrap)
        , test_case_new(error_pack_wrapper)
        , test_case_new(error_pack_error)
        , test_case_new(error_map_not_found)
        , test_case_new(error_map_single)
        , test_case_new(error_map_double)
        , test_case_new(error_map_wrapper)
        , test_case_new(error_map_sub)
        , test_case_new(error_pass_single)
        , test_case_new(error_pass_double)
        
        , test_case_new(errno_set)
        , test_case_new(errno_push)
        
        , test_case_new(error_push_errno)
        , test_case_new(error_wrap_errno)
        , test_case_new(error_pass_errno)
        
        , test_case_new_windows(error_push_win32)
        , test_case_new_windows(error_push_last_win32)
        , test_case_new_windows(error_wrap_win32)
        , test_case_new_windows(error_wrap_last_win32)
        , test_case_new_windows(error_pass_win32)
        , test_case_new_windows(error_pass_last_win32)
        
        , test_case_new_windows(error_push_hresult)
        , test_case_new_windows(error_wrap_hresult)
        , test_case_new_windows(error_pass_hresult)
        
        , test_case_new_windows(error_push_ntstatus)
        , test_case_new_windows(error_wrap_ntstatus)
        , test_case_new_windows(error_pass_ntstatus)
        
        , test_case_new(error_get_oob)
        , test_case_new(error_get_wrong_type)
        , test_case_new(error_check_oob)
        , test_case_new(error_check_wrong_type)
        
        , test_case_new(error_get_errno_oob)
        , test_case_new(error_get_errno_wrong_type)
        , test_case_new(error_check_errno_oob)
        , test_case_new(error_check_errno_wrong_type)
        
        , test_case_new_windows(error_get_win32_oob)
        , test_case_new_windows(error_get_win32_wrong_type)
        , test_case_new_windows(error_check_win32_oob)
        , test_case_new_windows(error_check_win32_wrong_type)
        
        , test_case_new_windows(error_get_hresult_oob)
        , test_case_new_windows(error_get_hresult_wrong_type)
        , test_case_new_windows(error_check_hresult_oob)
        , test_case_new_windows(error_check_hresult_wrong_type)
        
        , test_case_new_windows(error_get_ntstatus_oob)
        , test_case_new_windows(error_get_ntstatus_wrong_type)
        , test_case_new_windows(error_check_ntstatus_oob)
        , test_case_new_windows(error_check_ntstatus_wrong_type)
        
        , test_case_new(error_type_oob)
        , test_case_new(error_func_oob)
        , test_case_new(error_name_oob)
        , test_case_new(error_desc_oob)
        
        , test_case_new(error_stack_get_error_oob)
        , test_case_new(error_stack_get_error_wrong_type)
        , test_case_new(error_stack_check_error_oob)
        , test_case_new(error_stack_check_error_wrong_type)
        
        , test_case_new(error_stack_get_errno_oob)
        , test_case_new(error_stack_get_errno_wrong_type)
        , test_case_new(error_stack_check_errno_oob)
        , test_case_new(error_stack_check_errno_wrong_type)
        
        , test_case_new_windows(error_stack_get_win32_oob)
        , test_case_new_windows(error_stack_get_win32_wrong_type)
        , test_case_new_windows(error_stack_check_win32_oob)
        , test_case_new_windows(error_stack_check_win32_wrong_type)
        
        , test_case_new_windows(error_stack_get_hresult_oob)
        , test_case_new_windows(error_stack_get_hresult_wrong_type)
        , test_case_new_windows(error_stack_check_hresult_oob)
        , test_case_new_windows(error_stack_check_hresult_wrong_type)
        
        , test_case_new_windows(error_stack_get_ntstatus_oob)
        , test_case_new_windows(error_stack_get_ntstatus_wrong_type)
        , test_case_new_windows(error_stack_check_ntstatus_oob)
        , test_case_new_windows(error_stack_check_ntstatus_wrong_type)
        
        , test_case_new(error_stack_get_type_oob)
        , test_case_new(error_stack_get_func_oob)
        , test_case_new(error_stack_get_name_oob)
        , test_case_new(error_stack_get_desc_oob)
    );
}
