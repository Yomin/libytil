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

#include "art.h"
#include <ytil/test/test.h>
#include <ytil/con/art.h>

static const struct not_an_art
{
    int foo;
} not_an_art = { 123 };

art_ct art;
art_node_ct node;
str_ct key;
size_t count;


TEST_SETUP(art_new_empty)
{
    test_ptr_success(art = art_new(ART_MODE_ORDERED));
}

TEST_SETUP(art_new1)
{
    test_ptr_success(art = art_new(ART_MODE_ORDERED));
    test_ptr_success(art_insert(art, LIT("foobar"), NULL));
}

TEST_SETUP(art_new4)
{
    test_ptr_success(art = art_new(ART_MODE_ORDERED));
    test_ptr_success(art_insert_value(art, LIT("foobar"), 1));
    test_ptr_success(art_insert_value(art, LIT("foobaz"), 2));
    test_ptr_success(art_insert_value(art, LIT("fooduh"), 3));
    test_ptr_success(art_insert_value(art, LIT("xyz"), 4));
}

TEST_TEARDOWN(art_free)
{
    art_free(art);
}

TEST_CASE_SIGNAL(art_is_empty_invalid_magic, SIGABRT)
{
    art_is_empty((art_ct)&not_an_art);
}

TEST_CASE_FIXTURE(art_is_empty, art_new_empty, art_free)
{
    test_true(art_is_empty(art));
    test_ptr_success(node = art_insert(art, LIT("foo"), NULL));
    test_false(art_is_empty(art));
    test_void(art_remove(art, node));
    test_true(art_is_empty(art));
}

TEST_CASE_SIGNAL(art_size_invalid_magic, SIGABRT)
{
    art_size((art_ct)&not_an_art);
}

TEST_CASE_FIXTURE(art_size, art_new_empty, art_free)
{
    test_uint_eq(art_size(art), 0);
    test_ptr_success(node = art_insert(art, LIT("foo"), NULL));
    test_uint_eq(art_size(art), 1);
    test_void(art_remove(art, node));
    test_uint_eq(art_size(art), 0);
}

TEST_CASE_SIGNAL(art_memsize_invalid_magic, SIGABRT)
{
    art_memsize((art_ct)&not_an_art);
}

static size_t _test_art_size(art_const_ct art, void *data, void *ctx)
{
    return 1;
}

TEST_CASE_FIXTURE(art_memsize, art_new_empty, art_free)
{
    size_t size;
    
    test_ptr_success(art_insert(art, LIT("foo"), NULL));
    test_void(size = art_memsize(art));
    test_uint_eq(art_memsize_f(art, _test_art_size, NULL), size+1);
}

static art_ct _test_art_insert(art_ct art, int size)
{
    int k;
    
    key = tstr_dup_bl("x");
    
    for(k=0; k < size; k++)
    {
        str_overwrite_f(key, 0, "%c", k);
        
        if(!art_insert_value(art, key, k))
            return NULL;
    }
    
    return art;
}

TEST_CASE_SIGNAL(art_insert_invalid_magic, SIGABRT)
{
    art_insert((art_ct)&not_an_art, NULL, NULL);
}

TEST_CASE_FIXTURE(art_insert_invalid_key, art_new_empty, art_free)
{
    test_ptr_error(art_insert(art, BIN(""), NULL), E_ART_INVALID_KEY);
}

TEST_CASE_FIXTURE(art_insert_existing_key, art_new1, art_free)
{
    test_ptr_error(art_insert(art, LIT("foobar"), NULL), E_ART_EXISTS);
}

TEST_CASE_FIXTURE(art_insert1, art_new_empty, art_free)
{
    test_ptr_success(_test_art_insert(art, 1));
    test_uint_eq(art_size(art), 1);
}

TEST_CASE_FIXTURE(art_insert4, art_new_empty, art_free)
{
    test_ptr_success(_test_art_insert(art, 4));
    test_uint_eq(art_size(art), 4);
}

TEST_CASE_FIXTURE(art_insert8, art_new_empty, art_free)
{
    test_ptr_success(_test_art_insert(art, 8));
    test_uint_eq(art_size(art), 8);
}

TEST_CASE_FIXTURE(art_insert16, art_new_empty, art_free)
{
    test_ptr_success(_test_art_insert(art, 16));
    test_uint_eq(art_size(art), 16);
}

TEST_CASE_FIXTURE(art_insert32, art_new_empty, art_free)
{
    test_ptr_success(_test_art_insert(art, 32));
    test_uint_eq(art_size(art), 32);
}

TEST_CASE_FIXTURE(art_insert64, art_new_empty, art_free)
{
    test_ptr_success(_test_art_insert(art, 64));
    test_uint_eq(art_size(art), 64);
}

TEST_CASE_FIXTURE(art_insert128, art_new_empty, art_free)
{
    test_ptr_success(_test_art_insert(art, 128));
    test_uint_eq(art_size(art), 128);
}

