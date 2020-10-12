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

#include <ytil/enc/base85.h>
#include <ytil/def.h>
#include <ytil/bits.h>
#include <ytil/ext/string.h>
#include <stdint.h>


/// base85 error type definition
ERROR_DEFINE_LIST(BASE85,
      ERROR_INFO(E_BASE85_EMPTY, "No input data available.")
    , ERROR_INFO(E_BASE85_INVALID_ALPHABET, "Invalid base85 alphabet.")
    , ERROR_INFO(E_BASE85_INVALID_DATA, "Invalid base85 data.")
    , ERROR_INFO(E_BASE85_INVALID_COMPRESSION, "Invalid base85 compression set.")
);

/// default error type for base85 module
#define ERROR_TYPE_DEFAULT ERROR_TYPE_BASE85

const char base85_alphabet_a85[] =
    "!\"#$%&'()*"   "+,-./01234"    "56789:;<=>"
    "?@ABCDEFGH"    "IJKLMNOPQR"    "STUVWXYZ[\\"
    "]^_`abcdef"    "ghijklmnop"    "qrstu";
const char base85_alphabet_z85[] =
    "0123456789"    "abcdefghij"    "klmnopqrst"
    "uvwxyzABCD"    "EFGHIJKLMN"    "OPQRSTUVWX"
    "YZ.-:+=^!/"    "*?&<>()[]{"    "}@%$#";

const char *base85_compression_a85 = "z\0";

#define P1  85
#define P2  (P1*P1)
#define P3  (P2*P1)
#define P4  (P3*P1)


static bool base85_atab(unsigned char *atab, const char *alphabet)
{
    const unsigned char *ptr, *base = (const unsigned char*)alphabet;
    
    memset(atab, 0xff, 0xff);
    
    for(ptr=base; ptr[0]; ptr++)
        if(atab[ptr[0]] != 0xff)
            return false;
        else
            atab[ptr[0]] = ptr - base;
    
    return ptr - base == 85;
}

static bool base85_ctab(unsigned char *ectab, unsigned char *dctab, const char *compression, const unsigned char *atab)
{
    const unsigned char *ptr, *base = (const unsigned char*)compression;
    
    return_value_if_fail(compression, true);
    
    memset(ectab, 0x00, 0xff);
    memset(dctab, 0x00, 0xff); // not necessary, initialize to make valgrind happy
    
    for(ptr=base; ptr[0]; ptr += 2)
        if(atab[ptr[0]] != 0xff || ectab[ptr[1]] || ectab[dctab[ptr[0]]] == ptr[0])
        {
            return false;
        }
        else
        {
            ectab[ptr[1]] = ptr[0];
            dctab[ptr[0]] = ptr[1];
        }
    
    return true;
}

str_ct base85_encode(str_const_ct blob, const char *alphabet, const char *compression)
{
    const unsigned char *src = str_buc(blob);
    unsigned char atab[0xff], ectab[0xff], dctab[0xff];
    size_t len = str_len(blob);
    str_ct str;
    char *dst;
    uint32_t value;
    
    assert(alphabet);
    return_error_if_fail(base85_atab(atab, alphabet), E_BASE85_INVALID_ALPHABET, NULL);
    return_error_if_fail(base85_ctab(ectab, dctab, compression, atab), E_BASE85_INVALID_COMPRESSION, NULL);
    return_error_if_fail(len, E_BASE85_EMPTY, NULL);
    
    if(!(str = str_prepare((len+3) / 4 * 5)))
        return error_wrap(), NULL;
    
    for(dst=str_w(str); len >= 4; len-=4, src+=4)
    {
        if(compression && ectab[src[0]]
        && src[1] == src[0] && src[2] == src[0] && src[3] == src[0])
        {
            dst[0] = ectab[src[0]];
            dst++;
        }
        else
        {
            value = be32toh(*(uint32_t*)src);
            
            dst[0] = alphabet[value/P4];    value %= P4;
            dst[1] = alphabet[value/P3];    value %= P3;
            dst[2] = alphabet[value/P2];    value %= P2;
            dst[3] = alphabet[value/P1];    value %= P1;
            dst[4] = alphabet[value];
            dst += 5;
        }
    }
    
    if(len)
    {
        value = 0;
        memcpy(&value, src, len);
        value = htobe32(value);
        
        dst[0] = alphabet[value/P4];    value %= P4;
        dst[1] = alphabet[value/P3];    value %= P3;
        dst[2] = dst[3] = dst[4] = '\0';
        
        if(len > 1)
        {
            dst[2] = alphabet[value/P2];
            
            if(len > 2)
                dst[3] = alphabet[(value%P2)/P1];
        }
    }
    
    str_update(str);
    str_truncate(str);
    
    return str;
}

