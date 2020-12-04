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

#ifndef YTIL_TEST_TEST_RC_H_INCLUDED
#define YTIL_TEST_TEST_RC_H_INCLUDED

#include <ytil/test/test.h>


/// Test for expr to evaluate to >= 0.
///
/// \param expr     expression to test
/// \param expr_s   expression as text
#define test_int_success_f(expr, expr_s) do {                                   \
    test_begin();                                                               \
    error_clear();                                                              \
                                                                                \
    intmax_t rc = (expr);                                                       \
                                                                                \
    if(rc < 0)                                                                  \
        test_abort_fail_b("INT SUCCESS test failed\n%s\nrc = %jd", expr_s, rc); \
                                                                                \
    test_end();                                                                 \
} while(0)

/// Test for expr to evaluate to >= 0.
///
/// \param expr     expression to test
#define test_int_success(expr) \
    test_int_success_f((expr), #expr)

/// Test for ERRNO expr to evaluate to >= 0.
///
/// \param expr     expression to test
#define test_int_success_errno(expr) \
    test_int_success_f(error_proc_int((expr), set_last_s, ERRNO), #expr)

/// Test for EWIN32 expr to evaluate to >= 0.
///
/// \param expr     expression to test
#define test_int_success_ewin32(expr) \
    test_int_success_f(error_proc_int((expr), set_last_s, EWIN32), #expr)

/// Test for expr to evaluate to >= 0 or to set error.
///
/// \param expr     expression to test
/// \param expr_s   expression as text
/// \param error    error to expect in case of failure
/// \param error_s  error as text
#define test_int_maybe_f(expr, expr_s, error, error_s) do {                          \
    test_begin();                                                                    \
    error_clear();                                                                   \
                                                                                     \
    intmax_t rc = (expr);                                                            \
    int error1;                                                                      \
    int error2  = (error);                                                           \
                                                                                     \
    if(rc < 0 && !error_depth())                                                     \
    {                                                                                \
        test_abort_fail("INT MAYBE test failed\n%s\nrc = %jd\n<none> == %s (%d)",    \
            expr_s, rc, error_s, error2);                                            \
    }                                                                                \
    else if(rc < 0 && (error1 = error_code(0)) != error2)                            \
    {                                                                                \
        test_abort_fail_b("INT MAYBE test failed\n%s\nrc = %jd\n%s (%d) == %s (%d)", \
            expr_s, rc, error_name(0), error1, error_s, error2);                     \
    }                                                                                \
                                                                                     \
    test_end();                                                                      \
} while(0)

/// Test for expr to evaluate to >= 0 or to set error.
///
/// \param expr     expression to test
/// \param error    error to expect in case of failure
#define test_int_maybe(expr, error) \
    test_int_maybe_f((expr), #expr, (error), #error)

/// Test for ERRNO expr to evaluate to >= 0 or to set error.
///
/// \param expr     expression to test
/// \param error    error to expext in case of failure
#define test_int_maybe_errno(expr, error) \
    test_int_maybe_f(error_proc_int((expr), set_last_s, ERRNO), #expr, (error), #error)

/// Test for EWIN32 expr to evaluate to >= 0 or to set error.
///
/// \param expr     expression to test
/// \param error    error to expext in case of failure
#define test_int_maybe_ewin32(expr, error) \
    test_int_maybe_f(error_proc_int((expr), set_last_s, EWIN32), #expr, (error), #error)

/// Test for expr to evaluate to >= 0 or to set error which is picked.
///
/// \param expr         expression to test
/// \param pick_error   error to pick in case of failure
/// \param error        error to expect after picking
#define test_int_pick_maybe(expr, pick_error, error) \
    test_int_maybe_f(error_pick_int((pick_error), (expr)), #expr, (error), #error)

/// Test for expr to evaluate to >= 0 or to set error which is lifted.
///
/// \param expr         expression to test
/// \param lift_error   error to lift in case of failure
/// \param error        error to expect after lifting
#define test_int_lift_maybe(expr, lift_error, error) \
    test_int_maybe_f(error_lift_int((lift_error), (expr)), #expr, (error), #error)

/// Test for expr to evaluate to < 0 and set error.
///
/// \param expr     expression to test
/// \param expr_s   expression as text
/// \param error    error to expect
/// \param error_s  error as text
#define test_int_error_f(expr, expr_s, error, error_s) do {                 \
    test_begin();                                                           \
    error_clear();                                                          \
                                                                            \
    intmax_t rc = (expr);                                                   \
    int error1;                                                             \
    int error2  = (error);                                                  \
                                                                            \
    if(rc >= 0)                                                             \
    {                                                                       \
        test_abort_fail("INT ERROR test failed\n%s\nrc = %jd", expr_s, rc); \
    }                                                                       \
    else if(!error_depth())                                                 \
    {                                                                       \
        test_abort_fail("INT ERROR test failed\n%s\n<none> == %s (%d)",     \
            expr_s, error_s, error2);                                       \
    }                                                                       \
    else if((error1 = error_code(0)) != error2)                             \
    {                                                                       \
        test_abort_fail_b("INT ERROR test failed\n%s\n%s (%d) == %s (%d)",  \
            expr_s, error_name(0), error1, error_s, error2);                \
    }                                                                       \
                                                                            \
    test_end();                                                             \
} while(0)

/// Test for expr to evaluate to < 0 and set error.
///
/// \param expr     expression to test
/// \param error    error to expect
#define test_int_error(expr, error) \
    test_int_error_f((expr), #expr, (error), #error)

/// Test for ERRNO expr to evaluate to < 0 and set error.
///
/// \param expr     expression to test
/// \param error    error to expext
#define test_int_error_errno(expr, error) \
    test_int_error_f(error_proc_int((expr), set_last_s, ERRNO), #expr, (error), #error)

/// Test for EWIN32 expr to evaluate to < 0 and set error.
///
/// \param expr     expression to test
/// \param error    error to expext
#define test_int_error_ewin32(expr, error) \
    test_int_error_f(error_proc_int((expr), set_last_s, EWIN32), #expr, (error), #error)

/// Test for expr to evaluate to < 0 and to set error which is picked.
///
/// \param expr         expression to test
/// \param pick_error   error to pick
/// \param error        error to expect after picking
#define test_int_pick_error(expr, pick_error, error) \
    test_int_error_f(error_pick_int((pick_error), (expr)), #expr, (error), #error)

/// Test for expr to evaluate to < 0 and to set error which is lifted.
///
/// \param expr         expression to test
/// \param lift_error   error to lift
/// \param error        error to expect after lifting
#define test_int_lift_error(expr, lift_error, error) \
    test_int_error_f(error_lift_int((lift_error), (expr)), #expr, (error), #error)

/// Test for expr to evaluate to rc.
///
/// \param expr     expression to test
/// \param expr_s   expression as text
/// \param rc       rc to expect
/// \param rc_s     rc as text
/// \param rc_err   error rc
/// \param rc_err_s error rc as text
#define test_rc_success_f(expr, expr_s, rc, rc_s, rc_err, rc_err_s) do { \
    test_begin();                                                        \
    error_clear();                                                       \
                                                                         \
    intmax_t rc1    = (expr);                                            \
    intmax_t rc2    = (rc);                                              \
                                                                         \
    if(rc1 == (rc_err))                                                  \
    {                                                                    \
        test_abort_fail_b("RC SUCCESS test failed\n%s\nrc = %s (%jd)",   \
            expr_s, rc_err_s, rc1);                                      \
    }                                                                    \
    else if(rc1 != rc2)                                                  \
    {                                                                    \
        test_abort_fail("RC SUCCESS test failed\n%s == %s\n%jd == %jd",  \
            expr_s, rc_s, rc1, rc2);                                     \
    }                                                                    \
                                                                         \
    test_end();                                                          \
} while(0)

/// Test for expr to evaluate to rc.
///
/// \param expr     expression to test
/// \param rc       rc to expect
/// \param rc_err   error rc
#define test_rc_success(expr, rc, rc_err) \
    test_rc_success_f((expr), #expr, (rc), #rc, (rc_err), #rc_err)

/// Test for ERRNO expr to evaluate to rc.
///
/// \param expr     expression to test
/// \param rc       rc to expect
/// \param rc_err   error rc
#define test_rc_success_errno(expr, rc, rc_err)                           \
    test_rc_success_f(error_proc_rc((expr), (rc_err), set_last_s, ERRNO), \
        #expr, (rc), #rc, (rc_err), #rc_err)

/// Test for EWIN32 expr to evaluate to rc.
///
/// \param expr     expression to test
/// \param rc       rc to expect
/// \param rc_err   error rc
#define test_rc_success_ewin32(expr, rc, rc_err)                           \
    test_rc_success_f(error_proc_rc((expr), (rc_err), set_last_s, EWIN32), \
        #expr, (rc), #rc, (rc_err), #rc_err)

/// Test for expr to evaluate to rc, or to rc_err and set error.
///
/// \param expr     expression to test
/// \param expr_s   expression as text
/// \param rc       rc to expect
/// \param rc_s     rc as text
/// \param rc_err   error rc to expect
/// \param rc_err_s error rc as text
/// \param error    error to expect with rc_err
/// \param error_s  error as text
#define test_rc_maybe_f(expr, expr_s, rc, rc_s, rc_err, rc_err_s, error, error_s) do { \
    test_begin();                                                                      \
    error_clear();                                                                     \
                                                                                       \
    intmax_t rc1    = (expr);                                                          \
    intmax_t rc2    = (rc);                                                            \
    intmax_t rce    = (rc_err);                                                        \
    int error1;                                                                        \
    int error2      = (error);                                                         \
                                                                                       \
    if(rc1 == rce && !error_depth())                                                   \
    {                                                                                  \
        test_abort_fail("RC MAYBE test failed\n%s\n%jd == %s\n<none> == %s (%d)",      \
            expr_s, rc1, rc_err_s, error_s, error2);                                   \
    }                                                                                  \
    else if(rc1 == rce && (error1 = error_code(0)) != error2)                          \
    {                                                                                  \
        test_abort_fail_b("RC MAYBE test failed\n%s\n%jd == %s\n%s (%d) == %s (%d)",   \
            expr_s, rc1, rc_err_s, error_name(0), error1, error_s, error2);            \
    }                                                                                  \
    else if(rc1 != rce && rc1 != rc2)                                                  \
    {                                                                                  \
        test_abort_fail("RC MAYBE test failed\n%s\n%jd == %jd", expr_s, rc1, rc2);     \
    }                                                                                  \
                                                                                       \
    test_end();                                                                        \
} while(0)

/// Test for expr to evaluate to rc, or to rc_err and set error.
///
/// \param expr     expression to test
/// \param rc       rc to expect
/// \param rc_err   error rc to expect
/// \param error    error to expect with rc_err
#define test_rc_maybe(expr, rc, rc_err, error) \
    test_rc_maybe_f((expr), #expr, (rc), #rc, (rc_err), #rc_err, (error), #error)

/// Test for ERRNO expr to evaluate to rc, or to rc_err and set error.
///
/// \param expr     expression to test
/// \param rc       rc to expect
/// \param rc_err   error rc to expect
/// \param error    error to expect with rc_err
#define test_rc_maybe_errno(expr, rc, rc_err, error)                    \
    test_rc_maybe_f(error_proc_rc((expr), (rc_err), set_last_s, ERRNO), \
        #expr, (rc), #rc, (rc_err), #rc_err, (error), #error)

/// Test for EWIN32 expr to evaluate to rc, or to rc_err and set error.
///
/// \param expr     expression to test
/// \param rc       rc to expect
/// \param rc_err   error rc to expect
/// \param error    error to expect with rc_err
#define test_rc_maybe_ewin32(expr, rc, rc_err, error)                    \
    test_rc_maybe_f(error_proc_rc((expr), (rc_err), set_last_s, EWIN32), \
        #expr, (rc), #rc, (rc_err), #rc_err, (error), #error)

/// Test for expr to evaluate to rc, or to rc_err and set error which is picked.
///
/// \param expr         expression to test
/// \param rc           rc to expect
/// \param rc_err       error rc to expect
/// \param pick_error   error to pick with rc_err
/// \param error        error to expect after picking
#define test_rc_pick_maybe(expr, rc, rc_err, pick_error, error)    \
    test_rc_maybe_f(error_pick_rc((rc_err), (pick_error), (expr)), \
        #expr, (rc), #rc, (rc_err), #rc_err, (error), #error)

/// Test for expr to evaluate to rc, or to rc_err and set error which is lifted.
///
/// \param expr         expression to test
/// \param rc           rc to expect
/// \param rc_err       error rc to expect
/// \param lift_error   error to lift with rc_err
/// \param error        error to expect after lifting
#define test_rc_lift_maybe(expr, rc, rc_err, lift_error, error)    \
    test_rc_maybe_f(error_lift_rc((rc_err), (lift_error), (expr)), \
        #expr, (rc), #rc, (rc_err), #rc_err, (error), #error)

/// Test for expr to evaluate to rc and set error.
///
/// \param expr     expression to test
/// \param expr_s   expression as text
/// \param rc       rc to expect
/// \param rc_s     rc as text
/// \param error    error to expect
/// \param error_s  error as text
#define test_rc_error_f(expr, expr_s, rc, rc_s, error, error_s) do {      \
    test_begin();                                                         \
    error_clear();                                                        \
                                                                          \
    intmax_t rc1    = (expr);                                             \
    intmax_t rc2    = (rc);                                               \
    int error1;                                                           \
    int error2      = (error);                                            \
                                                                          \
    if(rc1 != rc2)                                                        \
    {                                                                     \
        test_abort_fail("RC ERROR test failed\n%s == %s\n%jd == %jd",     \
            expr_s, rc_s, rc1, rc2);                                      \
    }                                                                     \
    else if(!error_depth())                                               \
    {                                                                     \
        test_abort_fail("RC ERROR test failed\n%s\n<none> == %s (%d)",    \
            expr_s, error_s, error2);                                     \
    }                                                                     \
    else if((error1 = error_code(0)) != error2)                           \
    {                                                                     \
        test_abort_fail_b("RC ERROR test failed\n%s\n%s (%d) == %s (%d)", \
            expr_s, error_name(0), error1, error_s, error2);              \
    }                                                                     \
                                                                          \
    test_end();                                                           \
} while(0)

/// Test for expr to evaluate to rc and set error.
///
/// \param expr     expression to test
/// \param rc       rc to expect
/// \param error    error to expect
#define test_rc_error(expr, rc, error) \
    test_rc_error_f((expr), #expr, (rc), #rc, (error), #error)

/// Test for ERRNO expr to evaluate to rc and set error.
///
/// \param expr     expression to test
/// \param rc       rc to expect
/// \param error    error to expext
#define test_rc_error_errno(expr, rc, error)                        \
    test_rc_error_f(error_proc_rc((expr), (rc), set_last_s, ERRNO), \
        #expr, (rc), #rc, (error), #error)

/// Test for EWIN32 expr to evaluate to rc and set error.
///
/// \param expr     expression to test
/// \param rc       rc to expect
/// \param error    error to expext
#define test_rc_error_ewin32(expr, rc, error)                        \
    test_rc_error_f(error_proc_rc((expr), (rc), set_last_s, EWIN32), \
        #expr, (rc), #rc, (error), #error)

/// Test for expr to evaluate to rc and to set error which is picked.
///
/// \param expr         expression to test
/// \param rc           rc to expect
/// \param pick_error   error to pick
/// \param error        error to expect after picking
#define test_rc_pick_error(expr, rc, pick_error, error)        \
    test_rc_error_f(error_pick_rc((rc), (pick_error), (expr)), \
        #expr, (rc), #rc, (error), #error)

/// Test for expr to evaluate to rc and to set error which is lifted.
///
/// \param expr         expression to test
/// \param rc           rc to expect
/// \param lift_error   error to lift
/// \param error        error to expect after lifting
#define test_rc_lift_error(expr, rc, lift_error, error)        \
    test_rc_error_f(error_lift_rc((rc), (lift_error), (expr)), \
        #expr, (rc), #rc, (error), #error)

/// Test for expr to evaluate to non-NULL pointer.
///
/// \param expr     expression to test
/// \param expr_s   expression as text
#define test_ptr_success_f(expr, expr_s) do {                     \
    test_begin();                                                 \
    error_clear();                                                \
                                                                  \
    if(!(expr))                                                   \
        test_abort_fail_b("PTR SUCCESS test failed\n%s", expr_s); \
                                                                  \
    test_end();                                                   \
} while(0)

/// Test for expr to evaluate to non-NULL pointer.
///
/// \param expr     expression to test
#define test_ptr_success(expr) \
    test_ptr_success_f((expr), #expr)

/// Test for ERRNO expr to evaluate to non-NULL pointer.
///
/// \param expr     expression to test
#define test_ptr_success_errno(expr) \
    test_ptr_success_f(error_proc_ptr((expr), set_last_s, ERRNO), #expr)

/// Test for EWIN32 expr to evaluate to non-NULL pointer.
///
/// \param expr     expression to test
#define test_ptr_success_ewin32(expr) \
    test_ptr_success_f(error_proc_ptr((expr), set_last_s, EWIN32), #expr)

/// Test for expr to evaluate to non-NULL pointer or to set error.
///
/// \param expr     expression to test
/// \param expr_s   expression as text
/// \param error    error to expect in case of failure
/// \param error_s  error as text
#define test_ptr_maybe_f(expr, expr_s, error, error_s) do {                            \
    test_begin();                                                                      \
    error_clear();                                                                     \
                                                                                       \
    const void *ptr = (expr);                                                          \
    int error1;                                                                        \
    int error2      = (error);                                                         \
                                                                                       \
    if(!ptr && !error_depth())                                                         \
    {                                                                                  \
        test_abort_fail("PTR MAYBE test failed\n%s\n<none> == %s (%d)",                \
            expr_s, error_s, error2);                                                  \
    }                                                                                  \
    else if(!ptr && (error1 = error_code(0)) != error2)                                \
    {                                                                                  \
        test_abort_fail_b("PTR MAYBE test failed\n%s\nptr = 0x%p\n%s (%d) == %s (%d)", \
            expr_s, ptr, error_name(0), error1, error_s, error2);                      \
    }                                                                                  \
                                                                                       \
    test_end();                                                                        \
} while(0)

/// Test for expr to evaluate to non-NULL pointer or to set error.
///
/// \param expr     expression to test
/// \param error    error to expect in case of failure
#define test_ptr_maybe(expr, error) \
    test_ptr_maybe_f((expr), #expr, (error), #error)

/// Test for ERRNO expr to evaluate to non-NULL pointer or to set error.
///
/// \param expr     expression to test
/// \param error    error to expext in case of failure
#define test_ptr_maybe_errno(expr, error) \
    test_ptr_maybe_f(error_proc_ptr((expr), set_last_s, ERRNO), #expr, (error), #error)

/// Test for EWIN32 expr to evaluate to non-NULL pointer or to set error.
///
/// \param expr     expression to test
/// \param error    error to expext in case of failure
#define test_ptr_maybe_ewin32(expr, error) \
    test_ptr_maybe_f(error_proc_ptr((expr), set_last_s, EWIN32), #expr, (error), #error)

/// Test for expr to evaluate to non-NULL pointer or to set error which is picked.
///
/// \param expr         expression to test
/// \param pick_error   error to pick in case of failure
/// \param error        error to expect after picking
#define test_ptr_pick_maybe(expr, pick_error, error) \
    test_ptr_maybe_f(error_pick_ptr((pick_error), (expr)), #expr, (error), #error)

/// Test for expr to evaluate to non-NULL pointer or to set error which is lifted.
///
/// \param expr         expression to test
/// \param lift_error   error to lift in case of failure
/// \param error        error to expect after lifting
#define test_ptr_lift_maybe(expr, lift_error, error) \
    test_ptr_maybe_f(error_lift_ptr((lift_error), (expr)), #expr, (error), #error)

/// Test for expr to evaluate to NULL pointer and set error.
///
/// \param expr     expression to test
/// \param expr_s   expression as text
/// \param error    error to expect
/// \param error_s  error as text
#define test_ptr_error_f(expr, expr_s, error, error_s) do {                    \
    test_begin();                                                              \
    error_clear();                                                             \
                                                                               \
    const void *ptr = (expr);                                                  \
    int error1;                                                                \
    int error2      = (error);                                                 \
                                                                               \
    if(ptr)                                                                    \
    {                                                                          \
        test_abort_fail("PTR ERROR test failed\n%s\nptr = 0x%p", expr_s, ptr); \
    }                                                                          \
    else if(!error_depth())                                                    \
    {                                                                          \
        test_abort_fail("PTR ERROR test failed\n%s\n<none> == %s (%d)",        \
            expr_s, error_s, error2);                                          \
    }                                                                          \
    else if((error1 = error_code(0)) != error2)                                \
    {                                                                          \
        test_abort_fail_b("PTR ERROR test failed\n%s\n%s (%d) == %s (%d)",     \
            expr_s, error_name(0), error1, error_s, error2);                   \
    }                                                                          \
                                                                               \
    test_end();                                                                \
} while(0)

/// Test for expr to evaluate to NULL pointer and set error.
///
/// \param expr     expression to test
/// \param error    error to expect
#define test_ptr_error(expr, error) \
    test_ptr_error_f((expr), #expr, (error), #error)

/// Test for ERRNO expr to evaluate to NULL pointer and set error.
///
/// \param expr     expression to test
/// \param error    error to expext
#define test_ptr_error_errno(expr, error) \
    test_ptr_error_f(error_proc_ptr((expr), set_last_s, ERRNO), #expr, (error), #error)

/// Test for EWIN32 expr to evaluate to NULL pointer and set error.
///
/// \param expr     expression to test
/// \param error    error to expext
#define test_ptr_error_ewin32(expr, error) \
    test_ptr_error_f(error_proc_ptr((expr), set_last_s, EWIN32), #expr, (error), #error)

/// Test for expr to evaluate to NULL pointer and to set error which is picked.
///
/// \param expr         expression to test
/// \param pick_error   error to pick
/// \param error        error to expect after picking
#define test_ptr_pick_error(expr, pick_error, error) \
    test_ptr_error_f(error_pick_ptr((pick_error), (expr)), #expr, (error), #error)

/// Test for expr to evaluate to NULL pointer and to set error which is lifted.
///
/// \param expr         expression to test
/// \param lift_error   error to lift
/// \param error        error to expect after lifting
#define test_ptr_lift_error(expr, lift_error, error) \
    test_ptr_error_f(error_lift_ptr((lift_error), (expr)), #expr, (error), #error)


#if OS_WINDOWS

/// Test for HRESULT expr to evaluate to S_OK.
///
/// \param expr     expression to test
#define test_hresult_success(expr) do {                                      \
    test_begin();                                                            \
                                                                             \
    HRESULT result = (expr);                                                 \
                                                                             \
    if(result != S_OK)                                                       \
    {                                                                        \
        test_abort_fail("HRESULT SUCCESS test failed\n%s\nresult = %s (%d)", \
            #expr, error_type_get_name(HRESULT, result), result);            \
    }                                                                        \
                                                                             \
    test_end();                                                              \
} while(0)

/// Test for HRESULT expr to evaluate to S_OK or to error_result.
///
/// \param expr         expression to test
/// \param error_result result to expect in case of failure
#define test_hresult_maybe(expr, error_result) do {                          \
    test_begin();                                                            \
                                                                             \
    HRESULT _result         = (expr);                                        \
    HRESULT _error_result   = (error_result);                                \
                                                                             \
    if(_result != S_OK && _result != _error_result)                          \
    {                                                                        \
        test_abort_fail("HRESULT MAYBE test failed\n%s\n%s (%d) == %s (%d)", \
            #expr, error_type_get_name(HRESULT, _result),                    \
            _result, #error_result, _error_result);                          \
    }                                                                        \
                                                                             \
    test_end();                                                              \
} while(0)

/// Test for HRESULT expr to evaluate to error_result.
///
/// \param expr         expression to test
/// \param error_result result to expect
#define test_hresult_error(expr, error_result) do {                          \
    test_begin();                                                            \
                                                                             \
    HRESULT _result         = (expr);                                        \
    HRESULT _error_result   = (error_result);                                \
                                                                             \
    if(_result != _error_result)                                             \
    {                                                                        \
        test_abort_fail("HRESULT ERROR test failed\n%s\n%s (%d) == %s (%d)", \
            #expr, error_type_get_name(HRESULT, _result),                    \
            _result, #error_result, _error_result);                          \
    }                                                                        \
                                                                             \
    test_end();                                                              \
} while(0)

/// Test for NTSTATUS expr to evaluate to STATUS_SUCCESS.
///
/// \param expr     expression to test
#define test_ntstatus_success(expr) do {                                      \
    test_begin();                                                             \
                                                                              \
    NTSTATUS status = (expr);                                                 \
                                                                              \
    if(status != STATUS_SUCCESS)                                              \
    {                                                                         \
        test_abort_fail("NTSTATUS SUCCESS test failed\n%s\nstatus = %s (%d)", \
            #expr, error_type_get_name(NTSTATUS, status), status);            \
    }                                                                         \
                                                                              \
    test_end();                                                               \
} while(0)

/// Test for NTSTATUS expr to evaluate to STATUS_SUCCESS or to error_status.
///
/// \param expr         expression to test
/// \param error_status status to expect in case of failure
#define test_ntstatus_maybe(expr, error_status) do {                          \
    test_begin();                                                             \
                                                                              \
    NTSTATUS _status         = (expr);                                        \
    NTSTATUS _error_status   = (error_status);                                \
                                                                              \
    if(_status != STATUS_SUCCESS && _status != _error_status)                 \
    {                                                                         \
        test_abort_fail("NTSTATUS MAYBE test failed\n%s\n%s (%d) == %s (%d)", \
            #expr, error_type_get_name(NTSTATUS, _status),                    \
            _status, #error_status, _error_status);                           \
    }                                                                         \
                                                                              \
    test_end();                                                               \
} while(0)

/// Test for NTSTATUS expr to evaluate to error_status.
///
/// \param expr         expression to test
/// \param error_status status to expect
#define test_ntstatus_error(expr, error_status) do {                          \
    test_begin();                                                             \
                                                                              \
    NTSTATUS _status         = (expr);                                        \
    NTSTATUS _error_status   = (error_status);                                \
                                                                              \
    if(_status != _error_status)                                              \
    {                                                                         \
        test_abort_fail("NTSTATUS ERROR test failed\n%s\n%s (%d) == %s (%d)", \
            #expr, error_type_get_name(NTSTATUS, _status),                    \
            _status, #error_status, _error_status);                           \
    }                                                                         \
                                                                              \
    test_end();                                                               \
} while(0)

#endif // if OS_WINDOWS


#endif // ifndef YTIL_TEST_TEST_RC_H_INCLUDED
