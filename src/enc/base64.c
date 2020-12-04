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

#include <ytil/enc/base64.h>
#include <ytil/def.h>
#include <ytil/def/bits.h>
#include <stdlib.h>


/// base64 error type definition
ERROR_DEFINE_LIST(BASE64,
      ERROR_INFO(E_BASE64_EMPTY, "No input data available.")
    , ERROR_INFO(E_BASE64_INVALID_ALPHABET, "Invalid base64 alphabet.")
    , ERROR_INFO(E_BASE64_INVALID_DATA, "Invalid base64 data.")
    , ERROR_INFO(E_BASE64_INVALID_PAD, "Invalid base64 pad character.")
);

/// default error type for base64 module
#define ERROR_TYPE_DEFAULT ERROR_TYPE_BASE64

const char base64_alphabet_std[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "0123456789+/";
const char base64_alphabet_url[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "0123456789-_";
const char base64_pad_std = '=', base64_pad_url = '=';


static bool base64_mktab(unsigned char *tab, const char *alphabet)
{
    const unsigned char *ptr, *base = (const unsigned char*)alphabet;
    
    memset(tab, 0xff, 0xff);
    
    for(ptr=base; ptr[0]; ptr++)
        tab[ptr[0]] = ptr - base;
    
    return ptr - base == 64;
}

str_ct base64_encode(str_const_ct blob, const char *alphabet, char pad)
{
    const unsigned char *src = str_buc(blob);
    unsigned char tab[0xff];
    size_t len = str_len(blob);
    str_ct str;
    char *dst;
    
    assert(alphabet);
    return_error_if_fail(base64_mktab(tab, alphabet), E_BASE64_INVALID_ALPHABET, NULL);
    return_error_if_pass(tab[(unsigned char)pad] != 0xff, E_BASE64_INVALID_PAD, NULL);
    return_error_if_fail(len, E_BASE64_EMPTY, NULL);
    
    if(!(str = str_prepare((len+2) / 3 * 4)))
        return error_wrap(), NULL;
    
    for(dst=str_w(str); len >= 3; len-=3, src+=3, dst+=4)
    {
        dst[0] = alphabet[BMG(src[0], BM(6U), 2)];
        dst[1] = alphabet[BMV(BMG(src[0], BM(2U), 0), 4) | BMG(src[1], BM(4U), 4)];
        dst[2] = alphabet[BMV(BMG(src[1], BM(4U), 0), 2) | BMG(src[2], BM(2U), 6)];
        dst[3] = alphabet[BMG(src[2], BM(6U), 0)];
    }
    
    if(len)
    {
        dst[0] = alphabet[BMG(src[0], BM(6U), 2)];
        dst[1] = alphabet[BMV(BMG(src[0], BM(2U), 0), 4) | (len == 2 ? BMG(src[1], BM(4U), 4) : 0)];
        dst[2] = len == 2 ? alphabet[BMV(BMG(src[1], BM(4U), 0), 2)] : pad;
        dst[3] = pad;
    }
    
    return str;
}

str_ct base64_encode_std(str_const_ct blob)
{
    return error_pass_ptr(base64_encode(blob, base64_alphabet_std, base64_pad_std));
}

str_ct base64_encode_url(str_const_ct blob)
{
    return error_pass_ptr(base64_encode(blob, base64_alphabet_url, base64_pad_url));
}

str_ct base64_decode(str_const_ct str, const char *alphabet, char pad)
{
    unsigned char *dst, tab[0xff];
    const unsigned char *src = str_buc(str);
    size_t len = str_len(str), rem;
    str_ct blob;
    
    assert(alphabet);
    return_error_if_fail(base64_mktab(tab, alphabet), E_BASE64_INVALID_ALPHABET, NULL);
    return_error_if_pass(tab[(unsigned char)pad] != 0xff, E_BASE64_INVALID_PAD, NULL);
    return_error_if_fail(len, E_BASE64_EMPTY, NULL);
    
    if(!(rem = len % 4))
    {
        if(src[len-1] == pad) { len--; rem = 3; }
        if(src[len-1] == pad) { len--; rem = 2; }
    }
    
    return_error_if_pass(rem == 1, E_BASE64_INVALID_DATA, NULL);
    
    if(!(blob = str_prepare_b(len/4*3 + (rem ? rem-1 : 0))))
        return error_wrap(), NULL;
    
    for(dst=str_buw(blob); len >= 4; len-=4, dst+=3, src+=4)
    {
        if(tab[src[0]] == 0xff || tab[src[1]] == 0xff
        || tab[src[2]] == 0xff || tab[src[3]] == 0xff)
            return error_set(E_BASE64_INVALID_DATA), str_unref(blob), NULL;
        
        dst[0] = BMV(BMG(tab[src[0]], BM(6U), 0), 2) | BMG(tab[src[1]], BM(2U), 4);
        dst[1] = BMV(BMG(tab[src[1]], BM(4U), 0), 4) | BMG(tab[src[2]], BM(4U), 2);
        dst[2] = BMV(BMG(tab[src[2]], BM(2U), 0), 6) | BMG(tab[src[3]], BM(6U), 0);
    }
    
    if(len)
    {
        if(tab[src[0]] == 0xff || tab[src[1]] == 0xff
        || (len == 3 && tab[src[2]] == 0xff))
            return error_set(E_BASE64_INVALID_DATA), str_unref(blob), NULL;
        
        dst[0] = BMV(BMG(tab[src[0]], BM(6U), 0), 2) | BMG(tab[src[1]], BM(2U), 4);
        
        if(len == 3)
            dst[1] = BMV(BMG(tab[src[1]], BM(4U), 0), 4) | BMG(tab[src[2]], BM(4U), 2);
    }
    
    return blob;
}

str_ct base64_decode_std(str_const_ct str)
{
    return error_pass_ptr(base64_decode(str, base64_alphabet_std, base64_pad_std));
}

str_ct base64_decode_url(str_const_ct str)
{
    return error_pass_ptr(base64_decode(str, base64_alphabet_url, base64_pad_url));
}

bool base64_is_valid(str_const_ct str, const char *alphabet, char pad)
{
    unsigned char tab[0xff];
    const unsigned char *s = str_buc(str);
    size_t len = str_len(str);
    
    assert(alphabet);
    return_value_if_fail(len, false);
    
    if(!base64_mktab(tab, alphabet) || tab[(unsigned char)pad] != 0xff)
        abort();
    
    for(; len > 4; s+=4, len-=4)
        if(tab[s[0]] == 0xff || tab[s[1]] == 0xff
        || tab[s[2]] == 0xff || tab[s[3]] == 0xff)
            return false;
    
    if(len == 1 || tab[s[0]] == 0xff || tab[s[1]] == 0xff)
        return false;
    
    switch(len)
    {
    case 2:  return true;
    case 3:  return tab[s[2]] != 0xff;
    case 4:  return s[2] == pad ? s[3] == pad
                  : tab[s[2]] != 0xff && (tab[s[3]] != 0xff || s[3] == pad);
    default: abort();
    }
}

bool base64_is_valid_std(str_const_ct str)
{
    return base64_is_valid(str, base64_alphabet_std, base64_pad_std);
}

bool base64_is_valid_url(str_const_ct str)
{
    return base64_is_valid(str, base64_alphabet_url, base64_pad_url);
}
