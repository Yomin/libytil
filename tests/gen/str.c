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

#include "str.h"
#include <ytil/test/test.h>
#include <ytil/gen/str.h>
#include <ytil/ext/string.h>
#include <ytil/ext/stdio.h>

static const struct not_a_str
{
    int foo;
} not_a_str = { 123 };

static const char *lit = "1234567890", *bin = "1" "\0" "23456789";
static char *cstr;
static void *data;
static str_ct str, str2;


TEST_SETUP(str_new_h)
{
    test_ptr_success(cstr = strdup(lit));
    test_ptr_success(str = str_new_h(cstr));
}

TEST_SETUP(str_new_c)
{
    test_ptr_success(str = str_new_c(lit));
}

TEST_SETUP(str_new_b)
{
    test_ptr_success(str = str_new_bc(bin, 10));
}

TEST_SETUP(str_new_empty)
{
    test_ptr_success(str = str_new_c(""));
}

TEST_TEARDOWN(str_unref)
{
    str_unref(str);
}

TEST_SETUP(cstr_new)
{
    test_ptr_success(cstr = strdup(lit));
}

TEST_TEARDOWN(cstr_free)
{
    test_free(cstr);
}

TEST_SETUP(data_new)
{
    test_ptr_success(data = memdup(bin, 10));
}

TEST_TEARDOWN(data_free)
{
    test_free(data);
}


TEST_CASE(str_new_h_invalid_cstr)
{
    test_ptr_error(str_new_h(NULL), E_STR_INVALID_CSTR);
}

TEST_CASE(str_new_h)
{
    size_t len = strlen(lit);
    test_ptr_success(cstr = strdup(lit));
    test_ptr_success(str = str_new_h(cstr));
    test_uint_eq(str_get_refs(str), 1);
    test_uint_eq(str_len(str), len);
    test_uint_eq(str_capacity(str), len);
    test_true(str_data_is_heap(str));
    test_ptr_eq(str_c(str), cstr);
    str_unref(str);
}

TEST_CASE(str_new_hn_invalid_cstr)
{
    test_ptr_error(str_new_hn(NULL, 0), E_STR_INVALID_CSTR);
}

TEST_CASE_FIXTURE(str_new_hn, cstr_new, str_unref)
{
    size_t len = strlen(lit);
    test_ptr_success(str = str_new_hn(cstr, len/2));
    test_uint_eq(str_get_refs(str), 1);
    test_uint_eq(str_len(str), len/2);
    test_uint_eq(str_capacity(str), len/2);
    test_true(str_data_is_heap(str));
    test_ptr_eq(str_c(str), cstr);
}

TEST_CASE(str_new_hnc_invalid_cstr)
{
    test_ptr_error(str_new_hnc(NULL, 0, 0), E_STR_INVALID_CSTR);
}

TEST_CASE_FIXTURE(str_new_hnc_len_gt_capacity, cstr_new, cstr_free)
{
    test_ptr_error(str_new_hnc(cstr, 10, 5), E_STR_INVALID_LENGTH);
}

TEST_CASE_FIXTURE(str_new_hnc, cstr_new, str_unref)
{
    size_t len = strlen(lit);
    test_ptr_success(str = str_new_hnc(cstr, len/2, len-1));
    test_uint_eq(str_get_refs(str), 1);
    test_uint_eq(str_len(str), len/2);
    test_uint_eq(str_capacity(str), len-1);
    test_true(str_data_is_heap(str));
    test_ptr_eq(str_c(str), cstr);
}

TEST_CASE(str_new_c_invalid_cstr)
{
    test_ptr_error(str_new_c(NULL), E_STR_INVALID_CSTR);
}

TEST_CASE_FIXTURE(str_new_c, NULL, str_unref)
{
    size_t len = strlen(lit);
    test_ptr_success(str = str_new_c(lit));
    test_uint_eq(str_get_refs(str), 1);
    test_ptr_eq(str_c(str), lit);
    test_uint_eq(str_len(str), len);
    test_uint_eq(str_capacity(str), 0);
    test_true(str_data_is_const(str));
    test_ptr_eq(str_c(str), lit);
}

TEST_CASE(str_new_cn_invalid_cstr)
{
    test_ptr_error(str_new_cn(NULL, 5), E_STR_INVALID_CSTR);
}

TEST_CASE(str_new_cn_missing_terminator)
{
    test_ptr_error(str_new_cn(lit, 5), E_STR_INVALID_CSTR);
}

TEST_CASE_FIXTURE(str_new_cn, NULL, str_unref)
{
    size_t len = strlen(lit);
    test_ptr_success(str = str_new_cn(lit, len));
    test_uint_eq(str_get_refs(str), 1);
    test_ptr_eq(str_c(str), lit);
    test_uint_eq(str_len(str), len);
    test_uint_eq(str_capacity(str), 0);
    test_true(str_data_is_const(str));
    test_ptr_eq(str_c(str), lit);
}

TEST_CASE(str_new_bh_invalid_data)
{
    test_ptr_error(str_new_bh(NULL, 10), E_STR_INVALID_DATA);
}

TEST_CASE_FIXTURE(str_new_bh, data_new, str_unref)
{
    test_ptr_success(str = str_new_bh(data, 10));
    test_uint_eq(str_get_refs(str), 1);
    test_uint_eq(str_len(str), 10);
    test_uint_eq(str_capacity(str), 10);
    test_true(str_data_is_heap(str));
    test_true(str_data_is_binary(str));
    test_ptr_eq(str_bc(str), data);
}

TEST_CASE(str_new_bhc_invalid_data)
{
    test_ptr_error(str_new_bhc(NULL, 5, 8), E_STR_INVALID_DATA);
}

TEST_CASE_FIXTURE(str_new_bhc_len_gt_capacity, data_new, data_free)
{
    test_ptr_error(str_new_bhc(data, 10, 5), E_STR_INVALID_LENGTH);
}

TEST_CASE_FIXTURE(str_new_bhc, data_new, str_unref)
{
    test_ptr_success(str = str_new_bhc(data, 5, 8));
    test_uint_eq(str_get_refs(str), 1);
    test_uint_eq(str_len(str), 5);
    test_uint_eq(str_capacity(str), 8);
    test_true(str_data_is_heap(str));
    test_true(str_data_is_binary(str));
    test_ptr_eq(str_bc(str), data);
}

TEST_CASE(str_new_bc_invalid_data)
{
    test_ptr_error(str_new_bc(NULL, 5), E_STR_INVALID_DATA);
}

