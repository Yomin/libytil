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

#ifndef YTIL_TEST_TEST_H_INCLUDED
#define YTIL_TEST_TEST_H_INCLUDED

#include <ytil/test/state.h>
#include <ytil/gen/error.h>
#include <stdint.h>
#include <float.h>
#include <math.h>
#include <string.h>

#define TEST_CTX _test_case_ctx


void    _test_pos(void *ctx, test_pos_id type, const char *file, size_t line);
#define  test_pos()       _test_pos(TEST_CTX, TEST_POS_EXACT, __FILE__, __LINE__)
#define  test_pos_after() _test_pos(TEST_CTX, TEST_POS_AFTER, __FILE__, __LINE__)

void    _test_msg(void *ctx, const char *file, size_t line, test_msg_id type, size_t level, bool backtrace, const char *msg, ...);
#define  test_msg(type, bt, msg, ...) _test_msg(TEST_CTX, __FILE__, __LINE__, (type), 0, (bt), (msg) __VA_OPT__(,) __VA_ARGS__)
#define  test_msg_info(msg, ...)       test_msg(TEST_MSG_INFO, false, (msg) __VA_OPT__(,) __VA_ARGS__)
#define  test_msg_warn(msg, ...)       test_msg(TEST_MSG_WARNING, false, (msg) __VA_OPT__(,) __VA_ARGS__)
#define  test_msg_error(msg, ...)      test_msg(TEST_MSG_ERROR, false, (msg) __VA_OPT__(,) __VA_ARGS__)
#define  test_msg_backtrace(msg, ...)  test_msg(TEST_MSG_ERROR, true, (msg) __VA_OPT__(,) __VA_ARGS__)

void    _test_begin(void *ctx, const char *file, size_t line);
#define  test_begin() _test_begin(TEST_CTX, __FILE__, __LINE__)

void    _test_end(void *ctx, const char *file, size_t line);
#define  test_end() _test_end(TEST_CTX, __FILE__, __LINE__)

void    _test_abort(void *ctx, const char *file, size_t line, bool backtrace, const char *msg, ...);
#define  test_abort(msg, ...)           _test_abort(TEST_CTX, __FILE__, __LINE__, false, (msg) __VA_OPT__(,) __VA_ARGS__)
#define  test_abort_backtrace(msg, ...) _test_abort(TEST_CTX, __FILE__, __LINE__, true, (msg) __VA_OPT__(,) __VA_ARGS__)

void   *_test_alloc(void *ctx, const char *file, size_t line, size_t size);
#define  test_alloc(size) _test_alloc(TEST_CTX, __FILE__, __LINE__, (size))
void    _test_free(void *ctx, const char *file, size_t line, void *mem);
#define  test_free(mem) _test_free(TEST_CTX, __FILE__, __LINE__, (mem))

#define  test_state_alloc(size) (TEST_STATE = test_alloc(size))
#define  test_state_free() test_free(TEST_STATE)

#define  test_call(name, ...) _test_func_##name(TEST_CTX, &TEST_STATE __VA_OPT__(,) __VA_ARGS__)


#define test_case_info(expr, msg, ...) do { \
    test_begin(); \
    \
    if(expr) \
        test_msg_info((msg) __VA_OPT__(,) __VA_ARGS__); \
    \
    test_end(); \
} while(0)

#define test_case_warn(expr, msg, ...) do { \
    test_begin(); \
    \
    if(expr) \
        test_msg_warn((msg) __VA_OPT__(,) __VA_ARGS__); \
    \
    test_end(); \
} while(0)