TEST_CASE_FIXTURE(art_insert256, art_new_empty, art_free)
{
    test_ptr_success(_test_art_insert(art, 256));
    test_uint_eq(art_size(art), 256);
}

TEST_CASE_FIXTURE(art_insert_str_key_empty, art_new_empty, art_free)
{
    test_ptr_success(node = art_insert(art, LIT("foo"), NULL));
    test_ptr_success(key = art_node_get_key(node));
    test_uint_eq(str_len(key), 4);
    test_mem_eq(str_bc(key), "foo", 4);
    str_unref(key);
}

TEST_CASE_FIXTURE(art_insert_bin_key_empty, art_new_empty, art_free)
{
    test_ptr_success(node = art_insert(art, BIN("foo"), NULL));
    test_ptr_success(key = art_node_get_key(node));
    test_uint_eq(str_len(key), 3);
    test_mem_eq(str_bc(key), "foo", 3);
    str_unref(key);
}

TEST_CASE_FIXTURE(art_insert_str_key_split, art_new1, art_free)
{
    test_ptr_success(node = art_insert(art, LIT("foobaz"), NULL));
    test_ptr_success(key = art_node_get_key(node));
    test_uint_eq(str_len(key), 7);
    test_mem_eq(str_bc(key), "foobaz", 7);
    str_unref(key);
}

TEST_CASE_FIXTURE(art_insert_bin_key_split, art_new1, art_free)
{
    test_ptr_success(node = art_insert(art, BIN("foobaz"), NULL));
    test_ptr_success(key = art_node_get_key(node));
    test_uint_eq(str_len(key), 6);
    test_mem_eq(str_bc(key), "foobaz", 6);
    str_unref(key);
}

TEST_CASE_FIXTURE(art_insert_small_key_split_front, art_new_empty, art_free)
{
    test_ptr_success(art_insert(art, BIN("1aabb"), NULL));
    test_ptr_success(art_insert(art, BIN("21aabb"), NULL));
    test_ptr_success(art_insert(art, BIN("22aabb"), NULL));
    test_ptr_success(art_get(art, BIN("1aabb")));
    test_ptr_success(art_get(art, BIN("21aabb")));
    test_ptr_success(art_get(art, BIN("22aabb")));
}

TEST_CASE_FIXTURE(art_insert_small_key_split_center, art_new_empty, art_free)
{
    test_ptr_success(art_insert(art, BIN("aa1bb"), NULL));
    test_ptr_success(art_insert(art, BIN("aa2b1b"), NULL));
    test_ptr_success(art_insert(art, BIN("aa2b2b"), NULL));
    test_ptr_success(art_get(art, BIN("aa1bb")));
    test_ptr_success(art_get(art, BIN("aa2b1b")));
    test_ptr_success(art_get(art, BIN("aa2b2b")));
}

TEST_CASE_FIXTURE(art_insert_small_key_split_back, art_new_empty, art_free)
{
    test_ptr_success(art_insert(art, BIN("aabb1"), NULL));
    test_ptr_success(art_insert(art, BIN("aabb21"), NULL));
    test_ptr_success(art_insert(art, BIN("aabb22"), NULL));
    test_ptr_success(art_get(art, BIN("aabb1")));
    test_ptr_success(art_get(art, BIN("aabb21")));
    test_ptr_success(art_get(art, BIN("aabb22")));
}

TEST_CASE_FIXTURE(art_insert_large_key_split_front, art_new_empty, art_free)
{
    test_ptr_success(art_insert(art, BIN("1foofoobarbar"), NULL));
    test_ptr_success(art_insert(art, BIN("21foofoobarbar"), NULL));
    test_ptr_success(art_insert(art, BIN("22foofoobarbar"), NULL));
    test_ptr_success(art_get(art, BIN("1foofoobarbar")));
    test_ptr_success(art_get(art, BIN("21foofoobarbar")));
    test_ptr_success(art_get(art, BIN("22foofoobarbar")));
}

TEST_CASE_FIXTURE(art_insert_large_key_split_center, art_new_empty, art_free)
{
    test_ptr_success(art_insert(art, BIN("foofoo1barbar"), NULL));
    test_ptr_success(art_insert(art, BIN("foofoo2bar1bar"), NULL));
    test_ptr_success(art_insert(art, BIN("foofoo2bar2bar"), NULL));
    test_ptr_success(art_get(art, BIN("foofoo1barbar")));
    test_ptr_success(art_get(art, BIN("foofoo2bar1bar")));
    test_ptr_success(art_get(art, BIN("foofoo2bar2bar")));
}

TEST_CASE_FIXTURE(art_insert_large_key_split_back, art_new_empty, art_free)
{
    test_ptr_success(art_insert(art, BIN("foofoobarbar1"), NULL));
    test_ptr_success(art_insert(art, BIN("foofoobarbar21"), NULL));
    test_ptr_success(art_insert(art, BIN("foofoobarbar22"), NULL));
    test_ptr_success(art_get(art, BIN("foofoobarbar1")));
    test_ptr_success(art_get(art, BIN("foofoobarbar21")));
    test_ptr_success(art_get(art, BIN("foofoobarbar22")));
}

