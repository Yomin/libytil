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
        // set module specific error origin for local errors
        if(!ptr)
            return error_set(E_MODULE1_ERROR_1), -1;
        
        // set errno error origin for functions not using gen_error
        if(!(*ptr = malloc(123)))
            return error_set_errno(malloc), -1;
        
        // module2 set error origin, just add module1_func to error stack
        if(module2_func(*ptr))
            return error_push(), -1;
        
        // convert module3 error to module1 error
        if(module3_func())
            switch(error_get())
            {
            case E_MODULE3_ERROR:
                return error_set(E_MODULE1_ERROR_2), -1;
            default:
                return error_push(), -1;
            }
        
        // add module1_func to error stack but keep error context
        // -> module1_func2 errors checkable via error_get()
        return error_propagate_int(module1_func2());
    }
*/


typedef struct error_info
{
    const char *name, *desc;
} error_info_st;

#define ERROR_INFO(_error, _desc) \
    [_error] = { .name = #_error, .desc = _desc }


// set func with module specific error as error origin
void    _error_set(const char *func, const error_info_st *infos, size_t err);
#define  error_set(error) _error_set(__func__, error_infos, (error))

// set errno of sub as error origin, then add func to error stack
void    _error_set_errno(const char *func, const char *sub);
#define  error_set_errno(sub) _error_set_errno(__func__, #sub)

// add func to existing error stack, clear error context
void    _error_push(const char *func);
#define  error_push() _error_push(__func__)

// add func to existing error stack, keep error context
void    _error_propagate(const char *func);
#define  error_propagate() _error_propagate(__func__)


// convenience macro for enclosing functions with int rc
// to push/propagate error on the fly
#define error_pack_int(action, sub) __extension__ ({ \
    __auto_type rc = (sub); \
    \
    if(rc < 0) \
        error_##action(); \
    \
    rc; \
})

// convenience macro for enclosing functions with pointer rc
// to push/propagate error on the fly
#define error_pack_ptr(action, sub) __extension__ ({ \
    __auto_type rc = (sub); \
    \
    if(!rc) \
        error_##action(); \
    \
    rc; \
})

#define error_push_int(sub)         error_pack_int(push, sub)
#define error_push_ptr(sub)         error_pack_ptr(push, sub)
#define error_propagate_int(sub)    error_pack_int(propagate, sub)
#define error_propagate_ptr(sub)    error_pack_ptr(propagate, sub)


// clear error origin/stack
void error_clear(void);


// get numeric error from context, -1 if unset
ssize_t error_get(void);
// check if error context matches error code
bool    error_check(size_t error);


// get error code from origin
ssize_t     error_origin_get(void);
// get function from origin
const char *error_origin_get_func(void);
// get error name from origin
const char *error_origin_get_name(void);
// get error description from origin
const char *error_origin_get_desc(void);

// get number of errors on stack
size_t      error_stack_get_size(void);
// get function of specific stack level
const char *error_stack_get_func(size_t level);

#endif
