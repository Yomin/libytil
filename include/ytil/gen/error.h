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

#ifndef __YTIL_GEN_ERROR_H__
#define __YTIL_GEN_ERROR_H__

#include <errno.h>
#include <stddef.h>
#include <stdbool.h>
#include <sys/types.h>

#ifdef _WIN32
#   include <windows.h>
#endif

/*  to use module specific errors
    an 'error_infos' named array of error_info_st entries must exist
    
    use ERROR_INFO macro to add entries
    
    typedef enum module1_error
    {
          E_MODULE1_ERROR_1
        , E_MODULE1_ERROR_2
        , E_MODULE1_ERROR_3
    } module_error_id;
    
    static const error_info_st error_infos[] =
    {
          ERROR_INFO(E_MODULE1_ERROR_1, "Error 1 occured in module.")
        , ERROR_INFO(E_MODULE1_ERROR_2, "Error 2 occured in module.")
        , ERROR_INFO(E_MODULE1_ERROR_3, "Error 3 occured in module.")
    };
    
    int module1_func(void **ptr)
    {
        // set module error for gen-error functions
        if(!ptr)
            return error_set(E_MODULE1_ERROR_1), -1;
        
        // set errno error for errno functions
        if(!ptr)
            return errno_set(EINVAL), -1;
        
        // wrap errno errors of sub functions with generic wrapper error
        if(!(*ptr = malloc(123)))
            return error_wrap_errno(malloc), -1;
        
        // wrap errno errors of sub functions with module error
        if(!(*ptr = malloc(123)))
            return error_push_errno(E_MODULE1_ERROR_2, malloc), -1;
        
        // wrap errno error with transparent error, for wrapper functions
        if(!(*ptr = malloc(123)))
            return error_pass_errno(malloc), -1;
        
        // wrap gen-errors of sub functions with generic wrapper error
        if(!(*ptr = module2_func(123)))
            return error_wrap(), -1;
        
        // convert sub module error to parent module error
        if(module3_func())
            return error_map(E_MODULE3_ERROR, E_MODULE1_ERROR_3), -1;
        
        // wrap same module errors with transparent error
        return error_pass_int(module1_func2());
    }
*/


typedef enum error_error
{
      E_ERROR_UNSET = -1
    , E_ERROR_WRAP  = -2
    , E_ERROR_PASS  = -3
    , E_ERROR_SKIP  = -4
} error_error_id;

typedef enum error_type
{
      ERROR_TYPE_INVALID
    , ERROR_TYPE_ERROR
    , ERROR_TYPE_ERRNO
#ifdef _WIN32
    , ERROR_TYPE_WIN32
    , ERROR_TYPE_HRESULT
    , ERROR_TYPE_NTSTATUS
#endif
    , ERROR_TYPES
} error_type_id;

typedef struct error_info
{
    const char *name, *desc;
} error_info_st;