#define test_error(depth, type, error) do { \
    test_begin(); \
    \
    if(error_type(depth) != &ERROR_TYPE_ ## type) \
        test_abort_backtrace("ERROR type test failed: %s == %s", \
            error_type_name(error_type(depth)), error_type_name(&ERROR_TYPE_ ## type)); \
    else if(error_code(depth) != (error)) \
        test_abort_backtrace("%s test failed: %s == %s", \
            error_type_name(&ERROR_TYPE_ ## type), error_name(depth), #error); \
    \
    test_end(); \
} while(0)

#define test_stack_error(level, type, error) do { \
    test_begin(); \
    \
    if(error_stack_get_type(level) != &ERROR_TYPE_ ## type) \
        test_abort_backtrace("ERROR type test failed: %s == %s", \
            error_type_name(error_stack_get_type(level)), error_type_name(&ERROR_TYPE_ ## type)); \
    else if(error_stack_get_code(level) != (error)) \
        test_abort_backtrace("%s test failed: %s == %s", \
            error_type_name(&ERROR_TYPE_ ## type), error_stack_get_name(level), #error); \
    \
    test_end(); \
} while(0)


// --- [INT RC] ---

#define test_int_success(expr) do { \
    test_begin(); \
    \
    intmax_t rc; \
    \
    if((rc = (expr)) < 0) \
        test_abort_backtrace("INT SUCCESS test failed: %s (%jd)", #expr, rc); \
    \
    test_end(); \
} while(0)

#define _test_int_success_native(expr, expr_s, type, ctx) do { \
    test_begin(); \
    \
    intmax_t rc = (expr); \
    int code = error_type_get_last(&ERROR_TYPE_ ## type, ctx); \
    \
    if(rc < 0) \
        test_abort("INT SUCCESS test failed: %s (%jd, %s)", \
            expr_s, rc, error_type_get_name(&ERROR_TYPE_ ## type, code)); \
    \
    test_end(); \
} while(0)

#define test_int_success_errno(expr) _test_int_success_native((expr), #expr, ERRNO, NULL)
#define test_int_success_win32(expr) _test_int_success_native((expr), #expr, WIN32, NULL)

#define _test_int_maybe(expr, expr_s, err, err_s) do { \
    test_begin(); \
    error_clear(); \
    \
    intmax_t rc; \
    error_clear(); \
    \
    if((rc = expr) < 0 && error_code(0) != err) \
        test_abort_backtrace("INT MAYBE test failed: %s (%jd, %s == %s)", expr_s, rc, error_name(0), err_s); \
    \
    test_end(); \
} while(0)

#define test_int_maybe(expr, err)                _test_int_maybe((expr), #expr, (err), #err)
#define test_int_pick_maybe(expr, pick_err, err) _test_int_maybe(error_pick_int((pick_err), (expr)), #expr, (err), #err)
#define test_int_lift_maybe(expr, lift_err, err) _test_int_maybe(error_lift_int((lift_err), (expr)), #expr, (err), #err)

#define _test_int_maybe_native(expr, expr_s, err, err_s, type, ctx) do { \
    test_begin(); \
    \
    intmax_t rc = (expr); \
    int code = error_type_get_last(&ERROR_TYPE_ ## type, ctx); \
    \
    if(rc < 0 && err != code) \
        test_abort("INT MAYBE test failed: %s (%jd, %s == %s)", \
            expr_s, rc, error_type_get_name(&ERROR_TYPE_ ## type, code), err_s); \
    \
    test_end(); \
} while(0)

#define test_int_maybe_errno(expr, err) _test_int_maybe_native((expr), #expr, (err), #err, ERRNO, NULL)
#define test_int_maybe_win32(expr, err) _test_int_maybe_native((expr), #expr, (err), #err, WIN32, NULL)

#define _test_int_error(expr, expr_s, err, err_s) do { \
    test_begin(); \
    error_clear(); \
    \
    intmax_t rc; \
    \
    if((rc = expr) >= 0) \
        test_abort("INT ERROR test failed: %s (%jd)", expr_s, rc); \
    else if(error_code(0) != err) \
        test_abort_backtrace("INT ERROR test failed: %s (%s == %s)", expr_s, error_name(0), err_s); \
    \
    test_end(); \
} while(0)

#define test_int_error(expr, err)                _test_int_error((expr), #expr, (err), #err)
#define test_int_pick_error(expr, pick_err, err) _test_int_error(error_pick_int((pick_err), (expr)), #expr, (err), #err)
#define test_int_lift_error(expr, lift_err, err) _test_int_error(error_lift_int((lift_err), (expr)), #expr, (err), #err)

#define _test_int_error_native(expr, expr_s, err, err_s, type, ctx) do { \
    test_begin(); \
    \
    intmax_t rc = (expr); \
    int code = error_type_get_last(&ERROR_TYPE_ ## type, ctx); \
    \
    if(rc >= 0) \
        test_abort("INT ERROR test failed: %s (%jd)", expr_s, rc); \
    else if(err != code) \
        test_abort("INT ERROR test failed: %s (%s == %s)", \
            expr_s, error_type_get_name(&ERROR_TYPE_ ## type, code), err_s); \
    \
    test_end(); \
} while(0)

#define test_int_error_errno(expr, err) _test_int_error_native((expr), #expr, (err), #err, ERRNO, NULL)
#define test_int_error_win32(expr, err) _test_int_error_native((expr), #expr, (err), #err, WIN32, NULL)


// --- [INT VALUE RC] ---

#define test_rc_success(expr, trc) do { \
    test_begin(); \
    \
    intmax_t rc, _trc = (trc); \
    \
    if((rc = (expr)) < 0) \
        test_abort_backtrace("RC SUCCESS test failed: %s (%jd)", #expr, rc); \
    else if(rc != _trc) \
        test_abort("RC SUCCESS test failed: %s == %s (%jd == %jd)", #expr, #trc, rc, _trc); \
    \
    test_end(); \
} while(0)

#define _test_rc_success_native(expr, expr_s, trc, trc_s, type, ctx) do { \
    test_begin(); \
    \
    intmax_t rc, _trc = (trc); \
    int code = error_type_get_last(&ERROR_TYPE_ ## type, ctx); \
    \
    if((rc = (expr)) < 0) \
        test_abort("RC SUCCESS test failed: %s (%jd, %s)", \
            expr_s, rc, error_type_get_name(&ERROR_TYPE_ ## type, code)); \
    else if(rc != _trc) \
        test_abort("RC SUCCESS test failed: %s == %s (%jd == %jd)", expr_s, trc_s, rc, _trc); \
    \
    test_end(); \
} while(0)

#define test_rc_success_errno(expr, trc) _test_rc_success_native((expr), #expr, (trc), #trc, ERRNO, NULL)
#define test_rc_success_win32(expr, trc) _test_rc_success_native((expr), #expr, (trc), #trc, WIN32, NULL)

#define _test_rc_error(expr, expr_s, trc, trc_s, err, err_s) do { \
    test_begin(); \
    \
    intmax_t rc, _trc = (trc); \
    error_clear(); \
    \
    if((rc = expr) != _trc) \
        test_abort("RC ERROR test failed: %s == %s (%jd == %jd)", expr_s, trc_s, rc, _trc); \
    else if(error_code(0) != err) \
        test_abort_backtrace("RC ERROR test failed: %s (%s == %s)", expr_s, error_name(0), err_s); \
    \
    test_end(); \
} while(0)

#define test_rc_error(expr, trc, err)                _test_rc_error((expr), #expr, (trc), #trc, (err), #err)
#define test_rc_pick_error(expr, trc, pick_err, err) _test_rc_error(error_pick_int((pick_err), (expr)), #expr, (trc), #trc, (err), #err)
#define test_rc_lift_error(expr, trc, lift_err, err) _test_rc_error(error_lift_int((lift_err), (expr)), #expr, (trc), #trc, (err), #err)

#define _test_rc_error_native(expr, expr_s, trc, trc_s, err, err_s, type, ctx) do { \
    test_begin(); \
    \
    intmax_t rc = (expr), _trc = (trc); \
    int code = error_type_get_last(&ERROR_TYPE_ ## type, ctx); \
    \
    if(rc != _trc) \
        test_abort("RC ERROR test failed: %s == %s (%jd == %jd)", expr_s, trc_s, rc, _trc); \
    else if(err != code) \
        test_abort("RC ERROR test failed: %s (%s == %s)", \
            expr_s, error_type_get_name(&ERROR_TYPE_ ## type, code), err_s); \
    \
    test_end(); \
} while(0)

#define test_rc_error_errno(expr, trc, err) _test_rc_error_native((expr), #expr, (trc), #trc, (err), #err, errno, errno)
#define test_rc_error_win32(expr, trc, err) _test_rc_error_native((expr), #expr, (trc), #trc, (err), #err, win32, GetLastError())


// --- [PTR RC] ---

#define test_ptr_success(expr) do { \
    test_begin(); \
    \
    if(!(expr)) \
        test_abort_backtrace("PTR SUCCESS test failed: %s", #expr); \
    \
    test_end(); \
} while(0)

#define _test_ptr_success_native(expr, expr_s, type, ctx) do { \
    test_begin(); \
    \
    const void *ptr = (expr); \
    int code = error_type_get_last(&ERROR_TYPE_ ## type, ctx); \
    \
    if(!ptr) \
        test_abort("PTR SUCCESS test failed: %s (%s)", \
            expr_s, error_type_get_name(&ERROR_TYPE_ ## type, code)); \
    \
    test_end(); \
} while(0)

#define test_ptr_success_errno(expr) _test_ptr_success_native((expr), #expr, ERRNO, NULL)
#define test_ptr_success_win32(expr) _test_ptr_success_native((expr), #expr, WIN32, NULL)

#define _test_ptr_maybe(expr, expr_s, err, err_s) do { \
    test_begin(); \
    error_clear(); \
    \
    if(!expr && error_code(0) != err) \
        test_abort_backtrace("PTR MAYBE test failed: %s (%s == %s)", expr_s, error_name(0), err_s); \
    \
    test_end(); \
} while(0)

#define test_ptr_maybe(expr, err)                _test_ptr_maybe((expr), #expr, (err), #err)
#define test_ptr_pick_maybe(expr, pick_err, err) _test_ptr_maybe(error_pick_ptr((pick_err), (expr)), #expr, (err), #err)
#define test_ptr_lift_maybe(expr, lift_err, err) _test_ptr_maybe(error_lift_ptr((lift_err), (expr)), #expr, (err), #err)

#define _test_ptr_maybe_native(expr, expr_s, err, err_s, type, ctx) do { \
    test_begin(); \
    \
    const void *ptr = (expr); \
    int code = error_type_get_last(&ERROR_TYPE_ ## type, ctx); \
    \
    if(!ptr && err != _value) \
        test_abort("PTR MAYBE test failed: %s (%s == %s)", \
            expr_s, error_type_get_name(&ERROR_TYPE_ ## type, code), err_s); \
    \
    test_end(); \
} while(0)

#define test_ptr_maybe_errno(expr, err) _test_ptr_maybe_native((expr), #expr, (err), #err, ERRNO, NULL)
#define test_ptr_maybe_win32(expr, err) _test_ptr_maybe_native((expr), #expr, (err), #err, WIN32, NULL)

#define _test_ptr_error(expr, expr_s, err, err_s) do { \
    test_begin(); \
    error_clear(); \
    \
    const void *ptr; \
    \
    if((ptr = expr)) \
        test_abort("PTR ERROR test failed: %s (%p)", expr_s, ptr); \
    else if(error_code(0) != err) \
        test_abort_backtrace("PTR ERROR test failed: %s (%s == %s)", expr_s, error_name(0), err_s); \
    \
    test_end(); \
} while(0)

#define test_ptr_error(expr, err)                _test_ptr_error((expr), #expr, (err), #err)
#define test_ptr_pick_error(expr, pick_err, err) _test_ptr_error(error_pick_ptr((pick_err), (expr)), #expr, (err), #err)
#define test_ptr_lift_error(expr, lift_err, err) _test_ptr_error(error_lift_ptr((lift_err), (expr)), #expr, (err), #err)

#define _test_ptr_error_native(expr, expr_s, err, err_s, type, ctx) do { \
    test_begin(); \
    \
    const void *ptr = (expr); \
    int code = error_type_get_last(&ERROR_TYPE_ ## type, ctx); \
    \
    if(ptr) \
        test_abort("PTR ERROR test failed: %s (%p)", expr_s, ptr); \
    else if(err != code) \
        test_abort("PTR ERROR test failed: %s (%s == %s)", \
            expr_s, error_type_get_name(&ERROR_TYPE_ ## type, code), err_s); \
    \
    test_end(); \
} while(0)

#define test_ptr_error_errno(expr, err) _test_ptr_error_native((expr), #expr, (err), #err, ERRNO, NULL)
#define test_ptr_error_win32(expr, err) _test_ptr_error_native((expr), #expr, (err), #err, WIN32, NULL)


// --- [VOID] ---

#define test_void(expr) do { \
    test_begin(); \
    \
    expr; \
    \
    test_end(); \
} while(0)


// --- [BOOL] ---

#define test_true(expr) do { \
    test_begin(); \
    \
    if(!(expr)) \
        test_abort("TRUE test failed: %s", #expr); \
    \
    test_end(); \
} while(0)

#define test_false(expr) do { \
    test_begin(); \
    \
    if((expr)) \
        test_abort("FALSE test failed: %s", #expr); \
    \
    test_end(); \
} while(0)


// --- [GENERIC] ---

#define _test_val(name, type, spec, v1, e1, op, v2, e2) do { \
    test_begin(); \
    \
    type _v1 = v1; \
    type _v2 = v2; \
    \
    if(!(_v1 op _v2)) \
        test_abort("%s test failed: %s %s %s (%"spec" %s %"spec")", \
            name, e1, #op, e2, _v1, #op, _v2); \
    \
    test_end(); \
} while(0)

#define _test_range(name, type, spec, op, v1, e1, v2, e2, v3, e3) do { \
    test_begin(); \
    \
    type _v1 = v1; \
    type _v2 = v2; \
    type _v3 = v3; \
    \
    if(!(_v1 op _v2 && _v2 op _v3)) \
        test_abort("%s range test failed: %s %s %s %s %s (%"spec" %s %"spec" %s %"spec")", \
            name, e1, #op, e2, #op, e3, _v1, #op, _v2, #op, _v3); \
    \
    test_end(); \
} while(0)

#define _test_list(name, type, spec, l1, e1, l2, e2, n) do { \
    test_begin(); \
    \
    type _v1; \
    type _v2; \
    __auto_type _l1 = l1; \
    __auto_type _l2 = l2; \
    size_t _i, _n = n; \
    \
    for(_i=0; _i < _n; _i++) \
        if((_v1 = _l1[_i]) != (_v2 = _l2[_i])) \
            test_abort("%s list test failed: %s[%zu] == %s[%zu] (%"spec" == %"spec")", \
                name, e1, _i, e2, _i, _v1, _v2); \
    \
    test_end(); \
} while(0);


// --- [INT] ---

#define _test_int(i1, e1, op, i2, e2) \
    _test_val("INT", intmax_t, "jd", i1, e1, op, i2, e2)

#define test_int_lt(i1, i2) _test_int((i1), #i1,  <, (i2), #i2)
#define test_int_le(i1, i2) _test_int((i1), #i1, <=, (i2), #i2)
#define test_int_eq(i1, i2) _test_int((i1), #i1, ==, (i2), #i2)
#define test_int_ne(i1, i2) _test_int((i1), #i1, !=, (i2), #i2)
#define test_int_ge(i1, i2) _test_int((i1), #i1, >=, (i2), #i2)
#define test_int_gt(i1, i2) _test_int((i1), #i1,  >, (i2), #i2)

#define test_int_range(i1, i2, i3) \
    _test_range("INT", intmax_t, "jd", <=, (i1), #i1, (i2), #i2, (i3), #i3)

#define test_int_between(i1, i2, i3) \
    _test_range("INT", intmax_t, "jd", <, (i1), #i1, (i2), #i2, (i3), #i3)

#define test_int_list(l1, l2, n) \
    _test_list("INT", intmax_t, "jd", (l1), #l1, (l2), #l2, (n))


// --- [UINT] ---

#define _test_uint(u1, e1, op, u2, e2) \
    _test_val("UINT", uintmax_t, "ju", u1, e1, op, u2, e2)

#define test_uint_lt(u1, u2) _test_uint((u1), #u1,  <, (u2), #u2)
#define test_uint_le(u1, u2) _test_uint((u1), #u1, <=, (u2), #u2)
#define test_uint_eq(u1, u2) _test_uint((u1), #u1, ==, (u2), #u2)
#define test_uint_ne(u1, u2) _test_uint((u1), #u1, !=, (u2), #u2)
#define test_uint_ge(u1, u2) _test_uint((u1), #u1, >=, (u2), #u2)
#define test_uint_gt(u1, u2) _test_uint((u1), #u1,  >, (u2), #u2)

#define test_uint_range(u1, u2, u3) \
    _test_range("UINT", uintmax_t, "ju", <=, (u1), #u1, (u2), #u2, (u3), #u3)

#define test_uint_between(u1, u2, u3) \
    _test_range("UINT", uintmax_t, "ju", <, (u1), #u1, (u2), #u2, (u3), #u3)

#define test_uint_list(l1, l2, n) \
    _test_list("UINT", uintmax_t, "ju", (l1), #l1, (l2), #l2, (n))


// --- [PTR] ---

#define _test_ptr(p1, e1, op, p2, e2) \
    _test_val("PTR", const void*, "p", p1, e1, op, p2, e2)

#define test_ptr_lt(p1, p2) _test_ptr((p1), #p1,  <, (p2), #p2)
#define test_ptr_le(p1, p2) _test_ptr((p1), #p1, <=, (p2), #p2)
#define test_ptr_eq(p1, p2) _test_ptr((p1), #p1, ==, (p2), #p2)
#define test_ptr_ne(p1, p2) _test_ptr((p1), #p1, !=, (p2), #p2)
#define test_ptr_ge(p1, p2) _test_ptr((p1), #p1, >=, (p2), #p2)
#define test_ptr_gt(p1, p2) _test_ptr((p1), #p1,  >, (p2), #p2)

#define test_ptr_range(p1, p2, p3) \
    _test_range("PTR", const void*, "p", <=, (p1), #p1, (p2), #p2, (p3), #p3)

#define test_ptr_between(p1, p2, p3) \
    _test_range("PTR", const void*, "p", <, (p1), #p1, (p2), #p2, (p3), #p3)

#define test_ptr_list(l1, l2, n) \
    _test_list("PTR", const void*, "p", (l1), #l1, (l2), #l2, (n))


// --- [FLOAT] ---

#define _test_floating(v1, e1, op, v2, e2) do { \
    test_begin(); \
    \
    double _v1 = v1, _v2 = v2; \
    \
    if(!(_v1 op _v2)) \
        test_abort("FLOAT test failed: %s %s %s (%.*g %s %.*g)", \
            e1, #op, e2, DBL_DECIMAL_DIG, _v1, #op, DBL_DECIMAL_DIG, _v2); \
    \
    test_end(); \
} while(0)

#define _test_floating_range(op, v1, e1, v2, e2, v3, e3) do { \
    test_begin(); \
    \
    double _v1 = v1, _v2 = v2, _v3 = v3; \
    \
    if(!(_v1 op _v2 && _v2 op _v3)) \
        test_abort("FLOAT test failed: %s %s %s %s %s (%.*g %s %.*g %s %.*g)", \
            e1, #op, e2, #op, e3, \
            DBL_DECIMAL_DIG, _v1, #op, DBL_DECIMAL_DIG, _v2, #op, DBL_DECIMAL_DIG, _v3); \
    \
    test_end(); \
} while(0)

#define _test_floating_prop(v, e, op, expl) do { \
    test_begin(); \
    \
    double _v = v; \
    \
    if(!op(_v)) \
        test_abort("FLOAT test failed: %s %s (%.*g)", \
            e, expl, DBL_DECIMAL_DIG, _v); \
    \
    test_end(); \
} while(0)

#define test_float_list(l1, l2, n) do { \
    test_begin(); \
    \
    double _v1, _v2; \
    __auto_type _l1 = l1; \
    __auto_type _l2 = l2; \
    size_t _i, _n = n; \
    \
    for(_i=0; _i < _n; _i++) \
        if((_v1 = _l1[_i]) != (_v2 = _l2[_i])) \
            test_abort("FLOAT list test failed: %s[%zu] == %s[%zu] (%.*g == %.*g)", \
                #l1, _i, #l2, _i, DBL_DECIMAL_DIG, _v1, DBL_DECIMAL_DIG, _v2); \
    \
    test_end(); \
} while(0);


#define test_float_lt(f1, f2) _test_floating((f1), #f1,  <, (f2), #f2)
#define test_float_le(f1, f2) _test_floating((f1), #f1, <=, (f2), #f2)
#define test_float_eq(f1, f2) _test_floating((f1), #f1, ==, (f2), #f2)
#define test_float_ne(f1, f2) _test_floating((f1), #f1, !=, (f2), #f2)
#define test_float_ge(f1, f2) _test_floating((f1), #f1, >=, (f2), #f2)
#define test_float_gt(f1, f2) _test_floating((f1), #f1,  >, (f2), #f2)

#define test_float_range(f1, f2, f3) \
    _test_floating_range(<=, (f1), #f1, (f2), #f2, (f3), #f3)

#define test_float_between(f1, f2, f3) \
    _test_floating_range(<, (f1), #f1, (f2), #f2, (f3), #f3)

#define test_float_normal(f)     _test_floating_prop((f), #f, isnormal, "is normal")
#define test_float_not_normal(f) _test_floating_prop((f), #f, !isnormal, "is not normal")
#define test_float_nan(f)        _test_floating_prop((f), #f, isnan, "is NaN")
#define test_float_not_nan(f)    _test_floating_prop((f), #f, !isnan, "is not NaN")
#define test_float_finite(f)     _test_floating_prop((f), #f, isfinite, "is fininite")
#define test_float_infinite(f)   _test_floating_prop((f), #f, isinf, "is infinite")


// --- [STR] ---

#define _test_str(name, f, s1, e1, op, s2, e2) do { \
    test_begin(); \
    \
    const char *_s1 = s1, *_s2 = s2; \
    const char *_q1 = _s1 ? "\"" : "", *_q2 = _s2 ? "\"" : ""; \
    \
    if(!_s1 || !_s2 || !(f(_s1, _s2) op 0)) \
        test_abort("%s test failed: %s %s %s (%s%s%s %s %s%s%s)", \
            name, e1, #op, e2, \
            _q1, _s1 ? _s1 : "null", _q1, #op, \
            _q2, _s2 ? _s2 : "null", _q2); \
    \
    test_end(); \
} while(0)

#define _test_cstr(s1, e1, op, s2, e2) \
    _test_str("STR", strcmp, s1, e1, op, s2, e2)

#define test_str_lt(s1, s2) _test_cstr((s1), #s1,  <, (s2), #s2)
#define test_str_le(s1, s2) _test_cstr((s1), #s1, <=, (s2), #s2)
#define test_str_eq(s1, s2) _test_cstr((s1), #s1, ==, (s2), #s2)
#define test_str_ne(s1, s2) _test_cstr((s1), #s1, !=, (s2), #s2)
#define test_str_ge(s1, s2) _test_cstr((s1), #s1, >=, (s2), #s2)
#define test_str_gt(s1, s2) _test_cstr((s1), #s1,  >, (s2), #s2)

#define _test_istr(s1, e1, op, s2, e2) \
    _test_str("ISTR", strcasecmp, s1, e1, op, s2, e2)

#define test_istr_lt(s1, s2) _test_istr((s1), #s1,  <, (s2), #s2)
#define test_istr_le(s1, s2) _test_istr((s1), #s1, <=, (s2), #s2)
#define test_istr_eq(s1, s2) _test_istr((s1), #s1, ==, (s2), #s2)
#define test_istr_ne(s1, s2) _test_istr((s1), #s1, !=, (s2), #s2)
#define test_istr_ge(s1, s2) _test_istr((s1), #s1, >=, (s2), #s2)
#define test_istr_gt(s1, s2) _test_istr((s1), #s1,  >, (s2), #s2)


// --- [STRN] ---

#define _test_strn(name, f, s1, e1, op, s2, e2, n) do { \
    test_begin(); \
    \
    const char *_s1 = s1, *_s2 = s2; \
    const char *_q1 = _s1 ? "\"" : "", *_q2 = _s2 ? "\"" : ""; \
    int _n = n, _n1 = _s1 ? _n : 4, _n2 = _s2 ? _n : 4; \
    \
    if(!_s1 || !_s2 || !(f(_s1, _s2, n) op 0)) \
        test_abort("%s test failed: %s[:%d] %s %s[:%d] (%s%.*s%s %s %s%.*s%s)", \
            name, e1, _n, #op, e2, _n, \
            _q1, _n1, _s1 ? _s1 : "null", _q1, #op, \
            _q2, _n2, _s2 ? _s2 : "null", _q2); \
    \
    test_end(); \
} while(0)

#define _test_cstrn(s1, e1, op, s2, e2, n) \
    _test_strn("STRN", strncmp, s1, e1, op, s2, e2, n)

#define test_strn_lt(s1, s2, n) _test_cstrn((s1), #s1,  <, (s2), #s2, n)
#define test_strn_le(s1, s2, n) _test_cstrn((s1), #s1, <=, (s2), #s2, n)
#define test_strn_eq(s1, s2, n) _test_cstrn((s1), #s1, ==, (s2), #s2, n)
#define test_strn_ne(s1, s2, n) _test_cstrn((s1), #s1, !=, (s2), #s2, n)
#define test_strn_ge(s1, s2, n) _test_cstrn((s1), #s1, >=, (s2), #s2, n)
#define test_strn_gt(s1, s2, n) _test_cstrn((s1), #s1,  >, (s2), #s2, n)

#define _test_istrn(s1, e1, op, s2, e2, n) \
    _test_strn("ISTRN", strncasecmp, s1, e1, op, s2, e2, n)

#define test_istrn_lt(s1, s2, n) _test_istrn((s1), #s1,  <, (s2), #s2, n)
#define test_istrn_le(s1, s2, n) _test_istrn((s1), #s1, <=, (s2), #s2, n)
#define test_istrn_eq(s1, s2, n) _test_istrn((s1), #s1, ==, (s2), #s2, n)
#define test_istrn_ne(s1, s2, n) _test_istrn((s1), #s1, !=, (s2), #s2, n)
#define test_istrn_ge(s1, s2, n) _test_istrn((s1), #s1, >=, (s2), #s2, n)
#define test_istrn_gt(s1, s2, n) _test_istrn((s1), #s1,  >, (s2), #s2, n)


// --- [MEM] ---

// todo dump some memory
#define _test_mem(m1, e1, op, m2, e2, n) do { \
    test_begin(); \
    \
    const void *_m1 = m1, *_m2 = m2; \
    \
    if(!_m1 || !_m2 || !(memcmp(_m1, _m2, n) op 0)) \
        test_abort("MEM test failed: %s %s %s (0x%s %s 0x%s)", \
            e1, #op, e2, _m1 ? "?" : "null", #op, _m2 ? "?" : "null"); \
    \
    test_end(); \
} while(0)

#define test_mem_lt(m1, m2, n) _test_mem((m1), #m1,  <, (m2), #m2, n)
#define test_mem_le(m1, m2, n) _test_mem((m1), #m1, <=, (m2), #m2, n)
#define test_mem_eq(m1, m2, n) _test_mem((m1), #m1, ==, (m2), #m2, n)
#define test_mem_ne(m1, m2, n) _test_mem((m1), #m1, !=, (m2), #m2, n)
#define test_mem_ge(m1, m2, n) _test_mem((m1), #m1, >=, (m2), #m2, n)
#define test_mem_gt(m1, m2, n) _test_mem((m1), #m1,  >, (m2), #m2, n)


#ifdef _WIN32

// --- [HRESULT] ---

#define test_hresult_success(expr) do { \
    test_begin(); \
    \
    HRESULT result = (expr); \
    \
    if(result != S_OK) \
        test_abort("HRESULT SUCCESS test failed: %s (%s)", #expr, error_type_get_name(HRESULT, result)); \
    \
    test_end(); \
} while(0)

#define test_hresult_maybe(expr, eresult) do { \
    test_begin(); \
    \
    HRESULT result = (expr); \
    \
    if(result != S_OK && result != (eresult)) \
        test_abort("HRESULT MAYBE test failed: %s (%s == %s)", #expr, error_type_get_name(HRESULT, result), #eresult); \
    \
    test_end(); \
} while(0)

#define test_hresult_error(expr, eresult) do { \
    test_begin(); \
    \
    HRESULT result = (expr); \
    \
    if(result != (eresult)) \
        test_abort("HRESULT ERROR test failed: %s (%s == %s)", #expr, error_type_get_name(HRESULT, result), #eresult); \
    \
    test_end(); \
} while(0)


// --- [NTSTATUS] ---

#define test_ntstatus_success(expr) do { \
    test_begin(); \
    \
    NTSTATUS status = (expr); \
    \
    if(status != STATUS_SUCCESS) \
        test_abort("NTSTATUS SUCCESS test failed: %s (%s)", #expr, error_type_get_name(NTSTATUS, status)); \
    \
    test_end(); \
} while(0)

#define test_ntstatus_maybe(expr, estatus) do { \
    test_begin(); \
    \
    NTSTATUS status = (expr); \
    \
    if(status != STATUS_SUCCESS && status != (estatus)) \
        test_abort("NTSTATUS MAYBE test failed: %s (%s == %s)", #expr, error_type_get_name(NTSTATUS, status), #estatus); \
    \
    test_end(); \
} while(0)

#define test_ntstatus_error(expr, estatus) do { \
    test_begin(); \
    \
    NTSTATUS status = (expr); \
    \
    if(status != (estatus)) \
        test_abort("NTSTATUS ERROR test failed: %s (%s == %s)", #expr, error_type_get_name(NTSTATUS, status), #estatus); \
    \
    test_end(); \
} while(0)

#endif // _WIN32

#endif
