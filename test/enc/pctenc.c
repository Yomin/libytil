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
#include <ytil/enc/pctenc.h>
#include <stdio.h>

static const struct not_a_str
{
    int foo;
} not_a_str = { 123 };

static char text_plain[128], text_enc[128*3];
static size_t len_enc;
static str_ct str;


TEST_CASE_ABORT(pctenc_encode_invalid_blob1)
{
    pctenc_encode(NULL);
}

TEST_CASE_ABORT(pctenc_encode_invalid_blob2)
{
    pctenc_encode((str_ct)&not_a_str);
}

TEST_CASE(pctenc_encode_empty)
{
    test_ptr_error(pctenc_encode(LIT("")), E_PCTENC_EMPTY);
}

TEST_PSETUP(mktext, bool upper)
{
    int i;

    for(i=0,len_enc=0; i<128; i++)
    {
        text_plain[i] = i;

        if(isalnum(i) || i == '-' || i == '_' || i == '.' || i == '~')
            text_enc[len_enc++] = i;
        else
            len_enc += snprintf(&text_enc[len_enc], 4, upper ? "%%%02X" : "%%%02x", i);
    }

    text_enc[len_enc] = '\0';
}

TEST_CASE_PFIX(pctenc_encode, mktext, no_teardown, true)
{
    test_ptr_success(str = pctenc_encode(BLOB(text_plain, 128)));
    test_false(str_is_binary(str));
    test_str_eq(str_c(str), text_enc);
    str_unref(str);
}

TEST_CASE_ABORT(pctenc_decode_invalid_blob1)
{
    pctenc_decode(NULL);
}

TEST_CASE_ABORT(pctenc_decode_invalid_blob2)
{
    pctenc_decode((str_ct)&not_a_str);
}

TEST_CASE(pctenc_decode_empty)
{
    test_ptr_error(pctenc_decode(LIT("")), E_PCTENC_EMPTY);
}

TEST_CASE(pctenc_decode_invalid_data)
{
    test_ptr_error(pctenc_decode(LIT("foo\nbar")), E_PCTENC_INVALID_DATA);
}

TEST_CASE(pctenc_decode_incomplete_hex1)
{
    test_ptr_error(pctenc_decode(LIT("foo%")), E_PCTENC_INVALID_DATA);
}

TEST_CASE(pctenc_decode_incomplete_hex2)
{
    test_ptr_error(pctenc_decode(LIT("foo%A")), E_PCTENC_INVALID_DATA);
}

TEST_CASE(pctenc_decode_invalid_hex1)
{
    test_ptr_error(pctenc_decode(LIT("foo%GA")), E_PCTENC_INVALID_DATA);
}

TEST_CASE(pctenc_decode_invalid_hex2)
{
    test_ptr_error(pctenc_decode(LIT("foo%AG")), E_PCTENC_INVALID_DATA);
}

TEST_CASE_PFIX(pctenc_decode_upper, mktext, no_teardown, true)
{
    test_ptr_success(str = pctenc_decode(STR(text_enc)));
    test_true(str_is_binary(str));
    test_uint_eq(str_len(str), 128);
    test_mem_eq(str_bc(str), text_plain, 128);
    str_unref(str);
}

TEST_CASE_PFIX(pctenc_decode_lower, mktext, no_teardown, false)
{
    test_ptr_success(str = pctenc_decode(STR(text_enc)));
    test_true(str_is_binary(str));
    test_uint_eq(str_len(str), 128);
    test_mem_eq(str_bc(str), text_plain, 128);
    str_unref(str);
}

TEST_CASE_ABORT(pctenc_is_valid_invalid_blob1)
{
    pctenc_is_valid(NULL);
}

TEST_CASE_ABORT(pctenc_is_valid_invalid_blob2)
{
    pctenc_is_valid((str_ct)&not_a_str);
}

TEST_CASE(pctenc_is_valid_empty)
{
    test_false(pctenc_is_valid(LIT("")));
}

TEST_CASE(pctenc_is_valid_invalid_data)
{
    test_false(pctenc_is_valid(LIT("foo\nbar")));
}

TEST_CASE(pctenc_is_valid_incomplete_hex1)
{
    test_false(pctenc_is_valid(LIT("foo%")));
}

TEST_CASE(pctenc_is_valid_incomplete_hex2)
{
    test_false(pctenc_is_valid(LIT("foo%A")));
}

TEST_CASE(pctenc_is_valid_invalid_hex1)
{
    test_false(pctenc_is_valid(LIT("foo%GA")));
}

TEST_CASE(pctenc_is_valid_invalid_hex2)
{
    test_false(pctenc_is_valid(LIT("foo%AG")));
}

TEST_CASE_PFIX(pctenc_is_valid_upper, mktext, no_teardown, true)
{
    test_true(pctenc_is_valid(STR(text_enc)));
}

TEST_CASE_PFIX(pctenc_is_valid_lower, mktext, no_teardown, false)
{
    test_true(pctenc_is_valid(STR(text_enc)));
}

int test_suite_enc_pctenc(void)
{
    return error_pass_int(test_run_cases("pctenc",
        test_case(pctenc_encode_invalid_blob1),
        test_case(pctenc_encode_invalid_blob2),
        test_case(pctenc_encode_empty),
        test_case(pctenc_encode),

        test_case(pctenc_decode_invalid_blob1),
        test_case(pctenc_decode_invalid_blob2),
        test_case(pctenc_decode_empty),
        test_case(pctenc_decode_invalid_data),
        test_case(pctenc_decode_incomplete_hex1),
        test_case(pctenc_decode_incomplete_hex2),
        test_case(pctenc_decode_invalid_hex1),
        test_case(pctenc_decode_invalid_hex2),
        test_case(pctenc_decode_upper),
        test_case(pctenc_decode_lower),

        test_case(pctenc_is_valid_invalid_blob1),
        test_case(pctenc_is_valid_invalid_blob2),
        test_case(pctenc_is_valid_empty),
        test_case(pctenc_is_valid_invalid_data),
        test_case(pctenc_is_valid_incomplete_hex1),
        test_case(pctenc_is_valid_incomplete_hex2),
        test_case(pctenc_is_valid_invalid_hex1),
        test_case(pctenc_is_valid_invalid_hex2),
        test_case(pctenc_is_valid_upper),
        test_case(pctenc_is_valid_lower),

        NULL
    ));
}
