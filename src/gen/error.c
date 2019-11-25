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

typedef union error_value
{
    struct error_value_error
    {
        ssize_t code;
        const error_info_st *infos;
    } error;
    struct error_value_errno
    {
        int code;
    } _errno;
#ifdef _WIN32
    struct error_value_win32
    {
        DWORD code;
    } win32;
    struct error_value_hresult
    {
        HRESULT result;
    } hresult;
    struct error_value_ntstatus
    {
        NTSTATUS status;
    } ntstatus;
#endif
} error_value_un;

typedef struct error_entry
{
    error_type_id type;
    const char *func;
    error_value_un value;
} error_entry_st;

typedef struct error_state
{
    error_entry_st stack[ERROR_STACK_SIZE];
    size_t size;
} error_state_st;

static error_state_st errors;

static const error_info_st error_infos[] =
{
      [-E_ERROR_UNSET]   = { .name = "E_ERROR_UNSET",   .desc = "Error Unset" }
    , [-E_ERROR_WRAPPER] = { .name = "E_ERROR_WRAPPER", .desc = "Error Wrapper" }
    , [-E_ERROR_PASS]    = { .name = "E_ERROR_PASS",    .desc = "Error Pass" }
};


static error_entry_st *error_add(error_type_id type, const char *func)
{
    size_t id = errors.size < ERROR_STACK_SIZE ? errors.size : ERROR_STACK_SIZE-1;
    error_entry_st *entry = &errors.stack[id];
    
    entry->type = type;
    entry->func = func;
    
    errors.size = id+1;
    
    return entry;
}

void _error_set(const char *func, const error_info_st *infos, size_t error)
{
    errors.size = 0;
    _error_push(func, infos, error);
}

void _error_push(const char *func, const error_info_st *infos, size_t error)
{
    error_entry_st *entry = error_add(ERROR_TYPE_ERROR, func);
    
    entry->value.error.code = error;
    entry->value.error.infos = infos;
}

void _error_wrap(const char *func)
{
    _error_push(func, error_infos, E_ERROR_WRAPPER);
}

void _error_pack(const char *func, const error_info_st *infos, size_t error)
{
    if(error_type(0) == ERROR_TYPE_ERROR && error_get(0) == E_ERROR_WRAPPER)
        _error_wrap(func);
    else
        _error_push(func, infos, error);
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
            _error_wrap(func);
            break;
        }
        else if(match == error)
        {
            _error_push(func, infos, va_arg(ap, int));
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
    _error_push(func, error_infos, E_ERROR_PASS);
}

void _errno_set(const char *func, int error)
{
    errors.size = 0;
    _errno_push(func, error);
}

void _errno_push(const char *func, int error)
{
    error_entry_st *entry = error_add(ERROR_TYPE_ERRNO, func);
    
    entry->value._errno.code = error;
}

void _error_push_errno(const char *func, const error_info_st *infos, size_t error, const char *sub)
{
    _errno_set(sub, errno);
    _error_push(func, infos, error);
}

void _error_wrap_errno(const char *func, const char *sub)
{
    _errno_set(sub, errno);
    _error_wrap(func);
}

void _error_pass_errno(const char *func, const char *sub)
{
    _errno_set(sub, errno);
    _error_pass(func);
}

#ifdef _WIN32

static void error_set_win32(const char *sub, DWORD error)
{
    error_entry_st *entry;
    
    errors.size = 0;
    entry = error_add(ERROR_TYPE_WIN32, sub);
    entry->value.win32.code = error;
}

void _error_push_win32(const char *func, const error_info_st *infos, size_t error, const char *sub, DWORD error32)
{
    error_set_win32(sub, error32);
    _error_push(func, infos, error);
}

void _error_push_last_win32(const char *func, const error_info_st *infos, size_t error, const char *sub)
{
    _error_push_win32(func, infos, error, sub, GetLastError());
}

void _error_wrap_win32(const char *func, const char *sub, DWORD error)
{
    error_set_win32(sub, error);
    _error_wrap(func);
}