static art_ct _test_art_get(art_ct art, int size)
{
    int k;
    
    key = tstr_dup_bl("x");
    
    for(k=0; k < size; k++)
    {
        str_overwrite_f(key, 0, "%c", k);
        
        if(!(node = art_get(art, key))
        || art_node_get_value(node, int) != k)
            return NULL;
    }
    
    return art;
}

TEST_CASE_SIGNAL(art_get_invalid_magic, SIGABRT)
{
    art_get((art_ct)&not_an_art, NULL);
}

TEST_CASE_FIXTURE(art_get0_not_found, art_new_empty, art_free)
{
    test_ptr_error(art_get(art, LIT("foo")), E_ART_NOT_FOUND);
}

TEST_CASE_FIXTURE(art_get1_not_found, art_new1, art_free)
{
    test_ptr_error(art_get(art, LIT("foobaz")), E_ART_NOT_FOUND);
}

TEST_CASE_FIXTURE(art_get1, art_new_empty, art_free)
{
    test_ptr_success(_test_art_insert(art, 1));
    test_ptr_success(_test_art_get(art, 1));
}

TEST_CASE_FIXTURE(art_get4, art_new_empty, art_free)
{
    test_ptr_success(_test_art_insert(art, 4));
    test_ptr_success(_test_art_get(art, 4));
}

TEST_CASE_FIXTURE(art_get8, art_new_empty, art_free)
{
    test_ptr_success(_test_art_insert(art, 8));
    test_ptr_success(_test_art_get(art, 8));
}

TEST_CASE_FIXTURE(art_get16, art_new_empty, art_free)
{
    test_ptr_success(_test_art_insert(art, 16));
    test_ptr_success(_test_art_get(art, 16));
}

TEST_CASE_FIXTURE(art_get32, art_new_empty, art_free)
{
    test_ptr_success(_test_art_insert(art, 32));
    test_ptr_success(_test_art_get(art, 32));
}

TEST_CASE_FIXTURE(art_get64, art_new_empty, art_free)
{
    test_ptr_success(_test_art_insert(art, 64));
    test_ptr_success(_test_art_get(art, 64));
}

TEST_CASE_FIXTURE(art_get128, art_new_empty, art_free)
{
    test_ptr_success(_test_art_insert(art, 128));
    test_ptr_success(_test_art_get(art, 128));
}

TEST_CASE_FIXTURE(art_get256, art_new_empty, art_free)
{
    test_ptr_success(_test_art_insert(art, 256));
    test_ptr_success(_test_art_get(art, 256));
}

static void _test_art_dtor(art_const_ct art, void *data, void *ctx)
{
    int *count = ctx;
    
    *count += 1;
}

static int _test_art_remove(art_ct art, int size)
{
    int k, count = 0;
    
    key = tstr_dup_bl("x");
    
    for(k=0; k < size; k++)
    {
        str_overwrite_f(key, 0, "%c", k);
        
        if(art_remove_pf(art, key, _test_art_dtor, &count))
            return -1;
    }
    
    return count;
}

TEST_CASE_SIGNAL(art_remove_invalid_magic, SIGABRT)
{
    art_remove((art_ct)&not_an_art, NULL);
}

TEST_CASE_SIGNAL(art_remove_p_invalid_magic, SIGABRT)
{
    art_remove_p((art_ct)&not_an_art, NULL);
}

TEST_CASE_SIGNAL(art_remove_pf_invalid_magic, SIGABRT)
{
    art_remove_pf((art_ct)&not_an_art, NULL, _test_art_dtor, NULL);
}

TEST_CASE_FIXTURE(art_remove0_not_found, art_new_empty, art_free)
{
    test_int_error(art_remove_p(art, LIT("foobaz")), E_ART_NOT_FOUND);
}

TEST_CASE_FIXTURE(art_remove1_not_found, art_new1, art_free)
{
    test_int_error(art_remove_p(art, LIT("foobaz")), E_ART_NOT_FOUND);
}

TEST_CASE_FIXTURE(art_remove1, art_new_empty, art_free)
{
    test_ptr_success(_test_art_insert(art, 1));
    test_rc_success(_test_art_remove(art, 1), 1);
    test_uint_eq(art_size(art), 0);
}

TEST_CASE_FIXTURE(art_remove4, art_new_empty, art_free)
{
    test_ptr_success(_test_art_insert(art, 4));
    test_rc_success(_test_art_remove(art, 4), 4);
    test_uint_eq(art_size(art), 0);
}

TEST_CASE_FIXTURE(art_remove8, art_new_empty, art_free)
{
    test_ptr_success(_test_art_insert(art, 8));
    test_rc_success(_test_art_remove(art, 8), 8);
    test_uint_eq(art_size(art), 0);
}