TEST_CASE_FIXTURE(str_new_bc, NULL, str_unref)
{
    test_ptr_success(str = str_new_bc(bin, 8));
    test_uint_eq(str_get_refs(str), 1);
    test_ptr_eq(str_bc(str), bin);
    test_uint_eq(str_len(str), 8);
    test_uint_eq(str_capacity(str), 0);
    test_true(str_data_is_const(str));
    test_true(str_data_is_binary(str));
    test_ptr_eq(str_bc(str), bin);
}

TEST_CASE(tstr_new_h_invalid_cstr)
{
    test_ptr_error(tstr_new_h(NULL), E_STR_INVALID_CSTR);
}

TEST_CASE_FIXTURE(tstr_new_h, cstr_new, NULL)
{
    test_ptr_success(str = tstr_new_h(cstr));
    test_uint_eq(str_get_refs(str), 1);
    test_uint_eq(str_len(str), strlen(cstr));
    test_uint_eq(str_capacity(str), strlen(cstr));
    test_true(str_is_transient(str));
    test_true(str_data_is_heap(str));
    test_ptr_eq(str_c(str), cstr);
    str_unref(str);
}

TEST_CASE(tstr_new_hn_invalid_cstr)
{
    test_ptr_error(tstr_new_hn(NULL, 0), E_STR_INVALID_CSTR);
}

TEST_CASE_FIXTURE(tstr_new_hn, cstr_new, NULL)
{
    test_ptr_success(str = tstr_new_hn(cstr, 5));
    test_uint_eq(str_get_refs(str), 1);
    test_uint_eq(str_len(str), 5);
    test_uint_eq(str_capacity(str), 5);
    test_true(str_is_transient(str));
    test_true(str_data_is_heap(str));
    test_ptr_eq(str_c(str), cstr);
    str_unref(str);
}

TEST_CASE(tstr_new_hnc_invalid_cstr)
{
    test_ptr_error(tstr_new_hnc(NULL, 0, 0), E_STR_INVALID_CSTR);
}

TEST_CASE_FIXTURE(tstr_new_hnc_len_gt_capacity, cstr_new, cstr_free)
{
    test_ptr_error(tstr_new_hnc(cstr, 10, 5), E_STR_INVALID_LENGTH);
}

TEST_CASE_FIXTURE(tstr_new_hnc, cstr_new, NULL)
{
    test_ptr_success(str = tstr_new_hnc(cstr, 5, 8));
    test_uint_eq(str_get_refs(str), 1);
    test_uint_eq(str_len(str), 5);
    test_uint_eq(str_capacity(str), 8);
    test_true(str_is_transient(str));
    test_true(str_data_is_heap(str));
    test_ptr_eq(str_c(str), cstr);
    str_unref(str);
}

TEST_CASE(tstr_new_c_invalid_cstr)
{
    test_ptr_error(tstr_new_c(NULL), E_STR_INVALID_CSTR);
}

TEST_CASE(tstr_new_c)
{
    test_ptr_success(str = tstr_new_c(lit));
    test_uint_eq(str_get_refs(str), 0);
    test_uint_eq(str_len(str), strlen(lit));
    test_uint_eq(str_capacity(str), 0);
    test_true(str_is_transient(str));
    test_true(str_data_is_const(str));
    test_ptr_eq(str_c(str), lit);
    str_unref(str);
}

TEST_CASE(tstr_new_cn_invalid_cstr)
{
    test_ptr_error(tstr_new_cn(NULL, 0), E_STR_INVALID_CSTR);
}

TEST_CASE(tstr_new_cn_missing_terminator)
{
    test_ptr_error(tstr_new_cn(lit, 5), E_STR_INVALID_CSTR);
}

TEST_CASE(tstr_new_cn)
{
    size_t len = strlen(lit);
    test_ptr_success(str = tstr_new_cn(lit, len));
    test_uint_eq(str_get_refs(str), 0);
    test_uint_eq(str_len(str), len);
    test_uint_eq(str_capacity(str), 0);
    test_true(str_is_transient(str));
    test_true(str_data_is_const(str));
    test_ptr_eq(str_c(str), lit);
    str_unref(str);
}

TEST_CASE(tstr_new_bh_invalid_data)
{
    test_ptr_error(tstr_new_bh(NULL, 0), E_STR_INVALID_DATA);
}

TEST_CASE_FIXTURE(tstr_new_bh, data_new, NULL)
{
    test_ptr_success(str = tstr_new_bh(data, 5));
    test_uint_eq(str_get_refs(str), 1);
    test_uint_eq(str_len(str), 5);
    test_uint_eq(str_capacity(str), 5);
    test_true(str_is_transient(str));
    test_true(str_data_is_heap(str));
    test_true(str_data_is_binary(str));
    test_ptr_eq(str_bc(str), data);
    str_unref(str);
}

TEST_CASE(tstr_new_bhc_invalid_data)
{
    test_ptr_error(tstr_new_bhc(NULL, 0, 0), E_STR_INVALID_DATA);
}

TEST_CASE_FIXTURE(tstr_new_bhc_len_gt_capacity, data_new, data_free)
{
    test_ptr_error(tstr_new_bhc(data, 10, 5), E_STR_INVALID_LENGTH);
}

TEST_CASE_FIXTURE(tstr_new_bhc, data_new, NULL)
{
    test_ptr_success(str = tstr_new_bhc(data, 5, 8));
    test_uint_eq(str_get_refs(str), 1);
    test_uint_eq(str_len(str), 5);
    test_uint_eq(str_capacity(str), 8);
    test_true(str_is_transient(str));
    test_true(str_data_is_heap(str));
    test_true(str_data_is_binary(str));
    test_ptr_eq(str_bc(str), data);
    str_unref(str);
}

TEST_CASE(tstr_new_tn_invalid_cstr)
{
    test_ptr_error(tstr_new_tn(NULL, 0), E_STR_INVALID_CSTR);
}

TEST_CASE(tstr_new_tn)
{
    cstr = alloca(10);
    test_ptr_success(str = tstr_new_tn(cstr, 5));
    test_uint_eq(str_get_refs(str), 0);
    test_uint_eq(str_len(str), 5);
    test_uint_eq(str_capacity(str), 5);
    test_true(str_is_transient(str));
    test_true(str_data_is_transient(str));
    test_ptr_eq(str_c(str), cstr);
    str_unref(str);
}

TEST_CASE(tstr_new_tnc_invalid_cstr)
{
    test_ptr_error(tstr_new_tnc(NULL, 0, 0), E_STR_INVALID_CSTR);
}

TEST_CASE(tstr_new_tnc_len_gt_capacity)
{
    test_ptr_error(tstr_new_tnc(alloca(10), 10, 5), E_STR_INVALID_LENGTH);
}