void _error_wrap_last_win32(const char *func, const char *sub)
{
    _error_wrap_win32(func, sub, GetLastError());
}

void _error_pass_win32(const char *func, const char *sub, DWORD error)
{
    error_set_win32(sub, error);
    _error_pass(func);
}

void _error_pass_last_win32(const char *func, const char *sub)
{
    _error_pass_win32(func, sub, GetLastError());
}

static void error_set_hresult(const char *sub, HRESULT result)
{
    error_entry_st *entry;
    
    errors.size = 0;
    entry = error_add(ERROR_TYPE_HRESULT, sub);
    entry->value.hresult.result = result;
}

void _error_push_hresult(const char *func, const error_info_st *infos, size_t error, const char *sub, HRESULT result)
{
    error_set_hresult(sub, result);
    _error_push(func, infos, error);
}

void _error_wrap_hresult(const char *func, const char *sub, HRESULT result)
{
    error_set_hresult(sub, result);
    _error_wrap(func);
}

void _error_pass_hresult(const char *func, const char *sub, HRESULT result)
{
    error_set_hresult(sub, result);
    _error_pass(func);
}

static void error_set_ntstatus(const char *sub, NTSTATUS status)
{
    error_entry_st *entry;
    
    errors.size = 0;
    entry = error_add(ERROR_TYPE_NTSTATUS, sub);
    entry->value.ntstatus.status = status;
}

void _error_push_ntstatus(const char *func, const error_info_st *infos, size_t error, const char *sub, NTSTATUS status)
{
    error_set_ntstatus(sub, status);
    _error_push(func, infos, error);
}

void _error_wrap_ntstatus(const char *func, const char *sub, NTSTATUS status)
{
    error_set_ntstatus(sub, status);
    _error_wrap(func);
}

void _error_pass_ntstatus(const char *func, const char *sub, NTSTATUS status)
{
    error_set_ntstatus(sub, status);
    _error_pass(func);
}

#endif // _WIN32

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
    
    for(level = errors.size-1;
        level && errors.stack[level].type == ERROR_TYPE_ERROR
              && errors.stack[level].value.error.code == E_ERROR_PASS;
        level--);
    
    for(; level && depth; depth--)
        for(level--;
            level && errors.stack[level].type == ERROR_TYPE_ERROR
                  && errors.stack[level].value.error.code == E_ERROR_PASS;
            level--);
    
    if(depth || (errors.stack[level].type == ERROR_TYPE_ERROR
             &&  errors.stack[level].value.error.code == E_ERROR_PASS))
        return NULL;
    
    return &errors.stack[level];
}

static inline const char *error_entry_get_error_name(error_entry_st *entry)
{
    return entry->value.error.infos[ABS(entry->value.error.code)].name;
}

static inline const char *error_entry_get_errno_name(error_entry_st *entry)
{
    return IFNULL(strerrno(entry->value._errno.code), "<unknown_errno>");
}

#ifdef _WIN32

static char error_name_buf[20];

static inline const char *error_entry_get_win32_name(error_entry_st *entry)
{
    snprintf(error_name_buf, sizeof(error_name_buf),
        "WIN32_%08X", entry->value.win32.code);
    
    return error_name_buf;
}

static inline const char *error_entry_get_hresult_name(error_entry_st *entry)
{
    snprintf(error_name_buf, sizeof(error_name_buf),
        "HRESULT_%08X", entry->value.hresult.result);
    
    return error_name_buf;
}

static inline const char *error_entry_get_ntstatus_name(error_entry_st *entry)
{
    snprintf(error_name_buf, sizeof(error_name_buf),
        "NTSTATUS_%08X", entry->value.ntstatus.status);
    
    return error_name_buf;
}

#endif

static const char *error_entry_get_name(error_entry_st *entry)
{
    switch(entry->type)
    {
    case ERROR_TYPE_ERROR:      return error_entry_get_error_name(entry);
    case ERROR_TYPE_ERRNO:      return error_entry_get_errno_name(entry);
#ifdef _WIN32
    case ERROR_TYPE_WIN32:      return error_entry_get_win32_name(entry);
    case ERROR_TYPE_HRESULT:    return error_entry_get_hresult_name(entry);
    case ERROR_TYPE_NTSTATUS:   return error_entry_get_ntstatus_name(entry);
#endif
    default:                    abort();
    }
}