TEST_CASE_FIXTURE(art_remove16, art_new_empty, art_free)
{
    test_ptr_success(_test_art_insert(art, 16));
    test_rc_success(_test_art_remove(art, 16), 16);
    test_uint_eq(art_size(art), 0);
}

TEST_CASE_FIXTURE(art_remove32, art_new_empty, art_free)
{
    test_ptr_success(_test_art_insert(art, 32));
    test_rc_success(_test_art_remove(art, 32), 32);
    test_uint_eq(art_size(art), 0);
}

TEST_CASE_FIXTURE(art_remove64, art_new_empty, art_free)
{
    test_ptr_success(_test_art_insert(art, 64));
    test_rc_success(_test_art_remove(art, 64), 64);
    test_uint_eq(art_size(art), 0);
}

TEST_CASE_FIXTURE(art_remove128, art_new_empty, art_free)
{
    test_ptr_success(_test_art_insert(art, 128));
    test_rc_success(_test_art_remove(art, 128), 128);
    test_uint_eq(art_size(art), 0);
}

TEST_CASE_FIXTURE(art_remove256, art_new_empty, art_free)
{
    test_ptr_success(_test_art_insert(art, 256));
    test_rc_success(_test_art_remove(art, 256), 256);
    test_uint_eq(art_size(art), 0);
}

TEST_CASE_FIXTURE(art_remove_merge, art_new4, art_free)
{
    test_ptr_success(node = art_get(art, LIT("fooduh")));
    test_int_success(art_remove_p(art, LIT("foobar")));
    test_int_success(art_remove_p(art, LIT("foobaz")));
    test_ptr_success(key = art_node_get_key(node));
    test_str_eq(str_bc(key), "fooduh");
    str_unref(key);
}

static bool _test_art_pred_value(art_const_ct art, str_const_ct key, const void *data, void *ctx)
{
    int value1 = POINTER_TO_VALUE(data, int), value2 = POINTER_TO_VALUE(ctx, int);
    
    return value1 == value2;
}

static bool _test_art_pred_key(art_const_ct art, str_const_ct key1, const void *data, void *ctx)
{
    char *key2 = ctx;
    
    return !strcmp(str_bc(key1), key2);
}

TEST_CASE_SIGNAL(art_find_invalid_magic, SIGABRT)
{
    art_find((art_ct)&not_an_art, _test_art_pred_value, NULL);
}

TEST_CASE_FIXTURE_SIGNAL(art_find_invalid_pred, art_new_empty, art_free, SIGABRT)
{
    art_find(art, NULL, NULL);
}

TEST_CASE_FIXTURE(art_find_not_found, art_new4, art_free)
{
    test_ptr_error(art_find(art, _test_art_pred_value, VALUE_TO_POINTER(123)), E_ART_NOT_FOUND);
}

TEST_CASE_FIXTURE(art_find, art_new4, art_free)
{
    test_ptr_success(node = art_find(art, _test_art_pred_value, VALUE_TO_POINTER(3)));
    test_ptr_success(key = art_node_get_key(node));
    test_str_eq(str_bc(key), "fooduh");
    str_unref(key);
}

TEST_CASE_SIGNAL(art_find_k_invalid_magic, SIGABRT)
{
    art_find_k((art_ct)&not_an_art, _test_art_pred_key, NULL);
}

TEST_CASE_FIXTURE_SIGNAL(art_find_k_invalid_pred, art_new_empty, art_free, SIGABRT)
{
    art_find_k(art, NULL, NULL);
}

TEST_CASE_FIXTURE(art_find_k_not_found, art_new4, art_free)
{
    test_ptr_error(art_find_k(art, _test_art_pred_key, "blubb"), E_ART_NOT_FOUND);
}

TEST_CASE_FIXTURE(art_find_k, art_new4, art_free)
{
    test_ptr_success(node = art_find_k(art, _test_art_pred_key, "fooduh"));
    test_int_eq(art_node_get_value(node, int), 3);
}

TEST_CASE_SIGNAL(art_find_r_invalid_magic, SIGABRT)
{
    art_find_r((art_ct)&not_an_art, _test_art_pred_value, NULL);
}

TEST_CASE_FIXTURE_SIGNAL(art_find_r_invalid_pred, art_new_empty, art_free, SIGABRT)
{
    art_find_r(art, NULL, NULL);
}

TEST_CASE_FIXTURE(art_find_r_not_found, art_new4, art_free)
{
    test_ptr_error(art_find_r(art, _test_art_pred_value, VALUE_TO_POINTER(123)), E_ART_NOT_FOUND);
}

TEST_CASE_FIXTURE(art_find_r, art_new4, art_free)
{
    test_ptr_success(node = art_find_r(art, _test_art_pred_value, VALUE_TO_POINTER(1)));
    test_ptr_success(key = art_node_get_key(node));
    test_str_eq(str_bc(key), "foobar");
    str_unref(key);
}

TEST_CASE_SIGNAL(art_find_rk_invalid_magic, SIGABRT)
{
    art_find_rk((art_ct)&not_an_art, _test_art_pred_key, NULL);
}

