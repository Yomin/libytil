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

#ifndef __YTIL_MAGIC_H__
#define __YTIL_MAGIC_H__

#include <stdlib.h>
#include <endian.h>
#include <assert.h>


#ifdef NDEBUG
    
    // does nothing in non-debug mode
    #define DEBUG_MAGIC
    
    // does nothing in non-debug mode
    #define init_magic_n(obj, _magic)
    
    // check if obj exists
    #define check_magic_n(obj, _magic) \
        !!(obj)
    
    // abort if obj not exists
    #define assert_magic_n(obj, magic) \
        if(!(obj)) \
            do { abort(); } while(0)
    
    // does nothing in non-debug mode
    #define try_magic_n(obj, magic)
    
#else
    
    // magic variable to put into objects
    #define DEBUG_MAGIC unsigned int magic;
    
    // init obj magic
    #define init_magic_n(obj, _magic) \
        (obj)->magic = _magic;
    
    // check if obj exists and magic matches
    #define check_magic_n(obj, _magic) \
        ((obj) && (obj)->magic == _magic)
    
    // assert that obj exists and magic matches
    #define assert_magic_n(obj, _magic) \
        assert((obj) && (obj)->magic == _magic)
    
    // assert that magic matches if obj exists
    #define try_magic_n(obj, _magic) \
        if((obj)) \
            assert((obj)->magic == _magic)
    
#endif


// define magic value with 3 chars
#if __BYTE_ORDER == __LITTLE_ENDIAN
    #define define_magic(a, b, c)   ((c << 24)|(b << 16)|(a << 8)|'*')
#else
    #define define_magic(a, b, c)   (('*' << 24)|(a << 16)|(b << 8)|c)
#endif


#define init_magic(obj)   init_magic_n(obj, MAGIC)
#define check_magic(obj)  check_magic_n(obj, MAGIC)
#define assert_magic(obj) assert_magic_n(obj, MAGIC)
#define try_magic(obj)    try_magic_n(obj, MAGIC)

#endif
