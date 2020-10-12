/*
 * Copyright (c) 2018-2019 Martin Rödel a.k.a. Yomin Nimoy
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

#include <ytil/gen/str.h>
#include <ytil/def.h>
#include <ytil/magic.h>
#include <ytil/bits.h>
#include <ytil/ext/string.h>
#include <ytil/con/vec.h>
#include <stdint.h>
#include <stdio.h>


#define MAGIC define_magic("STR")


typedef enum str_flags
{
      NO_FLAGS          = 0
    , FLAG_TRANSIENT    = BV(0) // str is stack allocated
    , FLAG_UPDATE_LEN   = BV(1) // len is unset and has to be calculated
    , FLAG_UPDATE_CAP   = BV(2) // cap is unset and has to be calculated
    , FLAG_REDIRECT     = BV(3) // transient str points to heap allocated str
    , FLAG_CONST        = BV(4) // data is const i.e. not to be modified
    , FLAG_VOLATILE     = BV(5) // data is volatile i.e. is modified by other ref holders
    , FLAG_BINARY       = BV(6) // data my contain control chars and/or no null terminator
} str_flag_fs;

typedef enum str_type
{
      DATA_HEAP         // data is heap allocated
    , DATA_TRANSIENT    // data is stack allocated
    , DATA_STATIC       // data is static
    , DATA_TYPES
} str_type_id;

typedef struct str
{
    DEBUG_MAGIC
    unsigned char *data;
    uint32_t len, cap;
    uint16_t ref;
    uint8_t flags, type;
} str_st;

typedef enum str_cat_mode
{
      CAT_STR
    , CAT_STR_N
    , CAT_CSTR
    , CAT_CSTR_N
    , CAT_BIN
} str_cat_id;

typedef struct str_cat
{
    const unsigned char *data;
    size_t len;
} str_cat_st;


/// str error type definition
ERROR_DEFINE_LIST(STR,
      ERROR_INFO(E_STR_BINARY, "Operation not supported on binary data str.")
    , ERROR_INFO(E_STR_CONST, "Operation not supported on constant str.")
    , ERROR_INFO(E_STR_EMPTY, "Operation not supported on empty str.")
    , ERROR_INFO(E_STR_INVALID_CSTR, "Invalid str data.")
    , ERROR_INFO(E_STR_INVALID_CALLBACK, "Invalid callback.")
    , ERROR_INFO(E_STR_INVALID_DATA, "Invalid binary data.")
    , ERROR_INFO(E_STR_INVALID_FORMAT, "Invalid format.")
    , ERROR_INFO(E_STR_INVALID_LENGTH, "Invalid str length.")
    , ERROR_INFO(E_STR_OUT_OF_BOUNDS, "Out of bounds access.")
    , ERROR_INFO(E_STR_UNREFERENCED, "Operation not supported on unreferenced str.")
    , ERROR_INFO(E_STR_VOLATILE, "Operation not supported on volatile str.")
);

/// default error type for str module
#define ERROR_TYPE_DEFAULT ERROR_TYPE_STR



static inline bool _str_get_flags(str_const_ct str, str_flag_fs flags)
{
    return !!(str->flags & flags);
}

static inline void _str_set_flags(str_const_ct str, str_flag_fs flags)
{
    str_ct vstr = (str_ct)str;
    
    vstr->flags |= flags;
}

static inline void _str_clear_flags(str_const_ct str, str_flag_fs flags)
{
    str_ct vstr = (str_ct)str;
    
    vstr->flags &= ~flags;
}

static inline bool _str_is_transient(str_const_ct str)
{
    return _str_get_flags(str, FLAG_TRANSIENT);
}

static inline bool _str_is_const(str_const_ct str)
{
    return _str_get_flags(str, FLAG_CONST);
}

static inline bool _str_is_binary(str_const_ct str)
{
    return _str_get_flags(str, FLAG_BINARY);
}

static inline void _str_set_len(str_const_ct str, size_t len)
{
    str_ct vstr = (str_ct)str;
    
    _str_clear_flags(vstr, FLAG_UPDATE_LEN);
    vstr->len = len;
}

static inline size_t _str_get_len(str_const_ct str)
{
    if(_str_get_flags(str, FLAG_UPDATE_LEN))
    {
        assert(!_str_is_binary(str));
        _str_set_len(str, strlen((char*)str->data));
    }
    
    return str->len;
}

static inline void _str_set_cap(str_const_ct str, size_t cap)
{
    str_ct vstr = (str_ct)str;
    
    vstr->cap = cap;
    _str_clear_flags(vstr, FLAG_UPDATE_CAP);
}

static inline size_t _str_get_cap(str_const_ct str)
{
    if(_str_get_flags(str, FLAG_UPDATE_CAP))
        _str_set_cap(str, _str_get_len(str));
    
    return str->cap;
}

static bool _str_is_empty(str_const_ct str)
{
    return _str_is_binary(str) ? !str->len : !str->data[0];
}

static str_ct _str_get_writeable(str_ct str)
{
    unsigned char *data;
    
    return_error_if_pass(_str_is_const(str), E_STR_CONST, NULL);
    
    if(str->type != DATA_STATIC)
        return str;
    
    // heap data is only allowed on strings which are referenced
    return_error_if_fail(str->ref, E_STR_UNREFERENCED, NULL);
    
    if(!(data = calloc(1, _str_get_len(str)+1)))
        return error_wrap_errno(calloc), NULL;
    
    memcpy(data, str->data, _str_get_len(str));
    
    str->type = DATA_HEAP;
    str->cap = _str_get_len(str);
    str->data = data;
    
    return str;
}

str_ct _str_init(str_ct str, str_flag_fs flags, uint16_t ref, str_type_id type, unsigned char *data, ssize_t len, ssize_t cap)
{
    assert(str && type < DATA_TYPES && data && (!cap || len <= cap));
    assert(!(flags & ~(FLAG_TRANSIENT|FLAG_CONST|FLAG_VOLATILE|FLAG_BINARY)));
    
    init_magic(str);
    
    str->ref = ref;
    str->flags = flags;
    str->type = type;
    str->data = data;
    
    if(len < 0)
    {
        str->len = 0;
        str->flags |= FLAG_UPDATE_LEN;
    }
    else
    {
        if(type != DATA_STATIC && !_str_is_binary(str))
            data[len] = '\0';
        
        str->len = len;
    }
    
    if(cap < 0)
    {
        str->cap = 0;
        str->flags |= FLAG_UPDATE_CAP;
    }
    else
        str->cap = cap;
    
    return str;
}

str_ct _str_new(str_type_id type, unsigned char *data, ssize_t len, ssize_t cap, str_flag_fs flags)
{
    str_ct str;
    
    if(!(str = calloc(1, sizeof(str_st))))
        return error_wrap_errno(calloc), NULL;
    
    return _str_init(str, flags, 1, type, data, len, cap);
}

str_ct _str_set(str_ct str, str_type_id type, unsigned char *data, ssize_t len, ssize_t cap, str_flag_fs flags)
{
    // heap data is only allowed on strings which are referenced
    return_error_if_fail(str->ref || type != DATA_HEAP, E_STR_UNREFERENCED, NULL);
    return_error_if_pass(_str_is_const(str), E_STR_CONST, NULL);
    
    if(str->type == DATA_HEAP)
        free(str->data);
    
    if(_str_is_transient(str))
        flags |= FLAG_TRANSIENT;
    
    return _str_init(str, flags, str->ref, type, data, len, cap);
}



// assert magic and redirect if neccessary
#define assert_str(str) do { \
    assert_magic(str); \
    \
    if(_str_get_flags(str, FLAG_REDIRECT)) \
    { \
        str = (str_ct)str->data; \
        assert_magic(str); \
    } \
} while(0)



size_t str_len(str_const_ct str)
{
    assert_str(str);
    
    return _str_get_len(str);
}

size_t str_capacity(str_const_ct str)
{
    assert_str(str);
    
    return _str_get_cap(str);
}

size_t str_memsize(str_const_ct str)
{
    assert_str(str);
    
    return (_str_is_transient(str) ? 0 : sizeof(str_st))
         + (str->type == DATA_HEAP ? _str_get_cap(str) : 0);
}

size_t str_headsize(void)
{
    return sizeof(str_st);
}

bool str_is_empty(str_const_ct str)
{
    assert_str(str);
    
    return _str_is_empty(str);
}

bool str_is_transient(str_const_ct str)
{
    assert_str(str);
    
    return _str_is_transient(str);
}

bool str_is_redirected(str_const_ct str)
{
    assert_magic(str); // no redirect!
    
    return _str_get_flags(str, FLAG_REDIRECT);
}

bool str_is_const(str_const_ct str)
{
    assert_str(str);
    
    return _str_is_const(str);
}

bool str_is_volatile(str_const_ct str)
{
    assert_str(str);
    
    return _str_get_flags(str, FLAG_VOLATILE);
}

bool str_is_binary(str_const_ct str)
{
    assert_str(str);
    
    return _str_is_binary(str);
}

bool str_data_is_heap(str_const_ct str)
{
    assert_str(str);
    
    return str->type == DATA_HEAP;
}

bool str_data_is_static(str_const_ct str)
{
    assert_str(str);
    
    return str->type == DATA_STATIC;
}

bool str_data_is_transient(str_const_ct str)
{
    assert_str(str);
    
    return str->type == DATA_TRANSIENT;
}

str_ct str_mark_const(str_const_ct str)
{
    assert_str(str);
    return_error_if_pass(_str_get_flags(str, FLAG_VOLATILE), E_STR_VOLATILE, NULL);
    
    _str_set_flags(str, FLAG_CONST);
    
    return (str_ct)str;
}

str_ct str_mark_volatile(str_const_ct str)
{
    assert_str(str);
    return_error_if_pass(_str_is_const(str), E_STR_CONST, NULL);
    
    _str_set_flags(str, FLAG_VOLATILE);
    
    return (str_ct)str;
}

str_ct str_mark_binary(str_const_ct str)
{
    assert_str(str);
    
    _str_set_flags(str, FLAG_BINARY);
    
    return (str_ct)str;
}

const char *str_c(str_const_ct str)
{
    return (const char*)str_uc(str);
}

const unsigned char *str_uc(str_const_ct str)
{
    assert_str(str);
    return_error_if_pass(_str_is_binary(str), E_STR_BINARY, NULL);
    
    return str->data;
}

const void *str_bc(str_const_ct str)
{
    assert_str(str);
    
    return str->data;
}

const unsigned char *str_buc(str_const_ct str)
{
    assert_str(str);
    
    return str->data;
}

char *str_w(str_ct str)
{
    return (char*)str_uw(str);
}

unsigned char *str_uw(str_ct str)
{
    assert_str(str);
    return_error_if_pass(_str_is_binary(str), E_STR_BINARY, NULL);
    
    if(!_str_get_writeable(str))
        return error_pass(), NULL;
    
    return str->data;
}

void *str_bw(str_ct str)
{
    return str_buw(str);
}

unsigned char *str_buw(str_ct str)
{
    assert_str(str);
    
    if(!_str_get_writeable(str))
        return error_pass(), NULL;
    
    return str->data;
}

str_ct str_update(str_const_ct str)
{
    assert_str(str);
    
    // length is calculated next time str_len is used
    _str_set_flags(str, FLAG_UPDATE_LEN);
    
    return (str_ct)str;
}

str_ct str_set_len(str_const_ct str, size_t len)
{
    assert_str(str);
    return_error_if_fail(_str_get_flags(str, FLAG_UPDATE_CAP) || len <= str->cap, E_STR_INVALID_LENGTH, NULL);
    return_error_if_fail(str->type != DATA_STATIC || !str->data[len], E_STR_INVALID_LENGTH, NULL);
    return_error_if_pass(_str_is_const(str), E_STR_CONST, NULL);
    
    _str_set_len(str, len);
    
    if(str->type != DATA_STATIC && !_str_is_binary(str))
        str->data[len] = '\0';
    
    return (str_ct)str;
}

char str_first(str_const_ct str)
{
    assert_str(str);
    return_error_if_pass(_str_is_empty(str), E_STR_EMPTY, '\0');
    
    return str->data[0];
}

unsigned char str_first_u(str_const_ct str)
{
    assert_str(str);
    return_error_if_pass(_str_is_empty(str), E_STR_EMPTY, '\0');
    
    return str->data[0];
}

char str_last(str_const_ct str)
{
    assert_str(str);
    return_error_if_pass(_str_is_empty(str), E_STR_EMPTY, '\0');
    
    return str->data[_str_get_len(str)-1];
}

unsigned char str_last_u(str_const_ct str)
{
    assert_str(str);
    return_error_if_pass(_str_is_empty(str), E_STR_EMPTY, '\0');
    
    return str->data[_str_get_len(str)-1];
}

char str_at(str_const_ct str, size_t pos)
{
    assert_str(str);
    return_error_if_fail(pos < _str_get_len(str), E_STR_OUT_OF_BOUNDS, '\0');
    
    return str->data[pos];
}

unsigned char str_at_u(str_const_ct str, size_t pos)
{
    assert_str(str);
    return_error_if_fail(pos < _str_get_len(str), E_STR_OUT_OF_BOUNDS, '\0');
    
    return str->data[pos];
}

str_ct str_ref(str_const_ct str)
{
    str_ct vstr = (str_ct)str, nstr;
    size_t len, cap;
    str_flag_fs flags;
    str_type_id type;
    unsigned char *data;
    
    assert_str(str);
    
    if(!_str_is_transient(str))
        return ++vstr->ref, vstr;
    
    flags = str->flags & (FLAG_CONST|FLAG_BINARY);
    len = _str_get_len(str);
    
    if(str->type != DATA_TRANSIENT)
    {
        data = str->data;
        type = str->type;
        cap = str->cap;
    }
    else if((data = memdup(str->data, len+1)))
    {
        type = DATA_HEAP;
        cap = len;
    }
    else
        return error_wrap_errno(memdup), NULL;
    
    if(!(nstr = _str_new(type, data, len, cap, flags)))
    {
        if(str->type == DATA_TRANSIENT)
            free(data);
        
        return error_pass(), NULL;
    }
    
    // if transient string has ref redirect to heap string
    if(str->ref)
    {
        _str_set_flags(str, FLAG_REDIRECT);
        vstr->data = (void*)nstr;
        nstr->ref += str->ref;
    }
    else // no need to keep const flag if only owner
        _str_clear_flags(nstr, FLAG_CONST);
    
    return nstr;
}

str_ct str_ref_ensure(str_const_ct str, bool *referenced)
{
    assert_str(str);
    
    if(str->ref)
        return *referenced = false, (str_ct)str;
    
    if(!(str = str_ref(str)))
        return NULL;
    
    return *referenced = true, (str_ct)str;
}

str_ct str_unref(str_const_ct str)
{
    str_ct vstr = (str_ct)str;
    
    assert_str(vstr);
    return_error_if_fail(vstr->ref, E_STR_UNREFERENCED, NULL);
    
    if(--vstr->ref)
        return vstr;
    
    if(vstr->type == DATA_HEAP)
        free(vstr->data);
    
    if(!_str_is_transient(vstr))
        free(vstr);
    
    return NULL;
}

str_ct str_unref_if(str_const_ct str, bool cond)
{
    return cond ? str_unref(str) : (str_ct)str;
}

size_t str_get_refs(str_const_ct str)
{
    assert_str(str);
    
    return str->ref;
}

str_ct str_clear(str_ct str)
{
    assert_str(str);
    return_error_if_pass(_str_is_const(str), E_STR_CONST, NULL);
    
    if(str->type == DATA_STATIC)
        str->data = (unsigned char*)"";
    else if(!_str_is_binary(str))
        str->data[0] = '\0';
    
    _str_set_len(str, 0);
    
    return str;
}

str_ct str_truncate(str_const_ct str)
{
    str_ct vstr = (str_ct)str;
    unsigned char *data;
    
    assert_str(str);
    return_error_if_pass(_str_is_const(str), E_STR_CONST, NULL);
    
    switch(str->type)
    {
    case DATA_STATIC:
        break;
    case DATA_TRANSIENT:
        _str_set_cap(str, _str_get_len(str));
        break;
    case DATA_HEAP:
        if(!(data = realloc(str->data, _str_get_len(str)+1)))
            break;
        vstr->data = data;
        _str_set_cap(str, _str_get_len(str));
        break;
    default:
        abort();
    }
    
    return vstr;
}

str_ct str_resize(str_ct str, size_t len)
{
    unsigned char *data;
    
    assert_str(str);
    return_error_if_pass(_str_is_const(str), E_STR_CONST, NULL);
    
    switch(str->type)
    {
    case DATA_STATIC:
        if(!str->ref)
            return error_set(E_STR_UNREFERENCED), NULL;
        else if(!(data = calloc(1, len+1)))
            return error_wrap_errno(calloc), NULL;
        else
        {
            memcpy(data, str->data, MIN(len, _str_get_len(str)));
            str->type = DATA_HEAP;
            str->data = data;
            str->cap = len;
        }
        break;
    case DATA_TRANSIENT:
        if(len <= _str_get_cap(str))
            break;
        else if(!str->ref)
            return error_set(E_STR_UNREFERENCED), NULL;
        else if(!(data = calloc(1, len+1)))
            return error_wrap_errno(calloc), NULL;
        else
        {
            memcpy(data, str->data, MIN(len, _str_get_len(str)));
            str->type = DATA_HEAP;
            str->data = data;
            str->cap = len;
        }
        break;
    case DATA_HEAP:
        assert(str->ref);
        if(len <= _str_get_cap(str))
            break;
        else if(!(data = realloc(str->data, len+1)))
            return error_wrap_errno(realloc), NULL;
        else
        {
            str->cap = len;
            str->data = data;
        }
        break;
    default:
        abort();
    }
    
    _str_set_len(str, len);
    
    if(!_str_is_binary(str))
        str->data[len] = '\0';
    
    return str;
}

str_ct str_resize_set(str_ct str, size_t new_len, char c)
{
    size_t len;
    
    assert_str(str);
    
    len = _str_get_len(str);
    
    if(!str_resize(str, new_len))
        return error_pass(), NULL;
    
    if(new_len > len)
        memset(&str->data[len], c, new_len-len);
    
    return str;
}

str_ct str_grow(str_ct str, size_t len)
{
    assert_str(str);
    
    return error_pass_ptr(str_resize(str, _str_get_len(str)+len));
}

str_ct str_grow_set(str_ct str, size_t len, char c)
{
    size_t cur_len;
    
    assert_str(str);
    
    cur_len = _str_get_len(str);
    
    if(!str_resize(str, cur_len+len))
        return error_pass(), NULL;
    
    memset(&str->data[cur_len], c, len);
    
    return str;
}

str_ct str_shrink(str_ct str, size_t len)
{
    size_t cur_len;
    
    assert_str(str);
    
    cur_len = _str_get_len(str);
    len = cur_len <= len ? 0 : cur_len - len;
    
    return error_pass_ptr(str_resize(str, len));
}

str_ct str_prepare(size_t len)
{
    return error_pass_ptr(str_prepare_c(len, len));
}

str_ct str_prepare_c(size_t len, size_t cap)
{
    str_ct str;
    unsigned char *data;
    
    return_error_if_fail(len <= cap, E_STR_INVALID_LENGTH, NULL);
    
    if(!(data = calloc(1, cap+1)))
        return error_wrap_errno(calloc), NULL;
    
    if(!(str = _str_new(DATA_HEAP, data, len, cap, NO_FLAGS)))
        return error_pass(), free(data), NULL;
    
    return str;
}

str_ct str_prepare_b(size_t len)
{
    return error_pass_ptr(str_prepare_bc(len, len));
}

str_ct str_prepare_bc(size_t len, size_t cap)
{
    str_ct str;
    unsigned char *data;
    
    return_error_if_fail(len <= cap, E_STR_INVALID_LENGTH, NULL);
    
    if(!(data = calloc(1, cap+1)))
        return error_wrap_errno(calloc), NULL;
    
    if(!(str = _str_new(DATA_HEAP, data, len, cap, FLAG_BINARY)))
        return error_pass(), free(data), NULL;
    
    return str;
}

str_ct str_prepare_set(size_t len, char c)
{
    return error_pass_ptr(str_prepare_set_c(len, len, c));
}

str_ct str_prepare_set_c(size_t len, size_t cap, char c)
{
    str_ct str;
    
    if(!(str = str_prepare_c(len, cap)))
        return error_pass(), NULL;
    
    memset(str->data, c, len);
    
    return str;
}

str_ct str_prepare_set_b(size_t len, char c)
{
    return error_pass_ptr(str_prepare_set_bc(len, len, c));
}

str_ct str_prepare_set_bc(size_t len, size_t cap, char c)
{
    str_ct str;
    
    if(!(str = str_prepare_bc(len, cap)))
        return error_pass(), NULL;
    
    memset(str->data, c, len);
    
    return str;
}

str_ct str_new_h(char *cstr)
{
    return_error_if_fail(cstr, E_STR_INVALID_CSTR, NULL);
    
    return error_pass_ptr(_str_new(DATA_HEAP, (unsigned char*)cstr, -1, -1, NO_FLAGS));
}

str_ct str_new_hn(char *cstr, size_t len)
{
    return_error_if_fail(cstr, E_STR_INVALID_CSTR, NULL);
    
    return error_pass_ptr(_str_new(DATA_HEAP, (unsigned char*)cstr, len, len, NO_FLAGS));
}

str_ct str_new_hnc(char *cstr, size_t len, size_t cap)
{
    return_error_if_fail(cstr, E_STR_INVALID_CSTR, NULL);
    return_error_if_fail(len <= cap, E_STR_INVALID_LENGTH, NULL);
    
    return error_pass_ptr(_str_new(DATA_HEAP, (unsigned char*)cstr, len, cap, NO_FLAGS));
}

str_ct str_new_s(const char *cstr)
{
    return_error_if_fail(cstr, E_STR_INVALID_CSTR, NULL);
    
    return error_pass_ptr(_str_new(DATA_STATIC, (unsigned char*)cstr, -1, 0, NO_FLAGS));
}

str_ct str_new_sn(const char *cstr, size_t len)
{
    return_error_if_fail(cstr && !cstr[len], E_STR_INVALID_CSTR, NULL);
    
    return error_pass_ptr(_str_new(DATA_STATIC, (unsigned char*)cstr, len, 0, NO_FLAGS));
}

str_ct str_new_bh(void *data, size_t len)
{
    return_error_if_fail(data, E_STR_INVALID_DATA, NULL);
    
    return error_pass_ptr(_str_new(DATA_HEAP, data, len, len, FLAG_BINARY));
}

str_ct str_new_bhc(void *data, size_t len, size_t cap)
{
    return_error_if_fail(data, E_STR_INVALID_DATA, NULL);
    return_error_if_fail(len <= cap, E_STR_INVALID_LENGTH, NULL);
    
    return error_pass_ptr(_str_new(DATA_HEAP, data, len, cap, FLAG_BINARY));
}

str_ct str_new_bs(const void *data, size_t len)
{
    return_error_if_fail(data, E_STR_INVALID_DATA, NULL);
    
    return error_pass_ptr(_str_new(DATA_STATIC, (void*)data, len, 0, FLAG_BINARY));
}

str_ct tstr_init_h(str_ct str, char *hstr)
{
    return_error_if_fail(hstr, E_STR_INVALID_CSTR, NULL);
    
    return _str_init(str, FLAG_TRANSIENT, 1, DATA_HEAP, (unsigned char*)hstr, -1, -1);
}

str_ct tstr_init_hn(str_ct str, char *hstr, size_t len)
{
    return_error_if_fail(hstr, E_STR_INVALID_CSTR, NULL);
    
    return _str_init(str, FLAG_TRANSIENT, 1, DATA_HEAP, (unsigned char*)hstr, len, len);
}

str_ct tstr_init_hnc(str_ct str, char *hstr, size_t len, size_t cap)
{
    return_error_if_fail(hstr, E_STR_INVALID_CSTR, NULL);
    return_error_if_fail(len <= cap, E_STR_INVALID_LENGTH, NULL);
    
    return _str_init(str, FLAG_TRANSIENT, 1, DATA_HEAP, (unsigned char*)hstr, len, cap);
}

str_ct tstr_init_s(str_ct str, const char *cstr)
{
    return_error_if_fail(cstr, E_STR_INVALID_CSTR, NULL);
    
    return _str_init(str, FLAG_TRANSIENT, 0, DATA_STATIC, (unsigned char*)cstr, -1, 0);
}

str_ct tstr_init_sn(str_ct str, const char *cstr, size_t len)
{
    return_error_if_fail(cstr && !cstr[len], E_STR_INVALID_CSTR, NULL);
    
    return _str_init(str, FLAG_TRANSIENT, 0, DATA_STATIC, (unsigned char*)cstr, len, 0);
}

str_ct tstr_init_tn(str_ct str, char *cstr, size_t len)
{
    return_error_if_fail(cstr, E_STR_INVALID_CSTR, NULL);
    
    return _str_init(str, FLAG_TRANSIENT, 0, DATA_TRANSIENT, (unsigned char*)cstr, len, len);
}

str_ct tstr_init_tnc(str_ct str, char *cstr, size_t len, size_t cap)
{
    return_error_if_fail(cstr, E_STR_INVALID_CSTR, NULL);
    return_error_if_fail(len <= cap, E_STR_INVALID_LENGTH, NULL);
    
    return _str_init(str, FLAG_TRANSIENT, 0, DATA_TRANSIENT, (unsigned char*)cstr, len, cap);
}

str_ct tstr_init_bh(str_ct str, void *bin, size_t len)
{
    return_error_if_fail(bin, E_STR_INVALID_DATA, NULL);
    
    return _str_init(str, FLAG_TRANSIENT|FLAG_BINARY, 1, DATA_HEAP, bin, len, len);
}

str_ct tstr_init_bhc(str_ct str, void *bin, size_t len, size_t cap)
{
    return_error_if_fail(bin, E_STR_INVALID_DATA, NULL);
    return_error_if_fail(len <= cap, E_STR_INVALID_LENGTH, NULL);
    
    return _str_init(str, FLAG_TRANSIENT|FLAG_BINARY, 1, DATA_HEAP, bin, len, cap);
}

str_ct tstr_init_bs(str_ct str, const void *bin, size_t len)
{
    return_error_if_fail(bin, E_STR_INVALID_DATA, NULL);
    
    return _str_init(str, FLAG_TRANSIENT|FLAG_BINARY, 0, DATA_STATIC, (void*)bin, len, 0);
}

str_ct tstr_init_bt(str_ct str, void *bin, size_t len)
{
    return_error_if_fail(bin, E_STR_INVALID_DATA, NULL);
    
    return _str_init(str, FLAG_TRANSIENT|FLAG_BINARY, 0, DATA_TRANSIENT, (void*)bin, len, len);
}

str_ct tstr_init_btc(str_ct str, void *bin, size_t len, size_t cap)
{
    return_error_if_fail(bin, E_STR_INVALID_DATA, NULL);
    return_error_if_fail(len <= cap, E_STR_INVALID_LENGTH, NULL);
    
    return _str_init(str, FLAG_TRANSIENT|FLAG_BINARY, 0, DATA_TRANSIENT, (void*)bin, len, cap);
}

str_ct str_dup(str_const_ct str)
{
    return error_pass_ptr(str_dup_n(str, str_len(str)));
}

str_ct str_dup_n(str_const_ct str, size_t len)
{
    assert_str(str);
    
    len = MIN(len, _str_get_len(str));
    
    if(str->type == DATA_STATIC)
    {
        if(_str_is_binary(str)) // no terminator for binary required
            return error_pass_ptr(str_new_bs(str->data, len));
        else if(!len) // zero length const data can be statically provided
            return error_pass_ptr(str_new_l(""));
        else if(len == _str_get_len(str)) // just reference it
            return error_pass_ptr(str_new_sn((char*)str->data, len));
        else // heap dup to ensure terminator
            return error_pass_ptr(str_dup_cn((char*)str->data, len));
    }
    else
    {
        if(_str_is_binary(str))
            return error_pass_ptr(str_dup_b(str->data, len));
        else
            return error_pass_ptr(str_dup_cn((char*)str->data, len));
    }
}

str_ct str_dup_c(const char *cstr)
{
    return_error_if_fail(cstr, E_STR_INVALID_CSTR, NULL);
    
    return error_pass_ptr(str_dup_cn(cstr, strlen(cstr)));
}

str_ct str_dup_cn(const char *cstr, size_t len)
{
    str_ct str;
    
    return_error_if_fail(cstr, E_STR_INVALID_CSTR, NULL);
    
    if(!(str = str_prepare_c(len, len)))
        return error_pass(), NULL;
    
    memcpy(str->data, cstr, len);
    
    return str;
}

str_ct str_dup_b(const void *data, size_t len)
{
    str_ct str;
    
    return_error_if_fail(data, E_STR_INVALID_DATA, NULL);
    
    if(!(str = str_prepare_c(len, len)))
        return error_pass(), NULL;
    
    memcpy(str->data, data, len);
    _str_set_flags(str, FLAG_BINARY);
    
    return str;
}

str_ct str_dup_f(const char *fmt, ...)
{
    str_ct str;
    va_list ap;
    
    va_start(ap, fmt);
    str = error_pass_ptr(str_dup_vf(fmt, ap));
    va_end(ap);
    
    return str;
}

str_ct str_dup_vf(const char *fmt, va_list ap)
{
    size_t len;
    str_ct str;
    va_list ap2;
    
    return_error_if_fail(fmt, E_STR_INVALID_FORMAT, NULL);
    
    va_copy(ap2, ap);
    len = vsnprintf(NULL, 0, fmt, ap2);
    va_end(ap2);
    
    if(!(str = str_prepare_c(len, len)))
        return error_pass(), NULL;
    
    vsnprintf((char*)str->data, len+1, fmt, ap);
    
    return str;
}

str_ct tstr_init_dup_n(str_ct dst, void *data, str_ct src, size_t len)
{
    assert_str(src);
    
    len = MIN(len, _str_get_len(src));
    
    if(_str_is_binary(src))
        return error_pass_ptr(tstr_init_dup_b(dst, data, src->data, len));
    else
        return error_pass_ptr(tstr_init_dup_cn(dst, data, (char*)src->data, len));
}

str_ct tstr_init_dup_cn(str_ct dst, void *vdata, const char *cstr, size_t len)
{
    unsigned char *data = vdata;
    
    return_error_if_fail(cstr, E_STR_INVALID_CSTR, NULL);
    
    memcpy(data, cstr, len);
    data[len] = '\0';
    
    return _str_init(dst, FLAG_TRANSIENT, 0, DATA_TRANSIENT, data, len, len);
}

str_ct tstr_init_dup_b(str_ct dst, void *vdata, const void *bin, size_t len)
{
    unsigned char *data = vdata;
    
    return_error_if_fail(bin, E_STR_INVALID_DATA, NULL);
    
    memcpy(data, bin, len);
    data[len] = '\0';
    
    return _str_init(dst, FLAG_TRANSIENT|FLAG_BINARY, 0, DATA_TRANSIENT, data, len, len);
}

str_ct str_set_h(str_ct str, char *cstr)
{
    assert_str(str);
    return_error_if_fail(cstr, E_STR_INVALID_CSTR, NULL);
    
    return _str_set(str, DATA_HEAP, (unsigned char*)cstr, -1, -1, NO_FLAGS);
}

str_ct str_set_hn(str_ct str, char *cstr, size_t len)
{
    assert_str(str);
    return_error_if_fail(cstr, E_STR_INVALID_CSTR, NULL);
    
    return _str_set(str, DATA_HEAP, (unsigned char*)cstr, len, len, NO_FLAGS);
}

str_ct str_set_hnc(str_ct str, char *cstr, size_t len, size_t cap)
{
    assert_str(str);
    return_error_if_fail(cstr, E_STR_INVALID_CSTR, NULL);
    return_error_if_fail(len <= cap, E_STR_INVALID_LENGTH, NULL);
    
    return _str_set(str, DATA_HEAP, (unsigned char*)cstr, len, cap, NO_FLAGS);
}

str_ct str_set_s(str_ct str, const char *cstr)
{
    assert_str(str);
    return_error_if_fail(cstr, E_STR_INVALID_CSTR, NULL);
    
    return _str_set(str, DATA_STATIC, (unsigned char*)cstr, -1, 0, NO_FLAGS);
}

str_ct str_set_sn(str_ct str, const char *cstr, size_t len)
{
    assert_str(str);
    return_error_if_fail(cstr && !cstr[len], E_STR_INVALID_CSTR, NULL);
    
    return _str_set(str, DATA_STATIC, (unsigned char*)cstr, len, 0, NO_FLAGS);
}

str_ct str_set_bh(str_ct str, void *data, size_t len)
{
    assert_str(str);
    return_error_if_fail(data, E_STR_INVALID_DATA, NULL);
    
    return _str_set(str, DATA_HEAP, data, len, len, FLAG_BINARY);
}

str_ct str_set_bhc(str_ct str, void *data, size_t len, size_t cap)
{
    assert_str(str);
    return_error_if_fail(data, E_STR_INVALID_DATA, NULL);
    return_error_if_fail(len <= cap, E_STR_INVALID_LENGTH, NULL);
    
    return _str_set(str, DATA_HEAP, data, len, cap, FLAG_BINARY);
}

str_ct str_set_bs(str_ct str, const void *data, size_t len)
{
    assert_str(str);
    return_error_if_fail(data, E_STR_INVALID_DATA, NULL);
    
    return _str_set(str, DATA_STATIC, (void*)data, len, 0, FLAG_BINARY);
}

str_ct str_copy(str_ct dst, size_t pos, str_const_ct src)
{
    assert_str(src);
    
    if(_str_is_binary(src))
        return error_pass_ptr(str_copy_b(dst, pos, src->data, _str_get_len(src)));
    else
        return error_pass_ptr(str_copy_cn(dst, pos, (void*)src->data, _str_get_len(src)));
}

str_ct str_copy_n(str_ct dst, size_t pos, str_const_ct src, size_t len)
{
    assert_str(src);
    
    len = MIN(len, _str_get_len(src));
    
    if(_str_is_binary(src))
        return error_pass_ptr(str_copy_b(dst, pos, src->data, len));
    else
        return error_pass_ptr(str_copy_cn(dst, pos, (void*)src->data, len));
}

str_ct str_copy_c(str_ct dst, size_t pos, const char *src)
{
    return_error_if_fail(src, E_STR_INVALID_CSTR, NULL);
    
    return error_pass_ptr(str_copy_cn(dst, pos, src, strlen(src)));
}

str_ct str_copy_cn(str_ct dst, size_t pos, const char *src, size_t len)
{
    assert_str(dst);
    return_error_if_fail(src, E_STR_INVALID_CSTR, NULL);
    return_error_if_fail(pos <= _str_get_len(dst), E_STR_OUT_OF_BOUNDS, NULL);
    
    if(!(dst = str_resize(dst, pos + len)))
        return error_pass(), NULL;
    
    memcpy(&dst->data[pos], src, len);
    
    return dst;
}

str_ct str_copy_b(str_ct dst, size_t pos, const void *src, size_t len)
{
    return_error_if_fail(src, E_STR_INVALID_DATA, NULL);
    
    if(!(dst = str_copy_cn(dst, pos, src, len)))
        return error_pass(), NULL;
    
    _str_set_flags(dst, FLAG_BINARY);
    
    return dst;
}

str_ct str_copy_f(str_ct dst, size_t pos, const char *fmt, ...)
{
    va_list ap;
    
    va_start(ap, fmt);
    dst = error_pass_ptr(str_copy_vf(dst, pos, fmt, ap));
    va_end(ap);
    
    return dst;
}

str_ct str_copy_vf(str_ct dst, size_t pos, const char *fmt, va_list ap)
{
    va_list ap2;
    size_t len;
    
    assert_str(dst);
    return_error_if_fail(fmt, E_STR_INVALID_FORMAT, NULL);
    return_error_if_fail(pos <= _str_get_len(dst), E_STR_OUT_OF_BOUNDS, NULL);
    
    va_copy(ap2, ap);
    len = vsnprintf(NULL, 0, fmt, ap2);
    va_end(ap2);
    
    if(!(dst = str_resize(dst, pos + len)))
        return error_pass(), NULL;
    
    vsnprintf((char*)&dst->data[pos], len+1, fmt, ap);
    
    return dst;
}

ssize_t str_overwrite(str_ct dst, size_t pos, str_const_ct src)
{
    assert_str(src);
    
    if(_str_is_binary(src))
        return error_pass_int(str_overwrite_b(dst, pos, src->data, _str_get_len(src)));
    else
        return error_pass_int(str_overwrite_cn(dst, pos, (char*)src->data, _str_get_len(src)));
}

ssize_t str_overwrite_n(str_ct dst, size_t pos, str_const_ct src, size_t len)
{
    assert_str(src);
    
    len = MIN(len, _str_get_len(src));
    
    if(_str_is_binary(src))
        return error_pass_int(str_overwrite_b(dst, pos, src->data, len));
    else
        return error_pass_int(str_overwrite_cn(dst, pos, (char*)src->data, len));
}

ssize_t str_overwrite_c(str_ct dst, size_t pos, const char *src)
{
    return_error_if_fail(src, E_STR_INVALID_CSTR, -1);
    
    return error_pass_int(str_overwrite_cn(dst, pos, src, strlen(src)));
}

ssize_t str_overwrite_cn(str_ct dst, size_t pos, const char *src, size_t len)
{
    assert_str(dst);
    return_error_if_fail(src, E_STR_INVALID_CSTR, -1);
    return_error_if_fail(pos < _str_get_len(dst), E_STR_OUT_OF_BOUNDS, -1);
    
    if(!(dst = _str_get_writeable(dst)))
        return error_pass(), -1;
    
    len = MIN(len, _str_get_len(dst) - pos);
    memcpy(&dst->data[pos], src, len);
    
    return len;
}

ssize_t str_overwrite_b(str_ct dst, size_t pos, const void *src, size_t len)
{
    ssize_t count;
    
    return_error_if_fail(src, E_STR_INVALID_DATA, -1);
    
    if((count = str_overwrite_cn(dst, pos, src, len)) < 0)
        return error_pass(), -1;
    
    _str_set_flags(dst, FLAG_BINARY);
    
    return count;
}

ssize_t str_overwrite_f(str_ct dst, size_t pos, const char *fmt, ...)
{
    va_list ap;
    ssize_t len;
    
    va_start(ap, fmt);
    len = error_pass_int(str_overwrite_vfn(dst, pos, str_len(dst), fmt, ap));
    va_end(ap);
    
    return len;
}

ssize_t str_overwrite_fn(str_ct dst, size_t pos, size_t len, const char *fmt, ...)
{
    va_list ap;
    
    va_start(ap, fmt);
    len = error_pass_int(str_overwrite_vfn(dst, pos, len, fmt, ap));
    va_end(ap);
    
    return len;
}

ssize_t str_overwrite_vf(str_ct dst, size_t pos, const char *fmt, va_list ap)
{
    return error_pass_int(str_overwrite_vfn(dst, pos, str_len(dst), fmt, ap));
}

ssize_t str_overwrite_vfn(str_ct dst, size_t pos, size_t len, const char *fmt, va_list ap)
{
    va_list ap2;
    unsigned char tmp;
    
    assert_str(dst);
    return_error_if_fail(fmt, E_STR_INVALID_FORMAT, -1);
    return_error_if_fail(pos < _str_get_len(dst), E_STR_OUT_OF_BOUNDS, -1);
    
    if(!(dst = _str_get_writeable(dst)))
        return error_pass(), -1;
    
    va_copy(ap2, ap);
    len = MIN(len, _str_get_len(dst) - pos);
    len = MIN(len, (size_t)vsnprintf(NULL, 0, fmt, ap2));
    va_end(ap2);
    
    tmp = dst->data[pos + len];
    vsnprintf((char*)&dst->data[pos], len+1, fmt, ap);
    dst->data[pos + len] = tmp;
    
    return len;
}

static str_ct _str_insert(str_ct str, size_t pos, const char *sub, size_t sub_len)
{
    size_t len;
    
    len = _str_get_len(str);
    
    if(!(str = str_resize(str, len + sub_len)))
        return error_pass(), NULL;
    
    if(pos < len)
        memmove(&str->data[pos + sub_len], &str->data[pos], len - pos);
    
    if(sub)
        memcpy(&str->data[pos], sub, sub_len);
    
    return str;
}

static str_ct _str_insert_f(str_ct str, size_t pos, const char *fmt, va_list ap)
{
    size_t len, fmt_len;
    va_list ap2;
    char tmp;
    
    va_copy(ap2, ap);
    fmt_len = vsnprintf(NULL, 0, fmt, ap2);
    va_end(ap2);
    
    len = _str_get_len(str);
    
    if(!(str = str_resize(str, len + fmt_len)))
        return error_pass(), NULL;
    
    if(pos < len)
        memmove(&str->data[pos + fmt_len], &str->data[pos], len - pos);
    
    tmp = str->data[pos + fmt_len];
    vsnprintf((char*)&str->data[pos], fmt_len+1, fmt, ap);
    str->data[pos + fmt_len] = tmp;
    
    return str;
}

str_ct str_prepend(str_ct str, str_const_ct prefix)
{
    assert_str(prefix);
    
    if(_str_is_binary(prefix))
        return error_pass_ptr(str_prepend_b(str, prefix->data, _str_get_len(prefix)));
    else
        return error_pass_ptr(str_prepend_cn(str, (char*)prefix->data, _str_get_len(prefix)));
}

str_ct str_prepend_n(str_ct str, str_const_ct prefix, size_t len)
{
    assert_str(prefix);
    
    len = MIN(len, _str_get_len(prefix));
    
    if(_str_is_binary(prefix))
        return error_pass_ptr(str_prepend_b(str, prefix->data, len));
    else
        return error_pass_ptr(str_prepend_cn(str, (char*)prefix->data, len));
}

str_ct str_prepend_c(str_ct str, const char *prefix)
{
    assert_str(str);
    return_error_if_fail(prefix, E_STR_INVALID_CSTR, NULL);
    
    return error_pass_ptr(_str_insert(str, 0, prefix, strlen(prefix)));
}

str_ct str_prepend_cn(str_ct str, const char *prefix, size_t len)
{
    assert_str(str);
    return_error_if_fail(prefix, E_STR_INVALID_CSTR, NULL);
    
    return error_pass_ptr(_str_insert(str, 0, prefix, len));
}

str_ct str_prepend_b(str_ct str, const void *prefix, size_t len)
{
    assert_str(str);
    return_error_if_fail(prefix, E_STR_INVALID_DATA, NULL);
    
    if(!(str = _str_insert(str, 0, prefix, len)))
        return error_pass(), NULL;
    
    _str_set_flags(str, FLAG_BINARY);
    
    return str;
}

str_ct str_prepend_set(str_ct str, size_t len, char c)
{
    assert_str(str);
    
    if(!(str = _str_insert(str, 0, NULL, len)))
        return error_pass(), NULL;
    
    memset(str->data, c, len);
    
    return str;
}

str_ct str_prepend_f(str_ct str, const char *fmt, ...)
{
    va_list ap;
    
    assert_str(str);
    return_error_if_fail(fmt, E_STR_INVALID_FORMAT, NULL);
    
    va_start(ap, fmt);
    str = error_pass_ptr(_str_insert_f(str, 0, fmt, ap));
    va_end(ap);
    
    return str;
}

str_ct str_prepend_vf(str_ct str, const char *fmt, va_list ap)
{
    assert_str(str);
    return_error_if_fail(fmt, E_STR_INVALID_FORMAT, NULL);
    
    return error_pass_ptr(_str_insert_f(str, 0, fmt, ap));
}

str_ct str_append(str_ct str, str_const_ct suffix)
{
    assert_str(suffix);
    
    if(_str_is_binary(suffix))
        return error_pass_ptr(str_append_b(str, suffix->data, _str_get_len(suffix)));
    else
        return error_pass_ptr(str_append_cn(str, (char*)suffix->data, _str_get_len(suffix)));
}

str_ct str_append_n(str_ct str, str_const_ct suffix, size_t len)
{
    assert_str(suffix);
    
    len = MIN(len, _str_get_len(suffix));
    
    if(_str_is_binary(suffix))
        return error_pass_ptr(str_append_b(str, suffix->data, len));
    else
        return error_pass_ptr(str_append_cn(str, (char*)suffix->data, len));
}

str_ct str_append_c(str_ct str, const char *suffix)
{
    assert_str(str);
    return_error_if_fail(suffix, E_STR_INVALID_CSTR, NULL);
    
    return _str_insert(str, _str_get_len(str), suffix, strlen(suffix));
}

str_ct str_append_cn(str_ct str, const char *suffix, size_t len)
{
    assert_str(str);
    return_error_if_fail(suffix, E_STR_INVALID_CSTR, NULL);
    
    return _str_insert(str, _str_get_len(str), suffix, len);
}

str_ct str_append_b(str_ct str, const void *suffix, size_t len)
{
    assert_str(str);
    return_error_if_fail(suffix, E_STR_INVALID_DATA, NULL);
    
    if(!(str = _str_insert(str, _str_get_len(str), suffix, len)))
        return error_pass(), NULL;
    
    _str_set_flags(str, FLAG_BINARY);
    
    return str;
}

str_ct str_append_set(str_ct str, size_t suffix_len, char c)
{
    size_t len;
    
    assert_str(str);
    
    len = _str_get_len(str);
    
    if(!(str = _str_insert(str, len, NULL, suffix_len)))
        return error_pass(), NULL;
    
    memset(&str->data[len], c, suffix_len);
    
    return str;
}

str_ct str_append_f(str_ct str, const char *fmt, ...)
{
    va_list ap;
    
    assert_str(str);
    return_error_if_fail(fmt, E_STR_INVALID_FORMAT, NULL);
    
    va_start(ap, fmt);
    str = error_pass_ptr(_str_insert_f(str, _str_get_len(str), fmt, ap));
    va_end(ap);
    
    return str;
}

str_ct str_append_vf(str_ct str, const char *fmt, va_list ap)
{
    assert_str(str);
    return_error_if_fail(fmt, E_STR_INVALID_FORMAT, NULL);
    
    return error_pass_ptr(_str_insert_f(str, _str_get_len(str), fmt, ap));
}

str_ct str_insert(str_ct str, size_t pos, str_const_ct sub)
{
    assert_str(sub);
    
    if(_str_is_binary(sub))
        return error_pass_ptr(str_insert_b(str, pos, sub->data, _str_get_len(sub)));
    else
        return error_pass_ptr(str_insert_cn(str, pos, (char*)sub->data, _str_get_len(sub)));
}

str_ct str_insert_n(str_ct str, size_t pos, str_const_ct sub, size_t len)
{
    assert_str(sub);
    
    len = MIN(len, _str_get_len(sub));
    
    if(_str_is_binary(sub))
        return error_pass_ptr(str_insert_b(str, pos, sub->data, len));
    else
        return error_pass_ptr(str_insert_cn(str, pos, (char*)sub->data, len));
}

str_ct str_insert_c(str_ct str, size_t pos, const char *sub)
{
    assert_str(str);
    return_error_if_fail(sub, E_STR_INVALID_CSTR, NULL);
    return_error_if_fail(pos <= _str_get_len(str), E_STR_OUT_OF_BOUNDS, NULL);
    
    return _str_insert(str, pos, sub, strlen(sub));
}

str_ct str_insert_cn(str_ct str, size_t pos, const char *sub, size_t len)
{
    assert_str(str);
    return_error_if_fail(sub, E_STR_INVALID_CSTR, NULL);
    return_error_if_fail(pos <= _str_get_len(str), E_STR_OUT_OF_BOUNDS, NULL);
    
    return _str_insert(str, pos, sub, len);
}

str_ct str_insert_b(str_ct str, size_t pos, const void *sub, size_t len)
{
    assert_str(str);
    return_error_if_fail(sub, E_STR_INVALID_DATA, NULL);
    return_error_if_fail(pos <= _str_get_len(str), E_STR_OUT_OF_BOUNDS, NULL);
    
    if(!(str = _str_insert(str, pos, sub, len)))
        return error_pass(), NULL;
    
    _str_set_flags(str, FLAG_BINARY);
    
    return str;
}

str_ct str_insert_set(str_ct str, size_t pos, size_t len, char c)
{
    assert_str(str);
    return_error_if_fail(pos <= _str_get_len(str), E_STR_OUT_OF_BOUNDS, NULL);
    
    if(!(str = _str_insert(str, pos, NULL, len)))
        return error_pass(), NULL;
    
    memset(&str->data[pos], c, len);
    
    return str;
}

str_ct str_insert_f(str_ct str, size_t pos, const char *fmt, ...)
{
    va_list ap;
    
    assert_str(str);
    return_error_if_fail(fmt, E_STR_INVALID_FORMAT, NULL);
    return_error_if_fail(pos <= _str_get_len(str), E_STR_OUT_OF_BOUNDS, NULL);
    
    va_start(ap, fmt);
    str = error_pass_ptr(_str_insert_f(str, pos, fmt, ap));
    va_end(ap);
    
    return str;
}

str_ct str_insert_vf(str_ct str, size_t pos, const char *fmt, va_list ap)
{
    assert_str(str);
    return_error_if_fail(fmt, E_STR_INVALID_FORMAT, NULL);
    return_error_if_fail(pos <= _str_get_len(str), E_STR_OUT_OF_BOUNDS, NULL);
    
    return error_pass_ptr(_str_insert_f(str, pos, fmt, ap));
}

static str_ct _str_cat(size_t n, va_list ap, str_cat_id mode)
{
    str_const_ct str;
    const char *cstr;
    str_ct cat;
    size_t i, len, cat_len;
    str_cat_st *info;
    bool binary = mode == CAT_BIN;
    
    info = alloca(n*sizeof(str_cat_st));
    
    for(cat_len=0, i=0; i < n; cat_len += info[i].len, i++)
        switch(mode)
        {
        case CAT_STR:
            str = va_arg(ap, str_const_ct);
            assert_str(str);
            info[i].data = str->data;
            info[i].len = _str_get_len(str);
            binary = binary || _str_is_binary(str);
            break;
        case CAT_STR_N:
            str = va_arg(ap, str_const_ct);
            len = va_arg(ap, size_t);
            assert_str(str);
            info[i].data = str->data;
            info[i].len = MIN(len, _str_get_len(str));
            binary = binary || _str_is_binary(str);
            break;
        case CAT_CSTR:
            cstr = va_arg(ap, const char*);
            return_error_if_fail(cstr, E_STR_INVALID_CSTR, NULL);
            info[i].data = (unsigned char*)cstr;
            info[i].len = strlen(cstr);
            break;
        case CAT_CSTR_N:
            cstr = va_arg(ap, const char*);
            len = va_arg(ap, size_t);
            return_error_if_fail(cstr, E_STR_INVALID_CSTR, NULL);
            info[i].data = (unsigned char*)cstr;
            info[i].len = MIN(len, strlen(cstr));
            break;
        case CAT_BIN:
            info[i].data = va_arg(ap, const void*);
            info[i].len = va_arg(ap, size_t);
            return_error_if_fail(info[i].data, E_STR_INVALID_DATA, NULL);
            break;
        default:
            abort();
        }
    
    if(!(cat = str_prepare(cat_len)))
        return error_pass(), NULL;
    
    for(cat_len=0, i=0; i < n; cat_len += info[i].len, i++)
        memcpy(&cat->data[cat_len], info[i].data, info[i].len);
    
    if(binary)
        _str_set_flags(cat, FLAG_BINARY);
    
    return cat;
}

str_ct str_cat(size_t n, ...)
{
    va_list ap;
    str_ct cat;
    
    va_start(ap, n);
    cat = error_pass_ptr(_str_cat(n, ap, CAT_STR));
    va_end(ap);
    
    return cat;
}

str_ct str_cat_v(size_t n, va_list ap)
{
    return error_pass_ptr(_str_cat(n, ap, CAT_STR));
}

str_ct str_cat_n(size_t n, ...)
{
    va_list ap;
    str_ct cat;
    
    va_start(ap, n);
    cat = error_pass_ptr(_str_cat(n, ap, CAT_STR_N));
    va_end(ap);
    
    return cat;
}

str_ct str_cat_vn(size_t n, va_list ap)
{
    return error_pass_ptr(_str_cat(n, ap, CAT_STR_N));
}

str_ct str_cat_c(size_t n, ...)
{
    va_list ap;
    str_ct cat;
    
    va_start(ap, n);
    cat = error_pass_ptr(_str_cat(n, ap, CAT_CSTR));
    va_end(ap);
    
    return cat;
}

str_ct str_cat_vc(size_t n, va_list ap)
{
    return error_pass_ptr(_str_cat(n, ap, CAT_CSTR));
}

str_ct str_cat_cn(size_t n, ...)
{
    va_list ap;
    str_ct cat;
    
    va_start(ap, n);
    cat = error_pass_ptr(_str_cat(n, ap, CAT_CSTR_N));
    va_end(ap);
    
    return cat;
}

str_ct str_cat_vcn(size_t n, va_list ap)
{
    return error_pass_ptr(_str_cat(n, ap, CAT_CSTR_N));
}

str_ct str_cat_b(size_t n, ...)
{
    va_list ap;
    str_ct cat;
    
    va_start(ap, n);
    cat = error_pass_ptr(_str_cat(n, ap, CAT_BIN));
    va_end(ap);
    
    return cat;
}

str_ct str_cat_vb(size_t n, va_list ap)
{
    return error_pass_ptr(_str_cat(n, ap, CAT_BIN));
}

str_ct str_remove(str_ct str, str_const_ct sub)
{
    return error_pass_ptr(str_replace(str, sub, LIT("")));
}

str_ct str_remove_n(str_ct str, str_const_ct sub, size_t len)
{
    return error_pass_ptr(str_replace_n(str, sub, len, LIT(""), 0));
}

str_ct str_remove_c(str_ct str, const char *sub)
{
    return error_pass_ptr(str_replace_c(str, sub, ""));
}

str_ct str_remove_cn(str_ct str, const char *sub, size_t len)
{
    return error_pass_ptr(str_replace_cn(str, sub, len, "", 0));
}

str_ct str_replace(str_ct str, str_const_ct sub, str_const_ct nsub)
{
    assert_str(sub);
    assert_str(nsub);
    
    if(_str_is_binary(nsub))
        return error_pass_ptr(str_replace_b(
            str, sub->data, _str_get_len(sub), nsub->data, _str_get_len(nsub)));
    else
        return error_pass_ptr(str_replace_cn(
            str, (char*)sub->data, _str_get_len(sub), (char*)nsub->data, _str_get_len(nsub)));
}

str_ct str_replace_n(str_ct str, str_const_ct sub, size_t sublen, str_const_ct nsub, size_t nsublen)
{
    assert_str(sub);
    assert_str(nsub);
    
    sublen = MIN(sublen, _str_get_len(sub));
    nsublen = MIN(nsublen, _str_get_len(nsub));
    
    if(_str_is_binary(nsub))
        return error_pass_ptr(str_replace_b(
            str, sub->data, sublen, nsub->data, nsublen));
    else
        return error_pass_ptr(str_replace_cn(
            str, (char*)sub->data, sublen, (char*)nsub->data, nsublen));
}

str_ct str_replace_c(str_ct str, const char *sub, const char *nsub)
{
    return_error_if_fail(sub && nsub, E_STR_INVALID_CSTR, NULL);
    
    return error_pass_ptr(str_replace_cn(str, sub, strlen(sub), nsub, strlen(nsub)));
}

str_ct str_replace_cn(str_ct str, const char *sub, size_t sublen, const char *nsub, size_t nsublen)
{
    vec_ct positions;
    unsigned char *ptr;
    size_t len, pos, ins, data, i, size;
    
    assert_str(str);
    return_error_if_fail(sub && nsub, E_STR_INVALID_CSTR, NULL);
    
    if(!_str_get_len(str) || !sublen)
        return str;
    
    if(!(positions = vec_new(2, sizeof(size_t))))
        return error_wrap(), NULL;
    
    for(ptr = str->data, len = str->len;
        len && (ptr = memmem(ptr, len, sub, sublen));
        ptr += sublen, len -= sublen)
    {
        pos = ptr - str->data;
        
        if(!vec_push_e(positions, &pos))
            return error_wrap(), vec_free(positions), NULL;
    }
    
    if(vec_is_empty(positions))
        return vec_free(positions), str;
    
    if(nsublen <= sublen) // replace left to right
    {
        if(!_str_get_writeable(str))
            return error_pass(), vec_free(positions), NULL;
        
        len = str->len - vec_size(positions)*(sublen-nsublen);
        vec_get_first(positions, &ins);
        data = ins + sublen;
        
        for(i=1, size = vec_size(positions); i <= size; i++)
        {
            memcpy(&str->data[ins], nsub, nsublen);
            ins += nsublen;
            
            pos = i < size ? *(size_t*)vec_at(positions, i) : str->len;
            memmove(&str->data[ins], &str->data[data], pos - data);
            ins += pos - data;
            data = pos + sublen;
        }
        
        str->len = len;
        
        if(!_str_is_binary(str))
            str->data[str->len] = '\0';
    }
    else // replace right to left
    {
        data = str->len; // data position at end of old str
        
        if(!str_grow(str, vec_size(positions)*(nsublen-sublen)))
            return error_pass(), vec_free(positions), NULL;
        
        ins = str->len; // insert position at end of new str
        
        for(i = vec_size(positions); i > 0; i--)
        {
            pos = *(size_t*)vec_at(positions, i-1);
            len = data - (pos+sublen);
            ins -= len;
            memmove(&str->data[ins], &str->data[pos+sublen], len);
            data = pos;
            
            ins -= nsublen;
            memcpy(&str->data[ins], nsub, nsublen);
        }
    }
    
    vec_free(positions);
    
    return str;
}

str_ct str_replace_b(str_ct str, const void *sub, size_t sublen, const void *nsub, size_t nsublen)
{
    return_error_if_fail(sub && nsub, E_STR_INVALID_DATA, NULL);
    
    if(!(str = str_replace_cn(str, sub, sublen, nsub, nsublen)))
        return error_pass(), NULL;
    
    _str_set_flags(str, FLAG_BINARY);
    
    return str;
}

str_ct str_substr(str_const_ct str, ssize_t pos, size_t len)
{
    assert_str(str);
    
    if(pos < 0)
        pos += _str_get_len(str);
    
    return_error_if_fail(pos >= 0 && pos+len <= _str_get_len(str), E_STR_OUT_OF_BOUNDS, NULL);
    
    if(str->type == DATA_STATIC)
    {
        if(_str_is_binary(str)) // no null terminator required
            return error_pass_ptr(str_new_bs(&str->data[pos], len));
        
        if(pos+len == str->len) // suffix can just be referenced
            return error_pass_ptr(str_new_sn((char*)&str->data[pos], len));
    }
    
    if(_str_is_binary(str))
        return error_pass_ptr(str_dup_b(&str->data[pos], len));
    else
        return error_pass_ptr(str_dup_cn((char*)&str->data[pos], len));
}

str_ct str_substr_r(str_const_ct str, ssize_t pos, size_t len)
{
    assert_str(str);
    
    if(pos < 0)
        pos += _str_get_len(str);
    
    return_error_if_fail(pos >= 0 && pos+len <= _str_get_len(str), E_STR_OUT_OF_BOUNDS, NULL);
    
    if(!pos && len == str->len)
        return error_pass_ptr(str_ref(str));
    else
        return error_pass_ptr(str_substr(str, pos, len));
}

str_ct str_slice(str_ct str, ssize_t pos, size_t len)
{
    unsigned char *data;
    
    assert_str(str);
    return_error_if_pass(_str_is_const(str), E_STR_CONST, NULL);
    
    if(pos < 0)
        pos += _str_get_len(str);
    
    return_error_if_fail(pos >= 0 && pos+len <= _str_get_len(str), E_STR_OUT_OF_BOUNDS, NULL);
    
    if(!pos && len == str->len)
        return str;
    
    if(!len)
        return str_set_l(str, "");
    
    data = str->data;
    
    if(str->type == DATA_STATIC)
    {
        if(pos+len == str->len) // suffix, just move start forward
        {
            str->data += len;
            str->len = len;
            return str;
        }
        
        if(!str_resize(str, len))
            return error_pass(), NULL;
    }
    else
        str->len = len;
    
    if(pos)
        memmove(str->data, &data[pos], len);
    
    if(!_str_is_binary(str))
        str->data[str->len] = '\0';
    
    return str;
}

str_ct str_slice_head(str_ct str, size_t len)
{
    return error_pass_ptr(str_slice(str, 0, len));
}

str_ct str_slice_tail(str_ct str, size_t len)
{
    return error_pass_ptr(str_slice(str, -len, len));
}

str_ct str_cut(str_ct str, ssize_t pos, size_t len)
{
    unsigned char *data;
    
    assert_str(str);
    return_error_if_pass(_str_is_const(str), E_STR_CONST, NULL);
    
    if(pos < 0)
        pos += _str_get_len(str);
    
    return_error_if_fail(pos >= 0 && pos+len <= _str_get_len(str), E_STR_OUT_OF_BOUNDS, NULL);
    
    if(!len)
        return str;
    
    data = str->data;
    
    if(str->type == DATA_STATIC)
    {
        if(!pos) // suffix left, just move start forward
        {
            str->data += len;
            str->len -= len;
            return str;
        }
        
        if(!str_resize(str, str->len-len))
            return error_pass(), NULL;
    }
    else
        str->len -= len;
    
    if(pos < str->len)
        memmove(&str->data[pos], &data[pos+len], str->len - pos);
    
    if(!_str_is_binary(str))
        str->data[str->len] = '\0';
    
    return str;
}

str_ct str_cut_head(str_ct str, size_t len)
{
    return error_pass_ptr(str_cut(str, 0, len));
}

str_ct str_cut_tail(str_ct str, size_t len)
{
    return error_pass_ptr(str_cut(str, -len, len));
}

str_ct str_trim_pred(str_ct str, ctype_pred_cb pred)
{
    unsigned char *start, *end;
    
    assert_str(str);
    return_error_if_fail(pred, E_STR_INVALID_CALLBACK, NULL);
    
    if(!(start = memwhile(str->data, _str_get_len(str), pred)))
        return str_clear(str);
    
    if(!(end = memrwhile(start, str->len - (start-str->data), pred)))
        abort();
    
    return error_pass_ptr(str_slice(str, start - str->data, end+1 - start));
}

str_ct str_trim_blank(str_ct str)
{
    return error_pass_ptr(str_trim_pred(str, isblank));
}

str_ct str_trim_space(str_ct str)
{
    return error_pass_ptr(str_trim_pred(str, isspace));
}

str_ct str_transpose_f(str_ct str, ctype_transpose_cb trans)
{
    assert_str(str);
    return_error_if_fail(trans, E_STR_INVALID_CALLBACK, NULL);
    
    if(!_str_get_writeable(str))
        return error_pass(), NULL;
    
    if(_str_is_binary(str))
        memtranspose_f(str->data, _str_get_len(str), trans);
    else
        strtranspose_f((char*)str->data, trans);
    
    return str;
}

str_ct str_transpose_lower(str_ct str)
{
    return error_pass_ptr(str_transpose_f(str, tolower));
}

str_ct str_transpose_upper(str_ct str)
{
    return error_pass_ptr(str_transpose_f(str, toupper));
}

str_ct str_translate(str_ct str, ctype_translate_cb trans)
{
    size_t len;
    void *data;
    
    assert_str(str);
    return_error_if_fail(trans, E_STR_INVALID_CALLBACK, NULL);
    
    if(_str_is_binary(str))
    {
        len = strtranslate_mem(NULL, str->data, _str_get_len(str), trans);
        
        if(!(data = calloc(1, len+1)))
            return error_wrap_errno(calloc), NULL;
        
        strtranslate_mem(data, str->data, str->len, trans);
        
        if(!str_set_hn(str, data, len))
            return error_pass(), free(data), NULL;
    }
    else
    {
        len = strtranslate(NULL, (char*)str->data, trans);
        
        if(!(data = calloc(1, len+1)))
            return error_wrap_errno(calloc), NULL;
        
        strtranslate(data, (char*)str->data, trans);
        
        if(!str_set_hn(str, data, len))
            return error_pass(), free(data), NULL;
    }
    
    return str;
}

str_ct str_translate_b(str_ct str, ctype_translate_cb trans)
{
    if(!str_translate(str, trans))
        return error_pass(), NULL;
    
    _str_set_flags(str, FLAG_BINARY);
    
    return str;
}

str_ct str_dup_translate(str_ct str, ctype_translate_cb trans)
{
    str_ct nstr;
    size_t len;
    
    assert_str(str);
    return_error_if_fail(trans, E_STR_INVALID_CALLBACK, NULL);
    
    if(_str_is_binary(str))
    {
        len = strtranslate_mem(NULL, str->data, _str_get_len(str), trans);
        
        if(!(nstr = str_prepare(len)))
            return error_pass(), NULL;
        
        strtranslate_mem((char*)nstr->data, str->data, str->len, trans);
    }
    else
    {
        len = strtranslate(NULL, (char*)str->data, trans);
        
        if(!(nstr = str_prepare(len)))
            return error_pass(), NULL;
        
        strtranslate((char*)nstr->data, (char*)str->data, trans);
    }
    
    return nstr;
}

str_ct str_dup_translate_b(str_ct str, ctype_translate_cb trans)
{
    if(!(str = str_dup_translate(str, trans)))
        return error_pass(), NULL;
    
    _str_set_flags(str, FLAG_BINARY);
    
    return str;
}

str_ct str_escape(str_ct str)
{
    return error_pass_ptr(str_translate(str, translate_escape));
}

str_ct str_unescape(str_ct str)
{
    return error_pass_ptr(str_translate(str, translate_unescape));
}

str_ct str_unescape_b(str_ct str)
{
    return error_pass_ptr(str_translate_b(str, translate_unescape));
}

str_ct str_dup_escape(str_ct str)
{
    return error_pass_ptr(str_dup_translate(str, translate_escape));
}

str_ct str_dup_unescape(str_ct str)
{
    return error_pass_ptr(str_dup_translate(str, translate_unescape));
}

str_ct str_dup_unescape_b(str_ct str)
{
    return error_pass_ptr(str_dup_translate_b(str, translate_unescape));
}

int str_cmp(str_const_ct str1, str_const_ct str2)
{
    size_t len1, len2;
    int rc;
    
    assert_str(str1);
    assert_str(str2);
    
    if(!_str_is_binary(str1) && !_str_is_binary(str2))
        return strcmp((char*)str1->data, (char*)str2->data);
    
    len1 = _str_get_len(str1);
    len2 = _str_get_len(str2);
    rc = memcmp(str1->data, str2->data, MIN(len1, len2));
    
    return rc ? rc : len1 > len2 ? 1 : len1 < len2 ? -1 : 0;
}

int str_cmp_n(str_const_ct str1, str_const_ct str2, size_t n)
{
    size_t len1, len2;
    int rc;
    
    assert_str(str1);
    assert_str(str2);
    
    if(!n)
        return 0;
    
    if(!_str_is_binary(str1) && !_str_is_binary(str2))
        return strncmp((char*)str1->data, (char*)str2->data, n);
    
    len1 = _str_get_len(str1);
    len2 = _str_get_len(str2);
    
    if(len1 >= n && len2 >= n)
        return memcmp(str1->data, str2->data, n);
    
    rc = memcmp(str1->data, str2->data, MIN(len1, len2));
    
    return rc ? rc : len1 > len2 ? 1 : len1 < len2 ? -1 : 0;
}

int str_cmp_c(str_const_ct str, const char *cstr)
{
    return str_cmp(str, tstr_new_s(cstr));
}

int str_cmp_cn(str_const_ct str, const char *cstr, size_t len)
{
    return str_cmp(str, tstr_new_sn(cstr, len));
}

int str_cmp_nc(str_const_ct str, const char *cstr, size_t n)
{
    return str_cmp_n(str, tstr_new_s(cstr), n);
}

int str_cmp_ncn(str_const_ct str, const char *cstr, size_t len, size_t n)
{
    return str_cmp_n(str, tstr_new_sn(cstr, len), n);
}

int str_cmp_b(str_const_ct str, const void *mem, size_t size)
{
    return str_cmp(str, tstr_new_bs(mem, size));
}

int str_cmp_nb(str_const_ct str, const void *mem, size_t n)
{
    return str_cmp_n(str, tstr_new_bs(mem, n), n);
}

int str_casecmp(str_const_ct str1, str_const_ct str2)
{
    size_t len1, len2;
    int rc;
    
    assert_str(str1);
    assert_str(str2);
    
    if(!_str_is_binary(str1) && !_str_is_binary(str2))
        return strcasecmp((char*)str1->data, (char*)str2->data);
    
    len1 = _str_get_len(str1);
    len2 = _str_get_len(str2);
    rc = memcasecmp(str1->data, str2->data, MIN(len1, len2));
    
    return rc ? rc : len1 > len2 ? 1 : len1 < len2 ? -1 : 0;
}

int str_casecmp_n(str_const_ct str1, str_const_ct str2, size_t n)
{
    size_t len1, len2;
    int rc;
    
    assert_str(str1);
    assert_str(str2);
    
    if(!n)
        return 0;
    
    if(!_str_is_binary(str1) && !_str_is_binary(str2))
        return strncasecmp((char*)str1->data, (char*)str2->data, n);
    
    len1 = _str_get_len(str1);
    len2 = _str_get_len(str2);
    
    if(len1 >= n && len2 >= n)
        return memcasecmp(str1->data, str2->data, n);
    
    rc = memcasecmp(str1->data, str2->data, MIN(len1, len2));
    
    return rc ? rc : len1 > len2 ? 1 : len1 < len2 ? -1 : 0;
}

int str_casecmp_c(str_const_ct str, const char *cstr)
{
    return str_casecmp(str, tstr_new_s(cstr));
}

int str_casecmp_cn(str_const_ct str, const char *cstr, size_t len)
{
    return str_casecmp(str, tstr_new_sn(cstr, len));
}

int str_casecmp_nc(str_const_ct str, const char *cstr, size_t n)
{
    return str_casecmp_n(str, tstr_new_s(cstr), n);
}

int str_casecmp_ncn(str_const_ct str, const char *cstr, size_t len, size_t n)
{
    return str_casecmp_n(str, tstr_new_sn(cstr, len), n);
}

int str_casecmp_b(str_const_ct str, const void *mem, size_t size)
{
    return str_casecmp(str, tstr_new_bs(mem, size));
}

int str_casecmp_nb(str_const_ct str, const void *mem, size_t n)
{
    return str_casecmp_n(str, tstr_new_bs(mem, n), n);
}