TEST_CASE_FIXTURE_SIGNAL(art_find_rk_invalid_pred, art_new_empty, art_free, SIGABRT)
{
    art_find_rk(art, NULL, NULL);
}

TEST_CASE_FIXTURE(art_find_rk_not_found, art_new4, art_free)
{
    test_ptr_error(art_find_rk(art, _test_art_pred_key, "blubb"), E_ART_NOT_FOUND);
}

TEST_CASE_FIXTURE(art_find_rk, art_new4, art_free)
{
    test_ptr_success(node = art_find_rk(art, _test_art_pred_key, "foobar"));
    test_int_eq(art_node_get_value(node, int), 1);
}

TEST_CASE_SIGNAL(art_find_p_invalid_magic, SIGABRT)
{
    art_find_p((art_ct)&not_an_art, BIN("foo"), _test_art_pred_value, NULL);
}

TEST_CASE_FIXTURE_SIGNAL(art_find_p_invalid_pred, art_new_empty, art_free, SIGABRT)
{
    art_find_p(art, LIT("foo"), NULL, NULL);
}

TEST_CASE_FIXTURE(art_find_p_prefix_not_found, art_new4, art_free)
{
    test_ptr_error(art_find_p(art, BIN("blubb"), _test_art_pred_value, VALUE_TO_POINTER(1)), E_ART_NOT_FOUND);
}

TEST_CASE_FIXTURE(art_find_p_value_not_found, art_new4, art_free)
{
    test_ptr_error(art_find_p(art, BIN("foo"), _test_art_pred_value, VALUE_TO_POINTER(123)), E_ART_NOT_FOUND);
}

TEST_CASE_FIXTURE(art_find_p, art_new4, art_free)
{
    test_ptr_success(node = art_find_p(art, BIN("foo"), _test_art_pred_value, VALUE_TO_POINTER(3)));
    test_ptr_success(key = art_node_get_key(node));
    test_str_eq(str_bc(key), "fooduh");
    str_unref(key);
}

TEST_CASE_SIGNAL(art_find_pk_invalid_magic, SIGABRT)
{
    art_find_pk((art_ct)&not_an_art, BIN("foo"), _test_art_pred_key, NULL);
}

TEST_CASE_FIXTURE_SIGNAL(art_find_pk_invalid_pred, art_new_empty, art_free, SIGABRT)
{
    art_find_pk(art, BIN("foo"), NULL, NULL);
}

TEST_CASE_FIXTURE(art_find_pk_prefix_not_found, art_new4, art_free)
{
    test_ptr_error(art_find_pk(art, BIN("blubb"), _test_art_pred_key, "fooduh"), E_ART_NOT_FOUND);
}

TEST_CASE_FIXTURE(art_find_pk_key_not_found, art_new4, art_free)
{
    test_ptr_error(art_find_pk(art, BIN("foo"), _test_art_pred_key, "blubb"), E_ART_NOT_FOUND);
}

TEST_CASE_FIXTURE(art_find_pk, art_new4, art_free)
{
    test_ptr_success(node = art_find_pk(art, BIN("foo"), _test_art_pred_key, "fooduh"));
    test_int_eq(art_node_get_value(node, int), 3);
}

TEST_CASE_SIGNAL(art_find_pr_invalid_magic, SIGABRT)
{
    art_find_pr((art_ct)&not_an_art, BIN("foo"), _test_art_pred_value, NULL);
}

TEST_CASE_FIXTURE_SIGNAL(art_find_pr_invalid_pred, art_new_empty, art_free, SIGABRT)
{
    art_find_pr(art, BIN("foo"), NULL, NULL);
}

TEST_CASE_FIXTURE(art_find_pr_prefix_not_found, art_new4, art_free)
{
    test_ptr_error(art_find_pr(art, BIN("blubb"), _test_art_pred_value, VALUE_TO_POINTER(1)), E_ART_NOT_FOUND);
}

TEST_CASE_FIXTURE(art_find_pr_value_not_found, art_new4, art_free)
{
    test_ptr_error(art_find_pr(art, BIN("foo"), _test_art_pred_value, VALUE_TO_POINTER(123)), E_ART_NOT_FOUND);
}

TEST_CASE_FIXTURE(art_find_pr, art_new4, art_free)
{
    test_ptr_success(node = art_find_pr(art, BIN("foo"), _test_art_pred_value, VALUE_TO_POINTER(1)));
    test_ptr_success(key = art_node_get_key(node));
    test_str_eq(str_bc(key), "foobar");
    str_unref(key);
}

TEST_CASE_SIGNAL(art_find_prk_invalid_magic, SIGABRT)
{
    art_find_prk((art_ct)&not_an_art, BIN("foo"), _test_art_pred_key, NULL);
}

TEST_CASE_FIXTURE_SIGNAL(art_find_prk_invalid_pred, art_new_empty, art_free, SIGABRT)
{
    art_find_prk(art, BIN("foo"), NULL, NULL);
}

