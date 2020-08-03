/*
 * Copyright (c) 2018-2020 Martin Rödel a.k.a. Yomin Nimoy
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

#ifndef YTIL_ENC_BASE64_H_INCLUDED
#define YTIL_ENC_BASE64_H_INCLUDED

#include <stdbool.h>
#include <ytil/gen/str.h>

typedef enum base64_error
{
      E_BASE64_EMPTY
    , E_BASE64_INVALID_ALPHABET
    , E_BASE64_INVALID_DATA
    , E_BASE64_INVALID_PAD
} base64_error_id;

extern const char base64_alphabet_std[], base64_pad_std;
extern const char base64_alphabet_url[], base64_pad_url;


// base64 encode arbitrary data with given alphabet and padding character
str_ct base64_encode(str_const_ct blob, const char *alphabet, char pad);
// base64 encode arbitrary data with standard alphabet and padding character
str_ct base64_encode_std(str_const_ct blob);
// base64 encode arbitrary data with url alphabet and padding character
str_ct base64_encode_url(str_const_ct blob);

// decode base64 data with given alphabet and padding character
str_ct base64_decode(str_const_ct str, const char *alphabet, char pad);
// decode base64 data with standard alphabet and padding character
str_ct base64_decode_std(str_const_ct str);
// decode base64 data with url alphabet and padding character
str_ct base64_decode_url(str_const_ct str);

// check validity of base64 encoded data with given alphabet and padding character
bool base64_is_valid(str_const_ct str, const char *alphabet, char pad);
// check validity of base64 encoded data with standard alphabet and padding character
bool base64_is_valid_std(str_const_ct str);
// check validity of base64 encoded data with url alphabet and padding character
bool base64_is_valid_url(str_const_ct str);

#endif
