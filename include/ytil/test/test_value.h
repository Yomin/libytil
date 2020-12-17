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

#ifndef YTIL_TEST_TEST_NUM_H_INCLUDED
#define YTIL_TEST_TEST_NUM_H_INCLUDED

#include <ytil/test/test.h>
#include <stdint.h>
#include <string.h>
#include <float.h>
#include <math.h>


/// Test expression to evaluate to true.
///
/// \param expr     expression to test
#define test_true(expr) do {                            \
    test_begin();                                       \
                                                        \
    if(!(expr))                                         \
        test_abort_fail("TRUE test failed: %s", #expr); \
                                                        \
    test_end();                                         \
} while(0)

/// Test expression to evaluate to false.
///
/// \param expr     expression to test
#define test_false(expr) do {                            \
    test_begin();                                        \
                                                         \
    if((expr))                                           \
        test_abort_fail("FALSE test failed: %s", #expr); \
                                                         \
    test_end();                                          \
} while(0)


/// Comparison test of two generic values.
///
/// \param name     type name
/// \param type     value type
/// \param spec     type print specifier
/// \param v1       first value
/// \param t1       first value as text
/// \param op       compare operation
/// \param v2       second value
/// \param t2       second value as text
#define test_value(name, type, spec, v1, t1, op, v2, t2) do {         \
    test_begin();                                                     \
                                                                      \
    type _v1    = (v1);                                               \
    type _v2    = (v2);                                               \
                                                                      \
    if(!(_v1 op _v2))                                                 \
        test_abort_fail("%s test failed\n%s %s %s\n%"spec" %s %"spec, \
            name, t1, #op, t2, _v1, #op, _v2);                        \
                                                                      \
    test_end();                                                       \
} while(0)

/// Range test (v1 op v2 op v3) of generic values.
///
/// \param name     type name
/// \param type     value type
/// \param spec     type print specifier
/// \param op       compare operation
/// \param v1       start value
/// \param t1       start value as text
/// \param v2       in-range value
/// \param t2       in-range value as text
/// \param v3       end value
/// \param t3       end value as text
#define test_range(name, type, spec, op, v1, t1, v2, t2, v3, t3) do {            \
    test_begin();                                                                \
                                                                                 \
    type _v1    = (v1);                                                          \
    type _v2    = (v2);                                                          \
    type _v3    = (v3);                                                          \
                                                                                 \
    if(!(_v1 op _v2 && _v2 op _v3))                                              \
    {                                                                            \
        test_abort_fail(                                                         \
            "%s range test failed\n%s %s %s %s %s\n%"spec" %s %"spec" %s %"spec, \
            name, t1, #op, t2, #op, t3, _v1, #op, _v2, #op, _v3);                \
    }                                                                            \
                                                                                 \
    test_end();                                                                  \
} while(0)

/// List comparison test of generic values.
///
/// \param name     type name
/// \param type     value type
/// \param spec     type print specifier
/// \param l1       first list
/// \param t1       first list as text
/// \param op       compare operation
/// \param l2       second list
/// \param t2       second list as text
/// \param n        number of values to compare
#define test_list(name, type, spec, l1, t1, op, l2, t2, n) do {              \
    test_begin();                                                            \
                                                                             \
    __auto_type _l1 = (l1);                                                  \
    __auto_type _l2 = (l2);                                                  \
    size_t _n       = (n);                                                   \
    type _v1;                                                                \
    type _v2;                                                                \
    size_t _i;                                                               \
                                                                             \
    for(_i = 0; _i < _n; _i++)                                               \
    {                                                                        \
        if(!((_v1 = _l1[_i]) op (_v2 = _l2[_i])))                            \
        {                                                                    \
            test_abort_fail(                                                 \
                "%s list test failed\n%s[%ju] %s %s[%ju]\n%"spec" %s %"spec, \
                name, t1, _i, #op, t2, _i, _v1, #op, _v2);                   \
        }                                                                    \
    }                                                                        \
                                                                             \
    test_end();                                                              \
} while(0);


/// Comparison test of two signed integers.
///
/// \param i1   first signed int
/// \param t1   first signed int as text
/// \param op   compare operation
/// \param i2   second signed int
/// \param t2   second signed int as text
#define test_int(i1, t1, op, i2, t2) \
    test_value("INT", intmax_t, "jd", (i1), t1, op, (i2), t2)

/// Less-than comparison test of two signed integers.
///
/// \param i1   first signed int
/// \param i2   second signed int
#define test_int_lt(i1, i2) test_int((i1), #i1, <, (i2), #i2)

/// Less-equal comparison test of two signed integers.
///
/// \param i1   first signed int
/// \param i2   second signed int
#define test_int_le(i1, i2) test_int((i1), #i1, <=, (i2), #i2)