#define ERROR_INFO(_error, _desc) \
    [_error] = { .name = #_error, .desc = _desc }


// clear stack, push module error
void    _error_set(const char *func, const error_info_st *infos, size_t error);
#define  error_set(error) _error_set(__func__, error_infos, (error))

// push module error
void    _error_push(const char *func, const error_info_st *infos, size_t error);
#define  error_push(error) _error_push(__func__, error_infos, (error))

// push generic wrapper error
void    _error_wrap(const char *func);
#define  error_wrap() _error_wrap(__func__)

// push module error if last error not wrapper, else push generic wrapper error
void    _error_pack(const char *func, const error_info_st *infos, size_t error);
#define  error_pack(error) _error_pack(__func__, error_infos, (error))

// push mapped error or generic wrapper error
// error_map(0, E_CHILD_ERROR, E_PARENT_ERROR)
void    _error_map(const char *func, size_t depth, const error_info_st *infos, ...);
#define  error_map(depth, ...) _error_map(__func__, (depth), error_infos, __VA_ARGS__, E_ERROR_UNSET)

// push transparent pass error
void    _error_pass(const char *func);
#define  error_pass() _error_pass(__func__)

// push transparent skip error, makes underlying error also transparent
void    _error_skip(const char *func);
#define  error_skip() _error_skip(__func__)

// push transparent skip error if last error matches
// else push generic wrapper error
void    _error_pick(const char *func, size_t error);
#define  error_pick(error) _error_pick(__func__, (error))

// push transparent skip error if last error matches
// else push transparent pass error
void    _error_lift(const char *func, size_t error);
#define  error_lift(error) _error_lift(__func__, (error))


// clear stack, push errno error
void    _errno_set(const char *func, int error);
#define  errno_set(error) _errno_set(__func__, (error))

// push errno error
void    _errno_push(const char *func, int error);
#define  errno_push(error) _errno_push(__func__, (error))


// clear stack, push errno error from sub, push module error
void    _error_push_errno(const char *func, const error_info_st *infos, size_t error, const char *sub);
#define  error_push_errno(error, sub) _error_push_errno(__func__, error_infos, (error), #sub)

// clear stack, push errno error from sub, push generic wrapper error
void    _error_wrap_errno(const char *func, const char *sub);
#define  error_wrap_errno(sub) _error_wrap_errno(__func__, #sub)

// clear stack, push errno error from sub, push transparent pass error
void    _error_pass_errno(const char *func, const char *sub);
#define  error_pass_errno(sub) _error_pass_errno(__func__, #sub)


#ifdef _WIN32

// clear stack, push WIN32 error from sub, push module error
void    _error_push_win32(const char *func, const error_info_st *infos, size_t error, const char *sub, DWORD error32);
#define  error_push_win32(error, sub, error32) _error_push_win32(__func__, error_infos, (error), #sub, (error32))

// clear stack, push last WIN32 error from sub, push module error
void    _error_push_last_win32(const char *func, const error_info_st *infos, size_t error, const char *sub);
#define  error_push_last_win32(error, sub) _error_push_last_win32(__func__, error_infos, (error), #sub)

// clear stack, push WIN32 error from sub, push generic wrapper error
void    _error_wrap_win32(const char *func, const char *sub, DWORD error);
#define  error_wrap_win32(sub, error) _error_wrap_win32(__func__, #sub, (error))

// clear stack, push last WIN32 error from sub, push generic wrapper error
void    _error_wrap_last_win32(const char *func, const char *sub);
#define  error_wrap_last_win32(sub) _error_wrap_last_win32(__func__, #sub)

// clear stack, push WIN32 error from sub, push transparent pass error
void    _error_pass_win32(const char *func, const char *sub, DWORD error);
#define  error_pass_win32(sub, error) _error_pass_win32(__func__, #sub, (error))

// clear stack, push last WIN32 error from sub, push transparent pass error
void    _error_pass_last_win32(const char *func, const char *sub);
#define  error_pass_last_win32(sub) _error_pass_last_win32(__func__, #sub)


// clear stack, push HRESULT error from sub, push module error
void    _error_push_hresult(const char *func, const error_info_st *infos, size_t error, const char *sub, HRESULT result);
#define  error_push_hresult(error, sub, result) _error_push_hresult(__func__, error_infos, (error), #sub, (result))

// clear stack, push HRESULT error from sub, push generic wrapper error
void    _error_wrap_hresult(const char *func, const char *sub, HRESULT result);
#define  error_wrap_hresult(sub, result) _error_wrap_hresult(__func__, #sub, (result))

// clear stack, push HRESULT error from sub, push transparent pass error
void    _error_pass_hresult(const char *func, const char *sub, HRESULT result);
#define  error_pass_hresult(sub, result) _error_pass_hresult(__func__, #sub, (result))


// clear stack, push NTSTATUS error from sub, push module error
void    _error_push_ntstatus(const char *func, const error_info_st *infos, size_t error, const char *sub, NTSTATUS status);
#define  error_push_ntstatus(error, sub, status) _error_push_ntstatus(__func__, error_infos, (error), #sub, (status))

// clear stack, push NTSTATUS error from sub, push generic wrapper error
void    _error_wrap_ntstatus(const char *func, const char *sub, NTSTATUS status);
#define  error_wrap_ntstatus(sub, status) _error_wrap_ntstatus(__func__, #sub, (status))

// clear stack, push NTSTATUS error from sub, push transparent pass error
void    _error_pass_ntstatus(const char *func, const char *sub, NTSTATUS status);
#define  error_pass_ntstatus(sub, status) _error_pass_ntstatus(__func__, #sub, (status))

#endif // _WIN32


// convenience macro for enclosing functions to handle errors on the fly
#define error_proc(action, sub, cond, ...) __extension__ ({ \
    __auto_type rc = (sub); \
    \
    if(cond) \
        error_##action(__VA_ARGS__); \
    \
    rc; \
})

#define error_proc_int(action, sub, ...) error_proc(action, sub, rc < 0, __VA_ARGS__)
#define error_proc_ptr(action, sub, ...) error_proc(action, sub, !rc,    __VA_ARGS__)

#define error_push_int(err, sub) error_proc_int(push, sub, err)
#define error_push_ptr(err, sub) error_proc_ptr(push, sub, err)
#define error_wrap_int(sub)      error_proc_int(wrap, sub)
#define error_wrap_ptr(sub)      error_proc_ptr(wrap, sub)
#define error_pack_int(err, sub) error_proc_int(pack, sub, err)
#define error_pack_ptr(err, sub) error_proc_ptr(pack, sub, err)
#define error_pass_int(sub)      error_proc_int(pass, sub)
#define error_pass_ptr(sub)      error_proc_ptr(pass, sub)
#define error_skip_int(sub)      error_proc_int(skip, sub)
#define error_skip_ptr(sub)      error_proc_ptr(skip, sub)
#define error_pick_int(err, sub) error_proc_int(pick, sub, err)
#define error_pick_ptr(err, sub) error_proc_ptr(pick, sub, err)
#define error_lift_int(err, sub) error_proc_int(lift, sub, err)
#define error_lift_ptr(err, sub) error_proc_ptr(lift, sub, err)


