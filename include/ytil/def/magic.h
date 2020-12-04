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

#ifndef YTIL_DEF_MAGIC_H_INCLUDED
#define YTIL_DEF_MAGIC_H_INCLUDED

#include <ytil/ext/endian.h>
#include <assert.h>


#if NDEBUG

#define DEBUG_MAGIC
#define init_magic_n(obj, _magic)
#define check_magic_n(obj, _magic)
#define assert_magic_n(obj, magic)
#define try_magic_n(obj, magic)

#else

/// Magic variable to put into objects.
#define DEBUG_MAGIC unsigned int magic;

/// Init obj magic.
///
/// \param obj      object to init
/// \param _magic   magic value
#define init_magic_n(obj, _magic) \
    (obj)->magic = (_magic)

/// Check if obj exists and magic value matches.
///
/// \param obj      object to check
/// \param _magic   magic value to match
///
/// \retval true    magic good
/// \retval false   magic bad
#define check_magic_n(obj, _magic) \
    ((obj) && (obj)->magic == (_magic))

/// Assert that obj exists and magic value matches.
///
/// \param obj      object to check
/// \param _magic   magic value to match
#define assert_magic_n(obj, _magic) \
    assert((obj) && (obj)->magic == (_magic))

/// Assert that magic matches if obj exists.
///
/// \param obj      object to check
/// \param _magic   magic value to match
#define try_magic_n(obj, _magic) do {    \
    if((obj))                            \
        assert((obj)->magic == (_magic)) \
} while(0)

#endif // if NDEBUG


/// Init obj magic with default magic value.
///
/// \param obj      object to init
#define init_magic(obj) \
    init_magic_n(obj, MAGIC)

/// Check if obj exists and default magic value matches.
///
/// \param obj      object to check
///
/// \retval true    magic good
/// \retval false   magic bad
#define check_magic(obj) \
    check_magic_n(obj, MAGIC)

/// Assert that obj exists and default magic value matches.
///
/// \param obj      object to check
#define assert_magic(obj) \
    assert_magic_n(obj, MAGIC)

/// Assert that default magic matches if obj exists.
///
/// \param obj      object to check
#define try_magic(obj) \
    try_magic_n(obj, MAGIC)


/// Define magic value with string of 3 characters.
///
/// \param s    string to use as magic value
///
/// \returns    magic value
#if __BYTE_ORDER == __LITTLE_ENDIAN
    #define define_magic(s) __extension__({                        \
        static_assert(sizeof(s) == 4, "invalid magic string");     \
        (unsigned int)((s[2] << 24)|(s[1] << 16)|(s[0] << 8)|'*'); \
    })
#elif __BYTE_ORDER == __BIG_ENDIAN
    #define define_magic(s) __extension__({                        \
        static_assert(sizeof(s) == 4, "invalid magic string");     \
        (unsigned int)(('*' << 24)|(s[0] << 16)|(s[1] << 8)|s[2]); \
    })
#endif


#endif // ifndef YTIL_DEF_MAGIC_H_INCLUDED