TEST_CASE(tstr_new_tnc)
{
    cstr = alloca(10);
    test_ptr_success(str = tstr_new_tnc(cstr, 5, 8));
    test_uint_eq(str_get_refs(str), 0);
    test_uint_eq(str_len(str), 5);
    test_uint_eq(str_capacity(str), 8);
    test_true(str_is_transient(str));
    test_true(str_data_is_transient(str));
    test_ptr_eq(str_c(str), cstr);
    str_unref(str);
}

TEST_CASE(str_prepare)
{
    test_ptr_success(str = str_prepare(10));
    test_uint_eq(str_len(str), 10);
    test_uint_eq(str_capacity(str), 10);
    str_unref(str);
}

TEST_CASE(str_prepare_c_len_gt_capacity)
{
    test_ptr_error(str_prepare_c(10, 5), E_STR_INVALID_LENGTH);
}

TEST_CASE(str_prepare_c)
{
    test_ptr_success(str = str_prepare_c(5, 10));
    test_uint_eq(str_get_refs(str), 1);
    test_uint_eq(str_len(str), 5);
    test_uint_eq(str_capacity(str), 10);
    str_unref(str);
}

TEST_CASE(str_prepare_b)
{
    test_ptr_success(str = str_prepare_b(10));
    test_uint_eq(str_get_refs(str), 1);
    test_uint_eq(str_len(str), 10);
    test_uint_eq(str_capacity(str), 10);
    test_true(str_data_is_binary(str));
    str_unref(str);
}

TEST_CASE(str_prepare_bc_len_gt_capacity)
{
    test_ptr_error(str_prepare_bc(10, 5), E_STR_INVALID_LENGTH);
}

TEST_CASE(str_prepare_bc)
{
    test_ptr_success(str = str_prepare_bc(5, 10));
    test_uint_eq(str_get_refs(str), 1);
    test_uint_eq(str_len(str), 5);
    test_uint_eq(str_capacity(str), 10);
    test_true(str_data_is_binary(str));
    str_unref(str);
}

TEST_CASE(str_prepare_set)
{
    test_ptr_success(str = str_prepare_set(10, 42));
    test_uint_eq(str_get_refs(str), 1);
    test_uint_eq(str_len(str), 10);
    test_uint_eq(str_capacity(str), 10);
    test_int_eq(str_first(str), 42);
    test_int_eq(str_last(str), 42);
    str_unref(str);
}

TEST_CASE(str_prepare_set_c_len_gt_capacity)
{
    test_ptr_error(str_prepare_set_c(10, 5, 42), E_STR_INVALID_LENGTH);
}

TEST_CASE(str_prepare_set_c)
{
    test_ptr_success(str = str_prepare_set_c(5, 10, 42));
    test_uint_eq(str_get_refs(str), 1);
    test_uint_eq(str_len(str), 5);
    test_uint_eq(str_capacity(str), 10);
    test_int_eq(str_first(str), 42);
    test_int_eq(str_last(str), 42);
    str_unref(str);
}

TEST_CASE(str_prepare_set_b)
{
    test_ptr_success(str = str_prepare_set_b(10, '\x01'));
    test_uint_eq(str_get_refs(str), 1);
    test_uint_eq(str_len(str), 10);
    test_uint_eq(str_capacity(str), 10);
    test_true(str_data_is_binary(str));
    test_int_eq(str_first(str), '\x01');
    test_int_eq(str_last(str), '\x01');
    str_unref(str);
}

TEST_CASE(str_prepare_set_bc_len_gt_capacity)
{
    test_ptr_error(str_prepare_set_bc(10, 5, '\x01'), E_STR_INVALID_LENGTH);
}

TEST_CASE(str_prepare_set_bc)
{
    test_ptr_success(str = str_prepare_set_bc(5, 10, '\x01'));
    test_uint_eq(str_get_refs(str), 1);
    test_uint_eq(str_len(str), 5);
    test_uint_eq(str_capacity(str), 10);
    test_true(str_data_is_binary(str));
    test_int_eq(str_first(str), '\x01');
    test_int_eq(str_last(str), '\x01');
    str_unref(str);
}

test_suite_ct test_suite_str_add_new(test_suite_ct suite)
{
    return test_suite_add_cases(suite
        , test_case_new(str_new_h_invalid_cstr)
        , test_case_new(str_new_h)
        , test_case_new(str_new_hn_invalid_cstr)
        , test_case_new(str_new_hn)
        , test_case_new(str_new_hnc_invalid_cstr)
        , test_case_new(str_new_hnc_len_gt_capacity)
        , test_case_new(str_new_hnc)
        
        , test_case_new(str_new_c_invalid_cstr)
        , test_case_new(str_new_c)
        , test_case_new(str_new_cn_invalid_cstr)
        , test_case_new(str_new_cn_missing_terminator)
        , test_case_new(str_new_cn)
        
        , test_case_new(str_new_bh_invalid_data)
        , test_case_new(str_new_bh)
        , test_case_new(str_new_bhc_invalid_data)
        , test_case_new(str_new_bhc_len_gt_capacity)
        , test_case_new(str_new_bhc)
        , test_case_new(str_new_bc_invalid_data)
        , test_case_new(str_new_bc)
        
        , test_case_new(tstr_new_h_invalid_cstr)
        , test_case_new(tstr_new_h)
        , test_case_new(tstr_new_hn_invalid_cstr)
        , test_case_new(tstr_new_hn)
        , test_case_new(tstr_new_hnc_invalid_cstr)
        , test_case_new(tstr_new_hnc_len_gt_capacity)
        , test_case_new(tstr_new_hnc)
        
        , test_case_new(tstr_new_c_invalid_cstr)
        , test_case_new(tstr_new_c)
        , test_case_new(tstr_new_cn_invalid_cstr)
        , test_case_new(tstr_new_cn_missing_terminator)
        , test_case_new(tstr_new_cn)
        
        , test_case_new(tstr_new_bh_invalid_data)
        , test_case_new(tstr_new_bh)
        , test_case_new(tstr_new_bhc_invalid_data)
        , test_case_new(tstr_new_bhc_len_gt_capacity)
        , test_case_new(tstr_new_bhc)
        
        , test_case_new(tstr_new_tn_invalid_cstr)
        , test_case_new(tstr_new_tn)
        , test_case_new(tstr_new_tnc_invalid_cstr)
        , test_case_new(tstr_new_tnc_len_gt_capacity)
        , test_case_new(tstr_new_tnc)
        
        , test_case_new(str_prepare)
        , test_case_new(str_prepare_c_len_gt_capacity)
        , test_case_new(str_prepare_c)
        , test_case_new(str_prepare_b)
        , test_case_new(str_prepare_bc_len_gt_capacity)
        , test_case_new(str_prepare_bc)
        
        , test_case_new(str_prepare_set)
        , test_case_new(str_prepare_set_c_len_gt_capacity)
        , test_case_new(str_prepare_set_c)
        , test_case_new(str_prepare_set_b)
        , test_case_new(str_prepare_set_bc_len_gt_capacity)
        , test_case_new(str_prepare_set_bc)
    );
}

