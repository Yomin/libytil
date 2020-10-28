/*
 * Copyright (c) 2019-2020 Martin Rödel a.k.a. Yomin Nimoy
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

/// \file

#include <ytil/gen/error.h>
#include <ytil/ext/errno.h>
#include <ytil/def.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>


#ifdef _WIN32
    #include <windows.h>
    #include <ntstatus.h>
#endif

#ifndef ERROR_STACK_SIZE
    #define ERROR_STACK_SIZE 20 ///< number of maximum error entries
#endif


/// error stack entry
typedef struct error_entry
{
    const char          *func;      ///< error function
    const error_type_st *type;      ///< error type
    int                 code;       ///< error code
    char                *desc;      ///< error description override
} error_entry_st;

/// error stack
typedef struct error_stack
{
    error_entry_st  stack[ERROR_STACK_SIZE];    ///< error entry list
    size_t          size;                       ///< number of error entries
    size_t          clean;                      ///< number of entries which need cleanup
} error_stack_st;

/// error state
static error_stack_st errors;

static char error_name_buf[50];     ///< error name buffer
static char error_desc_buf[200];    ///< error description buffer


const char *error_type_name(const error_type_st *type)
{
    assert(type);

    return type->name;
}

const char *error_type_get_name(const error_type_st *type, int code)
{
    const char *name = NULL;

    assert(type);

    error_name_buf[0] = '\0';

    switch(type->iface_type)
    {
    case ERROR_INTERFACE_CALLBACK:

        if(type->iface.callback.error_name)
        {
            name = type->iface.callback.error_name(
                type, code, error_name_buf, sizeof(error_name_buf));
        }

        break;

    case ERROR_INTERFACE_LIST:
        name = type->iface.list.infos[ABS(code)].name;

        break;

    default:
        abort();
    }

    if(name)
        return name;

    snprintf(error_name_buf, sizeof(error_name_buf), "%s_%d",
        IFNULL(type->name, "UNKNOWN"), code);

    return error_name_buf;
}

const char *error_type_get_desc(const error_type_st *type, int code)
{
    const char *desc = NULL;

    assert(type);

    error_desc_buf[0] = '\0';

    switch(type->iface_type)
    {
    case ERROR_INTERFACE_CALLBACK:

        if(type->iface.callback.error_desc)
        {
            desc = type->iface.callback.error_desc(
                type, code, error_desc_buf, sizeof(error_desc_buf));
        }

        break;

    case ERROR_INTERFACE_LIST:
        desc = type->iface.list.infos[ABS(code)].desc;

        break;

    default:
        abort();
    }

    return desc ? desc : "<no_description_available>";
}

bool error_type_is_oom(const error_type_st *type, int code)
{
    assert(type);

    return type->error_is_oom && type->error_is_oom(type, code);
}

int error_type_get_last(const error_type_st *type, const char **desc, const void *ctx)
{
    assert(type);
    assert(type->error_last);

    return type->error_last(type, desc, (void *)ctx);
}

__attribute__((destructor))
void error_clear(void)
{
    size_t e;

    for(e = 0; errors.clean && e < errors.size; e++)
        if(errors.stack[e].desc)
        {
            free(errors.stack[e].desc);
            errors.clean--;
        }

    assert(!errors.clean);

    errors.size = 0;
}

size_t error_depth(void)
{
    return errors.size;
}

/// Find next deeper level containing non transparent error.
///
/// \param level    error level to start from
///
/// \returns        error level
static ssize_t error_next_level(ssize_t level)
{
    error_entry_st *entry;

    for(level--; level >= 0; level--)
    {
        entry = &errors.stack[level];

        if(entry->type != &ERROR_TYPE_GENERIC)
            return level;

        switch(entry->code)
        {
        case E_GENERIC_PASS:
            break;

        case E_GENERIC_SKIP:
            level = error_next_level(level);
            break;

        default:
            return level;
        }
    }

    return -1;
}

/// Get error entry from stack.
///
/// \param depth    error depth, 0 is top level error
///
/// \returns        error entry
static error_entry_st *error_get_entry(size_t depth)
{
    ssize_t level;

    assert(errors.size);

    for(level = errors.size, depth++; level >= 0 && depth; depth--)
        level = error_next_level(level);

    assert(level >= 0 && !depth);

    return &errors.stack[level];
}

/// Get error name from entry.
///
/// \param entry    error entry
///
/// \returns        error name
static const char *error_entry_get_name(const error_entry_st *entry)
{
    return error_type_get_name(entry->type, entry->code);
}

/// Get error description from entry.
///
/// \param entry    error entry
///
/// \returns        error description
static const char *error_entry_get_desc(const error_entry_st *entry)
{
    if(entry->desc)
        return entry->desc;

    return error_type_get_desc(entry->type, entry->code);
}

/// Check if error from entry is out-of-memory error.
///
/// \param entry    error entry
///
/// \returns        error name
static bool error_entry_is_oom(const error_entry_st *entry)
{
    return error_type_is_oom(entry->type, entry->code);
}

const char *error_func(size_t depth)
{
    return error_get_entry(depth)->func;
}

const error_type_st *error_type(size_t depth)
{
    return error_get_entry(depth)->type;
}

int error_code(size_t depth)
{
    return error_get_entry(depth)->code;
}

const char *error_name(size_t depth)
{
    return error_entry_get_name(error_get_entry(depth));
}

const char *error_desc(size_t depth)
{
    return error_entry_get_desc(error_get_entry(depth));
}

bool error_is_oom(size_t depth)
{
    return error_entry_is_oom(error_get_entry(depth));
}

bool error_check(size_t depth, size_t n, ...)
{
    error_entry_st *entry = error_get_entry(depth);
    va_list ap;
    int code;

    va_start(ap, n);

    for(; n; n--)
    {
        code = va_arg(ap, int);

        if(code == entry->code)
            return va_end(ap), true;
    }

    va_end(ap);

    return false;
}

const char *error_stack_get_func(size_t level)
{
    assert(level < errors.size);

    return errors.stack[level].func;
}

const error_type_st *error_stack_get_type(size_t level)
{
    assert(level < errors.size);

    return errors.stack[level].type;
}

int error_stack_get_code(size_t level)
{
    assert(level < errors.size);

    return errors.stack[level].code;
}

const char *error_stack_get_name(size_t level)
{
    assert(level < errors.size);

    return error_entry_get_name(&errors.stack[level]);
}

const char *error_stack_get_desc(size_t level)
{
    assert(level < errors.size);

    return error_entry_get_desc(&errors.stack[level]);
}

bool error_stack_is_oom(size_t level)
{
    assert(level < errors.size);

    return error_entry_is_oom(&errors.stack[level]);
}

void error_push_f(const char *func, const error_type_st *type, int code, const char *desc)
{
    error_entry_st *entry;
    size_t id;

    assert(type);
    assert(errors.size < ERROR_STACK_SIZE);

    id      = errors.size++;
    entry   = &errors.stack[id];

    entry->func = func;
    entry->type = type;
    entry->code = code;
    entry->desc = NULL;

    if(desc && (entry->desc = strdup(desc)))
        errors.clean++;
}

void error_push_last_f(const char *func, const error_type_st *type, const void *ctx)
{
    const char *desc = NULL;
    int code;

    assert(type);
    assert(type->error_last);

    code = type->error_last(type, &desc, (void *)ctx);
    error_push_f(func, type, code, desc);
}

void error_set_f(const char *func, const error_type_st *type, int code, const char *desc)
{
    error_clear();
    error_push_f(func, type, code, desc);
}

void error_set_last_f(const char *func, const error_type_st *type, const void *ctx)
{
    const char *desc = NULL;
    int code;

    assert(type);
    assert(type->error_last);

    code = type->error_last(type, &desc, (void *)ctx);
    error_set_f(func, type, code, desc);
}

void error_pass_f(const char *func)
{
    assert(errors.size);

    error_push_f(func, &ERROR_TYPE_GENERIC, E_GENERIC_PASS, NULL);
}

void error_skip_f(const char *func)
{
    assert(errors.size);

    error_push_f(func, &ERROR_TYPE_GENERIC, E_GENERIC_SKIP, NULL);
}

void error_wrap_f(const char *func)
{
    if(error_type(0) == &ERROR_TYPE_GENERIC && error_code(0) <= E_GENERIC_SYSTEM)
        error_pass_f(func);
    else if(error_is_oom(0))
        error_push_f(func, &ERROR_TYPE_GENERIC, E_GENERIC_OOM, NULL);
    else
        error_push_f(func, &ERROR_TYPE_GENERIC, E_GENERIC_WRAP, NULL);
}

void error_pack_f(const char *func, const error_type_st *type, int code, const char *desc)
{
    assert(type);

    if(error_type(0) == &ERROR_TYPE_GENERIC && error_code(0) <= E_GENERIC_SYSTEM)
        error_pass_f(func);
    else if(error_is_oom(0))
        error_push_f(func, &ERROR_TYPE_GENERIC, E_GENERIC_OOM, NULL);
    else
        error_push_f(func, type, code, desc);
}

void error_pack_last_f(const char *func, const error_type_st *type, const void *ctx)
{
    const char *desc = NULL;
    int code;

    assert(type);
    assert(type->error_last);

    code = type->error_last(type, &desc, (void *)ctx);
    error_pack_f(func, type, code, desc);
}

void error_map_f(const char *func, const error_type_st *type, error_map_cb map)
{
    int code;

    assert(type);

    if(error_type(0) == &ERROR_TYPE_GENERIC && error_code(0) <= E_GENERIC_SYSTEM)
        error_pass_f(func);
    else if(error_is_oom(0))
        error_push_f(func, &ERROR_TYPE_GENERIC, E_GENERIC_OOM, NULL);
    else if((code = map(error_type(0), error_code(0))) < 0)
        error_push_f(func, &ERROR_TYPE_GENERIC, E_GENERIC_WRAP, NULL);
    else
        error_push_f(func, type, code, NULL);
}

void error_pick_f(const char *func, int code)
{
    if(error_code(0) == code)
        error_skip_f(func);
    else
        error_wrap_f(func);
}

void error_lift_f(const char *func, int code)
{
    if(error_code(0) == code)
        error_skip_f(func);
    else
        error_pass_f(func);
}

void error_pass_sub_f(const char *func, const char *sub, const error_type_st *sub_type, int sub_code, const char *sub_desc)
{
    error_set_f(sub, sub_type, sub_code, sub_desc);
    error_pass_f(func);
}

void error_pass_last_sub_f(const char *func, const char *sub, const error_type_st *sub_type, const void *sub_ctx)
{
    const char *sub_desc = NULL;
    int sub_code;

    assert(sub_type);
    assert(sub_type->error_last);

    sub_code = sub_type->error_last(sub_type, &sub_desc, (void *)sub_ctx);
    error_pass_sub_f(func, sub, sub_type, sub_code, sub_desc);
}

void error_wrap_sub_f(const char *func, const char *sub, const error_type_st *sub_type, int sub_code, const char *sub_desc)
{
    error_set_f(sub, sub_type, sub_code, sub_desc);
    error_wrap_f(func);
}

void error_wrap_last_sub_f(const char *func, const char *sub, const error_type_st *sub_type, const void *sub_ctx)
{
    const char *sub_desc = NULL;
    int sub_code;

    assert(sub_type);
    assert(sub_type->error_last);

    sub_code = sub_type->error_last(sub_type, &sub_desc, (void *)sub_ctx);
    error_wrap_sub_f(func, sub, sub_type, sub_code, sub_desc);
}

void error_pack_sub_f(const char *func, const error_type_st *type, int code, const char *desc, const char *sub, const error_type_st *sub_type, int sub_code, const char *sub_desc)
{
    error_set_f(sub, sub_type, sub_code, sub_desc);
    error_pack_f(func, type, code, desc);
}

void error_pack_last_sub_f(const char *func, const error_type_st *type, int code, const char *desc, const char *sub, const error_type_st *sub_type, const void *sub_ctx)
{
    const char *sub_desc = NULL;
    int sub_code;

    assert(sub_type);
    assert(sub_type->error_last);

    sub_code = sub_type->error_last(sub_type, &sub_desc, (void *)sub_ctx);
    error_pack_sub_f(func, type, code, desc, sub, sub_type, sub_code, sub_desc);
}

void error_map_sub_f(const char *func, const error_type_st *type, error_map_cb map, const char *sub, const error_type_st *sub_type, int sub_code, const char *sub_desc)
{
    error_set_f(sub, sub_type, sub_code, sub_desc);
    error_map_f(func, type, map);
}

void error_map_last_sub_f(const char *func, const error_type_st *type, error_map_cb map, const char *sub, const error_type_st *sub_type, void *sub_ctx)
{
    const char *sub_desc = NULL;
    int sub_code;

    assert(sub_type);
    assert(sub_type->error_last);

    sub_code = sub_type->error_last(sub_type, &sub_desc, (void *)sub_ctx);
    error_map_sub_f(func, type, map, sub, sub_type, sub_code, sub_desc);
}


/// Define generic error info tuple.
///
/// \param _error   error ID
/// \param _desc    error description
///
/// \returns        error info tuple to be used in \a ERROR_DEFINE_LIST
#define ERROR_GENINFO(_error, _desc) \
    [-_error] = { .name = #_error, .desc = _desc }

/// generic error type definition
ERROR_DEFINE_LIST(GENERIC,
    ERROR_GENINFO(E_GENERIC_INVALID, "Invalid error."),
    ERROR_GENINFO(E_GENERIC_WRAP,    "WRAP error."),
    ERROR_GENINFO(E_GENERIC_PASS,    "PASS error."),
    ERROR_GENINFO(E_GENERIC_SKIP,    "SKIP error."),
    ERROR_GENINFO(E_GENERIC_SYSTEM,  "System error."),
    ERROR_GENINFO(E_GENERIC_OOM,     "Out of memory.")
);


/// Error name callback to retrieve ERRNO name.
///
/// \implements error_name_cb
static const char *error_errno_name(const error_type_st *type, int code, char *buf, size_t size)
{
    return strerrno(code);
}

/// Error description callback to retrieve ERRNO description.
///
/// \implements error_desc_cb
static const char *error_errno_desc(const error_type_st *type, int code, char *buf, size_t size)
{
    return strerror(code);
}

/// Error out-of-memory check callback to check ERRNO codes.
///
/// \implements error_oom_cb
static bool error_errno_is_oom(const error_type_st *type, int code)
{
    return code == ENOMEM;
}

/// Error retrieval callback to retrieve last ERRNO code.
///
/// \implements error_last_cb
static int error_errno_last(const error_type_st *type, const char **desc, void *ctx)
{
    return errno;
}

/// ERRNO error type definition
ERROR_DEFINE_CALLBACK(ERRNO,
    error_errno_name, error_errno_desc, error_errno_is_oom, error_errno_last);


#ifdef _WIN32

/// Error name callback to retrieve WIN32/HRESULT/NTSTATUS error name.
///
/// \implements error_name_cb
static const char *error_win_name(const error_type_st *type, int code, char *buf, size_t size)
{
    snprintf(buf, size, "%s_%08X", error_type_name(type), code);

    return buf;
}

/// Error description callback to retrieve WIN32 error description.
///
/// \implements error_desc_cb
static const char *error_win32_desc(const error_type_st *type, int code, char *buf, size_t size)
{
    DWORD rc;
    char *tmp;

    rc = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL, code, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        buf, size, NULL);

    if(rc)
    {
        for(; rc && (buf[rc - 1] == '\n' || buf[rc - 1] == '\r'); rc--)
            buf[rc - 1] = '\0';

        return buf;
    }

    if((rc = GetLastError()) != ERROR_INSUFFICIENT_BUFFER)
    {
        snprintf(buf, size, "<WIN32_FormatMessage_Error_%08lX>", rc);

        return buf;
    }

    rc = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_ALLOCATE_BUFFER,
        NULL, code, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (char *)&tmp, 0, NULL);

    if(!rc)
    {
        rc = GetLastError();
        snprintf(buf, size, "<WIN32_FormatMessage_Error_%08lX>", rc);

        return buf;
    }

    strncpy(buf, tmp, size);
    strncpy(&buf[size - 4], "...", 4);
    LocalFree(tmp);

    return buf;
}

/// Error out-of-memory check callback to check WIN32 codes.
///
/// \implements error_oom_cb
static bool error_win32_is_oom(const error_type_st *type, int code)
{
    return code == ERROR_NOT_ENOUGH_MEMORY || code == ERROR_OUTOFMEMORY;
}

/// Error retrieval callback to retrieve last WIN32 code.
///
/// \implements error_last_cb
static int error_win32_last(const error_type_st *type, const char **desc, void *ctx)
{
    return GetLastError();
}

/// WIN32 error type definition
ERROR_DEFINE_CALLBACK(WIN32,
    error_win_name, error_win32_desc, error_win32_is_oom, error_win32_last);


/// Error description callback to retrieve HRESULT error description.
///
/// \implements error_desc_cb
static const char *error_hresult_desc(const error_type_st *type, int code, char *buf, size_t size)
{
    return "<HRESULT_MESSAGE>"; /// \todo get HRESULT message
}

/// HRESULT error type definition
ERROR_DEFINE_CALLBACK(HRESULT, error_win_name, error_hresult_desc, NULL, NULL);


/// Error description callback to retrieve NTSTATUS error description.
///
/// \implements error_desc_cb
static const char *error_ntstatus_desc(const error_type_st *type, int code, char *buf, size_t size)
{
    return "<NTSTATUS_MESSAGE>"; /// \todo get NTSTATUS message
}

/// NTSTATUS error type definition
ERROR_DEFINE_CALLBACK(NTSTATUS, error_win_name, error_ntstatus_desc, NULL, NULL);

#endif /* ifdef _WIN32 */
