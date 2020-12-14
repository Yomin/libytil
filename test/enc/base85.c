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
#include <ytil/enc/base85.h>

static const struct not_a_str
{
    int foo;
} not_a_str = { 123 };

static const unsigned char raw[] =
    "\x00\x09\x98\x62\x0f\xc7\x99\x43\x1f\x85\x9a\x24\x2f\x43\x9b\x05"
    "\x3f\x01\x9b\xe6\x4e\xbf\x9c\xc7\x5e\x7d\x9d\xa8\x6e\x3b\x9e\x89"
    "\x7d\xf9\x9f\x6a\x8d\xb7\xa0\x4b\x9d\x75\xa1\x2c\xad\x33\xa2\x0d"
    "\xbc\xf1\xa2\xee\xcc\xaf\xa3\xcf\xdc\x6d\xa4\xb0\xec\x2b\xa5\x91"
    "\xfb\xe9\xa6\x72";

static str_ct str, blob;


TEST_CASE_ABORT(base85_encode_invalid_alphabet_null)
{
    base85_encode(BIN("foo"), NULL, NULL);
}

TEST_CASE(base85_encode_invalid_alphabet_insufficient)
{
    test_ptr_error(base85_encode(BIN("foo"), "123", NULL), E_BASE85_INVALID_ALPHABET);
}

TEST_CASE(base85_encode_invalid_alphabet_duplicates)
{
    char alphabet[86];

    memset(alphabet, 'a', 85);
    alphabet[85] = '\0';

    test_ptr_error(base85_encode(BIN("foo"), alphabet, NULL), E_BASE85_INVALID_ALPHABET);
}

TEST_CASE(base85_encode_invalid_compression_in_alphabet)
{
    test_ptr_error(base85_encode(BIN("foo"), base85_alphabet_a85, "a\0"), E_BASE85_INVALID_COMPRESSION);
}

TEST_CASE(base85_encode_invalid_compression_duplicates1)
{
    test_ptr_error(base85_encode(BIN("foo"), base85_alphabet_a85, "z\x01z\x02"), E_BASE85_INVALID_COMPRESSION);
}

TEST_CASE(base85_encode_invalid_compression_duplicates2)
{
    test_ptr_error(base85_encode(BIN("foo"), base85_alphabet_a85, "y\x01z\x01"), E_BASE85_INVALID_COMPRESSION);
}

TEST_CASE_ABORT(base85_encode_invalid_blob1)
{
    base85_encode(NULL, base85_alphabet_a85, NULL);
}

TEST_CASE_ABORT(base85_encode_invalid_blob2)
{
    base85_encode((str_ct)&not_a_str, base85_alphabet_a85, NULL);
}

TEST_CASE(base85_encode_empty)
{
    test_ptr_error(base85_encode(BIN(""), base85_alphabet_a85, NULL), E_BASE85_EMPTY);
}

TEST_CASE(base85_encode_a85_full)
{
    test_ptr_success(str = base85_encode_a85(BIN(raw)));
    test_false(str_is_binary(str));
    test_uint_eq(str_len(str), 85);
    test_str_eq(str_c(str), base85_alphabet_a85);
    str_unref(str);
}

TEST_CASE(base85_encode_z85_full)
{
    test_ptr_success(str = base85_encode_z85(BIN(raw)));
    test_false(str_is_binary(str));
    test_uint_eq(str_len(str), 85);
    test_str_eq(str_c(str), base85_alphabet_z85);
    str_unref(str);
}

TEST_CASE(base85_encode_a85_1)
{
    test_ptr_success(str = base85_encode_a85(BIN("1")));
    test_uint_eq(str_len(str), 2);
    test_str_eq(str_c(str), "0`");
    str_unref(str);
}

TEST_CASE(base85_encode_a85_2)
{
    test_ptr_success(str = base85_encode_a85(BIN("12")));
    test_uint_eq(str_len(str), 3);
    test_str_eq(str_c(str), "0er");
    str_unref(str);
}

TEST_CASE(base85_encode_a85_3)
{
    test_ptr_success(str = base85_encode_a85(BIN("123")));
    test_uint_eq(str_len(str), 4);
    test_str_eq(str_c(str), "0etN");
    str_unref(str);
}

TEST_CASE(base85_encode_a85_zero_1)
{
    test_ptr_success(str = base85_encode_a85(BIN("\0\0\0\0""1234")));
    test_uint_eq(str_len(str), 6);
    test_str_eq(str_c(str), "z0etOA");
    str_unref(str);
}