TEST_CASE_SIGNAL(str_resize_invalid_magic, SIGABRT)
{
    str_resize((str_ct)&not_a_str, 5);
}

TEST_CASE_FIXTURE(str_resize_heap_shrink, str_new_h, str_unref)
{
    size_t cap = str_capacity(str);
    test_ptr_success(str_resize(str, 5));
    test_uint_eq(str_len(str), 5);
    test_uint_eq(str_capacity(str), cap);
    test_int_eq(str_c(str)[5], '\0');
}

TEST_CASE_FIXTURE(str_resize_heap_grow, str_new_h, str_unref)
{
    size_t len = str_len(str)*2;
    test_ptr_success(str_resize(str, len));
    test_uint_eq(str_len(str), len);
    test_uint_ge(str_capacity(str), len);
    test_int_eq(str_c(str)[len], '\0');
}

TEST_CASE_FIXTURE(str_resize_const_shrink, str_new_c, str_unref)
{
    size_t len = str_len(str)/2;
    test_ptr_success(str_resize(str, len));
    test_uint_eq(str_len(str), len);
    test_uint_eq(str_capacity(str), len);
    test_true(str_data_is_heap(str));
    test_int_eq(str_c(str)[len], '\0');
}

TEST_CASE_FIXTURE(str_resize_const_grow, str_new_c, str_unref)
{
    size_t len = str_len(str)*2;
    test_ptr_success(str_resize(str, len));
    test_uint_eq(str_len(str), len);
    test_uint_eq(str_capacity(str), len);
    test_true(str_data_is_heap(str));
    test_int_eq(str_c(str)[len], '\0');
}

TEST_CASE_FIXTURE(str_resize_const_zero, str_new_c, str_unref)
{
    test_ptr_success(str_resize(str, 0));
    test_uint_eq(str_len(str), 0);
    test_uint_eq(str_capacity(str), 0);
    test_true(str_data_is_const(str));
    test_int_eq(str_c(str)[0], '\0');
}

TEST_CASE_FIXTURE(str_resize_transient_heap_shrink, cstr_new, NULL)
{
    size_t len = strlen(cstr);
    test_ptr_success(str = tstr_new_h(cstr));
    test_ptr_success(str_resize(str, len/2));
    test_uint_eq(str_len(str), len/2);
    test_uint_eq(str_capacity(str), len);
    test_true(str_data_is_heap(str));
    test_int_eq(str_c(str)[len/2], '\0');
    str_unref(str);
}

TEST_CASE_FIXTURE(str_resize_transient_heap_grow, cstr_new, NULL)
{
    size_t len = 2*strlen(cstr);
    test_ptr_success(str = tstr_new_h(cstr));
    test_ptr_success(str_resize(str, len));
    test_uint_eq(str_len(str), len);
    test_uint_ge(str_capacity(str), len);
    test_true(str_data_is_heap(str));
    test_int_eq(str_c(str)[len], '\0');
}

TEST_CASE(str_resize_transient_const_shrink)
{
    test_ptr_error(str_resize(LIT("123"), 1), E_STR_UNREFERENCED);
}

TEST_CASE(str_resize_transient_const_grow)
{
    test_ptr_error(str_resize(LIT("123"), 10), E_STR_UNREFERENCED);
}

TEST_CASE(str_resize_transient_transient_shrink)
{
    str = tstr_new_tn(FMT("%s", lit), strlen(lit));
    test_ptr_success(str_resize(str, 5));
    test_uint_eq(str_len(str), 5);
    test_uint_eq(str_capacity(str), 10);
    test_true(str_data_is_transient(str));
    test_int_eq(str_c(str)[5], '\0');
}

TEST_CASE(str_resize_transient_transient_grow)
{
    str = tstr_new_tn(FMT("%s", lit), strlen(lit));
    test_ptr_error(str_resize(str, 20), E_STR_UNREFERENCED);
}

TEST_CASE_SIGNAL(str_resize_set_invalid_magic, SIGABRT)
{
    str_resize_set((str_ct)&not_a_str, 0, 42);
}

TEST_CASE_FIXTURE(str_resize_set, str_new_h, str_unref)
{
    size_t len = str_len(str);
    test_ptr_success(str_resize_set(str, 2*len, 42));
    test_uint_eq(str_len(str), 2*len);
    test_uint_ge(str_capacity(str), 2*len);
    test_int_eq(str_at(str, len-1), lit[len-1]);
    test_int_eq(str_at(str, len), 42);
    test_int_eq(str_last(str), 42);
}

TEST_CASE_FIXTURE(str_clear_heap, str_new_h, str_unref)
{
    test_ptr_success(str_clear(str));
    test_uint_eq(str_len(str), 0);
    test_int_eq(str_c(str)[0], '\0');
}

TEST_CASE_FIXTURE(str_clear_const, str_new_c, str_unref)
{
    test_ptr_success(str_clear(str));
    test_uint_eq(str_len(str), 0);
    test_int_eq(str_c(str)[0], '\0');
}

TEST_CASE_FIXTURE(str_truncate, str_new_h, str_unref)
{
    test_uint_eq(str_capacity(str), strlen(lit));
    test_ptr_success(str_set_len(str, 5));
    test_uint_eq(str_capacity(str), strlen(lit));
    test_ptr_success(str_truncate(str));
    test_uint_eq(str_capacity(str), 5);
}

test_suite_ct test_suite_str_add_resize(test_suite_ct suite)
{
    return test_suite_add_cases(suite
        , test_case_new(str_resize_invalid_magic)
        , test_case_new(str_resize_heap_shrink)
        , test_case_new(str_resize_heap_grow)
        , test_case_new(str_resize_const_shrink)
        , test_case_new(str_resize_const_grow)
        , test_case_new(str_resize_const_zero)
        , test_case_new(str_resize_transient_heap_shrink)
        , test_case_new(str_resize_transient_heap_grow)
        , test_case_new(str_resize_transient_const_shrink)
        , test_case_new(str_resize_transient_const_grow)
        , test_case_new(str_resize_transient_transient_shrink)
        , test_case_new(str_resize_transient_transient_grow)
        , test_case_new(str_resize_set_invalid_magic)
        , test_case_new(str_resize_set)
        
        , test_case_new(str_clear_heap)
        , test_case_new(str_clear_const)
        , test_case_new(str_truncate)
    );
}

TEST_CASE_SIGNAL(str_dup_invalid_magic, SIGABRT)
{
    str_dup((str_ct)&not_a_str);
}

