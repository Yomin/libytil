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

#ifndef YTIL_GEN_ERROR_H_INCLUDED
#define YTIL_GEN_ERROR_H_INCLUDED

#include <ytil/def/os.h>
#include <stddef.h>
#include <stdbool.h>


struct error_type;
typedef struct error_type error_type_st;    ///< error type

/// error name callback
///
/// \param type     error type
/// \param code     error code
/// \param buf      static buffer, may be used for dynamically created names
/// \param size     \p buf size
///
/// \returns        error name
typedef const char *(*error_name_cb)(const error_type_st *type, int code, char *buf, size_t size);

/// error description callback
///
/// \param type     error type
/// \param code     error code
/// \param buf      static buffer, may be used for dynamically created descriptions
/// \param size     \p buf size
///
/// \returns        error description
typedef const char *(*error_desc_cb)(const error_type_st *type, int code, char *buf, size_t size);

/// error out-of-memory callback
///
/// \param type     error type
/// \param code     error code
///
/// \retval true    code is out-of-memory error
/// \retval false   code is not out-of-memory error
typedef bool (*error_oom_cb)(const error_type_st *type, int code);

/// error retrieval callback
///
/// \param type     error type
/// \param desc     override description pointer, may be NULL
/// \param ctx_type error context type string
/// \param ctx      error context
///
/// \returns        last error code
typedef int (*error_last_cb)(const error_type_st *type, const char **desc, const char *ctx_type, void *ctx);

/// error map callback
///
/// \param type     error type of \p code
/// \param code     error code to map
///
/// \returns                    mapped error code
/// \retval E_GENERIC_WRAP      error code could not be mapped
typedef int (*error_map_cb)(const error_type_st *type, int code);


/// error callback interface
typedef struct error_callback
{
    error_name_cb   error_name;         ///< callback to retrieve error name
    error_desc_cb   error_desc;         ///< callback to retrieve error description
} error_callback_st;

/// error info
typedef struct error_info
{
    const char  *name;                  ///< error name
    const char  *desc;                  ///< error description
} error_info_st;

/// error list interface
typedef struct error_list
{
    const error_info_st *infos;         ///< info list
} error_list_st;

/// error interface type
typedef enum error_interface_type
{
    ERROR_INTERFACE_CALLBACK,           ///< callback interface
    ERROR_INTERFACE_LIST,               ///< list interface
} error_interface_id;

/// error interface
typedef union error_interface
{
    error_callback_st   callback;       ///< callback interface
    error_list_st       list;           ///< list interface
} error_interface_un;

/// error type
typedef struct error_type
{
    const char          *name;          ///< type name
    error_interface_id  iface_type;     ///< interface type
    error_interface_un  iface;          ///< interface
    error_oom_cb        error_is_oom;   ///< callback to identify out-of-memory errors
    error_last_cb       error_last;     ///< callback to retrieve last error
} error_type_st;

/// Define callback error type.
///
/// \param _name            type name
/// \param _error_name      callback to retrieve error name, may be NULL
/// \param _error_desc      callback to retrieve error description
/// \param _error_is_oom    callback to identify out-of-memory errors, may be NULL
/// \param _error_last      callback to retrieve last error, may be NULL
#define ERROR_DEFINE_CALLBACK(_name, _error_name, _error_desc, _error_is_oom, _error_last) \
    const error_type_st ERROR_TYPE_ ## _name =                                             \
    {                                                                                      \
        .name           = #_name,                                                          \
        .iface_type     = ERROR_INTERFACE_CALLBACK,                                        \
        .iface.callback =                                                                  \
        {                                                                                  \
            .error_name = _error_name,                                                     \
            .error_desc = _error_desc                                                      \
        },                                                                                 \
        .error_is_oom   = _error_is_oom,                                                   \
        .error_last     = _error_last                                                      \
    }

