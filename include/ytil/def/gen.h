/*
 * Copyright (c) 2017-2020 Martin Rödel a.k.a. Yomin Nimoy
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

#ifndef YTIL_DEF_GEN_H_INCLUDED
#define YTIL_DEF_GEN_H_INCLUDED


/// Return the smaller of two values.
///
/// \param a    first value
/// \param b    second value
///
/// \returns    smaller value
#define MIN(a, b) __extension__ ({ \
    __auto_type _a = (a);          \
    __auto_type _b = (b);          \
    _a < _b ? _a : _b;             \
})

/// Return the smallest of three values.
///
/// \param a    first value
/// \param b    second value
/// \param c    third value
///
/// \returns    smallest value
#define MIN3(a, b, c) __extension__ ({                   \
    __auto_type _a = (a);                                \
    __auto_type _b = (b);                                \
    __auto_type _c = (c);                                \
    _a < _b ? (_a < _c ? _a : _c) : (_b < _c ? _b : _c); \
})

/// Return the larger of two values.
///
/// \param a    first value
/// \param b    second value
///
/// \returns    larger value
#define MAX(a, b) __extension__ ({ \
    __auto_type _a = (a);          \
    __auto_type _b = (b);          \
    _a > _b ? _a : _b;             \
})

/// Return the largest of three values.
///
/// \param a    first value
/// \param b    second value
/// \param c    third value
///
/// \returns    largest value
#define MAX3(a, b, c) __extension__ ({                   \
    __auto_type _a = (a);                                \
    __auto_type _b = (b);                                \
    __auto_type _c = (c);                                \
    _a > _b ? (_a > _c ? _a : _c) : (_b > _c ? _b : _c); \
})

/// Return the absolute value.
///
/// \param a    value
///
/// \returns    absolute value
#define ABS(a) __extension__ ({ \
    __auto_type _a = (a);       \
    _a < 0 ? -_a : _a;          \
})

/// Check if n is in range min <= n <= max.
///
/// \param n    value to check
/// \param min  minimum boundary
/// \param max  maximum boundary
///
/// \retval true    n is in range
/// \retval false   n is out of range
#define RANGE(n, min, max) __extension__ ({ \
    __auto_type _n = (n);                   \
    __auto_type _min = (min);               \
    __auto_type _max = (max);               \
    _min <= _n && _n <= _max;               \
})

/// Check if n is in range min < n < max.
///
/// \param n    value to check
/// \param min  minimum boundary
/// \param max  maximum boundary
///
/// \retval true    n is in range
/// \retval false   n is out of range
#define BETWEEN(n, min, max) __extension__ ({ \
    __auto_type _n = (n);                     \
    __auto_type _min = (min);                 \
    __auto_type _max = (max);                 \
    _min < _n && _n < _max;                   \
})

/// Clamp value to range min <= n <= max.
///
/// \param n    value to clamp
/// \param min  minimum boundary
/// \param max  maximum boundary
///
/// \returns    clamped value
#define CLAMP(n, min, max) __extension__ ({   \
    __auto_type _n = (n);                     \
    __auto_type _min = (min);                 \
    __auto_type _max = (max);                 \
    _n < _min ? _min : _n > _max ? _max : _n; \
})

/// Return default value if expr is NULL.
///
/// \param expr     expr to check for NULL
/// \param value    default value
///
/// \retval expr    if expr is not NULL
/// \retval value   if expr is NULL
#define IFNULL(expr, value) __extension__ ({ \
    __auto_type _expr = (expr);              \
    _expr ? _expr : (value);                 \
})

/// Return number of elements in static array.
///
/// \param array    array to get number of elements from
///
/// \returns        number of elements in array
#define ELEMS(array) \
    (sizeof(array) / sizeof(array[0]))

/// Suppress compiler warning for unused variable or function.
///
/// \param x    variable or function to suppress warning for
#define UNUSED(x) \
    (void)(x)

/// Suppress compiler warning for unused result.
///
/// \param expr     expr to suppress unused result warning for
#define UNUSED_RESULT(expr) \
    __extension__ ({ if((expr)) {}; })

/// Fallthrough statement attribute.
#define fallthrough \
    __attribute__((fallthrough))


/// Insert different values in debug and release mode.
///
/// \param d    value to insert in debug mode
/// \param r    value to insert in release mode
#if DOXYGEN
    #define DEBUG_RELEASE(d, r)
#elif NDEBUG
    #define DEBUG_RELEASE(d, r) r
#else
    #define DEBUG_RELEASE(d, r) d
#endif

/// Insert value only in debug mode.
///
/// \param d    value to insert
#define DEBUG(d) \
    DEBUG_RELEASE(d, )

/// Insert value only in release mode.
///
/// \param r    value to insert
#define RELEASE(r) \
    DEBUG_RELEASE(, r)

/// Insert object member only in debug mode.
///
/// \param m    member to insert
#define DEBUG_MEMBER(m) \
    DEBUG(m;)

/// Insert object member only in release mode.
///
/// \param m    member to insert
#define RELEASE_MEMBER(m) \
    RELEASE(m;)


#endif // ifndef YTIL_DEF_GEN_H_INCLUDED