TEST_CASE_FIXTURE(str_dup_heap, str_new_h, str_unref)
{
    test_ptr_success(str2 = str_dup(str));
    test_uint_eq(str_get_refs(str2), 1);
    test_uint_eq(str_len(str), str_len(str2));
    test_ptr_ne(str_c(str), str_c(str2));
    test_true(!strcmp(str_c(str), str_c(str2)));
    str_unref(str2);
}

TEST_CASE_FIXTURE(str_dup_const, str_new_c, str_unref)
{
    test_ptr_success(str2 = str_dup(str));
    test_uint_eq(str_get_refs(str2), 1);
    test_uint_eq(str_len(str), str_len(str2));
    test_ptr_eq(str_c(str), str_c(str2));
    str_unref(str2);
}

TEST_CASE_SIGNAL(str_dup_n_invalid_magic, SIGABRT)
{
    str_dup_n((str_ct)&not_a_str, 5);
}

TEST_CASE_FIXTURE(str_dup_n_heap, str_new_h, str_unref)
{
    test_ptr_success(str2 = str_dup_n(str, 5));
    test_uint_eq(str_get_refs(str2), 1);
    test_uint_eq(str_len(str2), 5);
    test_ptr_ne(str_c(str), str_c(str2));
    test_true(!strncmp(str_c(str), str_c(str2), 5));
    str_unref(str2);
}

TEST_CASE_FIXTURE(str_dup_n_const_shrink, str_new_c, str_unref)
{
    test_ptr_success(str2 = str_dup_n(str, 5));
    test_uint_eq(str_get_refs(str2), 1);
    test_uint_eq(str_len(str2), 5);
    test_true(str_data_is_heap(str2));
    test_ptr_ne(str_c(str), str_c(str2));
    str_unref(str2);
}

TEST_CASE_FIXTURE(str_dup_n_const, str_new_c, str_unref)
{
    test_ptr_success(str2 = str_dup_n(str, 10));
    test_uint_eq(str_get_refs(str2), 1);
    test_uint_eq(str_len(str2), 10);
    test_true(str_data_is_const(str2));
    test_ptr_eq(str_c(str), str_c(str2));
    str_unref(str2);
}

TEST_CASE(str_dup_c_invalid_cstr)
{
    test_ptr_error(str_dup_c(NULL), E_STR_INVALID_CSTR);
}

TEST_CASE_FIXTURE(str_dup_c, cstr_new, cstr_free)
{
    test_ptr_success(str = str_dup_c(cstr));
    test_uint_eq(str_get_refs(str), 1);
    test_uint_eq(str_len(str), strlen(cstr));
    test_true(str_data_is_heap(str));
    test_ptr_ne(str_c(str), cstr);
    test_true(!strcmp(str_c(str), cstr));
    str_unref(str);
}

TEST_CASE(str_dup_cn_invalid_cstr)
{
    test_ptr_error(str_dup_cn(NULL, 5), E_STR_INVALID_CSTR);
}

TEST_CASE_FIXTURE(str_dup_cn, cstr_new, cstr_free)
{
    test_ptr_success(str = str_dup_cn(cstr, 5));
    test_uint_eq(str_get_refs(str), 1);
    test_uint_eq(str_len(str), 5);
    test_true(str_data_is_heap(str));
    test_ptr_ne(str_c(str), cstr);
    test_true(!strncmp(str_c(str), cstr, 5));
    str_unref(str);
}

TEST_CASE(str_dup_b_invalid_data)
{
    test_ptr_error(str_dup_b(NULL, 8), E_STR_INVALID_DATA);
}

TEST_CASE_FIXTURE(str_dup_b, data_new, data_free)
{
    test_ptr_success(str = str_dup_b(data, 8));
    test_uint_eq(str_get_refs(str), 1);
    test_uint_eq(str_len(str), 8);
    test_true(str_data_is_heap(str));
    test_true(str_data_is_binary(str));
    test_ptr_ne(str_bc(str), data);
    test_true(!memcmp(str_bc(str), data, 8));
    str_unref(str);
}

TEST_CASE(str_dup_f_invalid_format)
{
    test_ptr_error(str_dup_f(NULL), E_STR_INVALID_FORMAT);
}

TEST_CASE(str_dup_f)
{
    test_ptr_success(str = str_dup_f("123%d890", 4567));
    test_uint_eq(str_get_refs(str), 1);
    test_uint_eq(str_len(str), 10);
    test_true(str_data_is_heap(str));
    test_true(!strcmp(str_c(str), lit));
    str_unref(str);
}

test_suite_ct test_suite_str_add_dup(test_suite_ct suite)
{
    return test_suite_add_cases(suite
        , test_case_new(str_dup_invalid_magic)
        , test_case_new(str_dup_heap)
        , test_case_new(str_dup_const)
        , test_case_new(str_dup_n_invalid_magic)
        , test_case_new(str_dup_n_heap)
        , test_case_new(str_dup_n_const_shrink)
        , test_case_new(str_dup_n_const)
        , test_case_new(str_dup_c_invalid_cstr)
        , test_case_new(str_dup_c)
        , test_case_new(str_dup_cn_invalid_cstr)
        , test_case_new(str_dup_cn)
        , test_case_new(str_dup_b_invalid_data)
        , test_case_new(str_dup_b)
        , test_case_new(str_dup_f_invalid_format)
        , test_case_new(str_dup_f)
    );
}

TEST_CASE_SIGNAL(str_set_h_invalid_magic, SIGABRT)
{
    str_set_h((str_ct)&not_a_str, NULL);
}

TEST_CASE_FIXTURE(str_set_h_invalid_cstr, str_new_b, str_unref)
{
    test_ptr_error(str_set_h(str, NULL), E_STR_INVALID_CSTR);
}

TEST_CASE_FIXTURE(str_set_h_unreferenced, cstr_new, cstr_free)
{
    test_ptr_error(str_set_h(LIT("123"), cstr), E_STR_UNREFERENCED);
}

TEST_CASE_FIXTURE(str_set_h, str_new_b, str_unref)
{
    test_ptr_success(cstr = strdup(lit));
    test_ptr_success(str_set_h(str, cstr));
    test_uint_eq(str_len(str), strlen(cstr));
    test_uint_eq(str_capacity(str), strlen(cstr));
    test_true(str_data_is_heap(str));
    test_false(str_data_is_binary(str));
    test_ptr_eq(str_c(str), cstr);
}

TEST_CASE_SIGNAL(str_set_hn_invalid_magic, SIGABRT)
{
    str_set_hn((str_ct)&not_a_str, NULL, 5);
}

TEST_CASE_FIXTURE(str_set_hn_invalid_cstr, str_new_b, str_unref)
{
    test_ptr_error(str_set_hn(str, NULL, 5), E_STR_INVALID_CSTR);
}