TEST_CASE(base85_encode_a85_zero_2)
{
    test_ptr_success(str = base85_encode_a85(BIN("1\0\0\0\0""234")));
    test_uint_eq(str_len(str), 10);
    test_str_eq(str_c(str), "0`V1R!&?>e");
    str_unref(str);
}

TEST_CASE(base85_encode_a85_zero_3)
{
    test_ptr_success(str = base85_encode_a85(BIN("12\0\0\0\0""34")));
    test_uint_eq(str_len(str), 10);
    test_str_eq(str_c(str), "0er_/!!\"f3");
    str_unref(str);
}

TEST_CASE(base85_encode_a85_zero_4)
{
    test_ptr_success(str = base85_encode_a85(BIN("123\0\0\0\0""4")));
    test_uint_eq(str_len(str), 10);
    test_str_eq(str_c(str), "0etNb!!!!U");
    str_unref(str);
}

TEST_CASE(base85_encode_a85_zero_5)
{
    test_ptr_success(str = base85_encode_a85(BIN("1234\0\0\0\0")));
    test_uint_eq(str_len(str), 6);
    test_str_eq(str_c(str), "0etOAz");
    str_unref(str);
}

TEST_CASE(base85_encode_compression)
{
    test_ptr_success(str = base85_encode(
        BIN("\x00\x00\x00\x00\x01\x01\x01\x01\x02\x02\x02\x02"),
        base85_alphabet_a85, "x\x00y\x01z\x02"));
    test_uint_eq(str_len(str), 3);
    test_str_eq(str_c(str), "xyz");
    str_unref(str);
}

TEST_CASE_ABORT(base85_decode_invalid_alphabet_null)
{
    base85_decode(LIT("foo"), NULL, NULL);
}

TEST_CASE(base85_decode_invalid_alphabet_insufficient)
{
    test_ptr_error(base85_decode(LIT("foo"), "123", NULL), E_BASE85_INVALID_ALPHABET);
}

TEST_CASE(base85_decode_invalid_alphabet_duplicates)
{
    char alphabet[86];

    memset(alphabet, 'a', 85);
    alphabet[85] = '\0';

    test_ptr_error(base85_decode(LIT("foo"), alphabet, NULL), E_BASE85_INVALID_ALPHABET);
}

TEST_CASE(base85_decode_invalid_compression_in_alphabet)
{
    test_ptr_error(base85_decode(LIT("foo"), base85_alphabet_a85, "a\0"), E_BASE85_INVALID_COMPRESSION);
}

TEST_CASE(base85_decode_invalid_compression_duplicates1)
{
    test_ptr_error(base85_decode(LIT("foo"), base85_alphabet_a85, "z\x01z\x02"), E_BASE85_INVALID_COMPRESSION);
}

TEST_CASE(base85_decode_invalid_compression_duplicates2)
{
    test_ptr_error(base85_decode(LIT("foo"), base85_alphabet_a85, "y\x01z\x01"), E_BASE85_INVALID_COMPRESSION);
}

TEST_CASE_ABORT(base85_decode_invalid_str1)
{
    base85_decode(NULL, base85_alphabet_a85, NULL);
}

TEST_CASE_ABORT(base85_decode_invalid_str2)
{
    base85_decode((str_ct)&not_a_str, base85_alphabet_a85, NULL);
}

TEST_CASE(base85_decode_invalid_len)
{
    test_ptr_error(base85_decode(LIT("a"), base85_alphabet_a85, NULL), E_BASE85_INVALID_DATA);
}

TEST_CASE(base85_decode_empty)
{
    test_ptr_error(base85_decode(LIT(""), base85_alphabet_a85, NULL), E_BASE85_EMPTY);
}

TEST_CASE(base85_decode_a85_invalid_b85_11)
{
    test_ptr_error(base85_decode_a85(LIT("{aaa")), E_BASE85_INVALID_DATA);
}

TEST_CASE(base85_decode_a85_invalid_b85_12)
{
    test_ptr_error(base85_decode_a85(LIT("{aaaabb")), E_BASE85_INVALID_DATA);
}

TEST_CASE(base85_decode_a85_invalid_b85_21)
{
    test_ptr_error(base85_decode_a85(LIT("a{aa")), E_BASE85_INVALID_DATA);
}

TEST_CASE(base85_decode_a85_invalid_b85_22)
{
    test_ptr_error(base85_decode_a85(LIT("a{aaabb")), E_BASE85_INVALID_DATA);
}

TEST_CASE(base85_decode_a85_invalid_b85_31)
{
    test_ptr_error(base85_decode_a85(LIT("aa{a")), E_BASE85_INVALID_DATA);
}

