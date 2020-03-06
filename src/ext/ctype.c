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

#include <ytil/ext/ctype.h>
#include <stdio.h>
#include <errno.h>


int isword(int c)
{
    return c == '_' || isalnum(c);
}

int issign(int c)
{
    return c == '+' || c == '-';
}

int isodigit(int c)
{
    return c >= '0' && c <= '7';
}

int isbdigit(int c)
{
    return c == '0' || c == '1';
}

int islxdigit(int c)
{
    return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f');
}

int isuxdigit(int c)
{
    return (c >= '0' && c <= '9') || (c >= 'A' && c <= 'F');
}

int flatten(int c)
{
    return isprint(c) ? c : ' ';
}

ssize_t translate_escape(unsigned char *dst, size_t *written, const unsigned char *src, size_t *read, ssize_t len, bool null_stop)
{
    unsigned char tmp;
    
    if(len < 0 && !null_stop)
        return errno = EINVAL, -1;
    
    if(!len || (null_stop && !src[0]))
        return 0;
    
    *read += 1;
    
    if(isprint(src[0]))
    {
        if(dst)
            dst[0] = src[0];
        
        *written += 1;
        return 1;
    }
    
    switch(src[0])
    {
    case '\0':   tmp = '0'; break;
    case '\a':   tmp = 'a'; break;
    case '\b':   tmp = 'b'; break;
    case '\x1b': tmp = 'e'; break;
    case '\f':   tmp = 'f'; break;
    case '\n':   tmp = 'n'; break;
    case '\r':   tmp = 'r'; break;
    case '\t':   tmp = 't'; break;
    case '\v':   tmp = 'v'; break;
    case '\\':   tmp = '\\'; break;
    default:     tmp = 0; break;
    }
    
    if(dst)
    {
        dst[0] = '\\';
        
        if(tmp)
            dst[1] = tmp;
        else
            snprintf((char*)&dst[1], 4, "x%02hhX", src[0]);
    }
    
    *written += tmp ? 2 : 4;
    
    return 1;
}

ssize_t translate_unescape(unsigned char *dst, size_t *written, const unsigned char *src, size_t *read, ssize_t len, bool null_stop)
{
    unsigned char tmp;
    
    if(len < 0 && !null_stop)
        return errno = EINVAL, -1;
    
    if(!len || (null_stop && !src[0]))
        return 0;
    
    if(src[0] != '\\')
    {
        if(dst)
            dst[0] = src[0];
        
        *read += 1;
        *written += 1;
        return 1;
    }
    
    if(len == 1 || (null_stop && !src[1]))
        return errno = EBADMSG, -1;
    
    switch(src[1])
    {
    case '0':  tmp = '\0'; break;
    case 'a':  tmp = '\a'; break;
    case 'b':  tmp = '\b'; break;
    case 'e':  tmp = '\x1b'; break;
    case 'f':  tmp = '\f'; break;
    case 'n':  tmp = '\n'; break;
    case 'r':  tmp = '\r'; break;
    case 't':  tmp = '\t'; break;
    case 'v':  tmp = '\v'; break;
    case '\\': tmp = '\\'; break;
    default:   tmp = 'x'; break;
    }
    
    if(tmp != 'x')
        *read += 2;
    else if(src[1] == 'x'
    && (len >= 4 || (null_stop && src[2] && src[3]))
    && sscanf((const char*)&src[2], "%02hhx", &tmp) == 1)
        *read += 4;
    else
        return errno = EBADMSG, -1;
    
    if(dst)
        dst[0] = tmp;
    
    *written += 1;
    
    return 1;
}
