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
#include <ytil/ext/string.h>
#include <stdint.h>
#include <stdio.h>


enum str_def
{
      MAGIC     = define_magic('S', 'T', 'R')
};

typedef enum str_flags
{
      NO_FLAGS          = 0
    , FLAG_TRANSIENT    = BV(0) // str is stack allocated
    , FLAG_UPDATE_LEN   = BV(1) // len is unset and has to be calculated
    , FLAG_UPDATE_CAP   = BV(2) // cap is unset and has to be calculated
    , FLAG_BINARY       = BV(3) // data my contain control chars
    , FLAG_REDIRECT     = BV(4) // transient str points to heap allocated str
} str_flag_fs;

typedef enum str_type
{
      DATA_HEAP         // data is heap allocated
    , DATA_CONST        // data is not to be modified
    , DATA_TRANSIENT    // data is stack allocated
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


static const error_info_st error_infos[] =
{
      ERROR_INFO(E_STR_BINARY, "Operation not supported on binary data str.")
    , ERROR_INFO(E_STR_EMPTY, "Operation not supported on empty str.")
    , ERROR_INFO(E_STR_INVALID_CSTR, "Invalid str data.")
    , ERROR_INFO(E_STR_INVALID_DATA, "Invalid binary data.")
    , ERROR_INFO(E_STR_INVALID_FORMAT, "Invalid format.")
    , ERROR_INFO(E_STR_INVALID_LENGTH, "Invalid str length.")
    , ERROR_INFO(E_STR_OUT_OF_BOUNDS, "Out of bounds access.")
    , ERROR_INFO(E_STR_UNREFERENCED, "Operation not supported on unreferenced str.")
};



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
    return (_str_is_binary(str) && !str->len) || !str->data[0];
}

static str_ct _str_get_writeable(str_ct str)
{
    unsigned char *data;
    
    if(str->type != DATA_CONST)
        return str;
    
    // heap data is only allowed on strings which are referenced
    return_error_if_fail(str->ref, E_STR_UNREFERENCED, NULL);
    
    if(!(data = calloc(1, _str_get_len(str)+1)))
        return error_set_errno(calloc), NULL;
    
    memcpy(data, str->data, _str_get_len(str));
    
    str->type = DATA_HEAP;
    str->cap = _str_get_len(str);
    str->data = data;
    
    return str;
}

str_ct _str_init(str_ct str, str_flag_fs flags, uint16_t ref, str_type_id type, unsigned char *data, ssize_t len, ssize_t cap)
{
    assert(str && type < DATA_TYPES && data && (!cap || len <= cap));
    
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
        if(type != DATA_CONST)
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
    
    assert(!(flags & ~FLAG_BINARY));
    
    if(!(str = calloc(1, sizeof(str_st))))
        return error_set_errno(calloc), NULL;
    
    return _str_init(str, flags, 1, type, data, len, cap);
}