TEST_CASE_FIXTURE(str_set_hn_unreferenced, cstr_new, cstr_free)
{
    test_ptr_error(str_set_hn(LIT("123"), cstr, 5), E_STR_UNREFERENCED);
}

TEST_CASE_FIXTURE(str_set_hn, str_new_b, str_unref)
{
    test_ptr_success(cstr = strdup(lit));
    test_ptr_success(str_set_hn(str, cstr, 5));
    test_uint_eq(str_len(str), 5);
    test_uint_eq(str_capacity(str), 5);
    test_true(str_data_is_heap(str));
    test_false(str_data_is_binary(str));
    test_ptr_eq(str_c(str), cstr);
}

TEST_CASE_SIGNAL(str_set_hnc_invalid_magic, SIGABRT)
{
    str_set_hnc((str_ct)&not_a_str, NULL, 5, 8);
}

TEST_CASE_FIXTURE(str_set_hnc_invalid_cstr, str_new_b, str_unref)
{
    test_ptr_error(str_set_hnc(str, NULL, 5, 8), E_STR_INVALID_CSTR);
}

TEST_CASE_FIXTURE(str_set_hnc_unreferenced, cstr_new, cstr_free)
{
    test_ptr_error(str_set_hnc(LIT("123"), cstr, 5, 8), E_STR_UNREFERENCED);
}

TEST_CASE_FIXTURE(str_set_hnc, str_new_b, str_unref)
{
    test_ptr_success(cstr = strdup(lit));
    test_ptr_success(str_set_hnc(str, cstr, 5, 8));
    test_uint_eq(str_len(str), 5);
    test_uint_eq(str_capacity(str), 8);
    test_true(str_data_is_heap(str));
    test_false(str_data_is_binary(str));
    test_ptr_eq(str_c(str), cstr);
}

TEST_CASE_SIGNAL(str_set_c_invalid_magic, SIGABRT)
{
    str_set_c((str_ct)&not_a_str, NULL);
}

TEST_CASE_FIXTURE(str_set_c_invalid_cstr, str_new_b, str_unref)
{
    test_ptr_error(str_set_c(str, NULL), E_STR_INVALID_CSTR);
}

TEST_CASE_FIXTURE(str_set_c, str_new_b, str_unref)
{
    test_ptr_success(str_set_c(str, lit));
    test_uint_eq(str_len(str), strlen(lit));
    test_uint_eq(str_capacity(str), 0);
    test_true(str_data_is_const(str));
    test_false(str_data_is_binary(str));
    test_ptr_eq(str_c(str), lit);
}

TEST_CASE_SIGNAL(str_set_cn_invalid_magic, SIGABRT)
{
    str_set_cn((str_ct)&not_a_str, NULL, 5);
}

TEST_CASE_FIXTURE(str_set_cn_invalid_cstr, str_new_b, str_unref)
{
    test_ptr_error(str_set_cn(str, NULL, 5), E_STR_INVALID_CSTR);
}

TEST_CASE_FIXTURE(str_set_cn_missing_terminator, str_new_b, str_unref)
{
    test_ptr_error(str_set_cn(str, lit, 5), E_STR_INVALID_CSTR);
}

TEST_CASE_FIXTURE(str_set_cn, str_new_c, str_unref)
{
    test_ptr_success(str_set_cn(str, lit, 10));
    test_uint_eq(str_len(str), 10);
    test_uint_eq(str_capacity(str), 0);
    test_true(str_data_is_const(str));
    test_false(str_data_is_binary(str));
    test_ptr_eq(str_c(str), lit);
}

TEST_CASE_SIGNAL(str_set_bh_invalid_magic, SIGABRT)
{
    str_set_bh((str_ct)&not_a_str, NULL, 5);
}

TEST_CASE_FIXTURE(str_set_bh_invalid_data, str_new_c, str_unref)
{
    test_ptr_error(str_set_bh(str, NULL, 5), E_STR_INVALID_DATA);
}

TEST_CASE_FIXTURE(str_set_bh_unreferenced, data_new, data_free)
{
    test_ptr_error(str_set_bh(LIT("123"), data, 5), E_STR_UNREFERENCED);
}

TEST_CASE_FIXTURE(str_set_bh, str_new_c, str_unref)
{
    test_ptr_success(data = memdup(bin, 10));
    test_ptr_success(str_set_bh(str, data, 5));
    test_uint_eq(str_len(str), 5);
    test_uint_eq(str_capacity(str), 5);
    test_true(str_data_is_heap(str));
    test_true(str_data_is_binary(str));
    test_ptr_eq(str_bc(str), data);
}

TEST_CASE_SIGNAL(str_set_bhc_invalid_magic, SIGABRT)
{
    str_set_bhc((str_ct)&not_a_str, NULL, 5, 8);
}

TEST_CASE_FIXTURE(str_set_bhc_invalid_data, str_new_c, str_unref)
{
    test_ptr_error(str_set_bhc(str, NULL, 5, 8), E_STR_INVALID_DATA);
}

TEST_CASE_FIXTURE(str_set_bhc_unreferenced, data_new, data_free)
{
    test_ptr_error(str_set_bhc(LIT("123"), data, 5, 8), E_STR_UNREFERENCED);
}

TEST_CASE_FIXTURE(str_set_bhc, str_new_c, str_unref)
{
    test_ptr_success(data = memdup(bin, 10));
    test_ptr_success(str_set_bhc(str, data, 5, 8));
    test_uint_eq(str_len(str), 5);
    test_uint_eq(str_capacity(str), 8);
    test_true(str_data_is_heap(str));
    test_true(str_data_is_binary(str));
    test_ptr_eq(str_bc(str), data);
}

TEST_CASE_SIGNAL(str_set_bc_invalid_magic, SIGABRT)
{
    str_set_bc((str_ct)&not_a_str, NULL, 5);
}

TEST_CASE_FIXTURE(str_set_bc_invalid_data, str_new_c, str_unref)
{
    test_ptr_error(str_set_bc(str, NULL, 5), E_STR_INVALID_DATA);
}

TEST_CASE_FIXTURE(str_set_bc, str_new_c, str_unref)
{
    test_ptr_success(str_set_bc(str, bin, 5));
    test_uint_eq(str_len(str), 5);
    test_uint_eq(str_capacity(str), 0);
    test_true(str_data_is_const(str));
    test_true(str_data_is_binary(str));
    test_ptr_eq(str_bc(str), bin);
}

