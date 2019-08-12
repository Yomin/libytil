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

#include <ytil/ext/stdio.h>
#include <stdlib.h>


char *strdup_printf(const char *fmt, ...)
{
    char *str;
    va_list ap;
    
    va_start(ap, fmt);
    str = strdup_vprintf(fmt, ap);
    va_end(ap);
    
    return str;
}

char *strdup_vprintf(const char *fmt, va_list ap)
{
    char *str;
    int len;
    
    if((len = vsnprintf(NULL, 0, fmt, ap)) < 0)
        return NULL;
    
    if(!(str = malloc(len+1)))
        return NULL;
    
    if(vsnprintf(str, len+1, fmt, ap) < 0)
        return free(str), NULL;
    
    return str;
}

void dump(void *mem, size_t size)
{
    fdump(stdout, mem, size);
}

void fdump(FILE *fp, void *vmem, size_t size)
{
    unsigned char *mem = vmem;
    size_t n;
    
    for(n=0; n < size; n++)
        fprintf(fp, "%02hhx", mem[n]);
}
