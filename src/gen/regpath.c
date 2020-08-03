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
    , ERROR_INFO(E_REGPATH_MALFORMED, "Malformed registry path.")
};


static regpath_ct regpath_new_empty(void)
{
    regpath_ct path;
    
    if(!(path = calloc(1, sizeof(regpath_st))))
        return error_wrap_errno(calloc), NULL;
    
    if(!(path->path = path_new_empty()))
        return error_wrap(), free(path), NULL;
    
    init_magic(path);
    
    return path;
}

regpath_ct regpath_new(str_const_ct str)
{
    regpath_ct path;
    
    if(!(path = regpath_new_empty()))
        return error_pass(), NULL;
    
    if(!regpath_set(path, str))
        return error_pass(), regpath_free(path), NULL;
    
    return path;
}

regpath_ct regpath_new_base(regpath_base_id base, str_const_ct str)
{
    regpath_ct path;
    
    if(!(path = regpath_new_empty()))
        return error_pass(), NULL;
    
    if(!regpath_set_base(path, base, str))
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
    
    if(!(npath->path = path_dup(path->path)))
        return error_wrap(), free(npath), NULL;
    
    return npath;
}

void regpath_free(regpath_ct path)
{
    path_free(path->path);
    free(path);
}

bool regpath_is_equal(regpath_const_ct path1, regpath_const_ct path2)
{
    assert_magic(path1);
    assert_magic(path2);
    
    return path1->base == path2->base
        && path_is_equal(path1->path, path2->path, &regpath_prop);
}

regpath_base_id regpath_base(regpath_const_ct path)
{
    assert_magic(path);
    
    return path->base;
}

size_t regpath_depth(regpath_const_ct path)
{
    assert_magic(path);
    
    return path_depth(path->path);
}

size_t regpath_len(regpath_const_ct path)
{
    assert_magic(path);
    
    return strlen(regpath_base_infos[path->base].name)
         + path_len_nonempty(path->path, &regpath_prop);
}

regpath_ct regpath_set(regpath_ct path, str_const_ct str)
{
    regpath_base_id base;
    size_t blen;
    char next;
    
    return_error_if_fail(str && !str_is_empty(str), E_REGPATH_MALFORMED, NULL);
    
    for(base=0; base < REGPATH_BASES; base++)
    {
        blen = strlen(regpath_base_infos[base].abbr);
        
        if(str_is_prefix_cn(str, regpath_base_infos[base].abbr, blen))
            break;
        
        blen = strlen(regpath_base_infos[base].name);
        
        if(str_is_prefix_cn(str, regpath_base_infos[base].name, blen))
            break;
    }
    
    if(base == REGPATH_BASES
    || ((next = str_at(str, blen)) && !strchr(regpath_prop.sep, next)))
        return error_set(E_REGPATH_INVALID_BASE), NULL;
    
    return error_pass_ptr(regpath_set_base(path, base, OFFSTR(str, blen)));
}

regpath_ct regpath_set_base(regpath_ct path, regpath_base_id base, str_const_ct str)
{
    assert_magic(path);
    return_error_if_fail(base < REGPATH_BASES, E_REGPATH_INVALID_BASE, NULL);
    
    if(!path_set(path->path, str, &regpath_prop))
        return error_wrap(), NULL;
    
    path_set_absolute(path->path, true);
    path_set_trailing(path->path, false);
    
    path->base = base;
    
    return path;
}

regpath_ct regpath_append(regpath_ct path, str_const_ct str)
{
    assert_magic(path);
    
    if(!path_append(path->path, str, &regpath_prop))
        return error_wrap(), NULL;
    
    return path;
}

regpath_ct regpath_drop(regpath_ct path, size_t n)
{
    assert_magic(path);
    
    path_drop(path->path, n);
    
    return path;
}

str_ct regpath_get(regpath_const_ct path)
{
    str_ct str;
    
    assert_magic(path);
    
    if(!(str = path_get_nonempty(path->path, &regpath_prop)))
        return error_wrap(), NULL;
    
    if(!str_prepend_c(str, regpath_base_infos[path->base].name))
        return error_wrap(), str_unref(str), NULL;
    
    return str;
}