TEST_CASE_FIXTURE(art_find_prk_prefix_not_found, art_new4, art_free)
{
    test_ptr_error(art_find_prk(art, BIN("blubb"), _test_art_pred_key, "foobar"), E_ART_NOT_FOUND);
}

TEST_CASE_FIXTURE(art_find_prk_key_not_found, art_new4, art_free)
{
    test_ptr_error(art_find_prk(art, BIN("foo"), _test_art_pred_key, "blubb"), E_ART_NOT_FOUND);
}

TEST_CASE_FIXTURE(art_find_prk, art_new4, art_free)
{
    test_ptr_success(node = art_find_prk(art, BIN("foo"), _test_art_pred_key, "foobar"));
    test_int_eq(art_node_get_value(node, int), 1);
}

static int _test_art_fold_value(art_const_ct art, str_const_ct key, void *data, void *ctx)
{
    int *sum = ctx;
    
    *sum = *sum*10 + POINTER_TO_VALUE(data, int);
    
    return 0;
}

static int _test_art_fold_key(art_const_ct art, str_const_ct key, void *data, void *ctx)
{
    str_ct sum = ctx;
    
    return str_append_n(sum, key, str_len(key)-1) ? 0 : -1;
}

TEST_CASE_SIGNAL(art_fold_invalid_magic, SIGABRT)
{
    art_fold((art_ct)&not_an_art, _test_art_fold_value, NULL);
}

TEST_CASE_FIXTURE_SIGNAL(art_fold_invalid_callback, art_new_empty, art_free, SIGABRT)
{
    art_fold(art, NULL, NULL);
}

TEST_CASE_FIXTURE(art_fold, art_new4, art_free)
{
    int sum = 0;
    test_int_success(art_fold(art, _test_art_fold_value, &sum));
    test_int_eq(sum, 1234);
}

TEST_CASE_SIGNAL(art_fold_k_invalid_magic, SIGABRT)
{
    art_fold_k((art_ct)&not_an_art, _test_art_fold_key, NULL);
}

TEST_CASE_FIXTURE_SIGNAL(art_fold_k_invalid_callback, art_new_empty, art_free, SIGABRT)
{
    art_fold_k(art, NULL, NULL);
}

TEST_CASE_FIXTURE(art_fold_k, art_new4, art_free)
{
    test_ptr_success(key = str_prepare_c(0, 30));
    test_int_success(art_fold_k(art, _test_art_fold_key, key));
    test_str_eq(str_bc(key), "foobarfoobazfooduhxyz");
    str_unref(key);
}

TEST_CASE_SIGNAL(art_fold_r_invalid_magic, SIGABRT)
{
    art_fold_r((art_ct)&not_an_art, _test_art_fold_value, NULL);
}

TEST_CASE_FIXTURE_SIGNAL(art_fold_r_invalid_callback, art_new_empty, art_free, SIGABRT)
{
    art_fold_r(art, NULL, NULL);
}

TEST_CASE_FIXTURE(art_fold_r, art_new4, art_free)
{
    int sum = 0;
    test_int_success(art_fold_r(art, _test_art_fold_value, &sum));
    test_int_eq(sum, 4321);
}

TEST_CASE_SIGNAL(art_fold_rk_invalid_magic, SIGABRT)
{
    art_fold_rk((art_ct)&not_an_art, _test_art_fold_key, NULL);
}

TEST_CASE_FIXTURE_SIGNAL(art_fold_rk_invalid_callback, art_new_empty, art_free, SIGABRT)
{
    art_fold_rk(art, NULL, NULL);
}

TEST_CASE_FIXTURE(art_fold_rk, art_new4, art_free)
{
    test_ptr_success(key = str_prepare_c(0, 30));
    test_int_success(art_fold_rk(art, _test_art_fold_key, key));
    test_str_eq(str_bc(key), "xyzfooduhfoobazfoobar");
    str_unref(key);
}

TEST_CASE_SIGNAL(art_fold_p_invalid_magic, SIGABRT)
{
    art_fold_p((art_ct)&not_an_art, BIN("foo"), _test_art_fold_value, NULL);
}

TEST_CASE_FIXTURE_SIGNAL(art_fold_p_invalid_callback, art_new_empty, art_free, SIGABRT)
{
    art_fold_p(art, BIN("foo"), NULL, NULL);
}

TEST_CASE_FIXTURE(art_fold_p_not_found, art_new4, art_free)
{
    test_int_error(art_fold_p(art, BIN("blubb"), _test_art_fold_value, NULL), E_ART_NOT_FOUND);
}

TEST_CASE_FIXTURE(art_fold_p, art_new4, art_free)
{
    int sum = 0;
    test_int_success(art_fold_p(art, BIN("foo"), _test_art_fold_value, &sum));
    test_int_eq(sum, 123);
}

TEST_CASE_SIGNAL(art_fold_pk_invalid_magic, SIGABRT)
{
    art_fold_pk((art_ct)&not_an_art, BIN("foo"), _test_art_fold_key, NULL);
}

