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

#ifndef YTIL_ENC_BASE85_H_INCLUDED
#define YTIL_ENC_BASE85_H_INCLUDED

#include <stdbool.h>
#include <ytil/gen/str.h>

typedef enum base85_error
{
      E_BASE85_EMPTY
    , E_BASE85_INVALID_ALPHABET
    , E_BASE85_INVALID_DATA
    , E_BASE85_INVALID_COMPRESSION
} base85_error_id;

extern const char base85_alphabet_a85[], *base85_compression_a85;
extern const char base85_alphabet_z85[];


// base85 encode arbitrary data with given alphabet and compression set
str_ct base85_encode(str_const_ct blob, const char *alphabet, const char *compression);
// base85 encode arbitrary data with ascii85 alphabet and zero compression
str_ct base85_encode_a85(str_const_ct blob);
// base85 encode arbitrary data with z85 alphabet
str_ct base85_encode_z85(str_const_ct blob);

// decode base85 data with given alphabet and compression set
str_ct base85_decode(str_const_ct str, const char *alphabet, const char *compression);
// decode base85 data with ascii85 alphabet and zero decompression
str_ct base85_decode_a85(str_const_ct str);
// decode base85 data with z85 alphabet
str_ct base85_decode_z85(str_const_ct str);

// check validity of base85 encoded data with given alphabet and compression set
bool base85_is_valid(str_const_ct str, const char *alphabet, const char *compression);
// check validity of base85 encoded data with ascii85 alphabet and zero compression
bool base85_is_valid_a85(str_const_ct str);
// check validity of base85 encoded data with z85 alphabet
bool base85_is_valid_z85(str_const_ct str);

#endif