// clear error stack
void   error_clear(void);
// get number of errors on stack
size_t error_depth(void);

// freeze error stack, modifications are silently ignored
void error_freeze(void);
// unfreeze error stack
void error_unfreeze(void);

#define error_freezer(sub) __extension__ ({ \
    error_freeze(); \
    __auto_type rc = (sub); \
    error_unfreeze(); \
    rc; \
})

// get error type name
const char *error_strtype(error_type_id type);

// get module error name
const char *error_name_error(const error_info_st *infos, size_t error);
// get errno name
const char *error_name_errno(int error);
#ifdef _WIN32
// get win32 error name
const char *error_name_win32(DWORD error);
// get HRESULT name
const char *error_name_hresult(HRESULT result);
// get NTSTATUS name
const char *error_name_ntstatus(NTSTATUS status);
#endif

// get module error description
const char *error_desc_error(const error_info_st *infos, size_t error);
// get errno description
const char *error_desc_errno(int error);
#ifdef _WIN32
// get win32 error description
const char *error_desc_win32(DWORD error);
// get HRESULT description
const char *error_desc_hresult(HRESULT result);
// get NTSTATUS description
const char *error_desc_ntstatus(NTSTATUS status);
#endif


// get module error from last error
ssize_t error_get(size_t depth);
// check if last module error matches error
bool    error_check(size_t depth, ssize_t error);
// get errno from last error
int     error_get_errno(size_t depth);
// check if last errno matches error
bool    error_check_errno(size_t depth, int error);

#ifdef _WIN32
// get win32 error from last error
DWORD    error_get_win32(size_t depth);
// check if last win32 error matches error
bool     error_check_win32(size_t depth, DWORD error);
// get HRESULT from last error
HRESULT  error_get_hresult(size_t depth);
// check if last HRESULT matches result
bool     error_check_hresult(size_t depth, HRESULT result);
// get NTSTATUS from last error
NTSTATUS error_get_ntstatus(size_t depth);
// check if last NTSTATUS matches status
bool     error_check_ntstatus(size_t depth, NTSTATUS status);
#endif

// get error type from last error
error_type_id error_type(size_t depth);
// get function from last error
const char   *error_func(size_t depth);
// get error name from last error
const char   *error_name(size_t depth);
// get error description from last error
const char   *error_desc(size_t depth);


// get module error from origin
ssize_t error_origin_get(void);
// check if origin module error matches error
bool    error_origin_check(ssize_t error);
// get errno from origin
int     error_origin_get_errno(void);
// check if origin errno matches error
bool    error_origin_check_errno(int error);

#ifdef _WIN32
// get win32 error from origin
DWORD    error_origin_get_win32(void);
// check if origin win32 error matches error
bool     error_origin_check_win32(DWORD error);
// get HRESULT from origin
HRESULT  error_origin_get_hresult(void);
// check if origin HRESULT matches result
bool     error_origin_check_hresult(HRESULT result);
// get NTSTATUS from origin
NTSTATUS error_origin_get_ntstatus(void);
// check if origin NTSTATUS matches status
bool     error_origin_check_ntstatus(NTSTATUS status);
#endif

// get error type from origin
error_type_id error_origin_type(void);
// get function from origin
const char   *error_origin_func(void);
// get error name from origin
const char   *error_origin_name(void);
// get error description from origin
const char   *error_origin_desc(void);


// get module error of stack level
ssize_t error_stack_get_error(size_t level);
// check if module error of stack level matches error
bool    error_stack_check_error(size_t level, ssize_t error);
// get errno of stack level
int     error_stack_get_errno(size_t level);
// check if errno of stack level matches error
bool    error_stack_check_errno(size_t level, int error);

#ifdef _WIN32
// get win32 error of stack level
DWORD    error_stack_get_win32(size_t level);
// check if win32 error of stack level matches error
bool     error_stack_check_win32(size_t level, DWORD error);
// get HRESULT of stack level
HRESULT  error_stack_get_hresult(size_t level);
// check if HRESULT of stack level matches result
bool     error_stack_check_hresult(size_t level, HRESULT result);
// get NTSTATUS of stack level
NTSTATUS error_stack_get_ntstatus(size_t level);
// check if NTSTATUS of stack level matches status
bool     error_stack_check_ntstatus(size_t level, NTSTATUS status);
#endif

// get error type of stack level
error_type_id error_stack_get_type(size_t level);
// get function of stack level
const char   *error_stack_get_func(size_t level);
// get error name of stack level
const char   *error_stack_get_name(size_t level);
// get error description of stack level
const char   *error_stack_get_desc(size_t level);

#endif
