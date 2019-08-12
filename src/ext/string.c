/*
 * Copyright (c) 2017-2018 Martin Rödel a.k.a. Yomin Nimoy
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

#include <ytil/ext/string.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <errno.h>


void *memdup(const void *mem, size_t size)
{
    void *ptr;
    
    if(!(ptr = malloc(size)))
        return NULL;
    
    memcpy(ptr, mem, size);
    
    return ptr;
}

void *mempat(void *vdst, size_t len, const void *vpat, size_t patlen)
{
    const unsigned char *pat = vpat;
    unsigned char *dst = vdst;
    
    if(len < patlen)
    {
        memcpy(dst, pat, len);
        return dst;
    }
    
    memcpy(dst, pat, patlen);
    pat = dst;
    dst += patlen;
    len -= patlen;
    
    while(patlen <= len)
    {
        memcpy(dst, pat, patlen);
        dst += patlen;
        len -= patlen;
        patlen *= 2;
    }
    
    memcpy(dst, pat, len);
    
    return dst;
}

void *memrcpy(void *vdst, const void *vsrc, size_t size)
{
    const unsigned char *src = vsrc;
    char *dst = vdst;
    
    for(dst+=size-1; size; src++,dst--,size--)
        dst[0] = src[0];
    
    return dst+1;
}

char *strnchr(const char *str, int c, size_t n)
{
    for(; str[0] && n && str[0] != c; str++, n--);
    
    return n && str[0] == c ? (char*)str : NULL;
}

char *strpnchr(const char *str, int c, size_t *pn)
{
    size_t n = *pn;
    
    for(; str[0] && n && str[0] != c; str++, n--);
    
    if(!n || str[0] != c)
        return NULL;
    
    *pn = n;
    
    return (char*)str;
}

char *strrpbrk(const char *str, const char *accept)
{
    return memrpbrk(str, strlen(str), accept, strlen(accept));
}

char *strcpbrk(const char *str, const char *reject)
{
    const char *r;
    
    if(!reject[0])
        return str[0] ? (char*)str : NULL;
    
    for(; str[0]; str++)
    {
        for(r=reject; r[0]; r++)
            if(str[0] == r[0])
                break;
        
        if(!r[0])
            return (char*)str;
    }
    
    return NULL;
}

char *strrcpbrk(const char *str, const char *reject)
{
    return memrcpbrk(str, strlen(str), reject, strlen(reject));
}

void *mempbrk(const void *vmem, size_t msize, const void *vaccept, size_t asize)
{
    const unsigned char *mem = vmem, *accept = vaccept;
    const unsigned char *mend = mem + msize, *a, *aend = accept + asize;
    
    if(!asize)
        return NULL;
    
    for(; mem < mend; mem++)
        for(a=accept; a < aend; a++)
            if(mem[0] == a[0])
                return (void*)mem;
    
    return NULL;
}

void *memrpbrk(const void *vmem, size_t msize, const void *vaccept, size_t asize)
{
    const unsigned char *mem = vmem, *accept = vaccept;
    const unsigned char *mend = mem + msize, *a, *aend = accept + asize;
    
    if(!asize)
        return NULL;
    
    for(; mem < mend; mend--)
        for(a=accept; a < aend; a++)
            if(mend[-1] == a[0])
                return (void*)(mend-1);
    
    return NULL;
}

void *memcpbrk(const void *vmem, size_t msize, const void *vreject, size_t rsize)
{
    const unsigned char *mem = vmem, *reject = vreject;
    const unsigned char *mend = mem + msize, *r, *rend = reject + rsize;
    
    if(!rsize)
        return msize ? (void*)mem : NULL;
    
    for(; mem < mend; mem++)
    {
        for(r=reject; r < rend; r++)
            if(mem[0] == r[0])
                break;
        
        if(r == rend)
            return (void*)mem;
    }
    
    return NULL;
}

void *memrcpbrk(const void *vmem, size_t msize, const void *vreject, size_t rsize)
{
    const unsigned char *mem = vmem, *reject = vreject;
    const unsigned char *mend = mem + msize, *r, *rend = reject + rsize;
    
    if(!rsize)
        return msize ? (void*)(mend-1) : NULL;
    
    for(; mem < mend; mend--)
    {
        for(r=reject; r < rend; r++)
            if(mend[-1] == r[0])
                break;
        
        if(r == rend)
            return (void*)(mend-1);
    }
    
    return NULL;
}

char *strwhile(const char *str, ctype_pred_cb pred)
{
    for(; str[0] && pred(str[0]); str++);
    
    return (char*)str;
}

char *strnwhile(const char *str, ctype_pred_cb pred, size_t n)
{
    for(; str[0] && n && pred(str[0]); str++, n--);
    
    return (char*)str;
}

char *strpnwhile(const char *str, ctype_pred_cb pred, size_t *n)
{
    for(; str[0] && *n && pred(str[0]); str++, n[0]--);
    
    return (char*)str;
}

char *strrwhile(const char *str, ctype_pred_cb pred)
{
    const char *end = str + strlen(str) -1;
    
    for(str--; str < end && pred(end[0]); end--);
    
    return str < end ? (char*)end : NULL;
}

char *strnrwhile(const char *str, ctype_pred_cb pred, size_t n)
{
    const char *end = str + strlen(str) -1;
    
    for(str--; str < end && n && pred(end[0]); end--, n--);
    
    return str < end ? (char*)end : NULL;
}

char *struntil(const char *str, ctype_pred_cb pred)
{
    for(; str[0] && !pred(str[0]); str++);
    
    return (char*)str;
}

char *strnuntil(const char *str, ctype_pred_cb pred, size_t n)
{
    for(; str[0] && n && !pred(str[0]); str++, n--);
    
    return (char*)str;
}

char *strpnuntil(const char *str, ctype_pred_cb pred, size_t *n)
{
    for(; str[0] && *n && !pred(str[0]); str++, n[0]--);
    
    return (char*)str;
}

char *strruntil(const char *str, ctype_pred_cb pred)
{
    const char *end = str + strlen(str) -1;
    
    for(str--; str < end && !pred(end[0]); end--);
    
    return str < end ? (char*)end : NULL;
}

char *strnruntil(const char *str, ctype_pred_cb pred, size_t n)
{
    const char *end = str + strlen(str) -1;
    
    for(str--; str < end && n && !pred(end[0]); end--, n--);
    
    return str < end ? (char*)end : NULL;
}

void *memwhile(const void *vmem, size_t size, ctype_pred_cb pred)
{
    const unsigned char *mem = vmem, *end = mem + size;
    
    for(; mem < end && pred(mem[0]); mem++);
    
    return mem < end ? (void*)mem : NULL;
}

void *memrwhile(const void *vmem, size_t size, ctype_pred_cb pred)
{
    const unsigned char *mem = vmem, *end = mem + size -1;
    
    for(mem--; mem < end && pred(end[0]); end--);
    
    return mem < end ? (void*)end : NULL;
}

void *memuntil(const void *vmem, size_t size, ctype_pred_cb pred)
{
    const unsigned char *mem = vmem, *end = mem + size;
    
    for(; mem < end && !pred(mem[0]); mem++);
    
    return mem < end ? (void*)mem : NULL;
}

void *memruntil(const void *vmem, size_t size, ctype_pred_cb pred)
{
    const unsigned char *mem = vmem, *end = mem + size -1;
    
    for(mem--; mem != end && !pred(end[0]); end--);
    
    return mem < end ? (void*)end : NULL;
}

char *strskip(const char *str, int c)
{
    for(; str[0] && str[0] == c; str++);
    
    return (char*)str;
}

char *strrskip(const char *str, int c)
{
    const char *end = str + strlen(str) -1;
    
    for(str--; str < end && end[0] == c; end--);
    
    return str < end ? (char*)end : NULL;
}

void *memskip(const void *vmem, size_t size, int c)
{
    const unsigned char *mem = vmem, *end = mem + size;
    
    for(; mem < end && mem[0] == c; mem++);
    
    return mem < end ? (void*)mem : NULL;
}

void *memrskip(const void *vmem, size_t size, int c)
{
    const unsigned char *mem = vmem, *end = mem + size -1;
    
    for(mem--; mem < end && end[0] == c; end--);
    
    return mem < end ? (void*)end : NULL;
}

size_t strnspn(const char *str, const char *accept, size_t n)
{
    size_t count;
    const char *a;
    
    for(count=0; str[0] && count < n; str++, count++)
    {
        for(a=accept; a[0]; a++)
            if(a[0] == str[0])
                break;
        
        if(!a[0])
            break;
    }
    
    return count;
}

size_t strncspn(const char *str, const char *reject, size_t n)
{
    size_t count;
    const char *r;
    
    for(count=0; str[0] && count < n; str++, count++)
        for(r=reject; r[0]; r++)
            if(r[0] == str[0])
                return count;
    
    return count;
}

size_t memspn(const void *vmem, size_t msize, const void *vaccept, size_t asize)
{
    const unsigned char *mem = vmem, *accept = vaccept;
    const unsigned char *mend = mem + msize, *a, *aend = accept + asize;
    
    if(!asize)
        return 0;
    
    for(; mem < mend; mem++)
    {
        for(a=accept; a < aend; a++)
            if(mem[0] == a[0])
                break;
        
        if(a == aend)
            break;
    }
    
    return mem - (const unsigned char*)vmem;
}

size_t memcspn(const void *vmem, size_t msize, const void *vreject, size_t rsize)
{
    const unsigned char *mem = vmem, *reject = vreject;
    const unsigned char *mend = mem + msize, *r, *rend = reject + rsize;
    
    if(!rsize)
        return msize;
    
    for(; mem < mend; mem++)
        for(r=rend; reject < r; r--)
            if(mem[0] == r[-1])
                return mem - (const unsigned char*)vmem;
    
    return mem - (const unsigned char*)vmem;
}

size_t strprefix(const char *str1, const char *str2)
{
    size_t plen = 0;
    
    while(str1[0] && str2[0] && str1[0] == str2[0])
        plen++, str1++, str2++;
    
    return plen;
}

size_t strcaseprefix(const char *str1, const char *str2)
{
    size_t plen = 0;
    
    while(str1[0] && str2[0] && toupper(str1[0]) == toupper(str2[0]))
        plen++, str1++, str2++;
    
    return plen;
}

size_t strnprefix(const char *str1, const char *str2, size_t len)
{
    size_t plen = 0;
    
    while(len && str1[0] && str2[0] && str1[0] == str2[0])
        len--, plen++, str1++, str2++;
    
    return plen;
}

size_t strncaseprefix(const char *str1, const char *str2, size_t len)
{
    size_t plen = 0;
    
    while(len && str1[0] && str2[0] && toupper(str1[0]) == toupper(str2[0]))
        len--, plen++, str1++, str2++;
    
    return plen;
}

size_t memprefix(const void *vmem1, const void *vmem2, size_t size)
{
    const unsigned char *mem1 = vmem1, *mem2 = vmem2;
    size_t plen = 0;
    
    while(size && mem1[0] == mem2[0])
        size--, plen++, mem1++, mem2++;
    
    return plen;
}

size_t memcaseprefix(const void *vmem1, const void *vmem2, size_t size)
{
    const unsigned char *mem1 = vmem1, *mem2 = vmem2;
    size_t plen = 0;
    
    while(size && toupper(mem1[0]) == toupper(mem2[0]))
        size--, plen++, mem1++, mem2++;
    
    return plen;
}

void *memmem(const void *vhaystack, size_t ssize, const void *vneedle, size_t nsize)
{
    const char *ptr, *haystack = vhaystack, *needle = vneedle;
    const char *last = haystack + ssize - nsize + 1;
    
    if(!nsize)
        return (void*)haystack;
    
    if(ssize < nsize)
        return NULL;
    
    for(; (ptr = memchr(haystack, needle[0], last-haystack)); haystack = ptr+1)
        if(!memcmp(ptr, needle, nsize))
            return (void*)ptr;
    
    return NULL;
}

int memcasecmp(const void *vmem1, const void *vmem2, size_t size)
{
    const unsigned char *mem1 = vmem1, *mem2 = vmem2, *mend = mem1 + size;
    int c1, c2;
    
    for(; mem1 < mend; mem1++, mem2++)
    {
        c1 = toupper(mem1[0]);
        c2 = toupper(mem2[0]);
        
        if(c1 < c2)
            return -1;
        if(c1 > c2)
            return 1;
    }
    
    return 0;
}

char *trim(char *str, const char *reject)
{
    if(!(str = strcpbrk(str, reject)))
        return str + strlen(str);
    
    return rtrim(str, reject);
}

char *trim_pred(char *str, ctype_pred_cb pred)
{
    return rtrim_pred(strwhile(str, pred), pred);
}

char *trim_blank(char *str)
{
    return rtrim_pred(strwhile(str, isblank), isblank);
}

char *trim_space(char *str)
{
    return rtrim_pred(strwhile(str, isspace), isspace);
}

char *rtrim(char *str, const char *reject)
{
    char *ptr = strrcpbrk(str, reject);
    
    if(ptr)
        ptr[1] = '\0';
    else
        str[0] = '\0';
    
    return str;
}

char *rtrim_pred(char *str, ctype_pred_cb pred)
{
    char *ptr = strrwhile(str, pred);
    
    if(ptr)
        ptr[1] = '\0';
    else
        str[0] = '\0';
    
    return str;
}

char *rtrim_blank(char *str)
{
    return rtrim_pred(str, isblank);
}

char *rtrim_space(char *str)
{
    return rtrim_pred(str, isspace);
}

void *memtrim(const void *mem, size_t *msize, const void *reject, size_t rsize)
{
    const char *m1 = mem, *m2;
    
    if(!(m2 = memcpbrk(m1, *msize, reject, rsize)))
        return *msize = 0, NULL;
    
    *msize -= m2 - m1;
    
    return memrtrim(m2, msize, reject, rsize);
}

void *memtrim_pred(const void *mem, size_t *size, ctype_pred_cb pred)
{
    const char *m1 = mem, *m2;
    
    if(!(m2 = memwhile(m1, *size, pred)))
        return *size = 0, NULL;
    
    *size -= m2 - m1;
    
    return memrtrim_pred(m2, size, pred);
}

void *memrtrim(const void *mem, size_t *msize, const void *reject, size_t rsize)
{
    const char *m1 = mem, *m2;
    
    if(!(m2 = memrcpbrk(m1, *msize, reject, rsize)))
        return *msize = 0, NULL;
    
    *msize = m2 - m1 +1;
    
    return (void*)m1;
}

void *memrtrim_pred(const void *mem, size_t *size, ctype_pred_cb pred)
{
    const char *m1 = mem, *m2;
    
    if(!(m2 = memrwhile(m1, *size, pred)))
        return *size = 0, NULL;
    
    *size = m2 - m1 +1;
    
    return (void*)m1;
}

char *strupper(char *str)
{
    char *s;
    
    for(s=str; s[0]; s[0] = toupper(s[0]), s++);
    
    return str;
}

char *strnupper(char *str, size_t n)
{
    char *s;
    
    for(s=str; s[0] && n; s[0] = toupper(s[0]), s++, n--);
    
    return str;
}

void *memupper(void *vmem, size_t size)
{
    unsigned char *mem = vmem, *mend = mem + size;
    
    for(; mem < mend; mem[0] = toupper(mem[0]), mem++);
    
    return vmem;
}

char *strlower(char *str)
{
    char *s;
    
    for(s=str; s[0]; s[0] = tolower(s[0]), s++);
    
    return str;
}

char *strnlower(char *str, size_t n)
{
    char *s;
    
    for(s=str; s[0] && n; s[0] = tolower(s[0]), s++, n--);
    
    return str;
}

void *memlower(void *vmem, size_t size)
{
    unsigned char *mem = vmem, *mend = mem + size;
    
    for(; mem < mend; mem[0] = tolower(mem[0]), mem++);
    
    return vmem;
}

char *strflat(char *str)
{
    for(; str[0]; str++)
        if(!isprint(str[0]))
            str[0] = ' ';
    
    return str;
}

char *strnflat(char *str, size_t n)
{
    for(; str[0] && n; str++, n--)
        if(!isprint(str[0]))
            str[0] = ' ';
    
    return str;
}

void *memflat(void *vmem, size_t size)
{
    unsigned char *mem = vmem;
    
    for(; size; mem++, size--)
        if(!isprint(mem[0]))
            mem[0] = ' ';
    
    return vmem;
}

char *strtrans(char *str, const char *from, const char *to)
{
    char *s;
    size_t t;
    
    if(!from[0] || !to[0])
        return str;
    
    for(s=str; s[0]; s++)
        for(t=0; from[t]; t++)
            if(s[0] == from[t])
                s[0] = to[t];
    
    return str;
}

void *memtrans(void *vmem, size_t msize, const void *vfrom, const void *vto, size_t tsize)
{
    unsigned char *mem = vmem, *mend = mem + msize;
    const unsigned char *from = vfrom, *to = vto;
    size_t t;
    
    if(!tsize)
        return vmem;
    
    for(; mem < mend; mem++)
        for(t=0; t < tsize; t++)
            if(mem[0] == from[t])
                mem[0] = to[t];
    
    return vmem;
}

size_t strescape(char *dst, const char *src, int esc, ctype_pred_cb keep)
{
    size_t n;
    char tmp;
    
    assert(!dst || dst != src);
    
    for(n=0; src[0]; src++, n++)
        if(src[0] == esc)
        {
            if(dst)
            {
                dst[n] = esc;
                dst[n+1] = esc;
            }
            n++;
        }
        else if(!keep(src[0]))
        {
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
            default:     tmp = 0; break;
            }
            
            if(dst)
            {
                dst[n] = esc;
                
                if(tmp)
                    dst[n+1] = tmp;
                else
                    snprintf(&dst[n+1], 4, "x%02hhX", (unsigned char)src[0]);
            }
            
            n += tmp ? 1 : 3;
        }
        else if(dst)
            dst[n] = src[0];
    
    dst[n] = '\0';
    
    return n;
}

ssize_t strunescape(char *dst, const char *src, int esc)
{
    size_t n;
    unsigned char tmp;
    
    for(n=0; src[0]; src++, n++)
        if(src[0] == esc)
        {
            src++;
            
            switch(src[0])
            {
            case '0': tmp = '\0'; break;
            case 'a': tmp = '\a'; break;
            case 'b': tmp = '\b'; break;
            case 'e': tmp = '\x1b'; break;
            case 'f': tmp = '\f'; break;
            case 'n': tmp = '\n'; break;
            case 'r': tmp = '\r'; break;
            case 't': tmp = '\t'; break;
            case 'v': tmp = '\v'; break;
            default:
                if(src[0] == esc)
                    tmp = esc;
                else if(sscanf(src, "x%02hhx", &tmp) != 1)
                    return errno = EINVAL, -1;
                else
                    src += 2;
            }
            
            if(dst)
                dst[n] = tmp;
        }
        else if(dst)
            dst[n] = src[0];
    
    dst[n] = '\0';
    
    return n;
}

char *strdup_escape(const char *src, int esc, ctype_pred_cb keep)
{
    char *dst;
    
    if(!(dst = malloc(strescape(NULL, src, esc, keep)+1)))
        return NULL;
    
    strescape(dst, src, esc, keep);
    
    return dst;
}

char *strdup_unescape(const char *src, int esc)
{
    char *dst;
    ssize_t len;
    
    if((len = strunescape(NULL, src, esc)) < 0
    || !(dst = malloc(len+1)))
        return NULL;
    
    strunescape(dst, src, esc);
    
    return dst;
}
