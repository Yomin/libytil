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

#include <ytil/enc/qpenc.h>
#include <ytil/def.h>
#include <ytil/ext/string.h>
#include <ytil/ext/stdlib.h>
#include <stdio.h>


static const error_info_st error_infos[] =
{
      ERROR_INFO(E_QPENC_EMPTY, "No input data available.")
    , ERROR_INFO(E_QPENC_INVALID_DATA, "Invalid quoted printable data.")
};


static ssize_t qpenc_translate_encode(unsigned char *dst, size_t *written, const unsigned char *src, size_t *read, ssize_t len, bool null_stop)
{
    return_value_if_fail(len, 0);
    
    if((RANGE(src[0], '!', '~') && src[0] != '=')
    || ((src[0] == ' ' || src[0] == '\t') && len > 1))
    {
        if(dst)
            dst[0] = src[0];
        
        *written += 1;
    }
    else
    {
        if(dst)
            snprintf((char*)&dst[0], 4, "=%02hhX", src[0]);
        
        *written += 3;
    }
    
    *read += 1;
    
    return 1;
}

str_ct qpenc_encode(str_const_ct blob)
{
    const unsigned char *src = str_buc(blob);
    size_t src_len = str_len(blob);
    ssize_t dst_len;
    str_ct dst;
    
    return_error_if_fail(src_len, E_QPENC_EMPTY, NULL);
    
    dst_len = memtranslate(NULL, src, src_len, qpenc_translate_encode);
    
    if(!(dst = str_prepare(dst_len)))
        return error_wrap(), NULL;
    
    memtranslate(str_buw(dst), src, src_len, qpenc_translate_encode);
    
    return dst;
}

static ssize_t qpenc_translate_decode(unsigned char *dst, size_t *written, const unsigned char *src, size_t *read, ssize_t len, bool null_stop)
{
    unsigned long int val;
    
    return_value_if_fail(len, 0);
    
    if(src[0] == '=')
    {
        if(len < 3 || !isuxdigit(src[1]) || !isuxdigit(src[2]))
            return -1;
        
        if(dst)
        {
            strn2ul(&val, (char*)&src[1], 2, 16);
            dst[0] = val;
        }
        
        *read += 3;
    }
    else if(RANGE(src[0], '!', '~')
    || ((src[0] == ' ' || src[0] == '\t') && len > 1))
    {
        if(dst)
            dst[0] = src[0];
        
        *read += 1;
    }
    else
        return -1;
    
    *written += 1;
    
    return 1;
}

str_ct qpenc_decode(str_const_ct str)
{
    const unsigned char *src = str_buc(str);
    size_t src_len = str_len(str);
    ssize_t dst_len;
    str_ct dst;
    
    return_error_if_fail(src_len, E_QPENC_EMPTY, NULL);
    
    if((dst_len = memtranslate(NULL, src, src_len, qpenc_translate_decode)) < 0)
        return error_set(E_QPENC_INVALID_DATA), NULL;
    
    if(!(dst = str_prepare_b(dst_len)))
        return error_wrap(), NULL;
    
    memtranslate(str_buw(dst), src, src_len, qpenc_translate_decode);
    
    return dst;
}

bool qpenc_is_valid(str_const_ct str)
{
    const char *s = str_bc(str);
    size_t len = str_len(str);
    
    return_value_if_fail(len, false);
    
    while(len)
    {
        if(s[0] == '=')
        {
            if(len < 3 || !isuxdigit(s[1]) || !isuxdigit(s[2]))
                return false;
            
            len -= 3;
            s += 3;
        }
        else if(RANGE(s[0], '!', '~')
        || ((s[0] == ' ' || s[0] == '\t') && len > 1))
        {
            len--;
            s++;
        }
        else
            return false;
    }
    
    return true;
}