TEST_CASE(base85_decode_a85_invalid_b85_32)
{
    test_ptr_error(base85_decode_a85(LIT("aa{aabb")), E_BASE85_INVALID_DATA);
}

TEST_CASE(base85_decode_a85_invalid_b85_41)
{
    test_ptr_error(base85_decode_a85(LIT("aaa{")), E_BASE85_INVALID_DATA);
}

TEST_CASE(base85_decode_a85_invalid_b85_42)
{
    test_ptr_error(base85_decode_a85(LIT("aaa{abb")), E_BASE85_INVALID_DATA);
}

TEST_CASE(base85_decode_a85_invalid_b85_5)
{
    test_ptr_error(base85_decode_a85(LIT("aaaa{bb")), E_BASE85_INVALID_DATA);
}

TEST_CASE(base85_decode_a85_invalid_compression_11)
{
    test_ptr_error(base85_decode_a85(LIT("azaa")), E_BASE85_INVALID_DATA);
}

TEST_CASE(base85_decode_a85_invalid_compression_12)
{
    test_ptr_error(base85_decode_a85(LIT("azaaabb")), E_BASE85_INVALID_DATA);
}

TEST_CASE(base85_decode_a85_invalid_compression_21)
{
    test_ptr_error(base85_decode_a85(LIT("aaza")), E_BASE85_INVALID_DATA);
}

TEST_CASE(base85_decode_a85_invalid_compression_22)
{
    test_ptr_error(base85_decode_a85(LIT("aazaabb")), E_BASE85_INVALID_DATA);
}

TEST_CASE(base85_decode_a85_invalid_compression_31)
{
    test_ptr_error(base85_decode_a85(LIT("aaaz")), E_BASE85_INVALID_DATA);
}

TEST_CASE(base85_decode_a85_invalid_compression_32)
{
    test_ptr_error(base85_decode_a85(LIT("aaazabb")), E_BASE85_INVALID_DATA);
}

TEST_CASE(base85_decode_a85_invalid_compression_4)
{
    test_ptr_error(base85_decode_a85(LIT("aaaazbb")), E_BASE85_INVALID_DATA);
}

TEST_CASE(base85_decode_a85_full)
{
    test_ptr_success(blob = base85_decode_a85(STR(base85_alphabet_a85)));
    test_true(str_is_binary(blob));
    test_uint_eq(str_len(blob), sizeof(raw)-1);
    test_mem_eq(str_buc(blob), raw, sizeof(raw)-1);
    str_unref(blob);
}

TEST_CASE(base85_decode_z85_full)
{
    test_ptr_success(blob = base85_decode_z85(STR(base85_alphabet_z85)));
    test_true(str_is_binary(blob));
    test_uint_eq(str_len(blob), sizeof(raw)-1);
    test_mem_eq(str_buc(blob), raw, sizeof(raw)-1);
    str_unref(blob);
}

TEST_CASE(base85_decode_a85_1)
{
    test_ptr_success(blob = base85_decode_a85(LIT("0`")));
    test_uint_eq(str_len(blob), 1);
    test_mem_eq(str_buc(blob), "1", 1);
    str_unref(blob);
}

TEST_CASE(base85_decode_a85_2)
{
    test_ptr_success(blob = base85_decode_a85(LIT("0er")));
    test_uint_eq(str_len(blob), 2);
    test_mem_eq(str_buc(blob), "12", 2);
    str_unref(blob);
}

TEST_CASE(base85_decode_a85_3)
{
    test_ptr_success(blob = base85_decode_a85(LIT("0etN")));
    test_uint_eq(str_len(blob), 3);
    test_mem_eq(str_bc(blob), "123", 3);
    str_unref(blob);
}

TEST_CASE(base85_decode_a85_4)
{
    test_ptr_success(blob = base85_decode_a85(LIT("0etOA")));
    test_uint_eq(str_len(blob), 4);
    test_mem_eq(str_buc(blob), "1234", 4);
    str_unref(blob);
}

TEST_CASE(base85_decode_compression)
{
    test_ptr_success(blob = base85_decode(LIT("xyz"), base85_alphabet_a85, "x\x00y\x01z\x02"));
    test_uint_eq(str_len(blob), 3*4);
    test_mem_eq(str_buc(blob), "\x00\x00\x00\x00\x01\x01\x01\x01\x02\x02\x02\x02", 3*4);
    str_unref(blob);
}

TEST_CASE_ABORT(base85_is_valid_invalid_alphabet_null)
{
    base85_is_valid(LIT("foo"), NULL, NULL);
}

TEST_CASE_ABORT(base85_is_valid_invalid_alphabet_insufficient)
{
    base85_is_valid(LIT("foo"), "123", NULL);
}

