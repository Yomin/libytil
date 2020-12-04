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
#include <ytil/enc/base64.h>

static const struct not_a_str
{
    int foo;
} not_a_str = { 123 };

static const char raw[] =
    "\x00\x10\x83\x10\x51\x87\x20\x92\x8b\x30\xd3\x8f\x41\x14\x93\x51"
    "\x55\x97\x61\x96\x9b\x71\xd7\x9f\x82\x18\xa3\x92\x59\xa7\xa2\x9a"
    "\xab\xb2\xdb\xaf\xc3\x1c\xb3\xd3\x5d\xb7\xe3\x9e\xbb\xf3\xdf\xbf";

static str_ct str, blob;


TEST_CASE_ABORT(base64_encode_invalid_alphabet1)
{
    base64_encode(LIT("foo"), NULL, '=');
}

TEST_CASE(base64_encode_invalid_alphabet2)
{
    test_ptr_error(base64_encode(LIT("foo"), "123", '='), E_BASE64_INVALID_ALPHABET);
}

TEST_CASE(base64_encode_invalid_pad)
{
    test_ptr_error(base64_encode(LIT("foo"), base64_alphabet_std, 'a'), E_BASE64_INVALID_PAD);
}

TEST_CASE_ABORT(base64_encode_invalid_blob1)
{
    base64_encode(NULL, base64_alphabet_std, base64_pad_std);
}

TEST_CASE_ABORT(base64_encode_invalid_blob2)
{
    base64_encode((str_ct)&not_a_str, base64_alphabet_std, base64_pad_std);
}

TEST_CASE(base64_encode_empty)
{
    test_ptr_error(base64_encode(LIT(""), base64_alphabet_std, base64_pad_std), E_BASE64_EMPTY);
}

TEST_CASE(base64_encode_std_full)
{
    test_ptr_success(str = base64_encode_std(BIN(raw)));
    test_false(str_is_binary(str));
    test_uint_eq(str_len(str), 64);
    test_str_eq(str_c(str), base64_alphabet_std);
    str_unref(str);
}

TEST_CASE(base64_encode_url_full)
{
    test_ptr_success(str = base64_encode_url(BIN(raw)));
    test_false(str_is_binary(str));
    test_uint_eq(str_len(str), 64);
    test_str_eq(str_c(str), base64_alphabet_url);
    str_unref(str);
}

TEST_CASE(base64_encode_std_1)
{
    test_ptr_success(str = base64_encode_std(LIT("1")));
    test_uint_eq(str_len(str), 4);
    test_str_eq(str_c(str), "MQ==");
    str_unref(str);
}

TEST_CASE(base64_encode_std_2)
{
    test_ptr_success(str = base64_encode_std(LIT("12")));
    test_uint_eq(str_len(str), 4);
    test_str_eq(str_c(str), "MTI=");
    str_unref(str);
}

TEST_CASE(base64_encode_std_3)
{
    test_ptr_success(str = base64_encode_std(LIT("123")));
    test_uint_eq(str_len(str), 4);
    test_str_eq(str_c(str), "MTIz");
    str_unref(str);
}

TEST_CASE_ABORT(base64_decode_invalid_alphabet1)
{
    base64_decode(LIT("foo"), NULL, '=');
}

TEST_CASE(base64_decode_invalid_alphabet2)
{
    test_ptr_error(base64_decode(LIT("foo"), "123", '='), E_BASE64_INVALID_ALPHABET);
}

TEST_CASE(base64_decode_invalid_pad)
{
    test_ptr_error(base64_decode(LIT("foo"), base64_alphabet_std, 'a'), E_BASE64_INVALID_PAD);
}

TEST_CASE_ABORT(base64_decode_invalid_str1)
{
    base64_decode(NULL, base64_alphabet_std, base64_pad_std);
}

TEST_CASE_ABORT(base64_decode_invalid_str2)
{
    base64_decode((str_ct)&not_a_str, base64_alphabet_std, base64_pad_std);
}

TEST_CASE(base64_decode_empty)
{
    test_ptr_error(base64_decode(LIT(""), base64_alphabet_std, base64_pad_std), E_BASE64_EMPTY);
}

TEST_CASE(base64_decode_std_invalid_len)
{
    test_ptr_error(base64_decode_std(LIT("M")), E_BASE64_INVALID_DATA);
}

TEST_CASE(base64_decode_std_invalid_b64_11)
{
    test_ptr_error(base64_decode_std(LIT("!aa")), E_BASE64_INVALID_DATA);
}

TEST_CASE(base64_decode_std_invalid_b64_12)
{
    test_ptr_error(base64_decode_std(LIT("!aaabb")), E_BASE64_INVALID_DATA);
}