/// Define error info tuple.
///
/// \param _error   error ID
/// \param _desc    error description
///
/// \returns        error info tuple to be used in \a ERROR_DEFINE_LIST
#define ERROR_INFO(_error, _desc) \
    [_error] = { .name = #_error, .desc = _desc }

/// Define list error type.
///
/// \param _name    type name
/// \param ...      list of \a ERROR_INFO tuples
#define ERROR_DEFINE_LIST(_name, ...)                                \
    const error_type_st ERROR_TYPE_ ## _name =                       \
    {                                                                \
        .name               = #_name,                                \
        .iface_type         = ERROR_INTERFACE_LIST,                  \
        .iface.list.infos   = (const error_info_st[]){ __VA_ARGS__ } \
    }

/// Declare error type.
///
/// Use in header file to expose defined error type.
///
/// \param _name    type name
#define ERROR_DECLARE(_name) \
    extern const error_type_st ERROR_TYPE_ ## _name

/// Get error type.
///
/// \param _name    type name
#define ERROR_TYPE(_name) \
    &ERROR_TYPE_ ## _name


/// Get error type name.
///
/// \param type     error type
///
/// \returns        error type name
const char *error_type_name(const error_type_st *type);

/// Get error name.
///
/// \param type     error type
/// \param code     error code
///
/// \returns        error name
const char *error_type_get_name(const error_type_st *type, int code);

/// Get error description.
///
/// \param type     error type
/// \param code     error code
///
/// \returns        error description
const char *error_type_get_desc(const error_type_st *type, int code);

/// Check if error is out-of-memory error.
///
/// \param type     error type
/// \param code     error code
///
/// \retval true    error is out-of-memory error
/// \retval false   error is not out-of-memory error
bool error_type_is_oom(const error_type_st *type, int code);

/// Get last error.
///
/// \param type     error type
/// \param desc     override description pointer, may be NULL
///
/// \returns        last error
int error_type_get_last(const error_type_st *type, const char **desc);

/// Get last error from context.
///
/// \param type     error type
/// \param desc     override description pointer, may be NULL
/// \param ctx_type error context type string
/// \param ctx      error context
///
/// \returns        last error
int error_type_get_last_x(const error_type_st *type, const char **desc, const char *ctx_type, const void *ctx);


/// Clear error stack.
///
///
void error_clear(void);

/// Get number of errors on stack.
///
/// \returns    number of errors on stack
size_t error_depth(void);

/// Get error function name from stack.
///
/// \param depth    error depth, 0 is top level error
///
/// \returns        error function name
const char *error_func(size_t depth);

/// Get error type from stack.
///
/// \param depth    error depth, 0 is top level error
///
/// \returns        pointer to error type
const error_type_st *error_type(size_t depth);

/// Get error code from stack.
///
/// \param depth    error depth, 0 is top level error
///
/// \returns        error code
int error_code(size_t depth);

/// Get error name from stack.
///
/// \param depth    error depth, 0 is top level error
///
/// \returns        error name
const char *error_name(size_t depth);

/// Get error description from stack.
///
/// \param depth    error depth, 0 is top level error
///
/// \returns        error description
const char *error_desc(size_t depth);

/// Check if error on stack is out-of-memory error.
///
/// \param depth    error depth, 0 is top level error
///
/// \retval true    error is out-of-memory error
/// \retval false   error is not out-of-memory error
bool error_is_oom(size_t depth);

/// Check if error from stack matches one of given error codes.
///
/// \param depth    error depth, 0 is top level error
/// \param n        number of error codes
/// \param ...      list of error codes
bool error_check(size_t depth, size_t n, ...);


/// Get error function name from stack.
///
/// \param level    error level, 0 is original error
///
/// \returns        error function name
const char *error_stack_get_func(size_t level);

/// Get error type from stack.
///
/// \param level    error level, 0 is original error
///
/// \returns        pointer to error type
const error_type_st *error_stack_get_type(size_t level);

/// Get error code from stack.
///
/// \param level    error level, 0 is original error
///
/// \returns        error code
int error_stack_get_code(size_t level);

/// Get error name from stack.
///
/// \param level    error level, 0 is original error
///
/// \returns        error name
const char *error_stack_get_name(size_t level);

/// Get error description from stack.
///
/// \param level    error level, 0 is original error
///
/// \returns        error description
const char *error_stack_get_desc(size_t level);

/// Check if error on stack is out-of-memory error.
///
/// \param level    error level, 0 is original error
///
/// \retval true    error is out-of-memory error
/// \retval false   error is not out-of-memory error
bool error_stack_is_oom(size_t level);


/// Push error.
///
/// \param func     name of error setting function
/// \param type     error type pointer
/// \param code     error code
/// \param desc     error description override, may be NULL
void error_push_f(const char *func, const error_type_st *type, int code, const char *desc);

/// Push error of default type.
///
/// \param code     error code
#define error_push(code) \
    error_push_f(__func__, ERROR_TYPE(DEFAULT), (code), NULL)

/// Push error of default type with override description.
///
/// \param code     error code
/// \param desc     error description override
#define error_push_d(code, desc) \
    error_push_f(__func__, ERROR_TYPE(DEFAULT), (code), (desc))

/// Push error of specific type.
///
/// \param type     error type name
/// \param code     error code
#define error_push_s(type, code) \
    error_push_f(__func__, ERROR_TYPE(type), (code), NULL)

/// Push error of specific type with override description.
///
/// \param type     error type name
/// \param code     error code
/// \param desc     error description override
#define error_push_sd(type, code, desc) \
    error_push_f(__func__, ERROR_TYPE(type), (code), (desc))

/// Push last error.
///
/// \param func     name of error setting function
/// \param type     error type pointer
/// \param ctx_type error context type string
/// \param ctx      error context
void error_push_last_f(const char *func, const error_type_st *type, const char *ctx_type, const void *ctx);

/// Push last error of default type.
///
///
#define error_push_last() \
    error_push_last_f(__func__, ERROR_TYPE(DEFAULT), NULL, NULL)

/// Push last error of default type from context.
///
/// \param ctx_type error context type string
/// \param ctx      error context
#define error_push_last_x(ctx_type, ctx) \
    error_push_last_f(__func__, ERROR_TYPE(DEFAULT), (ctx_type), (ctx))

/// Push last error of specific type.
///
/// \param type     error type name
#define error_push_last_s(type) \
    error_push_last_f(__func__, ERROR_TYPE(type), NULL, NULL)

/// Push last error of specific type from context.
///
/// \param type     error type name
/// \param ctx_type error context type string
/// \param ctx      error context
#define error_push_last_sx(type, ctx_type, ctx) \
    error_push_last_f(__func__, ERROR_TYPE(type), (ctx_type), (ctx))

/// Clear stack and push error.
///
/// \param func     name of error setting function
/// \param type     error type pointer
/// \param code     error code
/// \param desc     error description override, may be NULL
void error_set_f(const char *func, const error_type_st *type, int code, const char *desc);

/// Clear stack and push error of default type.
///
/// \param code     error code
#define error_set(code) \
    error_set_f(__func__, ERROR_TYPE(DEFAULT), (code), NULL)

/// Clear stack and push error of default type with override description.
///
/// \param code     error code
/// \param desc     error description override
#define error_set_d(code, desc) \
    error_set_f(__func__, ERROR_TYPE(DEFAULT), (code), (desc))

/// Clear stack and push error of specific type.
///
/// \param type     error type name
/// \param code     error code
#define error_set_s(type, code) \
    error_set_f(__func__, ERROR_TYPE(type), (code), NULL)

/// Clear stack and push error of specific type with override description.
///
/// \param type     error type name
/// \param code     error code
/// \param desc     error description override
#define error_set_sd(type, code, desc) \
    error_set_f(__func__, ERROR_TYPE(type), (code), (desc))

/// Clear stack and push last error.
///
/// \param func     name of error setting function
/// \param type     error type pointer
/// \param ctx_type error context type string
/// \param ctx      error context
void error_set_last_f(const char *func, const error_type_st *type, const char *ctx_type, const void *ctx);

/// Clear stack and push last error of default type.
///
///
#define error_set_last() \
    error_set_last_f(__func__, ERROR_TYPE(DEFAULT), NULL, NULL)

/// Clear stack and push last error of default type from context.
///
/// \param ctx_type error context type string
/// \param ctx      error context
#define error_set_last_x(ctx_type, ctx) \
    error_set_last_f(__func__, ERROR_TYPE(DEFAULT), (ctx_type), (ctx))

/// Clear stack and push last error of specific type.
///
/// \param type     error type name
#define error_set_last_s(type) \
    error_set_last_f(__func__, ERROR_TYPE(type), NULL, NULL)

/// Clear stack and push last error of specific type from context.
///
/// \param type     error type name
/// \param ctx_type error context type string
/// \param ctx      error context
#define error_set_last_sx(type, ctx_type, ctx) \
    error_set_last_f(__func__, ERROR_TYPE(type), (ctx_type), (ctx))

/// Pass error.
///
/// Push generic pass error which is just skipped when retrieving errors.
///
/// \param func     name of error setting function
void error_pass_f(const char *func);

/// Pass error.
///
/// Push generic pass error which is just skipped when retrieving errors.
#define error_pass() \
    error_pass_f(__func__)

/// Skip error.
///
/// Push generic skip error.
/// When retrieving errors, the error itself and also the next deeper error are skipped.
///
/// \param func     name of error setting function
void error_skip_f(const char *func);

/// Skip error.
///
/// Push generic skip error.
/// When retrieving errors, the error itself and also the next deeper error are skipped.
#define error_skip() \
    error_skip_f(__func__)

/// Wrap error.
///
/// Push generic pass error if last error is a generic system error.
/// Push generic OOM error if last error is a type specific OOM error.
/// Else push generic wrapper error.
///
/// \param func     name of error setting function
void error_wrap_f(const char *func);

/// Wrap error.
///
/// Push generic pass error if last error is a generic system error.
/// Push generic OOM error if last error is a type specific OOM error.
/// Else push generic wrapper error.
#define error_wrap() \
    error_wrap_f(__func__)

/// Pack error.
///
/// Push generic pass error if last error is a generic system error.
/// Push generic OOM error if last error is a type specific OOM error.
/// Else push given error.
///
/// \param func     name of error setting function
/// \param type     error type pointer
/// \param code     error code
/// \param desc     error description override, may be NULL
void error_pack_f(const char *func, const error_type_st *type, int code, const char *desc);

/// Pack error of default type.
///
/// Push generic pass error if last error is a generic system error.
/// Push generic OOM error if last error is a type specific OOM error.
/// Else push given error.
///
/// \param code     error code
#define error_pack(code) \
    error_pack_f(__func__, ERROR_TYPE(DEFAULT), (code), NULL)

/// Pack error of default type with override description.
///
/// Push generic pass error if last error is a generic system error.
/// Push generic OOM error if last error is a type specific OOM error.
/// Else push given error.
///
/// \param code     error code
/// \param desc     error description override
#define error_pack_d(code, desc) \
    error_pack_f(__func__, ERROR_TYPE(DEFAULT), (code), (desc))

/// Pack error of specific type.
///
/// Push generic pass error if last error is a generic system error.
/// Push generic OOM error if last error is a type specific OOM error.
/// Else push given error.
///
/// \param type     error type name
/// \param code     error code
#define error_pack_s(type, code) \
    error_pack_f(__func__, ERROR_TYPE(type), (code), NULL)

/// Pack error of specific type with override description.
///
/// Push generic pass error if last error is a generic system error.
/// Push generic OOM error if last error is a type specific OOM error.
/// Else push given error.
///
/// \param type     error type name
/// \param code     error code
/// \param desc     error description override
#define error_pack_sd(type, code, desc) \
    error_pack_f(__func__, ERROR_TYPE(type), (code), (desc))

/// Pack last error.
///
/// Push generic pass error if last error is a generic system error.
/// Push generic OOM error if last error is a type specific OOM error.
/// Else push given error.
///
/// \param func     name of error setting function
/// \param type     error type pointer
/// \param ctx_type error context type string
/// \param ctx      error context
void error_pack_last_f(const char *func, const error_type_st *type, const char *ctx_type, const void *ctx);

/// Pack last error of default type.
///
/// Push generic pass error if last error is a generic system error.
/// Push generic OOM error if last error is a type specific OOM error.
/// Else push given error.
#define error_pack_last() \
    error_pack_last_f(__func__, ERROR_TYPE(DEFAULT), NULL, NULL)

/// Pack last error of default type from context.
///
/// Push generic pass error if last error is a generic system error.
/// Push generic OOM error if last error is a type specific OOM error.
/// Else push given error.
///
/// \param ctx_type error context type string
/// \param ctx      error context
#define error_pack_last_x(ctx_type, ctx) \
    error_pack_last_f(__func__, ERROR_TYPE(DEFAULT), (ctx_type), (ctx))

/// Pack last error of specific type.
///
/// Push generic pass error if last error is a generic system error.
/// Push generic OOM error if last error is a type specific OOM error.
/// Else push given error.
///
/// \param type     error type name
#define error_pack_last_s(type) \
    error_pack_last_f(__func__, ERROR_TYPE(type), NULL, NULL)

/// Pack last error of specific type from context.
///
/// Push generic pass error if last error is a generic system error.
/// Push generic OOM error if last error is a type specific OOM error.
/// Else push given error.
///
/// \param type     error type name
/// \param ctx_type error context type string
/// \param ctx      error context
#define error_pack_last_sx(type, ctx_type, ctx) \
    error_pack_last_f(__func__, ERROR_TYPE(type), (ctx_type), (ctx))

/// Map error.
///
/// Push generic pass error if last error is a generic system error.
/// Push generic OOM error if last error is a type specific OOM error.
/// Push generic wrap error if map callback returns E_GENERIC_WRAP.
/// Else push mapped error.
///
/// \param func     name of error setting function
/// \param type     error type pointer
/// \param map      error map callback
void error_map_f(const char *func, const error_type_st *type, error_map_cb map);

/// Map error to error of default type.
///
/// Push generic pass error if last error is a generic system error.
/// Push generic OOM error if last error is a type specific OOM error.
/// Push generic wrap error if map callback returns E_GENERIC_INVALID.
/// Else push mapped error.
///
/// \param map      error map callback
#define error_map(map) \
    error_map_f(__func__, ERROR_TYPE(DEFAULT), map)

/// Map error to error of specific type.
///
/// Push generic pass error if last error is a generic system error.
/// Push generic OOM error if last error is a type specific OOM error.
/// Push generic wrap error if map callback returns E_GENERIC_INVALID.
/// Else push mapped error.
///
/// \param type     error type pointer
/// \param map      error map callback
#define error_map_s(type, map) \
    error_map_f(__func__, ERROR_TYPE(type), map)

/// Pick error.
///
/// Push generic skip error if last error matches code.
/// Else push generic wrapper error.
///
/// \param func     name of error setting function
/// \param code     error code to match
void error_pick_f(const char *func, int code);

/// Pick error.
///
/// Push generic skip error if last error matches code.
/// Else push generic wrapper error.
///
/// \param code     error code to match
#define error_pick(code) \
    error_pick_f(__func__, (code))

/// Lift error.
///
/// Push generic skip error if last error matches code.
/// Else push generic pass error.
///
/// \param func     name of error setting function
/// \param code     error code to match
void error_lift_f(const char *func, int code);

/// Lift error.
///
/// Push generic skip error if last error matches code.
/// Else push generic pass error.
///
/// \param code     error code to match
#define error_lift(code) \
    error_lift_f(__func__, (code))

/// Pass sub function error.
///
/// Clear stack, push sub function error and generic pass error.
///
/// \param func         name of error setting function
/// \param sub          name of sub function
/// \param sub_type     sub error type pointer
/// \param sub_code     sub error code
/// \param sub_desc     sub error description override, may be NULL
void error_pass_sub_f(const char *func, const char *sub, const error_type_st *sub_type, int sub_code, const char *sub_desc);

/// Pass sub function error.
///
/// Clear stack, push sub function error and generic pass error.
///
/// \param sub          name of sub function
/// \param sub_type     sub error type name
/// \param sub_code     sub error code
#define error_pass_sub(sub, sub_type, sub_code) \
    error_pass_sub_f(__func__, #sub, ERROR_TYPE(sub_type), (sub_code), NULL)

/// Pass sub function error with override description.
///
/// Clear stack, push sub function error and generic pass error.
///
/// \param sub          name of sub function
/// \param sub_type     sub error type name
/// \param sub_code     sub error code
/// \param sub_desc     sub error description override
#define error_pass_sub_d(sub, sub_type, sub_code, sub_desc) \
    error_pass_sub_f(__func__, #sub, ERROR_TYPE(sub_type), (sub_code), (sub_desc))

/// Pass last sub function error.
///
/// Clear stack, push last sub function error and generic pass error.
///
/// \param func         name of error setting function
/// \param sub          name of sub function
/// \param sub_type     sub error type pointer
/// \param sub_ctx_type sub error context type string
/// \param sub_ctx      sub error context
void error_pass_last_sub_f(const char *func, const char *sub, const error_type_st *sub_type, const char *sub_ctx_type, const void *sub_ctx);

/// Pass last sub function error.
///
/// Clear stack, push last sub function error and generic pass error.
///
/// \param sub          name of sub function
/// \param sub_type     sub error type name
#define error_pass_last_sub(sub, sub_type) \
    error_pass_last_sub_f(__func__, #sub, ERROR_TYPE(sub_type), NULL, NULL)

/// Pass last sub function error from context.
///
/// Clear stack, push last sub function error and generic pass error.
///
/// \param sub          name of sub function
/// \param sub_type     sub error type name
/// \param sub_ctx_type sub error context type string
/// \param sub_ctx      sub error context
#define error_pass_last_sub_x(sub, sub_type, sub_ctx_type, sub_ctx) \
    error_pass_last_sub_f(__func__, #sub, ERROR_TYPE(sub_type), (sub_ctx_type), (sub_ctx))

/// Wrap sub function error.
///
/// Clear stack, push sub function error and generic wrap error.
/// If sub error is a sub type OOM error, push generic OOM error instead.
///
/// \param func         name of error setting function
/// \param sub          name of sub function
/// \param sub_type     sub error type pointer
/// \param sub_code     sub error code
/// \param sub_desc     sub error description override, may be NULL
void error_wrap_sub_f(const char *func, const char *sub, const error_type_st *sub_type, int sub_code, const char *sub_desc);

/// Wrap sub function error.
///
/// Clear stack, push sub function error and generic wrap error.
/// If sub error is a sub type OOM error, push generic OOM error instead.
///
/// \param sub          name of sub function
/// \param sub_type     sub error type name
/// \param sub_code     sub error code
#define error_wrap_sub(sub, sub_type, sub_code) \
    error_wrap_sub_f(__func__, #sub, ERROR_TYPE(sub_type), (sub_code), NULL)

/// Wrap sub function error with override description.
///
/// Clear stack, push sub function error and generic wrap error.
/// If sub error is a sub type OOM error, push generic OOM error instead.
///
/// \param sub          name of sub function
/// \param sub_type     sub error type name
/// \param sub_code     sub error code
/// \param sub_desc     sub error description override
#define error_wrap_sub_d(sub, sub_type, sub_code, sub_desc) \
    error_wrap_sub_f(__func__, #sub, ERROR_TYPE(sub_type), (sub_code), (sub_desc))

/// Wrap last sub function error.
///
/// Clear stack, push last sub function error and generic wrap error.
/// If sub error is a sub type OOM error, push generic OOM error instead.
///
/// \param func         name of error setting function
/// \param sub          name of sub function
/// \param sub_type     sub error type pointer
/// \param sub_ctx_type sub error context type string
/// \param sub_ctx      sub error context
void error_wrap_last_sub_f(const char *func, const char *sub, const error_type_st *sub_type, const char *sub_ctx_type, const void *sub_ctx);

/// Wrap last sub function error.
///
/// Clear stack, push last sub function error and generic wrap error.
/// If sub error is a sub type OOM error, push generic OOM error instead.
///
/// \param sub          name of sub function
/// \param sub_type     sub error type name
#define error_wrap_last_sub(sub, sub_type) \
    error_wrap_last_sub_f(__func__, #sub, ERROR_TYPE(sub_type), NULL, NULL)

/// Wrap last sub function error from context.
///
/// Clear stack, push last sub function error and generic wrap error.
/// If sub error is a sub type OOM error, push generic OOM error instead.
///
/// \param sub          name of sub function
/// \param sub_type     sub error type name
/// \param sub_ctx_type sub error context type string
/// \param sub_ctx      sub error context
#define error_wrap_last_sub_x(sub, sub_type, sub_ctx_type, sub_ctx) \
    error_wrap_last_sub_f(__func__, #sub, ERROR_TYPE(sub_type), (sub_ctx_type), (sub_ctx))

/// Pack sub function error.
///
/// Clear stack, push sub function error and pack given error.
/// If sub error is a sub type OOM error, push generic OOM error instead.
///
/// \param func         name of error setting function
/// \param type         error type pointer
/// \param code         error code
/// \param desc         error description override, may be NULL
/// \param sub          name of sub function
/// \param sub_type     sub error type pointer
/// \param sub_code     sub error code
/// \param sub_desc     sub error description override, may be NULL
void error_pack_sub_f(const char *func, const error_type_st *type, int code, const char *desc, const char *sub, const error_type_st *sub_type, int sub_code, const char *sub_desc);

/// Pack sub function error with error of default type.
///
/// Clear stack, push sub function error and pack given error.
/// If sub error is a sub type OOM error, push generic OOM error instead.
///
/// \param code         error code
/// \param sub          name of sub function
/// \param sub_type     sub error type name
/// \param sub_code     sub error code
#define error_pack_sub(code, sub, sub_type, sub_code)             \
    error_pack_sub_f(__func__, ERROR_TYPE(DEFAULT), (code), NULL, \
        #sub, ERROR_TYPE(sub_type), (sub_code), NULL)

/// Pack sub function error with override description with error of default type.
///
/// Clear stack, push sub function error and pack given error.
/// If sub error is a sub type OOM error, push generic OOM error instead.
///
/// \param code         error code
/// \param sub          name of sub function
/// \param sub_type     sub error type name
/// \param sub_code     sub error code
/// \param sub_desc     sub error description override
#define error_pack_sub_d(code, sub, sub_type, sub_code, sub_desc) \
    error_pack_sub_f(__func__, ERROR_TYPE(DEFAULT), (code), NULL, \
        #sub, ERROR_TYPE(sub_type), (sub_code), (sub_desc))

/// Pack last sub function error.
///
/// Clear stack, push last sub function error and pack given error.
///
/// \param func         name of error setting function
/// \param type         error type pointer
/// \param code         error code
/// \param desc         error description override, may be NULL
/// \param sub          name of sub function
/// \param sub_type     sub error type pointer
/// \param sub_ctx_type sub error context type string
/// \param sub_ctx      sub error context
void error_pack_last_sub_f(const char *func, const error_type_st *type, int code, const char *desc, const char *sub, const error_type_st *sub_type, const char *sub_ctx_type, const void *sub_ctx);

/// Pack last sub function error with error of default type.
///
/// Clear stack, push last sub function error and pack given error.
///
/// \param code         error code
/// \param sub          name of sub function
/// \param sub_type     sub error type name
#define error_pack_last_sub(code, sub, sub_type)                       \
    error_pack_last_sub_f(__func__, ERROR_TYPE(DEFAULT), (code), NULL, \
        #sub, ERROR_TYPE(sub_type), NULL, NULL)

/// Pack last sub function error from context with error of default type.
///
/// Clear stack, push last sub function error and pack given error.
///
/// \param code         error code
/// \param sub          name of sub function
/// \param sub_type     sub error type name
/// \param sub_ctx_type sub error context type string
/// \param sub_ctx      sub error context
#define error_pack_last_sub_x(code, sub, sub_type, sub_ctx_type, sub_ctx) \
    error_pack_last_sub_f(__func__, ERROR_TYPE(DEFAULT), (code), NULL,    \
        #sub, ERROR_TYPE(sub_type), (sub_ctx_type), (sub_ctx))

/// Map sub function error.
///
/// Clear stack, push sub function error and map error.
/// If sub error is a sub type OOM error, push generic OOM error instead.
///
/// \param func         name of error setting function
/// \param type         error type pointer
/// \param map          error map callback
/// \param sub          name of sub function
/// \param sub_type     sub error type pointer
/// \param sub_code     sub error code
/// \param sub_desc     sub error description override, may be NULL
void error_map_sub_f(const char *func, const error_type_st *type, error_map_cb map, const char *sub, const error_type_st *sub_type, int sub_code, const char *sub_desc);

/// Map sub function error to error of default type.
///
/// Clear stack, push sub function error and map error.
/// If sub error is a sub type OOM error, push generic OOM error instead.
///
/// \param map          error map callback
/// \param sub          name of sub function
/// \param sub_type     sub error type pointer
/// \param sub_code     sub error code
#define error_map_sub(map, sub, sub_type, sub_code)     \
    error_map_sub_f(__func__, ERROR_TYPE(DEFAULT), map, \
        #sub, ERROR_TYPE(sub_type), (sub_code), NULL)

/// Map sub function error with override description to error of default type.
///
/// Clear stack, push sub function error and map error.
/// If sub error is a sub type OOM error, push generic OOM error instead.
///
/// \param map          error map callback
/// \param sub          name of sub function
/// \param sub_type     sub error type pointer
/// \param sub_code     sub error code
/// \param sub_desc     sub error description override
#define error_map_sub_d(map, sub, sub_type, sub_code, sub_desc) \
    error_map_sub_f(__func__, ERROR_TYPE(DEFAULT), map,         \
        #sub, ERROR_TYPE(sub_type), (sub_code), (sub_desc))

/// Map last sub function error.
///
/// Clear stack, push last sub function error and map error.
/// If sub error is a sub type OOM error, push generic OOM error instead.
///
/// \param func         name of error setting function
/// \param type         error type pointer
/// \param map          error map callback
/// \param sub          name of sub function
/// \param sub_type     sub error type pointer
/// \param sub_ctx_type sub error context type string
/// \param sub_ctx      sub error context
void error_map_last_sub_f(const char *func, const error_type_st *type, error_map_cb map, const char *sub, const error_type_st *sub_type, const char *sub_ctx_type, const void *sub_ctx);

/// Map last sub function error to error of default type.
///
/// Clear stack, push last sub function error and map error.
/// If sub error is a sub type OOM error, push generic OOM error instead.
///
/// \param map          error map callback
/// \param sub          name of sub function
/// \param sub_type     sub error type pointer
#define error_map_last_sub(map, sub, sub_type)               \
    error_map_last_sub_f(__func__, ERROR_TYPE(DEFAULT), map, \
        #sub, ERROR_TYPE(sub_type), NULL, NULL)

/// Map last sub function error from context to error of default type.
///
/// Clear stack, push last sub function error and map error.
/// If sub error is a sub type OOM error, push generic OOM error instead.
///
/// \param map          error map callback
/// \param sub          name of sub function
/// \param sub_type     sub error type pointer
/// \param sub_ctx_type sub error context type string
/// \param sub_ctx      sub error context
#define error_map_last_sub_x(map, sub, sub_type, sub_ctx_type, sub_ctx) \
    error_map_last_sub_f(__func__, ERROR_TYPE(DEFAULT), map,            \
        #sub, ERROR_TYPE(sub_type), (sub_ctx_type), (sub_ctx))

/// Map prior sub function error.
///
/// Change function name of prior sub function error and map error.
/// If sub error is a sub type OOM error, push generic OOM error instead.
///
/// \param func         name of error setting function
/// \param type         error type pointer
/// \param map          error map callback
/// \param sub          name of sub function
void error_map_pre_sub_f(const char *func, const error_type_st *type, error_map_cb map, const char *sub);

/// Map prior sub function error to error of default type.
///
/// Change function name of prior sub function error and map error.
/// If sub error is a sub type OOM error, push generic OOM error instead.
///
/// \param map          error map callback
/// \param sub          name of sub function
#define error_map_pre_sub(map, sub) \
    error_map_pre_sub_f(__func__, ERROR_TYPE(DEFAULT), map, #sub)


/// Convenience macro for enclosing functions to handle errors on the fly.
///
/// \param sub      encapsulated function
/// \param action   error action
/// \param cond     condition that is true if sub_rc indicates error
/// \param ...      error action args
///
/// \returns        \p sub rc
#define error_proc(sub, cond, action, ...) __extension__ ({ \
    __auto_type sub_rc = (sub);                             \
                                                            \
    if(cond)                                                \
        error_ ## action(__VA_ARGS__);                      \
                                                            \
    sub_rc;                                                 \
})

/// Convenience macro for enclosing int functions to handle errors on the fly.
///
/// \param sub      encapsulated function
/// \param action   error action
/// \param ...      error action args
///
/// \returns        \p sub rc
#define error_proc_int(sub, action, ...) \
    error_proc(sub, sub_rc < 0, action, __VA_ARGS__)

/// Convenience macro for enclosing pointer functions to handle errors on the fly.
///
/// \param sub      encapsulated function
/// \param action   error action
/// \param ...      error action args
///
/// \returns        \p sub rc
#define error_proc_ptr(sub, action, ...) \
    error_proc(sub, !sub_rc, action, __VA_ARGS__)

/// Convenience macro for enclosing arbitrary rc functions to handle errors on the fly.
///
/// \param sub      encapsulated function
/// \param rc       error rc
/// \param action   error action
/// \param ...      error action args
///
/// \returns        \p sub rc
#define error_proc_rc(sub, rc, action, ...) \
    error_proc(sub, sub_rc == (rc), action, __VA_ARGS__)

/// Convenience error push macro for enclosing int functions.
///
/// \param err      error to push
/// \param sub      encapsulated function
///
/// \returns        \p sub rc
#define error_push_int(err, sub) \
    error_proc_int(sub, push, err)

/// Convenience error push macro for enclosing pointer functions.
///
/// \param err      error to push
/// \param sub      encapsulated function
///
/// \returns        \p sub rc
#define error_push_ptr(err, sub) \
    error_proc_ptr(sub, push, err)

/// Convenience error push macro for enclosing arbitrary rc functions.
///
/// \param rc       error rc
/// \param err      error to push
/// \param sub      encapsulated function
///
/// \returns        \p sub rc
#define error_push_rc(rc, err, sub) \
    error_proc_rc(sub, rc, push, err)

/// Convenience error wrap macro for enclosing int functions.
///
/// \param sub      encapsulated function
///
/// \returns        \p sub rc
#define error_wrap_int(sub) \
    error_proc_int(sub, wrap)

/// Convenience error wrap macro for enclosing pointer functions.
///
/// \param sub      encapsulated function
///
/// \returns        \p sub rc
#define error_wrap_ptr(sub) \
    error_proc_ptr(sub, wrap)

/// Convenience error wrap macro for enclosing arbitrary rc functions.
///
/// \param rc       error rc
/// \param sub      encapsulated function
///
/// \returns        \p sub rc
#define error_wrap_rc(rc, sub) \
    error_proc_rc(sub, rc, wrap)

/// Convenience error pack macro for enclosing int functions.
///
/// \param err      error to pack
/// \param sub      encapsulated function
///
/// \returns        \p sub rc
#define error_pack_int(err, sub) \
    error_proc_int(sub, pack, err)

/// Convenience error pack macro for enclosing pointer functions.
///
/// \param err      error to pack
/// \param sub      encapsulated function
///
/// \returns        \p sub rc
#define error_pack_ptr(err, sub) \
    error_proc_ptr(sub, pack, err)

/// Convenience error pack macro for enclosing arbitrary rc functions.
///
/// \param rc       error rc
/// \param err      error to pack
/// \param sub      encapsulated function
///
/// \returns        \p sub rc
#define error_pack_rc(rc, err, sub) \
    error_proc_rc(sub, rc, pack, err)

/// Convenience error pass macro for enclosing int functions.
///
/// \param sub      encapsulated function
///
/// \returns        \p sub rc
#define error_pass_int(sub) \
    error_proc_int(sub, pass)

/// Convenience error pass macro for enclosing pointer functions.
///
/// \param sub      encapsulated function
///
/// \returns        \p sub rc
#define error_pass_ptr(sub) \
    error_proc_ptr(sub, pass)

/// Convenience error pass macro for enclosing arbitrary rc functions.
///
/// \param rc       error rc
/// \param sub      encapsulated function
///
/// \returns        \p sub rc
#define error_pass_rc(rc, sub) \
    error_proc_rc(sub, rc, pass)

/// Convenience error skip macro for enclosing int functions.
///
/// \param sub      encapsulated function
///
/// \returns        \p sub rc
#define error_skip_int(sub) \
    error_proc_int(sub, skip)

/// Convenience error pass macro for enclosing pointer functions.
///
/// \param sub      encapsulated function
///
/// \returns        \p sub rc
#define error_skip_ptr(sub) \
    error_proc_ptr(sub, skip)

/// Convenience error pass macro for enclosing arbitrary functions.
///
/// \param rc       error rc
/// \param sub      encapsulated function
///
/// \returns        \p sub rc
#define error_skip_rc(rc, sub) \
    error_proc_rc(sub, rc, skip)

/// Convenience error pick macro for enclosing int functions.
///
/// \param err      error to pick
/// \param sub      encapsulated function
///
/// \returns        \p sub rc
#define error_pick_int(err, sub) \
    error_proc_int(sub, pick, err)

/// Convenience error pick macro for enclosing pointer functions.
///
/// \param err      error to pick
/// \param sub      encapsulated function
///
/// \returns        \p sub rc
#define error_pick_ptr(err, sub) \
    error_proc_ptr(sub, pick, err)

/// Convenience error pick macro for enclosing arbitrary rc functions.
///
/// \param rc       error rc
/// \param err      error to pick
/// \param sub      encapsulated function
///
/// \returns        \p sub rc
#define error_pick_rc(rc, err, sub) \
    error_proc_rc(sub, rc, pick, err)

/// Convenience error lift macro for enclosing int functions.
///
/// \param err      error to lift
/// \param sub      encapsulated function
///
/// \returns        \p sub rc
#define error_lift_int(err, sub) \
    error_proc_int(sub, lift, err)

/// Convenience error lift macro for enclosing pointer functions.
///
/// \param err      error to lift
/// \param sub      encapsulated function
///
/// \returns        \p sub rc
#define error_lift_ptr(err, sub) \
    error_proc_ptr(sub, lift, err)

/// Convenience error lift macro for enclosing arbitrary rc functions.
///
/// \param rc       error rc
/// \param err      error to lift
/// \param sub      encapsulated function
///
/// \returns        \p sub rc
#define error_lift_rc(rc, err, sub) \
    error_proc_rc(sub, rc, lift, err)


/// generic error type declaration
ERROR_DECLARE(GENERIC);

/// generic error
typedef enum generic_error
{
    E_GENERIC_INVALID   = -1,   ///< invalid error
    E_GENERIC_WRAP      = -2,   ///< WRAP error
    E_GENERIC_PASS      = -3,   ///< PASS error
    E_GENERIC_SKIP      = -4,   ///< SKIP error
    E_GENERIC_SYSTEM    = -10,  ///< generic SYSTEM error
    E_GENERIC_OOM       = -11,  ///< generic out-of-memory error
} generic_error_id;


/// ERRNO error type declaration
ERROR_DECLARE(ERRNO);

/// Pass sub function errno.
///
/// Clear stack, push sub function errno and generic pass error.
///
/// \param sub          name of sub function
/// \param code         ERRNO
#define error_pass_errno(sub, code) \
    error_pass_sub(sub, ERRNO, (code))

/// Pass last sub function errno.
///
/// Clear stack, push last sub function errno and generic pass error.
///
/// \param sub          name of sub function
#define error_pass_last_errno(sub) \
    error_pass_last_sub(sub, ERRNO)

/// Wrap sub function errno.
///
/// Clear stack, push sub function errno and generic wrap error.
/// If sub errno is ENOMEM, push generic OOM error instead.
///
/// \param sub          name of sub function
/// \param code         ERRNO
#define error_wrap_errno(sub, code) \
    error_wrap_sub(sub, ERRNO, (code))

/// Wrap last sub function errno.
///
/// Clear stack, push last sub function errno and generic wrap error.
/// If sub errno is ENOMEM, push generic OOM error instead.
///
/// \param sub          name of sub function
#define error_wrap_last_errno(sub) \
    error_wrap_last_sub(sub, ERRNO)

/// Pack sub function errno with error of default type.
///
/// Clear stack, push sub function errno and pack given error.
/// If sub errno is ENOMEM, push generic OOM error instead.
///
/// \param code         error code
/// \param sub          name of sub function
/// \param sub_code     ERRNO
#define error_pack_errno(code, sub, sub_code) \
    error_pack_sub((code), sub, ERRNO, (sub_code))

/// Pack last sub function errno with error of default type.
///
/// Clear stack, push last sub function errno and pack given error.
/// If sub errno is ENOMEM, push generic OOM error instead.
///
/// \param code         error code
/// \param sub          name of sub function
#define error_pack_last_errno(code, sub) \
    error_pack_last_sub((code), sub, ERRNO)

/// Map sub function errno to error of default type.
///
/// Clear stack, push sub function errno and map error.
/// If sub errno is ENOMEM, push generic OOM error instead.
///
/// \param map          error map callback
/// \param sub          name of sub function
/// \param sub_code     ERRNO
#define error_map_errno(map, sub, sub_code) \
    error_map_sub((map), sub, ERRNO, (sub_code))

/// Map last sub function errno to error of default type.
///
/// Clear stack, push last sub function errno and map error.
/// If sub errno is ENOMEM, push generic OOM error instead.
///
/// \param map          error map callback
/// \param sub          name of sub function
#define error_map_last_errno(map, sub) \
    error_map_last_sub((map), sub, ERRNO)


#if OS_WINDOWS

/// EWIN32 error type declaration
ERROR_DECLARE(EWIN32);

/// Pass sub function EWIN32 error.
///
/// Clear stack, push sub function EWIN32 error and generic pass error.
///
/// \param sub          name of sub function
/// \param code         EWIN32 error
#define error_pass_ewin32(sub, code) \
    error_pass_sub(sub, EWIN32, (code))

/// Pass last sub function EWIN32 error.
///
/// Clear stack, push last sub function EWIN32 error and generic pass error.
///
/// \param sub          name of sub function
#define error_pass_last_ewin32(sub) \
    error_pass_last_sub(sub, EWIN32)

/// Wrap sub function EWIN32 error.
///
/// Clear stack, push sub function EWIN32 error and generic wrap error.
/// If sub error is ERROR_NOT_ENOUGH_MEMORY or ERROR_OUTOFMEMORY,
/// push generic OOM error instead.
///
/// \param sub          name of sub function
/// \param code         EWIN32 error
#define error_wrap_ewin32(sub, code) \
    error_wrap_sub(sub, EWIN32, (code))

/// Wrap last sub function EWIN32 error.
///
/// Clear stack, push last sub function EWIN32 error and generic wrap error.
/// If sub error is ERROR_NOT_ENOUGH_MEMORY or ERROR_OUTOFMEMORY,
/// push generic OOM error instead.
///
/// \param sub          name of sub function
#define error_wrap_last_ewin32(sub) \
    error_wrap_last_sub(sub, EWIN32)

/// Pack sub function EWIN32 error with error of default type.
///
/// Clear stack, push sub function EWIN32 error and pack given error.
/// If sub error is ERROR_NOT_ENOUGH_MEMORY or ERROR_OUTOFMEMORY,
/// push generic OOM error instead.
///
/// \param code         error code
/// \param sub          name of sub function
/// \param sub_code     EWIN32 error
#define error_pack_ewin32(code, sub, sub_code) \
    error_pack_sub((code), sub, EWIN32, (sub_code))

/// Pack last sub function EWIN32 error with error of default type.
///
/// Clear stack, push last sub function EWIN32 error and pack given error.
/// If sub error is ERROR_NOT_ENOUGH_MEMORY or ERROR_OUTOFMEMORY,
/// push generic OOM error instead.
///
/// \param code         error code
/// \param sub          name of sub function
#define error_pack_last_ewin32(code, sub) \
    error_pack_last_sub((code), sub, EWIN32)

/// Map sub function EWIN32 error to error of default type.
///
/// Clear stack, push sub function EWIN32 error and map error.
/// If sub error is ERROR_NOT_ENOUGH_MEMORY or ERROR_OUTOFMEMORY,
/// push generic OOM error instead.
///
/// \param map          error map callback
/// \param sub          name of sub function
/// \param sub_code     EWIN32 error
#define error_map_ewin32(map, sub, sub_code) \
    error_map_sub((map), sub, EWIN32, (sub_code))

/// Map last sub function EWIN32 error to error of default type.
///
/// Clear stack, push last sub function EWIN32 error and map error.
/// If sub error is ERROR_NOT_ENOUGH_MEMORY or ERROR_OUTOFMEMORY,
/// push generic OOM error instead.
///
/// \param map          error map callback
/// \param sub          name of sub function
#define error_map_last_ewin32(map, sub) \
    error_map_last_sub((map), sub, EWIN32)


/// HRESULT error type declaration
ERROR_DECLARE(HRESULT);

/// Pass sub function HRESULT error.
///
/// Clear stack, push sub function HRESULT error and generic pass error.
///
/// \param sub          name of sub function
/// \param result       HRESULT
#define error_pass_hresult(sub, result) \
    error_pass_sub(sub, HRESULT, (result))

/// Wrap sub function HRESULT error.
///
/// Clear stack, push sub function HRESULT error and generic wrap error.
///
/// \param sub          name of sub function
/// \param result       HRESULT
#define error_wrap_hresult(sub, result) \
    error_wrap_sub(sub, HRESULT, (result))

/// Pack sub function HRESULT error with error of default type.
///
/// Clear stack, push sub function HRESULT error and pack given error.
///
/// \param code         error code
/// \param sub          name of sub function
/// \param result       HRESULT
#define error_pack_hresult(code, sub, result) \
    error_pack_sub((code), sub, HRESULT, (result))

/// Map sub function HRESULT error to error of default type.
///
/// Clear stack, push sub function HRESULT error and map error.
///
/// \param map          error map callback
/// \param sub          name of sub function
/// \param result       HRESULT
#define error_map_hresult(map, sub, result) \
    error_map_sub((map), sub, HRESULT, (result))


/// NTSTATUS error type declaration
ERROR_DECLARE(NTSTATUS);

/// Pass sub function NTSTATUS error.
///
/// Clear stack, push sub function NTSTATUS error and generic pass error.
///
/// \param sub          name of sub function
/// \param status       NTSTATUS
#define error_pass_ntstatus(sub, status) \
    error_pass_sub(sub, NTSTATUS, (status))

/// Wrap sub function NTSTATUS error.
///
/// Clear stack, push sub function NTSTATUS error and generic wrap error.
///
/// \param sub          name of sub function
/// \param status       NTSTATUS
#define error_wrap_ntstatus(sub, status) \
    error_wrap_sub(sub, NTSTATUS, (status))

/// Pack sub function NTSTATUS error with error of default type.
///
/// Clear stack, push sub function NTSTATUS error and pack given error.
///
/// \param code         error code
/// \param sub          name of sub function
/// \param status       NTSTATUS
#define error_pack_ntstatus(code, sub, status) \
    error_pack_sub((code), sub, NTSTATUS, (status))

/// Map sub function NTSTATUS error to error of default type.
///
/// Clear stack, push sub function NTSTATUS error and map error.
///
/// \param map          error map callback
/// \param sub          name of sub function
/// \param status       NTSTATUS
#define error_map_ntstatus(map, sub, status) \
    error_map_sub((map), sub, NTSTATUS, (status))

#endif // if OS_WINDOWS


#endif // ifndef YTIL_GEN_ERROR_H_INCLUDED