static inline const char *error_entry_get_error_desc(error_entry_st *entry)
{
    return entry->value.error.infos[ABS(entry->value.error.code)].desc;
}

static inline const char *error_entry_get_errno_desc(error_entry_st *entry)
{
    return strerror(entry->value._errno.code);
}

#ifdef _WIN32

static char error_desc_buf[200+4];

static inline const char *error_entry_get_win32_desc(error_entry_st *entry)
{
    DWORD rc;
    
    rc = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_IGNORE_INSERTS, NULL,
        entry->value.win32.code, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        error_desc_buf, sizeof(error_desc_buf)-3, NULL);
    
    if(!rc)
        return "<WIN32_FormatMessage_Error>";
    
    if(rc == sizeof(error_desc_buf)-4)
        strncpy(&error_desc_buf[sizeof(error_desc_buf)-4], "...", 4);
    
    assert(!"test overflow!!!");
    
    return error_desc_buf;
}

static inline const char *error_entry_get_hresult_desc(error_entry_st *entry)
{
    return "<HRESULT_MESSAGE>"; // todo
}

static inline const char *error_entry_get_ntstatus_desc(error_entry_st *entry)
{
    return "<NTSTATUS_MESSAGE>"; // todo
}

#endif

static const char *error_entry_get_desc(error_entry_st *entry)
{
    switch(entry->type)
    {
    case ERROR_TYPE_ERROR:      return error_entry_get_error_desc(entry);
    case ERROR_TYPE_ERRNO:      return error_entry_get_errno_desc(entry);
#ifdef _WIN32
    case ERROR_TYPE_WIN32:      return error_entry_get_win32_desc(entry);
    case ERROR_TYPE_HRESULT:    return error_entry_get_hresult_desc(entry);
    case ERROR_TYPE_NTSTATUS:   return error_entry_get_ntstatus_desc(entry);
#endif
    default:                    abort();
    }
}

ssize_t error_get(size_t depth)
{
    error_entry_st *entry = error_get_entry(depth);
    assert(!entry || entry->type == ERROR_TYPE_ERROR);
    
    return entry ? entry->value.error.code : E_ERROR_UNSET;
}

bool error_check(size_t depth, ssize_t error)
{
    error_entry_st *entry = error_get_entry(depth);
    assert(!entry || entry->type == ERROR_TYPE_ERROR);
    
    return entry && entry->value.error.code == error;
}

int error_get_errno(size_t depth)
{
    error_entry_st *entry = error_get_entry(depth);
    assert(!entry || entry->type == ERROR_TYPE_ERRNO);
    
    return entry ? entry->value._errno.code : -1;
}

bool error_check_errno(size_t depth, int error)
{
    error_entry_st *entry = error_get_entry(depth);
    assert(!entry || entry->type == ERROR_TYPE_ERRNO);
    
    return entry && entry->value._errno.code == error;
}

#ifdef _WIN32

DWORD error_get_win32(size_t depth)
{
    error_entry_st *entry = error_get_entry(depth);
    assert(!entry || entry->type == ERROR_TYPE_WIN32);
    
    return entry ? entry->value.win32.code : -1;
}

bool error_check_win32(size_t depth, DWORD error)
{
    error_entry_st *entry = error_get_entry(depth);
    assert(!entry || entry->type == ERROR_TYPE_WIN32);
    
    return entry && entry->value.win32.code == error;
}

HRESULT error_get_hresult(size_t depth)
{
    error_entry_st *entry = error_get_entry(depth);
    assert(!entry || entry->type == ERROR_TYPE_HRESULT);
    
    return entry ? entry->value.hresult.result : -1;
}

bool error_check_hresult(size_t depth, HRESULT result)
{
    error_entry_st *entry = error_get_entry(depth);
    assert(!entry || entry->type == ERROR_TYPE_HRESULT);
    
    return entry && entry->value.hresult.result == result;
}