TEST_CASE_FIXTURE_SIGNAL(art_fold_pk_invalid_callback, art_new_empty, art_free, SIGABRT)
{
    art_fold_pk(art, BIN("foo"), NULL, NULL);
}

TEST_CASE_FIXTURE(art_fold_pk_not_found, art_new4, art_free)
{
    test_int_error(art_fold_pk(art, BIN("blubb"), _test_art_fold_key, NULL), E_ART_NOT_FOUND);
}

TEST_CASE_FIXTURE(art_fold_pk, art_new4, art_free)
{
    test_ptr_success(key = str_prepare_c(0, 30));
    test_int_success(art_fold_pk(art, BIN("foo"), _test_art_fold_key, key));
    test_str_eq(str_bc(key), "foobarfoobazfooduh");
    str_unref(key);
}

TEST_CASE_SIGNAL(art_fold_pr_invalid_magic, SIGABRT)
{
    art_fold_pr((art_ct)&not_an_art, BIN("foo"), _test_art_fold_value, NULL);
}

TEST_CASE_FIXTURE_SIGNAL(art_fold_pr_invalid_callback, art_new_empty, art_free, SIGABRT)
{
    art_fold_pr(art, BIN("foo"), NULL, NULL);
}

TEST_CASE_FIXTURE(art_fold_pr_not_found, art_new4, art_free)
{
    test_int_error(art_fold_pr(art, BIN("blubb"), _test_art_fold_value, NULL), E_ART_NOT_FOUND);
}

TEST_CASE_FIXTURE(art_fold_pr, art_new4, art_free)
{
    int sum = 0;
    test_int_success(art_fold_pr(art, BIN("foo"), _test_art_fold_value, &sum));
    test_int_eq(sum, 321);
}

TEST_CASE_SIGNAL(art_fold_prk_invalid_magic, SIGABRT)
{
    art_fold_prk((art_ct)&not_an_art, BIN("foo"), _test_art_fold_key, NULL);
}

TEST_CASE_FIXTURE_SIGNAL(art_fold_prk_invalid_callback, art_new_empty, art_free, SIGABRT)
{
    art_fold_prk(art, BIN("foo"), NULL, NULL);
}

TEST_CASE_FIXTURE(art_fold_prk_not_found, art_new4, art_free)
{
    test_int_error(art_fold_prk(art, BIN("blubb"), _test_art_fold_key, NULL), E_ART_NOT_FOUND);
}

TEST_CASE_FIXTURE(art_fold_prk, art_new4, art_free)
{
    test_ptr_success(key = str_prepare_c(0, 30));
    test_int_success(art_fold_prk(art, BIN("foo"), _test_art_fold_key, key));
    test_str_eq(str_bc(key), "fooduhfoobazfoobar");
    str_unref(key);
}

TEST_CASE_SIGNAL(art_complete_invalid_magic, SIGABRT)
{
    art_complete((art_ct)&not_an_art, BIN("fo"));
}

TEST_CASE_FIXTURE(art_complete_empty, art_new_empty, art_free)
{
    test_ptr_error(art_complete(art, BIN("fo")), E_ART_EMPTY);
}

TEST_CASE_FIXTURE(art_complete_not_found, art_new4, art_free)
{
    test_ptr_error(art_complete(art, BIN("blubb")), E_ART_NOT_FOUND);
}

TEST_CASE_FIXTURE(art_complete, art_new4, art_free)
{
    test_ptr_success(key = art_complete(art, BIN("fo")));
    test_str_eq(str_bc(key), "o");
    str_unref(key);
}

TEST_CASE_FIXTURE(art_complete_begin, art_new4, art_free)
{
    test_ptr_success(key = art_complete(art, NULL));
    test_true(str_is_empty(key));
    str_unref(key);
}

TEST_CASE_FIXTURE(art_complete_end, art_new4, art_free)
{
    test_ptr_success(key = art_complete(art, BIN("foo")));
    test_true(str_is_empty(key));
    str_unref(key);
}