str_ct base85_encode_a85(str_const_ct blob)
{
    return error_pass_ptr(base85_encode(blob, base85_alphabet_a85, base85_compression_a85));
}

str_ct base85_encode_z85(str_const_ct blob)
{
    return error_pass_ptr(base85_encode(blob, base85_alphabet_z85, NULL));
}

str_ct base85_decode(str_const_ct str, const char *alphabet, const char *compression)
{
    unsigned char *dst, atab[0xff], ectab[0xff], dctab[0xff];
    const unsigned char *src = str_buc(str);
    size_t len = str_len(str), compr = 0, rem;
    const char *ptr;
    str_ct blob;
    uint32_t value;
    
    assert(alphabet);
    return_error_if_fail(base85_atab(atab, alphabet), E_BASE85_INVALID_ALPHABET, NULL);
    return_error_if_fail(base85_ctab(ectab, dctab, compression, atab), E_BASE85_INVALID_COMPRESSION, NULL);
    return_error_if_fail(len, E_BASE85_EMPTY, NULL);

    if(compression)
        for(ptr = compression; ptr[0]; ptr += 2)
            compr += memcnt(src, len, ptr[0]);
    
    rem = (len-compr) % 5;
    return_error_if_pass(rem == 1, E_BASE85_INVALID_DATA, NULL);
    
    if(!(blob = str_prepare_b(((len - compr)/5 + compr)*4 + (rem ? rem-1 : 0))))
        return error_wrap(), NULL;
    
    for(dst=str_buw(blob); len; dst+=4)
    {
        if(compression && ectab[dctab[src[0]]] == src[0])
        {
            memset(dst, dctab[src[0]], 4);
            len--;
            src++;
        }
        else if(atab[src[0]] == 0xff || atab[src[1]] == 0xff
        || (len > 2 && atab[src[2]] == 0xff)
        || (len > 3 && atab[src[3]] == 0xff)
        || (len > 4 && atab[src[4]] == 0xff))
        {
            return error_set(E_BASE85_INVALID_DATA), str_unref(blob), NULL;
        }
        else
        {
            value = htobe32(atab[src[0]] * P4 + atab[src[1]] * P3
                + (len > 2 ? atab[src[2]] : 84) * P2
                + (len > 3 ? atab[src[3]] : 84) * P1
                + (len > 4 ? atab[src[4]] : 84));
            
            if(len < 5)
            {
                memcpy(dst, &value, len-1);
                break;
            }
            else
            {
                *(uint32_t*)dst = value;
                len -= 5;
                src += 5;
            }
        }
    }
    
    return blob;
}

str_ct base85_decode_a85(str_const_ct str)
{
    return error_pass_ptr(base85_decode(str, base85_alphabet_a85, base85_compression_a85));
}

str_ct base85_decode_z85(str_const_ct str)
{
    return error_pass_ptr(base85_decode(str, base85_alphabet_z85, NULL));
}

bool base85_is_valid(str_const_ct str, const char *alphabet, const char *compression)
{
    unsigned char atab[0xff], ectab[0xff], dctab[0xff];
    const unsigned char *s = str_buc(str);
    size_t len = str_len(str);
    
    assert(alphabet);
    return_value_if_pass(str_is_empty(str), false);
    
    if(!base85_atab(atab, alphabet) || !base85_ctab(ectab, dctab, compression, atab))
        abort();
    
    while(len)
    {
        if(compression && ectab[dctab[s[0]]] == s[0])
        {
            s++;
            len--;
        }
        else if(len == 1
        || atab[s[0]] == 0xff || atab[s[1]] == 0xff
        || (len > 2 && atab[s[2]] == 0xff)
        || (len > 3 && atab[s[3]] == 0xff)
        || (len > 4 && atab[s[4]] == 0xff))
        {
            return false;
        }
        else if(len < 5)
        {
            break;
        }
        else
        {
            s += 5;
            len -= 5;
        }
    }
    
    return true;
}

bool base85_is_valid_a85(str_const_ct str)
{
    return base85_is_valid(str, base85_alphabet_a85, base85_compression_a85);
}

bool base85_is_valid_z85(str_const_ct str)
{
    return base85_is_valid(str, base85_alphabet_z85, NULL);
}
