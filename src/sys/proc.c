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

#include <ytil/sys/proc.h>
#include <ytil/def.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>


typedef struct proc_title
{
    char *title, *data;
    size_t size, orig_len, ref;
} proc_title_st;

static const error_info_st error_infos[] =
{
      ERROR_INFO(E_PROC_INVALID_TITLE, "Invalid proc title.")
    , ERROR_INFO(E_PROC_NO_SPACE, "Not enough space scavenged.")
    , ERROR_INFO(E_PROC_NOT_AVAILABLE, "Function not available.")
    , ERROR_INFO(E_PROC_NOT_INITIALIZED, "proc title is not initalized.")
};

#ifndef _WIN32
static proc_title_st   *proc_title;
extern char           **environ;
#endif

int proc_init_title(int argc, char *argv[])
{
#ifdef _WIN32
    return error_set(E_PROC_NOT_AVAILABLE), -1;
#else
    proc_title_st *title;
    int envc, i;
    
    if(proc_title)
        return proc_title->ref++, 0;
    
    if(!(title = calloc(1, sizeof(proc_title_st))))
        return error_wrap_errno(calloc), -1;
    
    for(envc=0; environ[envc]; envc++);
    
    if(!envc)
        title->size = argv[argc-1] + strlen(argv[argc-1]) - argv[0] +1;
    else
        title->size = environ[envc-1] + strlen(environ[envc-1]) - argv[0] +1;
    
    if(title->size < 2)
        return free(title), error_set(E_PROC_NO_SPACE), -1;
    
    if(!(title->data = malloc(title->size)))
        return free(title), error_wrap_errno(malloc), -1;
    
    memcpy(title->data, argv[0], title->size);
    
    for(i=1; i < argc; i++)
        argv[i] = &title->data[argv[i] - argv[0]];
    
    for(i=0; i < envc; i++)
        environ[i] = &title->data[environ[i] - argv[0]];
    
    title->title = argv[0];
    title->orig_len = strlen(argv[0]);
    proc_title = title;
    
    return 0;
#endif
}

void proc_free_title(void)
{
#ifndef _WIN32
    if(proc_title)
    {
        if(proc_title->ref > 1)
            proc_title->ref--;
        else
        {
            free(proc_title->data);
            free(proc_title);
            proc_title = NULL;
        }
    }
#endif
}

int proc_set_title(const char *fmt, ...)
{
    va_list ap;
    int rc;
    
    va_start(ap, fmt);
    rc = error_pass_int(proc_set_title_v(fmt, ap));
    va_end(ap);
    
    return rc;
}

int proc_set_title_v(const char *fmt, va_list ap)
{
#ifdef _WIN32
    return error_set(E_PROC_NOT_AVAILABLE), -1;
#else
    return_error_if_fail(proc_title, E_PROC_NOT_INITIALIZED, -1);
    return_error_if_fail(fmt, E_PROC_INVALID_TITLE, -1);
    
    vsnprintf(proc_title->title, proc_title->size, fmt, ap);
    proc_title->title[proc_title->size-1] = '\0';
    
    return 0;
#endif
}

int proc_append_title(const char *fmt, ...)
{
    va_list ap;
    int rc;
    
    va_start(ap, fmt);
    rc = error_pass_int(proc_append_title_v(fmt, ap));
    va_end(ap);
    
    return rc;
}

int proc_append_title_v(const char *fmt, va_list ap)
{
#ifdef _WIN32
    return error_set(E_PROC_NOT_AVAILABLE), -1;
#else
    char *title;
    size_t size;
    
    return_error_if_fail(proc_title, E_PROC_NOT_INITIALIZED, -1);
    return_error_if_fail(fmt, E_PROC_INVALID_TITLE, -1);
    
    memcpy(proc_title->title, proc_title->data, proc_title->orig_len);
    title = proc_title->title + proc_title->orig_len;
    size = proc_title->size - proc_title->orig_len;
    
    vsnprintf(title, size, fmt, ap);
    title[size-1] = '\0';
    
    return 0;
#endif
}