test_suite_ct test_suite_str_add_set(test_suite_ct suite)
{
    return test_suite_add_cases(suite
        , test_case_new(str_set_h_invalid_magic)
        , test_case_new(str_set_h_invalid_cstr)
        , test_case_new(str_set_h_unreferenced)
        , test_case_new(str_set_h)
        , test_case_new(str_set_hn_invalid_magic)
        , test_case_new(str_set_hn_invalid_cstr)
        , test_case_new(str_set_hn_unreferenced)
        , test_case_new(str_set_hn)
        , test_case_new(str_set_hnc_invalid_magic)
        , test_case_new(str_set_hnc_invalid_cstr)
        , test_case_new(str_set_hnc_unreferenced)
        , test_case_new(str_set_hnc)
        
        , test_case_new(str_set_c_invalid_magic)
        , test_case_new(str_set_c_invalid_cstr)
        , test_case_new(str_set_c)
        , test_case_new(str_set_cn_invalid_magic)
        , test_case_new(str_set_cn_invalid_cstr)
        , test_case_new(str_set_cn_missing_terminator)
        , test_case_new(str_set_cn)
        
        , test_case_new(str_set_bh_invalid_magic)
        , test_case_new(str_set_bh_invalid_data)
        , test_case_new(str_set_bh_unreferenced)
        , test_case_new(str_set_bh)
        , test_case_new(str_set_bhc_invalid_magic)
        , test_case_new(str_set_bhc_invalid_data)
        , test_case_new(str_set_bhc_unreferenced)
        , test_case_new(str_set_bhc)
        , test_case_new(str_set_bc_invalid_magic)
        , test_case_new(str_set_bc_invalid_data)
        , test_case_new(str_set_bc)
    );
}

TEST_CASE_SIGNAL(str_c_invalid_magic, SIGABRT)
{
    str_c((str_ct)&not_a_str);
}

TEST_CASE_FIXTURE(str_c_binary, str_new_b, str_unref)
{
    test_ptr_error(str_c(str), E_STR_BINARY);
}

TEST_CASE_FIXTURE(str_c, str_new_c, str_unref)
{
    const char *cstr;
    test_ptr_success(cstr = str_c(str));
    test_ptr_eq(cstr, lit);
}

TEST_CASE_SIGNAL(str_uc_invalid_magic, SIGABRT)
{
    str_uc((str_ct)&not_a_str);
}

TEST_CASE_FIXTURE(str_uc_binary, str_new_b, str_unref)
{
    test_ptr_error(str_uc(str), E_STR_BINARY);
}

TEST_CASE_FIXTURE(str_uc, str_new_c, str_unref)
{
    const unsigned char *cstr;
    test_ptr_success(cstr = str_uc(str));
    test_ptr_eq(cstr, lit);
}

TEST_CASE_SIGNAL(str_bc_invalid_magic, SIGABRT)
{
    str_bc((str_ct)&not_a_str);
}

TEST_CASE_FIXTURE(str_bc, str_new_b, str_unref)
{
    const void *data;
    test_ptr_success(data = str_bc(str));
    test_ptr_eq(data, bin);
}

TEST_CASE_SIGNAL(str_buc_invalid_magic, SIGABRT)
{
    str_buc((str_ct)&not_a_str);
}

TEST_CASE_FIXTURE(str_buc, str_new_b, str_unref)
{
    const unsigned char *data;
    test_ptr_success(data = str_buc(str));
    test_ptr_eq(data, bin);
}

TEST_CASE_SIGNAL(str_w_invalid_magic, SIGABRT)
{
    str_w((str_ct)&not_a_str);
}

TEST_CASE_FIXTURE(str_w_binary, str_new_b, str_unref)
{
    test_ptr_error(str_w(str), E_STR_BINARY);
}

TEST_CASE(str_w_unreferenced)
{
    test_ptr_error(str_w(LIT("123")), E_STR_UNREFERENCED);
}

TEST_CASE_FIXTURE(str_w, str_new_c, str_unref)
{
    test_ptr_success(cstr = str_w(str));
    test_true(str_data_is_heap(str));
    test_ptr_ne(cstr, lit);
    test_true(!strcmp(cstr, lit));
}

TEST_CASE_SIGNAL(str_uw_invalid_magic, SIGABRT)
{
    str_uw((str_ct)&not_a_str);
}

TEST_CASE_FIXTURE(str_uw_binary, str_new_b, str_unref)
{
    test_ptr_error(str_uw(str), E_STR_BINARY);
}

TEST_CASE(str_uw_unreferenced)
{
    test_ptr_error(str_uw(LIT("123")), E_STR_UNREFERENCED);
}

TEST_CASE_FIXTURE(str_uw, str_new_c, str_unref)
{
    unsigned char *cstr;
    test_ptr_success(cstr = str_uw(str));
    test_true(str_data_is_heap(str));
    test_ptr_ne(cstr, lit);
    test_true(!strcmp((char*)cstr, lit));
}

TEST_CASE_SIGNAL(str_bw_invalid_magic, SIGABRT)
{
    str_bw((str_ct)&not_a_str);
}

TEST_CASE(str_bw_unreferenced)
{
    test_ptr_error(str_bw(LIT("123")), E_STR_UNREFERENCED);
}

TEST_CASE_FIXTURE(str_bw, str_new_b, str_unref)
{
    test_ptr_success(data = str_bw(str));
    test_true(str_data_is_heap(str));
    test_ptr_ne(data, bin);
    test_true(!memcmp(data, bin, 10));
}

TEST_CASE_SIGNAL(str_buw_invalid_magic, SIGABRT)
{
    str_buw((str_ct)&not_a_str);
}

TEST_CASE(str_buw_unreferenced)
{
    test_ptr_error(str_buw(LIT("123")), E_STR_UNREFERENCED);
}

TEST_CASE_FIXTURE(str_buw, str_new_b, str_unref)
{
    test_ptr_success(data = str_buw(str));
    test_true(str_data_is_heap(str));
    test_ptr_ne(data, bin);
    test_true(!memcmp(data, bin, 10));
}

TEST_CASE_FIXTURE(str_update, str_new_c, str_unref)
{
    size_t len = strlen(lit);
    test_ptr_success(cstr = str_w(str));
    test_uint_eq(str_len(str), len);
    cstr[len/2] = '\0';
    test_ptr_success(str_update(str));
    test_uint_eq(str_len(str), len/2);
    test_uint_eq(str_capacity(str), len);
}

TEST_CASE_FIXTURE(str_set_len_len_gt_capacity, str_new_c, str_unref)
{
    test_ptr_success(str_w(str));
    test_ptr_error(str_set_len(str, 2*strlen(lit)), E_STR_INVALID_LENGTH);
}

TEST_CASE_FIXTURE(str_set_len_const, str_new_c, str_unref)
{
    test_ptr_error(str_set_len(str, strlen(lit)/2), E_STR_INVALID_LENGTH);
}

