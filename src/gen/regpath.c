/*
 * Copyright (c) 2020 Martin Rödel a.k.a. Yomin Nimoy
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

#include <ytil/gen/regpath.h>
#include <ytil/gen/path.h>
#include <ytil/ext/string.h>
#include <ytil/magic.h>
#include <ytil/def.h>


enum def
{
      MAGIC = define_magic('R', 'P', 'A')
};

typedef struct regpath
{
    DEBUG_MAGIC
    
    regpath_base_id base;
    path_ct path;
} regpath_st;

typedef struct regpath_base_info
{
    const char *abbr, *name;
} regpath_base_info_st;

static const regpath_base_info_st regpath_base_infos[] =
{
      [REGPATH_CLASSES_ROOT]        = { "HKCR", "HKEY_CLASSES_ROOT" }
    , [REGPATH_CURRENT_CONFIG]      = { "HKCC", "HKEY_CURRENT_CONFIG" }
    , [REGPATH_CURRENT_USER]        = { "HKCU", "HKEY_CURRENT_USER" }
    , [REGPATH_DYN_DATA]            = { "HKDD", "HKEY_DYN_DATA" }
    , [REGPATH_LOCAL_MACHINE]       = { "HKLM", "HKEY_LOCAL_MACHINE" }
    , [REGPATH_PERFORMANCE_DATA]    = { "HKPD", "HKEY_PERFORMANCE_DATA" }
    , [REGPATH_USERS]               = { "HKU",  "HKEY_USERS" }
};

static const path_prop_st regpath_prop = { .sep = "\\", .case_sensitive = false };

static const error_info_st error_infos[] =
{
      ERROR_INFO(E_REGPATH_INVALID_BASE, "Invalid registry base.")
};


static regpath_ct regpath_new_empty(void)
{
    regpath_ct path;
    
    if(!(path = calloc(1, sizeof(regpath_st))))
        return error_wrap_errno(calloc), NULL;
    
    init_magic(path);
    
    return path;
}

regpath_ct regpath_new(str_const_ct str)
{
    return error_pass_ptr(regpath_new_cn(str_c(str), str_len(str)));
}

regpath_ct regpath_new_c(const char *str)
{
    return error_pass_ptr(regpath_new_cn(str, strlen(str)));
}

regpath_ct regpath_new_cn(const char *str, size_t len)
{
    regpath_ct path;
    
    if(!(path = regpath_new_empty()))
        return error_pass(), NULL;
    
    if(!regpath_set_cn(path, str, len))
        return error_pass(), regpath_free(path), NULL;
    
    return path;
}

regpath_ct regpath_new_with_base(regpath_base_id base, str_const_ct str)
{
    return error_pass_ptr(regpath_new_with_base_cn(base, str_c(str), str_len(str)));
}

regpath_ct regpath_new_with_base_c(regpath_base_id base, const char *str)
{
    return error_pass_ptr(regpath_new_with_base_cn(base, str, strlen(str)));
}

regpath_ct regpath_new_with_base_cn(regpath_base_id base, const char *str, size_t len)
{
    regpath_ct path;
    
    if(!(path = regpath_new_empty()))
        return error_pass(), NULL;
    
    if(!regpath_set_with_base_cn(path, base, str, len))
        return error_pass(), regpath_free(path), NULL;
    
    return path;
}

regpath_ct regpath_dup(regpath_const_ct path)
{
    regpath_ct npath;
    
    assert_magic(path);
    
    if(!(npath = calloc(1, sizeof(regpath_st))))
        return error_wrap_errno(calloc), NULL;
    
    memcpy(npath, path, sizeof(regpath_st));
    
    if(path->path && !(npath->path = path_dup(path->path)))
        return error_wrap(), regpath_free(npath), NULL;
    
    return npath;
}

void regpath_free(regpath_ct path)
{
    if(path->path)
        path_free(path->path);
    
    free(path);
}

bool regpath_is_equal(regpath_const_ct path1, regpath_const_ct path2)
{
    assert_magic(path1);
    assert_magic(path2);
    
    return path1->base == path2->base
        && ((!path1->path && !path2->path)
            || (path1->path && path2->path
                && path_is_equal(path1->path, path2->path, &regpath_prop)));
}

regpath_base_id regpath_base(regpath_const_ct path)
{
    assert_magic(path);
    
    return path->base;
}

size_t regpath_depth(regpath_const_ct path)
{
    assert_magic(path);
    
    return path->path ? path_depth(path->path) : 0;
}

size_t regpath_len(regpath_const_ct path)
{
    assert_magic(path);
    
    return strlen(regpath_base_infos[path->base].name)
        + (path->path ? path_len(path->path, &regpath_prop) : 0);
}

regpath_ct regpath_set(regpath_ct path, str_const_ct str)
{
    return error_pass_ptr(regpath_set_cn(path, str_c(str), str_len(str)));
}

regpath_ct regpath_set_c(regpath_ct path, const char *str)
{
    return error_pass_ptr(regpath_set_cn(path, str, strlen(str)));
}

regpath_ct regpath_set_cn(regpath_ct path, const char *str, size_t len)
{
    regpath_base_id base;
    size_t blen;
    
    return_error_if_fail(str && len, E_REGPATH_INVALID_BASE, NULL);
    
    for(base=0; base < REGPATH_BASES; base++)
    {
        blen = strlen(regpath_base_infos[base].abbr);
        
        if(strnprefix(regpath_base_infos[base].abbr, blen, str, len))
            break;
        
        blen = strlen(regpath_base_infos[base].name);
        
        if(strnprefix(regpath_base_infos[base].name, blen, str, len))
            break;
    }
    
    if(base == REGPATH_BASES || (len-blen > 0 && !strchr(regpath_prop.sep, str[blen])))
        return error_set(E_REGPATH_INVALID_BASE), NULL;
    
    return error_pass_ptr(regpath_set_with_base_cn(path, base, str+blen, len-blen));
}

regpath_ct regpath_set_with_base(regpath_ct path, regpath_base_id base, str_const_ct str)
{
    return error_pass_ptr(regpath_set_with_base_cn(path, base, str_c(str), str_len(str)));
}

regpath_ct regpath_set_with_base_c(regpath_ct path, regpath_base_id base, const char *str)
{
    return error_pass_ptr(regpath_set_with_base_cn(path, base, str, strlen(str)));
}

regpath_ct regpath_set_with_base_cn(regpath_ct path, regpath_base_id base, const char *str, size_t len)
{
    assert_magic(path);
    return_error_if_fail(base < REGPATH_BASES, E_REGPATH_INVALID_BASE, NULL);
    
    if(!str || !len)
    {
        if(path->path)
        {
            path_free(path->path);
            path->path = NULL;
        }
    }
    else if(!path->path)
    {
        if(!(path->path = path_new_cn(str, len, &regpath_prop)))
            return error_wrap(), NULL;
    }
    else
    {
        if(!path_set_cn(path->path, str, len, &regpath_prop))
            return error_wrap(), NULL;
    }
    
    path->base = base;
    
    return path;
}

regpath_ct regpath_set_base(regpath_ct path, regpath_base_id base)
{
    assert_magic(path);
    return_error_if_fail(base < REGPATH_BASES, E_REGPATH_INVALID_BASE, NULL);
    
    path->base = base;
    
    return path;
}

regpath_ct regpath_append(regpath_ct path, str_const_ct str)
{
    return error_pass_ptr(regpath_append_cn(path, str_c(str), str_len(str)));
}

regpath_ct regpath_append_c(regpath_ct path, const char *str)
{
    return error_pass_ptr(regpath_append_cn(path, str, strlen(str)));
}

regpath_ct regpath_append_cn(regpath_ct path, const char *str, size_t len)
{
    assert_magic(path);
    
    if(!path->path)
    {
        if(!(path->path = path_new_cn(str, len, &regpath_prop)))
            return error_wrap(), NULL;
    }
    else
    {
        if(!path_append_cn(path->path, str, len, &regpath_prop))
            return error_wrap(), NULL;
    }
    
    return path;
}

regpath_ct regpath_drop(regpath_ct path, size_t n)
{
    assert_magic(path);
    
    if(path->path)
        path_drop(path->path, n);
    
    return path;
}

str_ct regpath_get(regpath_const_ct path)
{
    str_ct str;
    
    assert_magic(path);
    
    if(!path->path)
    {
        if(!(str = str_new_s(regpath_base_infos[path->base].name)))
            return error_wrap(), NULL;
    }
    else
    {
        if(!(str = path_get(path->path, &regpath_prop)))
            return error_wrap(), NULL;
        
        if(!str_prepend_f(str, "%s%c", regpath_base_infos[path->base].name, regpath_prop.sep[0]))
            return error_wrap(), str_unref(str), NULL;
    }
    
    return str;
}