TEST_CASE(base64_decode_std_invalid_b64_21)
{
    test_ptr_error(base64_decode_std(LIT("a!a")), E_BASE64_INVALID_DATA);
}

TEST_CASE(base64_decode_std_invalid_b64_22)
{
    test_ptr_error(base64_decode_std(LIT("a!aabb")), E_BASE64_INVALID_DATA);
}

TEST_CASE(base64_decode_std_invalid_b64_31)
{
    test_ptr_error(base64_decode_std(LIT("aa!")), E_BASE64_INVALID_DATA);
}

TEST_CASE(base64_decode_std_invalid_b64_32)
{
    test_ptr_error(base64_decode_std(LIT("aa!abb")), E_BASE64_INVALID_DATA);
}

TEST_CASE(base64_decode_std_invalid_b64_4)
{
    test_ptr_error(base64_decode_std(LIT("aaa!bb")), E_BASE64_INVALID_DATA);
}

TEST_CASE(base64_decode_std_invalid_b64_eq1)
{
    test_ptr_error(base64_decode_std(LIT("aa=")), E_BASE64_INVALID_DATA);
}

TEST_CASE(base64_decode_std_invalid_b64_eq2)
{
    test_ptr_error(base64_decode_std(LIT("a===")), E_BASE64_INVALID_DATA);
}

TEST_CASE(base64_decode_std_invalid_b64_eq3)
{
    test_ptr_error(base64_decode_std(LIT("====")), E_BASE64_INVALID_DATA);
}

TEST_CASE(base64_decode_std_full)
{
    test_ptr_success(blob = base64_decode_std(STR(base64_alphabet_std)));
    test_true(str_is_binary(blob));
    test_uint_eq(str_len(blob), sizeof(raw)-1);
    test_mem_eq(str_buc(blob), raw, sizeof(raw)-1);
    str_unref(blob);
}

TEST_CASE(base64_decode_url_full)
{
    test_ptr_success(blob = base64_decode_url(STR(base64_alphabet_url)));
    test_true(str_is_binary(blob));
    test_uint_eq(str_len(blob), sizeof(raw)-1);
    test_mem_eq(str_buc(blob), raw, sizeof(raw)-1);
    str_unref(blob);
}

TEST_CASE(base64_decode_std_11)
{
    test_ptr_success(blob = base64_decode_std(LIT("MQ")));
    test_uint_eq(str_len(blob), 1);
    test_mem_eq(str_buc(blob), "1", 1);
    str_unref(blob);
}

TEST_CASE(base64_decode_std_12)
{
    test_ptr_success(blob = base64_decode_std(LIT("MQ==")));
    test_uint_eq(str_len(blob), 1);
    test_mem_eq(str_buc(blob), "1", 1);
    str_unref(blob);
}

TEST_CASE(base64_decode_std_21)
{
    test_ptr_success(blob = base64_decode_std(LIT("MTI")));
    test_uint_eq(str_len(blob), 2);
    test_mem_eq(str_buc(blob), "12", 2);
    str_unref(blob);
}

TEST_CASE(base64_decode_std_22)
{
    test_ptr_success(blob = base64_decode_std(LIT("MTI=")));
    test_uint_eq(str_len(blob), 2);
    test_mem_eq(str_buc(blob), "12", 2);
    str_unref(blob);
}

TEST_CASE(base64_decode_std_3)
{
    test_ptr_success(blob = base64_decode_std(LIT("MTIz")));
    test_uint_eq(str_len(blob), 3);
    test_mem_eq(str_buc(blob), "123", 3);
    str_unref(blob);
}

TEST_CASE_ABORT(base64_is_valid_invalid_alphabet1)
{
    base64_is_valid(LIT("foo"), NULL, '=');
}

TEST_CASE_ABORT(base64_is_valid_invalid_alphabet2)
{
    base64_is_valid(LIT("foo"), "123", '=');
}

TEST_CASE_ABORT(base64_is_valid_invalid_pad)
{
    base64_is_valid(LIT("foo"), base64_alphabet_std, 'a');
}

TEST_CASE_ABORT(base64_is_valid_invalid_str1)
{
    base64_is_valid(NULL, base64_alphabet_std, base64_pad_std);
}

TEST_CASE_ABORT(base64_is_valid_invalid_str2)
{
    base64_is_valid((str_ct)&not_a_str, base64_alphabet_std, base64_pad_std);
}

TEST_CASE(base64_is_valid_empty)
{
    test_false(base64_is_valid(LIT(""), base64_alphabet_std, base64_pad_std));
}

TEST_CASE(base64_is_valid_std_invalid_len)
{
    test_false(base64_is_valid_std(LIT("M")));
}