/// Equal comparison test of two signed integers.
///
/// \param i1   first signed int
/// \param i2   second signed int
#define test_int_eq(i1, i2) test_int((i1), #i1, ==, (i2), #i2)

/// Not-equal comparison test of two signed integers.
///
/// \param i1   first signed int
/// \param i2   second signed int
#define test_int_ne(i1, i2) test_int((i1), #i1, !=, (i2), #i2)

/// Greater-equal comparison test of two signed integers.
///
/// \param i1   first signed int
/// \param i2   second signed int
#define test_int_ge(i1, i2) test_int((i1), #i1, >=, (i2), #i2)

/// Greater-than comparison test of two signed integers.
///
/// \param i1   first signed int
/// \param i2   second signed int
#define test_int_gt(i1, i2) test_int((i1), #i1, >, (i2), #i2)

/// Range test (i1 <= i2 <= i3) of signed integers.
///
/// \param i1   start signed int
/// \param i2   in-range signed int
/// \param i3   end signed int
#define test_int_range(i1, i2, i3) \
    test_range("INT", intmax_t, "jd", <=, (i1), #i1, (i2), #i2, (i3), #i3)

/// Between test (i1 < i2 < i3) of signed integers.
///
/// \param i1   start signed int
/// \param i2   in-between signed int
/// \param i3   end signed int
#define test_int_between(i1, i2, i3) \
    test_range("INT", intmax_t, "jd", <, (i1), #i1, (i2), #i2, (i3), #i3)

/// List comparison test of two signed integer lists.
///
/// \param l1   first list
/// \param t1   first list as text
/// \param op   compare operation
/// \param l2   second list
/// \param t2   second list as text
/// \param n    number of values to compare
#define test_int_list(l1, t1, op, l2, t2, n) \
    test_list("INT", intmax_t, "jd", (l1), #l1, op, (l2), #l2, (n))

/// Less-than list comparison test of two signed integer lists.
///
/// \param l1   first list
/// \param l2   second list
/// \param n    number of values to compare
#define test_int_list_lt(l1, l2, n) \
    test_int_list((l1), #l1, <, (l2), #l2, (n))

/// Less-equal list comparison test of two signed integer lists.
///
/// \param l1   first list
/// \param l2   second list
/// \param n    number of values to compare
#define test_int_list_le(l1, l2, n) \
    test_int_list((l1), #l1, <=, (l2), #l2, (n))

/// Equal list comparison test of two signed integer lists.
///
/// \param l1   first list
/// \param l2   second list
/// \param n    number of values to compare
#define test_int_list_eq(l1, l2, n) \
    test_int_list((l1), #l1, ==, (l2), #l2, (n))

/// Not-equal list comparison test of two signed integer lists.
///
/// \param l1   first list
/// \param l2   second list
/// \param n    number of values to compare
#define test_int_list_ne(l1, l2, n) \
    test_int_list((l1), #l1, !=, (l2), #l2, (n))

/// Greater-equal list comparison test of two signed integer lists.
///
/// \param l1   first list
/// \param l2   second list
/// \param n    number of values to compare
#define test_int_list_ge(l1, l2, n) \
    test_int_list((l1), #l1, >=, (l2), #l2, (n))

/// Greater-than list comparison test of two signed integer lists.
///
/// \param l1   first list
/// \param l2   second list
/// \param n    number of values to compare
#define test_int_list_gt(l1, l2, n) \
    test_int_list((l1), #l1, >, (l2), #l2, (n))


/// Comparison test of two unsigned integers.
///
/// \param u1   first unsigned int
/// \param t1   first unsigned int as text
/// \param op   compare operation
/// \param u2   second unsigned int
/// \param t2   second unsigned int as text
#define test_uint(u1, t1, op, u2, t2) \
    test_value("UINT", uintmax_t, "ju", (u1), t1, op, (u2), t2)

/// Less-than comparison test of two unsigned integers.
///
/// \param u1   first unsigned int
/// \param u2   second unsigned int
#define test_uint_lt(u1, u2) test_uint((u1), #u1, <, (u2), #u2)

/// Less-equal comparison test of two unsigned integers.
///
/// \param u1   first unsigned int
/// \param u2   second unsigned int
#define test_uint_le(u1, u2) test_uint((u1), #u1, <=, (u2), #u2)

/// Equal comparison test of two unsigned integers.
///
/// \param u1   first unsigned int
/// \param u2   second unsigned int
#define test_uint_eq(u1, u2) test_uint((u1), #u1, ==, (u2), #u2)

/// Not-equal comparison test of two unsigned integers.
///
/// \param u1   first unsigned int
/// \param u2   second unsigned int
#define test_uint_ne(u1, u2) test_uint((u1), #u1, !=, (u2), #u2)

/// Greater-equal comparison test of two unsigned integers.
///
/// \param u1   first unsigned int
/// \param u2   second unsigned int
#define test_uint_ge(u1, u2) test_uint((u1), #u1, >=, (u2), #u2)

/// Greater-than comparison test of two unsigned integers.
///
/// \param u1   first unsigned int
/// \param u2   second unsigned int
#define test_uint_gt(u1, u2) test_uint((u1), #u1, >, (u2), #u2)

/// Range test (u1 <= u2 <= u3) of unsigned integers.
///
/// \param u1   start unsigned int
/// \param u2   in-range unsigned int
/// \param u3   end unsigned int
#define test_uint_range(u1, u2, u3) \
    test_range("UINT", uintmax_t, "jd", <=, (u1), #u1, (u2), #u2, (u3), #u3)

/// Between test (u1 < u2 < u3) of unsigned integers.
///
/// \param u1   start unsigned int
/// \param u2   in-between unsigned int
/// \param u3   end unsigned int
#define test_uint_between(u1, u2, u3) \
    test_range("UINT", uintmax_t, "jd", <, (u1), #u1, (u2), #u2, (u3), #u3)

/// List comparison test of two unsigned integer lists.
///
/// \param l1   first list
/// \param t1   first list as text
/// \param op   compare operation
/// \param l2   second list
/// \param t2   second list as text
/// \param n    number of values to compare
#define test_uint_list(l1, t1, op, l2, t2, n) \
    test_list("UINT", uintmax_t, "ju", (l1), #l1, op, (l2), #l2, (n))

/// Less-than list comparison test of two unsigned integer lists.
///
/// \param l1   first list
/// \param l2   second list
/// \param n    number of values to compare
#define test_uint_list_lt(l1, l2, n) \
    test_uint_list((l1), #l1, <, (l2), #l2, (n))

/// Less-equal list comparison test of two unsigned integer lists.
///
/// \param l1   first list
/// \param l2   second list
/// \param n    number of values to compare
#define test_uint_list_le(l1, l2, n) \
    test_uint_list((l1), #l1, <=, (l2), #l2, (n))

/// Equal list comparison test of two unsigned integer lists.
///
/// \param l1   first list
/// \param l2   second list
/// \param n    number of values to compare
#define test_uint_list_eq(l1, l2, n) \
    test_uint_list((l1), #l1, ==, (l2), #l2, (n))

/// Not-equal list comparison test of two unsigned integer lists.
///
/// \param l1   first list
/// \param l2   second list
/// \param n    number of values to compare
#define test_uint_list_ne(l1, l2, n) \
    test_uint_list((l1), #l1, !=, (l2), #l2, (n))

/// Greater-equal list comparison test of two unsigned integer lists.
///
/// \param l1   first list
/// \param l2   second list
/// \param n    number of values to compare
#define test_uint_list_ge(l1, l2, n) \
    test_uint_list((l1), #l1, >=, (l2), #l2, (n))

/// Greater-than list comparison test of two unsigned integer lists.
///
/// \param l1   first list
/// \param l2   second list
/// \param n    number of values to compare
#define test_uint_list_gt(l1, l2, n) \
    test_uint_list((l1), #l1, >, (l2), #l2, (n))


/// Comparison test of two pointers.
///
/// \param p1   first pointer
/// \param t1   first pointer as text
/// \param op   compare operation
/// \param p2   second pointer
/// \param t2   second pointer as text
#define test_ptr(p1, t1, op, p2, t2) \
    test_value("PTR", const void *, "p", (p1), t1, op, (p2), t2)

/// Less-than comparison test of two pointers.
///
/// \param p1   first pointer
/// \param p2   second pointer
#define test_ptr_lt(p1, p2) test_ptr((p1), #p1, <, (p2), #p2)

/// Less-equal comparison test of two pointers.
///
/// \param p1   first pointer
/// \param p2   second pointer
#define test_ptr_le(p1, p2) test_ptr((p1), #p1, <=, (p2), #p2)

/// Equal comparison test of two pointers.
///
/// \param p1   first pointer
/// \param p2   second pointer
#define test_ptr_eq(p1, p2) test_ptr((p1), #p1, ==, (p2), #p2)

/// Not-equal comparison test of two pointers.
///
/// \param p1   first pointer
/// \param p2   second pointer
#define test_ptr_ne(p1, p2) test_ptr((p1), #p1, !=, (p2), #p2)

/// Greater-equal comparison test of two pointers.
///
/// \param p1   first pointer
/// \param p2   second pointer
#define test_ptr_ge(p1, p2) test_ptr((p1), #p1, >=, (p2), #p2)

/// Greater-than comparison test of two pointers.
///
/// \param p1   first pointer
/// \param p2   second pointer
#define test_ptr_gt(p1, p2) test_ptr((p1), #p1, >, (p2), #p2)

/// Range test (p1 <= p2 <= p3) of pointers.
///
/// \param p1   start pointer
/// \param p2   in-range pointer
/// \param p3   end pointer
#define test_ptr_range(p1, p2, p3) \
    test_range("PTR", const void *, "p", <=, (p1), #p1, (p2), #p2, (p3), #p3)

/// Between test (p1 < p2 < p3) of pointers.
///
/// \param p1   start pointer
/// \param p2   in-between pointer
/// \param p3   end pointer
#define test_ptr_between(p1, p2, p3) \
    test_range("PTR", const void *, "p", <, (p1), #p1, (p2), #p2, (p3), #p3)

/// List comparison test of two pointer lists.
///
/// \param l1   first list
/// \param t1   first list as text
/// \param op   compare operation
/// \param l2   second list
/// \param t2   second list as text
/// \param n    number of values to compare
#define test_ptr_list(l1, t1, op, l2, t2, n) \
    test_list("PTR", const void *, "p", (l1), #l1, op, (l2), #l2, (n))

/// Less-than list comparison test of two pointer lists.
///
/// \param l1   first list
/// \param l2   second list
/// \param n    number of values to compare
#define test_ptr_list_lt(l1, l2, n) \
    test_ptr_list((l1), #l1, <, (l2), #l2, (n))

/// Less-equal list comparison test of two pointer lists.
///
/// \param l1   first list
/// \param l2   second list
/// \param n    number of values to compare
#define test_ptr_list_le(l1, l2, n) \
    test_ptr_list((l1), #l1, <=, (l2), #l2, (n))

/// Equal list comparison test of two pointer lists.
///
/// \param l1   first list
/// \param l2   second list
/// \param n    number of values to compare
#define test_ptr_list_eq(l1, l2, n) \
    test_ptr_list((l1), #l1, ==, (l2), #l2, (n))

/// Not-equal list comparison test of two pointer lists.
///
/// \param l1   first list
/// \param l2   second list
/// \param n    number of values to compare
#define test_ptr_list_ne(l1, l2, n) \
    test_ptr_list((l1), #l1, !=, (l2), #l2, (n))

/// Greater-equal list comparison test of two pointer lists.
///
/// \param l1   first list
/// \param l2   second list
/// \param n    number of values to compare
#define test_ptr_list_ge(l1, l2, n) \
    test_ptr_list((l1), #l1, >=, (l2), #l2, (n))

/// Greater-than list comparison test of two pointer lists.
///
/// \param l1   first list
/// \param l2   second list
/// \param n    number of values to compare
#define test_ptr_list_gt(l1, l2, n) \
    test_ptr_list((l1), #l1, >, (l2), #l2, (n))


/// Comparison test of two floats.
///
/// \param f1       first float
/// \param t1       first float as text
/// \param op       compare operation
/// \param f2       second float
/// \param t2       second float as text
#define test_float(f1, t1, op, f2, t2) do {                                \
    test_begin();                                                          \
                                                                           \
    double _f1  = (f1);                                                    \
    double _f2  = (f2);                                                    \
                                                                           \
    if(!(_f1 op _f2))                                                      \
    {                                                                      \
        test_abort_fail("FLOAT test failed\n%s %s %s\n%.*g %s %.*g",       \
            t1, #op, t2, DBL_DECIMAL_DIG, _f1, #op, DBL_DECIMAL_DIG, _f2); \
    }                                                                      \
                                                                           \
    test_end();                                                            \
} while(0)

/// Less-than comparison test of two floats.
///
/// \param f1   first float
/// \param f2   second float
#define test_float_lt(f1, f2) test_float((f1), #f1, <, (f2), #f2)

/// Less-equal comparison test of two floats.
///
/// \param f1   first float
/// \param f2   second float
#define test_float_le(f1, f2) test_float((f1), #f1, <=, (f2), #f2)

/// Equal comparison test of two floats.
///
/// \param f1   first float
/// \param f2   second float
#define test_float_eq(f1, f2) test_float((f1), #f1, ==, (f2), #f2)

/// Not-equal comparison test of two floats.
///
/// \param f1   first float
/// \param f2   second float
#define test_float_ne(f1, f2) test_float((f1), #f1, !=, (f2), #f2)

/// Greater-equal comparison test of two floats.
///
/// \param f1   first float
/// \param f2   second float
#define test_float_ge(f1, f2) test_float((f1), #f1, >=, (f2), #f2)

/// Greater-than comparison test of two floats.
///
/// \param f1   first float
/// \param f2   second float
#define test_float_gt(f1, f2) test_float((f1), #f1, >, (f2), #f2)

/// Range test (f1 op f2 op f3) of floats.
///
/// \param op       compare operation
/// \param f1       start float
/// \param t1       start float as text
/// \param f2       in-range float
/// \param t2       in-range float as text
/// \param f3       end float
/// \param t3       end float as text
#define test_float_range_f(op, f1, t1, f2, t2, f3, t3) do {                              \
    test_begin();                                                                        \
                                                                                         \
    double _f1  = (f1);                                                                  \
    double _f2  = (f2);                                                                  \
    double _f3  = (f3);                                                                  \
                                                                                         \
    if(!(_f1 op _f2 && _f2 op _f3))                                                      \
    {                                                                                    \
        test_abort_fail(                                                                 \
            "FLOAT range test failed\n%s %s %s %s %s\n%.*g %s %.*g %s %.*g",             \
            t1, #op, t2, #op, t3,                                                        \
            DBL_DECIMAL_DIG, _f1, #op, DBL_DECIMAL_DIG, _f2, #op, DBL_DECIMAL_DIG, _f3); \
    }                                                                                    \
                                                                                         \
    test_end();                                                                          \
} while(0)

/// Range test (f1 <= f2 <= f3) of floats.
///
/// \param f1   start float
/// \param f2   in-range float
/// \param f3   end float
#define test_float_range(f1, f2, f3) \
    test_float_range_f(<=, (f1), #f1, (f2), #f2, (f3), #f3)

/// Between test (f1 < f2 < f3) of floats.
///
/// \param f1   start float
/// \param f2   in-between float
/// \param f3   end float
#define test_float_between(f1, f2, f3) \
    test_float_range_f(<, (f1), #f1, (f2), #f2, (f3), #f3)

/// List comparison test of two float lists.
///
/// \param l1       first list
/// \param t1       first list as text
/// \param op       compare operation
/// \param l2       second list
/// \param t2       second list as text
/// \param n        number of values to compare
#define test_float_list(l1, t1, op, l2, t2, n) do {                                    \
    test_begin();                                                                      \
                                                                                       \
    __auto_type _l1 = (l1);                                                            \
    __auto_type _l2 = (l2);                                                            \
    size_t _n       = (n);                                                             \
    double _f1;                                                                        \
    double _f2;                                                                        \
    size_t _i;                                                                         \
                                                                                       \
    for(_i = 0; _i < _n; _i++)                                                         \
    {                                                                                  \
        if(!((_f1 = _l1[_i]) op (_f2 = _l2[_i])))                                      \
        {                                                                              \
            test_abort_fail(                                                           \
                "FLOAT list test failed\n%s[%ju] %s %s[%ju]\n%.*g %s %.*g",            \
                t1, _i, #op, t2, _i, DBL_DECIMAL_DIG, _f1, #op, DBL_DECIMAL_DIG, _f2); \
        }                                                                              \
    }                                                                                  \
                                                                                       \
    test_end();                                                                        \
} while(0);

/// Less-than list comparison test of two float lists.
///
/// \param l1   first list
/// \param l2   second list
/// \param n    number of values to compare
#define test_float_list_lt(l1, l2, n) \
    test_float_list((l1), #l1, <, (l2), #l2, (n))

/// Less-equal list comparison test of two float lists.
///
/// \param l1   first list
/// \param l2   second list
/// \param n    number of values to compare
#define test_float_list_le(l1, l2, n) \
    test_float_list((l1), #l1, <=, (l2), #l2, (n))

/// Equal list comparison test of two float lists.
///
/// \param l1   first list
/// \param l2   second list
/// \param n    number of values to compare
#define test_float_list_eq(l1, l2, n) \
    test_float_list((l1), #l1, ==, (l2), #l2, (n))

/// Not-equal list comparison test of two float lists.
///
/// \param l1   first list
/// \param l2   second list
/// \param n    number of values to compare
#define test_float_list_ne(l1, l2, n) \
    test_float_list((l1), #l1, !=, (l2), #l2, (n))

/// Greater-equal list comparison test of two float lists.
///
/// \param l1   first list
/// \param l2   second list
/// \param n    number of values to compare
#define test_float_list_ge(l1, l2, n) \
    test_float_list((l1), #l1, >=, (l2), #l2, (n))

/// Greater-than list comparison test of two float lists.
///
/// \param l1   first list
/// \param l2   second list
/// \param n    number of values to compare
#define test_float_list_gt(l1, l2, n) \
    test_float_list((l1), #l1, >, (l2), #l2, (n))

/// Float property test.
///
/// \param f    float
/// \param t    float as text
/// \param op   property operation
/// \param msg  property text
#define test_float_prop(f, t, op, msg) do {                        \
    test_begin();                                                  \
                                                                   \
    double _f = (f);                                               \
                                                                   \
    if(!(op(_f)))                                                  \
    {                                                              \
        test_abort_fail("FLOAT property test failed\n%s %s\n%.*g", \
            t, msg, DBL_DECIMAL_DIG, _f);                          \
    }                                                              \
                                                                   \
    test_end();                                                    \
} while(0)

/// Test for float to be normal.
///
/// \param f    float
#define test_float_is_normal(f) test_float_prop((f), #f, isnormal, "is normal")

/// Test for float to be not normal.
///
/// \param f    float
#define test_float_is_not_normal(f) test_float_prop((f), #f, !isnormal, "is not normal")

/// Test for float to be NaN.
///
/// \param f    float
#define test_float_is_nan(f) test_float_prop((f), #f, isnan, "is NaN")

/// Test for float to be not NaN.
///
/// \param f    float
#define test_float_is_not_nan(f) test_float_prop((f), #f, !isnan, "is not NaN")

/// Test for float to be finite.
///
/// \param f    float
#define test_float_is_finite(f) test_float_prop((f), #f, isfinite, "is finite")

/// Test for float to be infinite.
///
/// \param f    float
#define test_float_is_infinite(f) test_float_prop((f), #f, isinf, "is infinite")


/// Comparison test of two strings.
///
/// \param name     type name
/// \param f        compare function
/// \param s1       first string
/// \param t1       first string as text
/// \param op       compare operation
/// \param s2       second string
/// \param t2       second string as text
#define test_str(name, f, s1, t1, op, s2, t2) do {                    \
    test_begin();                                                     \
                                                                      \
    const char *_s1 = (s1);                                           \
    const char *_s2 = (s2);                                           \
                                                                      \
    if(!_s1 || !_s2 || !(f(_s1, _s2) op 0))                           \
    {                                                                 \
        test_abort_fail("%s test failed\n%s %s %s\n%s%s%s %s %s%s%s", \
            name, t1, #op, t2,                                        \
            _s1 ? "\"\x02" : "",                                      \
            _s1 ? _s1 : "null",                                       \
            _s1 ? "\x03\"" : "",                                      \
            #op,                                                      \
            _s2 ? "\"\x02" : "",                                      \
            _s2 ? _s2 : "null",                                       \
            _s2 ? "\x03\"" : "");                                     \
    }                                                                 \
                                                                      \
    test_end();                                                       \
} while(0)

/// Case sensitive comparison test of two strings.
///
/// \param s1       first string
/// \param t1       first string as text
/// \param op       compare operation
/// \param s2       second string
/// \param t2       second string as text
#define test_cstr(s1, t1, op, s2, t2) \
    test_str("STR", strcmp, (s1), t1, op, (s2), t2)

/// Less-than case sensitive comparison test of two strings.
///
/// \param s1   first string
/// \param s2   second string
#define test_str_lt(s1, s2) test_cstr((s1), #s1, <, (s2), #s2)

/// Less-equal case sensitive comparison test of two strings.
///
/// \param s1   first string
/// \param s2   second string
#define test_str_le(s1, s2) test_cstr((s1), #s1, <=, (s2), #s2)

/// Equal case sensitive comparison test of two strings.
///
/// \param s1   first string
/// \param s2   second string
#define test_str_eq(s1, s2) test_cstr((s1), #s1, ==, (s2), #s2)

/// Not-equal case sensitive comparison test of two strings.
///
/// \param s1   first string
/// \param s2   second string
#define test_str_ne(s1, s2) test_cstr((s1), #s1, !=, (s2), #s2)

/// Greater-equal case sensitive comparison test of two strings.
///
/// \param s1   first string
/// \param s2   second string
#define test_str_ge(s1, s2) test_cstr((s1), #s1, >=, (s2), #s2)

/// Greater-than case sensitive comparison test of two strings.
///
/// \param s1   first string
/// \param s2   second string
#define test_str_gt(s1, s2) test_cstr((s1), #s1, >, (s2), #s2)

/// Case insensitive comparison test of two strings.
///
/// \param s1       first string
/// \param t1       first string as text
/// \param op       compare operation
/// \param s2       second string
/// \param t2       second string as text
#define test_istr(s1, t1, op, s2, t2) \
    test_str("ISTR", strcasecmp, (s1), t1, op, (s2), t2)

/// Less-than case insensitive comparison test of two strings.
///
/// \param s1   first string
/// \param s2   second string
#define test_istr_lt(s1, s2) test_istr((s1), #s1, <, (s2), #s2)

/// Less-equal case insensitive comparison test of two strings.
///
/// \param s1   first string
/// \param s2   second string
#define test_istr_le(s1, s2) test_istr((s1), #s1, <=, (s2), #s2)

/// Equal case insensitive comparison test of two strings.
///
/// \param s1   first string
/// \param s2   second string
#define test_istr_eq(s1, s2) test_istr((s1), #s1, ==, (s2), #s2)

/// Not-equal case insensitive comparison test of two strings.
///
/// \param s1   first string
/// \param s2   second string
#define test_istr_ne(s1, s2) test_istr((s1), #s1, !=, (s2), #s2)

/// Greater-equal case insensitive comparison test of two strings.
///
/// \param s1   first string
/// \param s2   second string
#define test_istr_ge(s1, s2) test_istr((s1), #s1, >=, (s2), #s2)

/// Greater-than case insensitive comparison test of two strings.
///
/// \param s1   first string
/// \param s2   second string
#define test_istr_gt(s1, s2) test_istr((s1), #s1, >, (s2), #s2)


/// Comparison test of two string prefixes.
///
/// \param name     type name
/// \param f        compare function
/// \param p1       first string prefix
/// \param t1       first string prefix as text
/// \param op       compare operation
/// \param p2       second string prefix
/// \param t2       second string prefix as text
/// \param n        number of characters to compare
#define test_prefix(name, f, p1, t1, op, p2, t2, n) do {                            \
    test_begin();                                                                   \
                                                                                    \
    const char *_p1 = (p1);                                                         \
    const char *_p2 = (p2);                                                         \
    int _n          = (n);                                                          \
                                                                                    \
    if(!_p1 || !_p2 || !(f(_p1, _p2, _n) op 0))                                     \
    {                                                                               \
        test_abort_fail("%s test failed\n%s[:%d] %s %s[:%d]\n%s%.*s%s %s %s%.*s%s", \
            name, t1, _n, #op, t2, _n,                                              \
            _p1 ? "\"\x02" : "", _p1 ? _n : 4,                                      \
            _p1 ? _p1 : "null",                                                     \
            _p1 ? "\x03\"" : "",                                                    \
            #op,                                                                    \
            _p2 ? "\"\x02" : "", _p2 ? _n : 4,                                      \
            _p2 ? _p2 : "null",                                                     \
            _p2 ? "\x03\"" : "");                                                   \
    }                                                                               \
                                                                                    \
    test_end();                                                                     \
} while(0)

/// Case sensitive comparison test of two string prefixes.
///
/// \param p1       first string prefix
/// \param t1       first string prefix as text
/// \param op       compare operation
/// \param p2       second string prefix
/// \param t2       second string prefix as text
/// \param n        number of characters to compare
#define test_cprefix(p1, t1, op, p2, t2, n) \
    test_prefix("PREFIX", strncmp, (p1), t1, op, (p2), t2, (n))

/// Less-than case sensitive comparison test of two string prefixes.
///
/// \param p1   first string prefix
/// \param p2   second string prefix
/// \param n    number of characters to compare
#define test_prefix_lt(p1, p2, n) test_cprefix((p1), #p1, <, (p2), #p2, (n))

/// Less-equal case sensitive comparison test of two string prefixes.
///
/// \param p1   first string prefix
/// \param p2   second string prefix
/// \param n    number of characters to compare
#define test_prefix_le(p1, p2, n) test_cprefix((p1), #p1, <=, (p2), #p2, (n))

/// Equal case sensitive comparison test of two string prefixes.
///
/// \param p1   first string prefix
/// \param p2   second string prefix
/// \param n    number of characters to compare
#define test_prefix_eq(p1, p2, n) test_cprefix((p1), #p1, ==, (p2), #p2, (n))

/// Not-equal case sensitive comparison test of two string prefixes.
///
/// \param p1   first string prefix
/// \param p2   second string prefix
/// \param n    number of characters to compare
#define test_prefix_ne(p1, p2, n) test_cprefix((p1), #p1, !=, (p2), #p2, (n))

/// Greater-equal case sensitive comparison test of two string prefixes.
///
/// \param p1   first string prefix
/// \param p2   second string prefix
/// \param n    number of characters to compare
#define test_prefix_ge(p1, p2, n) test_cprefix((p1), #p1, >=, (p2), #p2, (n))

/// Greater-than case sensitive comparison test of two string prefixes.
///
/// \param p1   first string prefix
/// \param p2   second string prefix
/// \param n    number of characters to compare
#define test_prefix_gt(p1, p2, n) test_cprefix((p1), #p1, >, (p2), #p2, (n))

/// Case insensitive comparison test of two string prefixes.
///
/// \param p1       first string prefix
/// \param t1       first string prefix as text
/// \param op       compare operation
/// \param p2       second string prefix
/// \param t2       second string prefix as text
/// \param n        number of characters to compare
#define test_iprefix(p1, t1, op, p2, t2, n) \
    test_prefix("IPREFIX", strncasecmp, (p1), t1, op, (p2), t2, (n))

/// Less-than case insensitive comparison test of two string prefixes.
///
/// \param p1   first string prefix
/// \param p2   second string prefix
/// \param n    number of characters to compare
#define test_iprefix_lt(p1, p2, n) test_iprefix((p1), #p1, <, (p2), #p2, (n))

/// Less-equal case insensitive comparison test of two string prefixes.
///
/// \param p1   first string prefix
/// \param p2   second string prefix
/// \param n    number of characters to compare
#define test_iprefix_le(p1, p2, n) test_iprefix((p1), #p1, <=, (p2), #p2, (n))

/// Equal case insensitive comparison test of two string prefixes.
///
/// \param p1   first string prefix
/// \param p2   second string prefix
/// \param n    number of characters to compare
#define test_iprefix_eq(p1, p2, n) test_iprefix((p1), #p1, ==, (p2), #p2, (n))

/// Not-equal case insensitive comparison test of two string prefixes.
///
/// \param p1   first string prefix
/// \param p2   second string prefix
/// \param n    number of characters to compare
#define test_iprefix_ne(p1, p2, n) test_iprefix((p1), #p1, !=, (p2), #p2, (n))

/// Greater-equal case insensitive comparison test of two string prefixes.
///
/// \param p1   first string prefix
/// \param p2   second string prefix
/// \param n    number of characters to compare
#define test_iprefix_ge(p1, p2, n) test_iprefix((p1), #p1, >=, (p2), #p2, (n))

/// Greater-than case insensitive comparison test of two string prefixes.
///
/// \param p1   first string prefix
/// \param p2   second string prefix
/// \param n    number of characters to compare
#define test_iprefix_gt(p1, p2, n) test_iprefix((p1), #p1, >, (p2), #p2, (n))


/// Comparison test of two memory areas.
///
/// \param m1       first memory area
/// \param t1       first memory area as text
/// \param op       compare operation
/// \param m2       second memory area
/// \param t2       second memory area as text
/// \param n        number of bytes to compare
#define test_mem(m1, t1, op, m2, t2, n) do {                       \
    test_begin();                                                  \
                                                                   \
    const void *_m1 = (m1);                                        \
    const void *_m2 = (m2);                                        \
    size_t _n       = (n);                                         \
                                                                   \
    if(!_m1 || !_m2 || !(memcmp(_m1, _m2, _n) op 0))               \
        test_abort_fail("MEM test failed\n%s %s %s", t1, #op, t2); \
                                                                   \
    test_end();                                                    \
} while(0)

/// Less-than comparison test of two memory areas.
///
/// \param m1   first memory area
/// \param m2   second memory area
/// \param n    number of bytes to compare
#define test_mem_lt(m1, m2, n) test_mem((m1), #m1, <, (m2), #m2, (n))

/// Less-equal comparison test of two memory areas.
///
/// \param m1   first memory area
/// \param m2   second memory area
/// \param n    number of bytes to compare
#define test_mem_le(m1, m2, n) test_mem((m1), #m1, <=, (m2), #m2, (n))

/// Equal comparison test of two memory areas.
///
/// \param m1   first memory area
/// \param m2   second memory area
/// \param n    number of bytes to compare
#define test_mem_eq(m1, m2, n) test_mem((m1), #m1, ==, (m2), #m2, (n))

/// Not-equal comparison test of two memory areas.
///
/// \param m1   first memory area
/// \param m2   second memory area
/// \param n    number of bytes to compare
#define test_mem_ne(m1, m2, n) test_mem((m1), #m1, !=, (m2), #m2, (n))

/// Greater-equal comparison test of two memory areas.
///
/// \param m1   first memory area
/// \param m2   second memory area
/// \param n    number of bytes to compare
#define test_mem_ge(m1, m2, n) test_mem((m1), #m1, >=, (m2), #m2, (n))

/// Greater-than comparison test of two memory areas.
///
/// \param m1   first memory area
/// \param m2   second memory area
/// \param n    number of bytes to compare
#define test_mem_gt(m1, m2, n) test_mem((m1), #m1, >, (m2), #m2, (n))


#endif // ifndef YTIL_TEST_TEST_NUM_H_INCLUDED
