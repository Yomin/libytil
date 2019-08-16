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

#include <ytil/gen/error.h>
#include <ytil/ext/errno.h>
#include <ytil/def.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>


#ifndef ERROR_STACK_SIZE
#   define ERROR_STACK_SIZE 30
#endif

typedef enum error_type
{
      ERROR_TYPE_UNSET
    , ERROR_TYPE_ERROR
    , ERROR_TYPE_ERRNO
} error_type_id;

typedef struct error_origin
{

    error_type_id type;
    ssize_t error;
    const char *func;
    const error_info_st *infos;
} error_origin_st;

typedef struct error_entry
{
    const char *func;
} error_entry_st;

typedef struct error_state
{
    error_origin_st origin;
    error_entry_st stack[ERROR_STACK_SIZE];
    size_t size;
    const error_info_st *context;
} error_state_st;

static error_state_st error;


void _error_set(const char *func, const error_info_st *infos, size_t err)
{
    error.origin.type = ERROR_TYPE_ERROR;
    error.origin.func = func;
    error.origin.error = err;
    error.origin.infos = infos;
    
    error.size = 0;
    error.context = infos;
}

void _error_set_errno(const char *func, const char *sub)
{
    error.origin.type = ERROR_TYPE_ERRNO;
    error.origin.func = sub;
    error.origin.error = errno;
    error.origin.infos = NULL;
    
    error.size = 0;
    error.context = NULL;
}

void _error_push(const char *func)
{
    _error_propagate(func);
    error.context = NULL;
}

void _error_propagate(const char *func)
{
    size_t entry;
    
    entry = error.size < ERROR_STACK_SIZE ? error.size : ERROR_STACK_SIZE-1;
    error.stack[entry].func = func;
    
    error.size++;
}

void error_clear(void)
{
    memset(&error.origin, 0, sizeof(error_origin_st));
    
    error.origin.error = -1;
    error.size = 0;
    error.context = NULL;
}

ssize_t error_get(void)
{
    return error.context ? (ssize_t)error.origin.error : -1;
}

bool error_check(size_t err)
{
    return error.context && error.origin.error >= 0 && err == (size_t)error.origin.error;
}

ssize_t error_origin_get(void)
{
    return error.origin.error;
}

const char *error_origin_get_func(void)
{
    return error.origin.func;
}

const char *error_origin_get_name(void)
{
    switch(error.origin.type)
    {
    case ERROR_TYPE_ERROR:  return error.origin.infos[error.origin.error].name;
    case ERROR_TYPE_ERRNO:  return IFNULL(strerrno(error.origin.error), "UNKNOWN");
    default:                return NULL;
    }
}

const char *error_origin_get_desc(void)
{
    switch(error.origin.type)
    {
    case ERROR_TYPE_ERROR:  return error.origin.infos[error.origin.error].desc;
    case ERROR_TYPE_ERRNO:  return strerror(error.origin.error);
    default:                return NULL;
    }
}

size_t error_stack_get_size(void)
{
    return error.size;
}

const char *error_stack_get_func(size_t level)
{
    return level < error.size ? error.stack[level].func : NULL;
}