NTSTATUS error_get_ntstatus(size_t depth)
{
    error_entry_st *entry = error_get_entry(depth);
    assert(!entry || entry->type == ERROR_TYPE_NTSTATUS);
    
    return entry ? entry->value.ntstatus.status : -1;
}

bool error_check_ntstatus(size_t depth, NTSTATUS status)
{
    error_entry_st *entry = error_get_entry(depth);
    assert(!entry || entry->type == ERROR_TYPE_NTSTATUS);
    
    return entry && entry->value.ntstatus.status == status;
}

#endif // _WIN32

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

int error_origin_get_errno(void)
{
    return error_stack_get_errno(0);
}

bool error_origin_check_errno(int error)
{
    return error_stack_check_errno(0, error);
}

#ifdef _WIN32

DWORD error_origin_get_win32(void)
{
    return error_stack_get_win32(0);
}

bool error_origin_check_win32(DWORD error)
{
    return error_stack_check_win32(0, error);
}

HRESULT error_origin_get_hresult(void)
{
    return error_stack_get_hresult(0);
}

bool error_origin_check_hresult(HRESULT result)
{
    return error_stack_check_hresult(0, result);
}

NTSTATUS error_origin_get_ntstatus(void)
{
    return error_stack_get_ntstatus(0);
}

bool error_origin_check_ntstatus(NTSTATUS status)
{
    return error_stack_check_ntstatus(0, status);
}

#endif // _WIN32

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
    return_value_if_fail(level < errors.size, E_ERROR_UNSET);
    assert(errors.stack[level].type == ERROR_TYPE_ERROR);
    
    return errors.stack[level].value.error.code;
}

bool error_stack_check_error(size_t level, ssize_t error)
{
    return_value_if_fail(level < errors.size, false);
    assert(errors.stack[level].type == ERROR_TYPE_ERROR);
    
    return errors.stack[level].value.error.code == error;
}

int error_stack_get_errno(size_t level)
{
    return_value_if_fail(level < errors.size, -1);
    assert(errors.stack[level].type == ERROR_TYPE_ERRNO);
    
    return errors.stack[level].value._errno.code;
}

bool error_stack_check_errno(size_t level, int error)
{
    return_value_if_fail(level < errors.size, false);
    assert(errors.stack[level].type == ERROR_TYPE_ERRNO);
    
    return errors.stack[level].value._errno.code == error;
}

#ifdef _WIN32

DWORD error_stack_get_win32(size_t level)
{
    return_value_if_fail(level < errors.size, -1);
    assert(errors.stack[level].type == ERROR_TYPE_WIN32);
    
    return errors.stack[level].value.win32.code;
}

bool error_stack_check_win32(size_t level, DWORD error)
{
    return_value_if_fail(level < errors.size, false);
    assert(errors.stack[level].type == ERROR_TYPE_WIN32);
    
    return errors.stack[level].value.win32.code == error;
}

HRESULT error_stack_get_hresult(size_t level)
{
    return_value_if_fail(level < errors.size, -1);
    assert(errors.stack[level].type == ERROR_TYPE_HRESULT);
    
    return errors.stack[level].value.hresult.result;
}

bool error_stack_check_hresult(size_t level, HRESULT result)
{
    return_value_if_fail(level < errors.size, false);
    assert(errors.stack[level].type == ERROR_TYPE_HRESULT);
    
    return errors.stack[level].value.hresult.result == result;
}

NTSTATUS error_stack_get_ntstatus(size_t level)
{
    return_value_if_fail(level < errors.size, -1);
    assert(errors.stack[level].type == ERROR_TYPE_NTSTATUS);
    
    return errors.stack[level].value.ntstatus.status;
}

bool error_stack_check_ntstatus(size_t level, NTSTATUS status)
{
    return_value_if_fail(level < errors.size, false);
    assert(errors.stack[level].type == ERROR_TYPE_NTSTATUS);
    
    return errors.stack[level].value.ntstatus.status == status;
}

#endif // _WIN32

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