TEST_CASE_ABORT(base85_is_valid_invalid_alphabet_duplicates)
{
    char alphabet[86];

    memset(alphabet, 'a', 85);
    alphabet[85] = '\0';

    base85_is_valid(LIT("foo"), alphabet, NULL);
}

TEST_CASE_ABORT(base85_is_valid_invalid_compression_in_alphabet)
{
    base85_is_valid(LIT("foo"), base85_alphabet_a85, "a\0");
}

TEST_CASE_ABORT(base85_is_valid_invalid_compression_duplicates1)
{
    base85_is_valid(LIT("foo"), base85_alphabet_a85, "z\x01z\x02");
}

TEST_CASE_ABORT(base85_is_valid_invalid_compression_duplicates2)
{
    base85_is_valid(LIT("foo"), base85_alphabet_a85, "y\x01z\x01");
}

TEST_CASE_ABORT(base85_is_valid_invalid_str1)
{
    base85_is_valid(NULL, base85_alphabet_a85, NULL);
}

TEST_CASE_ABORT(base85_is_valid_invalid_str2)
{
    base85_is_valid((str_ct)&not_a_str, base85_alphabet_a85, NULL);
}

TEST_CASE(base85_is_valid_invalid_len)
{
    test_false(base85_is_valid(LIT("a"), base85_alphabet_a85, NULL));
}

TEST_CASE(base85_is_valid_empty)
{
    test_false(base85_is_valid(LIT(""), base85_alphabet_a85, NULL));
}

TEST_CASE(base85_is_valid_a85_invalid_b85_11)
{
    test_false(base85_is_valid_a85(LIT("{aaa")));
}

TEST_CASE(base85_is_valid_a85_invalid_b85_12)
{
    test_false(base85_is_valid_a85(LIT("{aaaabb")));
}

TEST_CASE(base85_is_valid_a85_invalid_b85_21)
{
    test_false(base85_is_valid_a85(LIT("a{aa")));
}

TEST_CASE(base85_is_valid_a85_invalid_b85_22)
{
    test_false(base85_is_valid_a85(LIT("a{aaabb")));
}

TEST_CASE(base85_is_valid_a85_invalid_b85_31)
{
    test_false(base85_is_valid_a85(LIT("aa{a")));
}

TEST_CASE(base85_is_valid_a85_invalid_b85_32)
{
    test_false(base85_is_valid_a85(LIT("aa{aabb")));
}

TEST_CASE(base85_is_valid_a85_invalid_b85_41)
{
    test_false(base85_is_valid_a85(LIT("aaa{a")));
}

TEST_CASE(base85_is_valid_a85_invalid_b85_42)
{
    test_false(base85_is_valid_a85(LIT("aaa{abb")));
}

TEST_CASE(base85_is_valid_a85_invalid_b85_5)
{
    test_false(base85_is_valid_a85(LIT("aaaa{bb")));
}

TEST_CASE(base85_is_valid_a85_invalid_compression_11)
{
    test_false(base85_is_valid_a85(LIT("azaa")));
}

TEST_CASE(base85_is_valid_a85_invalid_compression_12)
{
    test_false(base85_is_valid_a85(LIT("azaaabb")));
}

TEST_CASE(base85_is_valid_a85_invalid_compression_21)
{
    test_false(base85_is_valid_a85(LIT("aaza")));
}

TEST_CASE(base85_is_valid_a85_invalid_compression_22)
{
    test_false(base85_is_valid_a85(LIT("aazaabb")));
}

TEST_CASE(base85_is_valid_a85_invalid_compression_31)
{
    test_false(base85_is_valid_a85(LIT("aaaz")));
}

TEST_CASE(base85_is_valid_a85_invalid_compression_32)
{
    test_false(base85_is_valid_a85(LIT("aaazabb")));
}

TEST_CASE(base85_is_valid_a85_invalid_compression_4)
{
    test_false(base85_is_valid_a85(LIT("aaaazbb")));
}

TEST_CASE(base85_is_valid_a85)
{
    test_true(base85_is_valid_a85(STR(base85_alphabet_a85)));
}

TEST_CASE(base85_is_valid_z85)
{
    test_true(base85_is_valid_z85(STR(base85_alphabet_z85)));
}

TEST_CASE(base85_is_valid_compression)
{
    test_true(base85_is_valid(LIT("xyz"), base85_alphabet_a85, "x\x00y\x01z\x02"));
}