str_ct _str_set(str_ct str, str_type_id type, unsigned char *data, ssize_t len, ssize_t cap, str_flag_fs flags)
{
    // heap data is only allowed on strings which are referenced
    assert(str->ref || type != DATA_HEAP);
    assert(!(flags & ~FLAG_BINARY));
    
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

bool str_data_is_heap(str_const_ct str)
{
    assert_str(str);
    
    return str->type == DATA_HEAP;
}

bool str_data_is_const(str_const_ct str)
{
    assert_str(str);
    
    return str->type == DATA_CONST;
}

bool str_data_is_transient(str_const_ct str)
{
    assert_str(str);
    
    return str->type == DATA_TRANSIENT;
}

bool str_data_is_binary(str_const_ct str)
{
    assert_str(str);
    
    return _str_is_binary(str);
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
        return error_propagate(), NULL;
    
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
        return error_propagate(), NULL;
    
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
    return_error_if_fail(str->type != DATA_CONST || !str->data[len], E_STR_INVALID_LENGTH, NULL);
    
    _str_set_len(str, len);
    
    if(str->type != DATA_CONST)
        str->data[len] = '\0';
    
    return (str_ct)str;
}

char str_first(str_const_ct str)
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
    
    flags = str->flags & FLAG_BINARY;
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
        return error_set_errno(memdup), NULL;
    
    if(!(nstr = _str_new(type, data, len, cap, flags)))
    {
        if(str->type == DATA_TRANSIENT)
            free(data);
        
        return error_propagate(), NULL;
    }
    
    // if transient string has ref redirect to heap string
    if(str->ref)
    {
        _str_set_flags(str, FLAG_REDIRECT);
        vstr->data = (void*)nstr;
        nstr->ref += str->ref;
    }
    
    return nstr;
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

size_t str_get_refs(str_const_ct str)
{
    assert_str(str);
    
    return str->ref;
}

str_ct str_clear(str_ct str)
{
    assert_str(str);
    
    if(str->type == DATA_CONST)
        str->data = (unsigned char*)"";
    else
        str->data[0] = '\0';
    
    _str_set_len(str, 0);
    
    return str;
}

str_ct str_truncate(str_const_ct str)
{
    str_ct vstr = (str_ct)str;
    unsigned char *data;
    
    assert_str(str);
    
    switch(str->type)
    {
    case DATA_CONST:
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
    
    switch(str->type)
    {
    case DATA_CONST:
        if(!str->ref)
            return error_set(E_STR_UNREFERENCED), NULL;
        else if(!(data = calloc(1, len+1)))
            return error_set_errno(calloc), NULL;
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
            return error_set_errno(calloc), NULL;
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
            return error_set_errno(realloc), NULL;
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
    str->data[len] = '\0';
    
    return str;
}

str_ct str_resize_set(str_ct str, size_t new_len, char c)
{
    size_t len;
    
    assert_str(str);
    
    len = _str_get_len(str);
    
    if(!str_resize(str, new_len))
        return error_propagate(), NULL;
    
    if(new_len > len)
        memset(&str->data[len], c, new_len-len);
    
    return str;
}

str_ct str_prepare(size_t len)
{
    return error_propagate_ptr(str_prepare_c(len, len));
}

str_ct str_prepare_c(size_t len, size_t cap)
{
    str_ct str;
    unsigned char *data;
    
    return_error_if_fail(len <= cap, E_STR_INVALID_LENGTH, NULL);
    
    if(!(data = calloc(1, cap+1)))
        return error_set_errno(calloc), NULL;
    
    if(!(str = _str_new(DATA_HEAP, data, len, cap, NO_FLAGS)))
        return error_propagate(), free(data), NULL;
    
    return str;
}

str_ct str_prepare_b(size_t len)
{
    return error_propagate_ptr(str_prepare_bc(len, len));
}

str_ct str_prepare_bc(size_t len, size_t cap)
{
    str_ct str;
    unsigned char *data;
    
    return_error_if_fail(len <= cap, E_STR_INVALID_LENGTH, NULL);
    
    if(!(data = calloc(1, cap+1)))
        return error_set_errno(calloc), NULL;
    
    if(!(str = _str_new(DATA_HEAP, data, len, cap, FLAG_BINARY)))
        return error_propagate(), free(data), NULL;
    
    return str;
}

str_ct str_prepare_set(size_t len, char c)
{
    return error_propagate_ptr(str_prepare_set_c(len, len, c));
}

str_ct str_prepare_set_c(size_t len, size_t cap, char c)
{
    str_ct str;
    
    if(!(str = str_prepare_c(len, cap)))
        return error_propagate(), NULL;
    
    memset(str->data, c, len);
    
    return str;
}

str_ct str_prepare_set_b(size_t len, char c)
{
    return error_propagate_ptr(str_prepare_set_bc(len, len, c));
}

str_ct str_prepare_set_bc(size_t len, size_t cap, char c)
{
    str_ct str;
    
    if(!(str = str_prepare_bc(len, cap)))
        return error_propagate(), NULL;
    
    memset(str->data, c, len);
    
    return str;
}

str_ct str_new_h(char *cstr)
{
    return_error_if_fail(cstr, E_STR_INVALID_CSTR, NULL);
    
    return error_propagate_ptr(_str_new(DATA_HEAP, (unsigned char*)cstr, -1, -1, NO_FLAGS));
}

str_ct str_new_hn(char *cstr, size_t len)
{
    return_error_if_fail(cstr, E_STR_INVALID_CSTR, NULL);
    
    return error_propagate_ptr(_str_new(DATA_HEAP, (unsigned char*)cstr, len, len, NO_FLAGS));
}

str_ct str_new_hnc(char *cstr, size_t len, size_t cap)
{
    return_error_if_fail(cstr, E_STR_INVALID_CSTR, NULL);
    return_error_if_fail(len <= cap, E_STR_INVALID_LENGTH, NULL);
    
    return error_propagate_ptr(_str_new(DATA_HEAP, (unsigned char*)cstr, len, cap, NO_FLAGS));
}

str_ct str_new_c(const char *cstr)
{
    return_error_if_fail(cstr, E_STR_INVALID_CSTR, NULL);
    
    return error_propagate_ptr(_str_new(DATA_CONST, (unsigned char*)cstr, -1, 0, NO_FLAGS));
}

str_ct str_new_cn(const char *cstr, size_t len)
{
    return_error_if_fail(cstr && !cstr[len], E_STR_INVALID_CSTR, NULL);
    
    return error_propagate_ptr(_str_new(DATA_CONST, (unsigned char*)cstr, len, 0, NO_FLAGS));
}

str_ct str_new_bh(void *data, size_t len)
{
    return_error_if_fail(data, E_STR_INVALID_DATA, NULL);
    
    return error_propagate_ptr(_str_new(DATA_HEAP, data, len, len, FLAG_BINARY));
}

str_ct str_new_bhc(void *data, size_t len, size_t cap)
{
    return_error_if_fail(data, E_STR_INVALID_DATA, NULL);
    return_error_if_fail(len <= cap, E_STR_INVALID_LENGTH, NULL);
    
    return error_propagate_ptr(_str_new(DATA_HEAP, data, len, cap, FLAG_BINARY));
}

str_ct str_new_bc(const void *data, size_t len)
{
    return_error_if_fail(data, E_STR_INVALID_DATA, NULL);
    
    return error_propagate_ptr(_str_new(DATA_CONST, (void*)data, len, 0, FLAG_BINARY));
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

str_ct tstr_init_c(str_ct str, const char *cstr)
{
    return_error_if_fail(cstr, E_STR_INVALID_CSTR, NULL);
    
    return _str_init(str, FLAG_TRANSIENT, 0, DATA_CONST, (unsigned char*)cstr, -1, 0);
}

str_ct tstr_init_cn(str_ct str, const char *cstr, size_t len)
{
    return_error_if_fail(cstr && !cstr[len], E_STR_INVALID_CSTR, NULL);
    
    return _str_init(str, FLAG_TRANSIENT, 0, DATA_CONST, (unsigned char*)cstr, len, 0);
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

str_ct tstr_init_bc(str_ct str, const void *bin, size_t len)
{
    return_error_if_fail(bin, E_STR_INVALID_DATA, NULL);
    
    return _str_init(str, FLAG_TRANSIENT|FLAG_BINARY, 0, DATA_CONST, (void*)bin, len, 0);
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

str_ct str_dup(str_const_ct str)
{
    return error_propagate_ptr(str_dup_n(str, str_len(str)));
}

str_ct str_dup_n(str_const_ct str, size_t len)
{
    assert_str(str);
    
    len = MIN(len, _str_get_len(str));
    
    if(str->type == DATA_CONST)
    {
        if(_str_is_binary(str)) // no terminator for binary required
            return error_propagate_ptr(str_new_bc(str->data, len));
        else if(!len) // zero length const data can be statically provided
            return error_propagate_ptr(str_new_l(""));
        else if(len == _str_get_len(str)) // just reference it
            return error_propagate_ptr(str_new_cn((char*)str->data, len));
        else // heap dup to ensure terminator
            return error_propagate_ptr(str_dup_cn((char*)str->data, len));
    }
    else
    {
        if(_str_is_binary(str))
            return error_propagate_ptr(str_dup_b(str->data, len));
        else
            return error_propagate_ptr(str_dup_cn((char*)str->data, len));
    }
}

str_ct str_dup_c(const char *cstr)
{
    return_error_if_fail(cstr, E_STR_INVALID_CSTR, NULL);
    
    return error_propagate_ptr(str_dup_cn(cstr, strlen(cstr)));
}

str_ct str_dup_cn(const char *cstr, size_t len)
{
    str_ct str;
    
    return_error_if_fail(cstr, E_STR_INVALID_CSTR, NULL);
    
    if(!(str = str_prepare_c(len, len)))
        return error_propagate(), NULL;
    
    memcpy(str->data, cstr, len);
    
    return str;
}

str_ct str_dup_b(const void *data, size_t len)
{
    str_ct str;
    
    return_error_if_fail(data, E_STR_INVALID_DATA, NULL);
    
    if(!(str = str_dup_cn(data, len)))
        return error_propagate(), NULL;
    
    _str_set_flags(str, FLAG_BINARY);
    
    return str;
}

str_ct str_dup_f(const char *fmt, ...)
{
    str_ct str;
    va_list ap;
    
    va_start(ap, fmt);
    str = error_propagate_ptr(str_dup_vf(fmt, ap));
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
        return error_propagate(), NULL;
    
    vsnprintf((char*)str->data, len+1, fmt, ap);
    
    return str;
}

str_ct str_set_h(str_ct str, char *cstr)
{
    assert_str(str);
    return_error_if_fail(str->ref, E_STR_UNREFERENCED, NULL);
    return_error_if_fail(cstr, E_STR_INVALID_CSTR, NULL);
    
    return _str_set(str, DATA_HEAP, (unsigned char*)cstr, -1, -1, NO_FLAGS);
}

str_ct str_set_hn(str_ct str, char *cstr, size_t len)
{
    assert_str(str);
    return_error_if_fail(str->ref, E_STR_UNREFERENCED, NULL);
    return_error_if_fail(cstr, E_STR_INVALID_CSTR, NULL);
    
    return _str_set(str, DATA_HEAP, (unsigned char*)cstr, len, len, NO_FLAGS);
}

str_ct str_set_hnc(str_ct str, char *cstr, size_t len, size_t cap)
{
    assert_str(str);
    return_error_if_fail(str->ref, E_STR_UNREFERENCED, NULL);
    return_error_if_fail(cstr, E_STR_INVALID_CSTR, NULL);
    return_error_if_fail(len <= cap, E_STR_INVALID_LENGTH, NULL);
    
    return _str_set(str, DATA_HEAP, (unsigned char*)cstr, len, cap, NO_FLAGS);
}

str_ct str_set_c(str_ct str, const char *cstr)
{
    assert_str(str);
    return_error_if_fail(cstr, E_STR_INVALID_CSTR, NULL);
    
    return _str_set(str, DATA_CONST, (unsigned char*)cstr, -1, 0, NO_FLAGS);
}

str_ct str_set_cn(str_ct str, const char *cstr, size_t len)
{
    assert_str(str);
    return_error_if_fail(cstr && !cstr[len], E_STR_INVALID_CSTR, NULL);
    
    return _str_set(str, DATA_CONST, (unsigned char*)cstr, len, 0, NO_FLAGS);
}

str_ct str_set_bh(str_ct str, void *data, size_t len)
{
    assert_str(str);
    return_error_if_fail(str->ref, E_STR_UNREFERENCED, NULL);
    return_error_if_fail(data, E_STR_INVALID_DATA, NULL);
    
    return _str_set(str, DATA_HEAP, data, len, len, FLAG_BINARY);
}

str_ct str_set_bhc(str_ct str, void *data, size_t len, size_t cap)
{
    assert_str(str);
    return_error_if_fail(str->ref, E_STR_UNREFERENCED, NULL);
    return_error_if_fail(data, E_STR_INVALID_DATA, NULL);
    return_error_if_fail(len <= cap, E_STR_INVALID_LENGTH, NULL);
    
    return _str_set(str, DATA_HEAP, data, len, cap, FLAG_BINARY);
}

str_ct str_set_bc(str_ct str, const void *data, size_t len)
{
    assert_str(str);
    return_error_if_fail(data, E_STR_INVALID_DATA, NULL);
    
    return _str_set(str, DATA_CONST, (void*)data, len, 0, FLAG_BINARY);
}

str_ct str_copy(str_ct dst, size_t pos, str_const_ct src)
{
    assert_str(src);
    
    if(_str_is_binary(src))
        return error_propagate_ptr(str_copy_b(dst, pos, src->data, _str_get_len(src)));
    else
        return error_propagate_ptr(str_copy_cn(dst, pos, (void*)src->data, _str_get_len(src)));
}

str_ct str_copy_n(str_ct dst, size_t pos, str_const_ct src, size_t len)
{
    assert_str(src);
    
    len = MIN(len, _str_get_len(src));
    
    if(_str_is_binary(src))
        return error_propagate_ptr(str_copy_b(dst, pos, src->data, len));
    else
        return error_propagate_ptr(str_copy_cn(dst, pos, (void*)src->data, len));
}

str_ct str_copy_c(str_ct dst, size_t pos, const char *src)
{
    return_error_if_fail(src, E_STR_INVALID_CSTR, NULL);
    
    return error_propagate_ptr(str_copy_cn(dst, pos, src, strlen(src)));
}

str_ct str_copy_cn(str_ct dst, size_t pos, const char *src, size_t len)
{
    assert_str(dst);
    return_error_if_fail(src, E_STR_INVALID_CSTR, NULL);
    return_error_if_fail(pos <= _str_get_len(dst), E_STR_OUT_OF_BOUNDS, NULL);
    
    if(!(dst = str_resize(dst, pos + len)))
        return error_propagate(), NULL;
    
    memcpy(&dst->data[pos], src, len);
    
    return dst;
}

str_ct str_copy_b(str_ct dst, size_t pos, const void *src, size_t len)
{
    return_error_if_fail(src, E_STR_INVALID_DATA, NULL);
    
    if(!(dst = str_copy_cn(dst, pos, src, len)))
        return error_propagate(), NULL;
    
    _str_set_flags(dst, FLAG_BINARY);
    
    return dst;
}

str_ct str_copy_f(str_ct dst, size_t pos, const char *fmt, ...)
{
    va_list ap;
    
    va_start(ap, fmt);
    dst = error_propagate_ptr(str_copy_vf(dst, pos, fmt, ap));
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
        return error_propagate(), NULL;
    
    vsnprintf((char*)&dst->data[pos], len+1, fmt, ap);
    
    return dst;
}

ssize_t str_overwrite(str_ct dst, size_t pos, str_const_ct src)
{
    assert_str(src);
    
    if(_str_is_binary(src))
        return error_propagate_int(str_overwrite_b(dst, pos, src->data, _str_get_len(src)));
    else
        return error_propagate_int(str_overwrite_cn(dst, pos, (char*)src->data, _str_get_len(src)));
}

ssize_t str_overwrite_n(str_ct dst, size_t pos, str_const_ct src, size_t len)
{
    assert_str(src);
    
    len = MIN(len, _str_get_len(src));
    
    if(_str_is_binary(src))
        return error_propagate_int(str_overwrite_b(dst, pos, src->data, len));
    else
        return error_propagate_int(str_overwrite_cn(dst, pos, (char*)src->data, len));
}

ssize_t str_overwrite_c(str_ct dst, size_t pos, const char *src)
{
    return_error_if_fail(src, E_STR_INVALID_CSTR, -1);
    
    return error_propagate_int(str_overwrite_cn(dst, pos, src, strlen(src)));
}

ssize_t str_overwrite_cn(str_ct dst, size_t pos, const char *src, size_t len)
{
    assert_str(dst);
    return_error_if_fail(src, E_STR_INVALID_CSTR, -1);
    return_error_if_fail(pos < _str_get_len(dst), E_STR_OUT_OF_BOUNDS, -1);
    
    if(!(dst = _str_get_writeable(dst)))
        return error_propagate(), -1;
    
    len = MIN(len, _str_get_len(dst) - pos);
    memcpy(&dst->data[pos], src, len);
    
    return len;
}

ssize_t str_overwrite_b(str_ct dst, size_t pos, const void *src, size_t len)
{
    ssize_t count;
    
    return_error_if_fail(src, E_STR_INVALID_DATA, -1);
    
    if((count = str_overwrite_cn(dst, pos, src, len)) < 0)
        return error_propagate(), -1;
    
    _str_set_flags(dst, FLAG_BINARY);
    
    return count;
}

ssize_t str_overwrite_f(str_ct dst, size_t pos, const char *fmt, ...)
{
    va_list ap;
    ssize_t len;
    
    va_start(ap, fmt);
    len = error_propagate_int(str_overwrite_vfn(dst, pos, str_len(dst), fmt, ap));
    va_end(ap);
    
    return len;
}

ssize_t str_overwrite_fn(str_ct dst, size_t pos, size_t len, const char *fmt, ...)
{
    va_list ap;
    
    va_start(ap, fmt);
    len = error_propagate_int(str_overwrite_vfn(dst, pos, len, fmt, ap));
    va_end(ap);
    
    return len;
}

ssize_t str_overwrite_vf(str_ct dst, size_t pos, const char *fmt, va_list ap)
{
    return error_propagate_int(str_overwrite_vfn(dst, pos, str_len(dst), fmt, ap));
}

ssize_t str_overwrite_vfn(str_ct dst, size_t pos, size_t len, const char *fmt, va_list ap)
{
    va_list ap2;
    unsigned char tmp;
    
    assert_str(dst);
    return_error_if_fail(fmt, E_STR_INVALID_FORMAT, -1);
    return_error_if_fail(pos < _str_get_len(dst), E_STR_OUT_OF_BOUNDS, -1);
    
    if(!(dst = _str_get_writeable(dst)))
        return error_propagate(), -1;
    
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
        return error_propagate(), NULL;
    
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
        return error_propagate(), NULL;
    
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
        return error_propagate_ptr(str_prepend_b(str, prefix->data, _str_get_len(prefix)));
    else
        return error_propagate_ptr(str_prepend_cn(str, (char*)prefix->data, _str_get_len(prefix)));
}

str_ct str_prepend_n(str_ct str, str_const_ct prefix, size_t len)
{
    assert_str(prefix);
    
    len = MIN(len, _str_get_len(prefix));
    
    if(_str_is_binary(prefix))
        return error_propagate_ptr(str_prepend_b(str, prefix->data, len));
    else
        return error_propagate_ptr(str_prepend_cn(str, (char*)prefix->data, len));
}

str_ct str_prepend_c(str_ct str, const char *prefix)
{
    assert_str(str);
    return_error_if_fail(prefix, E_STR_INVALID_CSTR, NULL);
    
    return error_propagate_ptr(_str_insert(str, 0, prefix, strlen(prefix)));
}

str_ct str_prepend_cn(str_ct str, const char *prefix, size_t len)
{
    assert_str(str);
    return_error_if_fail(prefix, E_STR_INVALID_CSTR, NULL);
    
    return error_propagate_ptr(_str_insert(str, 0, prefix, len));
}

str_ct str_prepend_b(str_ct str, const void *prefix, size_t len)
{
    assert_str(str);
    return_error_if_fail(prefix, E_STR_INVALID_DATA, NULL);
    
    if(!(str = _str_insert(str, 0, prefix, len)))
        return error_propagate(), NULL;
    
    _str_set_flags(str, FLAG_BINARY);
    
    return str;
}

str_ct str_prepend_set(str_ct str, size_t len, char c)
{
    assert_str(str);
    
    if(!(str = _str_insert(str, 0, NULL, len)))
        return error_propagate(), NULL;
    
    memset(str->data, c, len);
    
    return str;
}

str_ct str_prepend_f(str_ct str, const char *fmt, ...)
{
    va_list ap;
    
    assert_str(str);
    return_error_if_fail(fmt, E_STR_INVALID_FORMAT, NULL);
    
    va_start(ap, fmt);
    str = error_propagate_ptr(_str_insert_f(str, 0, fmt, ap));
    va_end(ap);
    
    return str;
}

str_ct str_prepend_vf(str_ct str, const char *fmt, va_list ap)
{
    assert_str(str);
    return_error_if_fail(fmt, E_STR_INVALID_FORMAT, NULL);
    
    return error_propagate_ptr(_str_insert_f(str, 0, fmt, ap));
}

str_ct str_append(str_ct str, str_const_ct suffix)
{
    assert_str(suffix);
    
    if(_str_is_binary(suffix))
        return error_propagate_ptr(str_append_b(str, suffix->data, _str_get_len(suffix)));
    else
        return error_propagate_ptr(str_append_cn(str, (char*)suffix->data, _str_get_len(suffix)));
}

str_ct str_append_n(str_ct str, str_const_ct suffix, size_t len)
{
    assert_str(suffix);
    
    len = MIN(len, _str_get_len(suffix));
    
    if(_str_is_binary(suffix))
        return error_propagate_ptr(str_append_b(str, suffix->data, len));
    else
        return error_propagate_ptr(str_append_cn(str, (char*)suffix->data, len));
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
        return error_propagate(), NULL;
    
    _str_set_flags(str, FLAG_BINARY);
    
    return str;
}

str_ct str_append_set(str_ct str, size_t suffix_len, char c)
{
    size_t len;
    
    assert_str(str);
    
    len = _str_get_len(str);
    
    if(!(str = _str_insert(str, len, NULL, suffix_len)))
        return error_propagate(), NULL;
    
    memset(&str->data[len], c, suffix_len);
    
    return str;
}

str_ct str_append_f(str_ct str, const char *fmt, ...)
{
    va_list ap;
    
    assert_str(str);
    return_error_if_fail(fmt, E_STR_INVALID_FORMAT, NULL);
    
    va_start(ap, fmt);
    str = error_propagate_ptr(_str_insert_f(str, _str_get_len(str), fmt, ap));
    va_end(ap);
    
    return str;
}

str_ct str_append_vf(str_ct str, const char *fmt, va_list ap)
{
    assert_str(str);
    return_error_if_fail(fmt, E_STR_INVALID_FORMAT, NULL);
    
    return error_propagate_ptr(_str_insert_f(str, _str_get_len(str), fmt, ap));
}

str_ct str_insert(str_ct str, size_t pos, str_const_ct sub)
{
    assert_str(sub);
    
    if(_str_is_binary(sub))
        return error_propagate_ptr(str_insert_b(str, pos, sub->data, _str_get_len(sub)));
    else
        return error_propagate_ptr(str_insert_cn(str, pos, (char*)sub->data, _str_get_len(sub)));
}

str_ct str_insert_n(str_ct str, size_t pos, str_const_ct sub, size_t len)
{
    assert_str(sub);
    
    len = MIN(len, _str_get_len(sub));
    
    if(_str_is_binary(sub))
        return error_propagate_ptr(str_insert_b(str, pos, sub->data, len));
    else
        return error_propagate_ptr(str_insert_cn(str, pos, (char*)sub->data, len));
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
        return error_propagate(), NULL;
    
    _str_set_flags(str, FLAG_BINARY);
    
    return str;
}

str_ct str_insert_set(str_ct str, size_t pos, size_t len, char c)
{
    assert_str(str);
    return_error_if_fail(pos <= _str_get_len(str), E_STR_OUT_OF_BOUNDS, NULL);
    
    if(!(str = _str_insert(str, pos, NULL, len)))
        return error_propagate(), NULL;
    
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
    str = error_propagate_ptr(_str_insert_f(str, pos, fmt, ap));
    va_end(ap);
    
    return str;
}

str_ct str_insert_vf(str_ct str, size_t pos, const char *fmt, va_list ap)
{
    assert_str(str);
    return_error_if_fail(fmt, E_STR_INVALID_FORMAT, NULL);
    return_error_if_fail(pos <= _str_get_len(str), E_STR_OUT_OF_BOUNDS, NULL);
    
    return error_propagate_ptr(_str_insert_f(str, pos, fmt, ap));
}
