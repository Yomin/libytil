/*
 * Copyright (c) 2017-2019 Martin Rödel a.k.a. Yomin Nimoy
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

#ifndef YTIL_CAST_H_INCLUDED
#define YTIL_CAST_H_INCLUDED

#include <assert.h>


// cast value to pointer, check for truncation
#define VALUE_TO_POINTER(value) __extension__({ \
    static_assert(sizeof(value) <= sizeof(uintptr_t), "cast truncation"); \
    (void*)(uintptr_t)(value); })

// cast pointer to value, check for truncation
#define POINTER_TO_VALUE(ptr, type) __extension__({ \
    type value = (type)(uintptr_t)(ptr); \
    assert((uintptr_t)value == (uintptr_t)(ptr)); \
    value; })

// cast function pointer to object pointer, check for truncation
#define FUNC_TO_POINTER(func) __extension__({ \
    static_assert(sizeof(func) <= sizeof(uintptr_t), "cast truncation"); \
    (void*)(intptr_t)(func); })

// cast object pointer to function pointer
#define POINTER_TO_FUNC(ptr, type) \
    ((type)(intptr_t)(ptr))


#endif