int test_suite_enc_base85(void *param)
{
    return error_pass_int(test_run_cases("base85",
        test_case(base85_encode_invalid_alphabet_null),
        test_case(base85_encode_invalid_alphabet_insufficient),
        test_case(base85_encode_invalid_alphabet_duplicates),
        test_case(base85_encode_invalid_compression_in_alphabet),
        test_case(base85_encode_invalid_compression_duplicates1),
        test_case(base85_encode_invalid_compression_duplicates2),
        test_case(base85_encode_invalid_blob1),
        test_case(base85_encode_invalid_blob2),
        test_case(base85_encode_empty),
        test_case(base85_encode_a85_full),
        test_case(base85_encode_z85_full),
        test_case(base85_encode_a85_1),
        test_case(base85_encode_a85_2),
        test_case(base85_encode_a85_3),
        test_case(base85_encode_a85_zero_1),
        test_case(base85_encode_a85_zero_2),
        test_case(base85_encode_a85_zero_3),
        test_case(base85_encode_a85_zero_4),
        test_case(base85_encode_a85_zero_5),
        test_case(base85_encode_compression),

        test_case(base85_decode_invalid_alphabet_null),
        test_case(base85_decode_invalid_alphabet_insufficient),
        test_case(base85_decode_invalid_alphabet_duplicates),
        test_case(base85_decode_invalid_compression_in_alphabet),
        test_case(base85_decode_invalid_compression_duplicates1),
        test_case(base85_decode_invalid_compression_duplicates2),
        test_case(base85_decode_invalid_str1),
        test_case(base85_decode_invalid_str2),
        test_case(base85_decode_invalid_len),
        test_case(base85_decode_empty),
        test_case(base85_decode_a85_invalid_b85_11),
        test_case(base85_decode_a85_invalid_b85_12),
        test_case(base85_decode_a85_invalid_b85_21),
        test_case(base85_decode_a85_invalid_b85_22),
        test_case(base85_decode_a85_invalid_b85_31),
        test_case(base85_decode_a85_invalid_b85_32),
        test_case(base85_decode_a85_invalid_b85_41),
        test_case(base85_decode_a85_invalid_b85_42),
        test_case(base85_decode_a85_invalid_b85_5),
        test_case(base85_decode_a85_invalid_compression_11),
        test_case(base85_decode_a85_invalid_compression_12),
        test_case(base85_decode_a85_invalid_compression_21),
        test_case(base85_decode_a85_invalid_compression_22),
        test_case(base85_decode_a85_invalid_compression_31),
        test_case(base85_decode_a85_invalid_compression_32),
        test_case(base85_decode_a85_invalid_compression_4),
        test_case(base85_decode_a85_full),
        test_case(base85_decode_z85_full),
        test_case(base85_decode_a85_1),
        test_case(base85_decode_a85_2),
        test_case(base85_decode_a85_3),
        test_case(base85_decode_a85_4),
        test_case(base85_decode_compression),

        test_case(base85_is_valid_invalid_alphabet_null),
        test_case(base85_is_valid_invalid_alphabet_insufficient),
        test_case(base85_is_valid_invalid_alphabet_duplicates),
        test_case(base85_is_valid_invalid_compression_in_alphabet),
        test_case(base85_is_valid_invalid_compression_duplicates1),
        test_case(base85_is_valid_invalid_compression_duplicates2),
        test_case(base85_is_valid_invalid_str1),
        test_case(base85_is_valid_invalid_str2),
        test_case(base85_is_valid_invalid_len),
        test_case(base85_is_valid_empty),
        test_case(base85_is_valid_a85_invalid_b85_11),
        test_case(base85_is_valid_a85_invalid_b85_12),
        test_case(base85_is_valid_a85_invalid_b85_21),
        test_case(base85_is_valid_a85_invalid_b85_22),
        test_case(base85_is_valid_a85_invalid_b85_31),
        test_case(base85_is_valid_a85_invalid_b85_32),
        test_case(base85_is_valid_a85_invalid_b85_41),
        test_case(base85_is_valid_a85_invalid_b85_42),
        test_case(base85_is_valid_a85_invalid_b85_5),
        test_case(base85_is_valid_a85_invalid_compression_11),
        test_case(base85_is_valid_a85_invalid_compression_12),
        test_case(base85_is_valid_a85_invalid_compression_21),
        test_case(base85_is_valid_a85_invalid_compression_22),
        test_case(base85_is_valid_a85_invalid_compression_31),
        test_case(base85_is_valid_a85_invalid_compression_32),
        test_case(base85_is_valid_a85_invalid_compression_4),
        test_case(base85_is_valid_a85),
        test_case(base85_is_valid_z85),
        test_case(base85_is_valid_compression),

        NULL
    ));
}
