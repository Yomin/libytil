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
#   define ERROR_STACK_SIZE 10
#endif

typedef enum error_type
{
      ERROR_TYPE_WRAP
    , ERROR_TYPE_PROPAGATE
    , ERROR_TYPE_ERROR
    , ERROR_TYPE_ERRNO
} error_type_id;

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
    int context;
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
    
    switch(type)
    {
    case ERROR_TYPE_WRAP:   errors.context = -1; break;
    case ERROR_TYPE_ERROR:
    case ERROR_TYPE_ERRNO:  errors.context = entry; break;
    default:                break;
    }
    
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
    error_add(ERROR_TYPE_WRAP, func, -1, NULL);
}

void _error_push_wrap(const char *func, const error_info_st *infos, size_t error)
{
    if(errors.context >= 0)
        error_add(ERROR_TYPE_ERROR, func, error, infos);
    else
        error_add(ERROR_TYPE_WRAP, func, -1, NULL);
}

void _error_propagate(const char *func)
{
    error_add(ERROR_TYPE_PROPAGATE, func, -1, NULL);
}

void _error_set_errno(const char *func, int error)
{
    errors.size = 0;
    error_add(ERROR_TYPE_ERRNO, func, error, NULL);
}

void _error_push_errno(const char *func, int error)
{
    error_add(ERROR_TYPE_ERRNO, func, error, NULL);
}

void _error_wrap_errno(const char *func, const char *sub)
{
    errors.size = 0;
    error_add(ERROR_TYPE_ERRNO, sub, errno, NULL);
    error_add(ERROR_TYPE_WRAP, func, -1, NULL);
}

void _error_propagate_errno(const char *func, const char *sub)
{
    errors.size = 0;
    error_add(ERROR_TYPE_ERRNO, sub, errno, NULL);
    error_add(ERROR_TYPE_PROPAGATE, func, -1, NULL);
}

void error_clear(void)
{
    errors.size = 0;
    errors.context = -1;
}

bool error_check(ssize_t error)
{
    return errors.context >= 0 && error == errors.stack[errors.context].error;
}

ssize_t error_get(void)
{
    return errors.context < 0 ? -1 : error_stack_get_error(errors.context);
}

const char *error_get_func(const char *def)
{
    return errors.context < 0 ? def : error_stack_get_func(errors.context, def);
}

const char *error_get_name(const char *def)
{
    return errors.context < 0 ? def : error_stack_get_name(errors.context, def);
}

const char *error_get_desc(const char *def)
{
    return errors.context < 0 ? def : error_stack_get_desc(errors.context, def);
}

ssize_t error_origin_get(void)
{
    return error_stack_get_error(0);
}

const char *error_origin_get_func(const char *def)
{
    return error_stack_get_func(0, def);
}

const char *error_origin_get_name(const char *def)
{
    return error_stack_get_name(0, def);
}

const char *error_origin_get_desc(const char *def)
{
    return error_stack_get_desc(0, def);
}

size_t error_stack_get_size(void)
{
    return errors.size;
}

bool error_stack_is_wrapper(size_t level)
{
    if(level >= errors.size)
        return true;
    
    switch(errors.stack[level].type)
    {
    case ERROR_TYPE_ERROR:
    case ERROR_TYPE_ERRNO:  return false;
    default:                return true;
    }
}

ssize_t error_stack_get_error(size_t level)
{
    return level < errors.size ? errors.stack[level].error : -1;
}

const char *error_stack_get_func(size_t level, const char *def)
{
    return level < errors.size ? errors.stack[level].func : def;
}

const char *error_stack_get_name(size_t level, const char *def)
{
    error_entry_st *entry;
    
    if(level >= errors.size)
        return def;
    
    entry = &errors.stack[level];
    
    switch(entry->type)
    {
    case ERROR_TYPE_ERROR:  return entry->infos[entry->error].name;
    case ERROR_TYPE_ERRNO:  return IFNULL(strerrno(entry->error), "<unknown_errno>");
    default:                abort();
    }
}

const char *error_stack_get_desc(size_t level, const char *def)
{
    error_entry_st *entry;
    
    if(level >= errors.size)
        return def;
    
    entry = &errors.stack[level];
    
    switch(entry->type)
    {
    case ERROR_TYPE_ERROR:  return entry->infos[entry->error].desc;
    case ERROR_TYPE_ERRNO:  return strerror(entry->error);
    default:                abort();
    }
}