test_suite_ct test_suite_art(void)
{
    return test_suite_new_with_cases("art"
        , test_case_new(art_is_empty_invalid_magic)
        , test_case_new(art_is_empty)
        , test_case_new(art_size_invalid_magic)
        , test_case_new(art_size)
        , test_case_new(art_memsize_invalid_magic)
        , test_case_new(art_memsize)
        
        , test_case_new(art_insert_invalid_magic)
        , test_case_new(art_insert_invalid_key)
        , test_case_new(art_insert_existing_key)
        , test_case_new(art_insert1)
        , test_case_new(art_insert4)
        , test_case_new(art_insert8)
        , test_case_new(art_insert16)
        , test_case_new(art_insert32)
        , test_case_new(art_insert64)
        , test_case_new(art_insert128)
        , test_case_new(art_insert256)
        , test_case_new(art_insert_str_key_empty)
        , test_case_new(art_insert_bin_key_empty)
        , test_case_new(art_insert_str_key_split)
        , test_case_new(art_insert_bin_key_split)
        , test_case_new(art_insert_small_key_split_front)
        , test_case_new(art_insert_small_key_split_center)
        , test_case_new(art_insert_small_key_split_back)
        , test_case_new(art_insert_large_key_split_front)
        , test_case_new(art_insert_large_key_split_center)
        , test_case_new(art_insert_large_key_split_back)
        
        , test_case_new(art_get_invalid_magic)
        , test_case_new(art_get0_not_found)
        , test_case_new(art_get1_not_found)
        , test_case_new(art_get1)
        , test_case_new(art_get4)
        , test_case_new(art_get8)
        , test_case_new(art_get16)
        , test_case_new(art_get32)
        , test_case_new(art_get64)
        , test_case_new(art_get128)
        , test_case_new(art_get256)
        
        , test_case_new(art_remove_invalid_magic)
        , test_case_new(art_remove_p_invalid_magic)
        , test_case_new(art_remove_pf_invalid_magic)
        , test_case_new(art_remove0_not_found)
        , test_case_new(art_remove1_not_found)
        , test_case_new(art_remove1)
        , test_case_new(art_remove4)
        , test_case_new(art_remove8)
        , test_case_new(art_remove16)
        , test_case_new(art_remove32)
        , test_case_new(art_remove64)
        , test_case_new(art_remove128)
        , test_case_new(art_remove256)
        , test_case_new(art_remove_merge)
        
        , test_case_new(art_find_invalid_magic)
        , test_case_new(art_find_invalid_pred)
        , test_case_new(art_find_not_found)
        , test_case_new(art_find)
        , test_case_new(art_find_k_invalid_magic)
        , test_case_new(art_find_k_invalid_pred)
        , test_case_new(art_find_k_not_found)
        , test_case_new(art_find_k)
        , test_case_new(art_find_r_invalid_magic)
        , test_case_new(art_find_r_invalid_pred)
        , test_case_new(art_find_r_not_found)
        , test_case_new(art_find_r)
        , test_case_new(art_find_rk_invalid_magic)
        , test_case_new(art_find_rk_invalid_pred)
        , test_case_new(art_find_rk_not_found)
        , test_case_new(art_find_rk)
        
        , test_case_new(art_find_p_invalid_magic)
        , test_case_new(art_find_p_invalid_pred)
        , test_case_new(art_find_p_prefix_not_found)
        , test_case_new(art_find_p_value_not_found)
        , test_case_new(art_find_p)
        , test_case_new(art_find_pk_invalid_magic)
        , test_case_new(art_find_pk_invalid_pred)
        , test_case_new(art_find_pk_prefix_not_found)
        , test_case_new(art_find_pk_key_not_found)
        , test_case_new(art_find_pk)
        , test_case_new(art_find_pr_invalid_magic)
        , test_case_new(art_find_pr_invalid_pred)
        , test_case_new(art_find_pr_prefix_not_found)
        , test_case_new(art_find_pr_value_not_found)
        , test_case_new(art_find_pr)
        , test_case_new(art_find_prk_invalid_magic)
        , test_case_new(art_find_prk_invalid_pred)
        , test_case_new(art_find_prk_prefix_not_found)
        , test_case_new(art_find_prk_key_not_found)
        , test_case_new(art_find_prk)
        
        , test_case_new(art_fold_invalid_magic)
        , test_case_new(art_fold_invalid_callback)
        , test_case_new(art_fold)
        , test_case_new(art_fold_k_invalid_magic)
        , test_case_new(art_fold_k_invalid_callback)
        , test_case_new(art_fold_k)
        , test_case_new(art_fold_r_invalid_magic)
        , test_case_new(art_fold_r_invalid_callback)
        , test_case_new(art_fold_r)
        , test_case_new(art_fold_rk_invalid_magic)
        , test_case_new(art_fold_rk_invalid_callback)
        , test_case_new(art_fold_rk)
        
        , test_case_new(art_fold_p_invalid_magic)
        , test_case_new(art_fold_p_invalid_callback)
        , test_case_new(art_fold_p_not_found)
        , test_case_new(art_fold_p)
        , test_case_new(art_fold_pk_invalid_magic)
        , test_case_new(art_fold_pk_invalid_callback)
        , test_case_new(art_fold_pk_not_found)
        , test_case_new(art_fold_pk)
        , test_case_new(art_fold_pr_invalid_magic)
        , test_case_new(art_fold_pr_invalid_callback)
        , test_case_new(art_fold_pr_not_found)
        , test_case_new(art_fold_pr)
        , test_case_new(art_fold_prk_invalid_magic)
        , test_case_new(art_fold_prk_invalid_callback)
        , test_case_new(art_fold_prk_not_found)
        , test_case_new(art_fold_prk)
        
        , test_case_new(art_complete_invalid_magic)
        , test_case_new(art_complete_empty)
        , test_case_new(art_complete_not_found)
        , test_case_new(art_complete)
        , test_case_new(art_complete_begin)
        , test_case_new(art_complete_end)
    );
}
