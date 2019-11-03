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
        // set module1 error for local errors
        if(!ptr)
            return error_set(E_MODULE1_ERROR_1), -1;
        
        // set errno error for wrapper functions using errno
        if(!ptr)
            return error_set_errno(EINVAL), -1;
        
        // set errno error, add module1_func, errno is hidden
        // for gen_error functions with errno sub functions
        if(!(*ptr = malloc(123)))
            return error_wrap_errno(malloc), -1;
        
        // set errno error, add module1_func, errno is propagated
        // for wrapper functions using errno with errno sub functions
        if(!(*ptr = malloc(123)))
            return error_propagate_errno(malloc), -1;
        
        // add errno error, module2 error is hidden
        if(!(*ptr = module2_func(123)))
            return error_push_errno(EFAULT), -1;
        
        // add module1_func, module2 error is hidden
        if(!(*ptr = module2_func(123)))
            return error_wrap(), -1;
        
        // convert module3 error to module1 error
        if(module3_func())
            switch(error_get())
            {
            case E_MODULE3_ERROR:
                return error_push(E_MODULE1_ERROR_2), -1;
            default:
                return error_wrap(), -1;
            }
        
        // add module1_func but keep error module1_func2 error
        return error_propagate_int(module1_func2());
    }
*/


typedef struct error_info
{
    const char *name, *desc;
} error_info_st;

#define ERROR_INFO(_error, _desc) \
    [_error] = { .name = #_error, .desc = _desc }


// clear stack, push module error, update context
void    _error_set(const char *func, const error_info_st *infos, size_t error);
#define  error_set(error) _error_set(__func__, error_infos, (error))

// push module error, update context
void    _error_push(const char *func, const error_info_st *infos, size_t error);
#define  error_push(error) _error_push(__func__, error_infos, (error))

// push wrapper error, invalidate context
void    _error_wrap(const char *func);
#define  error_wrap() _error_wrap(__func__)

// push module error if context valid, else push wrapper error
void    _error_push_wrap(const char *func, const error_info_st *infos, size_t error);
#define  error_push_wrap(error) _error_push_wrap(__func__, error_infos, (error))

// push wrapper error, keep context
void    _error_propagate(const char *func);
#define  error_propagate() _error_propagate(__func__)

// clear stack, push errno error, update context
void    _error_set_errno(const char *func, int error);
#define  error_set_errno(error) _error_set_errno(__func__, (error))

// push errno error, update context
void    _error_push_errno(const char *func, int error);
#define  error_push_errno(error) _error_push_errno(__func__, (error))

// clear stack, push errno error from sub, push wrapper error, invalidate context
void    _error_wrap_errno(const char *func, const char *sub);
#define  error_wrap_errno(sub) _error_wrap_errno(__func__, #sub)

// clear stack, push errno error from sub, push wrapper error, keep context from errno error
void    _error_propagate_errno(const char *func, const char *sub);
#define  error_propagate_errno(sub) _error_propagate_errno(__func__, #sub)


// convenience macro for enclosing functions with int rc
// to wrap/propagate error on the fly
#define error_pack_int(action, sub) __extension__ ({ \
    __auto_type rc = (sub); \
    \
    if(rc < 0) \
        error_##action(); \
    \
    rc; \
})

// convenience macro for enclosing functions with pointer rc
// to wrap/propagate error on the fly
#define error_pack_ptr(action, sub) __extension__ ({ \
    __auto_type rc = (sub); \
    \
    if(!rc) \
        error_##action(); \
    \
    rc; \
})

#define error_wrap_int(sub)         error_pack_int(wrap, sub)
#define error_wrap_ptr(sub)         error_pack_ptr(wrap, sub)
#define error_propagate_int(sub)    error_pack_int(propagate, sub)
#define error_propagate_ptr(sub)    error_pack_ptr(propagate, sub)


// clear error stack
void error_clear(void);


// check if last error matches error code
bool error_check(ssize_t error);

// get error code from last error, -1 if unset
ssize_t     error_get(void);
// get function from last error, def if unset
const char *error_get_func(const char *def);
// get error name from last error, def if unset
const char *error_get_name(const char *def);
// get error description from last error, def if unset
const char *error_get_desc(const char *def);

// get error code from error origin, -1 if unset
ssize_t     error_origin_get(void);
// get function from error origin, def if unset
const char *error_origin_get_func(const char *def);
// get error name from error origin, def if unset
const char *error_origin_get_name(const char *def);
// get error description from error origin, def if unset
const char *error_origin_get_desc(const char *def);


// get number of errors on stack
size_t      error_stack_get_size(void);
// check if wrapper error on stack level
bool        error_stack_is_wrapper(size_t level);
// get error code of stack level, -1 if unset
ssize_t     error_stack_get_error(size_t level);
// get function of stack level, def if unset
const char *error_stack_get_func(size_t level, const char *def);
// get error name of stack level, def if unset
const char *error_stack_get_name(size_t level, const char *def);
// get error description of stack level, def if unset
const char *error_stack_get_desc(size_t level, const char *def);

#endif