TEST_CASE_FIXTURE(str_set_len, str_new_c, str_unref)
{
    size_t len = strlen(lit);
    test_ptr_success(cstr = str_w(str));
    test_uint_eq(str_len(str), len);
    test_ptr_success(str_set_len(str, len/2));
    test_uint_eq(str_len(str), len/2);
    test_uint_eq(str_capacity(str), len);
    test_int_eq(str_c(str)[len/2], '\0');
}

TEST_CASE_FIXTURE(str_first_empty, str_new_empty, str_unref)
{
    test_rc_error(str_first(str), '\0', E_STR_EMPTY);
}

TEST_CASE_FIXTURE(str_first, str_new_c, str_unref)
{
    char c = str_first(str);
    test_int_eq(c, lit[0]);
}

TEST_CASE_FIXTURE(str_last_empty, str_new_empty, str_unref)
{
    test_rc_error(str_last(str), '\0', E_STR_EMPTY);
}

TEST_CASE_FIXTURE(str_last, str_new_c, str_unref)
{
    char c = str_last(str);
    test_int_eq(c, lit[strlen(lit)-1]);
}

TEST_CASE_FIXTURE(str_at_oob, str_new_c, str_unref)
{
    test_rc_error(str_at(str, strlen(lit)), '\0', E_STR_OUT_OF_BOUNDS);
}

TEST_CASE_FIXTURE(str_at, str_new_c, str_unref)
{
    size_t len = strlen(lit);
    char c = str_at(str, len/2);
    test_int_eq(c, lit[len/2]);
}

TEST_CASE_FIXTURE(str_at_u_oob, str_new_c, str_unref)
{
    test_rc_error(str_at_u(str, strlen(lit)), '\0', E_STR_OUT_OF_BOUNDS);
}

TEST_CASE_FIXTURE(str_at_u, str_new_c, str_unref)
{
    size_t len = strlen(lit);
    unsigned char c = str_at_u(str, len/2);
    test_uint_eq(c, lit[len/2]);
}

test_suite_ct test_suite_str_add_get(test_suite_ct suite)
{
    return test_suite_add_cases(suite
        , test_case_new(str_c_invalid_magic)
        , test_case_new(str_c_binary)
        , test_case_new(str_c)
        , test_case_new(str_uc_invalid_magic)
        , test_case_new(str_uc_binary)
        , test_case_new(str_uc)
        , test_case_new(str_bc_invalid_magic)
        , test_case_new(str_bc)
        , test_case_new(str_buc_invalid_magic)
        , test_case_new(str_buc)
        
        , test_case_new(str_w_invalid_magic)
        , test_case_new(str_w_binary)
        , test_case_new(str_w_unreferenced)
        , test_case_new(str_w)
        , test_case_new(str_uw_invalid_magic)
        , test_case_new(str_uw_binary)
        , test_case_new(str_uw_unreferenced)
        , test_case_new(str_uw)
        , test_case_new(str_bw_invalid_magic)
        , test_case_new(str_bw_unreferenced)
        , test_case_new(str_bw)
        , test_case_new(str_buw_invalid_magic)
        , test_case_new(str_buw_unreferenced)
        , test_case_new(str_buw)
        
        , test_case_new(str_update)
        , test_case_new(str_set_len_len_gt_capacity)
        , test_case_new(str_set_len_const)
        , test_case_new(str_set_len)
        
        , test_case_new(str_first_empty)
        , test_case_new(str_first)
        , test_case_new(str_last_empty)
        , test_case_new(str_last)
        , test_case_new(str_at_oob)
        , test_case_new(str_at)
        , test_case_new(str_at_u_oob)
        , test_case_new(str_at_u)
    );
}

TEST_CASE_FIXTURE(str_ref, str_new_h, str_unref)
{
    str_ct nstr;
    test_uint_eq(str_get_refs(str), 1);
    test_ptr_success(nstr = str_ref(str));
    test_ptr_eq(str, nstr);
    test_uint_eq(str_get_refs(str), 2);
    test_ptr_success(str_unref(str));
    test_uint_eq(str_get_refs(str), 1);
}

TEST_CASE(str_ref_transient_const)
{
    str_ct nstr;
    test_ptr_success(str = LIT("123"));
    test_uint_eq(str_get_refs(str), 0);
    test_ptr_success(nstr = str_ref(str));
    test_uint_eq(str_get_refs(str), 0);
    test_uint_eq(str_get_refs(nstr), 1);
    test_false(str_is_transient(nstr));
    test_true(str_data_is_const(nstr));
    test_uint_eq(str_len(str), str_len(nstr));
    test_ptr_eq(str_c(str), str_c(nstr));
    str_unref(nstr);
}

TEST_CASE(str_ref_transient_transient)
{
    str_ct nstr;
    test_ptr_success(str = tstr_new_tn(FMT("%s", lit), strlen(lit)));
    test_uint_eq(str_get_refs(str), 0);
    test_ptr_success(nstr = str_ref(str));
    test_uint_eq(str_get_refs(str), 0);
    test_uint_eq(str_get_refs(nstr), 1);
    test_false(str_is_transient(nstr));
    test_true(str_data_is_heap(nstr));
    test_uint_eq(str_len(str), str_len(nstr));
    test_ptr_ne(str_c(str), str_c(nstr));
    test_true(!strcmp(str_c(str), str_c(nstr)));
    str_unref(nstr);
}

TEST_CASE_FIXTURE(str_ref_transient_heap, cstr_new, NULL)
{
    str_ct nstr;
    test_ptr_success(str = tstr_new_h(cstr));
    test_uint_eq(str_get_refs(str), 1);
    test_ptr_success(nstr = str_ref(str));
    test_uint_eq(str_get_refs(str), 2);
    test_uint_eq(str_get_refs(nstr), 2);
    test_true(str_is_redirected(str));
    test_false(str_is_transient(nstr));
    test_uint_eq(str_len(str), str_len(nstr));
    test_ptr_eq(str_c(str), str_c(nstr));
    str_unref(nstr);
    test_uint_eq(str_get_refs(str), 1);
    str_unref(str);
}

test_suite_ct test_suite_str_add_ref(test_suite_ct suite)
{
    return test_suite_add_cases(suite
        , test_case_new(str_ref)
        , test_case_new(str_ref_transient_const)
        , test_case_new(str_ref_transient_transient)
        , test_case_new(str_ref_transient_heap)
    );
}

test_suite_ct test_suite_str(void)
{
    test_suite_ct suite;
    
    if(!(suite = test_suite_new("str"))
    || !test_suite_str_add_new(suite)
    || !test_suite_str_add_resize(suite)
    || !test_suite_str_add_dup(suite)
    || !test_suite_str_add_set(suite)
    || !test_suite_str_add_get(suite)
    || !test_suite_str_add_ref(suite))
        return NULL;
    
    return suite;
}
