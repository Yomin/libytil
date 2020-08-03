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
#include <ytil/def.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <errno.h>


#ifdef _WIN32
char *strndup(const char *str, size_t n)
{
    char *str2;
    
    if(!(str2 = malloc(n+1)))
        return NULL;
    
    strncpy(str2, str, n);
    str2[n] = '\0';
    
    return str2;
}
#endif

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

char *strnpbrk(const char *str, const char *accept, size_t n)
{
    const char *a;
    
    for(; n && str[0]; str++, n--)
        for(a=accept; a[0]; a++)
            if(a[0] == str[0])
                return (char*)str;
    
    return NULL;
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

size_t strcnt(const char *str, int c)
{
    size_t count;
    
    for(count=0; str[0]; str++)
        if(str[0] == c)
            count++;
    
    return count;
}

size_t strncnt(const char *str, int c, size_t n)
{
    size_t count;
    
    for(count=0; n && str[0]; n--, str++)
        if(str[0] == c)
            count++;
    
    return count;
}

size_t memcnt(const void *vmem, size_t size, int c)
{
    const unsigned char *mem = vmem, *end = mem + size;
    size_t count;
    
    for(count=0; mem < end; mem++)
        if(mem[0] == c)
            count++;
    
    return count;
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

bool strprefix(const char *str, const char *prefix)
{
    return !strncmp(prefix, str, strlen(prefix));
}

bool strcaseprefix(const char *str, const char *prefix)
{
    return !strncasecmp(prefix, str, strlen(prefix));
}

bool strnprefix(const char *str, size_t slen, const char *prefix, size_t plen)
{
    return plen <= slen && !strncmp(prefix, str, plen);
}

bool strncaseprefix(const char *str, size_t slen, const char *prefix, size_t plen)
{
    return plen <= slen && !strncasecmp(prefix, str, plen);
}

bool memprefix(const void *mem, size_t msize, const void *prefix, size_t psize)
{
    return psize <= msize && !memcmp(prefix, mem, psize);
}

bool memcaseprefix(const void *mem, size_t msize, const void *prefix, size_t psize)
{
    return psize <= msize && !memcasecmp(prefix, mem, psize);
}

size_t strprefixlen(const char *str1, const char *str2)
{
    size_t len;
    
    for(len=0; str1[len] && str2[len] && str1[len] == str2[len]; len++);
    
    return len;
}

size_t strcaseprefixlen(const char *str1, const char *str2)
{
    size_t len;
    
    for(len=0; str1[len] && str2[len] && toupper(str1[len]) == toupper(str2[len]); len++);
    
    return len;
}

size_t strnprefixlen(const char *str1, size_t slen1, const char *str2, size_t slen2)
{
    size_t len, slen = MIN(slen1, slen2);
    
    for(len=0; len < slen && str1[len] && str2[len] && str1[len] == str2[len]; len++);
    
    return len;
}

size_t strncaseprefixlen(const char *str1, size_t slen1, const char *str2, size_t slen2)
{
    size_t len, slen = MIN(slen1, slen2);
    
    for(len=0; len < slen && str1[len] && str2[len] && toupper(str1[len]) == toupper(str2[len]); len++);
    
    return len;
}

size_t memprefixlen(const void *vmem1, size_t size1, const void *vmem2, size_t size2)
{
    const unsigned char *mem1 = vmem1, *mem2 = vmem2;
    size_t len, size = MIN(size1, size2);
    
    for(len=0; len < size && mem1[len] == mem2[len]; len++);
    
    return len;
}

size_t memcaseprefixlen(const void *vmem1, size_t size1, const void *vmem2, size_t size2)
{
    const unsigned char *mem1 = vmem1, *mem2 = vmem2;
    size_t len, size = MIN(size1, size2);
    
    for(len=0; len < size && toupper(mem1[len]) == toupper(mem2[len]); len++);
    
    return len;
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

char *strtranspose(char *str, const char *from, const char *to)
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

char *strtranspose_f(char *str, ctype_transpose_cb trans)
{
    char *s;
    
    for(s=str; s[0]; s++)
        s[0] = trans(s[0]);
    
    return str;
}

char *strtranspose_fn(char *str, ctype_transpose_cb trans, size_t n)
{
    char *s;
    
    for(s=str; n && s[0]; n--, s++)
        s[0] = trans(s[0]);
    
    return str;
}

char *strtranspose_upper(char *str)
{
    return strtranspose_f(str, toupper);
}

char *strtranspose_upper_n(char *str, size_t n)
{
    return strtranspose_fn(str, toupper, n);
}

char *strtranspose_lower(char *str)
{
    return strtranspose_f(str, tolower);
}

char *strtranspose_lower_n(char *str, size_t n)
{
    return strtranspose_fn(str, tolower, n);
}

char *strtranspose_flatten(char *str)
{
    return strtranspose_f(str, flatten);
}

char *strtranspose_flatten_n(char *str, size_t n)
{
    return strtranspose_fn(str, flatten, n);
}

void *memtranspose(void *vmem, size_t msize, const void *vfrom, const void *vto, size_t tsize)
{
    unsigned char *mem = vmem;
    const unsigned char *from = vfrom, *to = vto;
    size_t t;
    
    if(!tsize)
        return vmem;
    
    for(; msize; mem++, msize--)
        for(t=0; t < tsize; t++)
            if(mem[0] == from[t])
                mem[0] = to[t];
    
    return vmem;
}

void *memtranspose_f(void *vmem, size_t size, ctype_transpose_cb trans)
{
    unsigned char *mem = vmem;
    
    for(; size; mem++, size--)
        mem[0] = trans(mem[0]);
    
    return mem;
}

void *memtranspose_upper(void *mem, size_t size)
{
    return memtranspose_f(mem, size, toupper);
}

void *memtranspose_lower(void *mem, size_t size)
{
    return memtranspose_f(mem, size, tolower);
}

void *memtranspose_flatten(void *mem, size_t size)
{
    return memtranspose_f(mem, size, flatten);
}

ssize_t strtranslate(char *sdst, const char *ssrc, ctype_translate_cb trans)
{
    unsigned char *dst = (unsigned char*)sdst;
    const unsigned char *src = (const unsigned char *)ssrc;
    size_t written = 0, read = 0;
    ssize_t rc;
    
    assert(dst != src);
    
    while((rc = trans(dst ? &dst[written] : NULL, &written, &src[read], &read, -1, true)) > 0);
    
    if(rc < 0)
        return rc;
    
    if(dst)
        dst[written] = '\0';
    
    return written;
}

ssize_t strtranslate_n(char *sdst, const char *ssrc, ctype_translate_cb trans, size_t n)
{
    unsigned char *dst = (unsigned char*)sdst;
    const unsigned char *src = (const unsigned char *)ssrc;
    size_t written = 0, read = 0;
    ssize_t rc;
    
    assert(dst != src);
    
    while((rc = trans(dst ? &dst[written] : NULL, &written, &src[read], &read, n-read, true)) > 0);
    
    if(rc < 0)
        return rc;
    
    if(dst)
        dst[written] = '\0';
    
    return written;
}

ssize_t strtranslate_mem(char *sdst, const void *vsrc, size_t size, ctype_translate_cb trans)
{
    unsigned char *dst = (unsigned char*)sdst;
    const unsigned char *src = vsrc;
    size_t written = 0, read = 0;
    ssize_t rc;
    
    assert(dst != src);
    
    while((rc = trans(dst ? &dst[written] : NULL, &written, &src[read], &read, size-read, false)) > 0);
    
    if(rc < 0)
        return rc;
    
    if(dst)
        dst[written] = '\0';
    
    return written;
}

ssize_t memtranslate(void *vdst, const void *vsrc, size_t size, ctype_translate_cb trans)
{
    unsigned char *dst = vdst;
    const unsigned char *src = vsrc;
    size_t written = 0, read = 0;
    ssize_t rc;
    
    assert(dst != src);
    
    while((rc = trans(dst ? &dst[written] : NULL, &written, &src[read], &read, size-read, false)) > 0);
    
    if(rc < 0)
        return rc;
    
    return written;
}

ssize_t memtranslate_str(void *vdst, const char *ssrc, ctype_translate_cb trans)
{
    unsigned char *dst = vdst;
    const unsigned char *src = (const unsigned char*)ssrc;
    size_t written = 0, read = 0;
    ssize_t rc;
    
    assert(dst != src);
    
    while((rc = trans(dst ? &dst[written] : NULL, &written, &src[read], &read, -1, true)) > 0);
    
    if(rc < 0)
        return rc;
    
    return written;
}

ssize_t memtranslate_str_n(void *vdst, const char *ssrc, ctype_translate_cb trans, size_t n)
{
    unsigned char *dst = vdst;
    const unsigned char *src = (const unsigned char*)ssrc;
    size_t written = 0, read = 0;
    ssize_t rc;
    
    assert(dst != src);
    
    while((rc = trans(dst ? &dst[written] : NULL, &written, &src[read], &read, n-read, true)) > 0);
    
    if(rc < 0)
        return rc;
    
    return written;
}

size_t strescape(char *dst, const char *src)
{
    return strtranslate(dst, src, translate_escape);
}

size_t strescape_n(char *dst, const char *src, size_t n)
{
    return strtranslate_n(dst, src, translate_escape, n);
}

size_t strescape_mem(char *dst, const void *src, size_t size)
{
    return strtranslate_mem(dst, src, size, translate_escape);
}

ssize_t strunescape(char *dst, const char *src)
{
    return strtranslate(dst, src, translate_unescape);
}

ssize_t strunescape_n(char *dst, const char *src, size_t n)
{
    return strtranslate_n(dst, src, translate_unescape, n);
}

ssize_t strunescape_mem(void *dst, const char *src)
{
    return memtranslate_str(dst, src, translate_unescape);
}

ssize_t strunescape_mem_n(void *dst, const char *src, size_t n)
{
    return memtranslate_str_n(dst, src, translate_unescape, n);
}
