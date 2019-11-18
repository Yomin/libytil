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
#include <stdarg.h>


#ifndef ERROR_STACK_SIZE
#   define ERROR_STACK_SIZE 20
#endif

typedef struct error_entry
{
    error_type_id type;
    ssize_t error;
    const char *func;
    const error_info_st *infos;
} error_entry_st;

typedef struct error_state
{
    error_entry_st stack[ERROR_STACK_SIZE];
    size_t size;
} error_state_st;

static error_state_st errors;


static void error_add(error_type_id type, const char *func, ssize_t error, const error_info_st *infos)
{
    size_t entry = errors.size < ERROR_STACK_SIZE ? errors.size : ERROR_STACK_SIZE-1;
    
    errors.stack[entry] = (error_entry_st)
    {
          .type = type
        , .func = func
        , .error = error
        , .infos = infos
    };
    
    errors.size = entry+1;
}

void _error_set(const char *func, const error_info_st *infos, size_t error)
{
    errors.size = 0;
    error_add(ERROR_TYPE_ERROR, func, error, infos);
}

void _error_push(const char *func, const error_info_st *infos, size_t error)
{
    error_add(ERROR_TYPE_ERROR, func, error, infos);
}

void _error_wrap(const char *func)
{
    error_add(ERROR_TYPE_WRAPPER, func, E_ERROR_WRAPPER, NULL);
}

void _error_pack(const char *func, const error_info_st *infos, size_t error)
{
    if(error_get(0) < 0)
        error_add(ERROR_TYPE_WRAPPER, func, E_ERROR_WRAPPER, NULL);
    else
        error_add(ERROR_TYPE_ERROR, func, error, infos);
}

void _error_map(const char *func, size_t depth, const error_info_st *infos, ...)
{
    ssize_t error = error_get(depth), match;
    va_list ap;
    
    va_start(ap, infos);
    
    while(1)
    {
        if((match = va_arg(ap, int)) == E_ERROR_UNSET)
        {
            error_add(ERROR_TYPE_WRAPPER, func, E_ERROR_WRAPPER, NULL);
            break;
        }
        else if(match == error)
        {
            error_add(ERROR_TYPE_ERROR, func, va_arg(ap, int), infos);
            break;
        }
        else
        {
            va_arg(ap, int);
        }
    }
    
    va_end(ap);
}

void _error_pass(const char *func)
{
    error_add(ERROR_TYPE_PASS, func, -1, NULL);
}

void _errno_set(const char *func, int error)
{
    errors.size = 0;
    error_add(ERROR_TYPE_ERRNO, func, error, NULL);
}

void _errno_push(const char *func, int error)
{
    error_add(ERROR_TYPE_ERRNO, func, error, NULL);
}

void _error_push_errno(const char *func, const error_info_st *infos, size_t error, const char *sub)
{
    errors.size = 0;
    error_add(ERROR_TYPE_ERRNO, sub, errno, NULL);
    error_add(ERROR_TYPE_ERROR, func, error, infos);
}

void _error_wrap_errno(const char *func, const char *sub)
{
    errors.size = 0;
    error_add(ERROR_TYPE_ERRNO, sub, errno, NULL);
    error_add(ERROR_TYPE_WRAPPER, func, E_ERROR_WRAPPER, NULL);
}

void _error_pass_errno(const char *func, const char *sub)
{
    errors.size = 0;
    error_add(ERROR_TYPE_ERRNO, sub, errno, NULL);
    error_add(ERROR_TYPE_PASS, func, -1, NULL);
}

void error_clear(void)
{
    errors.size = 0;
}

size_t error_depth(void)
{
    return errors.size;
}

static error_entry_st *error_get_entry(size_t depth)
{
    size_t level;
    
    return_value_if_fail(errors.size, NULL);
    
    for(level = errors.size-1; level && errors.stack[level].type == ERROR_TYPE_PASS; level--);
    
    for(; level && depth; depth--)
        for(level--; level && errors.stack[level].type == ERROR_TYPE_PASS; level--);
    
    if(depth || errors.stack[level].type == ERROR_TYPE_PASS)
        return NULL;
    
    return &errors.stack[level];
}

static const char *error_entry_get_name(error_entry_st *entry)
{
    switch(entry->type)
    {
    case ERROR_TYPE_WRAPPER:    return "E_ERROR_WRAPPER";
    case ERROR_TYPE_ERROR:      return entry->infos[entry->error].name;
    case ERROR_TYPE_ERRNO:      return IFNULL(strerrno(entry->error), "<unknown_errno>");
    default:                    abort();
    }
}

static const char *error_entry_get_desc(error_entry_st *entry)
{
    switch(entry->type)
    {
    case ERROR_TYPE_WRAPPER:    return "Error Wrapper";
    case ERROR_TYPE_ERROR:      return entry->infos[entry->error].desc;
    case ERROR_TYPE_ERRNO:      return strerror(entry->error);
    default:                    abort();
    }
}

ssize_t error_get(size_t depth)
{
    error_entry_st *entry = error_get_entry(depth);
    
    return entry ? entry->error : E_ERROR_UNSET;
}

bool error_check(size_t depth, ssize_t error)
{
    error_entry_st *entry = error_get_entry(depth);
    
    return entry && entry->error == error;
}

error_type_id error_type(size_t depth)
{
    error_entry_st *entry = error_get_entry(depth);
    
    return entry ? entry->type : ERROR_TYPE_INVALID;
}

const char *error_func(size_t depth)
{
    error_entry_st *entry = error_get_entry(depth);
    
    return entry ? entry->func : NULL;
}

const char *error_name(size_t depth)
{
    error_entry_st *entry = error_get_entry(depth);
    
    return entry ? error_entry_get_name(entry) : NULL;
}

const char *error_desc(size_t depth)
{
    error_entry_st *entry = error_get_entry(depth);
    
    return entry ? error_entry_get_desc(entry) : NULL;
}

ssize_t error_origin_get(void)
{
    return error_stack_get_error(0);
}

bool error_origin_check(ssize_t error)
{
    return error_stack_check_error(0, error);
}

error_type_id error_origin_type(void)
{
    return error_stack_get_type(0);
}

const char *error_origin_func(void)
{
    return error_stack_get_func(0);
}

const char *error_origin_name(void)
{
    return error_stack_get_name(0);
}

const char *error_origin_desc(void)
{
    return error_stack_get_desc(0);
}

ssize_t error_stack_get_error(size_t level)
{
    return level >= errors.size ? E_ERROR_UNSET : errors.stack[level].error;
}

bool error_stack_check_error(size_t level, ssize_t error)
{
    return level < errors.size && errors.stack[level].error == error;
}

error_type_id error_stack_get_type(size_t level)
{
    return level >= errors.size ? ERROR_TYPE_INVALID : errors.stack[level].type;
}

const char *error_stack_get_func(size_t level)
{
    return level >= errors.size ? NULL : errors.stack[level].func;
}

const char *error_stack_get_name(size_t level)
{
    return level >= errors.size ? NULL : error_entry_get_name(&errors.stack[level]);
}

const char *error_stack_get_desc(size_t level)
{
    return level >= errors.size ? NULL : error_entry_get_desc(&errors.stack[level]);
}
