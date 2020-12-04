/*
 * Copyright (c) 2020 Martin Rödel a.k.a. Yomin Nimoy
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

#include "enc.h"
#include <ytil/test/run.h>
#include <ytil/test/test.h>
#include <ytil/enc/qpenc.h>
#include <stdio.h>

static const struct not_a_str
{
    int foo;
} not_a_str = { 123 };

static char text_plain[128], text_enc[128*3];
static size_t len_enc;
static str_ct str;


TEST_CASE_ABORT(qpenc_encode_invalid_blob1)
{
    qpenc_encode(NULL);
}

TEST_CASE_ABORT(qpenc_encode_invalid_blob2)
{
    qpenc_encode((str_ct)&not_a_str);
}

TEST_CASE(qpenc_encode_empty)
{
    test_ptr_error(qpenc_encode(LIT("")), E_QPENC_EMPTY);
}

TEST_SETUP(mktext)
{
    int i;

    for(i=0,len_enc=0; i<128; i++)
    {
        text_plain[i] = i;

        if((i >= '!' && i <= '~' && i != '=')
        || (i == ' ' || i == '\t'))
            text_enc[len_enc++] = i;
        else
            len_enc += snprintf(&text_enc[len_enc], 4, "=%02X", i);
    }

    text_enc[len_enc] = '\0';
}

TEST_CASE_FIX(qpenc_encode, mktext, no_teardown)
{
    test_ptr_success(str = qpenc_encode(BLOB(text_plain, 128)));
    test_false(str_is_binary(str));
    test_str_eq(str_c(str), text_enc);
    str_unref(str);
}

TEST_CASE(qpenc_encode_trailing_space)
{
    test_ptr_success(str = qpenc_encode(LIT("foo  ")));
    test_str_eq(str_c(str), "foo =20");
    str_unref(str);
}

TEST_CASE(qpenc_encode_trailing_tab)
{
    test_ptr_success(str = qpenc_encode(LIT("foo\t\t")));
    test_str_eq(str_c(str), "foo\t=09");
    str_unref(str);
}

TEST_CASE_ABORT(qpenc_decode_invalid_blob1)
{
    qpenc_decode(NULL);
}

TEST_CASE_ABORT(qpenc_decode_invalid_blob2)
{
    qpenc_decode((str_ct)&not_a_str);
}

TEST_CASE(qpenc_decode_empty)
{
    test_ptr_error(qpenc_decode(LIT("")), E_QPENC_EMPTY);
}

TEST_CASE(qpenc_decode_invalid_data)
{
    test_ptr_error(qpenc_decode(LIT("foo\nbar")), E_QPENC_INVALID_DATA);
}

TEST_CASE(qpenc_decode_incomplete_hex1)
{
    test_ptr_error(qpenc_decode(LIT("foo=")), E_QPENC_INVALID_DATA);
}

TEST_CASE(qpenc_decode_incomplete_hex2)
{
    test_ptr_error(qpenc_decode(LIT("foo=A")), E_QPENC_INVALID_DATA);
}

TEST_CASE(qpenc_decode_invalid_hex1)
{
    test_ptr_error(qpenc_decode(LIT("foo=GA")), E_QPENC_INVALID_DATA);
}

TEST_CASE(qpenc_decode_invalid_hex2)
{
    test_ptr_error(qpenc_decode(LIT("foo=AG")), E_QPENC_INVALID_DATA);
}

TEST_CASE(qpenc_decode_invalid_hex3)
{
    test_ptr_error(qpenc_decode(LIT("foo=aA")), E_QPENC_INVALID_DATA);
}

TEST_CASE(qpenc_decode_invalid_hex4)
{
    test_ptr_error(qpenc_decode(LIT("foo=Aa")), E_QPENC_INVALID_DATA);
}

TEST_CASE(qpenc_decode_trailing_space)
{
    test_ptr_error(qpenc_decode(LIT("foo ")), E_QPENC_INVALID_DATA);
}

TEST_CASE(qpenc_decode_trailing_tab)
{
    test_ptr_error(qpenc_decode(LIT("foo\t")), E_QPENC_INVALID_DATA);
}

TEST_CASE_FIX(qpenc_decode, mktext, no_teardown)
{
    test_ptr_success(str = qpenc_decode(STR(text_enc)));
    test_true(str_is_binary(str));
    test_uint_eq(str_len(str), 128);
    test_mem_eq(str_buc(str), text_plain, 128);
    str_unref(str);
}

TEST_CASE_ABORT(qpenc_is_valid_invalid_blob1)
{
    qpenc_is_valid(NULL);
}

TEST_CASE_ABORT(qpenc_is_valid_invalid_blob2)
{
    qpenc_is_valid((str_ct)&not_a_str);
}

TEST_CASE(qpenc_is_valid_empty)
{
    test_false(qpenc_is_valid(LIT("")));
}

TEST_CASE(qpenc_is_valid_invalid_data)
{
    test_false(qpenc_is_valid(LIT("foo\nbar")));
}

TEST_CASE(qpenc_is_valid_incomplete_hex1)
{
    test_false(qpenc_is_valid(LIT("foo=")));
}

TEST_CASE(qpenc_is_valid_incomplete_hex2)
{
    test_false(qpenc_is_valid(LIT("foo=A")));
}

TEST_CASE(qpenc_is_valid_invalid_hex1)
{
    test_false(qpenc_is_valid(LIT("foo=GA")));
}

TEST_CASE(qpenc_is_valid_invalid_hex2)
{
    test_false(qpenc_is_valid(LIT("foo=AG")));
}

TEST_CASE(qpenc_is_valid_invalid_hex3)
{
    test_false(qpenc_is_valid(LIT("foo=aA")));
}

TEST_CASE(qpenc_is_valid_invalid_hex4)
{
    test_false(qpenc_is_valid(LIT("foo=Aa")));
}

TEST_CASE(qpenc_is_valid_trailing_space)
{
    test_false(qpenc_is_valid(LIT("foo ")));
}

TEST_CASE(qpenc_is_valid_trailing_tab)
{
    test_false(qpenc_is_valid(LIT("foo\t")));
}

TEST_CASE_FIX(qpenc_is_valid, mktext, no_teardown)
{
    test_true(qpenc_is_valid(STR(text_enc)));
}

int test_suite_enc_qpenc(void)
{
    return error_pass_int(test_run_cases("qpenc",
        test_case(qpenc_encode_invalid_blob1),
        test_case(qpenc_encode_invalid_blob2),
        test_case(qpenc_encode_empty),
        test_case(qpenc_encode),
        test_case(qpenc_encode_trailing_space),
        test_case(qpenc_encode_trailing_tab),

        test_case(qpenc_decode_invalid_blob1),
        test_case(qpenc_decode_invalid_blob2),
        test_case(qpenc_decode_empty),
        test_case(qpenc_decode_invalid_data),
        test_case(qpenc_decode_incomplete_hex1),
        test_case(qpenc_decode_incomplete_hex2),
        test_case(qpenc_decode_invalid_hex1),
        test_case(qpenc_decode_invalid_hex2),
        test_case(qpenc_decode_invalid_hex3),
        test_case(qpenc_decode_invalid_hex4),
        test_case(qpenc_decode_trailing_space),
        test_case(qpenc_decode_trailing_tab),
        test_case(qpenc_decode),

        test_case(qpenc_is_valid_invalid_blob1),
        test_case(qpenc_is_valid_invalid_blob2),
        test_case(qpenc_is_valid_empty),
        test_case(qpenc_is_valid_invalid_data),
        test_case(qpenc_is_valid_incomplete_hex1),
        test_case(qpenc_is_valid_incomplete_hex2),
        test_case(qpenc_is_valid_invalid_hex1),
        test_case(qpenc_is_valid_invalid_hex2),
        test_case(qpenc_is_valid_invalid_hex3),
        test_case(qpenc_is_valid_invalid_hex4),
        test_case(qpenc_is_valid_trailing_space),
        test_case(qpenc_is_valid_trailing_tab),
        test_case(qpenc_is_valid),

        NULL
    ));
}
