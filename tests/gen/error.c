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
    
    test_uint_eq(error_type(0), ERROR_TYPE_WRAPPER);
    test_int_eq(error_get(0), E_ERROR_WRAPPER);
    
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
    
    test_uint_eq(error_type(0), ERROR_TYPE_WRAPPER);
    test_int_eq(error_get(0), E_ERROR_WRAPPER);
    test_uint_eq(error_type(1), ERROR_TYPE_WRAPPER);
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
    
    test_uint_eq(error_type(0), ERROR_TYPE_WRAPPER);
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
    test_uint_eq(error_stack_get_type(1), ERROR_TYPE_PASS);
    
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
    test_uint_eq(error_stack_get_type(1), ERROR_TYPE_PASS);
    test_uint_eq(error_stack_get_type(2), ERROR_TYPE_ERROR);
    test_uint_eq(error_stack_get_type(3), ERROR_TYPE_PASS);
    
    test_int_eq(error_get(0), E_TEST_ERROR_2);
    test_int_eq(error_get(1), E_TEST_ERROR_1);
    test_int_eq(error_get(2), E_ERROR_UNSET);
}

TEST_CASE(errno_set)
{
    test_void(errno_set(EINVAL));
    
    test_uint_eq(error_depth(), 1);
    test_uint_eq(error_type(0), ERROR_TYPE_ERRNO);
    test_int_eq(error_get(0), EINVAL);
    
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
    
    test_uint_eq(error_type(0), ERROR_TYPE_ERRNO);
    test_int_eq(error_get(0), ENOENT);
    
    test_uint_eq(error_type(1), ERROR_TYPE_ERRNO);
    test_int_eq(error_get(1), EINVAL);
    
    test_int_eq(error_get(2), E_ERROR_UNSET);
}

TEST_CASE(error_push_errno)
{
    errno = EFAULT;
    test_void(error_push_errno(E_TEST_ERROR_1, foo));
    
    test_uint_eq(error_depth(), 2);
    
    test_uint_eq(error_type(0), ERROR_TYPE_ERROR);
    test_int_eq(error_get(0), E_TEST_ERROR_1);
    
    test_uint_eq(error_type(1), ERROR_TYPE_ERRNO);
    test_int_eq(error_get(1), EFAULT);
    test_str_eq(error_func(1), "foo");
    
    test_int_eq(error_get(2), E_ERROR_UNSET);
}

TEST_CASE(error_wrap_errno)
{
    errno = EFAULT;
    test_void(error_wrap_errno(foo));
    
    test_uint_eq(error_depth(), 2);
    
    test_uint_eq(error_type(0), ERROR_TYPE_WRAPPER);
    test_int_eq(error_get(0), E_ERROR_WRAPPER);
    
    test_uint_eq(error_type(1), ERROR_TYPE_ERRNO);
    test_int_eq(error_get(1), EFAULT);
    
    test_int_eq(error_get(2), E_ERROR_UNSET);
}

TEST_CASE(error_pass_errno)
{
    errno = EFAULT;
    test_void(error_pass_errno(foo));
    
    test_uint_eq(error_depth(), 2);
    
    test_uint_eq(error_stack_get_type(0), ERROR_TYPE_ERRNO);
    test_uint_eq(error_stack_get_type(1), ERROR_TYPE_PASS);
    
    test_int_eq(error_get(0), EFAULT);
    test_int_eq(error_get(1), E_ERROR_UNSET);
}

TEST_CASE(error_get_oob)
{
    test_void(error_set(E_TEST_ERROR_1));
    test_int_eq(error_get(1), E_ERROR_UNSET);
}

TEST_CASE(error_check_oob)
{
    test_void(error_set(E_TEST_ERROR_1));
    test_false(error_check(1, E_TEST_ERROR_1));
}

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

TEST_CASE(error_stack_check_error_oob)
{
    test_void(error_set(E_TEST_ERROR_1));
    test_false(error_stack_check_error(1, E_TEST_ERROR_1));
}

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
        
        , test_case_new(error_get_oob)
        , test_case_new(error_check_oob)
        , test_case_new(error_type_oob)
        , test_case_new(error_func_oob)
        , test_case_new(error_name_oob)
        , test_case_new(error_desc_oob)
        
        , test_case_new(error_stack_get_error_oob)
        , test_case_new(error_stack_check_error_oob)
        , test_case_new(error_stack_get_type_oob)
        , test_case_new(error_stack_get_func_oob)
        , test_case_new(error_stack_get_name_oob)
        , test_case_new(error_stack_get_desc_oob)
    );
}