TEST_CASE(base64_is_valid_std_invalid_b64_11)
{
    test_false(base64_is_valid_std(LIT("!aa")));
}

TEST_CASE(base64_is_valid_std_invalid_b64_12)
{
    test_false(base64_is_valid_std(LIT("!aaabb")));
}

TEST_CASE(base64_is_valid_std_invalid_b64_21)
{
    test_false(base64_is_valid_std(LIT("a!a")));
}

TEST_CASE(base64_is_valid_std_invalid_b64_22)
{
    test_false(base64_is_valid_std(LIT("a!aabb")));
}

TEST_CASE(base64_is_valid_std_invalid_b64_31)
{
    test_false(base64_is_valid_std(LIT("aa!")));
}

TEST_CASE(base64_is_valid_std_invalid_b64_32)
{
    test_false(base64_is_valid_std(LIT("aa!abb")));
}

TEST_CASE(base64_is_valid_std_invalid_b64_4)
{
    test_false(base64_is_valid_std(LIT("aaa!bb")));
}

TEST_CASE(base64_is_valid_std_invalid_b64_eq1)
{
    test_false(base64_is_valid_std(LIT("aa=")));
}

TEST_CASE(base64_is_valid_std_invalid_b64_eq2)
{
    test_false(base64_is_valid_std(LIT("a===")));
}

TEST_CASE(base64_is_valid_std_invalid_b64_eq3)
{
    test_false(base64_is_valid_std(LIT("====")));
}

TEST_CASE(base64_is_valid_std)
{
    test_true(base64_is_valid_std(STR(base64_alphabet_std)));
}

TEST_CASE(base64_is_valid_url)
{
    test_true(base64_is_valid_url(STR(base64_alphabet_url)));
}

int test_suite_enc_base64(void)
{
    return error_pass_int(test_run_cases("base64",
        test_case(base64_encode_invalid_alphabet1),
        test_case(base64_encode_invalid_alphabet2),
        test_case(base64_encode_invalid_pad),
        test_case(base64_encode_invalid_blob1),
        test_case(base64_encode_invalid_blob2),
        test_case(base64_encode_empty),
        test_case(base64_encode_std_full),
        test_case(base64_encode_url_full),
        test_case(base64_encode_std_1),
        test_case(base64_encode_std_2),
        test_case(base64_encode_std_3),

        test_case(base64_decode_invalid_alphabet1),
        test_case(base64_decode_invalid_alphabet2),
        test_case(base64_decode_invalid_pad),
        test_case(base64_decode_invalid_str1),
        test_case(base64_decode_invalid_str2),
        test_case(base64_decode_empty),
        test_case(base64_decode_std_invalid_len),
        test_case(base64_decode_std_invalid_b64_11),
        test_case(base64_decode_std_invalid_b64_12),
        test_case(base64_decode_std_invalid_b64_21),
        test_case(base64_decode_std_invalid_b64_22),
        test_case(base64_decode_std_invalid_b64_31),
        test_case(base64_decode_std_invalid_b64_32),
        test_case(base64_decode_std_invalid_b64_4),
        test_case(base64_decode_std_invalid_b64_eq1),
        test_case(base64_decode_std_invalid_b64_eq2),
        test_case(base64_decode_std_invalid_b64_eq3),
        test_case(base64_decode_std_full),
        test_case(base64_decode_url_full),
        test_case(base64_decode_std_11),
        test_case(base64_decode_std_12),
        test_case(base64_decode_std_21),
        test_case(base64_decode_std_22),
        test_case(base64_decode_std_3),

        test_case(base64_is_valid_invalid_alphabet1),
        test_case(base64_is_valid_invalid_alphabet2),
        test_case(base64_is_valid_invalid_pad),
        test_case(base64_is_valid_invalid_str1),
        test_case(base64_is_valid_invalid_str2),
        test_case(base64_is_valid_empty),
        test_case(base64_is_valid_std_invalid_len),
        test_case(base64_is_valid_std_invalid_b64_11),
        test_case(base64_is_valid_std_invalid_b64_12),
        test_case(base64_is_valid_std_invalid_b64_21),
        test_case(base64_is_valid_std_invalid_b64_22),
        test_case(base64_is_valid_std_invalid_b64_31),
        test_case(base64_is_valid_std_invalid_b64_32),
        test_case(base64_is_valid_std_invalid_b64_4),
        test_case(base64_is_valid_std_invalid_b64_eq1),
        test_case(base64_is_valid_std_invalid_b64_eq2),
        test_case(base64_is_valid_std_invalid_b64_eq3),
        test_case(base64_is_valid_std),
        test_case(base64_is_valid_url),

        NULL
    ));
}
