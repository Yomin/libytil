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

#ifndef YTIL_DEF_CAST_H_INCLUDED
#define YTIL_DEF_CAST_H_INCLUDED

#include <assert.h>
#include <stdint.h>


/// Cast value to pointer, check for truncation.
///
/// \param value    value to cast
///
/// \returns        void pointer
#define VALUE_TO_POINTER(value) __extension__ ({                          \
    static_assert(sizeof(value) <= sizeof(uintptr_t), "cast truncation"); \
    (void *)(uintptr_t)(value);                                           \
})

/// Cast pointer to value, check for truncation.
///
/// \param ptr      pointer to cast
/// \param type     type to cast pointer to
///
/// \returns        value of \p type
#define POINTER_TO_VALUE(ptr, type) __extension__ ({ \
    type value = (type)(uintptr_t)(ptr);             \
    assert((uintptr_t)value == (uintptr_t)(ptr));    \
    value;                                           \
})

/// Cast function pointer to object pointer.
///
/// \param func     function to cast
///
/// \returns        void pointer
#define FUNC_TO_POINTER(func) \
    ((void *)(intptr_t)(func))

/// Cast object pointer to function pointer.
///
/// \param ptr      pointer to cast
/// \param type     function type to cast to
///
/// \returns        function pointer of \p type
#define POINTER_TO_FUNC(ptr, type) \
    ((type)(intptr_t)(ptr))


#endif // ifndef YTIL_DEF_CAST_H_INCLUDED
